# -*- coding: utf-8 -*-
"""
    pygments.cmdline
    ~~~~~~~~~~~~~~~~

    Command line interface.

    :copyright: Copyright 2006-2013 by the Pygments team, see AUTHORS.
    :license: BSD, see LICENSE for details.
"""
import sys
import getopt
from textwrap import dedent

from pygments import __version__, highlight
from pygments.util import ClassNotFound, OptionError, docstring_headline
from pygments.lexers import get_all_lexers, get_lexer_by_name, get_lexer_for_filename, \
     find_lexer_class, guess_lexer, TextLexer
from pygments.formatters import get_all_formatters, get_formatter_by_name, \
     get_formatter_for_filename, find_formatter_class, \
     TerminalFormatter  # pylint:disable-msg=E0611
from pygments.filters import get_all_filters, find_filter_class
from pygments.styles import get_all_styles, get_style_by_name


USAGE = """\
Usage: %s [-l <lexer> | -g] [-F <filter>[:<options>]] [-f <formatter>]
          [-O <options>] [-P <option=value>] [-o <outfile>] [<infile>]

       %s -S <style> -f <formatter> [-a <arg>] [-O <options>] [-P <option=value>]
       %s -L [<which> ...]
       %s -N <filename>
       %s -H <type> <name>
       %s -h | -V

Highlight the input file and write the result to <outfile>.

If no input file is given, use stdin, if -o is not given, use stdout.

<lexer> is a lexer name (query all lexer names with -L). If -l is not
given, the lexer is guessed from the extension of the input file name
(this obviously doesn't work if the input is stdin).  If -g is passed,
attempt to guess the lexer from the file contents, or pass through as
plain text if this fails (this can work for stdin).

Likewise, <formatter> is a formatter name, and will be guessed from
the extension of the output file name. If no output file is given,
the terminal formatter will be used by default.

With the -O option, you can give the lexer and formatter a comma-
separated list of options, e.g. ``-O bg=light,python=cool``.

The -P option adds lexer and formatter options like the -O option, but
you can only give one option per -P. That way, the option value may
contain commas and equals signs, which it can't with -O, e.g.
``-P "heading=Pygments, the Python highlighter".

With the -F option, you can add filters to the token stream, you can
give options in the same way as for -O after a colon (note: there must
not be spaces around the colon).

The -O, -P and -F options can be given multiple times.

With the -S option, print out style definitions for style <style>
for formatter <formatter>. The argument given by -a is formatter
dependent.

The -L option lists lexers, formatters, styles or filters -- set
`which` to the thing you want to list (e.g. "styles"), or omit it to
list everything.

The -N option guesses and prints out a lexer name based solely on
the given filename. It does not take input or highlight anything.
If no specific lexer can be determined "text" is returned.

The -H option prints detailed help for the object <name> of type <type>,
where <type> is one of "lexer", "formatter" or "filter".

The -h option prints this help.
The -V option prints the package version.
"""


def _parse_options(o_strs):
    opts = {}
    if not o_strs:
        return opts
    for o_str in o_strs:
        if not o_str:
            continue
        o_args = o_str.split(',')
        for o_arg in o_args:
            o_arg = o_arg.strip()
            try:
                o_key, o_val = o_arg.split('=')
                o_key = o_key.strip()
                o_val = o_val.strip()
            except ValueError:
                opts[o_arg] = True
            else:
                opts[o_key] = o_val
    return opts


def _parse_filters(f_strs):
    filters = []
    if not f_strs:
        return filters
    for f_str in f_strs:
        if ':' in f_str:
            fname, fopts = f_str.split(':', 1)
            filters.append((fname, _parse_options([fopts])))
        else:
            filters.append((f_str, {}))
    return filters


def _print_help(what, name):
    try:
        if what == 'lexer':
            cls = find_lexer_class(name)
            print "Help on the %s lexer:" % cls.name
            print dedent(cls.__doc__)
        elif what == 'formatter':
            cls = find_formatter_class(name)
            print "Help on the %s formatter:" % cls.name
            print dedent(cls.__doc__)
        elif what == 'filter':
            cls = find_filter_class(name)
            print "Help on the %s filter:" % name
            print dedent(cls.__doc__)
    except AttributeError:
        print >>sys.stderr, "%s not found!" % what


def _print_list(what):
    if what == 'lexer':
        print
        print "Lexers:"
        print "~~~~~~~"

        info = []
        for fullname, names, exts, _ in get_all_lexers():
            tup = (', '.join(names)+':', fullname,
                   exts and '(filenames ' + ', '.join(exts) + ')' or '')
            info.append(tup)
        info.sort()
        for i in info:
            print ('* %s\n    %s %s') % i

    elif what == 'formatter':
        print
        print "Formatters:"
        print "~~~~~~~~~~~"

        info = []
        for cls in get_all_formatters():
            doc = docstring_headline(cls)
            tup = (', '.join(cls.aliases) + ':', doc, cls.filenames and
                   '(filenames ' + ', '.join(cls.filenames) + ')' or '')
            info.append(tup)
        info.sort()
        for i in info:
            print ('* %s\n    %s %s') % i

    elif what == 'filter':
        print
        print "Filters:"
        print "~~~~~~~~"

        for name in get_all_filters():
            cls = find_filter_class(name)
            print "* " + name + ':'
            print "    %s" % docstring_headline(cls)

    elif what == 'style':
        print
        print "Styles:"
        print "~~~~~~~"

        for name in get_all_styles():
            cls = get_style_by_name(name)
            print "* " + name + ':'
            print "    %s" % docstring_headline(cls)


def main(args=sys.argv):
    """
    Main command line entry point.
    """
    # pylint: disable-msg=R0911,R0912,R0915

    usage = USAGE % ((args[0],) * 6)

    if sys.platform in ['win32', 'cygwin']:
        try:
            # Provide coloring under Windows, if possible
            import colorama
            colorama.init()
        except ImportError:
            pass

    try:
        popts, args = getopt.getopt(args[1:], "l:f:F:o:O:P:LS:a:N:hVHg")
    except getopt.GetoptError, err:
        print >>sys.stderr, usage
        return 2
    opts = {}
    O_opts = []
    P_opts = []
    F_opts = []
    for opt, arg in popts:
        if opt == '-O':
            O_opts.append(arg)
        elif opt == '-P':
            P_opts.append(arg)
        elif opt == '-F':
            F_opts.append(arg)
        opts[opt] = arg

    if not opts and not args:
        print usage
        return 0

    if opts.pop('-h', None) is not None:
        print usage
        return 0

    if opts.pop('-V', None) is not None:
        print 'Pygments version %s, (c) 2006-2013 by Georg Brandl.' % __version__
        return 0

    # handle ``pygmentize -L``
    L_opt = opts.pop('-L', None)
    if L_opt is not None:
        if opts:
            print >>sys.stderr, usage
            return 2

        # print version
        main(['', '-V'])
        if not args:
            args = ['lexer', 'formatter', 'filter', 'style']
        for arg in args:
            _print_list(arg.rstrip('s'))
        return 0

    # handle ``pygmentize -H``
    H_opt = opts.pop('-H', None)
    if H_opt is not None:
        if opts or len(args) != 2:
            print >>sys.stderr, usage
            return 2

        what, name = args
        if what not in ('lexer', 'formatter', 'filter'):
            print >>sys.stderr, usage
            return 2

        _print_help(what, name)
        return 0

    # parse -O options
    parsed_opts = _parse_options(O_opts)
    opts.pop('-O', None)

    # parse -P options
    for p_opt in P_opts:
        try:
            name, value = p_opt.split('=', 1)
        except ValueError:
            parsed_opts[p_opt] = True
        else:
            parsed_opts[name] = value
    opts.pop('-P', None)

    # handle ``pygmentize -N``
    infn = opts.pop('-N', None)
    if infn is not None:
        try:
            lexer = get_lexer_for_filename(infn, **parsed_opts)
        except ClassNotFound, err:
            lexer = TextLexer()
        except OptionError, err:
            print >>sys.stderr, 'Error:', err
            return 1

        print lexer.aliases[0]
        return 0

    # handle ``pygmentize -S``
    S_opt = opts.pop('-S', None)
    a_opt = opts.pop('-a', None)
    if S_opt is not None:
        f_opt = opts.pop('-f', None)
        if not f_opt:
            print >>sys.stderr, usage
            return 2
        if opts or args:
            print >>sys.stderr, usage
            return 2

        try:
            parsed_opts['style'] = S_opt
            fmter = get_formatter_by_name(f_opt, **parsed_opts)
        except ClassNotFound, err:
            print >>sys.stderr, err
            return 1

        arg = a_opt or ''
        try:
            print fmter.get_style_defs(arg)
        except Exception, err:
            print >>sys.stderr, 'Error:', err
            return 1
        return 0

    # if no -S is given, -a is not allowed
    if a_opt is not None:
        print >>sys.stderr, usage
        return 2

    # parse -F options
    F_opts = _parse_filters(F_opts)
    opts.pop('-F', None)

    # select formatter
    outfn = opts.pop('-o', None)
    fmter = opts.pop('-f', None)
    if fmter:
        try:
            fmter = get_formatter_by_name(fmter, **parsed_opts)
        except (OptionError, ClassNotFound), err:
            print >>sys.stderr, 'Error:', err
            return 1

    if outfn:
        if not fmter:
            try:
                fmter = get_formatter_for_filename(outfn, **parsed_opts)
            except (OptionError, ClassNotFound), err:
                print >>sys.stderr, 'Error:', err
                return 1
        try:
            outfile = open(outfn, 'wb')
        except Exception, err:
            print >>sys.stderr, 'Error: cannot open outfile:', err
            return 1
    else:
        if not fmter:
            fmter = TerminalFormatter(**parsed_opts)
        outfile = sys.stdout

    # select lexer
    lexer = opts.pop('-l', None)
    if lexer:
        try:
            lexer = get_lexer_by_name(lexer, **parsed_opts)
        except (OptionError, ClassNotFound), err:
            print >>sys.stderr, 'Error:', err
            return 1

    if args:
        if len(args) > 1:
            print >>sys.stderr, usage
            return 2

        infn = args[0]
        try:
            code = open(infn, 'rb').read()
        except Exception, err:
            print >>sys.stderr, 'Error: cannot read infile:', err
            return 1

        if not lexer:
            try:
                lexer = get_lexer_for_filename(infn, code, **parsed_opts)
            except ClassNotFound, err:
                if '-g' in opts:
                    try:
                        lexer = guess_lexer(code, **parsed_opts)
                    except ClassNotFound:
                        lexer = TextLexer(**parsed_opts)
                else:
                    print >>sys.stderr, 'Error:', err
                    return 1
            except OptionError, err:
                print >>sys.stderr, 'Error:', err
                return 1

    else:
        if '-g' in opts:
            code = sys.stdin.read()
            try:
                lexer = guess_lexer(code, **parsed_opts)
            except ClassNotFound:
                lexer = TextLexer(**parsed_opts)
        elif not lexer:
            print >>sys.stderr, 'Error: no lexer name given and reading ' + \
                                'from stdin (try using -g or -l <lexer>)'
            return 2
        else:
            code = sys.stdin.read()

    # No encoding given? Use latin1 if output file given,
    # stdin/stdout encoding otherwise.
    # (This is a compromise, I'm not too happy with it...)
    if 'encoding' not in parsed_opts and 'outencoding' not in parsed_opts:
        if outfn:
            # encoding pass-through
            fmter.encoding = 'latin1'
        else:
            if sys.version_info < (3,):
                # use terminal encoding; Python 3's terminals already do that
                lexer.encoding = getattr(sys.stdin, 'encoding',
                                         None) or 'ascii'
                fmter.encoding = getattr(sys.stdout, 'encoding',
                                         None) or 'ascii'
    elif not outfn and sys.version_info > (3,):
        # output to terminal with encoding -> use .buffer
        outfile = sys.stdout.buffer

    # ... and do it!
    try:
        # process filters
        for fname, fopts in F_opts:
            lexer.add_filter(fname, **fopts)
        highlight(code, lexer, fmter, outfile)
    except Exception, err:
        import traceback
        info = traceback.format_exception(*sys.exc_info())
        msg = info[-1].strip()
        if len(info) >= 3:
            # extract relevant file and position info
            msg += '\n   (f%s)' % info[-2].split('\n')[0].strip()[1:]
        print >>sys.stderr
        print >>sys.stderr, '*** Error while highlighting:'
        print >>sys.stderr, msg
        return 1

    return 0
