import os
import pathlib
import re
import shlex
import shutil

from common.structs import Os
from modules import (
    globals,
    utils,
)

def is_supported():
    return globals.os in (Os.Linux, Os.MacOS)

def steam_roots():
    home = pathlib.Path.home()
    data_home = os.environ.get("XDG_DATA_HOME")
    return (
        home / ".steam/root",
        home / ".steam/steam",
        (pathlib.Path(data_home) if data_home else home / ".local/share") / "Steam",
        home / ".var/app/com.valvesoftware.Steam/data/Steam",
        home / "snap/steam/common/.local/share/Steam",
    )

cache: list[tuple[str, pathlib.Path]] = []


def _libraries(root: pathlib.Path):
    yield root
    libraryfolders = root / "steamapps/libraryfolders.vdf"
    if not libraryfolders.is_file():
        return
    try:
        content = libraryfolders.read_text(errors="ignore")
    except OSError:
        return
    for match in re.finditer(r'"path"\s*"([^"]+)"', content):
        path = pathlib.Path(match.group(1))
        if path.is_dir():
            yield path


def discover():
    found: dict[str, pathlib.Path] = {}
    if system_wine := shutil.which("wine"):
        found["System Wine"] = pathlib.Path(system_wine)
    if system_proton_ge := shutil.which("proton-ge"):
        found["System Proton-GE"] = pathlib.Path(system_proton_ge)
    seen: set[pathlib.Path] = set()
    roots = list(steam_roots())
    roots.extend(pathlib.Path(extra) for extra in globals.settings.wine_extra_runners_dirs.get(globals.os, []))
    for root in roots:
        if not root.is_dir():
            continue
        try:
            root = root.resolve()
        except OSError:
            continue
        if root in seen:
            continue
        seen.add(root)
        candidates = [root / "compatibilitytools.d"]
        candidates += [library / "steamapps/common" for library in _libraries(root)]
        candidates.append(root)
        for parent in candidates:
            if not parent.is_dir():
                continue
            try:
                children = sorted(parent.iterdir())
            except OSError:
                continue
            for child in children:
                if not child.is_dir():
                   continue
                elif (child / "wine").is_file():
                    found[child.name] = child / "wine"
                elif (child / "proton").is_file():
                    found[child.name] = child / "proton"
    return sorted(found.items())


def refresh():
    global cache
    cache = discover()
    return cache


def prefix_root():
    configured = (globals.settings.wine_prefixes_dir.get(globals.os) or "").strip()
    return pathlib.Path(configured) if configured else (globals.data_path / "prefixes")


def prefix_for(name: str):
    return prefix_root() / utils.clean_str(name)


def build_wrapper(runner: pathlib.Path, prefix: pathlib.Path):
    if runner.name == "wine":
        return shlex.join([
            "env",
            f"WINEPREFIX={prefix}",
            str(runner),
            "%command%",
        ])
    elif runner.name in ("proton", "proton-ge"):
        steam_root = pathlib.Path.home() / ".steam/root"
        return shlex.join([
            "env",
            f"WINEPREFIX={prefix}",
            f"STEAM_COMPAT_DATA_PATH={prefix}",
            f"STEAM_COMPAT_CLIENT_INSTALL_PATH={steam_root}",
            str(runner),
            "run",
            "%command%"
        ])
    else:
        return "%command%"


PREFIX_VAR = "WINEPREFIX="


def ensure_prefix(wrapper: str):
    try:
        args = shlex.split(wrapper)
    except ValueError:
        return
    for arg in args:
        if arg.startswith(PREFIX_VAR):
            path = arg[len(PREFIX_VAR):]
            if path:
                try:
                    pathlib.Path(path).mkdir(parents=True, exist_ok=True)
                except OSError:
                    pass


def match_runner(wrapper: str | None):
    if not wrapper:
        return None
    for name, path in cache:
        if str(path) in wrapper:
            return name
    return None
