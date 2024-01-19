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

Few useful commands:

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
import sys
from enum import Enum

from collections import defaultdict

from tokenize_rt import Offset, src_to_tokens, tokens_to_src, reversed_enumerate, Token

from typing import Sequence

from PyQt6 import QtCore, QtGui, QtWidgets, QtTest, QtSql, QtSvg, QtXml, QtNetwork, QtPrintSupport, Qsci
from PyQt6.QtCore import *  # noqa: F403
from PyQt6.QtGui import *  # noqa: F403
from PyQt6.QtWidgets import *  # noqa: F403
from PyQt6.QtTest import *  # noqa: F403
from PyQt6.QtSql import *  # noqa: F403
from PyQt6.QtXml import *  # noqa: F403
from PyQt6.QtNetwork import *  # noqa: F403
from PyQt6.QtPrintSupport import *  # noqa: F403
from PyQt6.Qsci import *  # noqa: F403

try:
    import qgis.core as qgis_core  # noqa: F403
    import qgis.gui as qgis_gui  # noqa: F403
    import qgis.analysis as qgis_analysis  # noqa: F403
    import qgis._3d as qgis_3d  # noqa: F403
    from qgis.core import *  # noqa: F403
    from qgis.gui import *  # noqa: F403
    from qgis.analysis import *  # noqa: F403
    from qgis._3d import *  # noqa: F403
except ImportError:
    qgis_core = None
    qgis_gui = None
    qgis_analysis = None
    qgis_3d = None
    print('QGIS classes not available for introspection, only a partial upgrade will be performed')

target_modules = [QtCore,
                  QtGui,
                  QtWidgets,
                  QtTest,
                  QtSql,
                  QtSvg,
                  QtXml,
                  QtNetwork,
                  QtPrintSupport,
                  Qsci]
if qgis_core is not None:
    target_modules.extend([
        qgis_core,
        qgis_gui,
        qgis_analysis,
        qgis_3d
    ]
    )

# qmetatype which have been renamed
qmetatype_mapping = {
    "Invalid": "UnknownType",
    "BitArray": "QBitArray",
    "Bitmap": "QBitmap",
    "Brush": "QBrush",
    "ByteArray": "QByteArray",
    "Char": "QChar",
    "Color": "QColor",
    "Cursor": "QCursor",
    "Date": "QDate",
    "DateTime": "QDateTime",
    "EasingCurve": "QEasingCurve",
    "Uuid": "QUuid",
    "ModelIndex": "QModelIndex",
    "PersistentModelIndex": "QPersistentModelIndex",
    "Font": "QFont",
    "Hash": "QVariantHash",
    "Icon": "QIcon",
    "Image": "QImage",
    "KeySequence": "QKeySequence",
    "Line": "QLine",
    "LineF": "QLineF",
    "List": "QVariantList",
    "Locale": "QLocale",
    "Map": "QVariantMap",
    "Transform": "QTransform",
    "Matrix4x4": "QMatrix4x4",
    "Palette": "QPalette",
    "Pen": "QPen",
    "Pixmap": "QPixmap",
    "Point": "QPoint",
    "PointF": "QPointF",
    "Polygon": "QPolygon",
    "PolygonF": "QPolygonF",
    "Quaternion": "QQuaternion",
    "Rect": "QRect",
    "RectF": "QRectF",
    "RegularExpression": "QRegularExpression",
    "Region": "QRegion",
    "Size": "QSize",
    "SizeF": "QSizeF",
    "SizePolicy": "QSizePolicy",
    "String": "QString",
    "StringList": "QStringList",
    "TextFormat": "QTextFormat",
    "TextLength": "QTextLength",
    "Time": "QTime",
    "Url": "QUrl",
    "Vector2D": "QVector2D",
    "Vector3D": "QVector3D",
    "Vector4D": "QVector4D",
    "UserType": "User",
}

deprecated_renamed_enums = {
    ('Qt', 'MidButton'): ('MouseButton', 'MiddleButton')
}

# { (class, enum_value) : enum_name }
qt_enums = {}
ambiguous_enums = defaultdict(set)


def fix_file(filename: str, qgis3_compat: bool) -> int:

    with open(filename, encoding='UTF-8') as f:
        contents = f.read()

    fix_qvariant_type = []  # QVariant.Int, QVariant.Double ...
    fix_pyqt_import = []  # from PyQt5.QtXXX
    fix_qt_enums = {}  # Unscoping of enums
    rename_qt_enums = []  # Renaming deprecated removed enums

    tree = ast.parse(contents, filename=filename)
    for parent in ast.walk(tree):

        for node in ast.iter_child_nodes(parent):
            if (not qgis3_compat and isinstance(node, ast.Attribute) and isinstance(node.value, ast.Name)
                    and node.value.id == "QVariant"):
                fix_qvariant_type.append(Offset(node.lineno, node.col_offset))

            if (isinstance(node, ast.Attribute) and isinstance(node.value, ast.Name)
                    and (node.value.id, node.attr) in ambiguous_enums):
                disambiguated = False
                try:
                    actual = eval(f'{node.value.id}.{node.attr}')
                    obj = globals()[node.value.id]
                    if isinstance(obj, type):
                        for attr_name in dir(obj):
                            attr = getattr(obj, attr_name)
                            if attr is actual.__class__:
                                # print(f'Found alias {node.value.id}.{attr_name}')
                                disambiguated = True
                                fix_qt_enums[
                                    Offset(node.lineno, node.col_offset)] = (
                                    node.value.id, attr_name, node.attr
                                )
                                break

                except AttributeError:
                    pass

                if not disambiguated:
                    possible_values = [f'{node.value.id}.{e}.{node.attr}' for e
                                       in ambiguous_enums[
                                           (node.value.id, node.attr)]]
                    sys.stderr.write(f'{filename}:{node.lineno}:{node.col_offset} WARNING: ambiguous enum, cannot fix: {node.value.id}.{node.attr}. Could be: {", ".join(possible_values)}\n')
            elif (isinstance(node, ast.Attribute) and isinstance(node.value, ast.Name)
                    and (node.value.id, node.attr) in qt_enums):
                fix_qt_enums[Offset(node.lineno, node.col_offset)] = (node.value.id, qt_enums[(node.value.id, node.attr)], node.attr)

            if (isinstance(node, ast.Attribute) and isinstance(node.value, ast.Name)
                    and (node.value.id, node.attr) in deprecated_renamed_enums):
                rename_qt_enums.append(Offset(node.lineno, node.col_offset))

            elif (isinstance(node, ast.ImportFrom) and node.module and node.module.startswith("PyQt5.")):
                fix_pyqt_import.append(Offset(node.lineno, node.col_offset))

    if not fix_qvariant_type and not fix_pyqt_import and not fix_qt_enums and not rename_qt_enums:
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
            _class, enum_name, value = fix_qt_enums[token.offset]
            # make sure we CAN import enum!
            try:
                eval(f'{_class}.{enum_name}.{value}')
                tokens[i + 2] = tokens[i + 2]._replace(
                    src=f"{enum_name}.{tokens[i + 2].src}")
            except AttributeError:
                # let's see if we can find what the replacement should be automatically...
                # print(f'Trying to find {_class}.{value}.')
                actual = eval(f'{_class}.{value}')
                # print(f'Trying to find aliases for {actual.__class__}.')
                obj = globals()[_class]
                recovered = False
                if isinstance(obj, type):
                    for attr_name in dir(obj):
                        try:
                            attr = getattr(obj, attr_name)
                            if attr is actual.__class__:
                                # print(f'Found alias {_class}.{attr_name}')
                                recovered = True
                                tokens[i + 2] = tokens[i + 2]._replace(
                                    src=f"{attr_name}.{tokens[i + 2].src}")

                        except AttributeError:
                            continue
                if not recovered:
                    sys.stderr.write(
                        f'{filename}:{token.line}:{token.utf8_byte_offset} ERROR: wanted to replace with {_class}.{enum_name}.{value}, but does not exist\n')
                continue

        if token.offset in rename_qt_enums:
            assert tokens[i + 1].src == "."
            enum_name = deprecated_renamed_enums[(tokens[i].src, tokens[i + 2].src)]
            assert enum_name
            tokens[i + 2] = tokens[i + 2]._replace(src=f"{enum_name[0]}.{enum_name[1]}")

    new_contents = tokens_to_src(tokens)
    with open(filename, 'w') as f:
        f.write(new_contents)

    return new_contents != contents


def get_class_enums(item):
    if not inspect.isclass(item):
        return

    for key, value in item.__dict__.items():
        if inspect.isclass(value) and type(value).__name__ == 'EnumType':
            for ekey, evalue in value.__dict__.items():
                if isinstance(evalue, value):
                    try:
                        test_value = getattr(item, str(ekey))
                        if not issubclass(type(test_value), Enum):
                            # There's a naming clash between an enum value (Eg QgsAggregateMappingModel.ColumnDataIndex.Aggregate)
                            # and a class (QgsAggregateMappingModel.Aggregate)
                            # So don't do any upgrades for these values, as current code will always be referring
                            # to the CLASS
                            continue
                    except AttributeError:
                        pass

                    if (item.__name__, ekey) in ambiguous_enums:
                        if value.__name__ not in ambiguous_enums[(item.__name__, ekey)]:
                            ambiguous_enums[(item.__name__, ekey)].add(value.__name__)
                        continue

                    existing_entry = qt_enums.get((item.__name__, ekey))
                    if existing_entry != value.__name__ and existing_entry:
                        ambiguous_enums[(item.__name__, ekey)].add(existing_entry)
                        ambiguous_enums[(item.__name__, ekey)].add(
                            value.__name__)
                        del qt_enums[(item.__name__, ekey)]
                    else:
                        qt_enums[(item.__name__, ekey)] = f"{value.__name__}"

        elif inspect.isclass(value):
            get_class_enums(value)


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument('directory')
    parser.add_argument('--qgis3-incompatible-changes', action='store_true',
                        help='Apply modifications that would break behavior on QGIS 3, hence code may not work on QGIS 3')
    args = parser.parse_args(argv)

    # get all scope for all qt enum
    for module in target_modules:
        for key, value in module.__dict__.items():
            get_class_enums(value)

    ret = 0
    for filename in glob.glob(os.path.join(args.directory, "**/*.py"), recursive=True):
        # print(f'Processing {filename}')
        if 'auto_additions' in filename:
            continue

        ret |= fix_file(filename, not args.qgis3_incompatible_changes)
    return ret


if __name__ == '__main__':
    raise SystemExit(main())
