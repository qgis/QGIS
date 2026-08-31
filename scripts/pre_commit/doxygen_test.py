#!/usr/bin/env python3
###########################################################################
#    doxygen_test.py
#    ---------------------
#    Date                 : December 2017
#    Copyright            : (C) 2017 by Denis Rouzaud
#    Email                : denis.rouzaud@gmail.com
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################
"""Check the doxygen layout of header files. Requires nothing but Python 3."""

import re
import sys
from pathlib import Path

CHECKS = [
    (
        r"\\(note|since)[^\n]+(\n\s*\* [^\n]+)*\n\s*\* \\return",
        r"\return(s) should be placed before \note and \since",
    ),
    (
        r"(\\(deprecated|since)[^\n]+\n)+\s*\*[^/](?!\s*\\(deprecated|since))",
        r"\deprecated and \since should be placed at the end of command blocks",
    ),
    (r"\\ingroup 3d", r"use \ingroup qgis_3d, not \ingroup 3d"),
    (r"~~~\{\.\w+\}", r"code snippets should use \code{.xx} rather than ~~~{.xx}"),
]


def main(names):
    found = 0
    for name in names:
        path = Path(name)
        if path.suffix != ".h" or not path.is_file():
            continue
        content = path.read_text(encoding="utf-8", errors="surrogateescape")
        for pattern, message in CHECKS:
            for match in re.finditer(pattern, content):
                line = content.count("\n", 0, match.start()) + 1
                first = match.group().splitlines()[0].strip()
                print(f"{name}:{line} {message}\n  {first}")
                found += 1
    return 1 if found else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
