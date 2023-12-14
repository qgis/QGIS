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

from collections import defaultdict

from tokenize_rt import Offset, src_to_tokens, tokens_to_src, reversed_enumerate, Token

from typing import Sequence

qmetatype_mapping = {"String": "QString",
                     "Invalid": "UnknownType",
                     "Date": "QDate",
                     "Time": "QTime",
                     "DateTime": "QDateTime",
                     "ByteArray": "QByteArray",
                     "StringList": "QStringList"
                     }

# key: class that has moved from one module to another
# value: (qt5 module, qt6 module)
moved_classes = {"QAction": ("QtWidgets", "QtGui")}

NEW_LINE = Token('NEWLINE', '\n')
SPACE = Token('UNIMPORTANT_WS', ' ')
COMMA = Token('OP', ',')
POINT = Token('OP', '.')


def import_tokens(module, classes):
    classes_str = ",".join(classes)
    return src_to_tokens(f"from qgis.PyQt.{module} import {classes_str}")


def fix_file(filename: str, args: argparse.Namespace) -> int:

    if (filename != "../python/plugins/processing/gui/BatchPanel.py"):
        return 0

    with (open(filename, encoding='UTF-8') as f):
        contents = f.read()

    fix_qvariant_type = [] # QVariant.Int, QVariant.Double ...
    fix_pyqt_import = [] # from PyQt5.QtXXX
    fix_moved_classes = [] # classes changing from modules

    new_import_qt5 = defaultdict(list)
    new_import_qt6 = defaultdict(list)

    last_import = None

    tree = ast.parse(contents, filename=filename)
    for node in ast.walk(tree):

        if (isinstance(node, ast.ImportFrom)):
            last_import = Offset(node.end_lineno, node.end_col_offset)

        if (isinstance(node, ast.ImportFrom) and node.module in ["PyQt5.QtWidgets", "qgis.PyQt.QtWidgets", "PyQt5.QtGui", "qgis.PyQt.QtGui"]):
            for name in node.names:
                if name.name in moved_classes:
                    fix_moved_classes.append(Offset(name.lineno, name.col_offset))
                    new_import_qt5[moved_classes[name.name][0]].append(name.name)
                    new_import_qt6[moved_classes[name.name][1]].append(name.name)

        if (isinstance(node, ast.Attribute) and isinstance(node.value, ast.Name)
                and node.value.id == "QVariant"):
            fix_qvariant_type.append(Offset(node.lineno, node.col_offset))
        elif (isinstance(node, ast.ImportFrom) and node.module.startswith("PyQt5.")):
            fix_pyqt_import.append(Offset(node.lineno, node.col_offset))

    if not fix_qvariant_type and not fix_pyqt_import and not fix_moved_classes:
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

        if token.offset in fix_moved_classes:
            # TODO We need to deal with case where we have removed all imported classes

            tokens.pop(i)
            while tokens[i].src == "," or tokens[i].name in ['UNIMPORTANT_WS', 'NEWLINE', 'NL']:
                tokens.pop(i)

            # TODO Ugly, do better
            if tokens[i - 1].src == '    ':
                tokens.pop(i - 1)

        if token.offset == last_import:

            new_tokens = [NEW_LINE] * 2 + import_tokens("QtCore", ["QT_VERSION"])
            new_tokens += [NEW_LINE] + src_to_tokens("if (QT_VERSION >= 0x060000):") + [NEW_LINE]
            for module, classes in new_import_qt6.items():
                new_tokens += [Token('INDENT', '    ')] + import_tokens(module, classes)
            new_tokens += [NEW_LINE] + src_to_tokens("else:") + [NEW_LINE]
            for module, classes in new_import_qt5.items():
                new_tokens += [Token('INDENT', '    ')] + import_tokens(module, classes)

            for tok in reversed(new_tokens):
                tokens.insert(i, tok)

    new_contents = tokens_to_src(tokens)
    with (open(filename, 'w') as f):
        f.write(new_contents)

    return new_contents != contents


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument('directory')
    # parser.add_argument('--exit-zero-even-if-changed', action='store_true')
    # parser.add_argument('--py35-plus', action='store_true')
    # parser.add_argument('--py36-plus', action='store_true')
    args = parser.parse_args(argv)

    print(os.path.join(args.directory, "*.py"))

    ret = 0
    for filename in glob.glob(os.path.join(args.directory, "*.py")):
        ret |= fix_file(filename, args)
    return ret


if __name__ == '__main__':
    raise SystemExit(main())
