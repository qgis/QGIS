# -*- coding: utf-8 -*-
"""
    pygments.formatters.terminal
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    Formatter for terminal output with ANSI sequences.

    :copyright: Copyright 2006-2013 by the Pygments team, see AUTHORS.
    :license: BSD, see LICENSE for details.
"""

import sys

from pygments.formatter import Formatter
from pygments.token import Keyword, Name, Comment, String, Error, \
     Number, Operator, Generic, Token, Whitespace
from pygments.console import ansiformat
from pygments.util import get_choice_opt


__all__ = ['TerminalFormatter']


#: Map token types to a tuple of color values for light and dark
#: backgrounds.
TERMINAL_COLORS = {
    Token:              ('',            ''),

    Whitespace:         ('lightgray',   'darkgray'),
    Comment:            ('lightgray',   'darkgray'),
    Comment.Preproc:    ('teal',        'turquoise'),
    Keyword:            ('darkblue',    'blue'),
    Keyword.Type:       ('teal',        'turquoise'),
    Operator.Word:      ('purple',      'fuchsia'),
    Name.Builtin:       ('teal',        'turquoise'),
    Name.Function:      ('darkgreen',   'green'),
    Name.Namespace:     ('_teal_',      '_turquoise_'),
    Name.Class:         ('_darkgreen_', '_green_'),
    Name.Exception:     ('teal',        'turquoise'),
    Name.Decorator:     ('darkgray',    'lightgray'),
    Name.Variable:      ('darkred',     'red'),
    Name.Constant:      ('darkred',     'red'),
    Name.Attribute:     ('teal',        'turquoise'),
    Name.Tag:           ('blue',        'blue'),
    String:             ('brown',       'brown'),
    Number:             ('darkblue',    'blue'),

    Generic.Deleted:    ('red',        'red'),
    Generic.Inserted:   ('darkgreen',  'green'),
    Generic.Heading:    ('**',         '**'),
    Generic.Subheading: ('*purple*',   '*fuchsia*'),
    Generic.Error:      ('red',        'red'),

    Error:              ('_red_',      '_red_'),
}


class TerminalFormatter(Formatter):
    r"""
    Format tokens with ANSI color sequences, for output in a text console.
    Color sequences are terminated at newlines, so that paging the output
    works correctly.

    The `get_style_defs()` method doesn't do anything special since there is
    no support for common styles.

    Options accepted:

    `bg`
        Set to ``"light"`` or ``"dark"`` depending on the terminal's background
        (default: ``"light"``).

    `colorscheme`
        A dictionary mapping token types to (lightbg, darkbg) color names or
        ``None`` (default: ``None`` = use builtin colorscheme).
    """
    name = 'Terminal'
    aliases = ['terminal', 'console']
    filenames = []

    def __init__(self, **options):
        Formatter.__init__(self, **options)
        self.darkbg = get_choice_opt(options, 'bg',
                                     ['light', 'dark'], 'light') == 'dark'
        self.colorscheme = options.get('colorscheme', None) or TERMINAL_COLORS

    def format(self, tokensource, outfile):
        # hack: if the output is a terminal and has an encoding set,
        # use that to avoid unicode encode problems
        if not self.encoding and hasattr(outfile, "encoding") and \
           hasattr(outfile, "isatty") and outfile.isatty() and \
           sys.version_info < (3,):
            self.encoding = outfile.encoding
        return Formatter.format(self, tokensource, outfile)

    def format_unencoded(self, tokensource, outfile):
        for ttype, value in tokensource:
            color = self.colorscheme.get(ttype)
            while color is None:
                ttype = ttype[:-1]
                color = self.colorscheme.get(ttype)
            if color:
                color = color[self.darkbg]
                spl = value.split('\n')
                for line in spl[:-1]:
                    if line:
                        outfile.write(ansiformat(color, line))
                    outfile.write('\n')
                if spl[-1]:
                    outfile.write(ansiformat(color, spl[-1]))
            else:
                outfile.write(value)
