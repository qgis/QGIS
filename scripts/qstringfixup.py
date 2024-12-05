#!/usr/bin/env python
###########################################################################
#    qstringfixup.py
#    ---------------
#    Date                 : October 2020
#    Copyright            : (C) 2020 by Even Rouault
#    Email                : even.rouault@spatialys.com
###########################################################################
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
###########################################################################

# This script fixes several suboptimal uses of QStringLiteral where QLatin1String would be better
# It is not automatically run yet.

# Run it on whole code base with:
# ../scripts/qstringfixup.sh --all

# or on modified files only with:
# ../scripts/qstringfixup.sh

import re
import sys

lines = [l[0:-1] if l[-1] == "\n" else l for l in open(sys.argv[1]).readlines()]

# Double quoted strings that only include ASCII characters
string_literal = r"""(R?"(?:(?:\\['"\\nrt])|[\x00-\x21\x23-\x5B\x5D-\x7F])+?")"""

# Single quoted ASCII character
char_literal = r"""('(?:\\['"\\nrt]|[\x00-\x26\x28-\x5B\x5D-\x7F])')"""

# Simple expression like foo or foo.bar() or foo.bar(baz, baw)
simple_expr = (
    r"""([a-zA-Z0-9_:<>]+(?:\.(?:[a-zA-Z0-9_]+\([^\(\)]*\)|[a-zA-Z0-9_]+))?)"""
)

qsl = rf"""QStringLiteral\( {string_literal} \)"""

# Find lines like "    foo += QStringLiteral( "bla" );  // optional comment"
pattern_plus_equal = re.compile(rf"^([ ]*)([^ ]+) \+= {qsl};([ ]*//.*)?$")

# Find patterns like "...QString( tr( "foo" ) )..."
pattern_qstring_tr = re.compile(rf"""(.*)QString\( tr\( {string_literal} \) \)(.*)""")

# Find patterns like "...== QStringLiteral( "foo" ) something that is not like .arg()"
pattern_equalequal_qsl = re.compile(
    r"(.*)(==|!=) " + qsl + r"( \)| \|\|| &&| }|;| \?| ,)(.*)"
)

# Find patterns like "...startsWith( QStringLiteral( "foo" ) )..."
pattern_startswith_qsl = re.compile(
    rf"(.*)\.(startsWith|endsWith|indexOf|lastIndexOf|compare)\( {qsl} \)(.*)"
)

# .replace( 'a' or simple_expr or qsl, QStringLiteral( "foo" ) )
replace_char_qsl = re.compile(rf"""(.*)\.replace\( {char_literal}, {qsl} \)(.*)""")
replace_str_qsl = re.compile(rf"""(.*)\.replace\( {string_literal}, {qsl} \)(.*)""")
# Do not use that: if simple_expr is a QRegExp, there is no QString::replace(QRegExp, QLatin1String)
# replace_simple_expr_qsl = re.compile(r"""(.*)\.replace\( {simple_expr}, {qsl} \)(.*)""".format(simple_expr=simple_expr, qsl=qsl))

# .replace( QStringLiteral( "foo" ), QStringLiteral( "foo" ) )
replace_qsl_qsl = re.compile(r"""(.*)\.replace\( {qsl}, {qsl} \)(.*)""".format(qsl=qsl))

# .replace( QStringLiteral( "foo" ), something
replace_qsl_something = re.compile(rf"""(.*)\.replace\( {qsl}, (.+)""")

# .arg( QStringLiteral( "foo" ) )
# note: QString QString::arg(QLatin1String a) added in QT 5.10, but using QLatin1String() will work with older too
arg_qsl = re.compile(rf"""(.*)\.arg\( {qsl} \)(.*)""")

# .join( QStringLiteral( "foo" ) )
join = re.compile(rf"""(.*)\.join\( {qsl} \)(.*)""")

# if QT >= 5.14 .compare would be ok
qlatin1str_single_char = re.compile(
    r"""(.*)(.startsWith\(|.endsWith\(|.indexOf\(|.lastIndexOf\(|\+=) QLatin1String\( ("[^"]") \)(.*)"""
)


def qlatin1char_or_string(x):
    """x is a double quoted string"""
    if len(x) == 3 and x[1] == "'":
        return "QLatin1Char( '\\'' )"
    elif len(x) == 3:
        return "QLatin1Char( '" + x[1] + "' )"
    elif len(x) == 4 and x[1] == "\\":
        return "QLatin1Char( '" + x[1:3] + "' )"
    else:
        return "QLatin1String( " + x + " )"


i = 0
while i < len(lines):
    line = lines[i]
    modified = False

    m = pattern_plus_equal.match(line)
    if m:
        g = m.groups()
        newline = g[0] + g[1] + " += "
        newline += "QLatin1String( " + g[2] + " );"
        if g[3]:
            newline += g[3]
        line = newline

    m = pattern_qstring_tr.match(line)
    if m:
        g = m.groups()
        newline = g[0] + "tr( " + g[1] + " )"
        if g[2]:
            newline += g[2]
        line = newline

    while True:
        m = pattern_equalequal_qsl.match(line)
        if m and "qgetenv" not in line and "h.first" not in line:
            g = m.groups()
            newline = g[0] + g[1] + " QLatin1String( " + g[2] + " )" + g[3]
            if g[4]:
                newline += g[4]
            line = newline
        else:
            break

    while True:
        m = pattern_startswith_qsl.match(line)
        if m:
            g = m.groups()
            newline = g[0] + "." + g[1] + "( QLatin1String( " + g[2] + " ) )"
            if g[3]:
                newline += g[3]
            line = newline
        else:
            break

    while True:
        m = replace_char_qsl.match(line)
        if not m:
            m = replace_str_qsl.match(line)
        # if not m:
        #     m = replace_simple_expr_qsl.match(line)
        if m:
            g = m.groups()
            newline = g[0] + ".replace( " + g[1] + ", QLatin1String( " + g[2] + " ) )"
            if g[3]:
                newline += g[3]
            line = newline
        else:
            break

    while True:
        m = replace_qsl_qsl.match(line)
        if m:
            g = m.groups()
            newline = (
                g[0]
                + ".replace( QLatin1String( "
                + g[1]
                + " ), QLatin1String( "
                + g[2]
                + " ) )"
            )
            if g[3]:
                newline += g[3]
            line = newline
        else:
            break

    while True:
        m = replace_qsl_something.match(line)
        if m:
            g = m.groups()
            newline = g[0] + ".replace( QLatin1String( " + g[1] + " ), " + g[2]
            line = newline
        else:
            break

    while True:
        m = arg_qsl.match(line)
        if m:
            g = m.groups()
            newline = g[0] + ".arg( QLatin1String( " + g[1] + ") )"
            if g[2]:
                newline += g[2]
            line = newline
        else:
            break

    while True:
        m = join.match(line)
        if m:
            g = m.groups()
            newline = g[0] + ".join( " + qlatin1char_or_string(g[1]) + " )"
            if g[2]:
                newline += g[2]
            line = newline
        else:
            break

    while True:
        m = qlatin1str_single_char.match(line)
        if m:
            g = m.groups()
            newline = g[0] + g[1] + " " + qlatin1char_or_string(g[2])
            if g[3]:
                newline += g[3]
            line = newline
        else:
            break

    print(line)
    i += 1
