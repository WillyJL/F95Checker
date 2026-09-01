# https://gist.github.com/WillyJL/bb410bcc761f8bf5649180f22b7f3b44
import os
import threading
import typing

stack: list = None
thread: threading.Thread = None
threads: list[threading.Thread] = []
_condition: threading.Condition = None


def setup(worker_count: int = None):
    global stack, thread, threads, _condition

    stack = []
    _condition = threading.Condition()
    if worker_count is None:
        # Keep one core available for the UI and cap concurrency to avoid
        # multiplying memory use when many animated previews are decoded.
        worker_count = max(1, min(4, (os.cpu_count() or 2) - 1))
    worker_count = max(1, int(worker_count))

    def run_loop():
        while True:
            with _condition:
                while not stack:
                    _condition.wait()
                fn = stack.pop(0)
            fn()

    threads = [threading.Thread(target=run_loop, daemon=True, name=f"ImageWorker-{i + 1}") for i in range(worker_count)]
    thread = threads[0]
    for worker in threads:
        worker.start()


def queue(fn: typing.Callable):
    with _condition:
        stack.append(fn)
        _condition.notify()


def queue_front(fn: typing.Callable):
    """Queue work ahead of normal image loads."""
    with _condition:
        stack.insert(0, fn)
        _condition.notify()


def promote(fn: typing.Callable):
    """Move queued work to the front without duplicating active work."""
    with _condition:
        try:
            stack.remove(fn)
        except (ValueError, AttributeError):
            return False
        stack.insert(0, fn)
        _condition.notify()
        return True


def unqueue(fn: typing.Callable):
    with _condition:
        try:
            stack.remove(fn)
        except (ValueError, AttributeError):
            pass


# Example usage
if __name__ == "__main__":
    import sync_thread  # This script is designed as a module you import
    sync_thread.setup()

    def say_hello():
        print("Hello world!")

    for _ in range(10):
        sync_thread.queue(say_hello)
