# -*- coding: utf-8 -*-
"""
    pygments.util
    ~~~~~~~~~~~~~

    Utility functions.

    :copyright: Copyright 2006-2013 by the Pygments team, see AUTHORS.
    :license: BSD, see LICENSE for details.
"""

import re
import sys
import codecs


split_path_re = re.compile(r'[/\\ ]')
doctype_lookup_re = re.compile(r'''(?smx)
    (<\?.*?\?>)?\s*
    <!DOCTYPE\s+(
     [a-zA-Z_][a-zA-Z0-9]*\s+
     [a-zA-Z_][a-zA-Z0-9]*\s+
     "[^"]*")
     [^>]*>
''')
tag_re = re.compile(r'<(.+?)(\s.*?)?>.*?</.+?>(?uism)')


class ClassNotFound(ValueError):
    """
    If one of the get_*_by_* functions didn't find a matching class.
    """


class OptionError(Exception):
    pass


def get_choice_opt(options, optname, allowed, default=None, normcase=False):
    string = options.get(optname, default)
    if normcase:
        string = string.lower()
    if string not in allowed:
        raise OptionError('Value for option %s must be one of %s' %
                          (optname, ', '.join(map(str, allowed))))
    return string


def get_bool_opt(options, optname, default=None):
    string = options.get(optname, default)
    if isinstance(string, bool):
        return string
    elif isinstance(string, int):
        return bool(string)
    elif not isinstance(string, basestring):
        raise OptionError('Invalid type %r for option %s; use '
                          '1/0, yes/no, true/false, on/off' % (
                          string, optname))
    elif string.lower() in ('1', 'yes', 'true', 'on'):
        return True
    elif string.lower() in ('0', 'no', 'false', 'off'):
        return False
    else:
        raise OptionError('Invalid value %r for option %s; use '
                          '1/0, yes/no, true/false, on/off' % (
                          string, optname))


def get_int_opt(options, optname, default=None):
    string = options.get(optname, default)
    try:
        return int(string)
    except TypeError:
        raise OptionError('Invalid type %r for option %s; you '
                          'must give an integer value' % (
                          string, optname))
    except ValueError:
        raise OptionError('Invalid value %r for option %s; you '
                          'must give an integer value' % (
                          string, optname))


def get_list_opt(options, optname, default=None):
    val = options.get(optname, default)
    if isinstance(val, basestring):
        return val.split()
    elif isinstance(val, (list, tuple)):
        return list(val)
    else:
        raise OptionError('Invalid type %r for option %s; you '
                          'must give a list value' % (
                          val, optname))


def docstring_headline(obj):
    if not obj.__doc__:
        return ''
    res = []
    for line in obj.__doc__.strip().splitlines():
        if line.strip():
            res.append(" " + line.strip())
        else:
            break
    return ''.join(res).lstrip()


def make_analysator(f):
    """
    Return a static text analysation function that
    returns float values.
    """
    def text_analyse(text):
        try:
            rv = f(text)
        except Exception:
            return 0.0
        if not rv:
            return 0.0
        try:
            return min(1.0, max(0.0, float(rv)))
        except (ValueError, TypeError):
            return 0.0
    text_analyse.__doc__ = f.__doc__
    return staticmethod(text_analyse)


def shebang_matches(text, regex):
    """
    Check if the given regular expression matches the last part of the
    shebang if one exists.

        >>> from pygments.util import shebang_matches
        >>> shebang_matches('#!/usr/bin/env python', r'python(2\.\d)?')
        True
        >>> shebang_matches('#!/usr/bin/python2.4', r'python(2\.\d)?')
        True
        >>> shebang_matches('#!/usr/bin/python-ruby', r'python(2\.\d)?')
        False
        >>> shebang_matches('#!/usr/bin/python/ruby', r'python(2\.\d)?')
        False
        >>> shebang_matches('#!/usr/bin/startsomethingwith python',
        ...                 r'python(2\.\d)?')
        True

    It also checks for common windows executable file extensions::

        >>> shebang_matches('#!C:\\Python2.4\\Python.exe', r'python(2\.\d)?')
        True

    Parameters (``'-f'`` or ``'--foo'`` are ignored so ``'perl'`` does
    the same as ``'perl -e'``)

    Note that this method automatically searches the whole string (eg:
    the regular expression is wrapped in ``'^$'``)
    """
    index = text.find('\n')
    if index >= 0:
        first_line = text[:index].lower()
    else:
        first_line = text.lower()
    if first_line.startswith('#!'):
        try:
            found = [x for x in split_path_re.split(first_line[2:].strip())
                     if x and not x.startswith('-')][-1]
        except IndexError:
            return False
        regex = re.compile('^%s(\.(exe|cmd|bat|bin))?$' % regex, re.IGNORECASE)
        if regex.search(found) is not None:
            return True
    return False


def doctype_matches(text, regex):
    """
    Check if the doctype matches a regular expression (if present).
    Note that this method only checks the first part of a DOCTYPE.
    eg: 'html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"'
    """
    m = doctype_lookup_re.match(text)
    if m is None:
        return False
    doctype = m.group(2)
    return re.compile(regex).match(doctype.strip()) is not None


def html_doctype_matches(text):
    """
    Check if the file looks like it has a html doctype.
    """
    return doctype_matches(text, r'html\s+PUBLIC\s+"-//W3C//DTD X?HTML.*')


_looks_like_xml_cache = {}
def looks_like_xml(text):
    """
    Check if a doctype exists or if we have some tags.
    """
    key = hash(text)
    try:
        return _looks_like_xml_cache[key]
    except KeyError:
        m = doctype_lookup_re.match(text)
        if m is not None:
            return True
        rv = tag_re.search(text[:1000]) is not None
        _looks_like_xml_cache[key] = rv
        return rv

# Python narrow build compatibility

def _surrogatepair(c):
    return (0xd7c0 + (c >> 10), (0xdc00 + (c & 0x3ff)))

def unirange(a, b):
    """
    Returns a regular expression string to match the given non-BMP range.
    """
    if b < a:
        raise ValueError("Bad character range")
    if a < 0x10000 or b < 0x10000:
        raise ValueError("unirange is only defined for non-BMP ranges")

    if sys.maxunicode > 0xffff:
        # wide build
        return u'[%s-%s]' % (unichr(a), unichr(b))
    else:
        # narrow build stores surrogates, and the 're' module handles them
        # (incorrectly) as characters.  Since there is still ordering among
        # these characters, expand the range to one that it understands.  Some
        # background in http://bugs.python.org/issue3665 and
        # http://bugs.python.org/issue12749
        #
        # Additionally, the lower constants are using unichr rather than
        # literals because jython [which uses the wide path] can't load this
        # file if they are literals.
        ah, al = _surrogatepair(a)
        bh, bl = _surrogatepair(b)
        if ah == bh:
            return u'(?:%s[%s-%s])' % (unichr(ah), unichr(al), unichr(bl))
        else:
            buf = []
            buf.append(u'%s[%s-%s]' %
                       (unichr(ah), unichr(al),
                        ah == bh and unichr(bl) or unichr(0xdfff)))
            if ah - bh > 1:
                buf.append(u'[%s-%s][%s-%s]' %
                           unichr(ah+1), unichr(bh-1), unichr(0xdc00), unichr(0xdfff))
            if ah != bh:
                buf.append(u'%s[%s-%s]' %
                           (unichr(bh), unichr(0xdc00), unichr(bl)))

            return u'(?:' + u'|'.join(buf) + u')'

# Python 2/3 compatibility

if sys.version_info < (3,0):
    b = bytes = str
    u_prefix = 'u'
    import StringIO, cStringIO
    BytesIO = cStringIO.StringIO
    StringIO = StringIO.StringIO
    uni_open = codecs.open
else:
    import builtins
    bytes = builtins.bytes
    u_prefix = ''
    def b(s):
        if isinstance(s, str):
            return bytes(map(ord, s))
        elif isinstance(s, bytes):
            return s
        else:
            raise TypeError("Invalid argument %r for b()" % (s,))
    import io
    BytesIO = io.BytesIO
    StringIO = io.StringIO
    uni_open = builtins.open
