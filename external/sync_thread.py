# https://gist.github.com/WillyJL/bb410bcc761f8bf5649180f22b7f3b44
import threading
import typing

stack: list = []
threads: list[threading.Thread] = []
_lock: threading.Lock = None
_condition: threading.Condition = None


def setup(count: int):
    global _lock, _condition

    _lock = threading.Lock()
    _condition = threading.Condition()

    def run_loop():
        while True:
            while stack:
                try:
                    with _lock:
                        fn = stack.pop()
                except IndexError:
                    continue
                fn()
            with _condition:
                _condition.wait()

    for _ in range(count):
        thread = threading.Thread(target=run_loop, daemon=True)
        thread.start()
        threads.append(thread)


def queue(fn: typing.Callable):
    with _lock:
        stack.append(fn)
    with _condition:
        _condition.notify()


def unqueue(fn: typing.Callable):
    try:
        with _lock:
            stack.remove(fn)
    except ValueError:
        pass


# Example usage
if __name__ == "__main__":
    import sync_thread  # This script is designed as a module you import
    sync_thread.setup()

    def say_hello():
        print("Hello world!")

    for _ in range(10):
        sync_thread.queue(say_hello)
