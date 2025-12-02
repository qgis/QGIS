#!/usr/bin/env python
###########################################################################
#    code_fixup.py
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

# This script automatically sorts includes in files
# It is not automatically run yet.

# Run it on whole code base with:
# ../scripts/sort_includes.sh --all

# or on modified files only with:
# ../scripts/sort_includes.sh

import re
import sys

from functools import cmp_to_key
from pathlib import Path

input_file = Path(sys.argv[1])
input_file_stem = input_file.stem

lines = [l[0:-1] if l[-1] == "\n" else l for l in open(input_file).readlines()]

# scan till first include
while True:
    try:
        line = lines[0]
    except IndexError:
        sys.exit(0)

    if re.match(r"\s*#include", line):
        break

    print(line)
    lines.pop(0)

# collect all includes
include_lines = []
while True:
    try:
        line = lines[0]
    except IndexError:
        break

    if not line.strip():
        lines.pop(0)
        continue

    if re.match(r"^\s*#include.*", line):
        lines.pop(0)
        include_lines.append(line)
        continue

    # TODO: handle headers in conditional #if blocks
    break

# de-dupe includes
include_lines = list(set(include_lines))

NON_STANDARD_NAMED_QGIS_HEADERS = (
    "gpsdata.h",
    "fromencodedcomponenthelper.h",
    "wkbptr.h",
    "characterwidget.h",
    "RTree.h",
    "info.h",
    "gmath.h",
    "feature.h",
    "labelposition.h",
    "pal.h",
    "layer.h",
    "poly2tri.h",
    "kdbush.hpp",
    "vector_tile.pb.h",
    "delaunator.hpp",
    "o0requestparameter.h",
    "o0globals.h",
    "o2.h",
    "ParametricLine.h",
    "Vector3D.h",
    "TriDecorator.h",
    "MathUtils.h",
    "TriangleInterpolator.h",
    "CloughTocherInterpolator.h",
    "HalfEdge.h",
    "modeltest.h",
    "libdxfrw.h",
    "dockModel.h",
    "o2replyserver.h",
    "LinTriangleInterpolator.h",
    "testqgsmaptoolutils.h",
    "tiny_gltf.h",
    "pointset.h",
    "testgeometryutils.h",
    "testqgsmaptoolutils.h",
    "problem.h",
    "meshoptimizer.h",
    "costcalculator.h",
    "geomfunction.h",
    "priorityqueue.h",
    "rulesDialog.h",
    "offline_editing_progress_dialog.h",
    "o0settingsstore.h",
    "NormVecDecorator.h",
    "libdwgr.h",
    "topolTest.h",
    "topolError.h",
    "offline_editing_plugin_gui.h",
    "drw_interface.h",
    "nanoarrow/nanoarrow.hpp",
    "pointset.h",
    "util.h",
    "internalexception.h",
    "palrtree.h",
    "inja/inja.hpp",
    "qsql_ocispatial.h",
    "qobjectuniqueptr.h",
)

SPATIALITE_HEADERS = ("spatialite.h", "spatialite/gaiageo.h")

SPECIAL_CASE_LAST_INCLUDES = ("fcgi_stdio.h",)


def sort_standard_includes(includes: list[str]) -> list[str]:
    def compare(item1, item2):
        # spatialite headers must be last
        if item1 in SPATIALITE_HEADERS and not item2 in SPATIALITE_HEADERS:
            return 1
        elif item2 in SPATIALITE_HEADERS and not item1 in SPATIALITE_HEADERS:
            return -1
        elif item1 < item2:
            return -1
        elif item1 > item2:
            return 1
        else:
            return 0

    return sorted(includes, key=cmp_to_key(compare))


def print_sorted_includes(includes: list[str]):
    matching_header = None
    moc_header = None
    qgs_config_include = None
    ui_includes = []
    std_includes = []
    qt_includes = []
    qgis_includes = []
    special_case_last_includes = []

    for include in includes:
        header_match = re.match(r'^\s*#include [<"](.*)[">]', include)
        assert header_match

        header = header_match.group(1)
        if header == f"{input_file_stem}.h":
            matching_header = header
        elif header == f"moc_{input_file_stem}.cpp":
            moc_header = header
        elif header == "qgsconfig.h":
            qgs_config_include = header
        elif header in SPECIAL_CASE_LAST_INCLUDES:
            special_case_last_includes.append(header)
        elif (
            re.match(r"^(?:.*/)?qgi?s.*\.h", header, re.IGNORECASE)
            or header in NON_STANDARD_NAMED_QGIS_HEADERS
            or header.startswith("pal/")
            or header.startswith("lazperf/")
        ):
            qgis_includes.append(header)
        elif re.match(r"^ui_.*\.h", header, re.IGNORECASE):
            ui_includes.append(header)
        elif re.match(r"^Q", header, re.IGNORECASE):
            qt_includes.append(header)
        else:
            std_includes.append(header)

    qt_includes = sorted(qt_includes)
    qgis_includes = sorted(qgis_includes)
    ui_includes = sorted(ui_includes)
    std_includes = sort_standard_includes(std_includes)
    special_case_last_includes = sorted(special_case_last_includes)

    if qgs_config_include:
        # this header MUST come first, as it defines macros which may
        # impact on how other headers behave
        print(f'#include "{qgs_config_include}"')
    if ui_includes:
        for header in ui_includes:
            print(f'#include "{header}"')
    if matching_header:
        print(f'#include "{matching_header}"')

    if qgs_config_include or matching_header or ui_includes:
        print()

    if std_includes:
        for header in std_includes:
            print(f"#include <{header}>")
        print()

    if qgis_includes:
        for header in qgis_includes:
            print(f'#include "{header}"')
        print()

    if qt_includes:
        for header in qt_includes:
            print(f"#include <{header}>")
        print()

    if moc_header:
        # moc include should come last -- this may rely on other includes
        # to resolve forward declared classes
        print(f'#include "{moc_header}"')
        print()

    if special_case_last_includes:
        for header in special_case_last_includes:
            print(f"#include <{header}>")
        print()


print_sorted_includes(include_lines)

for line in lines:
    print(line)
