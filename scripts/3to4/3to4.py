#!/usr/bin/env python3

"""
***************************************************************************
    3to4.py
    ---------------------
    Date                 : 2023 December
    Copyright            : (C) 2023 by Julien Cabieces
    Email                : julien dot cabieces at oslandia dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

"""
Migrate a folder containing python files from QGIS3/Qt5 to QGIS4/Qt6

Highly inspired from this video https://www.youtube.com/watch?v=G1omxo5pphw

needed tools:
pip install astpretty tokenize-rt

Few usefull commands:

- display python file ast
astpretty --no-show-offsets myfile.py

- display file tokens
tokenize-rt myfile.py
"""

__author__ = 'Julien Cabieces'
__date__ = '2023 December'
__copyright__ = '(C) 2023, Julien Cabieces'


import argparse
import ast
import glob
import os
import inspect

from collections import defaultdict

from tokenize_rt import Offset, src_to_tokens, tokens_to_src, reversed_enumerate, Token

from typing import Sequence

from PyQt6 import QtCore, QtGui, QtWidgets

qmetatype_mapping = {"String": "QString",
                     "Invalid": "UnknownType",
                     "Date": "QDate",
                     "Time": "QTime",
                     "DateTime": "QDateTime",
                     "ByteArray": "QByteArray",
                     "StringList": "QStringList"
                     }

qt_enums = {}


def fix_file(filename: str, args: argparse.Namespace) -> int:

    # if (filename != "/home/julien/work/QGIS/.worktrees/pyqt6/build-fedora/output/python/plugins/processing/algs/qgis/QgisAlgorithmProvider.py"):
    #     return 0

    with (open(filename, encoding='UTF-8') as f):
        contents = f.read()

    fix_qvariant_type = [] # QVariant.Int, QVariant.Double ...
    fix_pyqt_import = [] # from PyQt5.QtXXX
    fix_qt_enums = [] # Unscopping of enums

    tree = ast.parse(contents, filename=filename)
    for node in ast.walk(tree):

        if (isinstance(node, ast.Attribute) and isinstance(node.value, ast.Name)
                and node.value.id == "QVariant"):
            fix_qvariant_type.append(Offset(node.lineno, node.col_offset))

        if (isinstance(node, ast.Attribute) and isinstance(node.value, ast.Name)
                and (node.value.id, node.attr) in qt_enums):
            fix_qt_enums.append(Offset(node.lineno, node.col_offset))

        elif (isinstance(node, ast.ImportFrom) and node.module and node.module.startswith("PyQt5.")):
            fix_pyqt_import.append(Offset(node.lineno, node.col_offset))

    if not fix_qvariant_type and not fix_pyqt_import and not fix_qt_enums:
        return 0

    tokens = src_to_tokens(contents)
    for i, token in reversed_enumerate(tokens):
        if token.offset in fix_qvariant_type:
            assert tokens[i].src == "QVariant"
            assert tokens[i + 1].src == "."
            tokens[i] = tokens[i]._replace(src="QMetaType.Type")
            attr = tokens[i + 2].src
            if attr in qmetatype_mapping:
                tokens[i + 2] = tokens[i + 2]._replace(src=qmetatype_mapping[attr])

        if token.offset in fix_pyqt_import:
            assert tokens[i + 2].src == "PyQt5"
            tokens[i + 2] = tokens[i + 2]._replace(src="qgis.PyQt")

        if token.offset in fix_qt_enums:
            assert tokens[i + 1].src == "."
            enum_name = qt_enums[(tokens[i].src, tokens[i + 2].src)]
            assert enum_name
            tokens[i + 2] = tokens[i + 2]._replace(src=f"{enum_name}.{tokens[i+2].src}")

    new_contents = tokens_to_src(tokens)
    with (open(filename, 'w') as f):
        f.write(new_contents)

    return new_contents != contents


def get_class_enums(item):
    if not inspect.isclass(item):
        return

    for key, value in item.__dict__.items():
        if inspect.isclass(value) and type(value).__name__ == 'EnumType':
            for e in value:
                qt_enums[(item.__name__, e.name)] = f"{value.__name__}"
        elif inspect.isclass(value):
            get_class_enums(value)


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument('directory')
    # parser.add_argument('--exit-zero-even-if-changed', action='store_true')
    # parser.add_argument('--py35-plus', action='store_true')
    # parser.add_argument('--py36-plus', action='store_true')
    args = parser.parse_args(argv)

    print(os.path.join(args.directory, "*.py"))

    for module in QtCore, QtGui, QtWidgets:
        for key, value in module.__dict__.items():
            get_class_enums(value)

    ret = 0
    for filename in glob.glob(os.path.join(args.directory, "**/*.py"), recursive=True):
        ret |= fix_file(filename, args)
    return ret


if __name__ == '__main__':
    raise SystemExit(main())
