# -*- coding: utf-8 -*-
"""
    pygments.lexers
    ~~~~~~~~~~~~~~~

    Pygments lexers.

    :copyright: Copyright 2006-2013 by the Pygments team, see AUTHORS.
    :license: BSD, see LICENSE for details.
"""

import sys
import types
import fnmatch
from os.path import basename

from pygments.lexers._mapping import LEXERS
from pygments.plugin import find_plugin_lexers
from pygments.util import ClassNotFound, bytes


__all__ = ['get_lexer_by_name', 'get_lexer_for_filename', 'find_lexer_class',
           'guess_lexer'] + LEXERS.keys()

_lexer_cache = {}


def _load_lexers(module_name):
    """
    Load a lexer (and all others in the module too).
    """
    mod = __import__(module_name, None, None, ['__all__'])
    for lexer_name in mod.__all__:
        cls = getattr(mod, lexer_name)
        _lexer_cache[cls.name] = cls


def get_all_lexers():
    """
    Return a generator of tuples in the form ``(name, aliases,
    filenames, mimetypes)`` of all know lexers.
    """
    for item in LEXERS.itervalues():
        yield item[1:]
    for lexer in find_plugin_lexers():
        yield lexer.name, lexer.aliases, lexer.filenames, lexer.mimetypes


def find_lexer_class(name):
    """
    Lookup a lexer class by name. Return None if not found.
    """
    if name in _lexer_cache:
        return _lexer_cache[name]
    # lookup builtin lexers
    for module_name, lname, aliases, _, _ in LEXERS.itervalues():
        if name == lname:
            _load_lexers(module_name)
            return _lexer_cache[name]
    # continue with lexers from setuptools entrypoints
    for cls in find_plugin_lexers():
        if cls.name == name:
            return cls


def get_lexer_by_name(_alias, **options):
    """
    Get a lexer by an alias.
    """
    # lookup builtin lexers
    for module_name, name, aliases, _, _ in LEXERS.itervalues():
        if _alias in aliases:
            if name not in _lexer_cache:
                _load_lexers(module_name)
            return _lexer_cache[name](**options)
    # continue with lexers from setuptools entrypoints
    for cls in find_plugin_lexers():
        if _alias in cls.aliases:
            return cls(**options)
    raise ClassNotFound('no lexer for alias %r found' % _alias)


def get_lexer_for_filename(_fn, code=None, **options):
    """
    Get a lexer for a filename.  If multiple lexers match the filename
    pattern, use ``analyze_text()`` to figure out which one is more
    appropriate.
    """
    matches = []
    fn = basename(_fn)
    for modname, name, _, filenames, _ in LEXERS.itervalues():
        for filename in filenames:
            if fnmatch.fnmatch(fn, filename):
                if name not in _lexer_cache:
                    _load_lexers(modname)
                matches.append((_lexer_cache[name], filename))
    for cls in find_plugin_lexers():
        for filename in cls.filenames:
            if fnmatch.fnmatch(fn, filename):
                matches.append((cls, filename))

    if sys.version_info > (3,) and isinstance(code, bytes):
        # decode it, since all analyse_text functions expect unicode
        code = code.decode('latin1')

    def get_rating(info):
        cls, filename = info
        # explicit patterns get a bonus
        bonus = '*' not in filename and 0.5 or 0
        # The class _always_ defines analyse_text because it's included in
        # the Lexer class.  The default implementation returns None which
        # gets turned into 0.0.  Run scripts/detect_missing_analyse_text.py
        # to find lexers which need it overridden.
        if code:
            return cls.analyse_text(code) + bonus
        return cls.priority + bonus

    if matches:
        matches.sort(key=get_rating)
        #print "Possible lexers, after sort:", matches
        return matches[-1][0](**options)
    raise ClassNotFound('no lexer for filename %r found' % _fn)


def get_lexer_for_mimetype(_mime, **options):
    """
    Get a lexer for a mimetype.
    """
    for modname, name, _, _, mimetypes in LEXERS.itervalues():
        if _mime in mimetypes:
            if name not in _lexer_cache:
                _load_lexers(modname)
            return _lexer_cache[name](**options)
    for cls in find_plugin_lexers():
        if _mime in cls.mimetypes:
            return cls(**options)
    raise ClassNotFound('no lexer for mimetype %r found' % _mime)


def _iter_lexerclasses():
    """
    Return an iterator over all lexer classes.
    """
    for key in sorted(LEXERS):
        module_name, name = LEXERS[key][:2]
        if name not in _lexer_cache:
            _load_lexers(module_name)
        yield _lexer_cache[name]
    for lexer in find_plugin_lexers():
        yield lexer


def guess_lexer_for_filename(_fn, _text, **options):
    """
    Lookup all lexers that handle those filenames primary (``filenames``)
    or secondary (``alias_filenames``). Then run a text analysis for those
    lexers and choose the best result.

    usage::

        >>> from pygments.lexers import guess_lexer_for_filename
        >>> guess_lexer_for_filename('hello.html', '<%= @foo %>')
        <pygments.lexers.templates.RhtmlLexer object at 0xb7d2f32c>
        >>> guess_lexer_for_filename('hello.html', '<h1>{{ title|e }}</h1>')
        <pygments.lexers.templates.HtmlDjangoLexer object at 0xb7d2f2ac>
        >>> guess_lexer_for_filename('style.css', 'a { color: <?= $link ?> }')
        <pygments.lexers.templates.CssPhpLexer object at 0xb7ba518c>
    """
    fn = basename(_fn)
    primary = None
    matching_lexers = set()
    for lexer in _iter_lexerclasses():
        for filename in lexer.filenames:
            if fnmatch.fnmatch(fn, filename):
                matching_lexers.add(lexer)
                primary = lexer
        for filename in lexer.alias_filenames:
            if fnmatch.fnmatch(fn, filename):
                matching_lexers.add(lexer)
    if not matching_lexers:
        raise ClassNotFound('no lexer for filename %r found' % fn)
    if len(matching_lexers) == 1:
        return matching_lexers.pop()(**options)
    result = []
    for lexer in matching_lexers:
        rv = lexer.analyse_text(_text)
        if rv == 1.0:
            return lexer(**options)
        result.append((rv, lexer))
    result.sort()
    if not result[-1][0] and primary is not None:
        return primary(**options)
    return result[-1][1](**options)


def guess_lexer(_text, **options):
    """
    Guess a lexer by strong distinctions in the text (eg, shebang).
    """
    best_lexer = [0.0, None]
    for lexer in _iter_lexerclasses():
        rv = lexer.analyse_text(_text)
        if rv == 1.0:
            return lexer(**options)
        if rv > best_lexer[0]:
            best_lexer[:] = (rv, lexer)
    if not best_lexer[0] or best_lexer[1] is None:
        raise ClassNotFound('no lexer matching the text found')
    return best_lexer[1](**options)


class _automodule(types.ModuleType):
    """Automatically import lexers."""

    def __getattr__(self, name):
        info = LEXERS.get(name)
        if info:
            _load_lexers(info[0])
            cls = _lexer_cache[info[1]]
            setattr(self, name, cls)
            return cls
        raise AttributeError(name)


oldmod = sys.modules['pygments.lexers']
newmod = _automodule('pygments.lexers')
newmod.__dict__.update(oldmod.__dict__)
sys.modules['pygments.lexers'] = newmod
del newmod.newmod, newmod.oldmod, newmod.sys, newmod.types
