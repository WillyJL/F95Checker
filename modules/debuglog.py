"""Append-only debug log with metrics, for diagnosing refresh hangs.

Writes F95Checker-debug.log next to the exe (or repo root when running from
source), falls back to the temp folder if that is not writable. Logging must
never break the app, so every failure is swallowed.

Also runs a freeze monitor thread: when progress stalls for >=10s (or the
event loop thread blocks in synchronous code), it dumps every thread's stack
and the innermost await line of every pending task.
"""
import datetime
import pathlib
import sys
import tempfile
import threading
import time
import traceback

from common import meta

path: pathlib.Path | None = None
_header_written = False
_provider = None
_monitor_started = False


def log(msg: str):
    global path, _header_written
    if path is None:
        path = meta.self_path / "F95Checker-debug.log"
    now = datetime.datetime.now()
    header = ""
    if not _header_written:
        _header_written = True
        header = f"===== F95Checker debug log {now:%Y-%m-%d %H:%M:%S} =====\n"
    line = f"{header}[{now:%H:%M:%S.%f}] {msg}\n"
    try:
        with open(path, "a", encoding="utf-8", errors="replace") as f:
            f.write(line)
    except OSError:
        try:
            path = pathlib.Path(tempfile.gettempdir()) / path.name
            with open(path, "a", encoding="utf-8", errors="replace") as f:
                f.write(line)
        except OSError:
            pass


def watch(provider):
    """Start the freeze monitor. provider() -> (progress_label, task_lines) or None when idle."""
    global _provider, _monitor_started
    _provider = provider
    if not _monitor_started:
        _monitor_started = True
        threading.Thread(target=_monitor, daemon=True, name="debuglog-monitor").start()


def unwatch():
    global _provider
    _provider = None


def _monitor():
    last_label = None
    frozen_since = None
    dumped_at = 0.0
    while True:
        time.sleep(2)
        try:
            provider = _provider
            if provider is None:
                last_label = frozen_since = None
                continue
            snap = provider()
            if snap is None:
                last_label = frozen_since = None
                continue
            label, task_lines = snap
            if label != last_label:
                last_label = label
                frozen_since = time.monotonic()
                continue
            if frozen_since is None:
                frozen_since = time.monotonic()
            if time.monotonic() - frozen_since >= 10 and time.monotonic() - dumped_at >= 10:
                dumped_at = time.monotonic()
                log(f"FROZEN >=10s at progress={label}, dumping stacks "
                    f"(threads={threading.active_count()}, tasks={len(task_lines)})")
                threads = {t.ident: t.name for t in threading.enumerate()}
                for tid, frame in sorted(sys._current_frames().items()):
                    log(f"thread [{threads.get(tid, tid)}]:\n"
                        + "".join(traceback.format_stack(frame)))
                for line in task_lines:
                    log(f"task {line}")
        except Exception:
            pass
