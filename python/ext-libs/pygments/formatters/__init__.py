# -*- coding: utf-8 -*-
"""
    pygments.formatters
    ~~~~~~~~~~~~~~~~~~~

    Pygments formatters.

    :copyright: Copyright 2006-2013 by the Pygments team, see AUTHORS.
    :license: BSD, see LICENSE for details.
"""
import os.path
import fnmatch

from pygments.formatters._mapping import FORMATTERS
from pygments.plugin import find_plugin_formatters
from pygments.util import ClassNotFound

ns = globals()
for fcls in FORMATTERS:
    ns[fcls.__name__] = fcls
del fcls

__all__ = ['get_formatter_by_name', 'get_formatter_for_filename',
           'get_all_formatters'] + [cls.__name__ for cls in FORMATTERS]


_formatter_alias_cache = {}
_formatter_filename_cache = []

def _init_formatter_cache():
    if _formatter_alias_cache:
        return
    for cls in get_all_formatters():
        for alias in cls.aliases:
            _formatter_alias_cache[alias] = cls
        for fn in cls.filenames:
            _formatter_filename_cache.append((fn, cls))


def find_formatter_class(name):
    _init_formatter_cache()
    cls = _formatter_alias_cache.get(name, None)
    return cls


def get_formatter_by_name(name, **options):
    _init_formatter_cache()
    cls = _formatter_alias_cache.get(name, None)
    if not cls:
        raise ClassNotFound("No formatter found for name %r" % name)
    return cls(**options)


def get_formatter_for_filename(fn, **options):
    _init_formatter_cache()
    fn = os.path.basename(fn)
    for pattern, cls in _formatter_filename_cache:
        if fnmatch.fnmatch(fn, pattern):
            return cls(**options)
    raise ClassNotFound("No formatter found for file name %r" % fn)


def get_all_formatters():
    """Return a generator for all formatters."""
    for formatter in FORMATTERS:
        yield formatter
    for _, formatter in find_plugin_formatters():
        yield formatter
