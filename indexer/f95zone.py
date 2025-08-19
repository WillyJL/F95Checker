import asyncio
import contextlib
import dataclasses
import datetime as dt
import logging
import os
import re
import sys

import aiohttp
import aiolimiter

from common import meta

RATELIMIT = aiolimiter.AsyncLimiter(max_rate=1, time_period=0.5)
TIMEOUT = aiohttp.ClientTimeout(total=30, connect=30, sock_read=30, sock_connect=30)
LOGIN_ERROR_MESSAGES = (
    b'<a href="/login/" data-xf-click="overlay">Log in or register now.</a>',
    b"<title>Log in | F95zone</title>",
    b'<form action="/login/login" method="post" class="block"',
)
RATELIMIT_FORUM_ERRORS = (
    b"<title>429 Too Many Requests</title>",
    b"<h1>429 Too Many Requests</h1>",
    b"<title>Error 429</title>",
    b"<title>DDoS-Guard</title>",
    b"<title>DDOS-GUARD</title>",
)
RATELIMIT_API_ERRORS = (
    "You have been temporarily blocked because of a large amount of requests, please try again later",
)
TEMP_ERROR_MESSAGES = (
    b"<title>502 Bad Gateway</title>",
    b"<title>Error 502</title>",
    b"An unexpected error occurred. Please try again later.",
    b"An unexpected database error occurred. Please try again later.\n<!--",
    b"<!-- Connection refused -->",
    b"<!-- Too many connections -->",
    b"<p>Automated backups are currently executing. During this time, the site will be unavailable</p>",
    b"<title>F95Zone :: Scheduled Maintenance</title>",
    b'<script src="https://static.f95zone.to/assets/SamF95/ErrorPage',
    b'<div class="blockMessage"><p>Please check back in 10 mins</p></div>',
)

logger = logging.getLogger(__name__)
session: aiohttp.ClientSession = None
cookies: dict = None

HOST = "https://f95zone.to"
THREAD_URL = f"{HOST}/threads/{{thread}}"
BULK_VERSION_CHECK_URL = f"{HOST}/sam/checker.php?threads={{threads}}"
LATEST_UPDATES_SEARCH_URL = f"{HOST}/sam/latest_alpha/latest_data.php?cmd={{cmd}}&cat={{cat}}&page={{page}}&{{search}}={{query}}&sort={{sort}}&rows={{rows}}&_={{ts}}"
LATEST_UPDATES_URL = f"{HOST}/sam/latest_alpha/latest_data.php?cmd={{cmd}}&cat={{cat}}&page={{page}}&sort={{sort}}&rows={{rows}}&_={{ts}}"
LATEST_UPDATES_CATEGORIES = (
    "games",
    "comics",
    "animations",
    "assets",
    # Doesn't seem to work
    # "mods",
)


@dataclasses.dataclass(slots=True)
class IndexerError:
    error_flag: str
    retry_delay: int


ERROR_SESSION_LOGGED_OUT = IndexerError(
    "SESSION_LOGGED_OUT", dt.timedelta(hours=2).total_seconds()
)
ERROR_F95ZONE_RATELIMIT = IndexerError(
    "F95ZONE_RATELIMIT", dt.timedelta(minutes=15).total_seconds()
)
ERROR_F95ZONE_UNAVAILABLE = IndexerError(
    "F95ZONE_UNAVAILABLE", dt.timedelta(minutes=15).total_seconds()
)
ERROR_THREAD_MISSING = IndexerError(
    "THREAD_MISSING", dt.timedelta(days=14).total_seconds()
)
ERROR_PARSING_FAILED = IndexerError(
    "PARSING_FAILED", dt.timedelta(hours=6).total_seconds()
)
ERROR_UNKNOWN_RESPONSE = IndexerError(
    "UNKNOWN_RESPONSE", dt.timedelta(hours=6).total_seconds()
)
ERROR_INTERNAL_ERROR = IndexerError(
    "INTERNAL_ERROR", dt.timedelta(hours=6).total_seconds()
)


@contextlib.asynccontextmanager
async def lifespan():
    global session, cookies
    session = aiohttp.ClientSession(
        cookie_jar=aiohttp.DummyCookieJar(),
        timeout=TIMEOUT,
        headers={
            "User-Agent": (
                f"F95Indexer/{meta.version} "
                f"Python/{sys.version.split(' ')[0]} "
                f"aiohttp/{aiohttp.__version__}"
            ),
        },
    )
    cookies = {
        "xf_user": os.environ.get("COOKIE_XF_USER"),
    }

    try:
        yield
    finally:

        await session.close()
        session = None


def check_error(
    res: bytes | dict | Exception, logger: logging.Logger
) -> IndexerError | None:
    if isinstance(res, bytes):
        if any((msg in res) for msg in LOGIN_ERROR_MESSAGES):
            logger.error("Logged out of F95zone")
            # TODO: maybe auto login, but xf_user cookie should be enough for a long time
            return ERROR_SESSION_LOGGED_OUT

        if any((msg in res) for msg in RATELIMIT_FORUM_ERRORS):
            logger.error("Hit F95zone Forum ratelimit")
            return ERROR_F95ZONE_RATELIMIT

        if any((msg in res) for msg in TEMP_ERROR_MESSAGES):
            logger.warning("F95zone temporarily unreachable")
            return ERROR_F95ZONE_UNAVAILABLE

    elif isinstance(res, dict):
        if res.get("status") == "error":

            if any((msg == res.get("msg")) for msg in RATELIMIT_API_ERRORS):
                logger.error("Hit F95zone API ratelimit")
                return ERROR_F95ZONE_RATELIMIT

            logger.error(f"F95zone API returned an error: {res}")
            return ERROR_UNKNOWN_RESPONSE

    elif isinstance(res, Exception):
        if isinstance(res, (asyncio.TimeoutError, aiohttp.ClientConnectionError)):
            logger.warning("F95zone temporarily unreachable")
            return ERROR_F95ZONE_UNAVAILABLE


def latest_updates_search_sanitize_query(query: str):
    redis_stopwords = (
        "a",
        "is",
        "the",
        "an",
        "and",
        "are",
        "as",
        "at",
        "be",
        "but",
        "by",
        "for",
        "if",
        "in",
        "into",
        "it",
        "no",
        "not",
        "of",
        "on",
        "or",
        "such",
        "that",
        "their",
        "then",
        "there",
        "these",
        "they",
        "this",
        "to",
        "was",
        "will",
        "with",
    )
    query = re.sub(r"’s |'s ", " ", query)
    query = query.encode("ascii", errors="replace").decode()
    query = re.sub(r"\.+ | \.+", " ", query)
    for char in "?&/':;-.+!~(),*":
        query = query.replace(char, " ")
    query = re.sub(r"\s+", " ", query).strip()
    words = query.split(" ")
    for stopword in redis_stopwords:
        for word in words.copy():
            if word.lower() == stopword:
                words.remove(word)
    query = ""
    while words:
        append = f"{' ' if query else ''}{words.pop(0)}"
        if len(query + append) > 30:
            append = append[: 30 - len(query)]
            if len(append) > 3 and append.strip().lower() not in redis_stopwords:
                query += append
            break
        query += append
    return query
