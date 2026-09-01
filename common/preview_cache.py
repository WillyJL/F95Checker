"""Disk cache and CPU-side processing for game preview images.

This module deliberately does not know about GUI or OpenGL state.  The game
loader owns the indexed list of ImageHelper instances; this module only
returns a cache path (or a processing error) for one preview URL.
"""

import asyncio
import hashlib
import io
import itertools
import os
import pathlib
from fractions import Fraction

import aiofiles
from PIL import Image


class PreviewCache:
    """Resolve, download, and process one game's preview cache."""

    target_height = 400  # 2x the owner's 200px gallery height

    def __init__(self, preview_dir: pathlib.Path, settings, api, image_ext):
        self.preview_dir = preview_dir
        self.settings = settings
        self.api = api
        self.image_ext = image_ext

    @staticmethod
    def digest(url: str) -> str:
        return hashlib.sha1(url.encode("utf-8")).hexdigest()

    async def _write_atomic(self, path: pathlib.Path, data: bytes):
        temporary = path.with_name(f".{path.name}.tmp")
        try:
            async with aiofiles.open(temporary, "wb") as file:
                await file.write(data)
            os.replace(temporary, path)
        finally:
            temporary.unlink(missing_ok=True)

    @classmethod
    def _process_still(cls, data: bytes, quality: int) -> bytes:
        with Image.open(io.BytesIO(data)) as source:
            source_format = (source.format or "").upper()
            if source_format not in ("JPEG", "AVIF"):
                return data
            image = source.copy()

        resized = image.height > cls.target_height
        if resized:
            scale = cls.target_height / image.height
            replacement = image.resize(
                (max(1, round(image.width * scale)), cls.target_height),
                Image.Resampling.LANCZOS,
            )
            image.close()
            image = replacement

        # Avoid rewriting an already suitable AVIF. JPEG is re-encoded to
        # apply the configured quality even when its dimensions are unchanged.
        if source_format == "AVIF" and not resized:
            image.close()
            return data

        output = io.BytesIO()
        if source_format == "JPEG":
            if image.mode not in ("RGB", "L"):
                image = image.convert("RGB")
            image.save(output, format="JPEG", quality=max(1, min(int(quality), 100)),
                       optimize=True, progressive=True)
        else:
            image.save(output, format="AVIF")
        image.close()
        processed = output.getvalue()
        with Image.open(io.BytesIO(processed)) as check:
            check.verify()
        return processed

    def _convert_gif_to_webm(self, data: bytes) -> bytes:
        import av

        with av.open(io.BytesIO(data), format="gif") as source:
            stream = next(iter(source.streams.video), None)
            if stream is None:
                raise ValueError("GIF contains no video stream")
            frames = source.decode(stream.index)
            first = next(frames, None)
            if first is None:
                raise ValueError("GIF contains no frames")

            width, height = first.width, first.height
            if height > self.target_height:
                scale = self.target_height / height
                width, height = round(width * scale), self.target_height
            width, height = max(2, width & ~1), max(2, height & ~1)
            codec = "libvpx-vp9" if self.settings.preview_webm_codec.name == "VP9" else "libvpx"
            quality = max(1, min(int(self.settings.preview_webm_quality), 100))
            crf = round(63 - (quality / 100) * 59)
            speed = max(0, min(int(self.settings.preview_webm_speed), 10))
            output_bytes = io.BytesIO()
            with av.open(output_bytes, mode="w", format="webm") as output:
                video = output.add_stream(codec, rate=1000, options={
                    "crf": str(crf), "b": "0", "cpu-used": str(speed),
                })
                video.width, video.height, video.pix_fmt = width, height, "yuv420p"
                timestamp = 0
                count = 0
                max_frames = max(int(self.settings.preview_max_animation_frames), 0)
                max_duration = max(int(self.settings.preview_max_animation_duration), 0) * 1000
                for frame in itertools.chain((first,), frames):
                    if max_frames and count >= max_frames:
                        break
                    if max_duration and timestamp >= max_duration:
                        break
                    encoded = frame.reformat(width=width, height=height, format="yuv420p")
                    encoded.pts = timestamp
                    encoded.time_base = Fraction(1, 1000)
                    for packet in video.encode(encoded):
                        output.mux(packet)
                    if frame.duration is not None and frame.time_base is not None:
                        duration = round(float(frame.duration * frame.time_base) * 1000)
                    else:
                        duration = 100
                    timestamp += max(20, duration)
                    count += 1
                for packet in video.encode():
                    output.mux(packet)

        result = output_bytes.getvalue()
        self._validate_webm_bytes(result)
        return result

    @staticmethod
    def _validate_webm_bytes(data: bytes):
        import av
        with av.open(io.BytesIO(data), mode="r", format="webm") as container:
            stream = next(iter(container.streams.video), None)
            if stream is None or next(container.decode(stream.index), None) is None:
                raise ValueError("WebM contains no decodable video frame")

    async def _valid_webm(self, path: pathlib.Path) -> bool:
        try:
            await asyncio.to_thread(self._validate_webm_bytes, await asyncio.to_thread(path.read_bytes))
            return True
        except Exception:
            path.unlink(missing_ok=True)
            return False

    async def resolve(self, url: str) -> tuple[pathlib.Path | None, str | None]:
        if not url.startswith(("http://", "https://")):
            return None, None
        digest = self.digest(url)
        glob = f"{digest}.*"
        self.preview_dir.mkdir(parents=True, exist_ok=True)
        paths = list(self.preview_dir.glob(glob))
        webm = next((p for p in paths if p.suffix.lower() == ".webm"), None)
        gifs = [p for p in paths if p.suffix.lower() == ".gif"]
        stills = [p for p in paths if p.suffix.lower() in (".jpg", ".jpeg", ".avif")]

        if webm and await self._valid_webm(webm):
            for path in gifs:
                path.unlink(missing_ok=True)
            return webm, None

        path = stills[0] if stills else (gifs[0] if gifs else None)
        if path is None:
            data, _ = await self.api.download_image(url)
            if not data:
                return None, "Image download returned no data"
            try:
                data = await asyncio.to_thread(self._process_still, data, self.settings.preview_jpeg_quality)
            except Exception:
                # A source image remains useful if optional processing fails.
                pass
            path = self.preview_dir / f"{digest}.{self.image_ext(data)}"
            await self._write_atomic(path, data)

        # Migrate older full-size still previews lazily when they are opened.
        # This keeps the new cache policy compatible with an existing images
        # directory without requiring a separate one-time migration command.
        if path.suffix.lower() in (".jpg", ".jpeg", ".avif"):
            try:
                with Image.open(path) as existing:
                    needs_resize = existing.height > self.target_height
                if needs_resize:
                    data = await asyncio.to_thread(path.read_bytes)
                    processed = await asyncio.to_thread(
                        self._process_still, data, self.settings.preview_jpeg_quality
                    )
                    await self._write_atomic(path, processed)
            except Exception:
                # The original cached file remains usable; ImageHelper will
                # surface a decode error if it is not actually readable.
                pass

        error = None
        if path.suffix.lower() == ".gif" and self.settings.preview_preserve_animation:
            try:
                data = await asyncio.to_thread(path.read_bytes)
                webm_data = await asyncio.to_thread(self._convert_gif_to_webm, data)
                webm_path = self.preview_dir / f"{digest}.webm"
                await self._write_atomic(webm_path, webm_data)
                path.unlink(missing_ok=True)
                path = webm_path
            except asyncio.CancelledError:
                raise
            except Exception as exc:
                error = str(exc)
        return path, error

    def cleanup(self, digests: set[str]):
        if not self.preview_dir.is_dir():
            return
        for path in self.preview_dir.iterdir():
            if path.is_file() and path.stem in digests:
                continue
            if path.is_file() and len(path.stem) == 40:
                path.unlink(missing_ok=True)
