"""Self-check for api.request() retry fixes (see plan.md).

Run: .venv/Scripts/python tests/test_api_request.py
"""
import asyncio
import pathlib
import sys
import types

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent.parent))

import modules.globals  # noqa: F401  must come before modules.api (circular imports)
import aiohttp
from modules import api


class BoomCtx:
    def __init__(self, exc):
        self.exc = exc

    async def __aenter__(self):
        raise self.exc

    async def __aexit__(self, *args):
        return False


class BoomSession:
    def __init__(self, exc):
        self.exc = exc
        self.calls = 0

    def request(self, *args, **kwargs):
        self.calls += 1
        return BoomCtx(self.exc)


class OkCtx:
    status = 200

    async def __aenter__(self):
        return self

    async def __aexit__(self, *args):
        return False

    async def read(self):
        return b"ok"


class OkSession:
    def __init__(self):
        self.calls = 0

    def request(self, *args, **kwargs):
        self.calls += 1
        return OkCtx()


url = "https://example.com/x"
real_sleep = asyncio.sleep
sleeps: list[float] = []


async def fast_sleep(delay, *args, **kwargs):
    sleeps.append(delay)
    await real_sleep(0)


async def run():
    g = api.globals
    g.settings = types.SimpleNamespace(
        max_retries=2,
        request_timeout=5,
        ignore_semaphore_timeouts=True,
        insecure_ssl=False,
    )
    asyncio.sleep = fast_sleep
    try:
        # 1. Persistent errno-121 with ignore_semaphore_timeouts: must hit the soft
        #    budget and raise instead of spinning forever (the original hang)
        api.session = BoomSession(TimeoutError(121, "semaphore timeout period has expired"))
        sleeps.clear()
        try:
            async with api.request("GET", url, cookies=False) as (res, req):
                raise AssertionError("request should not succeed")
        except TimeoutError:
            pass
        assert api.session.calls == 10, f"soft budget broken: {api.session.calls} attempts"
        assert sleeps == [1] * 9, sleeps

        # 2. Normal client errors: bounded retries with backoff sleeps
        g.settings.ignore_semaphore_timeouts = False
        api.session = BoomSession(aiohttp.ClientError("nope"))
        sleeps.clear()
        try:
            async with api.request("GET", url, cookies=False) as (res, req):
                raise AssertionError("request should not succeed")
        except aiohttp.ClientError:
            pass
        assert api.session.calls == 3, f"max_retries broken: {api.session.calls} attempts"
        assert sleeps == [0.5, 0.5], sleeps

        # 2b. Per-call retries override (image downloads: retries=1, no backoff wait)
        api.session = BoomSession(aiohttp.ClientError("nope"))
        sleeps.clear()
        try:
            async with api.request("GET", url, cookies=False, retries=1) as (res, req):
                raise AssertionError("request should not succeed")
        except aiohttp.ClientError:
            pass
        assert api.session.calls == 1, f"retries override broken: {api.session.calls} attempts"
        assert sleeps == [], sleeps

        # 3. Client error raised by the consumer body after yield must propagate
        #    as-is, no second yield inside athrow (RuntimeError), no retry
        api.session = OkSession()
        try:
            async with api.request("GET", url, cookies=False) as (res, req):
                assert res == b"ok"
                raise aiohttp.ClientError("body boom")
        except RuntimeError as exc:
            raise AssertionError(f"athrow bug: {exc}") from exc
        except aiohttp.ClientError as exc:
            assert "body boom" in str(exc)
        assert api.session.calls == 1, f"body exception retried: {api.session.calls} attempts"

        # 4. check_notifs early return (no login) still counts progress to 100%
        async def fake_fetch(*args, **kwargs):
            return b'{"foo": 1}'

        async def fake_assert_login():
            return False

        real_fetch, real_assert_login = api.fetch, api.assert_login
        api.fetch, api.assert_login = fake_fetch, fake_assert_login
        try:
            await api.check_notifs(standalone=True)
        finally:
            api.fetch, api.assert_login = real_fetch, real_assert_login
        assert g.refresh_progress == g.refresh_total == 1, (g.refresh_progress, g.refresh_total)
    finally:
        asyncio.sleep = real_sleep


if __name__ == "__main__":
    asyncio.run(run())
    print("all request() retry checks passed")
