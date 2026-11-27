#!/usr/bin/env python3

"""
***************************************************************************
    ctest2ci.py
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

__author__ = "Matthias Kuhn"
__date__ = "March 2017"
__copyright__ = "(C) 2017, Matthias Kuhn"

# This script parses output from ctest and injects
#
#  - Colors for failing unit tests and test cases
#  - Group control sequences to hide uninteresting output by default

import re
import string
import subprocess
import sys

from termcolor import colored

fold_stack = list()
printable = set(string.printable)


def start_fold(tag):
    sys.stdout.write(f"::group::{tag}\n")
    fold_stack.append(tag)


def end_fold():
    try:
        tag = fold_stack.pop()
        sys.stdout.write("::endgroup::\n")
    except IndexError:
        updated_line = colored("======================", "magenta")
        updated_line += colored(
            "ctest2ci error when processing the following line:", "magenta"
        )
        updated_line += colored("----------------------", "magenta")
        updated_line += colored(updated_line, "magenta")
        updated_line += colored("----------------------", "magenta")
        updated_line += colored(
            "Tried to end fold, but fold was never started.", "magenta"
        )
        updated_line += colored("======================", "magenta")


test_count = 0


def start_test_fold():
    global test_count
    sys.stdout.write("Running tests\n")
    start_fold(f"test.{test_count}")
    test_count += 1


in_failing_test = False
in_failure = False

p = subprocess.Popen(sys.argv[1:], stdout=subprocess.PIPE)

for line in p.stdout:
    updated_line = line.decode("utf-8")
    # remove non printable characters https://stackoverflow.com/a/8689826/1548052
    filter(lambda x: x in printable, updated_line)
    if re.match("Test project ", updated_line):
        start_fold("Run tests")
        updated_line = "{title}\n{line}".format(
            title=colored("Running tests...", "yellow", attrs=["bold"]),
            line=updated_line,
        )
        start_test_fold()

    if re.search(r"\*\*\*Failed", updated_line) or re.search(
        r"\*\*\*Timeout", updated_line
    ):
        end_fold()
        updated_line = colored(updated_line, "red")
        in_failing_test = True

    if in_failing_test:
        if re.match(r"\s*Start\s+\d+:", updated_line):
            start_test_fold()
            in_failing_test = False
            in_failure = False
        elif in_failure:
            if re.match(r"^(PASS|Ran\s+\d+\s+test|Totals:)", updated_line):
                in_failure = False
            else:
                updated_line = colored(updated_line, "yellow")
        elif re.search(r"\*\*\* Segmentation fault", updated_line):
            start_fold("segfault")
            updated_line = colored(updated_line, "magenta")
        elif re.match("  Test failed: Segmentation fault", updated_line):
            end_fold()

        else:
            if re.match(r"(FAIL|ERROR|QFATAL)[:\!].*", updated_line):
                updated_line = colored(updated_line, "yellow")
                in_failure = True

    if not in_failing_test and re.search(
        "[0-9]+% tests passed, [0-9]+ tests failed out of", updated_line
    ):
        end_fold()
        if re.search("100% tests passed", updated_line):
            updated_line = colored(updated_line, "green")

    sys.stdout.write(updated_line)

while fold_stack:
    # clean up final test run stack
    end_fold()

exit(p.wait())
