# -*- coding: utf-8 -*-
"""
    pygments.lexers.text
    ~~~~~~~~~~~~~~~~~~~~

    Lexers for non-source code file types.

    :copyright: Copyright 2006-2013 by the Pygments team, see AUTHORS.
    :license: BSD, see LICENSE for details.
"""

import re
from bisect import bisect

from pygments.lexer import Lexer, LexerContext, RegexLexer, ExtendedRegexLexer, \
     bygroups, include, using, this, do_insertions
from pygments.token import Punctuation, Text, Comment, Keyword, Name, String, \
     Generic, Operator, Number, Whitespace, Literal
from pygments.util import get_bool_opt, ClassNotFound
from pygments.lexers.other import BashLexer

__all__ = ['IniLexer', 'PropertiesLexer', 'SourcesListLexer', 'BaseMakefileLexer',
           'MakefileLexer', 'DiffLexer', 'IrcLogsLexer', 'TexLexer',
           'GroffLexer', 'ApacheConfLexer', 'BBCodeLexer', 'MoinWikiLexer',
           'RstLexer', 'VimLexer', 'GettextLexer', 'SquidConfLexer',
           'DebianControlLexer', 'DarcsPatchLexer', 'YamlLexer',
           'LighttpdConfLexer', 'NginxConfLexer', 'CMakeLexer', 'HttpLexer',
           'PyPyLogLexer', 'RegeditLexer', 'HxmlLexer']


class IniLexer(RegexLexer):
    """
    Lexer for configuration files in INI style.
    """

    name = 'INI'
    aliases = ['ini', 'cfg']
    filenames = ['*.ini', '*.cfg']
    mimetypes = ['text/x-ini']

    tokens = {
        'root': [
            (r'\s+', Text),
            (r'[;#].*', Comment.Single),
            (r'\[.*?\]$', Keyword),
            (r'(.*?)([ \t]*)(=)([ \t]*)(.*(?:\n[ \t].+)*)',
             bygroups(Name.Attribute, Text, Operator, Text, String))
        ]
    }

    def analyse_text(text):
        npos = text.find('\n')
        if npos < 3:
            return False
        return text[0] == '[' and text[npos-1] == ']'


class RegeditLexer(RegexLexer):
    """
    Lexer for `Windows Registry
    <http://en.wikipedia.org/wiki/Windows_Registry#.REG_files>`_ files produced
    by regedit.

    *New in Pygments 1.6.*
    """

    name = 'reg'
    aliases = ['registry']
    filenames = ['*.reg']
    mimetypes = ['text/x-windows-registry']

    tokens = {
        'root': [
            (r'Windows Registry Editor.*', Text),
            (r'\s+', Text),
            (r'[;#].*', Comment.Single),
            (r'(\[)(-?)(HKEY_[A-Z_]+)(.*?\])$',
             bygroups(Keyword, Operator, Name.Builtin, Keyword)),
            # String keys, which obey somewhat normal escaping
            (r'("(?:\\"|\\\\|[^"])+")([ \t]*)(=)([ \t]*)',
             bygroups(Name.Attribute, Text, Operator, Text),
             'value'),
            # Bare keys (includes @)
            (r'(.*?)([ \t]*)(=)([ \t]*)',
             bygroups(Name.Attribute, Text, Operator, Text),
             'value'),
        ],
        'value': [
            (r'-', Operator, '#pop'), # delete value
            (r'(dword|hex(?:\([0-9a-fA-F]\))?)(:)([0-9a-fA-F,]+)',
             bygroups(Name.Variable, Punctuation, Number), '#pop'),
            # As far as I know, .reg files do not support line continuation.
            (r'.*', String, '#pop'),
        ]
    }

    def analyse_text(text):
        return text.startswith('Windows Registry Editor')


class PropertiesLexer(RegexLexer):
    """
    Lexer for configuration files in Java's properties format.

    *New in Pygments 1.4.*
    """

    name = 'Properties'
    aliases = ['properties']
    filenames = ['*.properties']
    mimetypes = ['text/x-java-properties']

    tokens = {
        'root': [
            (r'\s+', Text),
            (r'(?:[;#]|//).*$', Comment),
            (r'(.*?)([ \t]*)([=:])([ \t]*)(.*(?:(?<=\\)\n.*)*)',
             bygroups(Name.Attribute, Text, Operator, Text, String)),
        ],
    }


class SourcesListLexer(RegexLexer):
    """
    Lexer that highlights debian sources.list files.

    *New in Pygments 0.7.*
    """

    name = 'Debian Sourcelist'
    aliases = ['sourceslist', 'sources.list']
    filenames = ['sources.list']
    mimetype = ['application/x-debian-sourceslist']

    tokens = {
        'root': [
            (r'\s+', Text),
            (r'#.*?$', Comment),
            (r'^(deb(?:-src)?)(\s+)',
             bygroups(Keyword, Text), 'distribution')
        ],
        'distribution': [
            (r'#.*?$', Comment, '#pop'),
            (r'\$\(ARCH\)', Name.Variable),
            (r'[^\s$[]+', String),
            (r'\[', String.Other, 'escaped-distribution'),
            (r'\$', String),
            (r'\s+', Text, 'components')
        ],
        'escaped-distribution': [
            (r'\]', String.Other, '#pop'),
            (r'\$\(ARCH\)', Name.Variable),
            (r'[^\]$]+', String.Other),
            (r'\$', String.Other)
        ],
        'components': [
            (r'#.*?$', Comment, '#pop:2'),
            (r'$', Text, '#pop:2'),
            (r'\s+', Text),
            (r'\S+', Keyword.Pseudo),
        ]
    }

    def analyse_text(text):
        for line in text.split('\n'):
            line = line.strip()
            if not (line.startswith('#') or line.startswith('deb ') or
                    line.startswith('deb-src ') or not line):
                return False
        return True


class MakefileLexer(Lexer):
    """
    Lexer for BSD and GNU make extensions (lenient enough to handle both in
    the same file even).

    *Rewritten in Pygments 0.10.*
    """

    name = 'Makefile'
    aliases = ['make', 'makefile', 'mf', 'bsdmake']
    filenames = ['*.mak', 'Makefile', 'makefile', 'Makefile.*', 'GNUmakefile']
    mimetypes = ['text/x-makefile']

    r_special = re.compile(r'^(?:'
        # BSD Make
        r'\.\s*(include|undef|error|warning|if|else|elif|endif|for|endfor)|'
        # GNU Make
        r'\s*(ifeq|ifneq|ifdef|ifndef|else|endif|-?include|define|endef|:))(?=\s)')
    r_comment = re.compile(r'^\s*@?#')

    def get_tokens_unprocessed(self, text):
        ins = []
        lines = text.splitlines(True)
        done = ''
        lex = BaseMakefileLexer(**self.options)
        backslashflag = False
        for line in lines:
            if self.r_special.match(line) or backslashflag:
                ins.append((len(done), [(0, Comment.Preproc, line)]))
                backslashflag = line.strip().endswith('\\')
            elif self.r_comment.match(line):
                ins.append((len(done), [(0, Comment, line)]))
            else:
                done += line
        for item in do_insertions(ins, lex.get_tokens_unprocessed(done)):
            yield item


class BaseMakefileLexer(RegexLexer):
    """
    Lexer for simple Makefiles (no preprocessing).

    *New in Pygments 0.10.*
    """

    name = 'Base Makefile'
    aliases = ['basemake']
    filenames = []
    mimetypes = []

    tokens = {
        'root': [
            (r'^(?:[\t ]+.*\n|\n)+', using(BashLexer)),
            (r'\$\((?:.*\\\n|.*\n)+', using(BashLexer)),
            (r'\s+', Text),
            (r'#.*?\n', Comment),
            (r'(export)(\s+)(?=[a-zA-Z0-9_${}\t -]+\n)',
             bygroups(Keyword, Text), 'export'),
            (r'export\s+', Keyword),
            # assignment
            (r'([a-zA-Z0-9_${}.-]+)(\s*)([!?:+]?=)([ \t]*)((?:.*\\\n)+|.*\n)',
             bygroups(Name.Variable, Text, Operator, Text, using(BashLexer))),
            # strings
            (r'(?s)"(\\\\|\\.|[^"\\])*"', String.Double),
            (r"(?s)'(\\\\|\\.|[^'\\])*'", String.Single),
            # targets
            (r'([^\n:]+)(:+)([ \t]*)', bygroups(Name.Function, Operator, Text),
             'block-header'),
            # TODO: add paren handling (grr)
        ],
        'export': [
            (r'[a-zA-Z0-9_${}-]+', Name.Variable),
            (r'\n', Text, '#pop'),
            (r'\s+', Text),
        ],
        'block-header': [
            (r'[^,\\\n#]+', Number),
            (r',', Punctuation),
            (r'#.*?\n', Comment),
            (r'\\\n', Text), # line continuation
            (r'\\.', Text),
            (r'(?:[\t ]+.*\n|\n)+', using(BashLexer), '#pop'),
        ],
    }


class DiffLexer(RegexLexer):
    """
    Lexer for unified or context-style diffs or patches.
    """

    name = 'Diff'
    aliases = ['diff', 'udiff']
    filenames = ['*.diff', '*.patch']
    mimetypes = ['text/x-diff', 'text/x-patch']

    tokens = {
        'root': [
            (r' .*\n', Text),
            (r'\+.*\n', Generic.Inserted),
            (r'-.*\n', Generic.Deleted),
            (r'!.*\n', Generic.Strong),
            (r'@.*\n', Generic.Subheading),
            (r'([Ii]ndex|diff).*\n', Generic.Heading),
            (r'=.*\n', Generic.Heading),
            (r'.*\n', Text),
        ]
    }

    def analyse_text(text):
        if text[:7] == 'Index: ':
            return True
        if text[:5] == 'diff ':
            return True
        if text[:4] == '--- ':
            return 0.9


DPATCH_KEYWORDS = ['hunk', 'addfile', 'adddir', 'rmfile', 'rmdir', 'move',
    'replace']

class DarcsPatchLexer(RegexLexer):
    """
    DarcsPatchLexer is a lexer for the various versions of the darcs patch
    format.  Examples of this format are derived by commands such as
    ``darcs annotate --patch`` and ``darcs send``.

    *New in Pygments 0.10.*
    """
    name = 'Darcs Patch'
    aliases = ['dpatch']
    filenames = ['*.dpatch', '*.darcspatch']

    tokens = {
        'root': [
            (r'<', Operator),
            (r'>', Operator),
            (r'{', Operator),
            (r'}', Operator),
            (r'(\[)((?:TAG )?)(.*)(\n)(.*)(\*\*)(\d+)(\s?)(\])',
             bygroups(Operator, Keyword, Name, Text, Name, Operator,
                      Literal.Date, Text, Operator)),
            (r'(\[)((?:TAG )?)(.*)(\n)(.*)(\*\*)(\d+)(\s?)',
             bygroups(Operator, Keyword, Name, Text, Name, Operator,
                      Literal.Date, Text), 'comment'),
            (r'New patches:', Generic.Heading),
            (r'Context:', Generic.Heading),
            (r'Patch bundle hash:', Generic.Heading),
            (r'(\s*)(%s)(.*\n)' % '|'.join(DPATCH_KEYWORDS),
                bygroups(Text, Keyword, Text)),
            (r'\+', Generic.Inserted, "insert"),
            (r'-', Generic.Deleted, "delete"),
            (r'.*\n', Text),
        ],
        'comment': [
            (r'[^\]].*\n', Comment),
            (r'\]', Operator, "#pop"),
        ],
        'specialText': [ # darcs add [_CODE_] special operators for clarity
            (r'\n', Text, "#pop"), # line-based
            (r'\[_[^_]*_]', Operator),
        ],
        'insert': [
            include('specialText'),
            (r'\[', Generic.Inserted),
            (r'[^\n\[]+', Generic.Inserted),
        ],
        'delete': [
            include('specialText'),
            (r'\[', Generic.Deleted),
            (r'[^\n\[]+', Generic.Deleted),
        ],
    }


class IrcLogsLexer(RegexLexer):
    """
    Lexer for IRC logs in *irssi*, *xchat* or *weechat* style.
    """

    name = 'IRC logs'
    aliases = ['irc']
    filenames = ['*.weechatlog']
    mimetypes = ['text/x-irclog']

    flags = re.VERBOSE | re.MULTILINE
    timestamp = r"""
        (
          # irssi / xchat and others
          (?: \[|\()?                  # Opening bracket or paren for the timestamp
            (?:                        # Timestamp
                (?: (?:\d{1,4} [-/]?)+ # Date as - or /-separated groups of digits
                 [T ])?                # Date/time separator: T or space
                (?: \d?\d [:.]?)+      # Time as :/.-separated groups of 1 or 2 digits
            )
          (?: \]|\))?\s+               # Closing bracket or paren for the timestamp
        |
          # weechat
          \d{4}\s\w{3}\s\d{2}\s        # Date
          \d{2}:\d{2}:\d{2}\s+         # Time + Whitespace
        |
          # xchat
          \w{3}\s\d{2}\s               # Date
          \d{2}:\d{2}:\d{2}\s+         # Time + Whitespace
        )?
    """
    tokens = {
        'root': [
                # log start/end
            (r'^\*\*\*\*(.*)\*\*\*\*$', Comment),
            # hack
            ("^" + timestamp + r'(\s*<[^>]*>\s*)$', bygroups(Comment.Preproc, Name.Tag)),
            # normal msgs
            ("^" + timestamp + r"""
                (\s*<.*?>\s*)          # Nick """,
             bygroups(Comment.Preproc, Name.Tag), 'msg'),
            # /me msgs
            ("^" + timestamp + r"""
                (\s*[*]\s+)            # Star
                (\S+\s+.*?\n)          # Nick + rest of message """,
             bygroups(Comment.Preproc, Keyword, Generic.Inserted)),
            # join/part msgs
            ("^" + timestamp + r"""
                (\s*(?:\*{3}|<?-[!@=P]?->?)\s*)  # Star(s) or symbols
                (\S+\s+)                     # Nick + Space
                (.*?\n)                         # Rest of message """,
             bygroups(Comment.Preproc, Keyword, String, Comment)),
            (r"^.*?\n", Text),
        ],
        'msg': [
            (r"\S+:(?!//)", Name.Attribute),  # Prefix
            (r".*\n", Text, '#pop'),
        ],
    }


class BBCodeLexer(RegexLexer):
    """
    A lexer that highlights BBCode(-like) syntax.

    *New in Pygments 0.6.*
    """

    name = 'BBCode'
    aliases = ['bbcode']
    mimetypes = ['text/x-bbcode']

    tokens = {
        'root': [
            (r'[^[]+', Text),
            # tag/end tag begin
            (r'\[/?\w+', Keyword, 'tag'),
            # stray bracket
            (r'\[', Text),
        ],
        'tag': [
            (r'\s+', Text),
            # attribute with value
            (r'(\w+)(=)("?[^\s"\]]+"?)',
             bygroups(Name.Attribute, Operator, String)),
            # tag argument (a la [color=green])
            (r'(=)("?[^\s"\]]+"?)',
             bygroups(Operator, String)),
            # tag end
            (r'\]', Keyword, '#pop'),
        ],
    }


class TexLexer(RegexLexer):
    """
    Lexer for the TeX and LaTeX typesetting languages.
    """

    name = 'TeX'
    aliases = ['tex', 'latex']
    filenames = ['*.tex', '*.aux', '*.toc']
    mimetypes = ['text/x-tex', 'text/x-latex']

    tokens = {
        'general': [
            (r'%.*?\n', Comment),
            (r'[{}]', Name.Builtin),
            (r'[&_^]', Name.Builtin),
        ],
        'root': [
            (r'\\\[', String.Backtick, 'displaymath'),
            (r'\\\(', String, 'inlinemath'),
            (r'\$\$', String.Backtick, 'displaymath'),
            (r'\$', String, 'inlinemath'),
            (r'\\([a-zA-Z]+|.)', Keyword, 'command'),
            include('general'),
            (r'[^\\$%&_^{}]+', Text),
        ],
        'math': [
            (r'\\([a-zA-Z]+|.)', Name.Variable),
            include('general'),
            (r'[0-9]+', Number),
            (r'[-=!+*/()\[\]]', Operator),
            (r'[^=!+*/()\[\]\\$%&_^{}0-9-]+', Name.Builtin),
        ],
        'inlinemath': [
            (r'\\\)', String, '#pop'),
            (r'\$', String, '#pop'),
            include('math'),
        ],
        'displaymath': [
            (r'\\\]', String, '#pop'),
            (r'\$\$', String, '#pop'),
            (r'\$', Name.Builtin),
            include('math'),
        ],
        'command': [
            (r'\[.*?\]', Name.Attribute),
            (r'\*', Keyword),
            (r'', Text, '#pop'),
        ],
    }

    def analyse_text(text):
        for start in ("\\documentclass", "\\input", "\\documentstyle",
                      "\\relax"):
            if text[:len(start)] == start:
                return True


class GroffLexer(RegexLexer):
    """
    Lexer for the (g)roff typesetting language, supporting groff
    extensions. Mainly useful for highlighting manpage sources.

    *New in Pygments 0.6.*
    """

    name = 'Groff'
    aliases = ['groff', 'nroff', 'man']
    filenames = ['*.[1234567]', '*.man']
    mimetypes = ['application/x-troff', 'text/troff']

    tokens = {
        'root': [
            (r'(\.)(\w+)', bygroups(Text, Keyword), 'request'),
            (r'\.', Punctuation, 'request'),
            # Regular characters, slurp till we find a backslash or newline
            (r'[^\\\n]*', Text, 'textline'),
        ],
        'textline': [
            include('escapes'),
            (r'[^\\\n]+', Text),
            (r'\n', Text, '#pop'),
        ],
        'escapes': [
            # groff has many ways to write escapes.
            (r'\\"[^\n]*', Comment),
            (r'\\[fn]\w', String.Escape),
            (r'\\\(.{2}', String.Escape),
            (r'\\.\[.*\]', String.Escape),
            (r'\\.', String.Escape),
            (r'\\\n', Text, 'request'),
        ],
        'request': [
            (r'\n', Text, '#pop'),
            include('escapes'),
            (r'"[^\n"]+"', String.Double),
            (r'\d+', Number),
            (r'\S+', String),
            (r'\s+', Text),
        ],
    }

    def analyse_text(text):
        if text[:1] != '.':
            return False
        if text[:3] == '.\\"':
            return True
        if text[:4] == '.TH ':
            return True
        if text[1:3].isalnum() and text[3].isspace():
            return 0.9


class ApacheConfLexer(RegexLexer):
    """
    Lexer for configuration files following the Apache config file
    format.

    *New in Pygments 0.6.*
    """

    name = 'ApacheConf'
    aliases = ['apacheconf', 'aconf', 'apache']
    filenames = ['.htaccess', 'apache.conf', 'apache2.conf']
    mimetypes = ['text/x-apacheconf']
    flags = re.MULTILINE | re.IGNORECASE

    tokens = {
        'root': [
            (r'\s+', Text),
            (r'(#.*?)$', Comment),
            (r'(<[^\s>]+)(?:(\s+)(.*?))?(>)',
             bygroups(Name.Tag, Text, String, Name.Tag)),
            (r'([a-zA-Z][a-zA-Z0-9_]*)(\s+)',
             bygroups(Name.Builtin, Text), 'value'),
            (r'\.+', Text),
        ],
        'value': [
            (r'$', Text, '#pop'),
            (r'[^\S\n]+', Text),
            (r'\d+\.\d+\.\d+\.\d+(?:/\d+)?', Number),
            (r'\d+', Number),
            (r'/([a-zA-Z0-9][a-zA-Z0-9_./-]+)', String.Other),
            (r'(on|off|none|any|all|double|email|dns|min|minimal|'
             r'os|productonly|full|emerg|alert|crit|error|warn|'
             r'notice|info|debug|registry|script|inetd|standalone|'
             r'user|group)\b', Keyword),
            (r'"([^"\\]*(?:\\.[^"\\]*)*)"', String.Double),
            (r'[^\s"]+', Text)
        ]
    }


class MoinWikiLexer(RegexLexer):
    """
    For MoinMoin (and Trac) Wiki markup.

    *New in Pygments 0.7.*
    """

    name = 'MoinMoin/Trac Wiki markup'
    aliases = ['trac-wiki', 'moin']
    filenames = []
    mimetypes = ['text/x-trac-wiki']
    flags = re.MULTILINE | re.IGNORECASE

    tokens = {
        'root': [
            (r'^#.*$', Comment),
            (r'(!)(\S+)', bygroups(Keyword, Text)), # Ignore-next
            # Titles
            (r'^(=+)([^=]+)(=+)(\s*#.+)?$',
             bygroups(Generic.Heading, using(this), Generic.Heading, String)),
            # Literal code blocks, with optional shebang
            (r'({{{)(\n#!.+)?', bygroups(Name.Builtin, Name.Namespace), 'codeblock'),
            (r'(\'\'\'?|\|\||`|__|~~|\^|,,|::)', Comment), # Formatting
            # Lists
            (r'^( +)([.*-])( )', bygroups(Text, Name.Builtin, Text)),
            (r'^( +)([a-z]{1,5}\.)( )', bygroups(Text, Name.Builtin, Text)),
            # Other Formatting
            (r'\[\[\w+.*?\]\]', Keyword), # Macro
            (r'(\[[^\s\]]+)(\s+[^\]]+?)?(\])',
             bygroups(Keyword, String, Keyword)), # Link
            (r'^----+$', Keyword), # Horizontal rules
            (r'[^\n\'\[{!_~^,|]+', Text),
            (r'\n', Text),
            (r'.', Text),
        ],
        'codeblock': [
            (r'}}}', Name.Builtin, '#pop'),
            # these blocks are allowed to be nested in Trac, but not MoinMoin
            (r'{{{', Text, '#push'),
            (r'[^{}]+', Comment.Preproc), # slurp boring text
            (r'.', Comment.Preproc), # allow loose { or }
        ],
    }


class RstLexer(RegexLexer):
    """
    For `reStructuredText <http://docutils.sf.net/rst.html>`_ markup.

    *New in Pygments 0.7.*

    Additional options accepted:

    `handlecodeblocks`
        Highlight the contents of ``.. sourcecode:: langauge`` and
        ``.. code:: language`` directives with a lexer for the given
        language (default: ``True``). *New in Pygments 0.8.*
    """
    name = 'reStructuredText'
    aliases = ['rst', 'rest', 'restructuredtext']
    filenames = ['*.rst', '*.rest']
    mimetypes = ["text/x-rst", "text/prs.fallenstein.rst"]
    flags = re.MULTILINE

    def _handle_sourcecode(self, match):
        from pygments.lexers import get_lexer_by_name

        # section header
        yield match.start(1), Punctuation, match.group(1)
        yield match.start(2), Text, match.group(2)
        yield match.start(3), Operator.Word, match.group(3)
        yield match.start(4), Punctuation, match.group(4)
        yield match.start(5), Text, match.group(5)
        yield match.start(6), Keyword, match.group(6)
        yield match.start(7), Text, match.group(7)

        # lookup lexer if wanted and existing
        lexer = None
        if self.handlecodeblocks:
            try:
                lexer = get_lexer_by_name(match.group(6).strip())
            except ClassNotFound:
                pass
        indention = match.group(8)
        indention_size = len(indention)
        code = (indention + match.group(9) + match.group(10) + match.group(11))

        # no lexer for this language. handle it like it was a code block
        if lexer is None:
            yield match.start(8), String, code
            return

        # highlight the lines with the lexer.
        ins = []
        codelines = code.splitlines(True)
        code = ''
        for line in codelines:
            if len(line) > indention_size:
                ins.append((len(code), [(0, Text, line[:indention_size])]))
                code += line[indention_size:]
            else:
                code += line
        for item in do_insertions(ins, lexer.get_tokens_unprocessed(code)):
            yield item

    # from docutils.parsers.rst.states
    closers = u'\'")]}>\u2019\u201d\xbb!?'
    unicode_delimiters = u'\u2010\u2011\u2012\u2013\u2014\u00a0'
    end_string_suffix = (r'((?=$)|(?=[-/:.,; \n\x00%s%s]))'
                         % (re.escape(unicode_delimiters),
                            re.escape(closers)))

    tokens = {
        'root': [
            # Heading with overline
            (r'^(=+|-+|`+|:+|\.+|\'+|"+|~+|\^+|_+|\*+|\++|#+)([ \t]*\n)'
             r'(.+)(\n)(\1)(\n)',
             bygroups(Generic.Heading, Text, Generic.Heading,
                      Text, Generic.Heading, Text)),
            # Plain heading
            (r'^(\S.*)(\n)(={3,}|-{3,}|`{3,}|:{3,}|\.{3,}|\'{3,}|"{3,}|'
             r'~{3,}|\^{3,}|_{3,}|\*{3,}|\+{3,}|#{3,})(\n)',
             bygroups(Generic.Heading, Text, Generic.Heading, Text)),
            # Bulleted lists
            (r'^(\s*)([-*+])( .+\n(?:\1  .+\n)*)',
             bygroups(Text, Number, using(this, state='inline'))),
            # Numbered lists
            (r'^(\s*)([0-9#ivxlcmIVXLCM]+\.)( .+\n(?:\1  .+\n)*)',
             bygroups(Text, Number, using(this, state='inline'))),
            (r'^(\s*)(\(?[0-9#ivxlcmIVXLCM]+\))( .+\n(?:\1  .+\n)*)',
             bygroups(Text, Number, using(this, state='inline'))),
            # Numbered, but keep words at BOL from becoming lists
            (r'^(\s*)([A-Z]+\.)( .+\n(?:\1  .+\n)+)',
             bygroups(Text, Number, using(this, state='inline'))),
            (r'^(\s*)(\(?[A-Za-z]+\))( .+\n(?:\1  .+\n)+)',
             bygroups(Text, Number, using(this, state='inline'))),
            # Line blocks
            (r'^(\s*)(\|)( .+\n(?:\|  .+\n)*)',
             bygroups(Text, Operator, using(this, state='inline'))),
            # Sourcecode directives
            (r'^( *\.\.)(\s*)((?:source)?code)(::)([ \t]*)([^\n]+)'
             r'(\n[ \t]*\n)([ \t]+)(.*)(\n)((?:(?:\8.*|)\n)+)',
             _handle_sourcecode),
            # A directive
            (r'^( *\.\.)(\s*)([\w:-]+?)(::)(?:([ \t]*)(.*))',
             bygroups(Punctuation, Text, Operator.Word, Punctuation, Text,
                      using(this, state='inline'))),
            # A reference target
            (r'^( *\.\.)(\s*)(_(?:[^:\\]|\\.)+:)(.*?)$',
             bygroups(Punctuation, Text, Name.Tag, using(this, state='inline'))),
            # A footnote/citation target
            (r'^( *\.\.)(\s*)(\[.+\])(.*?)$',
             bygroups(Punctuation, Text, Name.Tag, using(this, state='inline'))),
            # A substitution def
            (r'^( *\.\.)(\s*)(\|.+\|)(\s*)([\w:-]+?)(::)(?:([ \t]*)(.*))',
             bygroups(Punctuation, Text, Name.Tag, Text, Operator.Word,
                      Punctuation, Text, using(this, state='inline'))),
            # Comments
            (r'^ *\.\..*(\n( +.*\n|\n)+)?', Comment.Preproc),
            # Field list
            (r'^( *)(:[a-zA-Z-]+:)(\s*)$', bygroups(Text, Name.Class, Text)),
            (r'^( *)(:.*?:)([ \t]+)(.*?)$',
             bygroups(Text, Name.Class, Text, Name.Function)),
            # Definition list
            (r'^([^ ].*(?<!::)\n)((?:(?: +.*)\n)+)',
             bygroups(using(this, state='inline'), using(this, state='inline'))),
            # Code blocks
            (r'(::)(\n[ \t]*\n)([ \t]+)(.*)(\n)((?:(?:\3.*|)\n)+)',
             bygroups(String.Escape, Text, String, String, Text, String)),
            include('inline'),
        ],
        'inline': [
            (r'\\.', Text), # escape
            (r'``', String, 'literal'), # code
            (r'(`.+?)(<.+?>)(`__?)',  # reference with inline target
             bygroups(String, String.Interpol, String)),
            (r'`.+?`__?', String), # reference
            (r'(`.+?`)(:[a-zA-Z0-9:-]+?:)?',
             bygroups(Name.Variable, Name.Attribute)), # role
            (r'(:[a-zA-Z0-9:-]+?:)(`.+?`)',
             bygroups(Name.Attribute, Name.Variable)), # role (content first)
            (r'\*\*.+?\*\*', Generic.Strong), # Strong emphasis
            (r'\*.+?\*', Generic.Emph), # Emphasis
            (r'\[.*?\]_', String), # Footnote or citation
            (r'<.+?>', Name.Tag), # Hyperlink
            (r'[^\\\n\[*`:]+', Text),
            (r'.', Text),
        ],
        'literal': [
            (r'[^`]+', String),
            (r'``' + end_string_suffix, String, '#pop'),
            (r'`', String),
        ]
    }

    def __init__(self, **options):
        self.handlecodeblocks = get_bool_opt(options, 'handlecodeblocks', True)
        RegexLexer.__init__(self, **options)

    def analyse_text(text):
        if text[:2] == '..' and text[2:3] != '.':
            return 0.3
        p1 = text.find("\n")
        p2 = text.find("\n", p1 + 1)
        if (p2 > -1 and              # has two lines
            p1 * 2 + 1 == p2 and     # they are the same length
            text[p1+1] in '-=' and   # the next line both starts and ends with
            text[p1+1] == text[p2-1]): # ...a sufficiently high header
            return 0.5


class VimLexer(RegexLexer):
    """
    Lexer for VimL script files.

    *New in Pygments 0.8.*
    """
    name = 'VimL'
    aliases = ['vim']
    filenames = ['*.vim', '.vimrc', '.exrc', '.gvimrc',
                 '_vimrc', '_exrc', '_gvimrc', 'vimrc', 'gvimrc']
    mimetypes = ['text/x-vim']
    flags = re.MULTILINE

    tokens = {
        'root': [
            (r'^\s*".*', Comment),

            (r'[ \t]+', Text),
            # TODO: regexes can have other delims
            (r'/(\\\\|\\/|[^\n/])*/', String.Regex),
            (r'"(\\\\|\\"|[^\n"])*"', String.Double),
            (r"'(\\\\|\\'|[^\n'])*'", String.Single),

            # Who decided that doublequote was a good comment character??
            (r'(?<=\s)"[^\-:.%#=*].*', Comment),
            (r'-?\d+', Number),
            (r'#[0-9a-f]{6}', Number.Hex),
            (r'^:', Punctuation),
            (r'[()<>+=!|,~-]', Punctuation), # Inexact list.  Looks decent.
            (r'\b(let|if|else|endif|elseif|fun|function|endfunction)\b',
             Keyword),
            (r'\b(NONE|bold|italic|underline|dark|light)\b', Name.Builtin),
            (r'\b\w+\b', Name.Other), # These are postprocessed below
            (r'.', Text),
        ],
    }
    def __init__(self, **options):
        from pygments.lexers._vimbuiltins import command, option, auto
        self._cmd = command
        self._opt = option
        self._aut = auto

        RegexLexer.__init__(self, **options)

    def is_in(self, w, mapping):
        r"""
        It's kind of difficult to decide if something might be a keyword
        in VimL because it allows you to abbreviate them.  In fact,
        'ab[breviate]' is a good example.  :ab, :abbre, or :abbreviate are
        valid ways to call it so rather than making really awful regexps
        like::

            \bab(?:b(?:r(?:e(?:v(?:i(?:a(?:t(?:e)?)?)?)?)?)?)?)?\b

        we match `\b\w+\b` and then call is_in() on those tokens.  See
        `scripts/get_vimkw.py` for how the lists are extracted.
        """
        p = bisect(mapping, (w,))
        if p > 0:
            if mapping[p-1][0] == w[:len(mapping[p-1][0])] and \
               mapping[p-1][1][:len(w)] == w: return True
        if p < len(mapping):
            return mapping[p][0] == w[:len(mapping[p][0])] and \
                   mapping[p][1][:len(w)] == w
        return False

    def get_tokens_unprocessed(self, text):
        # TODO: builtins are only subsequent tokens on lines
        #       and 'keywords' only happen at the beginning except
        #       for :au ones
        for index, token, value in \
            RegexLexer.get_tokens_unprocessed(self, text):
            if token is Name.Other:
                if self.is_in(value, self._cmd):
                    yield index, Keyword, value
                elif self.is_in(value, self._opt) or \
                     self.is_in(value, self._aut):
                    yield index, Name.Builtin, value
                else:
                    yield index, Text, value
            else:
                yield index, token, value


class GettextLexer(RegexLexer):
    """
    Lexer for Gettext catalog files.

    *New in Pygments 0.9.*
    """
    name = 'Gettext Catalog'
    aliases = ['pot', 'po']
    filenames = ['*.pot', '*.po']
    mimetypes = ['application/x-gettext', 'text/x-gettext', 'text/gettext']

    tokens = {
        'root': [
            (r'^#,\s.*?$', Keyword.Type),
            (r'^#:\s.*?$', Keyword.Declaration),
            #(r'^#$', Comment),
            (r'^(#|#\.\s|#\|\s|#~\s|#\s).*$', Comment.Single),
            (r'^(")([A-Za-z-]+:)(.*")$',
             bygroups(String, Name.Property, String)),
            (r'^".*"$', String),
            (r'^(msgid|msgid_plural|msgstr)(\s+)(".*")$',
             bygroups(Name.Variable, Text, String)),
            (r'^(msgstr\[)(\d)(\])(\s+)(".*")$',
             bygroups(Name.Variable, Number.Integer, Name.Variable, Text, String)),
        ]
    }


class SquidConfLexer(RegexLexer):
    """
    Lexer for `squid <http://www.squid-cache.org/>`_ configuration files.

    *New in Pygments 0.9.*
    """

    name = 'SquidConf'
    aliases = ['squidconf', 'squid.conf', 'squid']
    filenames = ['squid.conf']
    mimetypes = ['text/x-squidconf']
    flags = re.IGNORECASE

    keywords = [
        "access_log", "acl", "always_direct", "announce_host",
        "announce_period", "announce_port", "announce_to", "anonymize_headers",
        "append_domain", "as_whois_server", "auth_param_basic",
        "authenticate_children", "authenticate_program", "authenticate_ttl",
        "broken_posts", "buffered_logs", "cache_access_log", "cache_announce",
        "cache_dir", "cache_dns_program", "cache_effective_group",
        "cache_effective_user", "cache_host", "cache_host_acl",
        "cache_host_domain", "cache_log", "cache_mem", "cache_mem_high",
        "cache_mem_low", "cache_mgr", "cachemgr_passwd", "cache_peer",
        "cache_peer_access", "cahce_replacement_policy", "cache_stoplist",
        "cache_stoplist_pattern", "cache_store_log", "cache_swap",
        "cache_swap_high", "cache_swap_log", "cache_swap_low", "client_db",
        "client_lifetime", "client_netmask", "connect_timeout", "coredump_dir",
        "dead_peer_timeout", "debug_options", "delay_access", "delay_class",
        "delay_initial_bucket_level", "delay_parameters", "delay_pools",
        "deny_info", "dns_children", "dns_defnames", "dns_nameservers",
        "dns_testnames", "emulate_httpd_log", "err_html_text",
        "fake_user_agent", "firewall_ip", "forwarded_for", "forward_snmpd_port",
        "fqdncache_size", "ftpget_options", "ftpget_program", "ftp_list_width",
        "ftp_passive", "ftp_user", "half_closed_clients", "header_access",
        "header_replace", "hierarchy_stoplist", "high_response_time_warning",
        "high_page_fault_warning", "hosts_file", "htcp_port", "http_access",
        "http_anonymizer", "httpd_accel", "httpd_accel_host",
        "httpd_accel_port", "httpd_accel_uses_host_header",
        "httpd_accel_with_proxy", "http_port", "http_reply_access",
        "icp_access", "icp_hit_stale", "icp_port", "icp_query_timeout",
        "ident_lookup", "ident_lookup_access", "ident_timeout",
        "incoming_http_average", "incoming_icp_average", "inside_firewall",
        "ipcache_high", "ipcache_low", "ipcache_size", "local_domain",
        "local_ip", "logfile_rotate", "log_fqdn", "log_icp_queries",
        "log_mime_hdrs", "maximum_object_size", "maximum_single_addr_tries",
        "mcast_groups", "mcast_icp_query_timeout", "mcast_miss_addr",
        "mcast_miss_encode_key", "mcast_miss_port", "memory_pools",
        "memory_pools_limit", "memory_replacement_policy", "mime_table",
        "min_http_poll_cnt", "min_icp_poll_cnt", "minimum_direct_hops",
        "minimum_object_size", "minimum_retry_timeout", "miss_access",
        "negative_dns_ttl", "negative_ttl", "neighbor_timeout",
        "neighbor_type_domain", "netdb_high", "netdb_low", "netdb_ping_period",
        "netdb_ping_rate", "never_direct", "no_cache", "passthrough_proxy",
        "pconn_timeout", "pid_filename", "pinger_program", "positive_dns_ttl",
        "prefer_direct", "proxy_auth", "proxy_auth_realm", "query_icmp",
        "quick_abort", "quick_abort", "quick_abort_max", "quick_abort_min",
        "quick_abort_pct", "range_offset_limit", "read_timeout",
        "redirect_children", "redirect_program",
        "redirect_rewrites_host_header", "reference_age", "reference_age",
        "refresh_pattern", "reload_into_ims", "request_body_max_size",
        "request_size", "request_timeout", "shutdown_lifetime",
        "single_parent_bypass", "siteselect_timeout", "snmp_access",
        "snmp_incoming_address", "snmp_port", "source_ping", "ssl_proxy",
        "store_avg_object_size", "store_objects_per_bucket",
        "strip_query_terms", "swap_level1_dirs", "swap_level2_dirs",
        "tcp_incoming_address", "tcp_outgoing_address", "tcp_recv_bufsize",
        "test_reachability", "udp_hit_obj", "udp_hit_obj_size",
        "udp_incoming_address", "udp_outgoing_address", "unique_hostname",
        "unlinkd_program", "uri_whitespace", "useragent_log",
        "visible_hostname", "wais_relay", "wais_relay_host", "wais_relay_port",
    ]

    opts = [
        "proxy-only", "weight", "ttl", "no-query", "default", "round-robin",
        "multicast-responder", "on", "off", "all", "deny", "allow", "via",
        "parent", "no-digest", "heap", "lru", "realm", "children", "q1", "q2",
        "credentialsttl", "none", "disable", "offline_toggle", "diskd",
    ]

    actions = [
        "shutdown", "info", "parameter", "server_list", "client_list",
        r'squid\.conf',
    ]

    actions_stats = [
        "objects", "vm_objects", "utilization", "ipcache", "fqdncache", "dns",
        "redirector", "io", "reply_headers", "filedescriptors", "netdb",
    ]

    actions_log = ["status", "enable", "disable", "clear"]

    acls = [
        "url_regex", "urlpath_regex", "referer_regex", "port", "proto",
        "req_mime_type", "rep_mime_type", "method", "browser", "user", "src",
        "dst", "time", "dstdomain", "ident", "snmp_community",
    ]

    ip_re = (
        r'(?:(?:(?:[3-9]\d?|2(?:5[0-5]|[0-4]?\d)?|1\d{0,2}|0x0*[0-9a-f]{1,2}|'
        r'0+[1-3]?[0-7]{0,2})(?:\.(?:[3-9]\d?|2(?:5[0-5]|[0-4]?\d)?|1\d{0,2}|'
        r'0x0*[0-9a-f]{1,2}|0+[1-3]?[0-7]{0,2})){3})|(?!.*::.*::)(?:(?!:)|'
        r':(?=:))(?:[0-9a-f]{0,4}(?:(?<=::)|(?<!::):)){6}(?:[0-9a-f]{0,4}'
        r'(?:(?<=::)|(?<!::):)[0-9a-f]{0,4}(?:(?<=::)|(?<!:)|(?<=:)(?<!::):)|'
        r'(?:25[0-4]|2[0-4]\d|1\d\d|[1-9]?\d)(?:\.(?:25[0-4]|2[0-4]\d|1\d\d|'
        r'[1-9]?\d)){3}))'
    )

    def makelistre(list):
        return r'\b(?:' + '|'.join(list) + r')\b'

    tokens = {
        'root': [
            (r'\s+', Whitespace),
            (r'#', Comment, 'comment'),
            (makelistre(keywords), Keyword),
            (makelistre(opts), Name.Constant),
            # Actions
            (makelistre(actions), String),
            (r'stats/'+makelistre(actions), String),
            (r'log/'+makelistre(actions)+r'=', String),
            (makelistre(acls), Keyword),
            (ip_re + r'(?:/(?:' + ip_re + r'|\b\d+\b))?', Number.Float),
            (r'(?:\b\d+\b(?:-\b\d+|%)?)', Number),
            (r'\S+', Text),
        ],
        'comment': [
            (r'\s*TAG:.*', String.Escape, '#pop'),
            (r'.*', Comment, '#pop'),
        ],
    }


class DebianControlLexer(RegexLexer):
    """
    Lexer for Debian ``control`` files and ``apt-cache show <pkg>`` outputs.

    *New in Pygments 0.9.*
    """
    name = 'Debian Control file'
    aliases = ['control']
    filenames = ['control']

    tokens = {
        'root': [
            (r'^(Description)', Keyword, 'description'),
            (r'^(Maintainer)(:\s*)', bygroups(Keyword, Text), 'maintainer'),
            (r'^((Build-)?Depends)', Keyword, 'depends'),
            (r'^((?:Python-)?Version)(:\s*)(\S+)$',
             bygroups(Keyword, Text, Number)),
            (r'^((?:Installed-)?Size)(:\s*)(\S+)$',
             bygroups(Keyword, Text, Number)),
            (r'^(MD5Sum|SHA1|SHA256)(:\s*)(\S+)$',
             bygroups(Keyword, Text, Number)),
            (r'^([a-zA-Z\-0-9\.]*?)(:\s*)(.*?)$',
             bygroups(Keyword, Whitespace, String)),
        ],
        'maintainer': [
            (r'<[^>]+>', Generic.Strong),
            (r'<[^>]+>$', Generic.Strong, '#pop'),
            (r',\n?', Text),
            (r'.', Text),
        ],
        'description': [
            (r'(.*)(Homepage)(: )(\S+)',
             bygroups(Text, String, Name, Name.Class)),
            (r':.*\n', Generic.Strong),
            (r' .*\n', Text),
            ('', Text, '#pop'),
        ],
        'depends': [
            (r':\s*', Text),
            (r'(\$)(\{)(\w+\s*:\s*\w+)', bygroups(Operator, Text, Name.Entity)),
            (r'\(', Text, 'depend_vers'),
            (r',', Text),
            (r'\|', Operator),
            (r'[\s]+', Text),
            (r'[}\)]\s*$', Text, '#pop'),
            (r'}', Text),
            (r'[^,]$', Name.Function, '#pop'),
            (r'([\+\.a-zA-Z0-9-])(\s*)', bygroups(Name.Function, Text)),
            (r'\[.*?\]', Name.Entity),
        ],
        'depend_vers': [
            (r'\),', Text, '#pop'),
            (r'\)[^,]', Text, '#pop:2'),
            (r'([><=]+)(\s*)([^\)]+)', bygroups(Operator, Text, Number))
        ]
    }


class YamlLexerContext(LexerContext):
    """Indentation context for the YAML lexer."""

    def __init__(self, *args, **kwds):
        super(YamlLexerContext, self).__init__(*args, **kwds)
        self.indent_stack = []
        self.indent = -1
        self.next_indent = 0
        self.block_scalar_indent = None


class YamlLexer(ExtendedRegexLexer):
    """
    Lexer for `YAML <http://yaml.org/>`_, a human-friendly data serialization
    language.

    *New in Pygments 0.11.*
    """

    name = 'YAML'
    aliases = ['yaml']
    filenames = ['*.yaml', '*.yml']
    mimetypes = ['text/x-yaml']


    def something(token_class):
        """Do not produce empty tokens."""
        def callback(lexer, match, context):
            text = match.group()
            if not text:
                return
            yield match.start(), token_class, text
            context.pos = match.end()
        return callback

    def reset_indent(token_class):
        """Reset the indentation levels."""
        def callback(lexer, match, context):
            text = match.group()
            context.indent_stack = []
            context.indent = -1
            context.next_indent = 0
            context.block_scalar_indent = None
            yield match.start(), token_class, text
            context.pos = match.end()
        return callback

    def save_indent(token_class, start=False):
        """Save a possible indentation level."""
        def callback(lexer, match, context):
            text = match.group()
            extra = ''
            if start:
                context.next_indent = len(text)
                if context.next_indent < context.indent:
                    while context.next_indent < context.indent:
                        context.indent = context.indent_stack.pop()
                    if context.next_indent > context.indent:
                        extra = text[context.indent:]
                        text = text[:context.indent]
            else:
                context.next_indent += len(text)
            if text:
                yield match.start(), token_class, text
            if extra:
                yield match.start()+len(text), token_class.Error, extra
            context.pos = match.end()
        return callback

    def set_indent(token_class, implicit=False):
        """Set the previously saved indentation level."""
        def callback(lexer, match, context):
            text = match.group()
            if context.indent < context.next_indent:
                context.indent_stack.append(context.indent)
                context.indent = context.next_indent
            if not implicit:
                context.next_indent += len(text)
            yield match.start(), token_class, text
            context.pos = match.end()
        return callback

    def set_block_scalar_indent(token_class):
        """Set an explicit indentation level for a block scalar."""
        def callback(lexer, match, context):
            text = match.group()
            context.block_scalar_indent = None
            if not text:
                return
            increment = match.group(1)
            if increment:
                current_indent = max(context.indent, 0)
                increment = int(increment)
                context.block_scalar_indent = current_indent + increment
            if text:
                yield match.start(), token_class, text
                context.pos = match.end()
        return callback

    def parse_block_scalar_empty_line(indent_token_class, content_token_class):
        """Process an empty line in a block scalar."""
        def callback(lexer, match, context):
            text = match.group()
            if (context.block_scalar_indent is None or
                    len(text) <= context.block_scalar_indent):
                if text:
                    yield match.start(), indent_token_class, text
            else:
                indentation = text[:context.block_scalar_indent]
                content = text[context.block_scalar_indent:]
                yield match.start(), indent_token_class, indentation
                yield (match.start()+context.block_scalar_indent,
                        content_token_class, content)
            context.pos = match.end()
        return callback

    def parse_block_scalar_indent(token_class):
        """Process indentation spaces in a block scalar."""
        def callback(lexer, match, context):
            text = match.group()
            if context.block_scalar_indent is None:
                if len(text) <= max(context.indent, 0):
                    context.stack.pop()
                    context.stack.pop()
                    return
                context.block_scalar_indent = len(text)
            else:
                if len(text) < context.block_scalar_indent:
                    context.stack.pop()
                    context.stack.pop()
                    return
            if text:
                yield match.start(), token_class, text
                context.pos = match.end()
        return callback

    def parse_plain_scalar_indent(token_class):
        """Process indentation spaces in a plain scalar."""
        def callback(lexer, match, context):
            text = match.group()
            if len(text) <= context.indent:
                context.stack.pop()
                context.stack.pop()
                return
            if text:
                yield match.start(), token_class, text
                context.pos = match.end()
        return callback



    tokens = {
        # the root rules
        'root': [
            # ignored whitespaces
            (r'[ ]+(?=#|$)', Text),
            # line breaks
            (r'\n+', Text),
            # a comment
            (r'#[^\n]*', Comment.Single),
            # the '%YAML' directive
            (r'^%YAML(?=[ ]|$)', reset_indent(Name.Tag), 'yaml-directive'),
            # the %TAG directive
            (r'^%TAG(?=[ ]|$)', reset_indent(Name.Tag), 'tag-directive'),
            # document start and document end indicators
            (r'^(?:---|\.\.\.)(?=[ ]|$)', reset_indent(Name.Namespace),
             'block-line'),
            # indentation spaces
            (r'[ ]*(?![ \t\n\r\f\v]|$)', save_indent(Text, start=True),
             ('block-line', 'indentation')),
        ],

        # trailing whitespaces after directives or a block scalar indicator
        'ignored-line': [
            # ignored whitespaces
            (r'[ ]+(?=#|$)', Text),
            # a comment
            (r'#[^\n]*', Comment.Single),
            # line break
            (r'\n', Text, '#pop:2'),
        ],

        # the %YAML directive
        'yaml-directive': [
            # the version number
            (r'([ ]+)([0-9]+\.[0-9]+)',
             bygroups(Text, Number), 'ignored-line'),
        ],

        # the %YAG directive
        'tag-directive': [
            # a tag handle and the corresponding prefix
            (r'([ ]+)(!|![0-9A-Za-z_-]*!)'
             r'([ ]+)(!|!?[0-9A-Za-z;/?:@&=+$,_.!~*\'()\[\]%-]+)',
             bygroups(Text, Keyword.Type, Text, Keyword.Type),
             'ignored-line'),
        ],

        # block scalar indicators and indentation spaces
        'indentation': [
            # trailing whitespaces are ignored
            (r'[ ]*$', something(Text), '#pop:2'),
            # whitespaces preceding block collection indicators
            (r'[ ]+(?=[?:-](?:[ ]|$))', save_indent(Text)),
            # block collection indicators
            (r'[?:-](?=[ ]|$)', set_indent(Punctuation.Indicator)),
            # the beginning a block line
            (r'[ ]*', save_indent(Text), '#pop'),
        ],

        # an indented line in the block context
        'block-line': [
            # the line end
            (r'[ ]*(?=#|$)', something(Text), '#pop'),
            # whitespaces separating tokens
            (r'[ ]+', Text),
            # tags, anchors and aliases,
            include('descriptors'),
            # block collections and scalars
            include('block-nodes'),
            # flow collections and quoted scalars
            include('flow-nodes'),
            # a plain scalar
            (r'(?=[^ \t\n\r\f\v?:,\[\]{}#&*!|>\'"%@`-]|[?:-][^ \t\n\r\f\v])',
             something(Name.Variable),
             'plain-scalar-in-block-context'),
        ],

        # tags, anchors, aliases
        'descriptors' : [
            # a full-form tag
            (r'!<[0-9A-Za-z;/?:@&=+$,_.!~*\'()\[\]%-]+>', Keyword.Type),
            # a tag in the form '!', '!suffix' or '!handle!suffix'
            (r'!(?:[0-9A-Za-z_-]+)?'
             r'(?:![0-9A-Za-z;/?:@&=+$,_.!~*\'()\[\]%-]+)?', Keyword.Type),
            # an anchor
            (r'&[0-9A-Za-z_-]+', Name.Label),
            # an alias
            (r'\*[0-9A-Za-z_-]+', Name.Variable),
        ],

        # block collections and scalars
        'block-nodes': [
            # implicit key
            (r':(?=[ ]|$)', set_indent(Punctuation.Indicator, implicit=True)),
            # literal and folded scalars
            (r'[|>]', Punctuation.Indicator,
             ('block-scalar-content', 'block-scalar-header')),
        ],

        # flow collections and quoted scalars
        'flow-nodes': [
            # a flow sequence
            (r'\[', Punctuation.Indicator, 'flow-sequence'),
            # a flow mapping
            (r'\{', Punctuation.Indicator, 'flow-mapping'),
            # a single-quoted scalar
            (r'\'', String, 'single-quoted-scalar'),
            # a double-quoted scalar
            (r'\"', String, 'double-quoted-scalar'),
        ],

        # the content of a flow collection
        'flow-collection': [
            # whitespaces
            (r'[ ]+', Text),
            # line breaks
            (r'\n+', Text),
            # a comment
            (r'#[^\n]*', Comment.Single),
            # simple indicators
            (r'[?:,]', Punctuation.Indicator),
            # tags, anchors and aliases
            include('descriptors'),
            # nested collections and quoted scalars
            include('flow-nodes'),
            # a plain scalar
            (r'(?=[^ \t\n\r\f\v?:,\[\]{}#&*!|>\'"%@`])',
             something(Name.Variable),
             'plain-scalar-in-flow-context'),
        ],

        # a flow sequence indicated by '[' and ']'
        'flow-sequence': [
            # include flow collection rules
            include('flow-collection'),
            # the closing indicator
            (r'\]', Punctuation.Indicator, '#pop'),
        ],

        # a flow mapping indicated by '{' and '}'
        'flow-mapping': [
            # include flow collection rules
            include('flow-collection'),
            # the closing indicator
            (r'\}', Punctuation.Indicator, '#pop'),
        ],

        # block scalar lines
        'block-scalar-content': [
            # line break
            (r'\n', Text),
            # empty line
            (r'^[ ]+$',
             parse_block_scalar_empty_line(Text, Name.Constant)),
            # indentation spaces (we may leave the state here)
            (r'^[ ]*', parse_block_scalar_indent(Text)),
            # line content
            (r'[^\n\r\f\v]+', Name.Constant),
        ],

        # the content of a literal or folded scalar
        'block-scalar-header': [
            # indentation indicator followed by chomping flag
            (r'([1-9])?[+-]?(?=[ ]|$)',
             set_block_scalar_indent(Punctuation.Indicator),
             'ignored-line'),
            # chomping flag followed by indentation indicator
            (r'[+-]?([1-9])?(?=[ ]|$)',
             set_block_scalar_indent(Punctuation.Indicator),
             'ignored-line'),
        ],

        # ignored and regular whitespaces in quoted scalars
        'quoted-scalar-whitespaces': [
            # leading and trailing whitespaces are ignored
            (r'^[ ]+', Text),
            (r'[ ]+$', Text),
            # line breaks are ignored
            (r'\n+', Text),
            # other whitespaces are a part of the value
            (r'[ ]+', Name.Variable),
        ],

        # single-quoted scalars
        'single-quoted-scalar': [
            # include whitespace and line break rules
            include('quoted-scalar-whitespaces'),
            # escaping of the quote character
            (r'\'\'', String.Escape),
            # regular non-whitespace characters
            (r'[^ \t\n\r\f\v\']+', String),
            # the closing quote
            (r'\'', String, '#pop'),
        ],

        # double-quoted scalars
        'double-quoted-scalar': [
            # include whitespace and line break rules
            include('quoted-scalar-whitespaces'),
            # escaping of special characters
            (r'\\[0abt\tn\nvfre "\\N_LP]', String),
            # escape codes
            (r'\\(?:x[0-9A-Fa-f]{2}|u[0-9A-Fa-f]{4}|U[0-9A-Fa-f]{8})',
             String.Escape),
            # regular non-whitespace characters
            (r'[^ \t\n\r\f\v\"\\]+', String),
            # the closing quote
            (r'"', String, '#pop'),
        ],

        # the beginning of a new line while scanning a plain scalar
        'plain-scalar-in-block-context-new-line': [
            # empty lines
            (r'^[ ]+$', Text),
            # line breaks
            (r'\n+', Text),
            # document start and document end indicators
            (r'^(?=---|\.\.\.)', something(Name.Namespace), '#pop:3'),
            # indentation spaces (we may leave the block line state here)
            (r'^[ ]*', parse_plain_scalar_indent(Text), '#pop'),
        ],

        # a plain scalar in the block context
        'plain-scalar-in-block-context': [
            # the scalar ends with the ':' indicator
            (r'[ ]*(?=:[ ]|:$)', something(Text), '#pop'),
            # the scalar ends with whitespaces followed by a comment
            (r'[ ]+(?=#)', Text, '#pop'),
            # trailing whitespaces are ignored
            (r'[ ]+$', Text),
            # line breaks are ignored
            (r'\n+', Text, 'plain-scalar-in-block-context-new-line'),
            # other whitespaces are a part of the value
            (r'[ ]+', Literal.Scalar.Plain),
            # regular non-whitespace characters
            (r'(?::(?![ \t\n\r\f\v])|[^ \t\n\r\f\v:])+', Literal.Scalar.Plain),
        ],

        # a plain scalar is the flow context
        'plain-scalar-in-flow-context': [
            # the scalar ends with an indicator character
            (r'[ ]*(?=[,:?\[\]{}])', something(Text), '#pop'),
            # the scalar ends with a comment
            (r'[ ]+(?=#)', Text, '#pop'),
            # leading and trailing whitespaces are ignored
            (r'^[ ]+', Text),
            (r'[ ]+$', Text),
            # line breaks are ignored
            (r'\n+', Text),
            # other whitespaces are a part of the value
            (r'[ ]+', Name.Variable),
            # regular non-whitespace characters
            (r'[^ \t\n\r\f\v,:?\[\]{}]+', Name.Variable),
        ],

    }

    def get_tokens_unprocessed(self, text=None, context=None):
        if context is None:
            context = YamlLexerContext(text, 0)
        return super(YamlLexer, self).get_tokens_unprocessed(text, context)


class LighttpdConfLexer(RegexLexer):
    """
    Lexer for `Lighttpd <http://lighttpd.net/>`_ configuration files.

    *New in Pygments 0.11.*
    """
    name = 'Lighttpd configuration file'
    aliases = ['lighty', 'lighttpd']
    filenames = []
    mimetypes = ['text/x-lighttpd-conf']

    tokens = {
        'root': [
            (r'#.*\n', Comment.Single),
            (r'/\S*', Name), # pathname
            (r'[a-zA-Z._-]+', Keyword),
            (r'\d+\.\d+\.\d+\.\d+(?:/\d+)?', Number),
            (r'[0-9]+', Number),
            (r'=>|=~|\+=|==|=|\+', Operator),
            (r'\$[A-Z]+', Name.Builtin),
            (r'[(){}\[\],]', Punctuation),
            (r'"([^"\\]*(?:\\.[^"\\]*)*)"', String.Double),
            (r'\s+', Text),
        ],

    }


class NginxConfLexer(RegexLexer):
    """
    Lexer for `Nginx <http://nginx.net/>`_ configuration files.

    *New in Pygments 0.11.*
    """
    name = 'Nginx configuration file'
    aliases = ['nginx']
    filenames = []
    mimetypes = ['text/x-nginx-conf']

    tokens = {
        'root': [
            (r'(include)(\s+)([^\s;]+)', bygroups(Keyword, Text, Name)),
            (r'[^\s;#]+', Keyword, 'stmt'),
            include('base'),
        ],
        'block': [
            (r'}', Punctuation, '#pop:2'),
            (r'[^\s;#]+', Keyword.Namespace, 'stmt'),
            include('base'),
        ],
        'stmt': [
            (r'{', Punctuation, 'block'),
            (r';', Punctuation, '#pop'),
            include('base'),
        ],
        'base': [
            (r'#.*\n', Comment.Single),
            (r'on|off', Name.Constant),
            (r'\$[^\s;#()]+', Name.Variable),
            (r'([a-z0-9.-]+)(:)([0-9]+)',
             bygroups(Name, Punctuation, Number.Integer)),
            (r'[a-z-]+/[a-z-+]+', String), # mimetype
            #(r'[a-zA-Z._-]+', Keyword),
            (r'[0-9]+[km]?\b', Number.Integer),
            (r'(~)(\s*)([^\s{]+)', bygroups(Punctuation, Text, String.Regex)),
            (r'[:=~]', Punctuation),
            (r'[^\s;#{}$]+', String), # catch all
            (r'/[^\s;#]*', Name), # pathname
            (r'\s+', Text),
            (r'[$;]', Text),  # leftover characters
        ],
    }


class CMakeLexer(RegexLexer):
    """
    Lexer for `CMake <http://cmake.org/Wiki/CMake>`_ files.

    *New in Pygments 1.2.*
    """
    name = 'CMake'
    aliases = ['cmake']
    filenames = ['*.cmake', 'CMakeLists.txt']
    mimetypes = ['text/x-cmake']

    tokens = {
        'root': [
            #(r'(ADD_CUSTOM_COMMAND|ADD_CUSTOM_TARGET|ADD_DEFINITIONS|'
            # r'ADD_DEPENDENCIES|ADD_EXECUTABLE|ADD_LIBRARY|ADD_SUBDIRECTORY|'
            # r'ADD_TEST|AUX_SOURCE_DIRECTORY|BUILD_COMMAND|BUILD_NAME|'
            # r'CMAKE_MINIMUM_REQUIRED|CONFIGURE_FILE|CREATE_TEST_SOURCELIST|'
            # r'ELSE|ELSEIF|ENABLE_LANGUAGE|ENABLE_TESTING|ENDFOREACH|'
            # r'ENDFUNCTION|ENDIF|ENDMACRO|ENDWHILE|EXEC_PROGRAM|'
            # r'EXECUTE_PROCESS|EXPORT_LIBRARY_DEPENDENCIES|FILE|FIND_FILE|'
            # r'FIND_LIBRARY|FIND_PACKAGE|FIND_PATH|FIND_PROGRAM|FLTK_WRAP_UI|'
            # r'FOREACH|FUNCTION|GET_CMAKE_PROPERTY|GET_DIRECTORY_PROPERTY|'
            # r'GET_FILENAME_COMPONENT|GET_SOURCE_FILE_PROPERTY|'
            # r'GET_TARGET_PROPERTY|GET_TEST_PROPERTY|IF|INCLUDE|'
            # r'INCLUDE_DIRECTORIES|INCLUDE_EXTERNAL_MSPROJECT|'
            # r'INCLUDE_REGULAR_EXPRESSION|INSTALL|INSTALL_FILES|'
            # r'INSTALL_PROGRAMS|INSTALL_TARGETS|LINK_DIRECTORIES|'
            # r'LINK_LIBRARIES|LIST|LOAD_CACHE|LOAD_COMMAND|MACRO|'
            # r'MAKE_DIRECTORY|MARK_AS_ADVANCED|MATH|MESSAGE|OPTION|'
            # r'OUTPUT_REQUIRED_FILES|PROJECT|QT_WRAP_CPP|QT_WRAP_UI|REMOVE|'
            # r'REMOVE_DEFINITIONS|SEPARATE_ARGUMENTS|SET|'
            # r'SET_DIRECTORY_PROPERTIES|SET_SOURCE_FILES_PROPERTIES|'
            # r'SET_TARGET_PROPERTIES|SET_TESTS_PROPERTIES|SITE_NAME|'
            # r'SOURCE_GROUP|STRING|SUBDIR_DEPENDS|SUBDIRS|'
            # r'TARGET_LINK_LIBRARIES|TRY_COMPILE|TRY_RUN|UNSET|'
            # r'USE_MANGLED_MESA|UTILITY_SOURCE|VARIABLE_REQUIRES|'
            # r'VTK_MAKE_INSTANTIATOR|VTK_WRAP_JAVA|VTK_WRAP_PYTHON|'
            # r'VTK_WRAP_TCL|WHILE|WRITE_FILE|'
            # r'COUNTARGS)\b', Name.Builtin, 'args'),
            (r'\b([A-Za-z_]+)([ \t]*)(\()', bygroups(Name.Builtin, Text,
                                                     Punctuation), 'args'),
            include('keywords'),
            include('ws')
        ],
        'args': [
            (r'\(', Punctuation, '#push'),
            (r'\)', Punctuation, '#pop'),
            (r'(\${)(.+?)(})', bygroups(Operator, Name.Variable, Operator)),
            (r'(?s)".*?"', String.Double),
            (r'\\\S+', String),
            (r'[^\)$"# \t\n]+', String),
            (r'\n', Text), # explicitly legal
            include('keywords'),
            include('ws')
        ],
        'string': [

        ],
        'keywords': [
            (r'\b(WIN32|UNIX|APPLE|CYGWIN|BORLAND|MINGW|MSVC|MSVC_IDE|MSVC60|'
             r'MSVC70|MSVC71|MSVC80|MSVC90)\b', Keyword),
        ],
        'ws': [
            (r'[ \t]+', Text),
            (r'#.+\n', Comment),
        ]
    }


class HttpLexer(RegexLexer):
    """
    Lexer for HTTP sessions.

    *New in Pygments 1.5.*
    """

    name = 'HTTP'
    aliases = ['http']

    flags = re.DOTALL

    def header_callback(self, match):
        if match.group(1).lower() == 'content-type':
            content_type = match.group(5).strip()
            if ';' in content_type:
                content_type = content_type[:content_type.find(';')].strip()
            self.content_type = content_type
        yield match.start(1), Name.Attribute, match.group(1)
        yield match.start(2), Text, match.group(2)
        yield match.start(3), Operator, match.group(3)
        yield match.start(4), Text, match.group(4)
        yield match.start(5), Literal, match.group(5)
        yield match.start(6), Text, match.group(6)

    def continuous_header_callback(self, match):
        yield match.start(1), Text, match.group(1)
        yield match.start(2), Literal, match.group(2)
        yield match.start(3), Text, match.group(3)

    def content_callback(self, match):
        content_type = getattr(self, 'content_type', None)
        content = match.group()
        offset = match.start()
        if content_type:
            from pygments.lexers import get_lexer_for_mimetype
            try:
                lexer = get_lexer_for_mimetype(content_type)
            except ClassNotFound:
                pass
            else:
                for idx, token, value in lexer.get_tokens_unprocessed(content):
                    yield offset + idx, token, value
                return
        yield offset, Text, content

    tokens = {
        'root': [
            (r'(GET|POST|PUT|DELETE|HEAD|OPTIONS|TRACE)( +)([^ ]+)( +)'
             r'(HTTPS?)(/)(1\.[01])(\r?\n|$)',
             bygroups(Name.Function, Text, Name.Namespace, Text,
                      Keyword.Reserved, Operator, Number, Text),
             'headers'),
            (r'(HTTPS?)(/)(1\.[01])( +)(\d{3})( +)([^\r\n]+)(\r?\n|$)',
             bygroups(Keyword.Reserved, Operator, Number, Text, Number,
                      Text, Name.Exception, Text),
             'headers'),
        ],
        'headers': [
            (r'([^\s:]+)( *)(:)( *)([^\r\n]+)(\r?\n|$)', header_callback),
            (r'([\t ]+)([^\r\n]+)(\r?\n|$)', continuous_header_callback),
            (r'\r?\n', Text, 'content')
        ],
        'content': [
            (r'.+', content_callback)
        ]
    }


class PyPyLogLexer(RegexLexer):
    """
    Lexer for PyPy log files.

    *New in Pygments 1.5.*
    """
    name = "PyPy Log"
    aliases = ["pypylog", "pypy"]
    filenames = ["*.pypylog"]
    mimetypes = ['application/x-pypylog']

    tokens = {
        "root": [
            (r"\[\w+\] {jit-log-.*?$", Keyword, "jit-log"),
            (r"\[\w+\] {jit-backend-counts$", Keyword, "jit-backend-counts"),
            include("extra-stuff"),
        ],
        "jit-log": [
            (r"\[\w+\] jit-log-.*?}$", Keyword, "#pop"),
            (r"^\+\d+: ", Comment),
            (r"--end of the loop--", Comment),
            (r"[ifp]\d+", Name),
            (r"ptr\d+", Name),
            (r"(\()(\w+(?:\.\w+)?)(\))",
             bygroups(Punctuation, Name.Builtin, Punctuation)),
            (r"[\[\]=,()]", Punctuation),
            (r"(\d+\.\d+|inf|-inf)", Number.Float),
            (r"-?\d+", Number.Integer),
            (r"'.*'", String),
            (r"(None|descr|ConstClass|ConstPtr|TargetToken)", Name),
            (r"<.*?>+", Name.Builtin),
            (r"(label|debug_merge_point|jump|finish)", Name.Class),
            (r"(int_add_ovf|int_add|int_sub_ovf|int_sub|int_mul_ovf|int_mul|"
             r"int_floordiv|int_mod|int_lshift|int_rshift|int_and|int_or|"
             r"int_xor|int_eq|int_ne|int_ge|int_gt|int_le|int_lt|int_is_zero|"
             r"int_is_true|"
             r"uint_floordiv|uint_ge|uint_lt|"
             r"float_add|float_sub|float_mul|float_truediv|float_neg|"
             r"float_eq|float_ne|float_ge|float_gt|float_le|float_lt|float_abs|"
             r"ptr_eq|ptr_ne|instance_ptr_eq|instance_ptr_ne|"
             r"cast_int_to_float|cast_float_to_int|"
             r"force_token|quasiimmut_field|same_as|virtual_ref_finish|"
             r"virtual_ref|mark_opaque_ptr|"
             r"call_may_force|call_assembler|call_loopinvariant|"
             r"call_release_gil|call_pure|call|"
             r"new_with_vtable|new_array|newstr|newunicode|new|"
             r"arraylen_gc|"
             r"getarrayitem_gc_pure|getarrayitem_gc|setarrayitem_gc|"
             r"getarrayitem_raw|setarrayitem_raw|getfield_gc_pure|"
             r"getfield_gc|getinteriorfield_gc|setinteriorfield_gc|"
             r"getfield_raw|setfield_gc|setfield_raw|"
             r"strgetitem|strsetitem|strlen|copystrcontent|"
             r"unicodegetitem|unicodesetitem|unicodelen|"
             r"guard_true|guard_false|guard_value|guard_isnull|"
             r"guard_nonnull_class|guard_nonnull|guard_class|guard_no_overflow|"
             r"guard_not_forced|guard_no_exception|guard_not_invalidated)",
             Name.Builtin),
            include("extra-stuff"),
        ],
        "jit-backend-counts": [
            (r"\[\w+\] jit-backend-counts}$", Keyword, "#pop"),
            (r":", Punctuation),
            (r"\d+", Number),
            include("extra-stuff"),
        ],
        "extra-stuff": [
            (r"\s+", Text),
            (r"#.*?$", Comment),
        ],
    }


class HxmlLexer(RegexLexer):
    """
    Lexer for `haXe build <http://haxe.org/doc/compiler>`_ files.

    *New in Pygments 1.6.*
    """
    name = 'Hxml'
    aliases = ['haxeml', 'hxml']
    filenames = ['*.hxml']

    tokens = {
        'root': [
            # Seperator
            (r'(--)(next)', bygroups(Punctuation, Generic.Heading)),
            # Compiler switches with one dash
            (r'(-)(prompt|debug|v)', bygroups(Punctuation, Keyword.Keyword)),
            # Compilerswitches with two dashes
            (r'(--)(neko-source|flash-strict|flash-use-stage|no-opt|no-traces|'
             r'no-inline|times|no-output)', bygroups(Punctuation, Keyword)),
            # Targets and other options that take an argument
            (r'(-)(cpp|js|neko|x|as3|swf9?|swf-lib|php|xml|main|lib|D|resource|'
             r'cp|cmd)( +)(.+)',
             bygroups(Punctuation, Keyword, Whitespace, String)),
            # Options that take only numerical arguments
            (r'(-)(swf-version)( +)(\d+)',
             bygroups(Punctuation, Keyword, Number.Integer)),
            # An Option that defines the size, the fps and the background
            # color of an flash movie
            (r'(-)(swf-header)( +)(\d+)(:)(\d+)(:)(\d+)(:)([A-Fa-f0-9]{6})',
             bygroups(Punctuation, Keyword, Whitespace, Number.Integer,
                      Punctuation, Number.Integer, Punctuation, Number.Integer,
                      Punctuation, Number.Hex)),
            # options with two dashes that takes arguments
            (r'(--)(js-namespace|php-front|php-lib|remap|gen-hx-classes)( +)'
             r'(.+)', bygroups(Punctuation, Keyword, Whitespace, String)),
            # Single line comment, multiline ones are not allowed.
            (r'#.*', Comment.Single)
        ]
    }
