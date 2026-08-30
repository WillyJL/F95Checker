# https://gist.github.com/WillyJL/9c5116e5a11abd559c56f23aa1270de9
import functools
import os
import pathlib
import platform
import shutil
import struct
import subprocess
import tempfile
import threading
import time
import typing

from PIL import (
    Image,
    ImageSequence,
    UnidentifiedImageError
)
from OpenGL.GL.KHR import texture_compression_astc_ldr as gl_astc
from OpenGL.GL.ARB import texture_compression_bptc as gl_bptc
import OpenGL.GL as gl
import imgui
import zstd

from common.structs import (
    Os,
    TexCompress,
)
from external import (
    error,
    sync_thread,
    weakerset,
)
from modules.api import temp_prefix
from modules import globals

redraw = False
apply_queue = []
unload_queue = []
compress_counter = 0
compress_thread = None
_dummy_texture_id = None

ktx_durations = b"durationsms\0"
ktx_endianness = 0x04030201
ktx_magic = b"\xABKTX 11\xBB\r\n\x1A\n"
zstd_level = 3
zstd_magic = b"\x28\xB5\x2F\xFD"

astc_block = "6x6"
astc_format = gl_astc.GL_COMPRESSED_RGBA_ASTC_6x6_KHR
astc_pixfmt = gl.GL_RGBA
astc_quality = "80"
astcenc = None

bc7_format = gl_bptc.GL_COMPRESSED_RGBA_BPTC_UNORM_ARB
bc7_pixfmt = gl.GL_RGBA
compressonator_encoder = None
compressonator = None


def setup():
    global compress_thread

    compress_thread = threading.Thread(target=_compress_thread, daemon=True)
    compress_thread.start()


def _cpu_supports_hpc():
    from external import cpuinfo
    flags = cpuinfo.get_cpu_info().get("flags", ())
    return all(flag in flags for flag in ("avx2", "sse4_2", "popcnt", "f16c"))


def _find_astcenc():
    global astcenc
    if astcenc is None:
        # Windows: F95Checker/lib/astcenc/astcenc-(avx2|sse2|neon).exe
        # Linux: F95Checker/lib/astcenc/astcenc-(avx2|sse2)
        # MacOS: F95Checker/lib/astcenc/astcenc
        _astcenc = globals.self_path / "lib/astcenc"
        if globals.os is Os.MacOS:
            _astcenc /= "astcenc"
        elif globals.os is Os.Windows and platform.machine().startswith("ARM"):
            _astcenc /= "astcenc-neon.exe"
        else:
            if _cpu_supports_hpc():
                _astcenc /= "astcenc-avx2"
            else:
                _astcenc /= "astcenc-sse2"
            if globals.os is Os.Windows:
                _astcenc = _astcenc.with_suffix(".exe")
        if not _astcenc.is_file():
            # Not bundled, look in PATH for astcenc-(avx2|sse2)[.exe] and astcenc[.exe]
            _astcenc = shutil.which(_astcenc.name) or shutil.which(_astcenc.with_stem("astcenc").name)
            if _astcenc:
                _astcenc = pathlib.Path(_astcenc)
            else:
                _astcenc = False
        if _astcenc:
            _astcenc = _astcenc.absolute()
        astcenc = _astcenc
    return astcenc


def _find_compressonator():
    global compressonator, compressonator_encoder
    if compressonator is None:
        # Windows: F95Checker/lib/compressonator/compressonatorcli.exe
        # Linux: F95Checker/lib/compressonator/compressonatorcli
        # MacOS: Not supported
        _compressonator = globals.self_path / "lib/compressonator"
        if globals.os is Os.Windows:
            _compressonator /= "compressonatorcli.exe"
        else:
            _compressonator /= "compressonatorcli"
        if not _compressonator.is_file():
            # Not bundled, look in PATH for compressonatorcli[.exe]
            _compressonator = shutil.which(_compressonator.name)
            if _compressonator:
                _compressonator = pathlib.Path(_compressonator)
            else:
                _compressonator = False
        if _compressonator:
            _compressonator = _compressonator.absolute()
            if _cpu_supports_hpc():
                compressonator_encoder = "HPC"
            else:
                compressonator_encoder = "CPU"
        compressonator = _compressonator
    return compressonator

def post_draw(draw_time: float):
    # Unload images if not visible
    if globals.settings.unload_offscreen_images:
        hidden = globals.gui.minimized or globals.gui.hidden
        for image in ImageHelper.instances:
            if image.loaded and (hidden or not image.shown):
                unload_queue.append(image)
            else:
                image.shown = False
    for image in reversed(unload_queue):
        image.unload()
        unload_queue.remove(image)
    # At least 1 apply per frame
    # Apply more based on how much delta time and draw time we have
    if apply_queue:
        apply_time_max_total = max(0, imgui.get_io().delta_time - draw_time)
        apply_stop = time.perf_counter() + apply_time_max_total
        apply_parallel = len(apply_queue)
        apply_time_max = apply_time_max_total / apply_parallel
        apply_idx = 0
        for _ in range(apply_parallel):
            if apply_queue[apply_idx].apply(apply_time_max):
                apply_queue.pop(apply_idx)
            else:
                apply_idx += 1
            if time.perf_counter() > apply_stop:
                break


def _compress_thread():
    while True:
        if globals.settings.tex_compress is TexCompress.Disabled:
            time.sleep(5)
            continue

        # Iterating over ImageHelper.instances (WeakerSet) holds a lock over it, blocking the main loop
        # Since this is a lengthy operation, this is a very bad idea
        # Instead we iterate once quickly to make a list, then try compressing
        # Since it's a lengthy operation, after compressing we might be iterating over an out-of-sync copy of ImageHelper.instances
        # That could mean there are since-deleted ImageHelper instances in our iterator copy, which we don't want to compress
        # So break after each lengthy operation to update our iterator
        for image in list(ImageHelper.instances):
            if image._maybe_compress():
                time.sleep(0)
                break
        else:  # Didn't break
            time.sleep(5)


def dummy_texture_id():
    global _dummy_texture_id
    if _dummy_texture_id is None:
        _dummy_texture_id = gl.glGenTextures(1)
        gl.glBindTexture(gl.GL_TEXTURE_2D, _dummy_texture_id)
        gl.glTexImage2D(gl.GL_TEXTURE_2D, 0, gl.GL_RGBA, 0, 0, 0, gl.GL_RGBA, gl.GL_UNSIGNED_BYTE, b"\x00\x00\x00\xff")
    return _dummy_texture_id


@functools.cache
def _crop_to_ratio(width, height, ratio: int | float, fit=False):
    img_ratio = width / height
    if (img_ratio >= ratio) != fit:
        crop_h = height
        crop_w = crop_h * ratio
        crop_x = (width - crop_w) / 2
        crop_y = 0
        left = crop_x / width
        top = 0
        right = (crop_x + crop_w) / width
        bottom = 1
    else:
        crop_w = width
        crop_h = crop_w / ratio
        crop_y = (height - crop_h) / 2
        crop_x = 0
        left = 0
        top = crop_y / height
        right = 1
        bottom = (crop_y + crop_h) / height
    return (left, top), (right, bottom)


class ImageHelper:
    instances = weakerset.WeakerSet()

    __slots__ = (
        "width",
        "height",
        "frame",
        "glob",
        "elapsed",
        "loaded",
        "loading",
        "applied",
        "_missing",
        "prev_time",
        "animated",
        "_load_error",
        "_compress_error",
        "_pending_reload",
        "textures",
        "durations",
        "texture_ids",
        "resolved_path",
        "path",
        "shown",
        "lock",
        "__weakref__",
    )

    def __init__(self, path: str | pathlib.Path, glob=""):
        self.width = 1
        self.height = 1
        self.frame = 0
        self.glob = glob
        self.elapsed = 0.0
        self.loaded = False
        self.loading = False
        self.applied = False
        self._missing = None
        self.prev_time = 0.0
        self.animated = False
        self._load_error: str = None
        self._compress_error: str = None
        self.textures: list[bytes] = []
        self.durations: list[float] = []
        self.texture_ids: list[int] = []
        self.resolved_path: pathlib.Path = None
        self.path: pathlib.Path = pathlib.Path(path)
        self.shown = False
        self.lock = threading.RLock()
        type(self).instances.add(self)

    @property
    def missing(self):
        if self._missing is None:
            self.resolve()
        return self._missing

    @property
    def error(self):
        if self._compress_error:
            return self._compress_error
        if self.loaded:
            return self._load_error
        return None

    def resolve(self):
        with self.lock:
            if self._missing is not None:
                return

            self.resolved_path = self.path

            if self.glob:
                paths = list(self.resolved_path.glob(self.glob))
                if not paths:
                    self._missing = True
                    return
                if globals.settings.tex_compress is TexCompress.ASTC:
                    # Prefer ASTC, then .gif, then anything else, then other compression
                    sorting = lambda path: 1 if path.name.endswith(".astc.ktx.zst") else 2 if path.suffix == ".gif" else 3 if not path.name.endswith(".bc7.ktx.zst") else 4
                elif globals.settings.tex_compress is TexCompress.BC7:
                    # Prefer BC7, then .gif, then anything else, then other compression
                    sorting = lambda path: 1 if path.name.endswith(".bc7.ktx.zst") else 2 if path.suffix == ".gif" else 3 if not path.name.endswith(".astc.ktx.zst") else 4
                else:
                    # Prefer .gif files, avoid compressed files unless nothing else available
                    sorting = lambda path: 1 if path.suffix == ".gif" else 2 if path.suffix == ".zst" else 3
                paths.sort(key=sorting)
                self.resolved_path = paths[0]

            # Choose compressed file by same name if not already using it
            if globals.settings.tex_compress is not TexCompress.Disabled and self.resolved_path.suffix != ".zst":
                ktx_path = self.resolved_path.with_suffix(f".{globals.settings.tex_compress.name.lower()}.ktx.zst")
                if ktx_path.is_file():
                    self.resolved_path = ktx_path

            self._missing = not self.resolved_path.is_file()

    def _compress_set_invalid(self, err: str):
        if self.loading:
            return
        self._compress_error = err
        if self.loaded:
            unload_queue.append(self)

    @classmethod
    def _build_ktx(cls, tex_format: int, tex_pixfmt: int, width: int, height: int, frames: list[tuple[bytes, int]]):
        ktx = bytearray(ktx_magic)  # identifier
        ktx += struct.pack("<I", ktx_endianness)  # endianness
        ktx += struct.pack("<I", 0)  # glType
        ktx += struct.pack("<I", 1)  # glTypeSize
        ktx += struct.pack("<I", 0)  # glFormat
        ktx += struct.pack("<I", tex_format)  # glInternalFormat
        ktx += struct.pack("<I", tex_pixfmt)  # glBaseInternalFormat
        ktx += struct.pack("<I", width)  # pixelWidth
        ktx += struct.pack("<I", height)  # pixelHeight
        ktx += struct.pack("<I", 0)  # pixelDepth
        if len(frames) > 1:
            ktx += struct.pack("<I", len(frames))  # numberOfArrayElements
        else:
            ktx += struct.pack("<I", 0)  # numberOfArrayElements
        ktx += struct.pack("<I", 1)  # numberOfFaces
        ktx += struct.pack("<I", 1)  # numberOfMipmapLevels

        if len(frames) > 1:
            ktx += struct.pack("<I", 16 + 4 * len(frames))  # bytesOfKeyValueData
            ktx += struct.pack("<I", 12 + 4 * len(frames))  # keyAndValueByteSize
            ktx += ktx_durations  # key
            for _, duration in frames:  # value
                ktx += struct.pack("<I", duration)
        else:
            ktx += struct.pack("<I", 0)  # bytesOfKeyValueData

        for texture, _ in frames:
            ktx += struct.pack("<I", len(texture))  # imageSize
            ktx += texture  # data

        return ktx

    def _compress_ktx(
        self,
        cli: typing.Callable[[str, str], list[str]],
        compressor_name: str,
        supported_formats: tuple[str],
        unsupported_msg: bytes,
        intermediary_format: str,
        texture_format: int,
        texture_pixfmt: int,
        format_name: str,
    ):
        path = self.resolved_path
        if path.suffix == ".zst":
            self._compress_set_invalid(
                f"No source image available to compress to {format_name}!\n"
                "Reset image in order to re-compress it"
            )
            return
        data = path.read_bytes()

        global compress_counter
        ktx = None

        ktx_temp_path = pathlib.Path(tempfile.mktemp(prefix=temp_prefix, suffix=".ktx"))
        def _ktx_compress_one(src_path: pathlib.Path):
            try:
                if globals.os is Os.Windows:
                    kwargs = dict(
                        creationflags=subprocess.DETACHED_PROCESS | subprocess.CREATE_NO_WINDOW,
                        startupinfo=subprocess.STARTUPINFO(dwFlags=subprocess.STARTF_USESHOWWINDOW),
                    )
                else:
                    kwargs = dict()
                subprocess.check_output(
                    cli(src_path, ktx_temp_path),
                    stderr=subprocess.STDOUT,
                    **kwargs,
                )
                ktx_temp = bytearray(ktx_temp_path.read_bytes())
                return ktx_temp, b""
            except subprocess.CalledProcessError as exc:
                err = f"Process returned code {exc.returncode}\n".encode()
                err += exc.stdout or b"No console output"
                return b"", err
            finally:
                ktx_temp_path.unlink(missing_ok=True)

        try:
            # Identify image format
            image = Image.open(path)
        except UnidentifiedImageError:
            self._compress_set_invalid(f"Pillow does not recognize this image format!")
            return

        frames_remaining = getattr(image, "n_frames", 1)
        compress_counter += frames_remaining
        global redraw
        redraw = True
        try:

            if image.format in supported_formats and not getattr(image, "is_animated", False):
                # Image may be compressable as is, try it
                ktx_temp, err = _ktx_compress_one(path)
                if ktx_temp:
                    # Compressed as is, just keep the ktx
                    ktx = ktx_temp
                else:
                    if unsupported_msg not in err:
                        # Something else went wrong, bail
                        self._compress_set_invalid(f"{compressor_name} failed to compress this image:\n{err.decode('utf-8', errors='replace')}")
                        return
                    # Image format not supported as is, use intermediary files

            if not ktx:
                # Can't compress as is, convert each frame to intermediary then compress
                intermediary_path = pathlib.Path(tempfile.mktemp(prefix=temp_prefix, suffix=intermediary_format))
                try:
                    frames = []
                    for i, frame in enumerate(ImageSequence.Iterator(image)):
                        frame.save(intermediary_path)
                        ktx_temp, err = _ktx_compress_one(intermediary_path)
                        if not ktx_temp:
                            self._compress_set_invalid(f"{compressor_name} failed to compress this image:\n{err.decode('utf-8', errors='replace')}")
                            return
                        magic = ktx_temp[0:12]
                        if magic != ktx_magic:
                            self._compress_set_invalid(f"{compressor_name} returned an invalid KTX file:\nWrong KTX magic, {bytes(magic)} != {ktx_magic}")
                            return
                        fmt = "<I" if struct.unpack("<I", ktx_temp[12:16])[0] == ktx_endianness else ">I"
                        if i == 0:
                            pix_w = struct.unpack(fmt, ktx_temp[36:40])[0]
                            pix_h = struct.unpack(fmt, ktx_temp[40:44])[0]
                        kv_len = struct.unpack(fmt, ktx_temp[60:64])[0]
                        tex_pos = 64 + kv_len
                        tex_len = struct.unpack(fmt, ktx_temp[tex_pos:tex_pos + 4])[0]
                        tex_pos += 4
                        texture = ktx_temp[tex_pos:tex_pos + tex_len]
                        duration = int(frame.info.get("duration", 0))
                        if duration < 1:
                            duration = 100
                        frames.append((bytes(texture), duration))
                        frames_remaining -= 1
                        compress_counter -= 1
                    ktx = self._build_ktx(texture_format, texture_pixfmt, pix_w, pix_h, frames)
                except Exception:
                    self._compress_set_invalid(f"Failed {compressor_name} intermediary step:\n{error.text()}")
                    return
                finally:
                    intermediary_path.unlink(missing_ok=True)
        finally:
            compress_counter -= frames_remaining
            image.close()

        if not ktx:
            return
        ktx = zstd.compress(bytes(ktx), zstd_level)
        ktx_path = path.with_suffix(f".{format_name.lower()}.ktx.zst")

        # Discard the result if source image was changed/deleted during compression
        try:
            if self.resolved_path != path or self.missing or self.resolved_path.read_bytes() != data:
                return
        except FileNotFoundError:
            return

        ktx_path.write_bytes(ktx)
        self.reload()

    def _compress_astc(self):
        # Compress to ASTC
        if not _find_astcenc():
            self._compress_set_invalid(
                f"ASTC-Encoder not found!\n" + (
                    "Was it deleted?"
                    if globals.frozen and (globals.release or globals.build_number) else
                    "Download it and place it in PATH:\n"
                    "https://github.com/ARM-software/astc-encoder/releases/tag/5.1.0"
                )
            )
            return
        self._compress_ktx(
            cli=lambda src, dst: [astcenc, "-cl", src, dst, astc_block, astc_quality, "-perceptual", "-silent"],
            compressor_name="ASTC-Encoder",
            supported_formats=("PNG", "JPEG", "BMP"),
            unsupported_msg=b"unknown image type",
            intermediary_format=".bmp",
            texture_format=astc_format,
            texture_pixfmt=astc_pixfmt,
            format_name="ASTC",
        )

    def _compress_bc7(self):
        # Compress to BC7
        if not _find_compressonator():
            self._compress_set_invalid(
                "BC7 compression isn't supported on MacOS!\n"
                "Compressornator doesn't exist yet for MacOS"
                if globals.os is Os.MacOS else
                f"Compressonator not found!\n" + (
                    "Was it deleted?"
                    if globals.frozen and (globals.release or globals.build_number) else
                    "Download it and place it in PATH:\n"
                    "https://github.com/GPUOpen-Tools/compressonator/releases/tag/V4.5.52"
                )
            )
            return
        self._compress_ktx(
            cli=lambda src, dst: [compressonator, "-fd", "BC7", "-EncodeWith", compressonator_encoder, "-NumThreads", str(os.cpu_count()), src, dst],
            compressor_name="Compressonator",
            supported_formats=("PNG", "JPEG", "BMP"),
            unsupported_msg=b"Could not load source file",
            intermediary_format=".bmp",
            texture_format=bc7_format,
            texture_pixfmt=bc7_pixfmt,
            format_name="BC7",
        )

    def _maybe_compress(self):
        if self._compress_error or self.loading or self.missing:
            return False
        if globals.images_path / "previews" in self.resolved_path.parents:
            return False

        if globals.settings.tex_compress is TexCompress.ASTC and not self.resolved_path.name.endswith(".astc.ktx.zst"):
            self._compress_astc()
            return True

        if globals.settings.tex_compress is TexCompress.BC7 and not self.resolved_path.name.endswith(".bc7.ktx.zst"):
            self._compress_bc7()
            return True

    def _load_set_invalid(self, err: str):
        self._load_error = err
        self.loaded = True
        self.loading = False

    def _load_ktx_zst(self):
        # Load compressed KTX
        if not self.resolved_path.name.endswith((".astc.ktx.zst", ".bc7.ktx.zst")):
            self._load_set_invalid(
                "Unknown KTX texture format!\n"
                "Reset image in order to re-compress it"
            )
            return

        ktx = self.resolved_path.read_bytes()
        magic = ktx[0:4]
        if magic != zstd_magic:
            self._load_set_invalid(f"KTX malformed:\nWrong ZSTD magic, {magic} != {zstd_magic}")
            return
        ktx = bytearray(zstd.decompress(ktx))

        magic = ktx[0:12]
        if magic != ktx_magic:
            self._load_set_invalid(f"KTX malformed:\nWrong KTX magic, {bytes(magic)} != {ktx_magic}")
            return
        fmt = "<I" if struct.unpack("<I", ktx[12:16])[0] == ktx_endianness else ">I"

        gl_type = struct.unpack(fmt, ktx[16:20])[0]
        gl_type_size = struct.unpack(fmt, ktx[20:24])[0]
        gl_format = struct.unpack(fmt, ktx[24:28])[0]
        gl_internal_format = struct.unpack(fmt, ktx[28:32])[0]
        gl_internal_pixfmt = struct.unpack(fmt, ktx[32:36])[0]
        if gl_type != 0 or gl_type_size != 1 or gl_format != 0:
            self._load_set_invalid(f"KTX malformed:\nUncompressed texture, only ASTC (6x6) and BC7 supported")
            return
        if gl_internal_format not in (astc_format, bc7_format):
            self._load_set_invalid(f"KTX malformed:\nUnknown format, only ASTC (6x6) and BC7 supported")
            return
        if gl_internal_format == astc_format:
            pixfmt = astc_pixfmt
        elif gl_internal_format == bc7_format:
            pixfmt = bc7_pixfmt
        if gl_internal_pixfmt != pixfmt:
            self._load_set_invalid(f"KTX malformed:\nWrong pixel format for compression type")
            return

        pix_w = struct.unpack(fmt, ktx[36:40])[0]
        pix_h = struct.unpack(fmt, ktx[40:44])[0]
        pix_d = struct.unpack(fmt, ktx[44:48])[0]
        if pix_d != 0:
            self._load_set_invalid(f"KTX malformed:\n3D texture, only 2D supported")
            return
        self.width = pix_w
        self.height = pix_h

        array_len = struct.unpack(fmt, ktx[48:52])[0] or 1

        faces_count = struct.unpack(fmt, ktx[52:56])[0]
        mipmap_count = struct.unpack(fmt, ktx[56:60])[0]
        if faces_count != 1:
            self._load_set_invalid(f"KTX malformed:\nCubemap texture, only 2D supported")
            return
        if mipmap_count != 1:
            self._load_set_invalid(f"KTX malformed:\nMipmapped texture, only non-mipmapped supported")
            return

        durations = []
        kv_len = struct.unpack(fmt, ktx[60:64])[0]
        if kv_len:
            kv = ktx[64:64 + kv_len]
            while kv:
                kv_pair_len = struct.unpack(fmt, kv[0:4])[0]
                if kv[4:4 + kv_pair_len].startswith(ktx_durations):
                    durationsms = kv[4 + len(ktx_durations):4 + kv_pair_len]
                    while len(durationsms) >= 4:
                        duration = struct.unpack(fmt, durationsms[0:4])[0]
                        if duration < 1:
                            duration = 100
                        durations.append(duration)
                        durationsms = durationsms[4:]
                    break
                kv = kv[4 + kv_pair_len:]

        frames_data = ktx[64 + kv_len:]
        data_pos = 0
        first_frame = True
        while len(self.textures) < array_len and data_pos < len(frames_data):
            texture_len = struct.unpack(fmt, frames_data[data_pos:data_pos + 4])[0]
            data_pos += 4
            texture = bytes(frames_data[data_pos:data_pos + texture_len])
            data_pos += texture_len
            self.textures.append((texture, gl_internal_format))
            if len(durations) < len(self.textures):
                duration = 100
            else:
                duration = durations[len(self.textures) - 1]
            self.durations.append(duration / 1000)
            if first_frame:
                apply_queue.append(self)
                first_frame = False
            else:
                self.animated = True
            if not globals.settings.play_gifs:
                break

        if self.glob and globals.settings.tex_compress is not TexCompress.Disabled and globals.settings.tex_compress_replace:
            paths = list(self.path.glob(self.glob))
            if len(paths) > 1:
                try:
                    for path in paths:
                        if path != self.resolved_path:
                            path.unlink(missing_ok=True)
                except Exception:
                    pass

        self.loaded = True
        self.loading = False

    def _load_rgba(self):
        # Fallback to RGBA loading
        try:
            image = Image.open(self.resolved_path)
        except UnidentifiedImageError:
            self._load_set_invalid(f"Pillow does not recognize this image format!")
            return

        with image:
            self.width, self.height = image.size
            first_frame = True
            for frame in ImageSequence.Iterator(image):
                if frame.mode == "RGB":
                    texture = frame.tobytes("raw", "RGBX")
                elif frame.mode == "RGBA":
                    texture = frame.tobytes("raw", "RGBA")
                else:
                    texture = frame.convert("RGBA").tobytes("raw", "RGBA")
                self.textures.append((texture, gl.GL_RGBA))
                duration = frame.info.get("duration", 0)
                if duration < 1:
                    duration = 100
                self.durations.append(duration / 1000)
                if first_frame:
                    apply_queue.append(self)
                    first_frame = False
                else:
                    self.animated = True
                if not globals.settings.play_gifs:
                    break

        self.loaded = True
        self.loading = False

    def load(self):
        self._pending_reload = False
        self.loading = True

        if self.missing:
            self._load_set_invalid("Image file missing")
            return

        self.frame = 0
        self.elapsed = 0.0
        self.textures.clear()
        self.animated = False
        self.durations.clear()
        # Don't reset width and height, keep ones from prior load (if any)
        # If this is first load, they're already (1, 1)
        # If this is a reload, they're unlikely to have changed, and even if they did there is no harm in giving the old size while loading the new image
        # Actually, it helps with dynamically sized layouts: in the case where unload_offscreen_images is on, this keeps the layout from jumping around

        if self.resolved_path.name.endswith(".ktx.zst"):
            self._load_ktx_zst()
        else:
            self._load_rgba()

        if self._pending_reload:
            self.reload()

    def apply(self, apply_time_max: float):
        apply_start = time.perf_counter()
        for i in range(len(self.texture_ids), len(self.textures)):
            (texture, texture_format) = self.textures[i]
            texture_id = gl.glGenTextures(1)
            self.texture_ids.extend([texture_id])
            gl.glBindTexture(gl.GL_TEXTURE_2D, texture_id)
            gl.glTexParameteri(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_MIN_FILTER, gl.GL_LINEAR)
            gl.glTexParameteri(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_MAG_FILTER, gl.GL_LINEAR)
            gl.glTexParameteri(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_WRAP_S, gl.GL_CLAMP_TO_BORDER)
            gl.glTexParameteri(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_WRAP_T, gl.GL_CLAMP_TO_BORDER)
            try:
                if texture_format == gl.GL_RGBA:
                    gl.glTexImage2D(gl.GL_TEXTURE_2D, 0, texture_format, self.width, self.height, 0, texture_format, gl.GL_UNSIGNED_BYTE, texture)
                else:
                    gl.glCompressedTexImage2D(gl.GL_TEXTURE_2D, 0, texture_format, self.width, self.height, 0, texture)
            except gl.GLError:
                self._load_error = "Error applying texture:\n" + error.text()
                break
            if time.perf_counter() - apply_start > apply_time_max:
                break
        if not self.loading and len(self.texture_ids) == len(self.textures):
            self.textures.clear()
            self.applied = True
            return True
        else:
            return False

    def unload(self):
        sync_thread.unqueue(self.load)
        if self.loaded:
            if self.texture_ids:
                gl.glDeleteTextures(self.texture_ids)
                self.texture_ids.clear()
            if self.textures:
                apply_queue.remove(self)
                self.textures.clear()
            if not self._missing and not self._load_error:
                self.loaded = False

    def reload(self):
        self._missing = None
        self._load_error = None
        self._compress_error = None
        self._pending_reload = True
        unload_queue.append(self)

    @property
    def texture_id(self):
        self.shown = True

        if not self.loaded:
            if not self.loading:
                self.loading = True
                self.applied = False
                # This next self.load() actually loads the image and does all the conversion. It takes time and resources!
                # self.load()
                # You can (and maybe should) run this in a thread! threading.Thread(target=self.load, daemon=True).start()
                # Or maybe setup an image thread and queue images to load one by one?
                # You could do this with https://gist.github.com/WillyJL/bb410bcc761f8bf5649180f22b7f3b44 like so:
                sync_thread.queue(self.load)
        else:
            if self._missing or self._load_error:
                return dummy_texture_id()

        if not self.texture_ids:
            return dummy_texture_id()

        if self.animated and globals.settings.play_gifs and (globals.gui.focused or globals.settings.play_gifs_unfocused):
            if self.prev_time != (new_time := imgui.get_time()):
                self.prev_time = new_time
                self.elapsed += imgui.get_io().delta_time
                while self.frame < (len(self.texture_ids) - 1) and (excess := self.elapsed - self.durations[max(self.frame, 0)]) > 0:
                    self.elapsed = excess
                    self.frame += 1
                    if self.frame == len(self.durations) - 1:
                        self.frame = 0

        return self.texture_ids[self.frame]

    def render(self, width: int, height: int, *args, **kwargs):
        if globals.settings.preload_nearby_images:
            cur = imgui.get_cursor_pos()
            size = imgui.get_io().display_size
            imgui.set_cursor_pos((cur.x - size.x, cur.y - size.y))
            should_draw = imgui.is_rect_visible(width + size.x * 2, height + size.y * 2)
            imgui.set_cursor_pos(cur)
        else:
            should_draw = imgui.is_rect_visible(width, height)
        if should_draw:
            if self.animated or self.loading:
                global redraw
                redraw = True
            if "rounding" in kwargs:
                flags = kwargs.pop("flags", None)
                if flags is None:
                    flags = imgui.DRAW_ROUND_CORNERS_ALL
                pos = imgui.get_cursor_screen_pos()
                pos2 = (pos.x + width, pos.y + height)
                draw_list = imgui.get_window_draw_list()
                draw_list.add_image_rounded(self.texture_id, tuple(pos), pos2, *args, flags=flags, **kwargs)
                imgui.dummy(width, height)
            else:
                imgui.image(self.texture_id, width, height, *args, **kwargs)
            return True
        else:
            # Skip if outside view
            imgui.dummy(width, height)
            return False

    def crop_to_ratio(self, ratio: int | float, fit=False):
        return _crop_to_ratio(self.width, self.height, ratio, fit)


# Example usage
if __name__ == "__main__":
    # Images are loaded lazily, you can create as many as you want,
    # they will only be loaded when shown for the first time.
    # GIFs are also supported!
    image = ImageHelper("example.png")
    # You can also use glob patterns, pass a folder path and add a file glob pattern:
    # image = ImageHelper("/path/to/images", glob="**/example.*")
    # Useful if you know the extension but not the name, or you know the name but not the extension

    show_bounding_rect = False
    # These are just to better illustrate cropping behavior, you don't need these in standard usage
    def draw_bounding_rect():
        draw_list = imgui.get_window_draw_list()
        draw_list.add_rect(*imgui.get_item_rect_min(), *imgui.get_item_rect_max(), imgui.get_color_u32_rgba(1, 1, 1, 1), thickness=2)

    while True:  # Your main window draw loop
        with imgui.begin("Example image"):
            scaled_width = image.width / 6
            scaled_height = image.height / 6

            _, show_bounding_rect = imgui.checkbox("Show bounding rect", show_bounding_rect)

            imgui.begin_group()
            ratio = 3.0
            imgui.text(f"Crop to ratio {ratio}:")
            image.render(scaled_width, scaled_height / ratio, *image.crop_to_ratio(ratio))
            if show_bounding_rect:
                draw_bounding_rect()

            ratio = 0.3
            imgui.text(f"Crop to ratio {ratio}:")
            image.render(scaled_width * ratio, scaled_height, *image.crop_to_ratio(ratio))
            if show_bounding_rect:
                draw_bounding_rect()

            ratio = 2.0
            imgui.text(f"Fit to ratio {ratio}:")
            image.render(scaled_width, scaled_height / ratio, *image.crop_to_ratio(ratio, fit=True))
            if show_bounding_rect:
                draw_bounding_rect()

            ratio = 0.4
            imgui.text(f"Fit to ratio {ratio}:")
            image.render(scaled_width * ratio, scaled_height, *image.crop_to_ratio(ratio, fit=True))
            if show_bounding_rect:
                draw_bounding_rect()
            imgui.end_group()

            imgui.same_line(spacing=30)

            imgui.begin_group()
            imgui.text("Scaled size:")
            image.render(scaled_width, scaled_height)
            if show_bounding_rect:
                draw_bounding_rect()

            imgui.text("Rounded corners:")
            image.render(scaled_width, scaled_height, rounding=26)
            if show_bounding_rect:
                draw_bounding_rect()

            imgui.text("Some rounded corners:")
            image.render(scaled_width, scaled_height, rounding=26, flags=imgui.DRAW_ROUND_CORNERS_TOP_LEFT | imgui.DRAW_ROUND_CORNERS_BOTTOM_RIGHT)
            if show_bounding_rect:
                draw_bounding_rect()
            imgui.end_group()

            imgui.same_line(spacing=30)

            imgui.begin_group()
            imgui.text("Actual size:")
            image.render(image.width, image.height)
            if show_bounding_rect:
                draw_bounding_rect()
            imgui.end_group()
