# -*- coding: utf-8 -*-
"""
    pygments.lexers.shell
    ~~~~~~~~~~~~~~~~~~~~~

    Lexers for various shells.

    :copyright: Copyright 2006-2013 by the Pygments team, see AUTHORS.
    :license: BSD, see LICENSE for details.
"""

import re

from pygments.lexer import Lexer, RegexLexer, do_insertions, bygroups, include
from pygments.token import Punctuation, \
     Text, Comment, Operator, Keyword, Name, String, Number, Generic
from pygments.util import shebang_matches


__all__ = ['BashLexer', 'BashSessionLexer', 'TcshLexer', 'BatchLexer',
           'PowerShellLexer', 'ShellSessionLexer']

line_re  = re.compile('.*?\n')


class BashLexer(RegexLexer):
    """
    Lexer for (ba|k|)sh shell scripts.

    *New in Pygments 0.6.*
    """

    name = 'Bash'
    aliases = ['bash', 'sh', 'ksh']
    filenames = ['*.sh', '*.ksh', '*.bash', '*.ebuild', '*.eclass',
                 '.bashrc', 'bashrc', '.bash_*', 'bash_*']
    mimetypes = ['application/x-sh', 'application/x-shellscript']

    tokens = {
        'root': [
            include('basic'),
            (r'\$\(\(', Keyword, 'math'),
            (r'\$\(', Keyword, 'paren'),
            (r'\${#?', Keyword, 'curly'),
            (r'`', String.Backtick, 'backticks'),
            include('data'),
        ],
        'basic': [
            (r'\b(if|fi|else|while|do|done|for|then|return|function|case|'
             r'select|continue|until|esac|elif)\s*\b',
             Keyword),
            (r'\b(alias|bg|bind|break|builtin|caller|cd|command|compgen|'
             r'complete|declare|dirs|disown|echo|enable|eval|exec|exit|'
             r'export|false|fc|fg|getopts|hash|help|history|jobs|kill|let|'
             r'local|logout|popd|printf|pushd|pwd|read|readonly|set|shift|'
             r'shopt|source|suspend|test|time|times|trap|true|type|typeset|'
             r'ulimit|umask|unalias|unset|wait)\s*\b(?!\.)',
             Name.Builtin),
            (r'#.*\n', Comment),
            (r'\\[\w\W]', String.Escape),
            (r'(\b\w+)(\s*)(=)', bygroups(Name.Variable, Text, Operator)),
            (r'[\[\]{}()=]', Operator),
            (r'<<<', Operator),  # here-string
            (r'<<-?\s*(\'?)\\?(\w+)[\w\W]+?\2', String),
            (r'&&|\|\|', Operator),
        ],
        'data': [
            (r'(?s)\$?"(\\\\|\\[0-7]+|\\.|[^"\\])*"', String.Double),
            (r"(?s)\$?'(\\\\|\\[0-7]+|\\.|[^'\\])*'", String.Single),
            (r';', Text),
            (r'\s+', Text),
            (r'[^=\s\[\]{}()$"\'`\\<]+', Text),
            (r'\d+(?= |\Z)', Number),
            (r'\$#?(\w+|.)', Name.Variable),
            (r'<', Text),
        ],
        'curly': [
            (r'}', Keyword, '#pop'),
            (r':-', Keyword),
            (r'[a-zA-Z0-9_]+', Name.Variable),
            (r'[^}:"\'`$]+', Punctuation),
            (r':', Punctuation),
            include('root'),
        ],
        'paren': [
            (r'\)', Keyword, '#pop'),
            include('root'),
        ],
        'math': [
            (r'\)\)', Keyword, '#pop'),
            (r'[-+*/%^|&]|\*\*|\|\|', Operator),
            (r'\d+', Number),
            include('root'),
        ],
        'backticks': [
            (r'`', String.Backtick, '#pop'),
            include('root'),
        ],
    }

    def analyse_text(text):
        return shebang_matches(text, r'(ba|z|)sh')


class BashSessionLexer(Lexer):
    """
    Lexer for simplistic shell sessions.

    *New in Pygments 1.1.*
    """

    name = 'Bash Session'
    aliases = ['console']
    filenames = ['*.sh-session']
    mimetypes = ['application/x-shell-session']

    def get_tokens_unprocessed(self, text):
        bashlexer = BashLexer(**self.options)

        pos = 0
        curcode = ''
        insertions = []

        for match in line_re.finditer(text):
            line = match.group()
            m = re.match(r'^((?:\(\S+\))?(?:|sh\S*?|\w+\S+[@:]\S+(?:\s+\S+)'
                          r'?|\[\S+[@:][^\n]+\].+)[$#%])(.*\n?)' , line)
            if m:
                # To support output lexers (say diff output), the output
                # needs to be broken by prompts whenever the output lexer
                # changes.
                if not insertions:
                    pos = match.start()

                insertions.append((len(curcode),
                                   [(0, Generic.Prompt, m.group(1))]))
                curcode += m.group(2)
            elif line.startswith('>'):
                insertions.append((len(curcode),
                                   [(0, Generic.Prompt, line[:1])]))
                curcode += line[1:]
            else:
                if insertions:
                    toks = bashlexer.get_tokens_unprocessed(curcode)
                    for i, t, v in do_insertions(insertions, toks):
                        yield pos+i, t, v
                yield match.start(), Generic.Output, line
                insertions = []
                curcode = ''
        if insertions:
            for i, t, v in do_insertions(insertions,
                                         bashlexer.get_tokens_unprocessed(curcode)):
                yield pos+i, t, v


class ShellSessionLexer(Lexer):
    """
    Lexer for shell sessions that works with different command prompts

    *New in Pygments 1.6.*
    """

    name = 'Shell Session'
    aliases = ['shell-session']
    filenames = ['*.shell-session']
    mimetypes = ['application/x-sh-session']

    def get_tokens_unprocessed(self, text):
        bashlexer = BashLexer(**self.options)

        pos = 0
        curcode = ''
        insertions = []

        for match in line_re.finditer(text):
            line = match.group()
            m = re.match(r'^((?:\[?\S+@[^$#%]+)[$#%])(.*\n?)', line)
            if m:
                # To support output lexers (say diff output), the output
                # needs to be broken by prompts whenever the output lexer
                # changes.
                if not insertions:
                    pos = match.start()

                insertions.append((len(curcode),
                                   [(0, Generic.Prompt, m.group(1))]))
                curcode += m.group(2)
            else:
                if insertions:
                    toks = bashlexer.get_tokens_unprocessed(curcode)
                    for i, t, v in do_insertions(insertions, toks):
                        yield pos+i, t, v
                yield match.start(), Generic.Output, line
                insertions = []
                curcode = ''
        if insertions:
            for i, t, v in do_insertions(insertions,
                                         bashlexer.get_tokens_unprocessed(curcode)):
                yield pos+i, t, v


class BatchLexer(RegexLexer):
    """
    Lexer for the DOS/Windows Batch file format.

    *New in Pygments 0.7.*
    """
    name = 'Batchfile'
    aliases = ['bat']
    filenames = ['*.bat', '*.cmd']
    mimetypes = ['application/x-dos-batch']

    flags = re.MULTILINE | re.IGNORECASE

    tokens = {
        'root': [
            # Lines can start with @ to prevent echo
            (r'^\s*@', Punctuation),
            (r'^(\s*)(rem\s.*)$', bygroups(Text, Comment)),
            (r'".*?"', String.Double),
            (r"'.*?'", String.Single),
            # If made more specific, make sure you still allow expansions
            # like %~$VAR:zlt
            (r'%%?[~$:\w]+%?', Name.Variable),
            (r'::.*', Comment), # Technically :: only works at BOL
            (r'(set)(\s+)(\w+)', bygroups(Keyword, Text, Name.Variable)),
            (r'(call)(\s+)(:\w+)', bygroups(Keyword, Text, Name.Label)),
            (r'(goto)(\s+)(\w+)', bygroups(Keyword, Text, Name.Label)),
            (r'\b(set|call|echo|on|off|endlocal|for|do|goto|if|pause|'
             r'setlocal|shift|errorlevel|exist|defined|cmdextversion|'
             r'errorlevel|else|cd|md|del|deltree|cls|choice)\b', Keyword),
            (r'\b(equ|neq|lss|leq|gtr|geq)\b', Operator),
            include('basic'),
            (r'.', Text),
        ],
        'echo': [
            # Escapes only valid within echo args?
            (r'\^\^|\^<|\^>|\^\|', String.Escape),
            (r'\n', Text, '#pop'),
            include('basic'),
            (r'[^\'"^]+', Text),
        ],
        'basic': [
            (r'".*?"', String.Double),
            (r"'.*?'", String.Single),
            (r'`.*?`', String.Backtick),
            (r'-?\d+', Number),
            (r',', Punctuation),
            (r'=', Operator),
            (r'/\S+', Name),
            (r':\w+', Name.Label),
            (r'\w:\w+', Text),
            (r'([<>|])(\s*)(\w+)', bygroups(Punctuation, Text, Name)),
        ],
    }


class TcshLexer(RegexLexer):
    """
    Lexer for tcsh scripts.

    *New in Pygments 0.10.*
    """

    name = 'Tcsh'
    aliases = ['tcsh', 'csh']
    filenames = ['*.tcsh', '*.csh']
    mimetypes = ['application/x-csh']

    tokens = {
        'root': [
            include('basic'),
            (r'\$\(', Keyword, 'paren'),
            (r'\${#?', Keyword, 'curly'),
            (r'`', String.Backtick, 'backticks'),
            include('data'),
        ],
        'basic': [
            (r'\b(if|endif|else|while|then|foreach|case|default|'
             r'continue|goto|breaksw|end|switch|endsw)\s*\b',
             Keyword),
            (r'\b(alias|alloc|bg|bindkey|break|builtins|bye|caller|cd|chdir|'
             r'complete|dirs|echo|echotc|eval|exec|exit|fg|filetest|getxvers|'
             r'glob|getspath|hashstat|history|hup|inlib|jobs|kill|'
             r'limit|log|login|logout|ls-F|migrate|newgrp|nice|nohup|notify|'
             r'onintr|popd|printenv|pushd|rehash|repeat|rootnode|popd|pushd|'
             r'set|shift|sched|setenv|setpath|settc|setty|setxvers|shift|'
             r'source|stop|suspend|source|suspend|telltc|time|'
             r'umask|unalias|uncomplete|unhash|universe|unlimit|unset|unsetenv|'
             r'ver|wait|warp|watchlog|where|which)\s*\b',
             Name.Builtin),
            (r'#.*\n', Comment),
            (r'\\[\w\W]', String.Escape),
            (r'(\b\w+)(\s*)(=)', bygroups(Name.Variable, Text, Operator)),
            (r'[\[\]{}()=]+', Operator),
            (r'<<\s*(\'?)\\?(\w+)[\w\W]+?\2', String),
        ],
        'data': [
            (r'(?s)"(\\\\|\\[0-7]+|\\.|[^"\\])*"', String.Double),
            (r"(?s)'(\\\\|\\[0-7]+|\\.|[^'\\])*'", String.Single),
            (r'\s+', Text),
            (r'[^=\s\[\]{}()$"\'`\\]+', Text),
            (r'\d+(?= |\Z)', Number),
            (r'\$#?(\w+|.)', Name.Variable),
        ],
        'curly': [
            (r'}', Keyword, '#pop'),
            (r':-', Keyword),
            (r'[a-zA-Z0-9_]+', Name.Variable),
            (r'[^}:"\'`$]+', Punctuation),
            (r':', Punctuation),
            include('root'),
        ],
        'paren': [
            (r'\)', Keyword, '#pop'),
            include('root'),
        ],
        'backticks': [
            (r'`', String.Backtick, '#pop'),
            include('root'),
        ],
    }


class PowerShellLexer(RegexLexer):
    """
    For Windows PowerShell code.

    *New in Pygments 1.5.*
    """
    name = 'PowerShell'
    aliases = ['powershell', 'posh', 'ps1']
    filenames = ['*.ps1']
    mimetypes = ['text/x-powershell']

    flags = re.DOTALL | re.IGNORECASE | re.MULTILINE

    keywords = (
        'while validateset validaterange validatepattern validatelength '
        'validatecount until trap switch return ref process param parameter in '
        'if global: function foreach for finally filter end elseif else '
        'dynamicparam do default continue cmdletbinding break begin alias \\? '
        '% #script #private #local #global mandatory parametersetname position '
        'valuefrompipeline valuefrompipelinebypropertyname '
        'valuefromremainingarguments helpmessage try catch').split()

    operators = (
        'and as band bnot bor bxor casesensitive ccontains ceq cge cgt cle '
        'clike clt cmatch cne cnotcontains cnotlike cnotmatch contains '
        'creplace eq exact f file ge gt icontains ieq ige igt ile ilike ilt '
        'imatch ine inotcontains inotlike inotmatch ireplace is isnot le like '
        'lt match ne not notcontains notlike notmatch or regex replace '
        'wildcard').split()

    verbs = (
        'write where wait use update unregister undo trace test tee take '
        'suspend stop start split sort skip show set send select scroll resume '
        'restore restart resolve resize reset rename remove register receive '
        'read push pop ping out new move measure limit join invoke import '
        'group get format foreach export expand exit enter enable disconnect '
        'disable debug cxnew copy convertto convertfrom convert connect '
        'complete compare clear checkpoint aggregate add').split()

    commenthelp = (
        'component description example externalhelp forwardhelpcategory '
        'forwardhelptargetname functionality inputs link '
        'notes outputs parameter remotehelprunspace role synopsis').split()

    tokens = {
        'root': [
            (r'\s+', Text),
            (r'^(\s*#[#\s]*)(\.(?:%s))([^\n]*$)' % '|'.join(commenthelp),
             bygroups(Comment, String.Doc, Comment)),
            (r'#[^\n]*?$', Comment),
            (r'(&lt;|<)#', Comment.Multiline, 'multline'),
            (r'@"\n.*?\n"@', String.Heredoc),
            (r"@'\n.*?\n'@", String.Heredoc),
            # escaped syntax
            (r'`[\'"$@-]', Punctuation),
            (r'"', String.Double, 'string'),
            (r"'([^']|'')*'", String.Single),
            (r'(\$|@@|@)((global|script|private|env):)?[a-z0-9_]+',
             Name.Variable),
            (r'(%s)\b' % '|'.join(keywords), Keyword),
            (r'-(%s)\b' % '|'.join(operators), Operator),
            (r'(%s)-[a-z_][a-z0-9_]*\b' % '|'.join(verbs), Name.Builtin),
            (r'\[[a-z_\[][a-z0-9_. `,\[\]]*\]', Name.Constant),  # .net [type]s
            (r'-[a-z_][a-z0-9_]*', Name),
            (r'\w+', Name),
            (r'[.,{}\[\]$()=+*/\\&%!~?^`|<>-]', Punctuation),
        ],
        'multline': [
            (r'[^#&.]+', Comment.Multiline),
            (r'#(>|&gt;)', Comment.Multiline, '#pop'),
            (r'\.(%s)' % '|'.join(commenthelp), String.Doc),
            (r'[#&.]', Comment.Multiline),
        ],
        'string': [
            (r'[^$`"]+', String.Double),
            (r'\$\(', String.Interpol, 'interpol'),
            (r'`"|""', String.Double),
            (r'[`$]', String.Double),
            (r'"', String.Double, '#pop'),
        ],
        'interpol': [
            (r'[^$)]+', String.Interpol),
            (r'\$\(', String.Interpol, '#push'),
            (r'\)', String.Interpol, '#pop'),
        ]
    }
