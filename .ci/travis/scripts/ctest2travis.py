#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
***************************************************************************
    ctest2travis.py
    ---------------------
    Date                 : March 2017
    Copyright            : (C) 2017 by Matthias Kuhn
    Email                : matthias@opengis.ch
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Matthias Kuhn'
__date__ = 'March 2017'
__copyright__ = '(C) 2017, Matthias Kuhn'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

# This script parses output from ctest and injects
#
#  - Colors for failing unit tests and test cases
#
#  - `travis_fold` control sequences to hide uninteresting output by default

import sys
import re
import shlex
import subprocess
from termcolor import colored

fold_stack = list()


def start_fold(tag):
    sys.stdout.write('travis_fold:start:{}\n'.format(tag))
    fold_stack.append(tag)


def end_fold():
    try:
        tag = fold_stack.pop()
        sys.stdout.write('travis_fold:end:{}\n'.format(tag))
    except IndexError:
        updated_line = colored("======================", 'magenta')
        updated_line += colored("ctest2travis error when processing the following line:", 'magenta')
        updated_line += colored("----------------------", 'magenta')
        updated_line += colored(updated_line, 'magenta')
        updated_line += colored("----------------------", 'magenta')
        updated_line += colored("Tried to end fold, but fold was never started.", 'magenta')
        updated_line += colored("======================", 'magenta')


test_count = 0


def start_test_fold():
    global test_count
    sys.stdout.write('Running tests')
    start_fold('test.{}'.format(test_count))
    test_count += 1


in_failing_test = False
in_failure = False

p = subprocess.Popen(sys.argv[1:], stdout=subprocess.PIPE)

for line in p.stdout:
    updated_line = line.decode('utf-8')
    if re.match('Run dashboard with model Experimental', updated_line):
        start_fold('build')
        updated_line = '{title}\n{line}'.format(title=colored('Running tests...', 'yellow', attrs=['bold']), line=updated_line)

    elif re.match('Test project /home/travis/build/qgis/QGIS/build', updated_line):
        end_fold() # tag=build
        start_test_fold()

    if re.search('\*\*\*Failed', updated_line) or re.search('\*\*\*Timeout', updated_line):
        end_fold()
        updated_line = colored(updated_line, 'red')
        in_failing_test = True

    if in_failing_test:
        if re.match('        Start', updated_line):
            start_test_fold()
            in_failing_test = False
        elif in_failure:
            if re.match('PASS', updated_line) or re.match('Ran', updated_line):
                in_failure = False
            else:
                updated_line = colored(updated_line, 'yellow')
        elif re.search('\*\*\* Segmentation fault', updated_line):
            start_fold('segfault')
            updated_line = colored(updated_line, 'magenta')
        elif re.match('  Test failed: Segmentation fault', updated_line):
            end_fold()

        else:
            if re.match('(FAIL|ERROR)[:\!].*', updated_line):
                updated_line = colored(updated_line, 'yellow')
                in_failure = True

    if not in_failing_test and re.search('[0-9]+% tests passed, [0-9]+ tests failed out of', updated_line):
        end_fold()

        if re.search('100% tests passed', updated_line):
            updated_line = colored(updated_line, 'green')

    if re.match('Submit files', updated_line):
        start_fold('submit')
    elif re.search('Test results submitted to', updated_line):
        end_fold()

    sys.stdout.write(updated_line)

exit(p.wait())
