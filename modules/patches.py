from common import meta

def apply():
    # Force UTF-8
    import sys
    sys.getfilesystemencoding = lambda: "utf-8"
    import locale
    locale.getpreferredencoding = lambda do_setlocale=True: "utf-8"
    import io
    real_io_text_encoding = io.text_encoding
    io.text_encoding = lambda encoding, stacklevel=2: "utf-8" if encoding is None else real_io_text_encoding(encoding)
    import os
    if not sys.platform.startswith("win"):
        real_os_device_encoding = os.device_encoding
        os.device_encoding = lambda fd: None if real_os_device_encoding(fd) is None else "utf-8"

    # Create dummy IO streams if missing
    if not sys.stdout: sys.stdout = open(os.devnull, "w")
    if not sys.stderr: sys.stderr = open(os.devnull, "w")

    # Fix frozen load paths
    if meta.frozen:
        if sys.platform.startswith("linux"):
            session_type = os.environ.get('XDG_SESSION_TYPE')
            if session_type in ("x11", "wayland"):
                library = meta.self_path / f"lib/glfw/{session_type}/libglfw.so"
                if library.is_file():
                    os.environ["PYGLFW_LIBRARY"] = str(library)
        elif sys.platform.startswith("darwin"):
            process = meta.self_path.parent / "Helpers/QtWebEngineProcess.app/Contents/MacOS/QtWebEngineProcess"
            if process.is_file():
                os.environ["QTWEBENGINEPROCESS_PATH"] = str(process)

    # Pillow image loading fixes
    from PIL import ImageFile, PngImagePlugin
    ImageFile.LOAD_TRUNCATED_IMAGES = True
    PngImagePlugin.MAX_TEXT_CHUNK *= 10

    # Optimize OpenGL
    import glfw
    import OpenGL
    glfw.ERROR_REPORTING = meta.debug
    for option in ("ERROR_LOGGING", "ERROR_CHECKING", "CONTEXT_CHECKING"):
        setattr(OpenGL, option, meta.debug)
    if meta.debug:
        import logging
        glfw.ERROR_REPORTING = {
            65548: "ignore",  # Wayland: The platform does not support window position/icon
            None: "raise",
        }
        logging.basicConfig()

    # Register archive formats
    import pathlib
    import shutil
    import stat

    import tarfile
    tarfile.TarFile.extraction_filter = tarfile.data_filter  # Fix path and link traversal

    import zipfile_deflate64 as zipfile
    def unpack_zipfile(archive, path):
        if not zipfile.is_zipfile(archive):
            raise shutil.ReadError(f"{archive} is not a zip file")
        with zipfile.ZipFile(archive) as arc:
            arc._ignore_invalid_names = True  # Fix path and link traversal in arc.extract()
            # Extract manually to create symlinks and apply permissions
            path = pathlib.Path(path)
            for file in arc.filelist:
                mode = file.external_attr >> 16
                if (mode & stat.S_IFLNK) == stat.S_IFLNK:
                    extracted = path / file.filename
                    if path.resolve() not in extracted.resolve().parents:
                        # Path traversal, ignore
                        continue
                    symlink = arc.read(file).decode()
                    if path.resolve() not in (extracted.resolve().parent / symlink).resolve().parents:
                        # Link traversal, ignore
                        continue
                    if extracted.exists():
                        extracted.unlink()
                    extracted.symlink_to(symlink)
                    try:
                        extracted.lchmod(mode)
                    except NotImplementedError:
                        pass
                else:
                    extracted = arc.extract(file, path)
                    if extracted and mode != 0:
                        extracted = pathlib.Path(extracted)
                        extracted.chmod(mode)
    shutil.unregister_unpack_format("zip")
    shutil.register_unpack_format("zip", [".zip"], unpack_zipfile)

    import py7zr
    shutil.register_unpack_format("7zip", [".7z"], py7zr.unpack_7zarchive)  # py7zr handles symlinks and permissions

    import rarfile
    def unpack_rarfile(archive, path):
        if not rarfile.is_rarfile(archive):
            raise shutil.ReadError(f"{archive} is not a RAR file")
        with rarfile.RarFile(archive) as arc:
            arc.extractall(path)  # rarfile handles symlinks and permissions
    try:
        if rarfile.tool_setup():
            shutil.register_unpack_format("rar", [".rar"], unpack_rarfile)
    except rarfile.RarCannotExec:
        pass
