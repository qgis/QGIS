# -*- coding: utf-8 -*-
"""
    pygments.lexers.math
    ~~~~~~~~~~~~~~~~~~~~

    Lexers for math languages.

    :copyright: Copyright 2006-2013 by the Pygments team, see AUTHORS.
    :license: BSD, see LICENSE for details.
"""

import re

from pygments.util import shebang_matches
from pygments.lexer import Lexer, RegexLexer, bygroups, include, \
    combined, do_insertions
from pygments.token import Comment, String, Punctuation, Keyword, Name, \
    Operator, Number, Text, Generic

from pygments.lexers.agile import PythonLexer
from pygments.lexers import _scilab_builtins
from pygments.lexers import _stan_builtins

__all__ = ['JuliaLexer', 'JuliaConsoleLexer', 'MuPADLexer', 'MatlabLexer',
           'MatlabSessionLexer', 'OctaveLexer', 'ScilabLexer', 'NumPyLexer',
           'RConsoleLexer', 'SLexer', 'JagsLexer', 'BugsLexer', 'StanLexer',
           'IDLLexer', 'RdLexer']


class JuliaLexer(RegexLexer):
    """
    For `Julia <http://julialang.org/>`_ source code.

    *New in Pygments 1.6.*
    """
    name = 'Julia'
    aliases = ['julia','jl']
    filenames = ['*.jl']
    mimetypes = ['text/x-julia','application/x-julia']

    builtins = [
        'exit','whos','edit','load','is','isa','isequal','typeof','tuple',
        'ntuple','uid','hash','finalizer','convert','promote','subtype',
        'typemin','typemax','realmin','realmax','sizeof','eps','promote_type',
        'method_exists','applicable','invoke','dlopen','dlsym','system',
        'error','throw','assert','new','Inf','Nan','pi','im',
    ]

    tokens = {
        'root': [
            (r'\n', Text),
            (r'[^\S\n]+', Text),
            (r'#.*$', Comment),
            (r'[]{}:(),;[@]', Punctuation),
            (r'\\\n', Text),
            (r'\\', Text),

            # keywords
            (r'(begin|while|for|in|return|break|continue|'
             r'macro|quote|let|if|elseif|else|try|catch|end|'
             r'bitstype|ccall|do|using|module|import|export|'
             r'importall|baremodule)\b', Keyword),
            (r'(local|global|const)\b', Keyword.Declaration),
            (r'(Bool|Int|Int8|Int16|Int32|Int64|Uint|Uint8|Uint16|Uint32|Uint64'
             r'|Float32|Float64|Complex64|Complex128|Any|Nothing|None)\b',
                Keyword.Type),

            # functions
            (r'(function)((?:\s|\\\s)+)',
                bygroups(Keyword,Name.Function), 'funcname'),

            # types
            (r'(type|typealias|abstract)((?:\s|\\\s)+)',
                bygroups(Keyword,Name.Class), 'typename'),

            # operators
            (r'==|!=|<=|>=|->|&&|\|\||::|<:|[-~+/*%=<>&^|.?!$]', Operator),
            (r'\.\*|\.\^|\.\\|\.\/|\\', Operator),

            # builtins
            ('(' + '|'.join(builtins) + r')\b',  Name.Builtin),

            # backticks
            (r'`(?s).*?`', String.Backtick),

            # chars
            (r"'(\\.|\\[0-7]{1,3}|\\x[a-fA-F0-9]{1,3}|\\u[a-fA-F0-9]{1,4}|"
             r"\\U[a-fA-F0-9]{1,6}|[^\\\'\n])'", String.Char),

            # try to match trailing transpose
            (r'(?<=[.\w\)\]])\'+', Operator),

            # strings
            (r'(?:[IL])"', String, 'string'),
            (r'[E]?"', String, combined('stringescape', 'string')),

            # names
            (r'@[a-zA-Z0-9_.]+', Name.Decorator),
            (r'[a-zA-Z_][a-zA-Z0-9_]*', Name),

            # numbers
            (r'(\d+\.\d*|\d*\.\d+)([eEf][+-]?[0-9]+)?', Number.Float),
            (r'\d+[eEf][+-]?[0-9]+', Number.Float),
            (r'0b[01]+', Number.Binary),
            (r'0o[0-7]+', Number.Oct),
            (r'0x[a-fA-F0-9]+', Number.Hex),
            (r'\d+', Number.Integer)
        ],

        'funcname': [
            ('[a-zA-Z_][a-zA-Z0-9_]*', Name.Function, '#pop'),
            ('\([^\s\w{]{1,2}\)', Operator, '#pop'),
            ('[^\s\w{]{1,2}', Operator, '#pop'),
        ],

        'typename': [
            ('[a-zA-Z_][a-zA-Z0-9_]*', Name.Class, '#pop')
        ],

        'stringescape': [
            (r'\\([\\abfnrtv"\']|\n|N{.*?}|u[a-fA-F0-9]{4}|'
             r'U[a-fA-F0-9]{8}|x[a-fA-F0-9]{2}|[0-7]{1,3})', String.Escape)
        ],

        'string': [
            (r'"', String, '#pop'),
            (r'\\\\|\\"|\\\n', String.Escape), # included here for raw strings
            (r'\$(\([a-zA-Z0-9_]+\))?[-#0 +]*([0-9]+|[*])?(\.([0-9]+|[*]))?',
                String.Interpol),
            (r'[^\\"$]+', String),
            # quotes, dollar signs, and backslashes must be parsed one at a time
            (r'["\\]', String),
            # unhandled string formatting sign
            (r'\$', String)
        ],
    }

    def analyse_text(text):
        return shebang_matches(text, r'julia')


line_re  = re.compile('.*?\n')

class JuliaConsoleLexer(Lexer):
    """
    For Julia console sessions. Modeled after MatlabSessionLexer.

    *New in Pygments 1.6.*
    """
    name = 'Julia console'
    aliases = ['jlcon']

    def get_tokens_unprocessed(self, text):
        jllexer = JuliaLexer(**self.options)

        curcode = ''
        insertions = []

        for match in line_re.finditer(text):
            line = match.group()

            if line.startswith('julia>'):
                insertions.append((len(curcode),
                                   [(0, Generic.Prompt, line[:3])]))
                curcode += line[3:]

            elif line.startswith('      '):

                idx = len(curcode)

                # without is showing error on same line as before...?
                line = "\n" + line
                token = (0, Generic.Traceback, line)
                insertions.append((idx, [token]))

            else:
                if curcode:
                    for item in do_insertions(
                        insertions, jllexer.get_tokens_unprocessed(curcode)):
                        yield item
                    curcode = ''
                    insertions = []

                yield match.start(), Generic.Output, line

        if curcode: # or item:
            for item in do_insertions(
                insertions, jllexer.get_tokens_unprocessed(curcode)):
                yield item


class MuPADLexer(RegexLexer):
    """
    A `MuPAD <http://www.mupad.com>`_ lexer.
    Contributed by Christopher Creutzig <christopher@creutzig.de>.

    *New in Pygments 0.8.*
    """
    name = 'MuPAD'
    aliases = ['mupad']
    filenames = ['*.mu']

    tokens = {
      'root' : [
        (r'//.*?$', Comment.Single),
        (r'/\*', Comment.Multiline, 'comment'),
        (r'"(?:[^"\\]|\\.)*"', String),
        (r'\(|\)|\[|\]|\{|\}', Punctuation),
        (r'''(?x)\b(?:
            next|break|end|
            axiom|end_axiom|category|end_category|domain|end_domain|inherits|
            if|%if|then|elif|else|end_if|
            case|of|do|otherwise|end_case|
            while|end_while|
            repeat|until|end_repeat|
            for|from|to|downto|step|end_for|
            proc|local|option|save|begin|end_proc|
            delete|frame
          )\b''', Keyword),
        (r'''(?x)\b(?:
            DOM_ARRAY|DOM_BOOL|DOM_COMPLEX|DOM_DOMAIN|DOM_EXEC|DOM_EXPR|
            DOM_FAIL|DOM_FLOAT|DOM_FRAME|DOM_FUNC_ENV|DOM_HFARRAY|DOM_IDENT|
            DOM_INT|DOM_INTERVAL|DOM_LIST|DOM_NIL|DOM_NULL|DOM_POLY|DOM_PROC|
            DOM_PROC_ENV|DOM_RAT|DOM_SET|DOM_STRING|DOM_TABLE|DOM_VAR
          )\b''', Name.Class),
        (r'''(?x)\b(?:
            PI|EULER|E|CATALAN|
            NIL|FAIL|undefined|infinity|
            TRUE|FALSE|UNKNOWN
          )\b''',
          Name.Constant),
        (r'\b(?:dom|procname)\b', Name.Builtin.Pseudo),
        (r'\.|,|:|;|=|\+|-|\*|/|\^|@|>|<|\$|\||!|\'|%|~=', Operator),
        (r'''(?x)\b(?:
            and|or|not|xor|
            assuming|
            div|mod|
            union|minus|intersect|in|subset
          )\b''',
          Operator.Word),
        (r'\b(?:I|RDN_INF|RD_NINF|RD_NAN)\b', Number),
        #(r'\b(?:adt|linalg|newDomain|hold)\b', Name.Builtin),
        (r'''(?x)
          ((?:[a-zA-Z_#][a-zA-Z_#0-9]*|`[^`]*`)
          (?:::[a-zA-Z_#][a-zA-Z_#0-9]*|`[^`]*`)*)(\s*)([(])''',
          bygroups(Name.Function, Text, Punctuation)),
        (r'''(?x)
          (?:[a-zA-Z_#][a-zA-Z_#0-9]*|`[^`]*`)
          (?:::[a-zA-Z_#][a-zA-Z_#0-9]*|`[^`]*`)*''', Name.Variable),
        (r'[0-9]+(?:\.[0-9]*)?(?:e[0-9]+)?', Number),
        (r'\.[0-9]+(?:e[0-9]+)?', Number),
        (r'.', Text)
      ],
      'comment' : [
        (r'[^*/]', Comment.Multiline),
        (r'/\*', Comment.Multiline, '#push'),
        (r'\*/', Comment.Multiline, '#pop'),
        (r'[*/]', Comment.Multiline)
      ]
    }


class MatlabLexer(RegexLexer):
    """
    For Matlab source code.

    *New in Pygments 0.10.*
    """
    name = 'Matlab'
    aliases = ['matlab']
    filenames = ['*.m']
    mimetypes = ['text/matlab']

    #
    # These lists are generated automatically.
    # Run the following in bash shell:
    #
    # for f in elfun specfun elmat; do
    #   echo -n "$f = "
    #   matlab -nojvm -r "help $f;exit;" | perl -ne \
    #   'push(@c,$1) if /^    (\w+)\s+-/; END {print q{["}.join(q{","},@c).qq{"]\n};}'
    # done
    #
    # elfun: Elementary math functions
    # specfun: Special Math functions
    # elmat: Elementary matrices and matrix manipulation
    #
    # taken from Matlab version 7.4.0.336 (R2007a)
    #
    elfun = ["sin","sind","sinh","asin","asind","asinh","cos","cosd","cosh",
             "acos","acosd","acosh","tan","tand","tanh","atan","atand","atan2",
             "atanh","sec","secd","sech","asec","asecd","asech","csc","cscd",
             "csch","acsc","acscd","acsch","cot","cotd","coth","acot","acotd",
             "acoth","hypot","exp","expm1","log","log1p","log10","log2","pow2",
             "realpow","reallog","realsqrt","sqrt","nthroot","nextpow2","abs",
             "angle","complex","conj","imag","real","unwrap","isreal","cplxpair",
             "fix","floor","ceil","round","mod","rem","sign"]
    specfun = ["airy","besselj","bessely","besselh","besseli","besselk","beta",
               "betainc","betaln","ellipj","ellipke","erf","erfc","erfcx",
               "erfinv","expint","gamma","gammainc","gammaln","psi","legendre",
               "cross","dot","factor","isprime","primes","gcd","lcm","rat",
               "rats","perms","nchoosek","factorial","cart2sph","cart2pol",
               "pol2cart","sph2cart","hsv2rgb","rgb2hsv"]
    elmat = ["zeros","ones","eye","repmat","rand","randn","linspace","logspace",
             "freqspace","meshgrid","accumarray","size","length","ndims","numel",
             "disp","isempty","isequal","isequalwithequalnans","cat","reshape",
             "diag","blkdiag","tril","triu","fliplr","flipud","flipdim","rot90",
             "find","end","sub2ind","ind2sub","bsxfun","ndgrid","permute",
             "ipermute","shiftdim","circshift","squeeze","isscalar","isvector",
             "ans","eps","realmax","realmin","pi","i","inf","nan","isnan",
             "isinf","isfinite","j","why","compan","gallery","hadamard","hankel",
             "hilb","invhilb","magic","pascal","rosser","toeplitz","vander",
             "wilkinson"]

    tokens = {
        'root': [
            # line starting with '!' is sent as a system command.  not sure what
            # label to use...
            (r'^!.*', String.Other),
            (r'%\{\s*\n', Comment.Multiline, 'blockcomment'),
            (r'%.*$', Comment),
            (r'^\s*function', Keyword, 'deffunc'),

            # from 'iskeyword' on version 7.11 (R2010):
            (r'(break|case|catch|classdef|continue|else|elseif|end|enumerated|'
             r'events|for|function|global|if|methods|otherwise|parfor|'
             r'persistent|properties|return|spmd|switch|try|while)\b', Keyword),

            ("(" + "|".join(elfun+specfun+elmat) + r')\b',  Name.Builtin),

            # line continuation with following comment:
            (r'\.\.\..*$', Comment),

            # operators:
            (r'-|==|~=|<|>|<=|>=|&&|&|~|\|\|?', Operator),
            # operators requiring escape for re:
            (r'\.\*|\*|\+|\.\^|\.\\|\.\/|\/|\\', Operator),

            # punctuation:
            (r'\[|\]|\(|\)|\{|\}|:|@|\.|,', Punctuation),
            (r'=|:|;', Punctuation),

            # quote can be transpose, instead of string:
            # (not great, but handles common cases...)
            (r'(?<=[\w\)\]])\'', Operator),

            (r'(\d+\.\d*|\d*\.\d+)([eEf][+-]?[0-9]+)?', Number.Float),
            (r'\d+[eEf][+-]?[0-9]+', Number.Float),
            (r'\d+', Number.Integer),

            (r'(?<![\w\)\]])\'', String, 'string'),
            ('[a-zA-Z_][a-zA-Z0-9_]*', Name),
            (r'.', Text),
        ],
        'string': [
            (r'[^\']*\'', String, '#pop')
        ],
        'blockcomment': [
            (r'^\s*%\}', Comment.Multiline, '#pop'),
            (r'^.*\n', Comment.Multiline),
            (r'.', Comment.Multiline),
        ],
        'deffunc': [
            (r'(\s*)(?:(.+)(\s*)(=)(\s*))?(.+)(\()(.*)(\))(\s*)',
             bygroups(Text.Whitespace, Text, Text.Whitespace, Punctuation,
                      Text.Whitespace, Name.Function, Punctuation, Text,
                      Punctuation, Text.Whitespace), '#pop'),
        ],
    }

    def analyse_text(text):
        if re.match('^\s*%', text, re.M): # comment
            return 0.9
        elif re.match('^!\w+', text, re.M): # system cmd
            return 0.9
        return 0.1


line_re  = re.compile('.*?\n')

class MatlabSessionLexer(Lexer):
    """
    For Matlab sessions.  Modeled after PythonConsoleLexer.
    Contributed by Ken Schutte <kschutte@csail.mit.edu>.

    *New in Pygments 0.10.*
    """
    name = 'Matlab session'
    aliases = ['matlabsession']

    def get_tokens_unprocessed(self, text):
        mlexer = MatlabLexer(**self.options)

        curcode = ''
        insertions = []

        for match in line_re.finditer(text):
            line = match.group()

            if line.startswith('>>'):
                insertions.append((len(curcode),
                                   [(0, Generic.Prompt, line[:3])]))
                curcode += line[3:]

            elif line.startswith('???'):

                idx = len(curcode)

                # without is showing error on same line as before...?
                line = "\n" + line
                token = (0, Generic.Traceback, line)
                insertions.append((idx, [token]))

            else:
                if curcode:
                    for item in do_insertions(
                        insertions, mlexer.get_tokens_unprocessed(curcode)):
                        yield item
                    curcode = ''
                    insertions = []

                yield match.start(), Generic.Output, line

        if curcode: # or item:
            for item in do_insertions(
                insertions, mlexer.get_tokens_unprocessed(curcode)):
                yield item


class OctaveLexer(RegexLexer):
    """
    For GNU Octave source code.

    *New in Pygments 1.5.*
    """
    name = 'Octave'
    aliases = ['octave']
    filenames = ['*.m']
    mimetypes = ['text/octave']

    # These lists are generated automatically.
    # Run the following in bash shell:
    #
    # First dump all of the Octave manual into a plain text file:
    #
    #   $ info octave --subnodes -o octave-manual
    #
    # Now grep through it:

    # for i in \
    #     "Built-in Function" "Command" "Function File" \
    #     "Loadable Function" "Mapping Function";
    # do
    #     perl -e '@name = qw('"$i"');
    #              print lc($name[0]),"_kw = [\n"';
    #
    #     perl -n -e 'print "\"$1\",\n" if /-- '"$i"': .* (\w*) \(/;' \
    #         octave-manual | sort | uniq ;
    #     echo "]" ;
    #     echo;
    # done

    # taken from Octave Mercurial changeset 8cc154f45e37 (30-jan-2011)

    builtin_kw = [ "addlistener", "addpath", "addproperty", "all",
                   "and", "any", "argnames", "argv", "assignin",
                   "atexit", "autoload",
                   "available_graphics_toolkits", "beep_on_error",
                   "bitand", "bitmax", "bitor", "bitshift", "bitxor",
                   "cat", "cell", "cellstr", "char", "class", "clc",
                   "columns", "command_line_path",
                   "completion_append_char", "completion_matches",
                   "complex", "confirm_recursive_rmdir", "cputime",
                   "crash_dumps_octave_core", "ctranspose", "cumprod",
                   "cumsum", "debug_on_error", "debug_on_interrupt",
                   "debug_on_warning", "default_save_options",
                   "dellistener", "diag", "diff", "disp",
                   "doc_cache_file", "do_string_escapes", "double",
                   "drawnow", "e", "echo_executing_commands", "eps",
                   "eq", "errno", "errno_list", "error", "eval",
                   "evalin", "exec", "exist", "exit", "eye", "false",
                   "fclear", "fclose", "fcntl", "fdisp", "feof",
                   "ferror", "feval", "fflush", "fgetl", "fgets",
                   "fieldnames", "file_in_loadpath", "file_in_path",
                   "filemarker", "filesep", "find_dir_in_path",
                   "fixed_point_format", "fnmatch", "fopen", "fork",
                   "formula", "fprintf", "fputs", "fread", "freport",
                   "frewind", "fscanf", "fseek", "fskipl", "ftell",
                   "functions", "fwrite", "ge", "genpath", "get",
                   "getegid", "getenv", "geteuid", "getgid",
                   "getpgrp", "getpid", "getppid", "getuid", "glob",
                   "gt", "gui_mode", "history_control",
                   "history_file", "history_size",
                   "history_timestamp_format_string", "home",
                   "horzcat", "hypot", "ifelse",
                   "ignore_function_time_stamp", "inferiorto",
                   "info_file", "info_program", "inline", "input",
                   "intmax", "intmin", "ipermute",
                   "is_absolute_filename", "isargout", "isbool",
                   "iscell", "iscellstr", "ischar", "iscomplex",
                   "isempty", "isfield", "isfloat", "isglobal",
                   "ishandle", "isieee", "isindex", "isinteger",
                   "islogical", "ismatrix", "ismethod", "isnull",
                   "isnumeric", "isobject", "isreal",
                   "is_rooted_relative_filename", "issorted",
                   "isstruct", "isvarname", "kbhit", "keyboard",
                   "kill", "lasterr", "lasterror", "lastwarn",
                   "ldivide", "le", "length", "link", "linspace",
                   "logical", "lstat", "lt", "make_absolute_filename",
                   "makeinfo_program", "max_recursion_depth", "merge",
                   "methods", "mfilename", "minus", "mislocked",
                   "mkdir", "mkfifo", "mkstemp", "mldivide", "mlock",
                   "mouse_wheel_zoom", "mpower", "mrdivide", "mtimes",
                   "munlock", "nargin", "nargout",
                   "native_float_format", "ndims", "ne", "nfields",
                   "nnz", "norm", "not", "numel", "nzmax",
                   "octave_config_info", "octave_core_file_limit",
                   "octave_core_file_name",
                   "octave_core_file_options", "ones", "or",
                   "output_max_field_width", "output_precision",
                   "page_output_immediately", "page_screen_output",
                   "path", "pathsep", "pause", "pclose", "permute",
                   "pi", "pipe", "plus", "popen", "power",
                   "print_empty_dimensions", "printf",
                   "print_struct_array_contents", "prod",
                   "program_invocation_name", "program_name",
                   "putenv", "puts", "pwd", "quit", "rats", "rdivide",
                   "readdir", "readlink", "read_readline_init_file",
                   "realmax", "realmin", "rehash", "rename",
                   "repelems", "re_read_readline_init_file", "reset",
                   "reshape", "resize", "restoredefaultpath",
                   "rethrow", "rmdir", "rmfield", "rmpath", "rows",
                   "save_header_format_string", "save_precision",
                   "saving_history", "scanf", "set", "setenv",
                   "shell_cmd", "sighup_dumps_octave_core",
                   "sigterm_dumps_octave_core", "silent_functions",
                   "single", "size", "size_equal", "sizemax",
                   "sizeof", "sleep", "source", "sparse_auto_mutate",
                   "split_long_rows", "sprintf", "squeeze", "sscanf",
                   "stat", "stderr", "stdin", "stdout", "strcmp",
                   "strcmpi", "string_fill_char", "strncmp",
                   "strncmpi", "struct", "struct_levels_to_print",
                   "strvcat", "subsasgn", "subsref", "sum", "sumsq",
                   "superiorto", "suppress_verbose_help_message",
                   "symlink", "system", "tic", "tilde_expand",
                   "times", "tmpfile", "tmpnam", "toc", "toupper",
                   "transpose", "true", "typeinfo", "umask", "uminus",
                   "uname", "undo_string_escapes", "unlink", "uplus",
                   "upper", "usage", "usleep", "vec", "vectorize",
                   "vertcat", "waitpid", "warning", "warranty",
                   "whos_line_format", "yes_or_no", "zeros",
                   "inf", "Inf", "nan", "NaN"]

    command_kw = [ "close", "load", "who", "whos", ]

    function_kw = [ "accumarray", "accumdim", "acosd", "acotd",
                   "acscd", "addtodate", "allchild", "ancestor",
                   "anova", "arch_fit", "arch_rnd", "arch_test",
                   "area", "arma_rnd", "arrayfun", "ascii", "asctime",
                   "asecd", "asind", "assert", "atand",
                   "autoreg_matrix", "autumn", "axes", "axis", "bar",
                   "barh", "bartlett", "bartlett_test", "beep",
                   "betacdf", "betainv", "betapdf", "betarnd",
                   "bicgstab", "bicubic", "binary", "binocdf",
                   "binoinv", "binopdf", "binornd", "bitcmp",
                   "bitget", "bitset", "blackman", "blanks",
                   "blkdiag", "bone", "box", "brighten", "calendar",
                   "cast", "cauchy_cdf", "cauchy_inv", "cauchy_pdf",
                   "cauchy_rnd", "caxis", "celldisp", "center", "cgs",
                   "chisquare_test_homogeneity",
                   "chisquare_test_independence", "circshift", "cla",
                   "clabel", "clf", "clock", "cloglog", "closereq",
                   "colon", "colorbar", "colormap", "colperm",
                   "comet", "common_size", "commutation_matrix",
                   "compan", "compare_versions", "compass",
                   "computer", "cond", "condest", "contour",
                   "contourc", "contourf", "contrast", "conv",
                   "convhull", "cool", "copper", "copyfile", "cor",
                   "corrcoef", "cor_test", "cosd", "cotd", "cov",
                   "cplxpair", "cross", "cscd", "cstrcat", "csvread",
                   "csvwrite", "ctime", "cumtrapz", "curl", "cut",
                   "cylinder", "date", "datenum", "datestr",
                   "datetick", "datevec", "dblquad", "deal",
                   "deblank", "deconv", "delaunay", "delaunayn",
                   "delete", "demo", "detrend", "diffpara", "diffuse",
                   "dir", "discrete_cdf", "discrete_inv",
                   "discrete_pdf", "discrete_rnd", "display",
                   "divergence", "dlmwrite", "dos", "dsearch",
                   "dsearchn", "duplication_matrix", "durbinlevinson",
                   "ellipsoid", "empirical_cdf", "empirical_inv",
                   "empirical_pdf", "empirical_rnd", "eomday",
                   "errorbar", "etime", "etreeplot", "example",
                   "expcdf", "expinv", "expm", "exppdf", "exprnd",
                   "ezcontour", "ezcontourf", "ezmesh", "ezmeshc",
                   "ezplot", "ezpolar", "ezsurf", "ezsurfc", "factor",
                   "factorial", "fail", "fcdf", "feather", "fftconv",
                   "fftfilt", "fftshift", "figure", "fileattrib",
                   "fileparts", "fill", "findall", "findobj",
                   "findstr", "finv", "flag", "flipdim", "fliplr",
                   "flipud", "fpdf", "fplot", "fractdiff", "freqz",
                   "freqz_plot", "frnd", "fsolve",
                   "f_test_regression", "ftp", "fullfile", "fzero",
                   "gamcdf", "gaminv", "gampdf", "gamrnd", "gca",
                   "gcbf", "gcbo", "gcf", "genvarname", "geocdf",
                   "geoinv", "geopdf", "geornd", "getfield", "ginput",
                   "glpk", "gls", "gplot", "gradient",
                   "graphics_toolkit", "gray", "grid", "griddata",
                   "griddatan", "gtext", "gunzip", "gzip", "hadamard",
                   "hamming", "hankel", "hanning", "hggroup",
                   "hidden", "hilb", "hist", "histc", "hold", "hot",
                   "hotelling_test", "housh", "hsv", "hurst",
                   "hygecdf", "hygeinv", "hygepdf", "hygernd",
                   "idivide", "ifftshift", "image", "imagesc",
                   "imfinfo", "imread", "imshow", "imwrite", "index",
                   "info", "inpolygon", "inputname", "interpft",
                   "interpn", "intersect", "invhilb", "iqr", "isa",
                   "isdefinite", "isdir", "is_duplicate_entry",
                   "isequal", "isequalwithequalnans", "isfigure",
                   "ishermitian", "ishghandle", "is_leap_year",
                   "isletter", "ismac", "ismember", "ispc", "isprime",
                   "isprop", "isscalar", "issquare", "isstrprop",
                   "issymmetric", "isunix", "is_valid_file_id",
                   "isvector", "jet", "kendall",
                   "kolmogorov_smirnov_cdf",
                   "kolmogorov_smirnov_test", "kruskal_wallis_test",
                   "krylov", "kurtosis", "laplace_cdf", "laplace_inv",
                   "laplace_pdf", "laplace_rnd", "legend", "legendre",
                   "license", "line", "linkprop", "list_primes",
                   "loadaudio", "loadobj", "logistic_cdf",
                   "logistic_inv", "logistic_pdf", "logistic_rnd",
                   "logit", "loglog", "loglogerr", "logm", "logncdf",
                   "logninv", "lognpdf", "lognrnd", "logspace",
                   "lookfor", "ls_command", "lsqnonneg", "magic",
                   "mahalanobis", "manova", "matlabroot",
                   "mcnemar_test", "mean", "meansq", "median", "menu",
                   "mesh", "meshc", "meshgrid", "meshz", "mexext",
                   "mget", "mkpp", "mode", "moment", "movefile",
                   "mpoles", "mput", "namelengthmax", "nargchk",
                   "nargoutchk", "nbincdf", "nbininv", "nbinpdf",
                   "nbinrnd", "nchoosek", "ndgrid", "newplot", "news",
                   "nonzeros", "normcdf", "normest", "norminv",
                   "normpdf", "normrnd", "now", "nthroot", "null",
                   "ocean", "ols", "onenormest", "optimget",
                   "optimset", "orderfields", "orient", "orth",
                   "pack", "pareto", "parseparams", "pascal", "patch",
                   "pathdef", "pcg", "pchip", "pcolor", "pcr",
                   "peaks", "periodogram", "perl", "perms", "pie",
                   "pink", "planerot", "playaudio", "plot",
                   "plotmatrix", "plotyy", "poisscdf", "poissinv",
                   "poisspdf", "poissrnd", "polar", "poly",
                   "polyaffine", "polyarea", "polyderiv", "polyfit",
                   "polygcd", "polyint", "polyout", "polyreduce",
                   "polyval", "polyvalm", "postpad", "powerset",
                   "ppder", "ppint", "ppjumps", "ppplot", "ppval",
                   "pqpnonneg", "prepad", "primes", "print",
                   "print_usage", "prism", "probit", "qp", "qqplot",
                   "quadcc", "quadgk", "quadl", "quadv", "quiver",
                   "qzhess", "rainbow", "randi", "range", "rank",
                   "ranks", "rat", "reallog", "realpow", "realsqrt",
                   "record", "rectangle_lw", "rectangle_sw",
                   "rectint", "refresh", "refreshdata",
                   "regexptranslate", "repmat", "residue", "ribbon",
                   "rindex", "roots", "rose", "rosser", "rotdim",
                   "rref", "run", "run_count", "rundemos", "run_test",
                   "runtests", "saveas", "saveaudio", "saveobj",
                   "savepath", "scatter", "secd", "semilogx",
                   "semilogxerr", "semilogy", "semilogyerr",
                   "setaudio", "setdiff", "setfield", "setxor",
                   "shading", "shift", "shiftdim", "sign_test",
                   "sinc", "sind", "sinetone", "sinewave", "skewness",
                   "slice", "sombrero", "sortrows", "spaugment",
                   "spconvert", "spdiags", "spearman", "spectral_adf",
                   "spectral_xdf", "specular", "speed", "spencer",
                   "speye", "spfun", "sphere", "spinmap", "spline",
                   "spones", "sprand", "sprandn", "sprandsym",
                   "spring", "spstats", "spy", "sqp", "stairs",
                   "statistics", "std", "stdnormal_cdf",
                   "stdnormal_inv", "stdnormal_pdf", "stdnormal_rnd",
                   "stem", "stft", "strcat", "strchr", "strjust",
                   "strmatch", "strread", "strsplit", "strtok",
                   "strtrim", "strtrunc", "structfun", "studentize",
                   "subplot", "subsindex", "subspace", "substr",
                   "substruct", "summer", "surf", "surface", "surfc",
                   "surfl", "surfnorm", "svds", "swapbytes",
                   "sylvester_matrix", "symvar", "synthesis", "table",
                   "tand", "tar", "tcdf", "tempdir", "tempname",
                   "test", "text", "textread", "textscan", "tinv",
                   "title", "toeplitz", "tpdf", "trace", "trapz",
                   "treelayout", "treeplot", "triangle_lw",
                   "triangle_sw", "tril", "trimesh", "triplequad",
                   "triplot", "trisurf", "triu", "trnd", "tsearchn",
                   "t_test", "t_test_regression", "type", "unidcdf",
                   "unidinv", "unidpdf", "unidrnd", "unifcdf",
                   "unifinv", "unifpdf", "unifrnd", "union", "unique",
                   "unix", "unmkpp", "unpack", "untabify", "untar",
                   "unwrap", "unzip", "u_test", "validatestring",
                   "vander", "var", "var_test", "vech", "ver",
                   "version", "view", "voronoi", "voronoin",
                   "waitforbuttonpress", "wavread", "wavwrite",
                   "wblcdf", "wblinv", "wblpdf", "wblrnd", "weekday",
                   "welch_test", "what", "white", "whitebg",
                   "wienrnd", "wilcoxon_test", "wilkinson", "winter",
                   "xlabel", "xlim", "ylabel", "yulewalker", "zip",
                   "zlabel", "z_test", ]

    loadable_kw = [ "airy", "amd", "balance", "besselh", "besseli",
                   "besselj", "besselk", "bessely", "bitpack",
                   "bsxfun", "builtin", "ccolamd", "cellfun",
                   "cellslices", "chol", "choldelete", "cholinsert",
                   "cholinv", "cholshift", "cholupdate", "colamd",
                   "colloc", "convhulln", "convn", "csymamd",
                   "cummax", "cummin", "daspk", "daspk_options",
                   "dasrt", "dasrt_options", "dassl", "dassl_options",
                   "dbclear", "dbdown", "dbstack", "dbstatus",
                   "dbstop", "dbtype", "dbup", "dbwhere", "det",
                   "dlmread", "dmperm", "dot", "eig", "eigs",
                   "endgrent", "endpwent", "etree", "fft", "fftn",
                   "fftw", "filter", "find", "full", "gcd",
                   "getgrent", "getgrgid", "getgrnam", "getpwent",
                   "getpwnam", "getpwuid", "getrusage", "givens",
                   "gmtime", "gnuplot_binary", "hess", "ifft",
                   "ifftn", "inv", "isdebugmode", "issparse", "kron",
                   "localtime", "lookup", "lsode", "lsode_options",
                   "lu", "luinc", "luupdate", "matrix_type", "max",
                   "min", "mktime", "pinv", "qr", "qrdelete",
                   "qrinsert", "qrshift", "qrupdate", "quad",
                   "quad_options", "qz", "rand", "rande", "randg",
                   "randn", "randp", "randperm", "rcond", "regexp",
                   "regexpi", "regexprep", "schur", "setgrent",
                   "setpwent", "sort", "spalloc", "sparse", "spparms",
                   "sprank", "sqrtm", "strfind", "strftime",
                   "strptime", "strrep", "svd", "svd_driver", "syl",
                   "symamd", "symbfact", "symrcm", "time", "tsearch",
                   "typecast", "urlread", "urlwrite", ]

    mapping_kw = [ "abs", "acos", "acosh", "acot", "acoth", "acsc",
                  "acsch", "angle", "arg", "asec", "asech", "asin",
                  "asinh", "atan", "atanh", "beta", "betainc",
                  "betaln", "bincoeff", "cbrt", "ceil", "conj", "cos",
                  "cosh", "cot", "coth", "csc", "csch", "erf", "erfc",
                  "erfcx", "erfinv", "exp", "finite", "fix", "floor",
                  "fmod", "gamma", "gammainc", "gammaln", "imag",
                  "isalnum", "isalpha", "isascii", "iscntrl",
                  "isdigit", "isfinite", "isgraph", "isinf",
                  "islower", "isna", "isnan", "isprint", "ispunct",
                  "isspace", "isupper", "isxdigit", "lcm", "lgamma",
                  "log", "lower", "mod", "real", "rem", "round",
                  "roundb", "sec", "sech", "sign", "sin", "sinh",
                  "sqrt", "tan", "tanh", "toascii", "tolower", "xor",
                  ]

    builtin_consts = [ "EDITOR", "EXEC_PATH", "I", "IMAGE_PATH", "NA",
                   "OCTAVE_HOME", "OCTAVE_VERSION", "PAGER",
                   "PAGER_FLAGS", "SEEK_CUR", "SEEK_END", "SEEK_SET",
                   "SIG", "S_ISBLK", "S_ISCHR", "S_ISDIR", "S_ISFIFO",
                   "S_ISLNK", "S_ISREG", "S_ISSOCK", "WCONTINUE",
                   "WCOREDUMP", "WEXITSTATUS", "WIFCONTINUED",
                   "WIFEXITED", "WIFSIGNALED", "WIFSTOPPED", "WNOHANG",
                   "WSTOPSIG", "WTERMSIG", "WUNTRACED", ]

    tokens = {
        'root': [
            #We should look into multiline comments
            (r'[%#].*$', Comment),
            (r'^\s*function', Keyword, 'deffunc'),

            # from 'iskeyword' on hg changeset 8cc154f45e37
            (r'(__FILE__|__LINE__|break|case|catch|classdef|continue|do|else|'
             r'elseif|end|end_try_catch|end_unwind_protect|endclassdef|'
             r'endevents|endfor|endfunction|endif|endmethods|endproperties|'
             r'endswitch|endwhile|events|for|function|get|global|if|methods|'
             r'otherwise|persistent|properties|return|set|static|switch|try|'
             r'until|unwind_protect|unwind_protect_cleanup|while)\b', Keyword),

            ("(" + "|".join(  builtin_kw + command_kw
                            + function_kw + loadable_kw
                            + mapping_kw) + r')\b',  Name.Builtin),

            ("(" + "|".join(builtin_consts) + r')\b', Name.Constant),

            # operators in Octave but not Matlab:
            (r'-=|!=|!|/=|--', Operator),
            # operators:
            (r'-|==|~=|<|>|<=|>=|&&|&|~|\|\|?', Operator),
            # operators in Octave but not Matlab requiring escape for re:
            (r'\*=|\+=|\^=|\/=|\\=|\*\*|\+\+|\.\*\*',Operator),
            # operators requiring escape for re:
            (r'\.\*|\*|\+|\.\^|\.\\|\.\/|\/|\\', Operator),


            # punctuation:
            (r'\[|\]|\(|\)|\{|\}|:|@|\.|,', Punctuation),
            (r'=|:|;', Punctuation),

            (r'"[^"]*"', String),

            (r'(\d+\.\d*|\d*\.\d+)([eEf][+-]?[0-9]+)?', Number.Float),
            (r'\d+[eEf][+-]?[0-9]+', Number.Float),
            (r'\d+', Number.Integer),

            # quote can be transpose, instead of string:
            # (not great, but handles common cases...)
            (r'(?<=[\w\)\]])\'', Operator),
            (r'(?<![\w\)\]])\'', String, 'string'),

            ('[a-zA-Z_][a-zA-Z0-9_]*', Name),
            (r'.', Text),
        ],
        'string': [
            (r"[^']*'", String, '#pop'),
        ],
        'deffunc': [
            (r'(\s*)(?:(.+)(\s*)(=)(\s*))?(.+)(\()(.*)(\))(\s*)',
             bygroups(Text.Whitespace, Text, Text.Whitespace, Punctuation,
                      Text.Whitespace, Name.Function, Punctuation, Text,
                      Punctuation, Text.Whitespace), '#pop'),
        ],
    }

    def analyse_text(text):
        if re.match('^\s*[%#]', text, re.M): #Comment
            return 0.1


class ScilabLexer(RegexLexer):
    """
    For Scilab source code.

    *New in Pygments 1.5.*
    """
    name = 'Scilab'
    aliases = ['scilab']
    filenames = ['*.sci', '*.sce', '*.tst']
    mimetypes = ['text/scilab']

    tokens = {
        'root': [
            (r'//.*?$', Comment.Single),
            (r'^\s*function', Keyword, 'deffunc'),

            (r'(__FILE__|__LINE__|break|case|catch|classdef|continue|do|else|'
             r'elseif|end|end_try_catch|end_unwind_protect|endclassdef|'
             r'endevents|endfor|endfunction|endif|endmethods|endproperties|'
             r'endswitch|endwhile|events|for|function|get|global|if|methods|'
             r'otherwise|persistent|properties|return|set|static|switch|try|'
             r'until|unwind_protect|unwind_protect_cleanup|while)\b', Keyword),

            ("(" + "|".join(_scilab_builtins.functions_kw +
                            _scilab_builtins.commands_kw +
                            _scilab_builtins.macros_kw
                            ) + r')\b',  Name.Builtin),

            (r'(%s)\b' % "|".join(map(re.escape, _scilab_builtins.builtin_consts)),
             Name.Constant),

            # operators:
            (r'-|==|~=|<|>|<=|>=|&&|&|~|\|\|?', Operator),
            # operators requiring escape for re:
            (r'\.\*|\*|\+|\.\^|\.\\|\.\/|\/|\\', Operator),

            # punctuation:
            (r'[\[\](){}@.,=:;]', Punctuation),

            (r'"[^"]*"', String),

            # quote can be transpose, instead of string:
            # (not great, but handles common cases...)
            (r'(?<=[\w\)\]])\'', Operator),
            (r'(?<![\w\)\]])\'', String, 'string'),

            (r'(\d+\.\d*|\d*\.\d+)([eEf][+-]?[0-9]+)?', Number.Float),
            (r'\d+[eEf][+-]?[0-9]+', Number.Float),
            (r'\d+', Number.Integer),

            ('[a-zA-Z_][a-zA-Z0-9_]*', Name),
            (r'.', Text),
        ],
        'string': [
            (r"[^']*'", String, '#pop'),
            (r'.', String, '#pop'),
        ],
        'deffunc': [
            (r'(\s*)(?:(.+)(\s*)(=)(\s*))?(.+)(\()(.*)(\))(\s*)',
             bygroups(Text.Whitespace, Text, Text.Whitespace, Punctuation,
                      Text.Whitespace, Name.Function, Punctuation, Text,
                      Punctuation, Text.Whitespace), '#pop'),
        ],
    }


class NumPyLexer(PythonLexer):
    """
    A Python lexer recognizing Numerical Python builtins.

    *New in Pygments 0.10.*
    """

    name = 'NumPy'
    aliases = ['numpy']

    # override the mimetypes to not inherit them from python
    mimetypes = []
    filenames = []

    EXTRA_KEYWORDS = set([
        'abs', 'absolute', 'accumulate', 'add', 'alen', 'all', 'allclose',
        'alltrue', 'alterdot', 'amax', 'amin', 'angle', 'any', 'append',
        'apply_along_axis', 'apply_over_axes', 'arange', 'arccos', 'arccosh',
        'arcsin', 'arcsinh', 'arctan', 'arctan2', 'arctanh', 'argmax', 'argmin',
        'argsort', 'argwhere', 'around', 'array', 'array2string', 'array_equal',
        'array_equiv', 'array_repr', 'array_split', 'array_str', 'arrayrange',
        'asanyarray', 'asarray', 'asarray_chkfinite', 'ascontiguousarray',
        'asfarray', 'asfortranarray', 'asmatrix', 'asscalar', 'astype',
        'atleast_1d', 'atleast_2d', 'atleast_3d', 'average', 'bartlett',
        'base_repr', 'beta', 'binary_repr', 'bincount', 'binomial',
        'bitwise_and', 'bitwise_not', 'bitwise_or', 'bitwise_xor', 'blackman',
        'bmat', 'broadcast', 'byte_bounds', 'bytes', 'byteswap', 'c_',
        'can_cast', 'ceil', 'choose', 'clip', 'column_stack', 'common_type',
        'compare_chararrays', 'compress', 'concatenate', 'conj', 'conjugate',
        'convolve', 'copy', 'corrcoef', 'correlate', 'cos', 'cosh', 'cov',
        'cross', 'cumprod', 'cumproduct', 'cumsum', 'delete', 'deprecate',
        'diag', 'diagflat', 'diagonal', 'diff', 'digitize', 'disp', 'divide',
        'dot', 'dsplit', 'dstack', 'dtype', 'dump', 'dumps', 'ediff1d', 'empty',
        'empty_like', 'equal', 'exp', 'expand_dims', 'expm1', 'extract', 'eye',
        'fabs', 'fastCopyAndTranspose', 'fft', 'fftfreq', 'fftshift', 'fill',
        'finfo', 'fix', 'flat', 'flatnonzero', 'flatten', 'fliplr', 'flipud',
        'floor', 'floor_divide', 'fmod', 'frexp', 'fromarrays', 'frombuffer',
        'fromfile', 'fromfunction', 'fromiter', 'frompyfunc', 'fromstring',
        'generic', 'get_array_wrap', 'get_include', 'get_numarray_include',
        'get_numpy_include', 'get_printoptions', 'getbuffer', 'getbufsize',
        'geterr', 'geterrcall', 'geterrobj', 'getfield', 'gradient', 'greater',
        'greater_equal', 'gumbel', 'hamming', 'hanning', 'histogram',
        'histogram2d', 'histogramdd', 'hsplit', 'hstack', 'hypot', 'i0',
        'identity', 'ifft', 'imag', 'index_exp', 'indices', 'inf', 'info',
        'inner', 'insert', 'int_asbuffer', 'interp', 'intersect1d',
        'intersect1d_nu', 'inv', 'invert', 'iscomplex', 'iscomplexobj',
        'isfinite', 'isfortran', 'isinf', 'isnan', 'isneginf', 'isposinf',
        'isreal', 'isrealobj', 'isscalar', 'issctype', 'issubclass_',
        'issubdtype', 'issubsctype', 'item', 'itemset', 'iterable', 'ix_',
        'kaiser', 'kron', 'ldexp', 'left_shift', 'less', 'less_equal', 'lexsort',
        'linspace', 'load', 'loads', 'loadtxt', 'log', 'log10', 'log1p', 'log2',
        'logical_and', 'logical_not', 'logical_or', 'logical_xor', 'logspace',
        'lstsq', 'mat', 'matrix', 'max', 'maximum', 'maximum_sctype',
        'may_share_memory', 'mean', 'median', 'meshgrid', 'mgrid', 'min',
        'minimum', 'mintypecode', 'mod', 'modf', 'msort', 'multiply', 'nan',
        'nan_to_num', 'nanargmax', 'nanargmin', 'nanmax', 'nanmin', 'nansum',
        'ndenumerate', 'ndim', 'ndindex', 'negative', 'newaxis', 'newbuffer',
        'newbyteorder', 'nonzero', 'not_equal', 'obj2sctype', 'ogrid', 'ones',
        'ones_like', 'outer', 'permutation', 'piecewise', 'pinv', 'pkgload',
        'place', 'poisson', 'poly', 'poly1d', 'polyadd', 'polyder', 'polydiv',
        'polyfit', 'polyint', 'polymul', 'polysub', 'polyval', 'power', 'prod',
        'product', 'ptp', 'put', 'putmask', 'r_', 'randint', 'random_integers',
        'random_sample', 'ranf', 'rank', 'ravel', 'real', 'real_if_close',
        'recarray', 'reciprocal', 'reduce', 'remainder', 'repeat', 'require',
        'reshape', 'resize', 'restoredot', 'right_shift', 'rint', 'roll',
        'rollaxis', 'roots', 'rot90', 'round', 'round_', 'row_stack', 's_',
        'sample', 'savetxt', 'sctype2char', 'searchsorted', 'seed', 'select',
        'set_numeric_ops', 'set_printoptions', 'set_string_function',
        'setbufsize', 'setdiff1d', 'seterr', 'seterrcall', 'seterrobj',
        'setfield', 'setflags', 'setmember1d', 'setxor1d', 'shape',
        'show_config', 'shuffle', 'sign', 'signbit', 'sin', 'sinc', 'sinh',
        'size', 'slice', 'solve', 'sometrue', 'sort', 'sort_complex', 'source',
        'split', 'sqrt', 'square', 'squeeze', 'standard_normal', 'std',
        'subtract', 'sum', 'svd', 'swapaxes', 'take', 'tan', 'tanh', 'tensordot',
        'test', 'tile', 'tofile', 'tolist', 'tostring', 'trace', 'transpose',
        'trapz', 'tri', 'tril', 'trim_zeros', 'triu', 'true_divide', 'typeDict',
        'typename', 'uniform', 'union1d', 'unique', 'unique1d', 'unravel_index',
        'unwrap', 'vander', 'var', 'vdot', 'vectorize', 'view', 'vonmises',
        'vsplit', 'vstack', 'weibull', 'where', 'who', 'zeros', 'zeros_like'
    ])

    def get_tokens_unprocessed(self, text):
        for index, token, value in \
                PythonLexer.get_tokens_unprocessed(self, text):
            if token is Name and value in self.EXTRA_KEYWORDS:
                yield index, Keyword.Pseudo, value
            else:
                yield index, token, value


class RConsoleLexer(Lexer):
    """
    For R console transcripts or R CMD BATCH output files.
    """

    name = 'RConsole'
    aliases = ['rconsole', 'rout']
    filenames = ['*.Rout']

    def get_tokens_unprocessed(self, text):
        slexer = SLexer(**self.options)

        current_code_block = ''
        insertions = []

        for match in line_re.finditer(text):
            line = match.group()
            if line.startswith('>') or line.startswith('+'):
                # Colorize the prompt as such,
                # then put rest of line into current_code_block
                insertions.append((len(current_code_block),
                                   [(0, Generic.Prompt, line[:2])]))
                current_code_block += line[2:]
            else:
                # We have reached a non-prompt line!
                # If we have stored prompt lines, need to process them first.
                if current_code_block:
                    # Weave together the prompts and highlight code.
                    for item in do_insertions(insertions,
                          slexer.get_tokens_unprocessed(current_code_block)):
                        yield item
                    # Reset vars for next code block.
                    current_code_block = ''
                    insertions = []
                # Now process the actual line itself, this is output from R.
                yield match.start(), Generic.Output, line

        # If we happen to end on a code block with nothing after it, need to
        # process the last code block. This is neither elegant nor DRY so
        # should be changed.
        if current_code_block:
            for item in do_insertions(insertions,
                    slexer.get_tokens_unprocessed(current_code_block)):
                yield item


class SLexer(RegexLexer):
    """
    For S, S-plus, and R source code.

    *New in Pygments 0.10.*
    """

    name = 'S'
    aliases = ['splus', 's', 'r']
    filenames = ['*.S', '*.R', '.Rhistory', '.Rprofile']
    mimetypes = ['text/S-plus', 'text/S', 'text/x-r-source', 'text/x-r',
                 'text/x-R', 'text/x-r-history', 'text/x-r-profile']

    tokens = {
        'comments': [
            (r'#.*$', Comment.Single),
        ],
        'valid_name': [
            (r'[a-zA-Z][0-9a-zA-Z\._]*', Text),
            # can begin with ., but not if that is followed by a digit
            (r'\.[a-zA-Z_][0-9a-zA-Z\._]*', Text),
        ],
        'punctuation': [
            (r'\[{1,2}|\]{1,2}|\(|\)|;|,', Punctuation),
        ],
        'keywords': [
            (r'(if|else|for|while|repeat|in|next|break|return|switch|function)'
             r'(?![0-9a-zA-Z\._])',
             Keyword.Reserved)
        ],
        'operators': [
            (r'<<?-|->>?|-|==|<=|>=|<|>|&&?|!=|\|\|?|\?', Operator),
            (r'\*|\+|\^|/|!|%[^%]*%|=|~|\$|@|:{1,3}', Operator)
        ],
        'builtin_symbols': [
            (r'(NULL|NA(_(integer|real|complex|character)_)?|'
             r'Inf|TRUE|FALSE|NaN|\.\.(\.|[0-9]+))'
             r'(?![0-9a-zA-Z\._])',
             Keyword.Constant),
            (r'(T|F)\b', Keyword.Variable),
        ],
        'numbers': [
            # hex number
            (r'0[xX][a-fA-F0-9]+([pP][0-9]+)?[Li]?', Number.Hex),
            # decimal number
            (r'[+-]?([0-9]+(\.[0-9]+)?|\.[0-9]+)([eE][+-]?[0-9]+)?[Li]?',
             Number),
        ],
        'statements': [
            include('comments'),
            # whitespaces
            (r'\s+', Text),
            (r'`.*?`', String.Backtick),
            (r'\'', String, 'string_squote'),
            (r'\"', String, 'string_dquote'),
            include('builtin_symbols'),
            include('numbers'),
            include('keywords'),
            include('punctuation'),
            include('operators'),
            include('valid_name'),
        ],
        'root': [
            include('statements'),
            # blocks:
            (r'\{|\}', Punctuation),
            #(r'\{', Punctuation, 'block'),
            (r'.', Text),
        ],
        #'block': [
        #    include('statements'),
        #    ('\{', Punctuation, '#push'),
        #    ('\}', Punctuation, '#pop')
        #],
        'string_squote': [
            (r'([^\'\\]|\\.)*\'', String, '#pop'),
        ],
        'string_dquote': [
            (r'([^"\\]|\\.)*"', String, '#pop'),
        ],
    }

    def analyse_text(text):
        return '<-' in text


class BugsLexer(RegexLexer):
    """
    Pygments Lexer for `OpenBugs <http://www.openbugs.info/w/>`_ and WinBugs
    models.

    *New in Pygments 1.6.*
    """

    name = 'BUGS'
    aliases = ['bugs', 'winbugs', 'openbugs']
    filenames = ['*.bug']

    _FUNCTIONS = [
        # Scalar functions
        'abs', 'arccos', 'arccosh', 'arcsin', 'arcsinh', 'arctan', 'arctanh',
        'cloglog', 'cos', 'cosh', 'cumulative', 'cut', 'density', 'deviance',
        'equals', 'expr', 'gammap', 'ilogit', 'icloglog', 'integral', 'log',
        'logfact', 'loggam', 'logit', 'max', 'min', 'phi', 'post.p.value',
        'pow', 'prior.p.value', 'probit', 'replicate.post', 'replicate.prior',
        'round', 'sin', 'sinh', 'solution', 'sqrt', 'step', 'tan', 'tanh',
        'trunc',
        # Vector functions
        'inprod', 'interp.lin', 'inverse', 'logdet', 'mean', 'eigen.vals',
        'ode', 'prod', 'p.valueM', 'rank', 'ranked', 'replicate.postM',
        'sd', 'sort', 'sum',
        ## Special
        'D', 'I', 'F', 'T', 'C']
    """ OpenBUGS built-in functions

    From http://www.openbugs.info/Manuals/ModelSpecification.html#ContentsAII

    This also includes

    - T, C, I : Truncation and censoring.
      ``T`` and ``C`` are in OpenBUGS. ``I`` in WinBUGS.
    - D : ODE
    - F : Functional http://www.openbugs.info/Examples/Functionals.html

    """

    _DISTRIBUTIONS = ['dbern', 'dbin', 'dcat', 'dnegbin', 'dpois',
                      'dhyper', 'dbeta', 'dchisqr', 'ddexp', 'dexp',
                      'dflat', 'dgamma', 'dgev', 'df', 'dggamma', 'dgpar',
                      'dloglik', 'dlnorm', 'dlogis', 'dnorm', 'dpar',
                      'dt', 'dunif', 'dweib', 'dmulti', 'ddirch', 'dmnorm',
                      'dmt', 'dwish']
    """ OpenBUGS built-in distributions

    Functions from
    http://www.openbugs.info/Manuals/ModelSpecification.html#ContentsAI
    """


    tokens = {
        'whitespace' : [
            (r"\s+", Text),
            ],
        'comments' : [
            # Comments
            (r'#.*$', Comment.Single),
            ],
        'root': [
            # Comments
            include('comments'),
            include('whitespace'),
            # Block start
            (r'(model)(\s+)({)',
             bygroups(Keyword.Namespace, Text, Punctuation)),
            # Reserved Words
            (r'(for|in)(?![0-9a-zA-Z\._])', Keyword.Reserved),
            # Built-in Functions
            (r'(%s)(?=\s*\()'
             % r'|'.join(_FUNCTIONS + _DISTRIBUTIONS),
             Name.Builtin),
            # Regular variable names
            (r'[A-Za-z][A-Za-z0-9_.]*', Name),
            # Number Literals
            (r'[-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?', Number),
            # Punctuation
            (r'\[|\]|\(|\)|:|,|;', Punctuation),
            # Assignment operators
            # SLexer makes these tokens Operators.
            (r'<-|~', Operator),
            # Infix and prefix operators
            (r'\+|-|\*|/', Operator),
            # Block
            (r'[{}]', Punctuation),
            ]
        }

    def analyse_text(text):
        if re.search(r"^\s*model\s*{", text, re.M):
            return 0.7
        else:
            return 0.0

class JagsLexer(RegexLexer):
    """
    Pygments Lexer for JAGS.

    *New in Pygments 1.6.*
    """

    name = 'JAGS'
    aliases = ['jags']
    filenames = ['*.jag', '*.bug']

    ## JAGS
    _FUNCTIONS = [
        'abs', 'arccos', 'arccosh', 'arcsin', 'arcsinh', 'arctan', 'arctanh',
        'cos', 'cosh', 'cloglog',
        'equals', 'exp', 'icloglog', 'ifelse', 'ilogit', 'log', 'logfact',
        'loggam', 'logit', 'phi', 'pow', 'probit', 'round', 'sin', 'sinh',
        'sqrt', 'step', 'tan', 'tanh', 'trunc', 'inprod', 'interp.lin',
        'logdet', 'max', 'mean', 'min', 'prod', 'sum', 'sd', 'inverse',
        'rank', 'sort', 't', 'acos', 'acosh', 'asin', 'asinh', 'atan',
        # Truncation/Censoring (should I include)
        'T', 'I']
    # Distributions with density, probability and quartile functions
    _DISTRIBUTIONS = ['[dpq]%s' % x for x in
                           ['bern', 'beta', 'dchiqsqr', 'ddexp', 'dexp',
                            'df', 'gamma', 'gen.gamma', 'logis', 'lnorm',
                            'negbin', 'nchisqr', 'norm', 'par', 'pois', 'weib']]
    # Other distributions without density and probability
    _OTHER_DISTRIBUTIONS = [
        'dt', 'dunif', 'dbetabin', 'dbern', 'dbin', 'dcat', 'dhyper',
        'ddirch', 'dmnorm', 'dwish', 'dmt', 'dmulti', 'dbinom', 'dchisq',
        'dnbinom', 'dweibull', 'ddirich']

    tokens = {
        'whitespace' : [
            (r"\s+", Text),
            ],
        'names' : [
            # Regular variable names
            (r'[a-zA-Z][a-zA-Z0-9_.]*\b', Name),
            ],
        'comments' : [
            # do not use stateful comments
            (r'(?s)/\*.*?\*/', Comment.Multiline),
            # Comments
            (r'#.*$', Comment.Single),
            ],
        'root': [
            # Comments
            include('comments'),
            include('whitespace'),
            # Block start
            (r'(model|data)(\s+)({)',
             bygroups(Keyword.Namespace, Text, Punctuation)),
            (r'var(?![0-9a-zA-Z\._])', Keyword.Declaration),
            # Reserved Words
            (r'(for|in)(?![0-9a-zA-Z\._])', Keyword.Reserved),
            # Builtins
            # Need to use lookahead because . is a valid char
            (r'(%s)(?=\s*\()' % r'|'.join(_FUNCTIONS
                                 + _DISTRIBUTIONS
                                 + _OTHER_DISTRIBUTIONS),
             Name.Builtin),
            # Names
            include('names'),
            # Number Literals
            (r'[-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?', Number),
            (r'\[|\]|\(|\)|:|,|;', Punctuation),
            # Assignment operators
            (r'<-|~', Operator),
            # # JAGS includes many more than OpenBUGS
            (r'\+|-|\*|\/|\|\|[&]{2}|[<>=]=?|\^|%.*?%', Operator),
            (r'[{}]', Punctuation),
            ]
        }

    def analyse_text(text):
        if re.search(r'^\s*model\s*\{', text, re.M):
            if re.search(r'^\s*data\s*\{', text, re.M):
                return 0.9
            elif re.search(r'^\s*var', text, re.M):
                return 0.9
            else:
                return 0.3
        else:
            return 0

class StanLexer(RegexLexer):
    """
    Pygments Lexer for Stan models.

    *New in Pygments 1.6.*
    """

    name = 'Stan'
    aliases = ['stan']
    filenames = ['*.stan']

    _RESERVED = ('for', 'in', 'while', 'repeat', 'until', 'if',
                 'then', 'else', 'true', 'false', 'T',
                 'lower', 'upper', 'print')

    _TYPES = ('int', 'real', 'vector', 'simplex', 'ordered', 'row_vector',
              'matrix', 'corr_matrix', 'cov_matrix', 'positive_ordered')

    tokens = {
        'whitespace' : [
            (r"\s+", Text),
            ],
        'comments' : [
            (r'(?s)/\*.*?\*/', Comment.Multiline),
            # Comments
            (r'(//|#).*$', Comment.Single),
            ],
        'root': [
            # Stan is more restrictive on strings than this regex
            (r'"[^"]*"', String),
            # Comments
            include('comments'),
            # block start
            include('whitespace'),
            # Block start
            (r'(%s)(\s*)({)' %
             r'|'.join(('data', r'transformed\s+?data',
                        'parameters', r'transformed\s+parameters',
                        'model', r'generated\s+quantities')),
             bygroups(Keyword.Namespace, Text, Punctuation)),
            # Reserved Words
            (r'(%s)\b' % r'|'.join(_RESERVED), Keyword.Reserved),
            # Data types
            (r'(%s)\b' % r'|'.join(_TYPES), Keyword.Type),
            # Punctuation
            (r"[;:,\[\]()<>]", Punctuation),
            # Builtin
            (r'(%s)(?=\s*\()'
             % r'|'.join(_stan_builtins.FUNCTIONS
                         + _stan_builtins.DISTRIBUTIONS),
             Name.Builtin),
            (r'(%s)(?=\s*\()'
             % r'|'.join(_stan_builtins.CONSTANTS), Keyword.Constant),
            # Special names ending in __, like lp__
            (r'[A-Za-z][A-Za-z0-9_]*__\b', Name.Builtin.Pseudo),
            # Regular variable names
            (r'[A-Za-z][A-Za-z0-9_]*\b', Name),
            # Real Literals
            (r'-?[0-9]+(\.[0-9]+)?[eE]-?[0-9]+', Number.Float),
            (r'-?[0-9]*\.[0-9]*', Number.Float),
            # Integer Literals
            (r'-?[0-9]+', Number.Integer),
            # Assignment operators
            # SLexer makes these tokens Operators.
            (r'<-|~', Operator),
            # Infix and prefix operators (and = )
            (r"\+|-|\.?\*|\.?/|\\|'|=", Operator),
            # Block delimiters
            (r'[{}]', Punctuation),
            ]
        }

    def analyse_text(text):
        if re.search(r'^\s*parameters\s*\{', text, re.M):
            return 1.0
        else:
            return 0.0


class IDLLexer(RegexLexer):
    """
    Pygments Lexer for IDL (Interactive Data Language).

    *New in Pygments 1.6.*
    """
    name = 'IDL'
    aliases = ['idl']
    filenames = ['*.pro']
    mimetypes = ['text/idl']

    _RESERVED = ['and', 'begin', 'break', 'case', 'common', 'compile_opt',
                 'continue', 'do', 'else', 'end', 'endcase', 'elseelse',
                 'endfor', 'endforeach', 'endif', 'endrep', 'endswitch',
                 'endwhile', 'eq', 'for', 'foreach', 'forward_function',
                 'function', 'ge', 'goto', 'gt', 'if', 'inherits', 'le',
                 'lt', 'mod', 'ne', 'not', 'of', 'on_ioerror', 'or', 'pro',
                 'repeat', 'switch', 'then', 'until', 'while', 'xor']
    """Reserved words from: http://www.exelisvis.com/docs/reswords.html"""

    _BUILTIN_LIB = ['abs', 'acos', 'adapt_hist_equal', 'alog', 'alog10',
                    'amoeba', 'annotate', 'app_user_dir', 'app_user_dir_query',
                    'arg_present', 'array_equal', 'array_indices', 'arrow',
                    'ascii_template', 'asin', 'assoc', 'atan', 'axis',
                    'a_correlate', 'bandpass_filter', 'bandreject_filter',
                    'barplot', 'bar_plot', 'beseli', 'beselj', 'beselk',
                    'besely', 'beta', 'bilinear', 'binary_template', 'bindgen',
                    'binomial', 'bin_date', 'bit_ffs', 'bit_population',
                    'blas_axpy', 'blk_con', 'box_cursor', 'breakpoint',
                    'broyden', 'butterworth', 'bytarr', 'byte', 'byteorder',
                    'bytscl', 'caldat', 'calendar', 'call_external',
                    'call_function', 'call_method', 'call_procedure', 'canny',
                    'catch', 'cd', 'cdf_[0-9a-za-z_]*', 'ceil', 'chebyshev',
                    'check_math',
                    'chisqr_cvf', 'chisqr_pdf', 'choldc', 'cholsol', 'cindgen',
                    'cir_3pnt', 'close', 'cluster', 'cluster_tree', 'clust_wts',
                    'cmyk_convert', 'colorbar', 'colorize_sample',
                    'colormap_applicable', 'colormap_gradient',
                    'colormap_rotation', 'colortable', 'color_convert',
                    'color_exchange', 'color_quan', 'color_range_map', 'comfit',
                    'command_line_args', 'complex', 'complexarr', 'complexround',
                    'compute_mesh_normals', 'cond', 'congrid', 'conj',
                    'constrained_min', 'contour', 'convert_coord', 'convol',
                    'convol_fft', 'coord2to3', 'copy_lun', 'correlate', 'cos',
                    'cosh', 'cpu', 'cramer', 'create_cursor', 'create_struct',
                    'create_view', 'crossp', 'crvlength', 'cti_test',
                    'ct_luminance', 'cursor', 'curvefit', 'cvttobm', 'cv_coord',
                    'cw_animate', 'cw_animate_getp', 'cw_animate_load',
                    'cw_animate_run', 'cw_arcball', 'cw_bgroup', 'cw_clr_index',
                    'cw_colorsel', 'cw_defroi', 'cw_field', 'cw_filesel',
                    'cw_form', 'cw_fslider', 'cw_light_editor',
                    'cw_light_editor_get', 'cw_light_editor_set', 'cw_orient',
                    'cw_palette_editor', 'cw_palette_editor_get',
                    'cw_palette_editor_set', 'cw_pdmenu', 'cw_rgbslider',
                    'cw_tmpl', 'cw_zoom', 'c_correlate', 'dblarr', 'db_exists',
                    'dcindgen', 'dcomplex', 'dcomplexarr', 'define_key',
                    'define_msgblk', 'define_msgblk_from_file', 'defroi',
                    'defsysv', 'delvar', 'dendrogram', 'dendro_plot', 'deriv',
                    'derivsig', 'determ', 'device', 'dfpmin', 'diag_matrix',
                    'dialog_dbconnect', 'dialog_message', 'dialog_pickfile',
                    'dialog_printersetup', 'dialog_printjob',
                    'dialog_read_image', 'dialog_write_image', 'digital_filter',
                    'dilate', 'dindgen', 'dissolve', 'dist', 'distance_measure',
                    'dlm_load', 'dlm_register', 'doc_library', 'double',
                    'draw_roi', 'edge_dog', 'efont', 'eigenql', 'eigenvec',
                    'ellipse', 'elmhes', 'emboss', 'empty', 'enable_sysrtn',
                    'eof', 'eos_[0-9a-za-z_]*', 'erase', 'erf', 'erfc', 'erfcx',
                    'erode', 'errorplot', 'errplot', 'estimator_filter',
                    'execute', 'exit', 'exp', 'expand', 'expand_path', 'expint',
                    'extrac', 'extract_slice', 'factorial', 'fft', 'filepath',
                    'file_basename', 'file_chmod', 'file_copy', 'file_delete',
                    'file_dirname', 'file_expand_path', 'file_info',
                    'file_lines', 'file_link', 'file_mkdir', 'file_move',
                    'file_poll_input', 'file_readlink', 'file_same',
                    'file_search', 'file_test', 'file_which', 'findgen',
                    'finite', 'fix', 'flick', 'float', 'floor', 'flow3',
                    'fltarr', 'flush', 'format_axis_values', 'free_lun',
                    'fstat', 'fulstr', 'funct', 'fv_test', 'fx_root',
                    'fz_roots', 'f_cvf', 'f_pdf', 'gamma', 'gamma_ct',
                    'gauss2dfit', 'gaussfit', 'gaussian_function', 'gaussint',
                    'gauss_cvf', 'gauss_pdf', 'gauss_smooth', 'getenv',
                    'getwindows', 'get_drive_list', 'get_dxf_objects',
                    'get_kbrd', 'get_login_info', 'get_lun', 'get_screen_size',
                    'greg2jul', 'grib_[0-9a-za-z_]*', 'grid3', 'griddata',
                    'grid_input', 'grid_tps', 'gs_iter',
                    'h5[adfgirst]_[0-9a-za-z_]*', 'h5_browser', 'h5_close',
                    'h5_create', 'h5_get_libversion', 'h5_open', 'h5_parse',
                    'hanning', 'hash', 'hdf_[0-9a-za-z_]*', 'heap_free',
                    'heap_gc', 'heap_nosave', 'heap_refcount', 'heap_save',
                    'help', 'hilbert', 'histogram', 'hist_2d', 'hist_equal',
                    'hls', 'hough', 'hqr', 'hsv', 'h_eq_ct', 'h_eq_int',
                    'i18n_multibytetoutf8', 'i18n_multibytetowidechar',
                    'i18n_utf8tomultibyte', 'i18n_widechartomultibyte',
                    'ibeta', 'icontour', 'iconvertcoord', 'idelete', 'identity',
                    'idlexbr_assistant', 'idlitsys_createtool', 'idl_base64',
                    'idl_validname', 'iellipse', 'igamma', 'igetcurrent',
                    'igetdata', 'igetid', 'igetproperty', 'iimage', 'image',
                    'image_cont', 'image_statistics', 'imaginary', 'imap',
                    'indgen', 'intarr', 'interpol', 'interpolate',
                    'interval_volume', 'int_2d', 'int_3d', 'int_tabulated',
                    'invert', 'ioctl', 'iopen', 'iplot', 'ipolygon',
                    'ipolyline', 'iputdata', 'iregister', 'ireset', 'iresolve',
                    'irotate', 'ir_filter', 'isa', 'isave', 'iscale',
                    'isetcurrent', 'isetproperty', 'ishft', 'isocontour',
                    'isosurface', 'isurface', 'itext', 'itranslate', 'ivector',
                    'ivolume', 'izoom', 'i_beta', 'journal', 'json_parse',
                    'json_serialize', 'jul2greg', 'julday', 'keyword_set',
                    'krig2d', 'kurtosis', 'kw_test', 'l64indgen', 'label_date',
                    'label_region', 'ladfit', 'laguerre', 'laplacian',
                    'la_choldc', 'la_cholmprove', 'la_cholsol', 'la_determ',
                    'la_eigenproblem', 'la_eigenql', 'la_eigenvec', 'la_elmhes',
                    'la_gm_linear_model', 'la_hqr', 'la_invert',
                    'la_least_squares', 'la_least_square_equality',
                    'la_linear_equation', 'la_ludc', 'la_lumprove', 'la_lusol',
                    'la_svd', 'la_tridc', 'la_trimprove', 'la_triql',
                    'la_trired', 'la_trisol', 'least_squares_filter', 'leefilt',
                    'legend', 'legendre', 'linbcg', 'lindgen', 'linfit',
                    'linkimage', 'list', 'll_arc_distance', 'lmfit', 'lmgr',
                    'lngamma', 'lnp_test', 'loadct', 'locale_get',
                    'logical_and', 'logical_or', 'logical_true', 'lon64arr',
                    'lonarr', 'long', 'long64', 'lsode', 'ludc', 'lumprove',
                    'lusol', 'lu_complex', 'machar', 'make_array', 'make_dll',
                    'make_rt', 'map', 'mapcontinents', 'mapgrid', 'map_2points',
                    'map_continents', 'map_grid', 'map_image', 'map_patch',
                    'map_proj_forward', 'map_proj_image', 'map_proj_info',
                    'map_proj_init', 'map_proj_inverse', 'map_set',
                    'matrix_multiply', 'matrix_power', 'max', 'md_test',
                    'mean', 'meanabsdev', 'mean_filter', 'median', 'memory',
                    'mesh_clip', 'mesh_decimate', 'mesh_issolid', 'mesh_merge',
                    'mesh_numtriangles', 'mesh_obj', 'mesh_smooth',
                    'mesh_surfacearea', 'mesh_validate', 'mesh_volume',
                    'message', 'min', 'min_curve_surf', 'mk_html_help',
                    'modifyct', 'moment', 'morph_close', 'morph_distance',
                    'morph_gradient', 'morph_hitormiss', 'morph_open',
                    'morph_thin', 'morph_tophat', 'multi', 'm_correlate',
                    'ncdf_[0-9a-za-z_]*', 'newton', 'noise_hurl', 'noise_pick',
                    'noise_scatter', 'noise_slur', 'norm', 'n_elements',
                    'n_params', 'n_tags', 'objarr', 'obj_class', 'obj_destroy',
                    'obj_hasmethod', 'obj_isa', 'obj_new', 'obj_valid',
                    'online_help', 'on_error', 'open', 'oplot', 'oploterr',
                    'parse_url', 'particle_trace', 'path_cache', 'path_sep',
                    'pcomp', 'plot', 'plot3d', 'ploterr', 'plots', 'plot_3dbox',
                    'plot_field', 'pnt_line', 'point_lun', 'polarplot',
                    'polar_contour', 'polar_surface', 'poly', 'polyfill',
                    'polyfillv', 'polygon', 'polyline', 'polyshade', 'polywarp',
                    'poly_2d', 'poly_area', 'poly_fit', 'popd', 'powell',
                    'pref_commit', 'pref_get', 'pref_set', 'prewitt', 'primes',
                    'print', 'printd', 'product', 'profile', 'profiler',
                    'profiles', 'project_vol', 'psafm', 'pseudo',
                    'ps_show_fonts', 'ptrarr', 'ptr_free', 'ptr_new',
                    'ptr_valid', 'pushd', 'p_correlate', 'qgrid3', 'qhull',
                    'qromb', 'qromo', 'qsimp', 'query_ascii', 'query_bmp',
                    'query_csv', 'query_dicom', 'query_gif', 'query_image',
                    'query_jpeg', 'query_jpeg2000', 'query_mrsid', 'query_pict',
                    'query_png', 'query_ppm', 'query_srf', 'query_tiff',
                    'query_wav', 'radon', 'randomn', 'randomu', 'ranks',
                    'rdpix', 'read', 'reads', 'readu', 'read_ascii',
                    'read_binary', 'read_bmp', 'read_csv', 'read_dicom',
                    'read_gif', 'read_image', 'read_interfile', 'read_jpeg',
                    'read_jpeg2000', 'read_mrsid', 'read_pict', 'read_png',
                    'read_ppm', 'read_spr', 'read_srf', 'read_sylk',
                    'read_tiff', 'read_wav', 'read_wave', 'read_x11_bitmap',
                    'read_xwd', 'real_part', 'rebin', 'recall_commands',
                    'recon3', 'reduce_colors', 'reform', 'region_grow',
                    'register_cursor', 'regress', 'replicate',
                    'replicate_inplace', 'resolve_all', 'resolve_routine',
                    'restore', 'retall', 'return', 'reverse', 'rk4', 'roberts',
                    'rot', 'rotate', 'round', 'routine_filepath',
                    'routine_info', 'rs_test', 'r_correlate', 'r_test',
                    'save', 'savgol', 'scale3', 'scale3d', 'scope_level',
                    'scope_traceback', 'scope_varfetch', 'scope_varname',
                    'search2d', 'search3d', 'sem_create', 'sem_delete',
                    'sem_lock', 'sem_release', 'setenv', 'set_plot',
                    'set_shading', 'sfit', 'shade_surf', 'shade_surf_irr',
                    'shade_volume', 'shift', 'shift_diff', 'shmdebug', 'shmmap',
                    'shmunmap', 'shmvar', 'show3', 'showfont', 'simplex', 'sin',
                    'sindgen', 'sinh', 'size', 'skewness', 'skip_lun',
                    'slicer3', 'slide_image', 'smooth', 'sobel', 'socket',
                    'sort', 'spawn', 'spher_harm', 'sph_4pnt', 'sph_scat',
                    'spline', 'spline_p', 'spl_init', 'spl_interp', 'sprsab',
                    'sprsax', 'sprsin', 'sprstp', 'sqrt', 'standardize',
                    'stddev', 'stop', 'strarr', 'strcmp', 'strcompress',
                    'streamline', 'stregex', 'stretch', 'string', 'strjoin',
                    'strlen', 'strlowcase', 'strmatch', 'strmessage', 'strmid',
                    'strpos', 'strput', 'strsplit', 'strtrim', 'struct_assign',
                    'struct_hide', 'strupcase', 'surface', 'surfr', 'svdc',
                    'svdfit', 'svsol', 'swap_endian', 'swap_endian_inplace',
                    'symbol', 'systime', 's_test', 't3d', 'tag_names', 'tan',
                    'tanh', 'tek_color', 'temporary', 'tetra_clip',
                    'tetra_surface', 'tetra_volume', 'text', 'thin', 'threed',
                    'timegen', 'time_test2', 'tm_test', 'total', 'trace',
                    'transpose', 'triangulate', 'trigrid', 'triql', 'trired',
                    'trisol', 'tri_surf', 'truncate_lun', 'ts_coef', 'ts_diff',
                    'ts_fcast', 'ts_smooth', 'tv', 'tvcrs', 'tvlct', 'tvrd',
                    'tvscl', 'typename', 't_cvt', 't_pdf', 'uindgen', 'uint',
                    'uintarr', 'ul64indgen', 'ulindgen', 'ulon64arr', 'ulonarr',
                    'ulong', 'ulong64', 'uniq', 'unsharp_mask', 'usersym',
                    'value_locate', 'variance', 'vector', 'vector_field', 'vel',
                    'velovect', 'vert_t3d', 'voigt', 'voronoi', 'voxel_proj',
                    'wait', 'warp_tri', 'watershed', 'wdelete', 'wf_draw',
                    'where', 'widget_base', 'widget_button', 'widget_combobox',
                    'widget_control', 'widget_displaycontextmen', 'widget_draw',
                    'widget_droplist', 'widget_event', 'widget_info',
                    'widget_label', 'widget_list', 'widget_propertysheet',
                    'widget_slider', 'widget_tab', 'widget_table',
                    'widget_text', 'widget_tree', 'widget_tree_move',
                    'widget_window', 'wiener_filter', 'window', 'writeu',
                    'write_bmp', 'write_csv', 'write_gif', 'write_image',
                    'write_jpeg', 'write_jpeg2000', 'write_nrif', 'write_pict',
                    'write_png', 'write_ppm', 'write_spr', 'write_srf',
                    'write_sylk', 'write_tiff', 'write_wav', 'write_wave',
                    'wset', 'wshow', 'wtn', 'wv_applet', 'wv_cwt',
                    'wv_cw_wavelet', 'wv_denoise', 'wv_dwt', 'wv_fn_coiflet',
                    'wv_fn_daubechies', 'wv_fn_gaussian', 'wv_fn_haar',
                    'wv_fn_morlet', 'wv_fn_paul', 'wv_fn_symlet',
                    'wv_import_data', 'wv_import_wavelet', 'wv_plot3d_wps',
                    'wv_plot_multires', 'wv_pwt', 'wv_tool_denoise',
                    'xbm_edit', 'xdisplayfile', 'xdxf', 'xfont',
                    'xinteranimate', 'xloadct', 'xmanager', 'xmng_tmpl',
                    'xmtool', 'xobjview', 'xobjview_rotate',
                    'xobjview_write_image', 'xpalette', 'xpcolor', 'xplot3d',
                    'xregistered', 'xroi', 'xsq_test', 'xsurface', 'xvaredit',
                    'xvolume', 'xvolume_rotate', 'xvolume_write_image',
                    'xyouts', 'zoom', 'zoom_24']
    """Functions from: http://www.exelisvis.com/docs/routines-1.html"""

    tokens = {
        'root': [
            (r'^\s*;.*?\n', Comment.Singleline),
            (r'\b(' + '|'.join(_RESERVED) + r')\b', Keyword),
            (r'\b(' + '|'.join(_BUILTIN_LIB) + r')\b', Name.Builtin),
            (r'\+=|-=|\^=|\*=|/=|#=|##=|<=|>=|=', Operator),
            (r'\+\+|--|->|\+|-|##|#|\*|/|<|>|&&|\^|~|\|\|\?|:', Operator),
            (r'\b(mod=|lt=|le=|eq=|ne=|ge=|gt=|not=|and=|or=|xor=)', Operator),
            (r'\b(mod|lt|le|eq|ne|ge|gt|not|and|or|xor)\b', Operator),
            (r'\b[0-9](L|B|S|UL|ULL|LL)?\b', Number),
            (r'.', Text),
        ]
    }


class RdLexer(RegexLexer):
    """
    Pygments Lexer for R documentation (Rd) files

    This is a very minimal implementation, highlighting little more
    than the macros. A description of Rd syntax is found in `Writing R
    Extensions <http://cran.r-project.org/doc/manuals/R-exts.html>`_
    and `Parsing Rd files <developer.r-project.org/parseRd.pdf>`_.

    *New in Pygments 1.6.*
    """
    name = 'Rd'
    aliases = ['rd']
    filenames = ['*.Rd']
    mimetypes = ['text/x-r-doc']

    # To account for verbatim / LaTeX-like / and R-like areas
    # would require parsing.
    tokens = {
        'root' : [
            # catch escaped brackets and percent sign
            (r'\\[\\{}%]', String.Escape),
            # comments
            (r'%.*$', Comment),
            # special macros with no arguments
            (r'\\(?:cr|l?dots|R|tab)\b', Keyword.Constant),
            # macros
            (r'\\[a-zA-Z]+\b', Keyword),
            # special preprocessor macros
            (r'^\s*#(?:ifn?def|endif).*\b', Comment.Preproc),
            # non-escaped brackets
            (r'[{}]', Name.Builtin),
            # everything else
            (r'[^\\%\n{}]+', Text),
            (r'.', Text),
            ]
        }
