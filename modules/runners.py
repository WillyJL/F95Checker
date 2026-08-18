import os
import pathlib
import re
import shlex

from modules import globals

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
    seen: set[pathlib.Path] = set()
    extra = (globals.settings.extra_runners_dir or "").strip()
    roots = list(steam_roots())
    if extra:
        roots.append(pathlib.Path(extra))
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
                if child.is_dir() and (child / "proton").is_file():
                    found[child.name] = child / "proton"
    return sorted(found.items())


def refresh():
    global cache
    cache = discover()
    return cache


def prefix_root():
    configured = (globals.settings.runner_prefix_dir or "").strip()
    return pathlib.Path(configured) if configured else globals.data_path / "prefixes"


def prefix_for(name: str):
    safe = re.sub(r"[^A-Za-z0-9._-]+", "_", name).strip("_") or "default"
    return prefix_root() / safe


def build_wrapper(runner: pathlib.Path, prefix: pathlib.Path):
    steam_root = pathlib.Path.home() / ".steam/root"
    return (
        f"env STEAM_COMPAT_DATA_PATH={shlex.quote(str(prefix))}"
        f" STEAM_COMPAT_CLIENT_INSTALL_PATH={shlex.quote(str(steam_root))}"
        f" {shlex.quote(str(runner))} run %command%"
    )


PREFIX_VAR = "STEAM_COMPAT_DATA_PATH="


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


def match_runner(wrapper: str):
    if not wrapper:
        return None
    for name, path in cache:
        if str(path) in wrapper:
            return name
    return None
