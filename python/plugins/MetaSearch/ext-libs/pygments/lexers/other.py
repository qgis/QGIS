# -*- coding: utf-8 -*-
"""
    pygments.lexers.other
    ~~~~~~~~~~~~~~~~~~~~~

    Lexers for other languages.

    :copyright: Copyright 2006-2013 by the Pygments team, see AUTHORS.
    :license: BSD, see LICENSE for details.
"""

import re

from pygments.lexer import RegexLexer, include, bygroups, using, \
     this, combined, ExtendedRegexLexer
from pygments.token import Error, Punctuation, Literal, Token, \
     Text, Comment, Operator, Keyword, Name, String, Number, Generic
from pygments.util import get_bool_opt
from pygments.lexers.web import HtmlLexer

from pygments.lexers._openedgebuiltins import OPENEDGEKEYWORDS
from pygments.lexers._robotframeworklexer import RobotFrameworkLexer

# backwards compatibility
from pygments.lexers.sql import SqlLexer, MySqlLexer, SqliteConsoleLexer
from pygments.lexers.shell import BashLexer, BashSessionLexer, BatchLexer, \
     TcshLexer

__all__ = ['BrainfuckLexer', 'BefungeLexer', 'RedcodeLexer', 'MOOCodeLexer',
           'SmalltalkLexer', 'LogtalkLexer', 'GnuplotLexer', 'PovrayLexer',
           'AppleScriptLexer', 'ModelicaLexer', 'RebolLexer', 'ABAPLexer',
           'NewspeakLexer', 'GherkinLexer', 'AsymptoteLexer', 'PostScriptLexer',
           'AutohotkeyLexer', 'GoodDataCLLexer', 'MaqlLexer', 'ProtoBufLexer',
           'HybrisLexer', 'AwkLexer', 'Cfengine3Lexer', 'SnobolLexer',
           'ECLLexer', 'UrbiscriptLexer', 'OpenEdgeLexer', 'BroLexer',
           'MscgenLexer', 'KconfigLexer', 'VGLLexer', 'SourcePawnLexer',
           'RobotFrameworkLexer', 'PuppetLexer', 'NSISLexer', 'RPMSpecLexer',
           'CbmBasicV2Lexer', 'AutoItLexer']


class ECLLexer(RegexLexer):
    """
    Lexer for the declarative big-data `ECL
    <http://hpccsystems.com/community/docs/ecl-language-reference/html>`_
    language.

    *New in Pygments 1.5.*
    """

    name = 'ECL'
    aliases = ['ecl']
    filenames = ['*.ecl']
    mimetypes = ['application/x-ecl']

    flags = re.IGNORECASE | re.MULTILINE

    tokens = {
        'root': [
            include('whitespace'),
            include('statements'),
        ],
        'whitespace': [
            (r'\s+', Text),
            (r'\/\/.*', Comment.Single),
            (r'/(\\\n)?\*(.|\n)*?\*(\\\n)?/', Comment.Multiline),
        ],
        'statements': [
            include('types'),
            include('keywords'),
            include('functions'),
            include('hash'),
            (r'"', String, 'string'),
            (r'\'', String, 'string'),
            (r'(\d+\.\d*|\.\d+|\d+)[eE][+-]?\d+[LlUu]*', Number.Float),
            (r'(\d+\.\d*|\.\d+|\d+[fF])[fF]?', Number.Float),
            (r'0x[0-9a-fA-F]+[LlUu]*', Number.Hex),
            (r'0[0-7]+[LlUu]*', Number.Oct),
            (r'\d+[LlUu]*', Number.Integer),
            (r'\*/', Error),
            (r'[~!%^&*+=|?:<>/-]+', Operator),
            (r'[{}()\[\],.;]', Punctuation),
            (r'[a-zA-Z_][a-zA-Z0-9_]*', Name),
        ],
        'hash': [
            (r'^#.*$', Comment.Preproc),
        ],
        'types': [
            (r'(RECORD|END)\D', Keyword.Declaration),
            (r'((?:ASCII|BIG_ENDIAN|BOOLEAN|DATA|DECIMAL|EBCDIC|INTEGER|PATTERN|'
             r'QSTRING|REAL|RECORD|RULE|SET OF|STRING|TOKEN|UDECIMAL|UNICODE|'
             r'UNSIGNED|VARSTRING|VARUNICODE)\d*)(\s+)',
             bygroups(Keyword.Type, Text)),
        ],
        'keywords': [
            (r'(APPLY|ASSERT|BUILD|BUILDINDEX|EVALUATE|FAIL|KEYDIFF|KEYPATCH|'
             r'LOADXML|NOTHOR|NOTIFY|OUTPUT|PARALLEL|SEQUENTIAL|SOAPCALL|WAIT'
             r'CHECKPOINT|DEPRECATED|FAILCODE|FAILMESSAGE|FAILURE|GLOBAL|'
             r'INDEPENDENT|ONWARNING|PERSIST|PRIORITY|RECOVERY|STORED|SUCCESS|'
             r'WAIT|WHEN)\b', Keyword.Reserved),
            # These are classed differently, check later
            (r'(ALL|AND|ANY|AS|ATMOST|BEFORE|BEGINC\+\+|BEST|BETWEEN|CASE|CONST|'
             r'COUNTER|CSV|DESCEND|ENCRYPT|ENDC\+\+|ENDMACRO|EXCEPT|EXCLUSIVE|'
             r'EXPIRE|EXPORT|EXTEND|FALSE|FEW|FIRST|FLAT|FULL|FUNCTION|GROUP|'
             r'HEADER|HEADING|HOLE|IFBLOCK|IMPORT|IN|JOINED|KEEP|KEYED|LAST|'
             r'LEFT|LIMIT|LOAD|LOCAL|LOCALE|LOOKUP|MACRO|MANY|MAXCOUNT|'
             r'MAXLENGTH|MIN SKEW|MODULE|INTERFACE|NAMED|NOCASE|NOROOT|NOSCAN|'
             r'NOSORT|NOT|OF|ONLY|OPT|OR|OUTER|OVERWRITE|PACKED|PARTITION|'
             r'PENALTY|PHYSICALLENGTH|PIPE|QUOTE|RELATIONSHIP|REPEAT|RETURN|'
             r'RIGHT|SCAN|SELF|SEPARATOR|SERVICE|SHARED|SKEW|SKIP|SQL|STORE|'
             r'TERMINATOR|THOR|THRESHOLD|TOKEN|TRANSFORM|TRIM|TRUE|TYPE|'
             r'UNICODEORDER|UNSORTED|VALIDATE|VIRTUAL|WHOLE|WILD|WITHIN|XML|'
             r'XPATH|__COMPRESSED__)\b', Keyword.Reserved),
        ],
        'functions': [
            (r'(ABS|ACOS|ALLNODES|ASCII|ASIN|ASSTRING|ATAN|ATAN2|AVE|CASE|'
             r'CHOOSE|CHOOSEN|CHOOSESETS|CLUSTERSIZE|COMBINE|CORRELATION|COS|'
             r'COSH|COUNT|COVARIANCE|CRON|DATASET|DEDUP|DEFINE|DENORMALIZE|'
             r'DISTRIBUTE|DISTRIBUTED|DISTRIBUTION|EBCDIC|ENTH|ERROR|EVALUATE|'
             r'EVENT|EVENTEXTRA|EVENTNAME|EXISTS|EXP|FAILCODE|FAILMESSAGE|'
             r'FETCH|FROMUNICODE|GETISVALID|GLOBAL|GRAPH|GROUP|HASH|HASH32|'
             r'HASH64|HASHCRC|HASHMD5|HAVING|IF|INDEX|INTFORMAT|ISVALID|'
             r'ITERATE|JOIN|KEYUNICODE|LENGTH|LIBRARY|LIMIT|LN|LOCAL|LOG|LOOP|'
             r'MAP|MATCHED|MATCHLENGTH|MATCHPOSITION|MATCHTEXT|MATCHUNICODE|'
             r'MAX|MERGE|MERGEJOIN|MIN|NOLOCAL|NONEMPTY|NORMALIZE|PARSE|PIPE|'
             r'POWER|PRELOAD|PROCESS|PROJECT|PULL|RANDOM|RANGE|RANK|RANKED|'
             r'REALFORMAT|RECORDOF|REGEXFIND|REGEXREPLACE|REGROUP|REJECTED|'
             r'ROLLUP|ROUND|ROUNDUP|ROW|ROWDIFF|SAMPLE|SET|SIN|SINH|SIZEOF|'
             r'SOAPCALL|SORT|SORTED|SQRT|STEPPED|STORED|SUM|TABLE|TAN|TANH|'
             r'THISNODE|TOPN|TOUNICODE|TRANSFER|TRIM|TRUNCATE|TYPEOF|UNGROUP|'
             r'UNICODEORDER|VARIANCE|WHICH|WORKUNIT|XMLDECODE|XMLENCODE|'
             r'XMLTEXT|XMLUNICODE)\b', Name.Function),
        ],
        'string': [
            (r'"', String, '#pop'),
            (r'\'', String, '#pop'),
            (r'[^"\']+', String),
        ],
    }


class BrainfuckLexer(RegexLexer):
    """
    Lexer for the esoteric `BrainFuck <http://www.muppetlabs.com/~breadbox/bf/>`_
    language.
    """

    name = 'Brainfuck'
    aliases = ['brainfuck', 'bf']
    filenames = ['*.bf', '*.b']
    mimetypes = ['application/x-brainfuck']

    tokens = {
        'common': [
            # use different colors for different instruction types
            (r'[.,]+', Name.Tag),
            (r'[+-]+', Name.Builtin),
            (r'[<>]+', Name.Variable),
            (r'[^.,+\-<>\[\]]+', Comment),
        ],
        'root': [
            (r'\[', Keyword, 'loop'),
            (r'\]', Error),
            include('common'),
        ],
        'loop': [
            (r'\[', Keyword, '#push'),
            (r'\]', Keyword, '#pop'),
            include('common'),
        ]
    }


class BefungeLexer(RegexLexer):
    """
    Lexer for the esoteric `Befunge <http://en.wikipedia.org/wiki/Befunge>`_
    language.

    *New in Pygments 0.7.*
    """
    name = 'Befunge'
    aliases = ['befunge']
    filenames = ['*.befunge']
    mimetypes = ['application/x-befunge']

    tokens = {
        'root': [
            (r'[0-9a-f]', Number),
            (r'[\+\*/%!`-]', Operator), # Traditional math
            (r'[<>^v?\[\]rxjk]', Name.Variable), # Move, imperatives
            (r'[:\\$.,n]', Name.Builtin), # Stack ops, imperatives
            (r'[|_mw]', Keyword),
            (r'[{}]', Name.Tag), # Befunge-98 stack ops
            (r'".*?"', String.Double), # Strings don't appear to allow escapes
            (r'\'.', String.Single), # Single character
            (r'[#;]', Comment), # Trampoline... depends on direction hit
            (r'[pg&~=@iotsy]', Keyword), # Misc
            (r'[()A-Z]', Comment), # Fingerprints
            (r'\s+', Text), # Whitespace doesn't matter
        ],
    }


class RedcodeLexer(RegexLexer):
    """
    A simple Redcode lexer based on ICWS'94.
    Contributed by Adam Blinkinsop <blinks@acm.org>.

    *New in Pygments 0.8.*
    """
    name = 'Redcode'
    aliases = ['redcode']
    filenames = ['*.cw']

    opcodes = ['DAT','MOV','ADD','SUB','MUL','DIV','MOD',
               'JMP','JMZ','JMN','DJN','CMP','SLT','SPL',
               'ORG','EQU','END']
    modifiers = ['A','B','AB','BA','F','X','I']

    tokens = {
        'root': [
            # Whitespace:
            (r'\s+', Text),
            (r';.*$', Comment.Single),
            # Lexemes:
            #  Identifiers
            (r'\b(%s)\b' % '|'.join(opcodes), Name.Function),
            (r'\b(%s)\b' % '|'.join(modifiers), Name.Decorator),
            (r'[A-Za-z_][A-Za-z_0-9]+', Name),
            #  Operators
            (r'[-+*/%]', Operator),
            (r'[#$@<>]', Operator), # mode
            (r'[.,]', Punctuation), # mode
            #  Numbers
            (r'[-+]?\d+', Number.Integer),
        ],
    }


class MOOCodeLexer(RegexLexer):
    """
    For `MOOCode <http://www.moo.mud.org/>`_ (the MOO scripting
    language).

    *New in Pygments 0.9.*
    """
    name = 'MOOCode'
    filenames = ['*.moo']
    aliases = ['moocode']
    mimetypes = ['text/x-moocode']

    tokens = {
        'root' : [
            # Numbers
            (r'(0|[1-9][0-9_]*)', Number.Integer),
            # Strings
            (r'"(\\\\|\\"|[^"])*"', String),
            # exceptions
            (r'(E_PERM|E_DIV)', Name.Exception),
            # db-refs
            (r'((#[-0-9]+)|(\$[a-z_A-Z0-9]+))', Name.Entity),
            # Keywords
            (r'\b(if|else|elseif|endif|for|endfor|fork|endfork|while'
             r'|endwhile|break|continue|return|try'
             r'|except|endtry|finally|in)\b', Keyword),
            # builtins
            (r'(random|length)', Name.Builtin),
            # special variables
            (r'(player|caller|this|args)', Name.Variable.Instance),
            # skip whitespace
            (r'\s+', Text),
            (r'\n', Text),
            # other operators
            (r'([!;=,{}&\|:\.\[\]@\(\)\<\>\?]+)', Operator),
            # function call
            (r'([a-z_A-Z0-9]+)(\()', bygroups(Name.Function, Operator)),
            # variables
            (r'([a-zA-Z_0-9]+)', Text),
        ]
    }


class SmalltalkLexer(RegexLexer):
    """
    For `Smalltalk <http://www.smalltalk.org/>`_ syntax.
    Contributed by Stefan Matthias Aust.
    Rewritten by Nils Winter.

    *New in Pygments 0.10.*
    """
    name = 'Smalltalk'
    filenames = ['*.st']
    aliases = ['smalltalk', 'squeak']
    mimetypes = ['text/x-smalltalk']

    tokens = {
        'root' : [
            (r'(<)(\w+:)(.*?)(>)', bygroups(Text, Keyword, Text, Text)),
            include('squeak fileout'),
            include('whitespaces'),
            include('method definition'),
            (r'(\|)([\w\s]*)(\|)', bygroups(Operator, Name.Variable, Operator)),
            include('objects'),
            (r'\^|\:=|\_', Operator),
            # temporaries
            (r'[\]({}.;!]', Text),
        ],
        'method definition' : [
            # Not perfect can't allow whitespaces at the beginning and the
            # without breaking everything
            (r'([a-zA-Z]+\w*:)(\s*)(\w+)',
             bygroups(Name.Function, Text, Name.Variable)),
            (r'^(\b[a-zA-Z]+\w*\b)(\s*)$', bygroups(Name.Function, Text)),
            (r'^([-+*/\\~<>=|&!?,@%]+)(\s*)(\w+)(\s*)$',
             bygroups(Name.Function, Text, Name.Variable, Text)),
        ],
        'blockvariables' : [
            include('whitespaces'),
            (r'(:)(\s*)(\w+)',
             bygroups(Operator, Text, Name.Variable)),
            (r'\|', Operator, '#pop'),
            (r'', Text, '#pop'), # else pop
        ],
        'literals' : [
            (r"'(''|[^'])*'", String, 'afterobject'),
            (r'\$.', String.Char, 'afterobject'),
            (r'#\(', String.Symbol, 'parenth'),
            (r'\)', Text, 'afterobject'),
            (r'(\d+r)?-?\d+(\.\d+)?(e-?\d+)?', Number, 'afterobject'),
        ],
        '_parenth_helper' : [
            include('whitespaces'),
            (r'(\d+r)?-?\d+(\.\d+)?(e-?\d+)?', Number),
            (r'[-+*/\\~<>=|&#!?,@%\w:]+', String.Symbol),
            # literals
            (r"'(''|[^'])*'", String),
            (r'\$.', String.Char),
            (r'#*\(', String.Symbol, 'inner_parenth'),
        ],
        'parenth' : [
            # This state is a bit tricky since
            # we can't just pop this state
            (r'\)', String.Symbol, ('root', 'afterobject')),
            include('_parenth_helper'),
        ],
        'inner_parenth': [
            (r'\)', String.Symbol, '#pop'),
            include('_parenth_helper'),
        ],
        'whitespaces' : [
            # skip whitespace and comments
            (r'\s+', Text),
            (r'"(""|[^"])*"', Comment),
        ],
        'objects' : [
            (r'\[', Text, 'blockvariables'),
            (r'\]', Text, 'afterobject'),
            (r'\b(self|super|true|false|nil|thisContext)\b',
             Name.Builtin.Pseudo, 'afterobject'),
            (r'\b[A-Z]\w*(?!:)\b', Name.Class, 'afterobject'),
            (r'\b[a-z]\w*(?!:)\b', Name.Variable, 'afterobject'),
            (r'#("(""|[^"])*"|[-+*/\\~<>=|&!?,@%]+|[\w:]+)',
             String.Symbol, 'afterobject'),
            include('literals'),
        ],
        'afterobject' : [
            (r'! !$', Keyword , '#pop'), # squeak chunk delimeter
            include('whitespaces'),
            (r'\b(ifTrue:|ifFalse:|whileTrue:|whileFalse:|timesRepeat:)',
             Name.Builtin, '#pop'),
            (r'\b(new\b(?!:))', Name.Builtin),
            (r'\:=|\_', Operator, '#pop'),
            (r'\b[a-zA-Z]+\w*:', Name.Function, '#pop'),
            (r'\b[a-zA-Z]+\w*', Name.Function),
            (r'\w+:?|[-+*/\\~<>=|&!?,@%]+', Name.Function, '#pop'),
            (r'\.', Punctuation, '#pop'),
            (r';', Punctuation),
            (r'[\])}]', Text),
            (r'[\[({]', Text, '#pop'),
        ],
        'squeak fileout' : [
            # Squeak fileout format (optional)
            (r'^"(""|[^"])*"!', Keyword),
            (r"^'(''|[^'])*'!", Keyword),
            (r'^(!)(\w+)( commentStamp: )(.*?)( prior: .*?!\n)(.*?)(!)',
                bygroups(Keyword, Name.Class, Keyword, String, Keyword, Text, Keyword)),
            (r"^(!)(\w+(?: class)?)( methodsFor: )('(?:''|[^'])*')(.*?!)",
                bygroups(Keyword, Name.Class, Keyword, String, Keyword)),
            (r'^(\w+)( subclass: )(#\w+)'
             r'(\s+instanceVariableNames: )(.*?)'
             r'(\s+classVariableNames: )(.*?)'
             r'(\s+poolDictionaries: )(.*?)'
             r'(\s+category: )(.*?)(!)',
                bygroups(Name.Class, Keyword, String.Symbol, Keyword, String, Keyword,
                         String, Keyword, String, Keyword, String, Keyword)),
            (r'^(\w+(?: class)?)(\s+instanceVariableNames: )(.*?)(!)',
                bygroups(Name.Class, Keyword, String, Keyword)),
            (r'(!\n)(\].*)(! !)$', bygroups(Keyword, Text, Keyword)),
            (r'! !$', Keyword),
        ],
    }


class LogtalkLexer(RegexLexer):
    """
    For `Logtalk <http://logtalk.org/>`_ source code.

    *New in Pygments 0.10.*
    """

    name = 'Logtalk'
    aliases = ['logtalk']
    filenames = ['*.lgt']
    mimetypes = ['text/x-logtalk']

    tokens = {
        'root': [
            # Directives
            (r'^\s*:-\s',Punctuation,'directive'),
            # Comments
            (r'%.*?\n', Comment),
            (r'/\*(.|\n)*?\*/',Comment),
            # Whitespace
            (r'\n', Text),
            (r'\s+', Text),
            # Numbers
            (r"0'.", Number),
            (r'0b[01]+', Number),
            (r'0o[0-7]+', Number),
            (r'0x[0-9a-fA-F]+', Number),
            (r'\d+\.?\d*((e|E)(\+|-)?\d+)?', Number),
            # Variables
            (r'([A-Z_][a-zA-Z0-9_]*)', Name.Variable),
            # Event handlers
            (r'(after|before)(?=[(])', Keyword),
            # Execution-context methods
            (r'(parameter|this|se(lf|nder))(?=[(])', Keyword),
            # Reflection
            (r'(current_predicate|predicate_property)(?=[(])', Keyword),
            # DCGs and term expansion
            (r'(expand_(goal|term)|(goal|term)_expansion|phrase)(?=[(])',
             Keyword),
            # Entity
            (r'(abolish|c(reate|urrent))_(object|protocol|category)(?=[(])',
             Keyword),
            (r'(object|protocol|category)_property(?=[(])', Keyword),
            # Entity relations
            (r'co(mplements_object|nforms_to_protocol)(?=[(])', Keyword),
            (r'extends_(object|protocol|category)(?=[(])', Keyword),
            (r'imp(lements_protocol|orts_category)(?=[(])', Keyword),
            (r'(instantiat|specializ)es_class(?=[(])', Keyword),
            # Events
            (r'(current_event|(abolish|define)_events)(?=[(])', Keyword),
            # Flags
            (r'(current|set)_logtalk_flag(?=[(])', Keyword),
            # Compiling, loading, and library paths
            (r'logtalk_(compile|l(ibrary_path|oad_context|oad))(?=[(])',
             Keyword),
            # Database
            (r'(clause|retract(all)?)(?=[(])', Keyword),
            (r'a(bolish|ssert(a|z))(?=[(])', Keyword),
            # Control constructs
            (r'(ca(ll|tch)|throw)(?=[(])', Keyword),
            (r'(fail|true)\b', Keyword),
            # All solutions
            (r'((bag|set)of|f(ind|or)all)(?=[(])', Keyword),
            # Multi-threading meta-predicates
            (r'threaded(_(call|once|ignore|exit|peek|wait|notify))?(?=[(])',
             Keyword),
            # Term unification
            (r'unify_with_occurs_check(?=[(])', Keyword),
            # Term creation and decomposition
            (r'(functor|arg|copy_term|numbervars)(?=[(])', Keyword),
            # Evaluable functors
            (r'(rem|mod|abs|sign)(?=[(])', Keyword),
            (r'float(_(integer|fractional)_part)?(?=[(])', Keyword),
            (r'(floor|truncate|round|ceiling)(?=[(])', Keyword),
            # Other arithmetic functors
            (r'(cos|atan|exp|log|s(in|qrt))(?=[(])', Keyword),
            # Term testing
            (r'(var|atom(ic)?|integer|float|c(allable|ompound)|n(onvar|umber)|'
             r'ground)(?=[(])', Keyword),
            # Term comparison
            (r'compare(?=[(])', Keyword),
            # Stream selection and control
            (r'(curren|se)t_(in|out)put(?=[(])', Keyword),
            (r'(open|close)(?=[(])', Keyword),
            (r'flush_output(?=[(])', Keyword),
            (r'(at_end_of_stream|flush_output)\b', Keyword),
            (r'(stream_property|at_end_of_stream|set_stream_position)(?=[(])',
             Keyword),
            # Character and byte input/output
            (r'(nl|(get|peek|put)_(byte|c(har|ode)))(?=[(])', Keyword),
            (r'\bnl\b', Keyword),
            # Term input/output
            (r'read(_term)?(?=[(])', Keyword),
            (r'write(q|_(canonical|term))?(?=[(])', Keyword),
            (r'(current_)?op(?=[(])', Keyword),
            (r'(current_)?char_conversion(?=[(])', Keyword),
            # Atomic term processing
            (r'atom_(length|c(hars|o(ncat|des)))(?=[(])', Keyword),
            (r'(char_code|sub_atom)(?=[(])', Keyword),
            (r'number_c(har|ode)s(?=[(])', Keyword),
            # Implementation defined hooks functions
            (r'(se|curren)t_prolog_flag(?=[(])', Keyword),
            (r'\bhalt\b', Keyword),
            (r'halt(?=[(])', Keyword),
            # Message sending operators
            (r'(::|:|\^\^)', Operator),
            # External call
            (r'[{}]', Keyword),
            # Logic and control
            (r'\b(ignore|once)(?=[(])', Keyword),
            (r'\brepeat\b', Keyword),
            # Sorting
            (r'(key)?sort(?=[(])', Keyword),
            # Bitwise functors
            (r'(>>|<<|/\\|\\\\|\\)', Operator),
            # Arithemtic evaluation
            (r'\bis\b', Keyword),
            # Arithemtic comparison
            (r'(=:=|=\\=|<|=<|>=|>)', Operator),
            # Term creation and decomposition
            (r'=\.\.', Operator),
            # Term unification
            (r'(=|\\=)', Operator),
            # Term comparison
            (r'(==|\\==|@=<|@<|@>=|@>)', Operator),
            # Evaluable functors
            (r'(//|[-+*/])', Operator),
            (r'\b(e|pi|mod|rem)\b', Operator),
            # Other arithemtic functors
            (r'\b\*\*\b', Operator),
            # DCG rules
            (r'-->', Operator),
            # Control constructs
            (r'([!;]|->)', Operator),
            # Logic and control
            (r'\\+', Operator),
            # Mode operators
            (r'[?@]', Operator),
            # Existential quantifier
            (r'\^', Operator),
            # Strings
            (r'"(\\\\|\\"|[^"])*"', String),
            # Ponctuation
            (r'[()\[\],.|]', Text),
            # Atoms
            (r"[a-z][a-zA-Z0-9_]*", Text),
            (r"'", String, 'quoted_atom'),
        ],

        'quoted_atom': [
            (r"''", String),
            (r"'", String, '#pop'),
            (r'\\([\\abfnrtv"\']|(x[a-fA-F0-9]+|[0-7]+)\\)', String.Escape),
            (r"[^\\'\n]+", String),
            (r'\\', String),
        ],

        'directive': [
            # Conditional compilation directives
            (r'(el)?if(?=[(])', Keyword, 'root'),
            (r'(e(lse|ndif))[.]', Keyword, 'root'),
            # Entity directives
            (r'(category|object|protocol)(?=[(])', Keyword, 'entityrelations'),
            (r'(end_(category|object|protocol))[.]',Keyword, 'root'),
            # Predicate scope directives
            (r'(public|protected|private)(?=[(])', Keyword, 'root'),
            # Other directives
            (r'e(n(coding|sure_loaded)|xport)(?=[(])', Keyword, 'root'),
            (r'in(fo|itialization)(?=[(])', Keyword, 'root'),
            (r'(dynamic|synchronized|threaded)[.]', Keyword, 'root'),
            (r'(alias|d(ynamic|iscontiguous)|m(eta_predicate|ode|ultifile)|'
             r's(et_(logtalk|prolog)_flag|ynchronized))(?=[(])',
             Keyword, 'root'),
            (r'op(?=[(])', Keyword, 'root'),
            (r'(c(alls|oinductive)|reexport|use(s|_module))(?=[(])',
             Keyword, 'root'),
            (r'[a-z][a-zA-Z0-9_]*(?=[(])', Text, 'root'),
            (r'[a-z][a-zA-Z0-9_]*[.]', Text, 'root'),
        ],

        'entityrelations': [
            (r'(complements|extends|i(nstantiates|mp(lements|orts))|specializes)'
             r'(?=[(])', Keyword),
            # Numbers
            (r"0'.", Number),
            (r'0b[01]+', Number),
            (r'0o[0-7]+', Number),
            (r'0x[0-9a-fA-F]+', Number),
            (r'\d+\.?\d*((e|E)(\+|-)?\d+)?', Number),
            # Variables
            (r'([A-Z_][a-zA-Z0-9_]*)', Name.Variable),
            # Atoms
            (r"[a-z][a-zA-Z0-9_]*", Text),
            (r"'", String, 'quoted_atom'),
            # Strings
            (r'"(\\\\|\\"|[^"])*"', String),
            # End of entity-opening directive
            (r'([)]\.)', Text, 'root'),
            # Scope operator
            (r'(::)', Operator),
            # Ponctuation
            (r'[()\[\],.|]', Text),
            # Comments
            (r'%.*?\n', Comment),
            (r'/\*(.|\n)*?\*/',Comment),
            # Whitespace
            (r'\n', Text),
            (r'\s+', Text),
        ]
    }

    def analyse_text(text):
        if ':- object(' in text:
            return True
        if ':- protocol(' in text:
            return True
        if ':- category(' in text:
            return True
        return False


def _shortened(word):
    dpos = word.find('$')
    return '|'.join([word[:dpos] + word[dpos+1:i] + r'\b'
                     for i in range(len(word), dpos, -1)])
def _shortened_many(*words):
    return '|'.join(map(_shortened, words))

class GnuplotLexer(RegexLexer):
    """
    For `Gnuplot <http://gnuplot.info/>`_ plotting scripts.

    *New in Pygments 0.11.*
    """

    name = 'Gnuplot'
    aliases = ['gnuplot']
    filenames = ['*.plot', '*.plt']
    mimetypes = ['text/x-gnuplot']

    tokens = {
        'root': [
            include('whitespace'),
            (_shortened('bi$nd'), Keyword, 'bind'),
            (_shortened_many('ex$it', 'q$uit'), Keyword, 'quit'),
            (_shortened('f$it'), Keyword, 'fit'),
            (r'(if)(\s*)(\()', bygroups(Keyword, Text, Punctuation), 'if'),
            (r'else\b', Keyword),
            (_shortened('pa$use'), Keyword, 'pause'),
            (_shortened_many('p$lot', 'rep$lot', 'sp$lot'), Keyword, 'plot'),
            (_shortened('sa$ve'), Keyword, 'save'),
            (_shortened('se$t'), Keyword, ('genericargs', 'optionarg')),
            (_shortened_many('sh$ow', 'uns$et'),
             Keyword, ('noargs', 'optionarg')),
            (_shortened_many('low$er', 'ra$ise', 'ca$ll', 'cd$', 'cl$ear',
                             'h$elp', '\\?$', 'hi$story', 'l$oad', 'pr$int',
                             'pwd$', 're$read', 'res$et', 'scr$eendump',
                             'she$ll', 'sy$stem', 'up$date'),
             Keyword, 'genericargs'),
            (_shortened_many('pwd$', 're$read', 'res$et', 'scr$eendump',
                             'she$ll', 'test$'),
             Keyword, 'noargs'),
            ('([a-zA-Z_][a-zA-Z0-9_]*)(\s*)(=)',
             bygroups(Name.Variable, Text, Operator), 'genericargs'),
            ('([a-zA-Z_][a-zA-Z0-9_]*)(\s*\(.*?\)\s*)(=)',
             bygroups(Name.Function, Text, Operator), 'genericargs'),
            (r'@[a-zA-Z_][a-zA-Z0-9_]*', Name.Constant), # macros
            (r';', Keyword),
        ],
        'comment': [
            (r'[^\\\n]', Comment),
            (r'\\\n', Comment),
            (r'\\', Comment),
            # don't add the newline to the Comment token
            ('', Comment, '#pop'),
        ],
        'whitespace': [
            ('#', Comment, 'comment'),
            (r'[ \t\v\f]+', Text),
        ],
        'noargs': [
            include('whitespace'),
            # semicolon and newline end the argument list
            (r';', Punctuation, '#pop'),
            (r'\n', Text, '#pop'),
        ],
        'dqstring': [
            (r'"', String, '#pop'),
            (r'\\([\\abfnrtv"\']|x[a-fA-F0-9]{2,4}|[0-7]{1,3})', String.Escape),
            (r'[^\\"\n]+', String), # all other characters
            (r'\\\n', String), # line continuation
            (r'\\', String), # stray backslash
            (r'\n', String, '#pop'), # newline ends the string too
        ],
        'sqstring': [
            (r"''", String), # escaped single quote
            (r"'", String, '#pop'),
            (r"[^\\'\n]+", String), # all other characters
            (r'\\\n', String), # line continuation
            (r'\\', String), # normal backslash
            (r'\n', String, '#pop'), # newline ends the string too
        ],
        'genericargs': [
            include('noargs'),
            (r'"', String, 'dqstring'),
            (r"'", String, 'sqstring'),
            (r'(\d+\.\d*|\.\d+|\d+)[eE][+-]?\d+', Number.Float),
            (r'(\d+\.\d*|\.\d+)', Number.Float),
            (r'-?\d+', Number.Integer),
            ('[,.~!%^&*+=|?:<>/-]', Operator),
            ('[{}()\[\]]', Punctuation),
            (r'(eq|ne)\b', Operator.Word),
            (r'([a-zA-Z_][a-zA-Z0-9_]*)(\s*)(\()',
             bygroups(Name.Function, Text, Punctuation)),
            (r'[a-zA-Z_][a-zA-Z0-9_]*', Name),
            (r'@[a-zA-Z_][a-zA-Z0-9_]*', Name.Constant), # macros
            (r'\\\n', Text),
        ],
        'optionarg': [
            include('whitespace'),
            (_shortened_many(
                "a$ll","an$gles","ar$row","au$toscale","b$ars","bor$der",
                "box$width","cl$abel","c$lip","cn$trparam","co$ntour","da$ta",
                "data$file","dg$rid3d","du$mmy","enc$oding","dec$imalsign",
                "fit$","font$path","fo$rmat","fu$nction","fu$nctions","g$rid",
                "hid$den3d","his$torysize","is$osamples","k$ey","keyt$itle",
                "la$bel","li$nestyle","ls$","loa$dpath","loc$ale","log$scale",
                "mac$ros","map$ping","map$ping3d","mar$gin","lmar$gin",
                "rmar$gin","tmar$gin","bmar$gin","mo$use","multi$plot",
                "mxt$ics","nomxt$ics","mx2t$ics","nomx2t$ics","myt$ics",
                "nomyt$ics","my2t$ics","nomy2t$ics","mzt$ics","nomzt$ics",
                "mcbt$ics","nomcbt$ics","of$fsets","or$igin","o$utput",
                "pa$rametric","pm$3d","pal$ette","colorb$ox","p$lot",
                "poi$ntsize","pol$ar","pr$int","obj$ect","sa$mples","si$ze",
                "st$yle","su$rface","table$","t$erminal","termo$ptions","ti$cs",
                "ticsc$ale","ticsl$evel","timef$mt","tim$estamp","tit$le",
                "v$ariables","ve$rsion","vi$ew","xyp$lane","xda$ta","x2da$ta",
                "yda$ta","y2da$ta","zda$ta","cbda$ta","xl$abel","x2l$abel",
                "yl$abel","y2l$abel","zl$abel","cbl$abel","xti$cs","noxti$cs",
                "x2ti$cs","nox2ti$cs","yti$cs","noyti$cs","y2ti$cs","noy2ti$cs",
                "zti$cs","nozti$cs","cbti$cs","nocbti$cs","xdti$cs","noxdti$cs",
                "x2dti$cs","nox2dti$cs","ydti$cs","noydti$cs","y2dti$cs",
                "noy2dti$cs","zdti$cs","nozdti$cs","cbdti$cs","nocbdti$cs",
                "xmti$cs","noxmti$cs","x2mti$cs","nox2mti$cs","ymti$cs",
                "noymti$cs","y2mti$cs","noy2mti$cs","zmti$cs","nozmti$cs",
                "cbmti$cs","nocbmti$cs","xr$ange","x2r$ange","yr$ange",
                "y2r$ange","zr$ange","cbr$ange","rr$ange","tr$ange","ur$ange",
                "vr$ange","xzeroa$xis","x2zeroa$xis","yzeroa$xis","y2zeroa$xis",
                "zzeroa$xis","zeroa$xis","z$ero"), Name.Builtin, '#pop'),
        ],
        'bind': [
            ('!', Keyword, '#pop'),
            (_shortened('all$windows'), Name.Builtin),
            include('genericargs'),
        ],
        'quit': [
            (r'gnuplot\b', Keyword),
            include('noargs'),
        ],
        'fit': [
            (r'via\b', Name.Builtin),
            include('plot'),
        ],
        'if': [
            (r'\)', Punctuation, '#pop'),
            include('genericargs'),
        ],
        'pause': [
            (r'(mouse|any|button1|button2|button3)\b', Name.Builtin),
            (_shortened('key$press'), Name.Builtin),
            include('genericargs'),
        ],
        'plot': [
            (_shortened_many('ax$es', 'axi$s', 'bin$ary', 'ev$ery', 'i$ndex',
                             'mat$rix', 's$mooth', 'thru$', 't$itle',
                             'not$itle', 'u$sing', 'w$ith'),
             Name.Builtin),
            include('genericargs'),
        ],
        'save': [
            (_shortened_many('f$unctions', 's$et', 't$erminal', 'v$ariables'),
             Name.Builtin),
            include('genericargs'),
        ],
    }


class PovrayLexer(RegexLexer):
    """
    For `Persistence of Vision Raytracer <http://www.povray.org/>`_ files.

    *New in Pygments 0.11.*
    """
    name = 'POVRay'
    aliases = ['pov']
    filenames = ['*.pov', '*.inc']
    mimetypes = ['text/x-povray']

    tokens = {
        'root': [
            (r'/\*[\w\W]*?\*/', Comment.Multiline),
            (r'//.*\n', Comment.Single),
            (r'(?s)"(?:\\.|[^"\\])+"', String.Double),
            (r'#(debug|default|else|end|error|fclose|fopen|ifdef|ifndef|'
             r'include|range|read|render|statistics|switch|undef|version|'
             r'warning|while|write|define|macro|local|declare)\b',
             Comment.Preproc),
            (r'\b(aa_level|aa_threshold|abs|acos|acosh|adaptive|adc_bailout|'
             r'agate|agate_turb|all|alpha|ambient|ambient_light|angle|'
             r'aperture|arc_angle|area_light|asc|asin|asinh|assumed_gamma|'
             r'atan|atan2|atanh|atmosphere|atmospheric_attenuation|'
             r'attenuating|average|background|black_hole|blue|blur_samples|'
             r'bounded_by|box_mapping|bozo|break|brick|brick_size|'
             r'brightness|brilliance|bumps|bumpy1|bumpy2|bumpy3|bump_map|'
             r'bump_size|case|caustics|ceil|checker|chr|clipped_by|clock|'
             r'color|color_map|colour|colour_map|component|composite|concat|'
             r'confidence|conic_sweep|constant|control0|control1|cos|cosh|'
             r'count|crackle|crand|cube|cubic_spline|cylindrical_mapping|'
             r'debug|declare|default|degrees|dents|diffuse|direction|'
             r'distance|distance_maximum|div|dust|dust_type|eccentricity|'
             r'else|emitting|end|error|error_bound|exp|exponent|'
             r'fade_distance|fade_power|falloff|falloff_angle|false|'
             r'file_exists|filter|finish|fisheye|flatness|flip|floor|'
             r'focal_point|fog|fog_alt|fog_offset|fog_type|frequency|gif|'
             r'global_settings|glowing|gradient|granite|gray_threshold|'
             r'green|halo|hexagon|hf_gray_16|hierarchy|hollow|hypercomplex|'
             r'if|ifdef|iff|image_map|incidence|include|int|interpolate|'
             r'inverse|ior|irid|irid_wavelength|jitter|lambda|leopard|'
             r'linear|linear_spline|linear_sweep|location|log|looks_like|'
             r'look_at|low_error_factor|mandel|map_type|marble|material_map|'
             r'matrix|max|max_intersections|max_iteration|max_trace_level|'
             r'max_value|metallic|min|minimum_reuse|mod|mortar|'
             r'nearest_count|no|normal|normal_map|no_shadow|number_of_waves|'
             r'octaves|off|offset|omega|omnimax|on|once|onion|open|'
             r'orthographic|panoramic|pattern1|pattern2|pattern3|'
             r'perspective|pgm|phase|phong|phong_size|pi|pigment|'
             r'pigment_map|planar_mapping|png|point_at|pot|pow|ppm|'
             r'precision|pwr|quadratic_spline|quaternion|quick_color|'
             r'quick_colour|quilted|radial|radians|radiosity|radius|rainbow|'
             r'ramp_wave|rand|range|reciprocal|recursion_limit|red|'
             r'reflection|refraction|render|repeat|rgb|rgbf|rgbft|rgbt|'
             r'right|ripples|rotate|roughness|samples|scale|scallop_wave|'
             r'scattering|seed|shadowless|sin|sine_wave|sinh|sky|sky_sphere|'
             r'slice|slope_map|smooth|specular|spherical_mapping|spiral|'
             r'spiral1|spiral2|spotlight|spotted|sqr|sqrt|statistics|str|'
             r'strcmp|strength|strlen|strlwr|strupr|sturm|substr|switch|sys|'
             r't|tan|tanh|test_camera_1|test_camera_2|test_camera_3|'
             r'test_camera_4|texture|texture_map|tga|thickness|threshold|'
             r'tightness|tile2|tiles|track|transform|translate|transmit|'
             r'triangle_wave|true|ttf|turbulence|turb_depth|type|'
             r'ultra_wide_angle|up|use_color|use_colour|use_index|u_steps|'
             r'val|variance|vaxis_rotate|vcross|vdot|version|vlength|'
             r'vnormalize|volume_object|volume_rendered|vol_with_light|'
             r'vrotate|v_steps|warning|warp|water_level|waves|while|width|'
             r'wood|wrinkles|yes)\b', Keyword),
            (r'(bicubic_patch|blob|box|camera|cone|cubic|cylinder|difference|'
             r'disc|height_field|intersection|julia_fractal|lathe|'
             r'light_source|merge|mesh|object|plane|poly|polygon|prism|'
             r'quadric|quartic|smooth_triangle|sor|sphere|superellipsoid|'
             r'text|torus|triangle|union)\b', Name.Builtin),
            # TODO: <=, etc
            (r'[\[\](){}<>;,]', Punctuation),
            (r'[-+*/=]', Operator),
            (r'\b(x|y|z|u|v)\b', Name.Builtin.Pseudo),
            (r'[a-zA-Z_][a-zA-Z_0-9]*', Name),
            (r'[0-9]+\.[0-9]*', Number.Float),
            (r'\.[0-9]+', Number.Float),
            (r'[0-9]+', Number.Integer),
            (r'\s+', Text),
        ]
    }


class AppleScriptLexer(RegexLexer):
    """
    For `AppleScript source code
    <http://developer.apple.com/documentation/AppleScript/
    Conceptual/AppleScriptLangGuide>`_,
    including `AppleScript Studio
    <http://developer.apple.com/documentation/AppleScript/
    Reference/StudioReference>`_.
    Contributed by Andreas Amann <aamann@mac.com>.
    """

    name = 'AppleScript'
    aliases = ['applescript']
    filenames = ['*.applescript']

    flags = re.MULTILINE | re.DOTALL

    Identifiers = r'[a-zA-Z]\w*'
    Literals = ['AppleScript', 'current application', 'false', 'linefeed',
                'missing value', 'pi','quote', 'result', 'return', 'space',
                'tab', 'text item delimiters', 'true', 'version']
    Classes = ['alias ', 'application ', 'boolean ', 'class ', 'constant ',
               'date ', 'file ', 'integer ', 'list ', 'number ', 'POSIX file ',
               'real ', 'record ', 'reference ', 'RGB color ', 'script ',
               'text ', 'unit types', '(?:Unicode )?text', 'string']
    BuiltIn = ['attachment', 'attribute run', 'character', 'day', 'month',
               'paragraph', 'word', 'year']
    HandlerParams = ['about', 'above', 'against', 'apart from', 'around',
                     'aside from', 'at', 'below', 'beneath', 'beside',
                     'between', 'for', 'given', 'instead of', 'on', 'onto',
                     'out of', 'over', 'since']
    Commands = ['ASCII (character|number)', 'activate', 'beep', 'choose URL',
                'choose application', 'choose color', 'choose file( name)?',
                'choose folder', 'choose from list',
                'choose remote application', 'clipboard info',
                'close( access)?', 'copy', 'count', 'current date', 'delay',
                'delete', 'display (alert|dialog)', 'do shell script',
                'duplicate', 'exists', 'get eof', 'get volume settings',
                'info for', 'launch', 'list (disks|folder)', 'load script',
                'log', 'make', 'mount volume', 'new', 'offset',
                'open( (for access|location))?', 'path to', 'print', 'quit',
                'random number', 'read', 'round', 'run( script)?',
                'say', 'scripting components',
                'set (eof|the clipboard to|volume)', 'store script',
                'summarize', 'system attribute', 'system info',
                'the clipboard', 'time to GMT', 'write', 'quoted form']
    References = ['(in )?back of', '(in )?front of', '[0-9]+(st|nd|rd|th)',
                  'first', 'second', 'third', 'fourth', 'fifth', 'sixth',
                  'seventh', 'eighth', 'ninth', 'tenth', 'after', 'back',
                  'before', 'behind', 'every', 'front', 'index', 'last',
                  'middle', 'some', 'that', 'through', 'thru', 'where', 'whose']
    Operators = ["and", "or", "is equal", "equals", "(is )?equal to", "is not",
                 "isn't", "isn't equal( to)?", "is not equal( to)?",
                 "doesn't equal", "does not equal", "(is )?greater than",
                 "comes after", "is not less than or equal( to)?",
                 "isn't less than or equal( to)?", "(is )?less than",
                 "comes before", "is not greater than or equal( to)?",
                 "isn't greater than or equal( to)?",
                 "(is  )?greater than or equal( to)?", "is not less than",
                 "isn't less than", "does not come before",
                 "doesn't come before", "(is )?less than or equal( to)?",
                 "is not greater than", "isn't greater than",
                 "does not come after", "doesn't come after", "starts? with",
                 "begins? with", "ends? with", "contains?", "does not contain",
                 "doesn't contain", "is in", "is contained by", "is not in",
                 "is not contained by", "isn't contained by", "div", "mod",
                 "not", "(a  )?(ref( to)?|reference to)", "is", "does"]
    Control = ['considering', 'else', 'error', 'exit', 'from', 'if',
               'ignoring', 'in', 'repeat', 'tell', 'then', 'times', 'to',
               'try', 'until', 'using terms from', 'while', 'whith',
               'with timeout( of)?', 'with transaction', 'by', 'continue',
               'end', 'its?', 'me', 'my', 'return', 'of' , 'as']
    Declarations = ['global', 'local', 'prop(erty)?', 'set', 'get']
    Reserved = ['but', 'put', 'returning', 'the']
    StudioClasses = ['action cell', 'alert reply', 'application', 'box',
                     'browser( cell)?', 'bundle', 'button( cell)?', 'cell',
                     'clip view', 'color well', 'color-panel',
                     'combo box( item)?', 'control',
                     'data( (cell|column|item|row|source))?', 'default entry',
                     'dialog reply', 'document', 'drag info', 'drawer',
                     'event', 'font(-panel)?', 'formatter',
                     'image( (cell|view))?', 'matrix', 'menu( item)?', 'item',
                     'movie( view)?', 'open-panel', 'outline view', 'panel',
                     'pasteboard', 'plugin', 'popup button',
                     'progress indicator', 'responder', 'save-panel',
                     'scroll view', 'secure text field( cell)?', 'slider',
                     'sound', 'split view', 'stepper', 'tab view( item)?',
                     'table( (column|header cell|header view|view))',
                     'text( (field( cell)?|view))?', 'toolbar( item)?',
                     'user-defaults', 'view', 'window']
    StudioEvents = ['accept outline drop', 'accept table drop', 'action',
                    'activated', 'alert ended', 'awake from nib', 'became key',
                    'became main', 'begin editing', 'bounds changed',
                    'cell value', 'cell value changed', 'change cell value',
                    'change item value', 'changed', 'child of item',
                    'choose menu item', 'clicked', 'clicked toolbar item',
                    'closed', 'column clicked', 'column moved',
                    'column resized', 'conclude drop', 'data representation',
                    'deminiaturized', 'dialog ended', 'document nib name',
                    'double clicked', 'drag( (entered|exited|updated))?',
                    'drop', 'end editing', 'exposed', 'idle', 'item expandable',
                    'item value', 'item value changed', 'items changed',
                    'keyboard down', 'keyboard up', 'launched',
                    'load data representation', 'miniaturized', 'mouse down',
                    'mouse dragged', 'mouse entered', 'mouse exited',
                    'mouse moved', 'mouse up', 'moved',
                    'number of browser rows', 'number of items',
                    'number of rows', 'open untitled', 'opened', 'panel ended',
                    'parameters updated', 'plugin loaded', 'prepare drop',
                    'prepare outline drag', 'prepare outline drop',
                    'prepare table drag', 'prepare table drop',
                    'read from file', 'resigned active', 'resigned key',
                    'resigned main', 'resized( sub views)?',
                    'right mouse down', 'right mouse dragged',
                    'right mouse up', 'rows changed', 'scroll wheel',
                    'selected tab view item', 'selection changed',
                    'selection changing', 'should begin editing',
                    'should close', 'should collapse item',
                    'should end editing', 'should expand item',
                    'should open( untitled)?',
                    'should quit( after last window closed)?',
                    'should select column', 'should select item',
                    'should select row', 'should select tab view item',
                    'should selection change', 'should zoom', 'shown',
                    'update menu item', 'update parameters',
                    'update toolbar item', 'was hidden', 'was miniaturized',
                    'will become active', 'will close', 'will dismiss',
                    'will display browser cell', 'will display cell',
                    'will display item cell', 'will display outline cell',
                    'will finish launching', 'will hide', 'will miniaturize',
                    'will move', 'will open', 'will pop up', 'will quit',
                    'will resign active', 'will resize( sub views)?',
                    'will select tab view item', 'will show', 'will zoom',
                    'write to file', 'zoomed']
    StudioCommands = ['animate', 'append', 'call method', 'center',
                      'close drawer', 'close panel', 'display',
                      'display alert', 'display dialog', 'display panel', 'go',
                      'hide', 'highlight', 'increment', 'item for',
                      'load image', 'load movie', 'load nib', 'load panel',
                      'load sound', 'localized string', 'lock focus', 'log',
                      'open drawer', 'path for', 'pause', 'perform action',
                      'play', 'register', 'resume', 'scroll', 'select( all)?',
                      'show', 'size to fit', 'start', 'step back',
                      'step forward', 'stop', 'synchronize', 'unlock focus',
                      'update']
    StudioProperties = ['accepts arrow key', 'action method', 'active',
                        'alignment', 'allowed identifiers',
                        'allows branch selection', 'allows column reordering',
                        'allows column resizing', 'allows column selection',
                        'allows customization',
                        'allows editing text attributes',
                        'allows empty selection', 'allows mixed state',
                        'allows multiple selection', 'allows reordering',
                        'allows undo', 'alpha( value)?', 'alternate image',
                        'alternate increment value', 'alternate title',
                        'animation delay', 'associated file name',
                        'associated object', 'auto completes', 'auto display',
                        'auto enables items', 'auto repeat',
                        'auto resizes( outline column)?',
                        'auto save expanded items', 'auto save name',
                        'auto save table columns', 'auto saves configuration',
                        'auto scroll', 'auto sizes all columns to fit',
                        'auto sizes cells', 'background color', 'bezel state',
                        'bezel style', 'bezeled', 'border rect', 'border type',
                        'bordered', 'bounds( rotation)?', 'box type',
                        'button returned', 'button type',
                        'can choose directories', 'can choose files',
                        'can draw', 'can hide',
                        'cell( (background color|size|type))?', 'characters',
                        'class', 'click count', 'clicked( data)? column',
                        'clicked data item', 'clicked( data)? row',
                        'closeable', 'collating', 'color( (mode|panel))',
                        'command key down', 'configuration',
                        'content(s| (size|view( margins)?))?', 'context',
                        'continuous', 'control key down', 'control size',
                        'control tint', 'control view',
                        'controller visible', 'coordinate system',
                        'copies( on scroll)?', 'corner view', 'current cell',
                        'current column', 'current( field)?  editor',
                        'current( menu)? item', 'current row',
                        'current tab view item', 'data source',
                        'default identifiers', 'delta (x|y|z)',
                        'destination window', 'directory', 'display mode',
                        'displayed cell', 'document( (edited|rect|view))?',
                        'double value', 'dragged column', 'dragged distance',
                        'dragged items', 'draws( cell)? background',
                        'draws grid', 'dynamically scrolls', 'echos bullets',
                        'edge', 'editable', 'edited( data)? column',
                        'edited data item', 'edited( data)? row', 'enabled',
                        'enclosing scroll view', 'ending page',
                        'error handling', 'event number', 'event type',
                        'excluded from windows menu', 'executable path',
                        'expanded', 'fax number', 'field editor', 'file kind',
                        'file name', 'file type', 'first responder',
                        'first visible column', 'flipped', 'floating',
                        'font( panel)?', 'formatter', 'frameworks path',
                        'frontmost', 'gave up', 'grid color', 'has data items',
                        'has horizontal ruler', 'has horizontal scroller',
                        'has parent data item', 'has resize indicator',
                        'has shadow', 'has sub menu', 'has vertical ruler',
                        'has vertical scroller', 'header cell', 'header view',
                        'hidden', 'hides when deactivated', 'highlights by',
                        'horizontal line scroll', 'horizontal page scroll',
                        'horizontal ruler view', 'horizontally resizable',
                        'icon image', 'id', 'identifier',
                        'ignores multiple clicks',
                        'image( (alignment|dims when disabled|frame style|'
                            'scaling))?',
                        'imports graphics', 'increment value',
                        'indentation per level', 'indeterminate', 'index',
                        'integer value', 'intercell spacing', 'item height',
                        'key( (code|equivalent( modifier)?|window))?',
                        'knob thickness', 'label', 'last( visible)? column',
                        'leading offset', 'leaf', 'level', 'line scroll',
                        'loaded', 'localized sort', 'location', 'loop mode',
                        'main( (bunde|menu|window))?', 'marker follows cell',
                        'matrix mode', 'maximum( content)? size',
                        'maximum visible columns',
                        'menu( form representation)?', 'miniaturizable',
                        'miniaturized', 'minimized image', 'minimized title',
                        'minimum column width', 'minimum( content)? size',
                        'modal', 'modified', 'mouse down state',
                        'movie( (controller|file|rect))?', 'muted', 'name',
                        'needs display', 'next state', 'next text',
                        'number of tick marks', 'only tick mark values',
                        'opaque', 'open panel', 'option key down',
                        'outline table column', 'page scroll', 'pages across',
                        'pages down', 'palette label', 'pane splitter',
                        'parent data item', 'parent window', 'pasteboard',
                        'path( (names|separator))?', 'playing',
                        'plays every frame', 'plays selection only', 'position',
                        'preferred edge', 'preferred type', 'pressure',
                        'previous text', 'prompt', 'properties',
                        'prototype cell', 'pulls down', 'rate',
                        'released when closed', 'repeated',
                        'requested print time', 'required file type',
                        'resizable', 'resized column', 'resource path',
                        'returns records', 'reuses columns', 'rich text',
                        'roll over', 'row height', 'rulers visible',
                        'save panel', 'scripts path', 'scrollable',
                        'selectable( identifiers)?', 'selected cell',
                        'selected( data)? columns?', 'selected data items?',
                        'selected( data)? rows?', 'selected item identifier',
                        'selection by rect', 'send action on arrow key',
                        'sends action when done editing', 'separates columns',
                        'separator item', 'sequence number', 'services menu',
                        'shared frameworks path', 'shared support path',
                        'sheet', 'shift key down', 'shows alpha',
                        'shows state by', 'size( mode)?',
                        'smart insert delete enabled', 'sort case sensitivity',
                        'sort column', 'sort order', 'sort type',
                        'sorted( data rows)?', 'sound', 'source( mask)?',
                        'spell checking enabled', 'starting page', 'state',
                        'string value', 'sub menu', 'super menu', 'super view',
                        'tab key traverses cells', 'tab state', 'tab type',
                        'tab view', 'table view', 'tag', 'target( printer)?',
                        'text color', 'text container insert',
                        'text container origin', 'text returned',
                        'tick mark position', 'time stamp',
                        'title(d| (cell|font|height|position|rect))?',
                        'tool tip', 'toolbar', 'trailing offset', 'transparent',
                        'treat packages as directories', 'truncated labels',
                        'types', 'unmodified characters', 'update views',
                        'use sort indicator', 'user defaults',
                        'uses data source', 'uses ruler',
                        'uses threaded animation',
                        'uses title from previous column', 'value wraps',
                        'version',
                        'vertical( (line scroll|page scroll|ruler view))?',
                        'vertically resizable', 'view',
                        'visible( document rect)?', 'volume', 'width', 'window',
                        'windows menu', 'wraps', 'zoomable', 'zoomed']

    tokens = {
        'root': [
            (r'\s+', Text),
            (ur'¬\n', String.Escape),
            (r"'s\s+", Text), # This is a possessive, consider moving
            (r'(--|#).*?$', Comment),
            (r'\(\*', Comment.Multiline, 'comment'),
            (r'[\(\){}!,.:]', Punctuation),
            (ur'(«)([^»]+)(»)',
             bygroups(Text, Name.Builtin, Text)),
            (r'\b((?:considering|ignoring)\s*)'
             r'(application responses|case|diacriticals|hyphens|'
             r'numeric strings|punctuation|white space)',
             bygroups(Keyword, Name.Builtin)),
            (ur'(-|\*|\+|&|≠|>=?|<=?|=|≥|≤|/|÷|\^)', Operator),
            (r"\b(%s)\b" % '|'.join(Operators), Operator.Word),
            (r'^(\s*(?:on|end)\s+)'
             r'(%s)' % '|'.join(StudioEvents[::-1]),
             bygroups(Keyword, Name.Function)),
            (r'^(\s*)(in|on|script|to)(\s+)', bygroups(Text, Keyword, Text)),
            (r'\b(as )(%s)\b' % '|'.join(Classes),
             bygroups(Keyword, Name.Class)),
            (r'\b(%s)\b' % '|'.join(Literals), Name.Constant),
            (r'\b(%s)\b' % '|'.join(Commands), Name.Builtin),
            (r'\b(%s)\b' % '|'.join(Control), Keyword),
            (r'\b(%s)\b' % '|'.join(Declarations), Keyword),
            (r'\b(%s)\b' % '|'.join(Reserved), Name.Builtin),
            (r'\b(%s)s?\b' % '|'.join(BuiltIn), Name.Builtin),
            (r'\b(%s)\b' % '|'.join(HandlerParams), Name.Builtin),
            (r'\b(%s)\b' % '|'.join(StudioProperties), Name.Attribute),
            (r'\b(%s)s?\b' % '|'.join(StudioClasses), Name.Builtin),
            (r'\b(%s)\b' % '|'.join(StudioCommands), Name.Builtin),
            (r'\b(%s)\b' % '|'.join(References), Name.Builtin),
            (r'"(\\\\|\\"|[^"])*"', String.Double),
            (r'\b(%s)\b' % Identifiers, Name.Variable),
            (r'[-+]?(\d+\.\d*|\d*\.\d+)(E[-+][0-9]+)?', Number.Float),
            (r'[-+]?\d+', Number.Integer),
        ],
        'comment': [
            ('\(\*', Comment.Multiline, '#push'),
            ('\*\)', Comment.Multiline, '#pop'),
            ('[^*(]+', Comment.Multiline),
            ('[*(]', Comment.Multiline),
        ],
    }


class ModelicaLexer(RegexLexer):
    """
    For `Modelica <http://www.modelica.org/>`_ source code.

    *New in Pygments 1.1.*
    """
    name = 'Modelica'
    aliases = ['modelica']
    filenames = ['*.mo']
    mimetypes = ['text/x-modelica']

    flags = re.IGNORECASE | re.DOTALL

    tokens = {
        'whitespace': [
            (r'\n', Text),
            (r'\s+', Text),
            (r'\\\n', Text), # line continuation
            (r'//(\n|(.|\n)*?[^\\]\n)', Comment),
            (r'/(\\\n)?\*(.|\n)*?\*(\\\n)?/', Comment),
        ],
        'statements': [
            (r'"', String, 'string'),
            (r'(\d+\.\d*|\.\d+|\d+|\d.)[eE][+-]?\d+[lL]?', Number.Float),
            (r'(\d+\.\d*|\.\d+)', Number.Float),
            (r'\d+[Ll]?', Number.Integer),
            (r'[~!%^&*+=|?:<>/-]', Operator),
            (r'[()\[\]{},.;]', Punctuation),
            (r'(true|false|NULL|Real|Integer|Boolean)\b', Name.Builtin),
            (r"([a-zA-Z_][\w]*|'[a-zA-Z_\+\-\*\/\^][\w]*')"
             r"(\.([a-zA-Z_][\w]*|'[a-zA-Z_\+\-\*\/\^][\w]*'))+", Name.Class),
            (r"('[\w\+\-\*\/\^]+'|\w+)", Name),
        ],
        'root': [
            include('whitespace'),
            include('keywords'),
            include('functions'),
            include('operators'),
            include('classes'),
            (r'("<html>|<html>)', Name.Tag, 'html-content'),
            include('statements'),
        ],
        'keywords': [
            (r'(algorithm|annotation|break|connect|constant|constrainedby|'
            r'discrete|each|else|elseif|elsewhen|encapsulated|enumeration|'
            r'end|equation|exit|expandable|extends|'
            r'external|false|final|flow|for|if|import|impure|in|initial\sequation|'
            r'inner|input|loop|nondiscrete|outer|output|parameter|partial|'
            r'protected|public|pure|redeclare|replaceable|stream|time|then|true|'
            r'when|while|within)\b', Keyword),
        ],
        'functions': [
            (r'(abs|acos|acosh|asin|asinh|atan|atan2|atan3|ceil|cos|cosh|'
             r'cross|div|exp|floor|getInstanceName|log|log10|mod|rem|'
             r'semiLinear|sign|sin|sinh|size|spatialDistribution|sqrt|tan|'
             r'tanh|zeros)\b', Name.Function),
        ],
        'operators': [
            (r'(actualStream|and|assert|cardinality|change|Clock|delay|der|edge|'
             r'hold|homotopy|initial|inStream|noEvent|not|or|pre|previous|reinit|'
             r'return|sample|smooth|spatialDistribution|subSample|terminal|'
             r'terminate)\b', Name.Builtin),
        ],
        'classes': [
            (r'(block|class|connector|function|model|package|'
             r'record|type)(\s+)([A-Za-z_]+)',
             bygroups(Keyword, Text, Name.Class))
        ],
        'string': [
            (r'"', String, '#pop'),
            (r'\\([\\abfnrtv"\']|x[a-fA-F0-9]{2,4}|[0-7]{1,3})',
             String.Escape),
            (r'[^\\"\n]+', String), # all other characters
            (r'\\\n', String), # line continuation
            (r'\\', String), # stray backslash
        ],
        'html-content': [
            (r'<\s*/\s*html\s*>', Name.Tag, '#pop'),
            (r'.+?(?=<\s*/\s*html\s*>)', using(HtmlLexer)),
        ]
    }


class RebolLexer(RegexLexer):
    """
    A `REBOL <http://www.rebol.com/>`_ lexer.

    *New in Pygments 1.1.*
    """
    name = 'REBOL'
    aliases = ['rebol']
    filenames = ['*.r', '*.r3']
    mimetypes = ['text/x-rebol']

    flags = re.IGNORECASE | re.MULTILINE

    re.IGNORECASE

    escape_re = r'(?:\^\([0-9a-fA-F]{1,4}\)*)'

    def word_callback(lexer, match):
        word = match.group()

        if re.match(".*:$", word):
            yield match.start(), Generic.Subheading, word
        elif re.match(
            r'(native|alias|all|any|as-string|as-binary|bind|bound\?|case|'
            r'catch|checksum|comment|debase|dehex|exclude|difference|disarm|'
            r'either|else|enbase|foreach|remove-each|form|free|get|get-env|if|'
            r'in|intersect|loop|minimum-of|maximum-of|mold|new-line|'
            r'new-line\?|not|now|prin|print|reduce|compose|construct|repeat|'
            r'reverse|save|script\?|set|shift|switch|throw|to-hex|trace|try|'
            r'type\?|union|unique|unless|unprotect|unset|until|use|value\?|'
            r'while|compress|decompress|secure|open|close|read|read-io|'
            r'write-io|write|update|query|wait|input\?|exp|log-10|log-2|'
            r'log-e|square-root|cosine|sine|tangent|arccosine|arcsine|'
            r'arctangent|protect|lowercase|uppercase|entab|detab|connected\?|'
            r'browse|launch|stats|get-modes|set-modes|to-local-file|'
            r'to-rebol-file|encloak|decloak|create-link|do-browser|bind\?|'
            r'hide|draw|show|size-text|textinfo|offset-to-caret|'
            r'caret-to-offset|local-request-file|rgb-to-hsv|hsv-to-rgb|'
            r'crypt-strength\?|dh-make-key|dh-generate-key|dh-compute-key|'
            r'dsa-make-key|dsa-generate-key|dsa-make-signature|'
            r'dsa-verify-signature|rsa-make-key|rsa-generate-key|'
            r'rsa-encrypt)$', word):
            yield match.start(), Name.Builtin, word
        elif re.match(
            r'(add|subtract|multiply|divide|remainder|power|and~|or~|xor~|'
            r'minimum|maximum|negate|complement|absolute|random|head|tail|'
            r'next|back|skip|at|pick|first|second|third|fourth|fifth|sixth|'
            r'seventh|eighth|ninth|tenth|last|path|find|select|make|to|copy\*|'
            r'insert|remove|change|poke|clear|trim|sort|min|max|abs|cp|'
            r'copy)$', word):
            yield match.start(), Name.Function, word
        elif re.match(
            r'(error|source|input|license|help|install|echo|Usage|with|func|'
            r'throw-on-error|function|does|has|context|probe|\?\?|as-pair|'
            r'mod|modulo|round|repend|about|set-net|append|join|rejoin|reform|'
            r'remold|charset|array|replace|move|extract|forskip|forall|alter|'
            r'first+|also|take|for|forever|dispatch|attempt|what-dir|'
            r'change-dir|clean-path|list-dir|dirize|rename|split-path|delete|'
            r'make-dir|delete-dir|in-dir|confirm|dump-obj|upgrade|what|'
            r'build-tag|process-source|build-markup|decode-cgi|read-cgi|'
            r'write-user|save-user|set-user-name|protect-system|parse-xml|'
            r'cvs-date|cvs-version|do-boot|get-net-info|desktop|layout|'
            r'scroll-para|get-face|alert|set-face|uninstall|unfocus|'
            r'request-dir|center-face|do-events|net-error|decode-url|'
            r'parse-header|parse-header-date|parse-email-addrs|import-email|'
            r'send|build-attach-body|resend|show-popup|hide-popup|open-events|'
            r'find-key-face|do-face|viewtop|confine|find-window|'
            r'insert-event-func|remove-event-func|inform|dump-pane|dump-face|'
            r'flag-face|deflag-face|clear-fields|read-net|vbug|path-thru|'
            r'read-thru|load-thru|do-thru|launch-thru|load-image|'
            r'request-download|do-face-alt|set-font|set-para|get-style|'
            r'set-style|make-face|stylize|choose|hilight-text|hilight-all|'
            r'unlight-text|focus|scroll-drag|clear-face|reset-face|scroll-face|'
            r'resize-face|load-stock|load-stock-block|notify|request|flash|'
            r'request-color|request-pass|request-text|request-list|'
            r'request-date|request-file|dbug|editor|link-relative-path|'
            r'emailer|parse-error)$', word):
            yield match.start(), Keyword.Namespace, word
        elif re.match(
            r'(halt|quit|do|load|q|recycle|call|run|ask|parse|view|unview|'
            r'return|exit|break)$', word):
            yield match.start(), Name.Exception, word
        elif re.match('REBOL$', word):
            yield match.start(), Generic.Heading, word
        elif re.match("to-.*", word):
            yield match.start(), Keyword, word
        elif re.match('(\+|-|\*|/|//|\*\*|and|or|xor|=\?|=|==|<>|<|>|<=|>=)$',
                      word):
            yield match.start(), Operator, word
        elif re.match(".*\?$", word):
            yield match.start(), Keyword, word
        elif re.match(".*\!$", word):
            yield match.start(), Keyword.Type, word
        elif re.match("'.*", word):
            yield match.start(), Name.Variable.Instance, word # lit-word
        elif re.match("#.*", word):
            yield match.start(), Name.Label, word # issue
        elif re.match("%.*", word):
            yield match.start(), Name.Decorator, word # file
        else:
            yield match.start(), Name.Variable, word

    tokens = {
        'root': [
            (r'REBOL', Generic.Strong, 'script'),
            (r'R', Comment),
            (r'[^R]+', Comment),
        ],
        'script': [
            (r'\s+', Text),
            (r'#"', String.Char, 'char'),
            (r'#{[0-9a-fA-F]*}', Number.Hex),
            (r'2#{', Number.Hex, 'bin2'),
            (r'64#{[0-9a-zA-Z+/=\s]*}', Number.Hex),
            (r'"', String, 'string'),
            (r'{', String, 'string2'),
            (r';#+.*\n', Comment.Special),
            (r';\*+.*\n', Comment.Preproc),
            (r';.*\n', Comment),
            (r'%"', Name.Decorator, 'stringFile'),
            (r'%[^(\^{^")\s\[\]]+', Name.Decorator),
            (r'<[a-zA-Z0-9:._-]*>', Name.Tag),
            (r'<[^(<>\s")]+', Name.Tag, 'tag'),
            (r'[+-]?([a-zA-Z]{1,3})?\$\d+(\.\d+)?', Number.Float), # money
            (r'[+-]?\d+\:\d+(\:\d+)?(\.\d+)?', String.Other), # time
            (r'\d+\-[0-9a-zA-Z]+\-\d+(\/\d+\:\d+(\:\d+)?'
             r'([\.\d+]?([+-]?\d+:\d+)?)?)?', String.Other), # date
            (r'\d+(\.\d+)+\.\d+', Keyword.Constant), # tuple
            (r'\d+[xX]\d+', Keyword.Constant), # pair
            (r'[+-]?\d+(\'\d+)?([\.,]\d*)?[eE][+-]?\d+', Number.Float),
            (r'[+-]?\d+(\'\d+)?[\.,]\d*', Number.Float),
            (r'[+-]?\d+(\'\d+)?', Number),
            (r'[\[\]\(\)]', Generic.Strong),
            (r'[a-zA-Z]+[^(\^{"\s:)]*://[^(\^{"\s)]*', Name.Decorator), # url
            (r'mailto:[^(\^{"@\s)]+@[^(\^{"@\s)]+', Name.Decorator), # url
            (r'[^(\^{"@\s)]+@[^(\^{"@\s)]+', Name.Decorator), # email
            (r'comment\s', Comment, 'comment'),
            (r'/[^(\^{^")\s/[\]]*', Name.Attribute),
            (r'([^(\^{^")\s/[\]]+)(?=[:({"\s/\[\]])', word_callback),
            (r'([^(\^{^")\s]+)', Text),
        ],
        'string': [
            (r'[^(\^")]+', String),
            (escape_re, String.Escape),
            (r'[\(|\)]+', String),
            (r'\^.', String.Escape),
            (r'"', String, '#pop'),
        ],
        'string2': [
            (r'[^(\^{^})]+', String),
            (escape_re, String.Escape),
            (r'[\(|\)]+', String),
            (r'\^.', String.Escape),
            (r'{', String, '#push'),
            (r'}', String, '#pop'),
        ],
        'stringFile': [
            (r'[^(\^")]+', Name.Decorator),
            (escape_re, Name.Decorator),
            (r'\^.', Name.Decorator),
            (r'"', Name.Decorator, '#pop'),
        ],
        'char': [
            (escape_re + '"', String.Char, '#pop'),
            (r'\^."', String.Char, '#pop'),
            (r'."', String.Char, '#pop'),
        ],
        'tag': [
            (escape_re, Name.Tag),
            (r'"', Name.Tag, 'tagString'),
            (r'[^(<>\r\n")]+', Name.Tag),
            (r'>', Name.Tag, '#pop'),
        ],
        'tagString': [
            (r'[^(\^")]+', Name.Tag),
            (escape_re, Name.Tag),
            (r'[\(|\)]+', Name.Tag),
            (r'\^.', Name.Tag),
            (r'"', Name.Tag, '#pop'),
        ],
        'tuple': [
            (r'(\d+\.)+', Keyword.Constant),
            (r'\d+', Keyword.Constant, '#pop'),
        ],
        'bin2': [
            (r'\s+', Number.Hex),
            (r'([0-1]\s*){8}', Number.Hex),
            (r'}', Number.Hex, '#pop'),
        ],
        'comment': [
            (r'"', Comment, 'commentString1'),
            (r'{', Comment, 'commentString2'),
            (r'\[', Comment, 'commentBlock'),
            (r'[^(\s{\"\[]+', Comment, '#pop'),
        ],
        'commentString1': [
            (r'[^(\^")]+', Comment),
            (escape_re, Comment),
            (r'[\(|\)]+', Comment),
            (r'\^.', Comment),
            (r'"', Comment, '#pop'),
        ],
        'commentString2': [
            (r'[^(\^{^})]+', Comment),
            (escape_re, Comment),
            (r'[\(|\)]+', Comment),
            (r'\^.', Comment),
            (r'{', Comment, '#push'),
            (r'}', Comment, '#pop'),
        ],
        'commentBlock': [
            (r'\[', Comment, '#push'),
            (r'\]', Comment, '#pop'),
            (r'[^(\[\])]+', Comment),
        ],
    }


class ABAPLexer(RegexLexer):
    """
    Lexer for ABAP, SAP's integrated language.

    *New in Pygments 1.1.*
    """
    name = 'ABAP'
    aliases = ['abap']
    filenames = ['*.abap']
    mimetypes = ['text/x-abap']

    flags = re.IGNORECASE | re.MULTILINE

    tokens = {
        'common': [
            (r'\s+', Text),
            (r'^\*.*$', Comment.Single),
            (r'\".*?\n', Comment.Single),
            ],
        'variable-names': [
            (r'<[\S_]+>', Name.Variable),
            (r'\w[\w~]*(?:(\[\])|->\*)?', Name.Variable),
            ],
        'root': [
            include('common'),
            #function calls
            (r'(CALL\s+(?:BADI|CUSTOMER-FUNCTION|FUNCTION))(\s+)(\'?\S+\'?)',
                bygroups(Keyword, Text, Name.Function)),
            (r'(CALL\s+(?:DIALOG|SCREEN|SUBSCREEN|SELECTION-SCREEN|'
             r'TRANSACTION|TRANSFORMATION))\b',
                Keyword),
            (r'(FORM|PERFORM)(\s+)(\w+)',
                bygroups(Keyword, Text, Name.Function)),
            (r'(PERFORM)(\s+)(\()(\w+)(\))',
                bygroups(Keyword, Text, Punctuation, Name.Variable, Punctuation )),
            (r'(MODULE)(\s+)(\S+)(\s+)(INPUT|OUTPUT)',
                bygroups(Keyword, Text, Name.Function, Text, Keyword)),

            # method implementation
            (r'(METHOD)(\s+)([\w~]+)',
                bygroups(Keyword, Text, Name.Function)),
            # method calls
            (r'(\s+)([\w\-]+)([=\-]>)([\w\-~]+)',
                bygroups(Text, Name.Variable, Operator, Name.Function)),
            # call methodnames returning style
            (r'(?<=(=|-)>)([\w\-~]+)(?=\()', Name.Function),

            # keywords with dashes in them.
            # these need to be first, because for instance the -ID part
            # of MESSAGE-ID wouldn't get highlighted if MESSAGE was
            # first in the list of keywords.
            (r'(ADD-CORRESPONDING|AUTHORITY-CHECK|'
             r'CLASS-DATA|CLASS-EVENTS|CLASS-METHODS|CLASS-POOL|'
             r'DELETE-ADJACENT|DIVIDE-CORRESPONDING|'
             r'EDITOR-CALL|ENHANCEMENT-POINT|ENHANCEMENT-SECTION|EXIT-COMMAND|'
             r'FIELD-GROUPS|FIELD-SYMBOLS|FUNCTION-POOL|'
             r'INTERFACE-POOL|INVERTED-DATE|'
             r'LOAD-OF-PROGRAM|LOG-POINT|'
             r'MESSAGE-ID|MOVE-CORRESPONDING|MULTIPLY-CORRESPONDING|'
             r'NEW-LINE|NEW-PAGE|NEW-SECTION|NO-EXTENSION|'
             r'OUTPUT-LENGTH|PRINT-CONTROL|'
             r'SELECT-OPTIONS|START-OF-SELECTION|SUBTRACT-CORRESPONDING|'
             r'SYNTAX-CHECK|SYSTEM-EXCEPTIONS|'
             r'TYPE-POOL|TYPE-POOLS'
             r')\b', Keyword),

             # keyword kombinations
            (r'CREATE\s+(PUBLIC|PRIVATE|DATA|OBJECT)|'
             r'((PUBLIC|PRIVATE|PROTECTED)\s+SECTION|'
             r'(TYPE|LIKE)(\s+(LINE\s+OF|REF\s+TO|'
             r'(SORTED|STANDARD|HASHED)\s+TABLE\s+OF))?|'
             r'FROM\s+(DATABASE|MEMORY)|CALL\s+METHOD|'
             r'(GROUP|ORDER) BY|HAVING|SEPARATED BY|'
             r'GET\s+(BADI|BIT|CURSOR|DATASET|LOCALE|PARAMETER|'
                      r'PF-STATUS|(PROPERTY|REFERENCE)\s+OF|'
                      r'RUN\s+TIME|TIME\s+(STAMP)?)?|'
             r'SET\s+(BIT|BLANK\s+LINES|COUNTRY|CURSOR|DATASET|EXTENDED\s+CHECK|'
                      r'HANDLER|HOLD\s+DATA|LANGUAGE|LEFT\s+SCROLL-BOUNDARY|'
                      r'LOCALE|MARGIN|PARAMETER|PF-STATUS|PROPERTY\s+OF|'
                      r'RUN\s+TIME\s+(ANALYZER|CLOCK\s+RESOLUTION)|SCREEN|'
                      r'TITLEBAR|UPADTE\s+TASK\s+LOCAL|USER-COMMAND)|'
             r'CONVERT\s+((INVERTED-)?DATE|TIME|TIME\s+STAMP|TEXT)|'
             r'(CLOSE|OPEN)\s+(DATASET|CURSOR)|'
             r'(TO|FROM)\s+(DATA BUFFER|INTERNAL TABLE|MEMORY ID|'
                            r'DATABASE|SHARED\s+(MEMORY|BUFFER))|'
             r'DESCRIBE\s+(DISTANCE\s+BETWEEN|FIELD|LIST|TABLE)|'
             r'FREE\s(MEMORY|OBJECT)?|'
             r'PROCESS\s+(BEFORE\s+OUTPUT|AFTER\s+INPUT|'
                          r'ON\s+(VALUE-REQUEST|HELP-REQUEST))|'
             r'AT\s+(LINE-SELECTION|USER-COMMAND|END\s+OF|NEW)|'
             r'AT\s+SELECTION-SCREEN(\s+(ON(\s+(BLOCK|(HELP|VALUE)-REQUEST\s+FOR|'
                                     r'END\s+OF|RADIOBUTTON\s+GROUP))?|OUTPUT))?|'
             r'SELECTION-SCREEN:?\s+((BEGIN|END)\s+OF\s+((TABBED\s+)?BLOCK|LINE|'
                                     r'SCREEN)|COMMENT|FUNCTION\s+KEY|'
                                     r'INCLUDE\s+BLOCKS|POSITION|PUSHBUTTON|'
                                     r'SKIP|ULINE)|'
             r'LEAVE\s+(LIST-PROCESSING|PROGRAM|SCREEN|'
                        r'TO LIST-PROCESSING|TO TRANSACTION)'
             r'(ENDING|STARTING)\s+AT|'
             r'FORMAT\s+(COLOR|INTENSIFIED|INVERSE|HOTSPOT|INPUT|FRAMES|RESET)|'
             r'AS\s+(CHECKBOX|SUBSCREEN|WINDOW)|'
             r'WITH\s+(((NON-)?UNIQUE)?\s+KEY|FRAME)|'
             r'(BEGIN|END)\s+OF|'
             r'DELETE(\s+ADJACENT\s+DUPLICATES\sFROM)?|'
             r'COMPARING(\s+ALL\s+FIELDS)?|'
             r'INSERT(\s+INITIAL\s+LINE\s+INTO|\s+LINES\s+OF)?|'
             r'IN\s+((BYTE|CHARACTER)\s+MODE|PROGRAM)|'
             r'END-OF-(DEFINITION|PAGE|SELECTION)|'
             r'WITH\s+FRAME(\s+TITLE)|'

             # simple kombinations
             r'AND\s+(MARK|RETURN)|CLIENT\s+SPECIFIED|CORRESPONDING\s+FIELDS\s+OF|'
             r'IF\s+FOUND|FOR\s+EVENT|INHERITING\s+FROM|LEAVE\s+TO\s+SCREEN|'
             r'LOOP\s+AT\s+(SCREEN)?|LOWER\s+CASE|MATCHCODE\s+OBJECT|MODIF\s+ID|'
             r'MODIFY\s+SCREEN|NESTING\s+LEVEL|NO\s+INTERVALS|OF\s+STRUCTURE|'
             r'RADIOBUTTON\s+GROUP|RANGE\s+OF|REF\s+TO|SUPPRESS DIALOG|'
             r'TABLE\s+OF|UPPER\s+CASE|TRANSPORTING\s+NO\s+FIELDS|'
             r'VALUE\s+CHECK|VISIBLE\s+LENGTH|HEADER\s+LINE)\b', Keyword),

            # single word keywords.
            (r'(^|(?<=(\s|\.)))(ABBREVIATED|ADD|ALIASES|APPEND|ASSERT|'
             r'ASSIGN(ING)?|AT(\s+FIRST)?|'
             r'BACK|BLOCK|BREAK-POINT|'
             r'CASE|CATCH|CHANGING|CHECK|CLASS|CLEAR|COLLECT|COLOR|COMMIT|'
             r'CREATE|COMMUNICATION|COMPONENTS?|COMPUTE|CONCATENATE|CONDENSE|'
             r'CONSTANTS|CONTEXTS|CONTINUE|CONTROLS|'
             r'DATA|DECIMALS|DEFAULT|DEFINE|DEFINITION|DEFERRED|DEMAND|'
             r'DETAIL|DIRECTORY|DIVIDE|DO|'
             r'ELSE(IF)?|ENDAT|ENDCASE|ENDCLASS|ENDDO|ENDFORM|ENDFUNCTION|'
             r'ENDIF|ENDLOOP|ENDMETHOD|ENDMODULE|ENDSELECT|ENDTRY|'
             r'ENHANCEMENT|EVENTS|EXCEPTIONS|EXIT|EXPORT|EXPORTING|EXTRACT|'
             r'FETCH|FIELDS?|FIND|FOR|FORM|FORMAT|FREE|FROM|'
             r'HIDE|'
             r'ID|IF|IMPORT|IMPLEMENTATION|IMPORTING|IN|INCLUDE|INCLUDING|'
             r'INDEX|INFOTYPES|INITIALIZATION|INTERFACE|INTERFACES|INTO|'
             r'LENGTH|LINES|LOAD|LOCAL|'
             r'JOIN|'
             r'KEY|'
             r'MAXIMUM|MESSAGE|METHOD[S]?|MINIMUM|MODULE|MODIFY|MOVE|MULTIPLY|'
             r'NODES|'
             r'OBLIGATORY|OF|OFF|ON|OVERLAY|'
             r'PACK|PARAMETERS|PERCENTAGE|POSITION|PROGRAM|PROVIDE|PUBLIC|PUT|'
             r'RAISE|RAISING|RANGES|READ|RECEIVE|REFRESH|REJECT|REPORT|RESERVE|'
             r'RESUME|RETRY|RETURN|RETURNING|RIGHT|ROLLBACK|'
             r'SCROLL|SEARCH|SELECT|SHIFT|SINGLE|SKIP|SORT|SPLIT|STATICS|STOP|'
             r'SUBMIT|SUBTRACT|SUM|SUMMARY|SUMMING|SUPPLY|'
             r'TABLE|TABLES|TIMES|TITLE|TO|TOP-OF-PAGE|TRANSFER|TRANSLATE|TRY|TYPES|'
             r'ULINE|UNDER|UNPACK|UPDATE|USING|'
             r'VALUE|VALUES|VIA|'
             r'WAIT|WHEN|WHERE|WHILE|WITH|WINDOW|WRITE)\b', Keyword),

             # builtins
            (r'(abs|acos|asin|atan|'
             r'boolc|boolx|bit_set|'
             r'char_off|charlen|ceil|cmax|cmin|condense|contains|'
             r'contains_any_of|contains_any_not_of|concat_lines_of|cos|cosh|'
             r'count|count_any_of|count_any_not_of|'
             r'dbmaxlen|distance|'
             r'escape|exp|'
             r'find|find_end|find_any_of|find_any_not_of|floor|frac|from_mixed|'
             r'insert|'
             r'lines|log|log10|'
             r'match|matches|'
             r'nmax|nmin|numofchar|'
             r'repeat|replace|rescale|reverse|round|'
             r'segment|shift_left|shift_right|sign|sin|sinh|sqrt|strlen|'
             r'substring|substring_after|substring_from|substring_before|substring_to|'
             r'tan|tanh|to_upper|to_lower|to_mixed|translate|trunc|'
             r'xstrlen)(\()\b', bygroups(Name.Builtin, Punctuation)),

            (r'&[0-9]', Name),
            (r'[0-9]+', Number.Integer),

            # operators which look like variable names before
            # parsing variable names.
            (r'(?<=(\s|.))(AND|EQ|NE|GT|LT|GE|LE|CO|CN|CA|NA|CS|NOT|NS|CP|NP|'
             r'BYTE-CO|BYTE-CN|BYTE-CA|BYTE-NA|BYTE-CS|BYTE-NS|'
             r'IS\s+(NOT\s+)?(INITIAL|ASSIGNED|REQUESTED|BOUND))\b', Operator),

            include('variable-names'),

            # standard oparators after variable names,
            # because < and > are part of field symbols.
            (r'[?*<>=\-+]', Operator),
            (r"'(''|[^'])*'", String.Single),
            (r'[/;:()\[\],\.]', Punctuation)
        ],
    }


class NewspeakLexer(RegexLexer):
    """
    For `Newspeak <http://newspeaklanguage.org/>` syntax.
    """
    name = 'Newspeak'
    filenames = ['*.ns2']
    aliases = ['newspeak', ]
    mimetypes = ['text/x-newspeak']

    tokens = {
       'root' : [
           (r'\b(Newsqueak2)\b',Keyword.Declaration),
           (r"'[^']*'",String),
           (r'\b(class)(\s+)([a-zA-Z0-9_]+)(\s*)',
            bygroups(Keyword.Declaration,Text,Name.Class,Text)),
           (r'\b(mixin|self|super|private|public|protected|nil|true|false)\b',
            Keyword),
           (r'([a-zA-Z0-9_]+\:)(\s*)([a-zA-Z_]\w+)',
            bygroups(Name.Function,Text,Name.Variable)),
           (r'([a-zA-Z0-9_]+)(\s*)(=)',
            bygroups(Name.Attribute,Text,Operator)),
           (r'<[a-zA-Z0-9_]+>', Comment.Special),
           include('expressionstat'),
           include('whitespace')
        ],

       'expressionstat': [
          (r'(\d+\.\d*|\.\d+|\d+[fF])[fF]?', Number.Float),
          (r'\d+', Number.Integer),
          (r':\w+',Name.Variable),
          (r'(\w+)(::)', bygroups(Name.Variable, Operator)),
          (r'\w+:', Name.Function),
          (r'\w+', Name.Variable),
          (r'\(|\)', Punctuation),
          (r'\[|\]', Punctuation),
          (r'\{|\}', Punctuation),

          (r'(\^|\+|\/|~|\*|<|>|=|@|%|\||&|\?|!|,|-|:)', Operator),
          (r'\.|;', Punctuation),
          include('whitespace'),
          include('literals'),
       ],
       'literals': [
         (r'\$.', String),
         (r"'[^']*'", String),
         (r"#'[^']*'", String.Symbol),
         (r"#\w+:?", String.Symbol),
         (r"#(\+|\/|~|\*|<|>|=|@|%|\||&|\?|!|,|-)+", String.Symbol)

       ],
       'whitespace' : [
         (r'\s+', Text),
         (r'"[^"]*"', Comment)
       ]
    }


class GherkinLexer(RegexLexer):
    """
    For `Gherkin <http://github.com/aslakhellesoy/gherkin/>` syntax.

    *New in Pygments 1.2.*
    """
    name = 'Gherkin'
    aliases = ['Cucumber', 'cucumber', 'Gherkin', 'gherkin']
    filenames = ['*.feature']
    mimetypes = ['text/x-gherkin']

    feature_keywords         = ur'^(기능|機能|功能|フィーチャ|خاصية|תכונה|Функціонал|Функционалност|Функционал|Фича|Особина|Могућност|Özellik|Właściwość|Tính năng|Trajto|Savybė|Požiadavka|Požadavek|Osobina|Ominaisuus|Omadus|OH HAI|Mogućnost|Mogucnost|Jellemző|Fīča|Funzionalità|Funktionalität|Funkcionalnost|Funkcionalitāte|Funcționalitate|Functionaliteit|Functionalitate|Funcionalitat|Funcionalidade|Fonctionnalité|Fitur|Feature|Egenskap|Egenskab|Crikey|Característica|Arwedd)(:)(.*)$'
    feature_element_keywords = ur'^(\s*)(시나리오 개요|시나리오|배경|背景|場景大綱|場景|场景大纲|场景|劇本大綱|劇本|テンプレ|シナリオテンプレート|シナリオテンプレ|シナリオアウトライン|シナリオ|سيناريو مخطط|سيناريو|الخلفية|תרחיש|תבנית תרחיש|רקע|Тарих|Сценарій|Сценарио|Сценарий структураси|Сценарий|Структура сценарію|Структура сценарија|Структура сценария|Скица|Рамка на сценарий|Пример|Предыстория|Предистория|Позадина|Передумова|Основа|Концепт|Контекст|Założenia|Wharrimean is|Tình huống|The thing of it is|Tausta|Taust|Tapausaihio|Tapaus|Szenariogrundriss|Szenario|Szablon scenariusza|Stsenaarium|Struktura scenarija|Skica|Skenario konsep|Skenario|Situācija|Senaryo taslağı|Senaryo|Scénář|Scénario|Schema dello scenario|Scenārijs pēc parauga|Scenārijs|Scenár|Scenaro|Scenariusz|Scenariul de şablon|Scenariul de sablon|Scenariu|Scenario Outline|Scenario Amlinellol|Scenario|Scenarijus|Scenarijaus šablonas|Scenarij|Scenarie|Rerefons|Raamstsenaarium|Primer|Pozadí|Pozadina|Pozadie|Plan du scénario|Plan du Scénario|Osnova scénáře|Osnova|Náčrt Scénáře|Náčrt Scenáru|Mate|MISHUN SRSLY|MISHUN|Kịch bản|Konturo de la scenaro|Kontext|Konteksts|Kontekstas|Kontekst|Koncept|Khung tình huống|Khung kịch bản|Háttér|Grundlage|Geçmiş|Forgatókönyv vázlat|Forgatókönyv|Fono|Esquema do Cenário|Esquema do Cenario|Esquema del escenario|Esquema de l\'escenari|Escenario|Escenari|Dis is what went down|Dasar|Contexto|Contexte|Contesto|Condiţii|Conditii|Cenário|Cenario|Cefndir|Bối cảnh|Blokes|Bakgrunn|Bakgrund|Baggrund|Background|B4|Antecedents|Antecedentes|All y\'all|Achtergrond|Abstrakt Scenario|Abstract Scenario)(:)(.*)$'
    examples_keywords        = ur'^(\s*)(예|例子|例|サンプル|امثلة|דוגמאות|Сценарији|Примери|Приклади|Мисоллар|Значения|Örnekler|Voorbeelden|Variantai|Tapaukset|Scenarios|Scenariji|Scenarijai|Příklady|Példák|Príklady|Przykłady|Primjeri|Primeri|Piemēri|Pavyzdžiai|Paraugs|Juhtumid|Exemplos|Exemples|Exemplele|Exempel|Examples|Esempi|Enghreifftiau|Ekzemploj|Eksempler|Ejemplos|EXAMPLZ|Dữ liệu|Contoh|Cobber|Beispiele)(:)(.*)$'
    step_keywords            = ur'^(\s*)(하지만|조건|먼저|만일|만약|단|그리고|그러면|那麼|那么|而且|當|当|前提|假設|假如|但是|但し|並且|もし|ならば|ただし|しかし|かつ|و |متى |لكن |عندما |ثم |بفرض |اذاً |כאשר |וגם |בהינתן |אזי |אז |אבל |Якщо |Унда |То |Припустимо, що |Припустимо |Онда |Но |Нехай |Лекин |Когато |Када |Кад |К тому же |И |Задато |Задати |Задате |Если |Допустим |Дадено |Ва |Бирок |Аммо |Али |Але |Агар |А |І |Și |És |Zatati |Zakładając |Zadato |Zadate |Zadano |Zadani |Zadan |Youse know when youse got |Youse know like when |Yna |Ya know how |Ya gotta |Y |Wun |Wtedy |When y\'all |When |Wenn |WEN |Và |Ve |Und |Un |Thì |Then y\'all |Then |Tapi |Tak |Tada |Tad |Så |Stel |Soit |Siis |Si |Sed |Se |Quando |Quand |Quan |Pryd |Pokud |Pokiaľ |Però |Pero |Pak |Oraz |Onda |Ond |Oletetaan |Og |Och |O zaman |Når |När |Niin |Nhưng |N |Mutta |Men |Mas |Maka |Majd |Mais |Maar |Ma |Lorsque |Lorsqu\'|Kun |Kuid |Kui |Khi |Keď |Ketika |Když |Kaj |Kai |Kada |Kad |Jeżeli |Ja |Ir |I CAN HAZ |I |Ha |Givun |Givet |Given y\'all |Given |Gitt |Gegeven |Gegeben sei |Fakat |Eğer ki |Etant donné |Et |Então |Entonces |Entao |En |Eeldades |E |Duota |Dun |Donitaĵo |Donat |Donada |Do |Diyelim ki |Dengan |Den youse gotta |De |Dato |Dar |Dann |Dan |Dado |Dacă |Daca |DEN |Când |Cuando |Cho |Cept |Cand |Cal |But y\'all |But |Buh |Biết |Bet |BUT |Atès |Atunci |Atesa |Anrhegedig a |Angenommen |And y\'all |And |An |Ama |Als |Alors |Allora |Ali |Aleshores |Ale |Akkor |Aber |AN |A také |A |\* )'

    tokens = {
        'comments': [
            (r'#.*$', Comment),
          ],
        'feature_elements' : [
            (step_keywords, Keyword, "step_content_stack"),
            include('comments'),
            (r"(\s|.)", Name.Function),
          ],
        'feature_elements_on_stack' : [
            (step_keywords, Keyword, "#pop:2"),
            include('comments'),
            (r"(\s|.)", Name.Function),
          ],
        'examples_table': [
            (r"\s+\|", Keyword, 'examples_table_header'),
            include('comments'),
            (r"(\s|.)", Name.Function),
          ],
        'examples_table_header': [
            (r"\s+\|\s*$", Keyword, "#pop:2"),
            include('comments'),
            (r"\s*\|", Keyword),
            (r"[^\|]", Name.Variable),
          ],
        'scenario_sections_on_stack': [
            (feature_element_keywords, bygroups(Name.Function, Keyword, Keyword, Name.Function), "feature_elements_on_stack"),
          ],
        'narrative': [
            include('scenario_sections_on_stack'),
            (r"(\s|.)", Name.Function),
          ],
        'table_vars': [
            (r'(<[^>]+>)', Name.Variable),
          ],
        'numbers': [
            (r'(\d+\.?\d*|\d*\.\d+)([eE][+-]?[0-9]+)?', String),
          ],
        'string': [
            include('table_vars'),
            (r'(\s|.)', String),
          ],
        'py_string': [
            (r'"""', Keyword, "#pop"),
            include('string'),
          ],
          'step_content_root':[
            (r"$", Keyword, "#pop"),
            include('step_content'),
          ],
          'step_content_stack':[
            (r"$", Keyword, "#pop:2"),
            include('step_content'),
          ],
          'step_content':[
            (r'"', Name.Function, "double_string"),
            include('table_vars'),
            include('numbers'),
            include('comments'),
            (r'(\s|.)', Name.Function),
          ],
          'table_content': [
            (r"\s+\|\s*$", Keyword, "#pop"),
            include('comments'),
            (r"\s*\|", Keyword),
            include('string'),
          ],
        'double_string': [
            (r'"', Name.Function, "#pop"),
            include('string'),
          ],
        'root': [
            (r'\n', Name.Function),
            include('comments'),
            (r'"""', Keyword, "py_string"),
            (r'\s+\|', Keyword, 'table_content'),
            (r'"', Name.Function, "double_string"),
            include('table_vars'),
            include('numbers'),
            (r'(\s*)(@[^@\r\n\t ]+)', bygroups(Name.Function, Name.Tag)),
            (step_keywords, bygroups(Name.Function, Keyword),
             'step_content_root'),
            (feature_keywords, bygroups(Keyword, Keyword, Name.Function),
             'narrative'),
            (feature_element_keywords,
             bygroups(Name.Function, Keyword, Keyword, Name.Function),
             'feature_elements'),
            (examples_keywords,
             bygroups(Name.Function, Keyword, Keyword, Name.Function),
             'examples_table'),
            (r'(\s|.)', Name.Function),
        ]
    }

class AsymptoteLexer(RegexLexer):
    """
    For `Asymptote <http://asymptote.sf.net/>`_ source code.

    *New in Pygments 1.2.*
    """
    name = 'Asymptote'
    aliases = ['asy', 'asymptote']
    filenames = ['*.asy']
    mimetypes = ['text/x-asymptote']

    #: optional Comment or Whitespace
    _ws = r'(?:\s|//.*?\n|/\*.*?\*/)+'

    tokens = {
        'whitespace': [
            (r'\n', Text),
            (r'\s+', Text),
            (r'\\\n', Text), # line continuation
            (r'//(\n|(.|\n)*?[^\\]\n)', Comment),
            (r'/(\\\n)?\*(.|\n)*?\*(\\\n)?/', Comment),
        ],
        'statements': [
            # simple string (TeX friendly)
            (r'"(\\\\|\\"|[^"])*"', String),
            # C style string (with character escapes)
            (r"'", String, 'string'),
            (r'(\d+\.\d*|\.\d+|\d+)[eE][+-]?\d+[lL]?', Number.Float),
            (r'(\d+\.\d*|\.\d+|\d+[fF])[fF]?', Number.Float),
            (r'0x[0-9a-fA-F]+[Ll]?', Number.Hex),
            (r'0[0-7]+[Ll]?', Number.Oct),
            (r'\d+[Ll]?', Number.Integer),
            (r'[~!%^&*+=|?:<>/-]', Operator),
            (r'[()\[\],.]', Punctuation),
            (r'\b(case)(.+?)(:)', bygroups(Keyword, using(this), Text)),
            (r'(and|controls|tension|atleast|curl|if|else|while|for|do|'
             r'return|break|continue|struct|typedef|new|access|import|'
             r'unravel|from|include|quote|static|public|private|restricted|'
             r'this|explicit|true|false|null|cycle|newframe|operator)\b', Keyword),
            # Since an asy-type-name can be also an asy-function-name,
            # in the following we test if the string "  [a-zA-Z]" follows
            # the Keyword.Type.
            # Of course it is not perfect !
            (r'(Braid|FitResult|Label|Legend|TreeNode|abscissa|arc|arrowhead|'
             r'binarytree|binarytreeNode|block|bool|bool3|bounds|bqe|circle|'
             r'conic|coord|coordsys|cputime|ellipse|file|filltype|frame|grid3|'
             r'guide|horner|hsv|hyperbola|indexedTransform|int|inversion|key|'
             r'light|line|linefit|marginT|marker|mass|object|pair|parabola|path|'
             r'path3|pen|picture|point|position|projection|real|revolution|'
             r'scaleT|scientific|segment|side|slice|splitface|string|surface|'
             r'tensionSpecifier|ticklocate|ticksgridT|tickvalues|transform|'
             r'transformation|tree|triangle|trilinear|triple|vector|'
             r'vertex|void)(?=([ ]{1,}[a-zA-Z]))', Keyword.Type),
            # Now the asy-type-name which are not asy-function-name
            # except yours !
            # Perhaps useless
            (r'(Braid|FitResult|TreeNode|abscissa|arrowhead|block|bool|bool3|'
             r'bounds|coord|frame|guide|horner|int|linefit|marginT|pair|pen|'
             r'picture|position|real|revolution|slice|splitface|ticksgridT|'
             r'tickvalues|tree|triple|vertex|void)\b', Keyword.Type),
            ('[a-zA-Z_][a-zA-Z0-9_]*:(?!:)', Name.Label),
            ('[a-zA-Z_][a-zA-Z0-9_]*', Name),
            ],
        'root': [
            include('whitespace'),
            # functions
            (r'((?:[a-zA-Z0-9_*\s])+?(?:\s|\*))'    # return arguments
             r'([a-zA-Z_][a-zA-Z0-9_]*)'             # method name
             r'(\s*\([^;]*?\))'                      # signature
             r'(' + _ws + r')({)',
             bygroups(using(this), Name.Function, using(this), using(this),
                      Punctuation),
             'function'),
            # function declarations
            (r'((?:[a-zA-Z0-9_*\s])+?(?:\s|\*))'    # return arguments
             r'([a-zA-Z_][a-zA-Z0-9_]*)'             # method name
             r'(\s*\([^;]*?\))'                      # signature
             r'(' + _ws + r')(;)',
             bygroups(using(this), Name.Function, using(this), using(this),
                      Punctuation)),
            ('', Text, 'statement'),
        ],
        'statement' : [
            include('whitespace'),
            include('statements'),
            ('[{}]', Punctuation),
            (';', Punctuation, '#pop'),
        ],
        'function': [
            include('whitespace'),
            include('statements'),
            (';', Punctuation),
            ('{', Punctuation, '#push'),
            ('}', Punctuation, '#pop'),
        ],
        'string': [
            (r"'", String, '#pop'),
            (r'\\([\\abfnrtv"\'?]|x[a-fA-F0-9]{2,4}|[0-7]{1,3})', String.Escape),
            (r'\n', String),
            (r"[^\\'\n]+", String), # all other characters
            (r'\\\n', String),
            (r'\\n', String), # line continuation
            (r'\\', String), # stray backslash
            ]
        }

    def get_tokens_unprocessed(self, text):
        from pygments.lexers._asybuiltins import ASYFUNCNAME, ASYVARNAME
        for index, token, value in \
               RegexLexer.get_tokens_unprocessed(self, text):
           if token is Name and value in ASYFUNCNAME:
               token = Name.Function
           elif token is Name and value in ASYVARNAME:
               token = Name.Variable
           yield index, token, value


class PostScriptLexer(RegexLexer):
    """
    Lexer for PostScript files.

    The PostScript Language Reference published by Adobe at
    <http://partners.adobe.com/public/developer/en/ps/PLRM.pdf>
    is the authority for this.

    *New in Pygments 1.4.*
    """
    name = 'PostScript'
    aliases = ['postscript']
    filenames = ['*.ps', '*.eps']
    mimetypes = ['application/postscript']

    delimiter = r'\(\)\<\>\[\]\{\}\/\%\s'
    delimiter_end = r'(?=[%s])' % delimiter

    valid_name_chars = r'[^%s]' % delimiter
    valid_name = r"%s+%s" % (valid_name_chars, delimiter_end)

    tokens = {
        'root': [
            # All comment types
            (r'^%!.+\n', Comment.Preproc),
            (r'%%.*\n', Comment.Special),
            (r'(^%.*\n){2,}', Comment.Multiline),
            (r'%.*\n', Comment.Single),

            # String literals are awkward; enter separate state.
            (r'\(', String, 'stringliteral'),

            (r'[\{\}(\<\<)(\>\>)\[\]]', Punctuation),

            # Numbers
            (r'<[0-9A-Fa-f]+>' + delimiter_end, Number.Hex),
            # Slight abuse: use Oct to signify any explicit base system
            (r'[0-9]+\#(\-|\+)?([0-9]+\.?|[0-9]*\.[0-9]+|[0-9]+\.[0-9]*)'
             r'((e|E)[0-9]+)?' + delimiter_end, Number.Oct),
            (r'(\-|\+)?([0-9]+\.?|[0-9]*\.[0-9]+|[0-9]+\.[0-9]*)((e|E)[0-9]+)?'
             + delimiter_end, Number.Float),
            (r'(\-|\+)?[0-9]+' + delimiter_end, Number.Integer),

            # References
            (r'\/%s' % valid_name, Name.Variable),

            # Names
            (valid_name, Name.Function),      # Anything else is executed

            # These keywords taken from
            # <http://www.math.ubc.ca/~cass/graphics/manual/pdf/a1.pdf>
            # Is there an authoritative list anywhere that doesn't involve
            # trawling documentation?

            (r'(false|true)' + delimiter_end, Keyword.Constant),

            # Conditionals / flow control
            (r'(eq|ne|ge|gt|le|lt|and|or|not|if|ifelse|for|forall)'
             + delimiter_end, Keyword.Reserved),

            ('(abs|add|aload|arc|arcn|array|atan|begin|bind|ceiling|charpath|'
             'clip|closepath|concat|concatmatrix|copy|cos|currentlinewidth|'
             'currentmatrix|currentpoint|curveto|cvi|cvs|def|defaultmatrix|'
             'dict|dictstackoverflow|div|dtransform|dup|end|exch|exec|exit|exp|'
             'fill|findfont|floor|get|getinterval|grestore|gsave|gt|'
             'identmatrix|idiv|idtransform|index|invertmatrix|itransform|'
             'length|lineto|ln|load|log|loop|matrix|mod|moveto|mul|neg|newpath|'
             'pathforall|pathbbox|pop|print|pstack|put|quit|rand|rangecheck|'
             'rcurveto|repeat|restore|rlineto|rmoveto|roll|rotate|round|run|'
             'save|scale|scalefont|setdash|setfont|setgray|setlinecap|'
             'setlinejoin|setlinewidth|setmatrix|setrgbcolor|shfill|show|'
             'showpage|sin|sqrt|stack|stringwidth|stroke|strokepath|sub|'
             'syntaxerror|transform|translate|truncate|typecheck|undefined|'
             'undefinedfilename|undefinedresult)' + delimiter_end,
             Name.Builtin),

            (r'\s+', Text),
        ],

        'stringliteral': [
            (r'[^\(\)\\]+', String),
            (r'\\', String.Escape, 'escape'),
            (r'\(', String, '#push'),
            (r'\)', String, '#pop'),
        ],

        'escape': [
            (r'([0-8]{3}|n|r|t|b|f|\\|\(|\))?', String.Escape, '#pop'),
        ],
    }


class AutohotkeyLexer(RegexLexer):
    """
    For `autohotkey <http://www.autohotkey.com/>`_ source code.

    *New in Pygments 1.4.*
    """
    name = 'autohotkey'
    aliases = ['ahk']
    filenames = ['*.ahk', '*.ahkl']
    mimetypes = ['text/x-autohotkey']

    tokens = {
        'root': [
            (r'^(\s*)(/\*)', bygroups(Text, Comment.Multiline),
                             'incomment'),
            (r'^(\s*)(\()', bygroups(Text, Generic), 'incontinuation'),
            (r'\s+;.*?$', Comment.Singleline),
            (r'^;.*?$', Comment.Singleline),
            (r'[]{}(),;[]', Punctuation),
            (r'(in|is|and|or|not)\b', Operator.Word),
            (r'\%[a-zA-Z_#@$][a-zA-Z0-9_#@$]*\%', Name.Variable),
            (r'!=|==|:=|\.=|<<|>>|[-~+/*%=<>&^|?:!.]', Operator),
            include('commands'),
            include('labels'),
            include('builtInFunctions'),
            include('builtInVariables'),
            (r'"', String, combined('stringescape', 'dqs')),
            include('numbers'),
            (r'[a-zA-Z_#@$][a-zA-Z0-9_#@$]*', Name),
            (r'\\|\'', Text),
            (r'\`([\,\%\`abfnrtv\-\+;])', String.Escape),
            include('garbage'),
        ],
        'incomment': [
            (r'^\s*\*/', Comment.Multiline, '#pop'),
            (r'[^*/]', Comment.Multiline),
            (r'[*/]', Comment.Multiline)
        ],
        'incontinuation': [
            (r'^\s*\)', Generic, '#pop'),
            (r'[^)]', Generic),
            (r'[)]', Generic),
        ],
        'commands': [
            (r'(?i)^(\s*)(global|local|static|'
             r'#AllowSameLineComments|#ClipboardTimeout|#CommentFlag|'
             r'#ErrorStdOut|#EscapeChar|#HotkeyInterval|#HotkeyModifierTimeout|'
             r'#Hotstring|#IfWinActive|#IfWinExist|#IfWinNotActive|'
             r'#IfWinNotExist|#IncludeAgain|#Include|#InstallKeybdHook|'
             r'#InstallMouseHook|#KeyHistory|#LTrim|#MaxHotkeysPerInterval|'
             r'#MaxMem|#MaxThreads|#MaxThreadsBuffer|#MaxThreadsPerHotkey|'
             r'#NoEnv|#NoTrayIcon|#Persistent|#SingleInstance|#UseHook|'
             r'#WinActivateForce|AutoTrim|BlockInput|Break|Click|ClipWait|'
             r'Continue|Control|ControlClick|ControlFocus|ControlGetFocus|'
             r'ControlGetPos|ControlGetText|ControlGet|ControlMove|ControlSend|'
             r'ControlSendRaw|ControlSetText|CoordMode|Critical|'
             r'DetectHiddenText|DetectHiddenWindows|Drive|DriveGet|'
             r'DriveSpaceFree|Edit|Else|EnvAdd|EnvDiv|EnvGet|EnvMult|EnvSet|'
             r'EnvSub|EnvUpdate|Exit|ExitApp|FileAppend|'
             r'FileCopy|FileCopyDir|FileCreateDir|FileCreateShortcut|'
             r'FileDelete|FileGetAttrib|FileGetShortcut|FileGetSize|'
             r'FileGetTime|FileGetVersion|FileInstall|FileMove|FileMoveDir|'
             r'FileRead|FileReadLine|FileRecycle|FileRecycleEmpty|'
             r'FileRemoveDir|FileSelectFile|FileSelectFolder|FileSetAttrib|'
             r'FileSetTime|FormatTime|GetKeyState|Gosub|Goto|GroupActivate|'
             r'GroupAdd|GroupClose|GroupDeactivate|Gui|GuiControl|'
             r'GuiControlGet|Hotkey|IfEqual|IfExist|IfGreaterOrEqual|IfGreater|'
             r'IfInString|IfLess|IfLessOrEqual|IfMsgBox|IfNotEqual|IfNotExist|'
             r'IfNotInString|IfWinActive|IfWinExist|IfWinNotActive|'
             r'IfWinNotExist|If |ImageSearch|IniDelete|IniRead|IniWrite|'
             r'InputBox|Input|KeyHistory|KeyWait|ListHotkeys|ListLines|'
             r'ListVars|Loop|Menu|MouseClickDrag|MouseClick|MouseGetPos|'
             r'MouseMove|MsgBox|OnExit|OutputDebug|Pause|PixelGetColor|'
             r'PixelSearch|PostMessage|Process|Progress|Random|RegDelete|'
             r'RegRead|RegWrite|Reload|Repeat|Return|RunAs|RunWait|Run|'
             r'SendEvent|SendInput|SendMessage|SendMode|SendPlay|SendRaw|Send|'
             r'SetBatchLines|SetCapslockState|SetControlDelay|'
             r'SetDefaultMouseSpeed|SetEnv|SetFormat|SetKeyDelay|'
             r'SetMouseDelay|SetNumlockState|SetScrollLockState|'
             r'SetStoreCapslockMode|SetTimer|SetTitleMatchMode|'
             r'SetWinDelay|SetWorkingDir|Shutdown|Sleep|Sort|SoundBeep|'
             r'SoundGet|SoundGetWaveVolume|SoundPlay|SoundSet|'
             r'SoundSetWaveVolume|SplashImage|SplashTextOff|SplashTextOn|'
             r'SplitPath|StatusBarGetText|StatusBarWait|StringCaseSense|'
             r'StringGetPos|StringLeft|StringLen|StringLower|StringMid|'
             r'StringReplace|StringRight|StringSplit|StringTrimLeft|'
             r'StringTrimRight|StringUpper|Suspend|SysGet|Thread|ToolTip|'
             r'Transform|TrayTip|URLDownloadToFile|While|WinActivate|'
             r'WinActivateBottom|WinClose|WinGetActiveStats|WinGetActiveTitle|'
             r'WinGetClass|WinGetPos|WinGetText|WinGetTitle|WinGet|WinHide|'
             r'WinKill|WinMaximize|WinMenuSelectItem|WinMinimizeAllUndo|'
             r'WinMinimizeAll|WinMinimize|WinMove|WinRestore|WinSetTitle|'
             r'WinSet|WinShow|WinWaitActive|WinWaitClose|WinWaitNotActive|'
             r'WinWait)\b', bygroups(Text, Name.Builtin)),
        ],
        'builtInFunctions': [
            (r'(?i)(Abs|ACos|Asc|ASin|ATan|Ceil|Chr|Cos|DllCall|Exp|FileExist|'
             r'Floor|GetKeyState|IL_Add|IL_Create|IL_Destroy|InStr|IsFunc|'
             r'IsLabel|Ln|Log|LV_Add|LV_Delete|LV_DeleteCol|LV_GetCount|'
             r'LV_GetNext|LV_GetText|LV_Insert|LV_InsertCol|LV_Modify|'
             r'LV_ModifyCol|LV_SetImageList|Mod|NumGet|NumPut|OnMessage|'
             r'RegExMatch|RegExReplace|RegisterCallback|Round|SB_SetIcon|'
             r'SB_SetParts|SB_SetText|Sin|Sqrt|StrLen|SubStr|Tan|TV_Add|'
             r'TV_Delete|TV_GetChild|TV_GetCount|TV_GetNext|TV_Get|'
             r'TV_GetParent|TV_GetPrev|TV_GetSelection|TV_GetText|TV_Modify|'
             r'VarSetCapacity|WinActive|WinExist|Object|ComObjActive|'
             r'ComObjArray|ComObjEnwrap|ComObjUnwrap|ComObjParameter|'
             r'ComObjType|ComObjConnect|ComObjCreate|ComObjGet|ComObjError|'
             r'ComObjValue|Insert|MinIndex|MaxIndex|Remove|SetCapacity|'
             r'GetCapacity|GetAddress|_NewEnum|FileOpen|Read|Write|ReadLine|'
             r'WriteLine|ReadNumType|WriteNumType|RawRead|RawWrite|Seek|Tell|'
             r'Close|Next|IsObject|StrPut|StrGet|Trim|LTrim|RTrim)\b',
             Name.Function),
        ],
        'builtInVariables': [
            (r'(?i)(A_AhkPath|A_AhkVersion|A_AppData|A_AppDataCommon|'
             r'A_AutoTrim|A_BatchLines|A_CaretX|A_CaretY|A_ComputerName|'
             r'A_ControlDelay|A_Cursor|A_DDDD|A_DDD|A_DD|A_DefaultMouseSpeed|'
             r'A_Desktop|A_DesktopCommon|A_DetectHiddenText|'
             r'A_DetectHiddenWindows|A_EndChar|A_EventInfo|A_ExitReason|'
             r'A_FormatFloat|A_FormatInteger|A_Gui|A_GuiEvent|A_GuiControl|'
             r'A_GuiControlEvent|A_GuiHeight|A_GuiWidth|A_GuiX|A_GuiY|A_Hour|'
             r'A_IconFile|A_IconHidden|A_IconNumber|A_IconTip|A_Index|'
             r'A_IPAddress1|A_IPAddress2|A_IPAddress3|A_IPAddress4|A_ISAdmin|'
             r'A_IsCompiled|A_IsCritical|A_IsPaused|A_IsSuspended|A_KeyDelay|'
             r'A_Language|A_LastError|A_LineFile|A_LineNumber|A_LoopField|'
             r'A_LoopFileAttrib|A_LoopFileDir|A_LoopFileExt|A_LoopFileFullPath|'
             r'A_LoopFileLongPath|A_LoopFileName|A_LoopFileShortName|'
             r'A_LoopFileShortPath|A_LoopFileSize|A_LoopFileSizeKB|'
             r'A_LoopFileSizeMB|A_LoopFileTimeAccessed|A_LoopFileTimeCreated|'
             r'A_LoopFileTimeModified|A_LoopReadLine|A_LoopRegKey|'
             r'A_LoopRegName|A_LoopRegSubkey|A_LoopRegTimeModified|'
             r'A_LoopRegType|A_MDAY|A_Min|A_MM|A_MMM|A_MMMM|A_Mon|A_MouseDelay|'
             r'A_MSec|A_MyDocuments|A_Now|A_NowUTC|A_NumBatchLines|A_OSType|'
             r'A_OSVersion|A_PriorHotkey|A_ProgramFiles|A_Programs|'
             r'A_ProgramsCommon|A_ScreenHeight|A_ScreenWidth|A_ScriptDir|'
             r'A_ScriptFullPath|A_ScriptName|A_Sec|A_Space|A_StartMenu|'
             r'A_StartMenuCommon|A_Startup|A_StartupCommon|A_StringCaseSense|'
             r'A_Tab|A_Temp|A_ThisFunc|A_ThisHotkey|A_ThisLabel|A_ThisMenu|'
             r'A_ThisMenuItem|A_ThisMenuItemPos|A_TickCount|A_TimeIdle|'
             r'A_TimeIdlePhysical|A_TimeSincePriorHotkey|A_TimeSinceThisHotkey|'
             r'A_TitleMatchMode|A_TitleMatchModeSpeed|A_UserName|A_WDay|'
             r'A_WinDelay|A_WinDir|A_WorkingDir|A_YDay|A_YEAR|A_YWeek|A_YYYY|'
             r'Clipboard|ClipboardAll|ComSpec|ErrorLevel|ProgramFiles|True|'
             r'False|A_IsUnicode|A_FileEncoding|A_OSVersion|A_PtrSize)\b',
             Name.Variable),
        ],
        'labels': [
            # hotkeys and labels
            # technically, hotkey names are limited to named keys and buttons
            (r'(^\s*)([^:\s\(\"]+?:{1,2})', bygroups(Text, Name.Label)),
            (r'(^\s*)(::[^:\s]+?::)', bygroups(Text, Name.Label)),
        ],
        'numbers': [
            (r'(\d+\.\d*|\d*\.\d+)([eE][+-]?[0-9]+)?', Number.Float),
            (r'\d+[eE][+-]?[0-9]+', Number.Float),
            (r'0\d+', Number.Oct),
            (r'0[xX][a-fA-F0-9]+', Number.Hex),
            (r'\d+L', Number.Integer.Long),
            (r'\d+', Number.Integer)
        ],
        'stringescape': [
            (r'\"\"|\`([\,\%\`abfnrtv])', String.Escape),
        ],
        'strings': [
            (r'[^"\n]+', String),
        ],
        'dqs': [
            (r'"', String, '#pop'),
            include('strings')
        ],
        'garbage': [
            (r'[^\S\n]', Text),
            # (r'.', Text),      # no cheating
        ],
    }


class MaqlLexer(RegexLexer):
    """
    Lexer for `GoodData MAQL
    <https://secure.gooddata.com/docs/html/advanced.metric.tutorial.html>`_
    scripts.

    *New in Pygments 1.4.*
    """

    name = 'MAQL'
    aliases = ['maql']
    filenames = ['*.maql']
    mimetypes = ['text/x-gooddata-maql','application/x-gooddata-maql']

    flags = re.IGNORECASE
    tokens = {
        'root': [
            # IDENTITY
            (r'IDENTIFIER\b', Name.Builtin),
            # IDENTIFIER
            (r'\{[^}]+\}', Name.Variable),
            # NUMBER
            (r'[0-9]+(?:\.[0-9]+)?(?:[eE][+-]?[0-9]{1,3})?', Literal.Number),
            # STRING
            (r'"', Literal.String, 'string-literal'),
            #  RELATION
            (r'\<\>|\!\=', Operator),
            (r'\=|\>\=|\>|\<\=|\<', Operator),
            # :=
            (r'\:\=', Operator),
            # OBJECT
            (r'\[[^]]+\]', Name.Variable.Class),
            # keywords
            (r'(DIMENSIONS?|BOTTOM|METRIC|COUNT|OTHER|FACT|WITH|TOP|OR|'
             r'ATTRIBUTE|CREATE|PARENT|FALSE|ROWS?|FROM|ALL|AS|PF|'
             r'COLUMNS?|DEFINE|REPORT|LIMIT|TABLE|LIKE|AND|BY|'
             r'BETWEEN|EXCEPT|SELECT|MATCH|WHERE|TRUE|FOR|IN|'
             r'WITHOUT|FILTER|ALIAS|ORDER|FACT|WHEN|NOT|ON|'
             r'KEYS|KEY|FULLSET|PRIMARY|LABELS|LABEL|VISUAL|'
             r'TITLE|DESCRIPTION|FOLDER|ALTER|DROP|ADD|DATASET|'
             r'DATATYPE|INT|BIGINT|DOUBLE|DATE|VARCHAR|DECIMAL|'
             r'SYNCHRONIZE|TYPE|DEFAULT|ORDER|ASC|DESC|HYPERLINK|'
             r'INCLUDE|TEMPLATE|MODIFY)\b', Keyword),
            # FUNCNAME
            (r'[a-zA-Z]\w*\b', Name.Function),
            # Comments
            (r'#.*', Comment.Single),
            # Punctuation
            (r'[,;\(\)]', Token.Punctuation),
            # Space is not significant
            (r'\s+', Text)
        ],
        'string-literal': [
            (r'\\[tnrfbae"\\]', String.Escape),
            (r'"', Literal.String, '#pop'),
            (r'[^\\"]+', Literal.String)
        ],
    }


class GoodDataCLLexer(RegexLexer):
    """
    Lexer for `GoodData-CL <http://github.com/gooddata/GoodData-CL/raw/master/cli/src/main/resources/com/gooddata/processor/COMMANDS.txt>`_
    script files.

    *New in Pygments 1.4.*
    """

    name = 'GoodData-CL'
    aliases = ['gooddata-cl']
    filenames = ['*.gdc']
    mimetypes = ['text/x-gooddata-cl']

    flags = re.IGNORECASE
    tokens = {
        'root': [
            # Comments
            (r'#.*', Comment.Single),
            # Function call
            (r'[a-zA-Z]\w*', Name.Function),
            # Argument list
            (r'\(', Token.Punctuation, 'args-list'),
            # Punctuation
            (r';', Token.Punctuation),
            # Space is not significant
            (r'\s+', Text)
        ],
        'args-list': [
            (r'\)', Token.Punctuation, '#pop'),
            (r',', Token.Punctuation),
            (r'[a-zA-Z]\w*', Name.Variable),
            (r'=', Operator),
            (r'"', Literal.String, 'string-literal'),
            (r'[0-9]+(?:\.[0-9]+)?(?:[eE][+-]?[0-9]{1,3})?', Literal.Number),
            # Space is not significant
            (r'\s', Text)
        ],
        'string-literal': [
            (r'\\[tnrfbae"\\]', String.Escape),
            (r'"', Literal.String, '#pop'),
            (r'[^\\"]+', Literal.String)
        ]
    }


class ProtoBufLexer(RegexLexer):
    """
    Lexer for `Protocol Buffer <http://code.google.com/p/protobuf/>`_
    definition files.

    *New in Pygments 1.4.*
    """

    name = 'Protocol Buffer'
    aliases = ['protobuf']
    filenames = ['*.proto']

    tokens = {
        'root': [
            (r'[ \t]+', Text),
            (r'[,;{}\[\]\(\)]', Punctuation),
            (r'/(\\\n)?/(\n|(.|\n)*?[^\\]\n)', Comment.Single),
            (r'/(\\\n)?\*(.|\n)*?\*(\\\n)?/', Comment.Multiline),
            (r'\b(import|option|optional|required|repeated|default|packed|'
             r'ctype|extensions|to|max|rpc|returns)\b', Keyword),
            (r'(int32|int64|uint32|uint64|sint32|sint64|'
             r'fixed32|fixed64|sfixed32|sfixed64|'
             r'float|double|bool|string|bytes)\b', Keyword.Type),
            (r'(true|false)\b', Keyword.Constant),
            (r'(package)(\s+)', bygroups(Keyword.Namespace, Text), 'package'),
            (r'(message|extend)(\s+)',
             bygroups(Keyword.Declaration, Text), 'message'),
            (r'(enum|group|service)(\s+)',
             bygroups(Keyword.Declaration, Text), 'type'),
            (r'\".*\"', String),
            (r'(\d+\.\d*|\.\d+|\d+)[eE][+-]?\d+[LlUu]*', Number.Float),
            (r'(\d+\.\d*|\.\d+|\d+[fF])[fF]?', Number.Float),
            (r'(\-?(inf|nan))', Number.Float),
            (r'0x[0-9a-fA-F]+[LlUu]*', Number.Hex),
            (r'0[0-7]+[LlUu]*', Number.Oct),
            (r'\d+[LlUu]*', Number.Integer),
            (r'[+-=]', Operator),
            (r'([a-zA-Z_][a-zA-Z0-9_\.]*)([ \t]*)(=)',
             bygroups(Name.Attribute, Text, Operator)),
            ('[a-zA-Z_][a-zA-Z0-9_\.]*', Name),
        ],
        'package': [
            (r'[a-zA-Z_][a-zA-Z0-9_]*', Name.Namespace, '#pop')
        ],
        'message': [
            (r'[a-zA-Z_][a-zA-Z0-9_]*', Name.Class, '#pop')
        ],
        'type': [
            (r'[a-zA-Z_][a-zA-Z0-9_]*', Name, '#pop')
        ],
    }


class HybrisLexer(RegexLexer):
    """
    For `Hybris <http://www.hybris-lang.org>`_ source code.

    *New in Pygments 1.4.*
    """

    name = 'Hybris'
    aliases = ['hybris', 'hy']
    filenames = ['*.hy', '*.hyb']
    mimetypes = ['text/x-hybris', 'application/x-hybris']

    flags = re.MULTILINE | re.DOTALL

    tokens = {
        'root': [
            # method names
            (r'^(\s*(?:function|method|operator\s+)+?)'
             r'([a-zA-Z_][a-zA-Z0-9_]*)'
             r'(\s*)(\()', bygroups(Keyword, Name.Function, Text, Operator)),
            (r'[^\S\n]+', Text),
            (r'//.*?\n', Comment.Single),
            (r'/\*.*?\*/', Comment.Multiline),
            (r'@[a-zA-Z_][a-zA-Z0-9_\.]*', Name.Decorator),
            (r'(break|case|catch|next|default|do|else|finally|for|foreach|of|'
             r'unless|if|new|return|switch|me|throw|try|while)\b', Keyword),
            (r'(extends|private|protected|public|static|throws|function|method|'
             r'operator)\b', Keyword.Declaration),
            (r'(true|false|null|__FILE__|__LINE__|__VERSION__|__LIB_PATH__|'
             r'__INC_PATH__)\b', Keyword.Constant),
            (r'(class|struct)(\s+)',
             bygroups(Keyword.Declaration, Text), 'class'),
            (r'(import|include)(\s+)',
             bygroups(Keyword.Namespace, Text), 'import'),
            (r'(gc_collect|gc_mm_items|gc_mm_usage|gc_collect_threshold|'
             r'urlencode|urldecode|base64encode|base64decode|sha1|crc32|sha2|'
             r'md5|md5_file|acos|asin|atan|atan2|ceil|cos|cosh|exp|fabs|floor|'
             r'fmod|log|log10|pow|sin|sinh|sqrt|tan|tanh|isint|isfloat|ischar|'
             r'isstring|isarray|ismap|isalias|typeof|sizeof|toint|tostring|'
             r'fromxml|toxml|binary|pack|load|eval|var_names|var_values|'
             r'user_functions|dyn_functions|methods|call|call_method|mknod|'
             r'mkfifo|mount|umount2|umount|ticks|usleep|sleep|time|strtime|'
             r'strdate|dllopen|dlllink|dllcall|dllcall_argv|dllclose|env|exec|'
             r'fork|getpid|wait|popen|pclose|exit|kill|pthread_create|'
             r'pthread_create_argv|pthread_exit|pthread_join|pthread_kill|'
             r'smtp_send|http_get|http_post|http_download|socket|bind|listen|'
             r'accept|getsockname|getpeername|settimeout|connect|server|recv|'
             r'send|close|print|println|printf|input|readline|serial_open|'
             r'serial_fcntl|serial_get_attr|serial_get_ispeed|serial_get_ospeed|'
             r'serial_set_attr|serial_set_ispeed|serial_set_ospeed|serial_write|'
             r'serial_read|serial_close|xml_load|xml_parse|fopen|fseek|ftell|'
             r'fsize|fread|fwrite|fgets|fclose|file|readdir|pcre_replace|size|'
             r'pop|unmap|has|keys|values|length|find|substr|replace|split|trim|'
             r'remove|contains|join)\b', Name.Builtin),
            (r'(MethodReference|Runner|Dll|Thread|Pipe|Process|Runnable|'
             r'CGI|ClientSocket|Socket|ServerSocket|File|Console|Directory|'
             r'Exception)\b', Keyword.Type),
            (r'"(\\\\|\\"|[^"])*"', String),
            (r"'\\.'|'[^\\]'|'\\u[0-9a-f]{4}'", String.Char),
            (r'(\.)([a-zA-Z_][a-zA-Z0-9_]*)',
             bygroups(Operator, Name.Attribute)),
            (r'[a-zA-Z_][a-zA-Z0-9_]*:', Name.Label),
            (r'[a-zA-Z_\$][a-zA-Z0-9_]*', Name),
            (r'[~\^\*!%&\[\]\(\)\{\}<>\|+=:;,./?\-@]+', Operator),
            (r'[0-9][0-9]*\.[0-9]+([eE][0-9]+)?[fd]?', Number.Float),
            (r'0x[0-9a-f]+', Number.Hex),
            (r'[0-9]+L?', Number.Integer),
            (r'\n', Text),
        ],
        'class': [
            (r'[a-zA-Z_][a-zA-Z0-9_]*', Name.Class, '#pop')
        ],
        'import': [
            (r'[a-zA-Z0-9_.]+\*?', Name.Namespace, '#pop')
        ],
    }


class AwkLexer(RegexLexer):
    """
    For Awk scripts.

    *New in Pygments 1.5.*
    """

    name = 'Awk'
    aliases = ['awk', 'gawk', 'mawk', 'nawk']
    filenames = ['*.awk']
    mimetypes = ['application/x-awk']

    tokens = {
        'commentsandwhitespace': [
            (r'\s+', Text),
            (r'#.*$', Comment.Single)
        ],
        'slashstartsregex': [
            include('commentsandwhitespace'),
            (r'/(\\.|[^[/\\\n]|\[(\\.|[^\]\\\n])*])+/'
             r'\B', String.Regex, '#pop'),
            (r'(?=/)', Text, ('#pop', 'badregex')),
            (r'', Text, '#pop')
        ],
        'badregex': [
            (r'\n', Text, '#pop')
        ],
        'root': [
            (r'^(?=\s|/)', Text, 'slashstartsregex'),
            include('commentsandwhitespace'),
            (r'\+\+|--|\|\||&&|in|\$|!?~|'
             r'(\*\*|[-<>+*%\^/!=])=?', Operator, 'slashstartsregex'),
            (r'[{(\[;,]', Punctuation, 'slashstartsregex'),
            (r'[})\].]', Punctuation),
            (r'(break|continue|do|while|exit|for|if|'
             r'return)\b', Keyword, 'slashstartsregex'),
            (r'function\b', Keyword.Declaration, 'slashstartsregex'),
            (r'(atan2|cos|exp|int|log|rand|sin|sqrt|srand|gensub|gsub|index|'
             r'length|match|split|sprintf|sub|substr|tolower|toupper|close|'
             r'fflush|getline|next|nextfile|print|printf|strftime|systime|'
             r'delete|system)\b', Keyword.Reserved),
            (r'(ARGC|ARGIND|ARGV|CONVFMT|ENVIRON|ERRNO|FIELDWIDTHS|FILENAME|FNR|FS|'
             r'IGNORECASE|NF|NR|OFMT|OFS|ORFS|RLENGTH|RS|RSTART|RT|'
             r'SUBSEP)\b', Name.Builtin),
            (r'[$a-zA-Z_][a-zA-Z0-9_]*', Name.Other),
            (r'[0-9][0-9]*\.[0-9]+([eE][0-9]+)?[fd]?', Number.Float),
            (r'0x[0-9a-fA-F]+', Number.Hex),
            (r'[0-9]+', Number.Integer),
            (r'"(\\\\|\\"|[^"])*"', String.Double),
            (r"'(\\\\|\\'|[^'])*'", String.Single),
        ]
    }


class Cfengine3Lexer(RegexLexer):
    """
    Lexer for `CFEngine3 <http://cfengine.org>`_ policy files.

    *New in Pygments 1.5.*
    """

    name = 'CFEngine3'
    aliases = ['cfengine3', 'cf3']
    filenames = ['*.cf']
    mimetypes = []

    tokens = {
        'root': [
            (r'#.*?\n', Comment),
            (r'(body)(\s+)(\S+)(\s+)(control)',
             bygroups(Keyword, Text, Keyword, Text, Keyword)),
            (r'(body|bundle)(\s+)(\S+)(\s+)(\w+)(\()',
             bygroups(Keyword, Text, Keyword, Text, Name.Function, Punctuation),
             'arglist'),
            (r'(body|bundle)(\s+)(\S+)(\s+)(\w+)',
             bygroups(Keyword, Text, Keyword, Text, Name.Function)),
            (r'(")([^"]+)(")(\s+)(string|slist|int|real)(\s*)(=>)(\s*)',
             bygroups(Punctuation,Name.Variable,Punctuation,
                      Text,Keyword.Type,Text,Operator,Text)),
            (r'(\S+)(\s*)(=>)(\s*)',
             bygroups(Keyword.Reserved,Text,Operator,Text)),
            (r'"', String, 'string'),
            (r'(\w+)(\()', bygroups(Name.Function, Punctuation)),
            (r'([\w.!&|\(\)]+)(::)', bygroups(Name.Class, Punctuation)),
            (r'(\w+)(:)', bygroups(Keyword.Declaration,Punctuation)),
            (r'@[\{\(][^\)\}]+[\}\)]', Name.Variable),
            (r'[(){},;]', Punctuation),
            (r'=>', Operator),
            (r'->', Operator),
            (r'\d+\.\d+', Number.Float),
            (r'\d+', Number.Integer),
            (r'\w+', Name.Function),
            (r'\s+', Text),
        ],
        'string': [
            (r'\$[\{\(]', String.Interpol, 'interpol'),
            (r'\\.', String.Escape),
            (r'"', String, '#pop'),
            (r'\n', String),
            (r'.', String),
        ],
        'interpol': [
            (r'\$[\{\(]', String.Interpol, '#push'),
            (r'[\}\)]', String.Interpol, '#pop'),
            (r'[^\$\{\(\)\}]+', String.Interpol),
        ],
        'arglist': [
            (r'\)', Punctuation, '#pop'),
            (r',', Punctuation),
            (r'\w+', Name.Variable),
            (r'\s+', Text),
        ],
    }


class SnobolLexer(RegexLexer):
    """
    Lexer for the SNOBOL4 programming language.

    Recognizes the common ASCII equivalents of the original SNOBOL4 operators.
    Does not require spaces around binary operators.

    *New in Pygments 1.5.*
    """

    name = "Snobol"
    aliases = ["snobol"]
    filenames = ['*.snobol']
    mimetypes = ['text/x-snobol']

    tokens = {
        # root state, start of line
        # comments, continuation lines, and directives start in column 1
        # as do labels
        'root': [
            (r'\*.*\n', Comment),
            (r'[\+\.] ', Punctuation, 'statement'),
            (r'-.*\n', Comment),
            (r'END\s*\n', Name.Label, 'heredoc'),
            (r'[A-Za-z\$][\w$]*', Name.Label, 'statement'),
            (r'\s+', Text, 'statement'),
        ],
        # statement state, line after continuation or label
        'statement': [
            (r'\s*\n', Text, '#pop'),
            (r'\s+', Text),
            (r'(?<=[^\w.])(LT|LE|EQ|NE|GE|GT|INTEGER|IDENT|DIFFER|LGT|SIZE|'
             r'REPLACE|TRIM|DUPL|REMDR|DATE|TIME|EVAL|APPLY|OPSYN|LOAD|UNLOAD|'
             r'LEN|SPAN|BREAK|ANY|NOTANY|TAB|RTAB|REM|POS|RPOS|FAIL|FENCE|'
             r'ABORT|ARB|ARBNO|BAL|SUCCEED|INPUT|OUTPUT|TERMINAL)(?=[^\w.])',
             Name.Builtin),
            (r'[A-Za-z][\w\.]*', Name),
            # ASCII equivalents of original operators
            # | for the EBCDIC equivalent, ! likewise
            # \ for EBCDIC negation
            (r'\*\*|[\?\$\.!%\*/#+\-@\|&\\=]', Operator),
            (r'"[^"]*"', String),
            (r"'[^']*'", String),
            # Accept SPITBOL syntax for real numbers
            # as well as Macro SNOBOL4
            (r'[0-9]+(?=[^\.EeDd])', Number.Integer),
            (r'[0-9]+(\.[0-9]*)?([EDed][-+]?[0-9]+)?', Number.Float),
            # Goto
            (r':', Punctuation, 'goto'),
            (r'[\(\)<>,;]', Punctuation),
        ],
        # Goto block
        'goto': [
            (r'\s*\n', Text, "#pop:2"),
            (r'\s+', Text),
            (r'F|S', Keyword),
            (r'(\()([A-Za-z][\w.]*)(\))',
             bygroups(Punctuation, Name.Label, Punctuation))
        ],
        # everything after the END statement is basically one
        # big heredoc.
        'heredoc': [
            (r'.*\n', String.Heredoc)
        ]
    }


class UrbiscriptLexer(ExtendedRegexLexer):
    """
    For UrbiScript source code.

    *New in Pygments 1.5.*
    """

    name = 'UrbiScript'
    aliases = ['urbiscript']
    filenames = ['*.u']
    mimetypes = ['application/x-urbiscript']

    flags = re.DOTALL

    ## TODO
    # - handle Experimental and deprecated tags with specific tokens
    # - handle Angles and Durations with specific tokens

    def blob_callback(lexer, match, ctx):
        text_before_blob = match.group(1)
        blob_start = match.group(2)
        blob_size_str = match.group(3)
        blob_size = int(blob_size_str)
        yield match.start(), String, text_before_blob
        ctx.pos += len(text_before_blob)

        # if blob size doesn't match blob format (example : "\B(2)(aaa)")
        # yield blob as a string
        if ctx.text[match.end() + blob_size] != ")":
            result = "\\B(" + blob_size_str + ")("
            yield match.start(), String, result
            ctx.pos += len(result)
            return

        # if blob is well formated, yield as Escape
        blob_text = blob_start + ctx.text[match.end():match.end()+blob_size] + ")"
        yield match.start(), String.Escape, blob_text
        ctx.pos = match.end() + blob_size + 1 # +1 is the ending ")"

    tokens = {
        'root': [
            (r'\s+', Text),
            # comments
            (r'//.*?\n', Comment),
            (r'/\*', Comment.Multiline, 'comment'),
            (r'(?:every|for|loop|while)(?:;|&|\||,)',Keyword),
            (r'(?:assert|at|break|case|catch|closure|compl|continue|'
             r'default|else|enum|every|external|finally|for|freezeif|if|new|'
             r'onleave|return|stopif|switch|this|throw|timeout|try|'
             r'waituntil|whenever|while)\b', Keyword),
            (r'(?:asm|auto|bool|char|const_cast|delete|double|dynamic_cast|'
             r'explicit|export|extern|float|friend|goto|inline|int|'
             r'long|mutable|namespace|register|reinterpret_cast|short|'
             r'signed|sizeof|static_cast|struct|template|typedef|typeid|'
             r'typename|union|unsigned|using|virtual|volatile|'
             r'wchar_t)\b', Keyword.Reserved),
            # deprecated keywords, use a meaningfull token when available
            (r'(?:emit|foreach|internal|loopn|static)\b', Keyword),
            # ignored keywords, use a meaningfull token when available
            (r'(?:private|protected|public)\b', Keyword),
            (r'(?:var|do|const|function|class)\b', Keyword.Declaration),
            (r'(?:true|false|nil|void)\b', Keyword.Constant),
            (r'(?:Barrier|Binary|Boolean|CallMessage|Channel|Code|'
             r'Comparable|Container|Control|Date|Dictionary|Directory|'
             r'Duration|Enumeration|Event|Exception|Executable|File|Finalizable|'
             r'Float|FormatInfo|Formatter|Global|Group|Hash|InputStream|'
             r'IoService|Job|Kernel|Lazy|List|Loadable|Lobby|Location|Logger|Math|'
             r'Mutex|nil|Object|Orderable|OutputStream|Pair|Path|Pattern|Position|'
             r'Primitive|Process|Profile|PseudoLazy|PubSub|RangeIterable|Regexp|'
             r'Semaphore|Server|Singleton|Socket|StackFrame|Stream|String|System|'
             r'Tag|Timeout|Traceable|TrajectoryGenerator|Triplet|Tuple'
             r'|UObject|UValue|UVar)\b', Name.Builtin),
            (r'(?:this)\b', Name.Builtin.Pseudo),
            # don't match single | and &
            (r'(?:[-=+*%/<>~^:]+|\.&?|\|\||&&)', Operator),
            (r'(?:and_eq|and|bitand|bitor|in|not|not_eq|or_eq|or|xor_eq|xor)\b',
             Operator.Word),
            (r'[{}\[\]()]+', Punctuation),
            (r'(?:;|\||,|&|\?|!)+', Punctuation),
            (r'[$a-zA-Z_][a-zA-Z0-9_]*', Name.Other),
            (r'0x[0-9a-fA-F]+', Number.Hex),
            # Float, Integer, Angle and Duration
            (r'(?:[0-9]+(?:(?:\.[0-9]+)?(?:[eE][+-]?[0-9]+)?)?'
             r'((?:rad|deg|grad)|(?:ms|s|min|h|d))?)\b', Number.Float),
            # handle binary blob in strings
            (r'"', String.Double, "string.double"),
            (r"'", String.Single, "string.single"),
        ],
        'string.double': [
            (r'((?:\\\\|\\"|[^"])*?)(\\B\((\d+)\)\()', blob_callback),
            (r'(\\\\|\\"|[^"])*?"', String.Double, '#pop'),
        ],
        'string.single': [
            (r"((?:\\\\|\\'|[^'])*?)(\\B\((\d+)\)\()", blob_callback),
            (r"(\\\\|\\'|[^'])*?'", String.Single, '#pop'),
        ],
        # from http://pygments.org/docs/lexerdevelopment/#changing-states
        'comment': [
            (r'[^*/]', Comment.Multiline),
            (r'/\*', Comment.Multiline, '#push'),
            (r'\*/', Comment.Multiline, '#pop'),
            (r'[*/]', Comment.Multiline),
        ]
    }


class OpenEdgeLexer(RegexLexer):
    """
    Lexer for `OpenEdge ABL (formerly Progress)
    <http://web.progress.com/en/openedge/abl.html>`_ source code.

    *New in Pygments 1.5.*
    """
    name = 'OpenEdge ABL'
    aliases = ['openedge', 'abl', 'progress']
    filenames = ['*.p', '*.cls']
    mimetypes = ['text/x-openedge', 'application/x-openedge']

    types = (r'(?i)(^|(?<=[^0-9a-z_\-]))(CHARACTER|CHAR|CHARA|CHARAC|CHARACT|CHARACTE|'
             r'COM-HANDLE|DATE|DATETIME|DATETIME-TZ|'
             r'DECIMAL|DEC|DECI|DECIM|DECIMA|HANDLE|'
             r'INT64|INTEGER|INT|INTE|INTEG|INTEGE|'
             r'LOGICAL|LONGCHAR|MEMPTR|RAW|RECID|ROWID)\s*($|(?=[^0-9a-z_\-]))')

    keywords = (r'(?i)(^|(?<=[^0-9a-z_\-]))(' +
                r'|'.join(OPENEDGEKEYWORDS) +
                r')\s*($|(?=[^0-9a-z_\-]))')
    tokens = {
        'root': [
            (r'/\*', Comment.Multiline, 'comment'),
            (r'\{', Comment.Preproc, 'preprocessor'),
            (r'\s*&.*', Comment.Preproc),
            (r'0[xX][0-9a-fA-F]+[LlUu]*', Number.Hex),
            (r'(?i)(DEFINE|DEF|DEFI|DEFIN)\b', Keyword.Declaration),
            (types, Keyword.Type),
            (keywords, Name.Builtin),
            (r'"(\\\\|\\"|[^"])*"', String.Double),
            (r"'(\\\\|\\'|[^'])*'", String.Single),
            (r'[0-9][0-9]*\.[0-9]+([eE][0-9]+)?[fd]?', Number.Float),
            (r'[0-9]+', Number.Integer),
            (r'\s+', Text),
            (r'[+*/=-]', Operator),
            (r'[.:()]', Punctuation),
            (r'.', Name.Variable), # Lazy catch-all
        ],
        'comment': [
            (r'[^*/]', Comment.Multiline),
            (r'/\*', Comment.Multiline, '#push'),
            (r'\*/', Comment.Multiline, '#pop'),
            (r'[*/]', Comment.Multiline)
        ],
        'preprocessor': [
            (r'[^{}]', Comment.Preproc),
            (r'{', Comment.Preproc, '#push'),
            (r'}', Comment.Preproc, '#pop'),
        ],
    }


class BroLexer(RegexLexer):
    """
    For `Bro <http://bro-ids.org/>`_ scripts.

    *New in Pygments 1.5.*
    """
    name = 'Bro'
    aliases = ['bro']
    filenames = ['*.bro']

    _hex = r'[0-9a-fA-F_]+'
    _float = r'((\d*\.?\d+)|(\d+\.?\d*))([eE][-+]?\d+)?'
    _h = r'[A-Za-z0-9][-A-Za-z0-9]*'

    tokens = {
        'root': [
            # Whitespace
            (r'^@.*?\n', Comment.Preproc),
            (r'#.*?\n', Comment.Single),
            (r'\n', Text),
            (r'\s+', Text),
            (r'\\\n', Text),
            # Keywords
            (r'(add|alarm|break|case|const|continue|delete|do|else|enum|event'
             r'|export|for|function|if|global|local|module|next'
             r'|of|print|redef|return|schedule|type|when|while)\b', Keyword),
            (r'(addr|any|bool|count|counter|double|file|int|interval|net'
             r'|pattern|port|record|set|string|subnet|table|time|timer'
             r'|vector)\b', Keyword.Type),
            (r'(T|F)\b', Keyword.Constant),
            (r'(&)((?:add|delete|expire)_func|attr|(?:create|read|write)_expire'
             r'|default|disable_print_hook|raw_output|encrypt|group|log'
             r'|mergeable|optional|persistent|priority|redef'
             r'|rotate_(?:interval|size)|synchronized)\b', bygroups(Punctuation,
                 Keyword)),
            (r'\s+module\b', Keyword.Namespace),
            # Addresses, ports and networks
            (r'\d+/(tcp|udp|icmp|unknown)\b', Number),
            (r'(\d+\.){3}\d+', Number),
            (r'(' + _hex + r'){7}' + _hex, Number),
            (r'0x' + _hex + r'(' + _hex + r'|:)*::(' + _hex + r'|:)*', Number),
            (r'((\d+|:)(' + _hex + r'|:)*)?::(' + _hex + r'|:)*', Number),
            (r'(\d+\.\d+\.|(\d+\.){2}\d+)', Number),
            # Hostnames
            (_h + r'(\.' + _h + r')+', String),
            # Numeric
            (_float + r'\s+(day|hr|min|sec|msec|usec)s?\b', Literal.Date),
            (r'0[xX]' + _hex, Number.Hex),
            (_float, Number.Float),
            (r'\d+', Number.Integer),
            (r'/', String.Regex, 'regex'),
            (r'"', String, 'string'),
            # Operators
            (r'[!%*/+:<=>?~|-]', Operator),
            (r'([-+=&|]{2}|[+=!><-]=)', Operator),
            (r'(in|match)\b', Operator.Word),
            (r'[{}()\[\]$.,;]', Punctuation),
            # Identfier
            (r'([_a-zA-Z]\w*)(::)', bygroups(Name, Name.Namespace)),
            (r'[a-zA-Z_][a-zA-Z_0-9]*', Name)
        ],
        'string': [
            (r'"', String, '#pop'),
            (r'\\([\\abfnrtv"\']|x[a-fA-F0-9]{2,4}|[0-7]{1,3})', String.Escape),
            (r'[^\\"\n]+', String),
            (r'\\\n', String),
            (r'\\', String)
        ],
        'regex': [
            (r'/', String.Regex, '#pop'),
            (r'\\[\\nt/]', String.Regex), # String.Escape is too intense here.
            (r'[^\\/\n]+', String.Regex),
            (r'\\\n', String.Regex),
            (r'\\', String.Regex)
        ]
    }


class CbmBasicV2Lexer(RegexLexer):
    """
    For CBM BASIC V2 sources.

    *New in Pygments 1.6.*
    """
    name = 'CBM BASIC V2'
    aliases = ['cbmbas']
    filenames = ['*.bas']

    flags = re.IGNORECASE

    tokens = {
        'root': [
            (r'rem.*\n', Comment.Single),
            (r'\s+', Text),
            (r'new|run|end|for|to|next|step|go(to|sub)?|on|return|stop|cont'
             r'|if|then|input#?|read|wait|load|save|verify|poke|sys|print#?'
             r'|list|clr|cmd|open|close|get#?', Keyword.Reserved),
            (r'data|restore|dim|let|def|fn', Keyword.Declaration),
            (r'tab|spc|sgn|int|abs|usr|fre|pos|sqr|rnd|log|exp|cos|sin|tan|atn'
             r'|peek|len|val|asc|(str|chr|left|right|mid)\$', Name.Builtin),
            (r'[-+*/^<>=]', Operator),
            (r'not|and|or', Operator.Word),
            (r'"[^"\n]*.', String),
            (r'\d+|[-+]?\d*\.\d*(e[-+]?\d+)?', Number.Float),
            (r'[\(\),:;]', Punctuation),
            (r'\w+[$%]?', Name),
        ]
    }

    def analyse_text(self, text):
        # if it starts with a line number, it shouldn't be a "modern" Basic
        # like VB.net
        if re.match(r'\d+', text):
            return True


class MscgenLexer(RegexLexer):
    """
    For `Mscgen <http://www.mcternan.me.uk/mscgen/>`_ files.

    *New in Pygments 1.6.*
    """
    name = 'Mscgen'
    aliases = ['mscgen', 'msc']
    filenames = ['*.msc']

    _var = r'([a-zA-Z0-9_]+|"(?:\\"|[^"])*")'

    tokens = {
        'root': [
            (r'msc\b', Keyword.Type),
            # Options
            (r'(hscale|HSCALE|width|WIDTH|wordwraparcs|WORDWRAPARCS'
             r'|arcgradient|ARCGRADIENT)\b', Name.Property),
            # Operators
            (r'(abox|ABOX|rbox|RBOX|box|BOX|note|NOTE)\b', Operator.Word),
            (r'(\.|-|\|){3}', Keyword),
            (r'(?:-|=|\.|:){2}'
             r'|<<=>>|<->|<=>|<<>>|<:>'
             r'|->|=>>|>>|=>|:>|-x|-X'
             r'|<-|<<=|<<|<=|<:|x-|X-|=', Operator),
            # Names
            (r'\*', Name.Builtin),
            (_var, Name.Variable),
            # Other
            (r'\[', Punctuation, 'attrs'),
            (r'\{|\}|,|;', Punctuation),
            include('comments')
        ],
        'attrs': [
            (r'\]', Punctuation, '#pop'),
            (_var + r'(\s*)(=)(\s*)' + _var,
             bygroups(Name.Attribute, Text.Whitespace, Operator, Text.Whitespace,
                      String)),
            (r',', Punctuation),
            include('comments')
        ],
        'comments': [
            (r'(?://|#).*?\n', Comment.Single),
            (r'/\*(?:.|\n)*?\*/', Comment.Multiline),
            (r'[ \t\r\n]+', Text.Whitespace)
        ]
    }


def _rx_indent(level):
    # Kconfig *always* interprets a tab as 8 spaces, so this is the default.
    # Edit this if you are in an environment where KconfigLexer gets expanded
    # input (tabs expanded to spaces) and the expansion tab width is != 8,
    # e.g. in connection with Trac (trac.ini, [mimeviewer], tab_width).
    # Value range here is 2 <= {tab_width} <= 8.
    tab_width = 8
    # Regex matching a given indentation {level}, assuming that indentation is
    # a multiple of {tab_width}. In other cases there might be problems.
    return r'(?:\t| {1,%s}\t| {%s}){%s}.*\n' % (tab_width-1, tab_width, level)


class KconfigLexer(RegexLexer):
    """
    For Linux-style Kconfig files.

    *New in Pygments 1.6.*
    """

    name = 'Kconfig'
    aliases = ['kconfig', 'menuconfig', 'linux-config', 'kernel-config']
    # Adjust this if new kconfig file names appear in your environment
    filenames = ['Kconfig', '*Config.in*', 'external.in*',
                 'standard-modules.in']
    mimetypes = ['text/x-kconfig']
    # No re.MULTILINE, indentation-aware help text needs line-by-line handling
    flags = 0

    def call_indent(level):
        # If indentation >= {level} is detected, enter state 'indent{level}'
        return (_rx_indent(level), String.Doc, 'indent%s' % level)

    def do_indent(level):
        # Print paragraphs of indentation level >= {level} as String.Doc,
        # ignoring blank lines. Then return to 'root' state.
        return [
            (_rx_indent(level), String.Doc),
            (r'\s*\n', Text),
            (r'', Generic, '#pop:2')
        ]

    tokens = {
        'root': [
            (r'\s+', Text),
            (r'#.*?\n', Comment.Single),
            (r'(mainmenu|config|menuconfig|choice|endchoice|comment|menu|'
             r'endmenu|visible if|if|endif|source|prompt|select|depends on|'
             r'default|range|option)\b', Keyword),
            (r'(---help---|help)[\t ]*\n', Keyword, 'help'),
            (r'(bool|tristate|string|hex|int|defconfig_list|modules|env)\b',
             Name.Builtin),
            (r'[!=&|]', Operator),
            (r'[()]', Punctuation),
            (r'[0-9]+', Number.Integer),
            (r"'(''|[^'])*'", String.Single),
            (r'"(""|[^"])*"', String.Double),
            (r'\S+', Text),
        ],
        # Help text is indented, multi-line and ends when a lower indentation
        # level is detected.
        'help': [
            # Skip blank lines after help token, if any
            (r'\s*\n', Text),
            # Determine the first help line's indentation level heuristically(!).
            # Attention: this is not perfect, but works for 99% of "normal"
            # indentation schemes up to a max. indentation level of 7.
            call_indent(7),
            call_indent(6),
            call_indent(5),
            call_indent(4),
            call_indent(3),
            call_indent(2),
            call_indent(1),
            ('', Text, '#pop'),  # for incomplete help sections without text
        ],
        # Handle text for indentation levels 7 to 1
        'indent7': do_indent(7),
        'indent6': do_indent(6),
        'indent5': do_indent(5),
        'indent4': do_indent(4),
        'indent3': do_indent(3),
        'indent2': do_indent(2),
        'indent1': do_indent(1),
    }


class VGLLexer(RegexLexer):
    """
    For `SampleManager VGL <http://www.thermoscientific.com/samplemanager>`_
    source code.

    *New in Pygments 1.6.*
    """
    name = 'VGL'
    aliases = ['vgl']
    filenames = ['*.rpf']

    flags = re.MULTILINE | re.DOTALL | re.IGNORECASE

    tokens = {
        'root': [
            (r'\{[^\}]*\}', Comment.Multiline),
            (r'declare', Keyword.Constant),
            (r'(if|then|else|endif|while|do|endwhile|and|or|prompt|object'
             r'|create|on|line|with|global|routine|value|endroutine|constant'
             r'|global|set|join|library|compile_option|file|exists|create|copy'
             r'|delete|enable|windows|name|notprotected)(?! *[=<>.,()])',
             Keyword),
            (r'(true|false|null|empty|error|locked)', Keyword.Constant),
            (r'[~\^\*\#!%&\[\]\(\)<>\|+=:;,./?-]', Operator),
            (r'"[^"]*"', String),
            (r'(\.)([a-z_\$][a-z0-9_\$]*)', bygroups(Operator, Name.Attribute)),
            (r'[0-9][0-9]*(\.[0-9]+(e[+\-]?[0-9]+)?)?', Number),
            (r'[a-z_\$][a-z0-9_\$]*', Name),
            (r'[\r\n]+', Text),
            (r'\s+', Text)
        ]
    }


class SourcePawnLexer(RegexLexer):
    """
    For SourcePawn source code with preprocessor directives.

    *New in Pygments 1.6.*
    """
    name = 'SourcePawn'
    aliases = ['sp']
    filenames = ['*.sp']
    mimetypes = ['text/x-sourcepawn']

    #: optional Comment or Whitespace
    _ws = r'(?:\s|//.*?\n|/\*.*?\*/)+'

    tokens = {
        'root': [
            # preprocessor directives: without whitespace
            ('^#if\s+0', Comment.Preproc, 'if0'),
            ('^#', Comment.Preproc, 'macro'),
            # or with whitespace
            ('^' + _ws + r'#if\s+0', Comment.Preproc, 'if0'),
            ('^' + _ws + '#', Comment.Preproc, 'macro'),
            (r'\n', Text),
            (r'\s+', Text),
            (r'\\\n', Text), # line continuation
            (r'/(\\\n)?/(\n|(.|\n)*?[^\\]\n)', Comment.Single),
            (r'/(\\\n)?\*(.|\n)*?\*(\\\n)?/', Comment.Multiline),
            (r'[{}]', Punctuation),
            (r'L?"', String, 'string'),
            (r"L?'(\\.|\\[0-7]{1,3}|\\x[a-fA-F0-9]{1,2}|[^\\\'\n])'", String.Char),
            (r'(\d+\.\d*|\.\d+|\d+)[eE][+-]?\d+[LlUu]*', Number.Float),
            (r'(\d+\.\d*|\.\d+|\d+[fF])[fF]?', Number.Float),
            (r'0x[0-9a-fA-F]+[LlUu]*', Number.Hex),
            (r'0[0-7]+[LlUu]*', Number.Oct),
            (r'\d+[LlUu]*', Number.Integer),
            (r'\*/', Error),
            (r'[~!%^&*+=|?:<>/-]', Operator),
            (r'[()\[\],.;]', Punctuation),
            (r'(case|const|continue|native|'
             r'default|else|enum|for|if|new|operator|'
             r'public|return|sizeof|static|decl|struct|switch)\b', Keyword),
            (r'(bool|Float)\b', Keyword.Type),
            (r'(true|false)\b', Keyword.Constant),
            ('[a-zA-Z_][a-zA-Z0-9_]*', Name),
        ],
        'string': [
            (r'"', String, '#pop'),
            (r'\\([\\abfnrtv"\']|x[a-fA-F0-9]{2,4}|[0-7]{1,3})', String.Escape),
            (r'[^\\"\n]+', String), # all other characters
            (r'\\\n', String), # line continuation
            (r'\\', String), # stray backslash
        ],
        'macro': [
            (r'[^/\n]+', Comment.Preproc),
            (r'/\*(.|\n)*?\*/', Comment.Multiline),
            (r'//.*?\n', Comment.Single, '#pop'),
            (r'/', Comment.Preproc),
            (r'(?<=\\)\n', Comment.Preproc),
            (r'\n', Comment.Preproc, '#pop'),
        ],
        'if0': [
            (r'^\s*#if.*?(?<!\\)\n', Comment.Preproc, '#push'),
            (r'^\s*#endif.*?(?<!\\)\n', Comment.Preproc, '#pop'),
            (r'.*?\n', Comment),
        ]
    }

    SM_TYPES = ['Action', 'bool', 'Float', 'Plugin', 'String', 'any',
                'AdminFlag', 'OverrideType', 'OverrideRule', 'ImmunityType',
                'GroupId', 'AdminId', 'AdmAccessMode', 'AdminCachePart',
                'CookieAccess', 'CookieMenu', 'CookieMenuAction', 'NetFlow',
                'ConVarBounds', 'QueryCookie', 'ReplySource',
                'ConVarQueryResult', 'ConVarQueryFinished', 'Function',
                'Action', 'Identity', 'PluginStatus', 'PluginInfo', 'DBResult',
                'DBBindType', 'DBPriority', 'PropType', 'PropFieldType',
                'MoveType', 'RenderMode', 'RenderFx', 'EventHookMode',
                'EventHook', 'FileType', 'FileTimeMode', 'PathType',
                'ParamType', 'ExecType', 'DialogType', 'Handle', 'KvDataTypes',
                'NominateResult', 'MapChange', 'MenuStyle', 'MenuAction',
                'MenuSource', 'RegexError', 'SDKCallType', 'SDKLibrary',
                'SDKFuncConfSource', 'SDKType', 'SDKPassMethod', 'RayType',
                'TraceEntityFilter', 'ListenOverride', 'SortOrder', 'SortType',
                'SortFunc2D', 'APLRes', 'FeatureType', 'FeatureStatus',
                'SMCResult', 'SMCError', 'TFClassType', 'TFTeam', 'TFCond',
                'TFResourceType', 'Timer', 'TopMenuAction', 'TopMenuObjectType',
                'TopMenuPosition', 'TopMenuObject', 'UserMsg']

    def __init__(self, **options):
        self.smhighlighting = get_bool_opt(options,
                'sourcemod', True)

        self._functions = []
        if self.smhighlighting:
            from pygments.lexers._sourcemodbuiltins import FUNCTIONS
            self._functions.extend(FUNCTIONS)
        RegexLexer.__init__(self, **options)

    def get_tokens_unprocessed(self, text):
        for index, token, value in \
            RegexLexer.get_tokens_unprocessed(self, text):
            if token is Name:
                if self.smhighlighting:
                    if value in self.SM_TYPES:
                        token = Keyword.Type
                    elif value in self._functions:
                        token = Name.Builtin
            yield index, token, value


class PuppetLexer(RegexLexer):
    """
    For `Puppet <http://puppetlabs.com/>`__ configuration DSL.

    *New in Pygments 1.6.*
    """
    name = 'Puppet'
    aliases = ['puppet']
    filenames = ['*.pp']

    tokens = {
        'root': [
            include('comments'),
            include('keywords'),
            include('names'),
            include('numbers'),
            include('operators'),
            include('strings'),

            (r'[]{}:(),;[]', Punctuation),
            (r'[^\S\n]+', Text),
        ],

        'comments': [
            (r'\s*#.*$', Comment),
            (r'/(\\\n)?[*](.|\n)*?[*](\\\n)?/', Comment.Multiline),
        ],

        'operators': [
            (r'(=>|\?|<|>|=|\+|-|/|\*|~|!|\|)', Operator),
            (r'(in|and|or|not)\b', Operator.Word),
        ],

        'names': [
            ('[a-zA-Z_][a-zA-Z0-9_]*', Name.Attribute),
            (r'(\$\S+)(\[)(\S+)(\])', bygroups(Name.Variable, Punctuation,
                                               String, Punctuation)),
            (r'\$\S+', Name.Variable),
        ],

        'numbers': [
            # Copypasta from the Python lexer
            (r'(\d+\.\d*|\d*\.\d+)([eE][+-]?[0-9]+)?j?', Number.Float),
            (r'\d+[eE][+-]?[0-9]+j?', Number.Float),
            (r'0[0-7]+j?', Number.Oct),
            (r'0[xX][a-fA-F0-9]+', Number.Hex),
            (r'\d+L', Number.Integer.Long),
            (r'\d+j?', Number.Integer)
        ],

        'keywords': [
            # Left out 'group' and 'require'
            # Since they're often used as attributes
            (r'(?i)(absent|alert|alias|audit|augeas|before|case|check|class|'
             r'computer|configured|contained|create_resources|crit|cron|debug|'
             r'default|define|defined|directory|else|elsif|emerg|err|exec|'
             r'extlookup|fail|false|file|filebucket|fqdn_rand|generate|host|if|'
             r'import|include|info|inherits|inline_template|installed|'
             r'interface|k5login|latest|link|loglevel|macauthorization|'
             r'mailalias|maillist|mcx|md5|mount|mounted|nagios_command|'
             r'nagios_contact|nagios_contactgroup|nagios_host|'
             r'nagios_hostdependency|nagios_hostescalation|nagios_hostextinfo|'
             r'nagios_hostgroup|nagios_service|nagios_servicedependency|'
             r'nagios_serviceescalation|nagios_serviceextinfo|'
             r'nagios_servicegroup|nagios_timeperiod|node|noop|notice|notify|'
             r'package|present|purged|realize|regsubst|resources|role|router|'
             r'running|schedule|scheduled_task|search|selboolean|selmodule|'
             r'service|sha1|shellquote|split|sprintf|ssh_authorized_key|sshkey|'
             r'stage|stopped|subscribe|tag|tagged|template|tidy|true|undef|'
             r'unmounted|user|versioncmp|vlan|warning|yumrepo|zfs|zone|'
             r'zpool)\b', Keyword),
        ],

        'strings': [
            (r'"([^"])*"', String),
            (r'\'([^\'])*\'', String),
        ],

    }


class NSISLexer(RegexLexer):
    """
    For `NSIS <http://nsis.sourceforge.net/>`_ scripts.

    *New in Pygments 1.6.*
    """
    name = 'NSIS'
    aliases = ['nsis', 'nsi', 'nsh']
    filenames = ['*.nsi', '*.nsh']
    mimetypes = ['text/x-nsis']

    flags = re.IGNORECASE

    tokens = {
        'root': [
            (r'[;\#].*\n', Comment),
            (r"'.*'", String.Single),
            (r'"', String.Double, 'str_double'),
            (r'`', String.Backtick, 'str_backtick'),
            include('macro'),
            include('interpol'),
            include('basic'),
            (r'\$\{[a-z_|][\w|]*\}', Keyword.Pseudo),
            (r'/[a-z_]\w*', Name.Attribute),
            ('.', Text),
        ],
        'basic': [
            (r'(\n)(Function)(\s+)([._a-z][.\w]*)\b',
             bygroups(Text, Keyword, Text, Name.Function)),
            (r'\b([_a-z]\w*)(::)([a-z][a-z0-9]*)\b',
             bygroups(Keyword.Namespace, Punctuation, Name.Function)),
            (r'\b([_a-z]\w*)(:)', bygroups(Name.Label, Punctuation)),
            (r'(\b[ULS]|\B)([\!\<\>=]?=|\<\>?|\>)\B', Operator),
            (r'[|+-]', Operator),
            (r'\\', Punctuation),
            (r'\b(Abort|Add(?:BrandingImage|Size)|'
             r'Allow(?:RootDirInstall|SkipFiles)|AutoCloseWindow|'
             r'BG(?:Font|Gradient)|BrandingText|BringToFront|Call(?:InstDLL)?|'
             r'(?:Sub)?Caption|ChangeUI|CheckBitmap|ClearErrors|CompletedText|'
             r'ComponentText|CopyFiles|CRCCheck|'
             r'Create(?:Directory|Font|Shortcut)|Delete(?:INI(?:Sec|Str)|'
             r'Reg(?:Key|Value))?|DetailPrint|DetailsButtonText|'
             r'Dir(?:Show|Text|Var|Verify)|(?:Disabled|Enabled)Bitmap|'
             r'EnableWindow|EnumReg(?:Key|Value)|Exch|Exec(?:Shell|Wait)?|'
             r'ExpandEnvStrings|File(?:BufSize|Close|ErrorText|Open|'
             r'Read(?:Byte)?|Seek|Write(?:Byte)?)?|'
             r'Find(?:Close|First|Next|Window)|FlushINI|Function(?:End)?|'
             r'Get(?:CurInstType|CurrentAddress|DlgItem|DLLVersion(?:Local)?|'
             r'ErrorLevel|FileTime(?:Local)?|FullPathName|FunctionAddress|'
             r'InstDirError|LabelAddress|TempFileName)|'
             r'Goto|HideWindow|Icon|'
             r'If(?:Abort|Errors|FileExists|RebootFlag|Silent)|'
             r'InitPluginsDir|Install(?:ButtonText|Colors|Dir(?:RegKey)?)|'
             r'Inst(?:ProgressFlags|Type(?:[GS]etText)?)|Int(?:CmpU?|Fmt|Op)|'
             r'IsWindow|LangString(?:UP)?|'
             r'License(?:BkColor|Data|ForceSelection|LangString|Text)|'
             r'LoadLanguageFile|LockWindow|Log(?:Set|Text)|MessageBox|'
             r'MiscButtonText|Name|Nop|OutFile|(?:Uninst)?Page(?:Ex(?:End)?)?|'
             r'PluginDir|Pop|Push|Quit|Read(?:(?:Env|INI|Reg)Str|RegDWORD)|'
             r'Reboot|(?:Un)?RegDLL|Rename|RequestExecutionLevel|ReserveFile|'
             r'Return|RMDir|SearchPath|Section(?:Divider|End|'
             r'(?:(?:Get|Set)(?:Flags|InstTypes|Size|Text))|Group(?:End)?|In)?|'
             r'SendMessage|Set(?:AutoClose|BrandingImage|Compress(?:ionLevel|'
             r'or(?:DictSize)?)?|CtlColors|CurInstType|DatablockOptimize|'
             r'DateSave|Details(?:Print|View)|Error(?:s|Level)|FileAttributes|'
             r'Font|OutPath|Overwrite|PluginUnload|RebootFlag|ShellVarContext|'
             r'Silent|StaticBkColor)|'
             r'Show(?:(?:I|Uni)nstDetails|Window)|Silent(?:Un)?Install|Sleep|'
             r'SpaceTexts|Str(?:CmpS?|Cpy|Len)|SubSection(?:End)?|'
             r'Uninstall(?:ButtonText|(?:Sub)?Caption|EXEName|Icon|Text)|'
             r'UninstPage|Var|VI(?:AddVersionKey|ProductVersion)|WindowIcon|'
             r'Write(?:INIStr|Reg(:?Bin|DWORD|(?:Expand)?Str)|Uninstaller)|'
             r'XPStyle)\b', Keyword),
            (r'\b(CUR|END|(?:FILE_ATTRIBUTE_)?'
             r'(?:ARCHIVE|HIDDEN|NORMAL|OFFLINE|READONLY|SYSTEM|TEMPORARY)|'
             r'HK(CC|CR|CU|DD|LM|PD|U)|'
             r'HKEY_(?:CLASSES_ROOT|CURRENT_(?:CONFIG|USER)|DYN_DATA|'
             r'LOCAL_MACHINE|PERFORMANCE_DATA|USERS)|'
             r'ID(?:ABORT|CANCEL|IGNORE|NO|OK|RETRY|YES)|'
             r'MB_(?:ABORTRETRYIGNORE|DEFBUTTON[1-4]|'
             r'ICON(?:EXCLAMATION|INFORMATION|QUESTION|STOP)|'
             r'OK(?:CANCEL)?|RETRYCANCEL|RIGHT|SETFOREGROUND|TOPMOST|USERICON|'
             r'YESNO(?:CANCEL)?)|SET|SHCTX|'
             r'SW_(?:HIDE|SHOW(?:MAXIMIZED|MINIMIZED|NORMAL))|'
             r'admin|all|auto|both|bottom|bzip2|checkbox|colored|current|false|'
             r'force|hide|highest|if(?:diff|newer)|lastused|leave|left|'
             r'listonly|lzma|nevershow|none|normal|off|on|pop|push|'
             r'radiobuttons|right|show|silent|silentlog|smooth|textonly|top|'
             r'true|try|user|zlib)\b', Name.Constant),
        ],
        'macro': [
            (r'\!(addincludedir(?:dir)?|addplugindir|appendfile|cd|define|'
             r'delfilefile|echo(?:message)?|else|endif|error|execute|'
             r'if(?:macro)?n?(?:def)?|include|insertmacro|macro(?:end)?|packhdr|'
             r'search(?:parse|replace)|system|tempfilesymbol|undef|verbose|'
             r'warning)\b', Comment.Preproc),
        ],
        'interpol': [
            (r'\$(R?[0-9])', Name.Builtin.Pseudo),    # registers
            (r'\$(ADMINTOOLS|APPDATA|CDBURN_AREA|COOKIES|COMMONFILES(?:32|64)|'
            r'DESKTOP|DOCUMENTS|EXE(?:DIR|FILE|PATH)|FAVORITES|FONTS|HISTORY|'
            r'HWNDPARENT|INTERNET_CACHE|LOCALAPPDATA|MUSIC|NETHOOD|PICTURES|'
            r'PLUGINSDIR|PRINTHOOD|PROFILE|PROGRAMFILES(?:32|64)|QUICKLAUNCH|'
            r'RECENT|RESOURCES(?:_LOCALIZED)?|SENDTO|SM(?:PROGRAMS|STARTUP)|'
            r'STARTMENU|SYSDIR|TEMP(?:LATES)?|VIDEOS|WINDIR|\{NSISDIR\})',
             Name.Builtin),
            (r'\$(CMDLINE|INSTDIR|OUTDIR|LANGUAGE)', Name.Variable.Global),
            (r'\$[a-z_]\w*', Name.Variable),
        ],
        'str_double': [
            (r'"', String, '#pop'),
            (r'\$(\\[nrt"]|\$)', String.Escape),
            include('interpol'),
            (r'.', String.Double),
        ],
        'str_backtick': [
            (r'`', String, '#pop'),
            (r'\$(\\[nrt"]|\$)', String.Escape),
            include('interpol'),
            (r'.', String.Double),
        ],
    }


class RPMSpecLexer(RegexLexer):
    """
    For RPM *.spec files

    *New in Pygments 1.6.*
    """

    name = 'RPMSpec'
    aliases = ['spec']
    filenames = ['*.spec']
    mimetypes = ['text/x-rpm-spec']

    _directives = ('(?:package|prep|build|install|clean|check|pre[a-z]*|'
                   'post[a-z]*|trigger[a-z]*|files)')

    tokens = {
        'root': [
            (r'#.*\n', Comment),
            include('basic'),
        ],
        'description': [
            (r'^(%' + _directives + ')(.*)$',
             bygroups(Name.Decorator, Text), '#pop'),
            (r'\n', Text),
            (r'.', Text),
        ],
        'changelog': [
            (r'\*.*\n', Generic.Subheading),
            (r'^(%' + _directives + ')(.*)$',
             bygroups(Name.Decorator, Text), '#pop'),
            (r'\n', Text),
            (r'.', Text),
        ],
        'string': [
            (r'"', String.Double, '#pop'),
            (r'\\([\\abfnrtv"\']|x[a-fA-F0-9]{2,4}|[0-7]{1,3})', String.Escape),
            include('interpol'),
            (r'.', String.Double),
        ],
        'basic': [
            include('macro'),
            (r'(?i)^(Name|Version|Release|Epoch|Summary|Group|License|Packager|'
             r'Vendor|Icon|URL|Distribution|Prefix|Patch[0-9]*|Source[0-9]*|'
             r'Requires\(?[a-z]*\)?|[a-z]+Req|Obsoletes|Provides|Conflicts|'
             r'Build[a-z]+|[a-z]+Arch|Auto[a-z]+)(:)(.*)$',
             bygroups(Generic.Heading, Punctuation, using(this))),
            (r'^%description', Name.Decorator, 'description'),
            (r'^%changelog', Name.Decorator, 'changelog'),
            (r'^(%' + _directives + ')(.*)$', bygroups(Name.Decorator, Text)),
            (r'%(attr|defattr|dir|doc(?:dir)?|setup|config(?:ure)?|'
             r'make(?:install)|ghost|patch[0-9]+|find_lang|exclude|verify)',
             Keyword),
            include('interpol'),
            (r"'.*'", String.Single),
            (r'"', String.Double, 'string'),
            (r'.', Text),
        ],
        'macro': [
            (r'%define.*\n', Comment.Preproc),
            (r'%\{\!\?.*%define.*\}', Comment.Preproc),
            (r'(%(?:if(?:n?arch)?|else(?:if)?|endif))(.*)$',
             bygroups(Comment.Preproc, Text)),
        ],
        'interpol': [
            (r'%\{?__[a-z_]+\}?', Name.Function),
            (r'%\{?_([a-z_]+dir|[a-z_]+path|prefix)\}?', Keyword.Pseudo),
            (r'%\{\?[A-Za-z0-9_]+\}', Name.Variable),
            (r'\$\{?RPM_[A-Z0-9_]+\}?', Name.Variable.Global),
            (r'%\{[a-zA-Z][a-zA-Z0-9_]+\}', Keyword.Constant),
        ]
    }


class AutoItLexer(RegexLexer):
    """
    For `AutoIt <http://www.autoitscript.com/site/autoit/>`_ files.

    AutoIt is a freeware BASIC-like scripting language
    designed for automating the Windows GUI and general scripting

    *New in Pygments 1.6.*
    """
    name = 'AutoIt'
    aliases = ['autoit', 'Autoit']
    filenames = ['*.au3']
    mimetypes = ['text/x-autoit']

    # Keywords, functions, macros from au3.keywords.properties
    # which can be found in AutoIt installed directory, e.g.
    # c:\Program Files (x86)\AutoIt3\SciTE\au3.keywords.properties

    keywords = """\
    #include-once #include #endregion #forcedef #forceref #region
    and byref case continueloop dim do else elseif endfunc endif
    endselect exit exitloop for func global
    if local next not or return select step
    then to until wend while exit""".split()

    functions = """\
    abs acos adlibregister adlibunregister asc ascw asin assign atan
    autoitsetoption autoitwingettitle autoitwinsettitle beep binary binarylen
    binarymid binarytostring bitand bitnot bitor bitrotate bitshift bitxor
    blockinput break call cdtray ceiling chr chrw clipget clipput consoleread
    consolewrite consolewriteerror controlclick controlcommand controldisable
    controlenable controlfocus controlgetfocus controlgethandle controlgetpos
    controlgettext controlhide controllistview controlmove controlsend
    controlsettext controlshow controltreeview cos dec dircopy dircreate
    dirgetsize dirmove dirremove dllcall dllcalladdress dllcallbackfree
    dllcallbackgetptr dllcallbackregister dllclose dllopen dllstructcreate
    dllstructgetdata dllstructgetptr dllstructgetsize dllstructsetdata
    drivegetdrive drivegetfilesystem drivegetlabel drivegetserial drivegettype
    drivemapadd drivemapdel drivemapget drivesetlabel drivespacefree
    drivespacetotal drivestatus envget envset envupdate eval execute exp
    filechangedir fileclose filecopy filecreatentfslink filecreateshortcut
    filedelete fileexists filefindfirstfile filefindnextfile fileflush
    filegetattrib filegetencoding filegetlongname filegetpos filegetshortcut
    filegetshortname filegetsize filegettime filegetversion fileinstall filemove
    fileopen fileopendialog fileread filereadline filerecycle filerecycleempty
    filesavedialog fileselectfolder filesetattrib filesetpos filesettime
    filewrite filewriteline floor ftpsetproxy guicreate guictrlcreateavi
    guictrlcreatebutton guictrlcreatecheckbox guictrlcreatecombo
    guictrlcreatecontextmenu guictrlcreatedate guictrlcreatedummy
    guictrlcreateedit guictrlcreategraphic guictrlcreategroup guictrlcreateicon
    guictrlcreateinput guictrlcreatelabel guictrlcreatelist
    guictrlcreatelistview guictrlcreatelistviewitem guictrlcreatemenu
    guictrlcreatemenuitem guictrlcreatemonthcal guictrlcreateobj
    guictrlcreatepic guictrlcreateprogress guictrlcreateradio
    guictrlcreateslider guictrlcreatetab guictrlcreatetabitem
    guictrlcreatetreeview guictrlcreatetreeviewitem guictrlcreateupdown
    guictrldelete guictrlgethandle guictrlgetstate guictrlread guictrlrecvmsg
    guictrlregisterlistviewsort guictrlsendmsg guictrlsendtodummy
    guictrlsetbkcolor guictrlsetcolor guictrlsetcursor guictrlsetdata
    guictrlsetdefbkcolor guictrlsetdefcolor guictrlsetfont guictrlsetgraphic
    guictrlsetimage guictrlsetlimit guictrlsetonevent guictrlsetpos
    guictrlsetresizing guictrlsetstate guictrlsetstyle guictrlsettip guidelete
    guigetcursorinfo guigetmsg guigetstyle guiregistermsg guisetaccelerators
    guisetbkcolor guisetcoord guisetcursor guisetfont guisethelp guiseticon
    guisetonevent guisetstate guisetstyle guistartgroup guiswitch hex hotkeyset
    httpsetproxy httpsetuseragent hwnd inetclose inetget inetgetinfo inetgetsize
    inetread inidelete iniread inireadsection inireadsectionnames
    inirenamesection iniwrite iniwritesection inputbox int isadmin isarray
    isbinary isbool isdeclared isdllstruct isfloat ishwnd isint iskeyword
    isnumber isobj isptr isstring log memgetstats mod mouseclick mouseclickdrag
    mousedown mousegetcursor mousegetpos mousemove mouseup mousewheel msgbox
    number objcreate objcreateinterface objevent objevent objget objname
    onautoitexitregister onautoitexitunregister opt ping pixelchecksum
    pixelgetcolor pixelsearch pluginclose pluginopen processclose processexists
    processgetstats processlist processsetpriority processwait processwaitclose
    progressoff progresson progressset ptr random regdelete regenumkey
    regenumval regread regwrite round run runas runaswait runwait send
    sendkeepactive seterror setextended shellexecute shellexecutewait shutdown
    sin sleep soundplay soundsetwavevolume splashimageon splashoff splashtexton
    sqrt srandom statusbargettext stderrread stdinwrite stdioclose stdoutread
    string stringaddcr stringcompare stringformat stringfromasciiarray
    stringinstr stringisalnum stringisalpha stringisascii stringisdigit
    stringisfloat stringisint stringislower stringisspace stringisupper
    stringisxdigit stringleft stringlen stringlower stringmid stringregexp
    stringregexpreplace stringreplace stringright stringsplit stringstripcr
    stringstripws stringtoasciiarray stringtobinary stringtrimleft
    stringtrimright stringupper tan tcpaccept tcpclosesocket tcpconnect
    tcplisten tcpnametoip tcprecv tcpsend tcpshutdown tcpstartup timerdiff
    timerinit tooltip traycreateitem traycreatemenu traygetmsg trayitemdelete
    trayitemgethandle trayitemgetstate trayitemgettext trayitemsetonevent
    trayitemsetstate trayitemsettext traysetclick trayseticon traysetonevent
    traysetpauseicon traysetstate traysettooltip traytip ubound udpbind
    udpclosesocket udpopen udprecv udpsend udpshutdown udpstartup vargettype
    winactivate winactive winclose winexists winflash wingetcaretpos
    wingetclasslist wingetclientsize wingethandle wingetpos wingetprocess
    wingetstate wingettext wingettitle winkill winlist winmenuselectitem
    winminimizeall winminimizeallundo winmove winsetontop winsetstate
    winsettitle winsettrans winwait winwaitactive winwaitclose
    winwaitnotactive""".split()

    macros = """\
    @appdatacommondir @appdatadir @autoitexe @autoitpid @autoitversion
    @autoitx64 @com_eventobj @commonfilesdir @compiled @computername @comspec
    @cpuarch @cr @crlf @desktopcommondir @desktopdepth @desktopdir
    @desktopheight @desktoprefresh @desktopwidth @documentscommondir @error
    @exitcode @exitmethod @extended @favoritescommondir @favoritesdir
    @gui_ctrlhandle @gui_ctrlid @gui_dragfile @gui_dragid @gui_dropid
    @gui_winhandle @homedrive @homepath @homeshare @hotkeypressed @hour
    @ipaddress1 @ipaddress2 @ipaddress3 @ipaddress4 @kblayout @lf
    @logondnsdomain @logondomain @logonserver @mday @min @mon @msec @muilang
    @mydocumentsdir @numparams @osarch @osbuild @oslang @osservicepack @ostype
    @osversion @programfilesdir @programscommondir @programsdir @scriptdir
    @scriptfullpath @scriptlinenumber @scriptname @sec @startmenucommondir
    @startmenudir @startupcommondir @startupdir @sw_disable @sw_enable @sw_hide
    @sw_lock @sw_maximize @sw_minimize @sw_restore @sw_show @sw_showdefault
    @sw_showmaximized @sw_showminimized @sw_showminnoactive @sw_showna
    @sw_shownoactivate @sw_shownormal @sw_unlock @systemdir @tab @tempdir
    @tray_id @trayiconflashing @trayiconvisible @username @userprofiledir @wday
    @windowsdir @workingdir @yday @year""".split()

    tokens = {
        'root': [
            (r';.*\n', Comment.Single),
            (r'(#comments-start|#cs).*?(#comments-end|#ce)', Comment.Multiline),
            (r'[\[\]{}(),;]', Punctuation),
            (r'(and|or|not)\b', Operator.Word),
            (r'[\$|@][a-zA-Z_][a-zA-Z0-9_]*', Name.Variable),
            (r'!=|==|:=|\.=|<<|>>|[-~+/*%=<>&^|?:!.]', Operator),
            include('commands'),
            include('labels'),
            include('builtInFunctions'),
            include('builtInMarcros'),
            (r'"', String, combined('stringescape', 'dqs')),
            include('numbers'),
            (r'[a-zA-Z_#@$][a-zA-Z0-9_#@$]*', Name),
            (r'\\|\'', Text),
            (r'\`([\,\%\`abfnrtv\-\+;])', String.Escape),
            (r'_\n', Text), # Line continuation
            include('garbage'),
        ],
        'commands': [
            (r'(?i)(\s*)(%s)\b' % '|'.join(keywords),
            bygroups(Text, Name.Builtin)),
        ],
        'builtInFunctions': [
            (r'(?i)(%s)\b' % '|'.join(functions),
             Name.Function),
        ],
        'builtInMarcros': [
            (r'(?i)(%s)\b' % '|'.join(macros),
             Name.Variable.Global),
        ],
        'labels': [
            # sendkeys
            (r'(^\s*)({\S+?})', bygroups(Text, Name.Label)),
        ],
        'numbers': [
            (r'(\d+\.\d*|\d*\.\d+)([eE][+-]?[0-9]+)?', Number.Float),
            (r'\d+[eE][+-]?[0-9]+', Number.Float),
            (r'0\d+', Number.Oct),
            (r'0[xX][a-fA-F0-9]+', Number.Hex),
            (r'\d+L', Number.Integer.Long),
            (r'\d+', Number.Integer)
        ],
        'stringescape': [
            (r'\"\"|\`([\,\%\`abfnrtv])', String.Escape),
        ],
        'strings': [
            (r'[^"\n]+', String),
        ],
        'dqs': [
            (r'"', String, '#pop'),
            include('strings')
        ],
        'garbage': [
            (r'[^\S\n]', Text),
        ],
    }
