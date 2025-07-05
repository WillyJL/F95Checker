import asyncio
import dataclasses
import datetime as dt
import enum
import functools
import hashlib
import json
import os
import pathlib
import shutil
import sys
import time
import typing
import weakref

from external import weakerset


class CounterContext:
    __slots__ = ("count",)

    def __init__(self):
        self.count = 0

    def __enter__(self):
        self.count += 1

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.count -= 1

    async def __aenter__(self):
        self.__enter__()

    async def __aexit__(self, exc_type, exc_val, exc_tb):
        self.__exit__(exc_type, exc_val, exc_tb)


class Popup(functools.partial):
    next_uuid = 0

    __slots__ = ("open", "uuid",)

    def __init__(self, *_, **__):
        from modules import utils
        self.open = True
        cls = type(self)
        uuid = cls.next_uuid
        cls.next_uuid += 1
        self.uuid = f"{uuid}_{str(time.time()).split('.')[-1]}_{utils.rand_num_str()}"

    def __call__(self, *args, **kwargs):
        if not self.open:
            return 0, True
        opened, closed = super().__call__(*args, popup_uuid=self.uuid, **kwargs)
        if closed:
            self.open = False
        return opened, closed


class DaemonProcess:
    __slots__ = ("finalize",)

    def __init__(self, proc):
        self.finalize = weakref.finalize(proc, self.kill, proc)

    @staticmethod
    def kill(proc):
        try:
            # Multiprocessing
            if getattr(proc, "exitcode", False) is None:
                proc.kill()
            # Asyncio subprocess
            elif getattr(proc, "returncode", False) is None:
                proc.kill()
            # Standard subprocess
            elif getattr(proc, "poll", lambda: False)() is None:
                proc.kill()
        except Exception:
            pass

    def __enter__(self):
        return self

    def __exit__(self, *_):
        self.finalize()


class AbstractPipe:
    def __init__(self):
        raise NotImplementedError()

    async def get_async(self):
        raise NotImplementedError()

    def put(self, data: dict | list | str):
        raise NotImplementedError()


class DaemonPipe(AbstractPipe):
    __slots__ = ("proc", "daemon",)

    class DaemonPipeExit(Exception):
        __slots__ = ()

    def __init__(self, proc: asyncio.subprocess.Process):
        self.proc = proc
        self.daemon = DaemonProcess(proc)

    async def is_alive(self):
        await asyncio.sleep(0)
        return self.proc.returncode is None

    async def get_async(self):
        assert self.proc.stdout
        while await self.is_alive() and not self.proc.stdout.at_eof():
            line = await self.proc.stdout.readline()
            try:
                return json.loads(line)
            except json.JSONDecodeError:
                pass
        raise self.DaemonPipeExit()

    def put(self, data: dict | list | str):
        assert self.proc.stdin
        if self.proc.returncode is None:
            self.proc.stdin.write(json.dumps(data).encode() + b"\n")
            return
        raise self.DaemonPipeExit()

    def __enter__(self):
        self.daemon.__enter__()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.daemon.__exit__()
        if exc_type is self.DaemonPipeExit:
            return True

    def kill(self):
        self.daemon.__exit__()


class ChildPipe(AbstractPipe):
    __slots__ = ("loop", "stdin_event")

    def __init__(self):
        try:
            self.loop = asyncio.get_running_loop()
        except RuntimeError:
            self.loop = None

        if self.loop and sys.stdin:
            self.stdin_event = asyncio.Event()
            try:
                self.loop.add_reader(sys.stdin, self.stdin_event.set)
            except NotImplementedError:
                self.stdin_event = None

    async def get(self):
        assert sys.stdin
        while True:
            line = sys.stdin.readline()
            try:
                return json.loads(line)
            except json.JSONDecodeError:
                pass

    async def get_async(self):
        assert sys.stdin
        assert self.loop
        while True:
            if self.stdin_event:
                await self.stdin_event.wait()
                self.stdin_event.clear()
            line = await self.loop.run_in_executor(None, sys.stdin.readline)
            try:
                return json.loads(line)
            except json.JSONDecodeError:
                pass
            await asyncio.sleep(0)

    def put(self, data: dict | list | str):
        assert sys.stdout
        print(json.dumps(data), flush=True)


class Timestamp:
    instances = weakerset.WeakerSet()

    __slots__ = ("value", "_display", "__weakref__",)

    def __init__(self, unix_time: int | float):
        self.update(unix_time)
        type(self).instances.add(self)

    def update(self, unix_time: int | float = None):
        if unix_time is not None:
            self.value = int(unix_time)
        self._display = None

    @property
    def format(self):
        from modules import globals
        return globals.settings.timestamp_format

    @property
    def display(self):
        if self._display is None:
            if self.value == 0:
                self._display = ""
            else:
                try:
                    self._display = dt.datetime.fromtimestamp(self.value).strftime(self.format)
                except Exception:
                    self._display = "Bad format!"
        return self._display


class Datestamp(Timestamp):
    instances = weakerset.WeakerSet()

    __slots__ = ()

    def __init__(self, unix_time: int | float):
        self.update(unix_time)
        type(self).instances.add(self)

    @property
    def format(self):
        from modules import globals
        return globals.settings.datestamp_format


@dataclasses.dataclass(slots=True)
class ThreadMatch:
    title: str
    id: int


@dataclasses.dataclass(slots=True)
class SearchResult:
    title: str
    creator: str
    url: str
    id: int


@dataclasses.dataclass(slots=True)
class TorrentResult:
    id: int
    title: str
    size: int | str
    seeders: int | str
    leechers:int | str
    date: int | str

    def __post_init__(self):
        from modules import (
            globals,
            utils,
        )
        self.size = utils.sizeof_fmt(self.size)
        self.seeders = str(self.seeders)
        self.leechers = str(self.leechers)
        self.date = dt.datetime.fromtimestamp(self.date).strftime(globals.settings.datestamp_format)


@dataclasses.dataclass(slots=True)
class DdlFile:
    thread_id: int
    id: str
    title: str
    filename: str
    size: int
    date: str
    sha1: str
    size_display: str = None

    def __post_init__(self):
        if not self.id:
            return
        from modules import (
            globals,
            utils,
        )
        self.size_display = utils.sizeof_fmt(int(self.size))
        self.date = dt.datetime.strptime(self.date, r"%Y-%m-%d").strftime(globals.settings.datestamp_format)


@dataclasses.dataclass(slots=True)
class FileDownload:
    url: str = ""
    cookies: dict = True
    checksum: tuple[str, str] = None
    path: pathlib.Path = None
    progress: int = 0
    total: int = None

    class State(enum.IntEnum):
        Preparing = enum.auto()
        Downloading = enum.auto()
        Verifying = enum.auto()
        Extracting = enum.auto()
        Stopped = enum.auto()
        Deleting = enum.auto()
        Removed = enum.auto()

    cancel: bool = False
    state: State = State.Preparing
    extracted: pathlib.Path = None
    error: str = None
    traceback: str = None

    async def delete(self):
        assert self.state == self.State.Stopped
        self.state = self.State.Deleting
        try:
            loop = asyncio.get_event_loop()
            if self.extracted:
                await loop.run_in_executor(None, functools.partial(shutil.rmtree, self.extracted, ignore_errors=True))
            await loop.run_in_executor(None, functools.partial(self.path.unlink, missing_ok=True))
        except Exception:
            pass
        self.state = self.State.Removed


@dataclasses.dataclass(slots=True)
class SortSpec:
    index: int
    reverse: bool


class IntEnumHack(enum.IntEnum):
    def __new__(cls, value, attrs: dict = None):
        self = int.__new__(cls, value)
        self._value_ = value
        # Add additional attributes
        if isinstance(attrs, dict):
            for key, value in attrs.items():
                setattr(self, key, value)
        return self
    def __init__(self, *args, **kwargs):
        cls = type(self)
        # Add index for use with _member_names_
        self._index_ = len(cls._member_names_)  # self is added later, so the length is up to the previous item, so not len() - 1
        # Replace spaces with _, - with __ and add _ in front if starting with a number. Allows using Enum._1_special__name in code for "1 special-name"
        new_name = "_" * self._name_[0].isdigit() + self._name_.replace(" ", "_").replace("-", "__")
        if new_name != self._name_:
            setattr(cls, new_name, self)


Status = IntEnumHack("Status", [
    ("Normal",    1),
    ("Completed", 2),
    ("OnHold",    3),
    ("Abandoned", 4),
    ("Unchecked", 5),
    ("Custom",    6),
])


Tag = IntEnumHack("Tag", [
    ("2d-game",                1),
    ("2dcg",                   2),
    ("3d-game",                3),
    ("3dcg",                   4),
    ("adventure",              5),
    ("ahegao",                 6),
    ("ai-cg",                  140),
    ("anal-sex",               7),
    ("animated",               8),
    ("asset-addon",            9),
    ("asset-ai-shoujo",        10),
    ("asset-animal",           11),
    ("asset-animation",        12),
    ("asset-audio",            13),
    ("asset-bundle",           14),
    ("asset-character",        15),
    ("asset-clothing",         16),
    ("asset-daz-gen1",         151),
    ("asset-daz-gen2",         141),
    ("asset-daz-gen3",         142),
    ("asset-daz-gen8",         143),
    ("asset-daz-gen81",        144),
    ("asset-daz-gen9",         145),
    ("asset-daz-m4",           152),
    ("asset-daz-v4",           153),
    ("asset-environment",      17),
    ("asset-expression",       18),
    ("asset-female",           146),
    ("asset-hair",             19),
    ("asset-hdri",             20),
    ("asset-honey-select",     21),
    ("asset-honey-select2",    22),
    ("asset-koikatu",          23),
    ("asset-light",            24),
    ("asset-male",             147),
    ("asset-morph",            25),
    ("asset-nonbinary",        148),
    ("asset-playhome",         150),
    ("asset-plugin",           26),
    ("asset-pose",             27),
    ("asset-prop",             28),
    ("asset-scene",            149),
    ("asset-script",           29),
    ("asset-shader",           30),
    ("asset-texture",          31),
    ("asset-utility",          32),
    ("asset-vehicle",          33),
    ("bdsm",                   34),
    ("bestiality",             35),
    ("big-ass",                36),
    ("big-tits",               37),
    ("blackmail",              38),
    ("bukkake",                39),
    ("censored",               40),
    ("character-creation",     41),
    ("cheating",               42),
    ("combat",                 43),
    ("corruption",             44),
    ("cosplay",                45),
    ("creampie",               46),
    ("dating-sim",             47),
    ("dilf",                   48),
    ("drugs",                  49),
    ("dystopian-setting",      50),
    ("exhibitionism",          51),
    ("fantasy",                52),
    ("female-protagonist",     54),
    ("femaledomination",       53),
    ("footjob",                55),
    ("furry",                  56),
    ("futa-trans",             57),
    ("futa-trans-protagonist", 58),
    ("gay",                    59),
    ("graphic-violence",       60),
    ("groping",                61),
    ("group-sex",              62),
    ("handjob",                63),
    ("harem",                  64),
    ("horror",                 65),
    ("humiliation",            66),
    ("humor",                  67),
    ("incest",                 68),
    ("internal-view",          69),
    ("interracial",            70),
    ("japanese-game",          71),
    ("kinetic-novel",          72),
    ("lactation",              73),
    ("lesbian",                74),
    ("loli",                   75),
    ("male-protagonist",       77),
    ("maledomination",         76),
    ("management",             78),
    ("masturbation",           79),
    ("milf",                   80),
    ("mind-control",           81),
    ("mobile-game",            82),
    ("monster",                83),
    ("monster-girl",           84),
    ("multiple-endings",       85),
    ("multiple-penetration",   86),
    ("multiple-protagonist",   87),
    ("necrophilia",            88),
    ("no-sexual-content",      89),
    ("ntr",                    90),
    ("oral-sex",               91),
    ("paranormal",             92),
    ("parody",                 93),
    ("platformer",             94),
    ("point-click",            95),
    ("possession",             96),
    ("pov",                    97),
    ("pregnancy",              98),
    ("prostitution",           99),
    ("puzzle",                 100),
    ("rape",                   101),
    ("real-porn",              102),
    ("religion",               103),
    ("romance",                104),
    ("rpg",                    105),
    ("sandbox",                106),
    ("scat",                   107),
    ("school-setting",         108),
    ("sci-fi",                 109),
    ("sex-toys",               110),
    ("sexual-harassment",      111),
    ("shooter",                112),
    ("shota",                  113),
    ("side-scroller",          114),
    ("simulator",              115),
    ("sissification",          116),
    ("slave",                  117),
    ("sleep-sex",              118),
    ("spanking",               119),
    ("strategy",               120),
    ("stripping",              121),
    ("superpowers",            122),
    ("swinging",               123),
    ("teasing",                124),
    ("tentacles",              125),
    ("text-based",             126),
    ("titfuck",                127),
    ("trainer",                128),
    ("transformation",         129),
    ("trap",                   130),
    ("turn-based-combat",      131),
    ("twins",                  132),
    ("urination",              133),
    ("vaginal-sex",            134),
    ("virgin",                 135),
    ("virtual-reality",        136),
    ("voiced",                 137),
    ("vore",                   138),
    ("voyeurism",              139),
])


ExeState = IntEnumHack("ExeState", [
    ("Invalid",  1),
    ("Selected", 2),
    ("Unset",    3),
])


MsgBox = IntEnumHack("MsgBox", [
    ("info",  (1, {"color": (0.10, 0.69, 0.95), "icon": "information"})),
    ("warn",  (2, {"color": (0.95, 0.69, 0.10), "icon": "alert_rhombus"})),
    ("error", (3, {"color": (0.95, 0.22, 0.22), "icon": "alert_octagon"})),
])


FilterMode = IntEnumHack("FilterMode", [
    ("Choose",    1),
    ("Archived",  2),
    ("Custom",    13),
    ("Exe State", 3),
    ("Finished",  6),
    ("Installed", 4),
    ("Label",     5),
    ("Rating",    7),
    ("Score",     8),
    ("Status",    9),
    ("Tag",       10),
    ("Type",      11),
    ("Updated",   12),
])


@dataclasses.dataclass(slots=True)
class Filter:
    mode: FilterMode
    invert: bool = False
    match: typing.Any = None
    id: int = None

    def __post_init__(self):
        self.id = id(self)


@dataclasses.dataclass(slots=True)
class Label:
    @property
    def short_name(self):
        return "".join(word[:1] for word in self.name.split(" "))


@dataclasses.dataclass(slots=True)
class Tab:
    @classmethod
    def first_tab_label(cls):
        from modules import globals, icons
        if globals.settings.default_tab_is_new:
            return f"{icons.alert_decagram} New"
        else:
            return f"{icons.heart_box} Default"


Type = IntEnumHack("Type", [
    ("ADRIFT",     2),
    ("Flash",      4),
    ("HTML",       5),
    ("Java",       6),
    ("Others",     9),
    ("QSP",        10),
    ("RAGS",       11),
    ("RenPy",      14),
    ("RPGM",       13),
    ("Tads",       16),
    ("Unity",      19),
    ("Unreal Eng", 20),
    ("WebGL",      21),
    ("Wolf RPG",   22),
    ("CG",         30),
    ("Collection", 7),
    ("Comics",     24),
    ("GIF",        25),
    ("Manga",      26),
    ("Pinup",      27),
    ("SiteRip",    28),
    ("Video",      29),
    ("Cheat Mod",  3),
    ("Mod",        8),
    ("READ ME",    12),
    ("Request",    15),
    ("Tool",       17),
    ("Tutorial",   18),
    ("Misc",       1),
    ("Unchecked",  23),
])


@dataclasses.dataclass(slots=True)
class Game:
    _did_init          : bool = False

    def __post_init__(self):
        self._did_init = True
        from external import imagehelper
        from modules import globals
        self.image = imagehelper.ImageHelper(globals.images_path, glob=f"{self.id}.*")
        self.validate_executables()

    def delete_images(self):
        from modules import globals
        for img in globals.images_path.glob(f"{self.id}.*"):
            try:
                img.unlink()
            except Exception:
                pass

    def refresh_image(self):
        self.image.glob = f"{self.id}.*"
        self.image.reload()

    async def set_image_async(self, data: bytes):
        from modules import globals, utils
        import aiofiles
        self.delete_images()
        if data:
            async with aiofiles.open(globals.images_path / f"{self.id}.{utils.image_ext(data)}", "wb") as f:
                await f.write(data)
        self.refresh_image()

    def set_image_sync(self, data: bytes):
        from modules import globals, utils
        self.delete_images()
        if data:
            (globals.images_path / f"{self.id}.{utils.image_ext(data)}").write_bytes(data)
        self.refresh_image()

    def validate_executables(self):
        from modules import globals, utils
        if globals.settings.default_exe_dir.get(globals.os):
            changed = False
            executables_valids = []
            base = pathlib.Path(globals.settings.default_exe_dir.get(globals.os))
            for i, executable in enumerate(self.executables):
                if utils.is_uri(executable):
                    executables_valids.append(True)
                    continue
                exe = pathlib.Path(executable)
                if exe.is_absolute():
                    if base in exe.parents:
                        self.executables[i] = exe.relative_to(base).as_posix()
                        changed = True
                    executables_valids.append(exe.is_file())
                else:
                    executables_valids.append((base / exe).is_file())
            self.executables_valids = executables_valids
            if changed:
                from external import async_thread
                from modules import db
                async_thread.run(db.update_game(self, "executables"))
        else:
            self.executables_valids = [utils.is_uri(executable) or os.path.isfile(executable) for executable in self.executables]
        self.executables_valid = all(self.executables_valids)
        if globals.gui:
            globals.gui.recalculate_ids = True

    def add_executable(self, executable: str):
        from modules import globals, utils
        if not utils.is_uri(executable):
            exe = pathlib.Path(executable)
            if globals.settings.default_exe_dir.get(globals.os):
                base = pathlib.Path(globals.settings.default_exe_dir.get(globals.os))
                if base in exe.parents:
                    exe = exe.relative_to(base)
            executable = exe.as_posix()
        if executable in self.executables:
            return
        self.executables.append(executable)
        from external import async_thread
        from modules import db
        async_thread.run(db.update_game(self, "executables"))
        self.validate_executables()
        if self.installed != self.version:
            self.add_timeline_event(TimelineEventType.GameInstalled, self.version)
            self.installed = self.version
            self.updated = False

    def remove_executable(self, executable: str):
        self.executables.remove(executable)
        from external import async_thread
        from modules import db
        async_thread.run(db.update_game(self, "executables"))
        self.validate_executables()

    def clear_executables(self):
        self.executables.clear()
        from external import async_thread
        from modules import db
        async_thread.run(db.update_game(self, "executables"))
        self.validate_executables()

    def add_label(self, label: Label):
        if label not in self.labels:
            self.labels.append(label)
        self.labels.sort(key=lambda label: Label.instances.index(label))
        from external import async_thread
        from modules import db, globals
        async_thread.run(db.update_game(self, "labels"))
        if globals.gui:
            globals.gui.recalculate_ids = True

    def remove_label(self, label: Label):
        while label in self.labels:
            self.labels.remove(label)
        from external import async_thread
        from modules import db, globals
        async_thread.run(db.update_game(self, "labels"))
        if globals.gui:
            globals.gui.recalculate_ids = True


    def __setattr__(self, name: str, value: typing.Any):
        if hasattr(self, "_did_init") and self._did_init and name in [
            "custom",
            "name",
            "version",
            "developer",
            "type",
            "status",
            "url",
            "added_on",
            "last_updated",
            "last_full_check",
            "last_check_version",
            "last_launched",
            "score",
            "votes",
            "rating",
            "finished",
            "installed",
            "updated",
            "archived",
            "executables",
            "description",
            "changelog",
            "tags",
            "unknown_tags",
            "unknown_tags_flag",
            "labels",
            "tab",
            "notes",
            "image_url",
            "previews_urls",
            "downloads",
            "reviews_total",
            "reviews",
        ]:
            if isinstance(attr := getattr(self, name), Timestamp):
                attr.update(value)
            else:
                super(Game, self).__setattr__(name, value)
            from external import async_thread
            from modules import db, globals
            async_thread.run(db.update_game(self, name))
            if globals.gui:
                globals.gui.recalculate_ids = True
            return
        super(Game, self).__setattr__(name, value)
        if name == "selected":
            from modules import globals
            if globals.gui:
                if value:
                    globals.gui.last_selected_game = self
                globals.gui.selected_games_count = len(list(filter(lambda game: game.selected, globals.games.values())))


@dataclasses.dataclass(slots=True)
class OldGame:
    id                   : int
    name                 : str
    version              : str
    status               : Status
