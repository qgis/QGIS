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
    ('Qt', 'MidButton'): ('MouseButton', 'MiddleButton'),
    ('Qt', 'TextColorRole'): ('ItemDataRole', 'ForegroundRole'),
    ('Qt', 'BackgroundColorRole'): ('ItemDataRole', 'BackgroundRole'),
}

rename_function_attributes = {
    'exec_': 'exec'
}

rename_function_definitions = {
    'exec_': 'exec'
}

import_warnings = {
    'QRegExp': 'QRegExp is removed in Qt6, please use QRegularExpression for Qt5/Qt6 compatibility'
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
    member_renames = {}
    token_renames = {}
    function_def_renames = {}
    rename_qt_enums = []  # Renaming deprecated removed enums
    custom_updates = {}
    imported_modules = set()
    extra_imports = defaultdict(set)
    removed_imports = defaultdict(set)
    import_offsets = {}

    def visit_call(_node: ast.Call, _parent):
        if isinstance(_node.func, ast.Attribute):
            if _node.func.attr in rename_function_attributes:
                attr_node = _node.func
                member_renames[
                    Offset(_node.func.lineno, attr_node.end_col_offset - len(
                        _node.func.attr) - 1)] = rename_function_attributes[
                    _node.func.attr]
            if _node.func.attr == 'addAction':
                if len(_node.args) >= 4:
                    sys.stderr.write(
                        f'{filename}:{_node.lineno}:{_node.col_offset} WARNING: fragile call to addAction. Use my_action = QAction(...), obj.addAction(my_action) instead.\n')
            if _node.func.attr == 'desktop':
                if len(_node.args) == 0:
                    sys.stderr.write(
                        f'{filename}:{_node.lineno}:{_node.col_offset} WARNING: QDesktopWidget is deprecated and removed in Qt6. Replace with alternative approach instead.\n')

        if isinstance(_node.func, ast.Name) and _node.func.id == 'QVariant':
            if not _node.args:
                extra_imports['qgis.core'].update({'NULL'})

                def _invalid_qvariant_to_null(start_index: int, tokens):
                    assert tokens[start_index].src == 'QVariant'
                    assert tokens[start_index + 1].src == '('
                    assert tokens[start_index + 2].src == ')'

                    tokens[start_index] = tokens[start_index]._replace(src='NULL')
                    for i in range(start_index + 1, start_index + 3):
                        tokens[i] = tokens[i]._replace(src='')

                custom_updates[Offset(_node.lineno,
                                      _node.col_offset)] = _invalid_qvariant_to_null
            elif len(_node.args) == 1 and isinstance(_node.args[0], ast.Attribute) and isinstance(_node.args[0].value, ast.Name) and _node.args[0].value.id == 'QVariant':
                extra_imports['qgis.core'].update({'NULL'})

                def _fix_null_qvariant(start_index: int, tokens):
                    assert tokens[start_index].src == 'QVariant'
                    assert tokens[start_index + 1].src == '('
                    assert tokens[start_index + 2].src == 'QVariant'
                    assert tokens[start_index + 3].src == '.'
                    assert tokens[start_index + 5].src == ')'

                    tokens[start_index] = tokens[start_index]._replace(src='NULL')
                    for i in range(start_index + 1, start_index + 6):
                        tokens[i] = tokens[i]._replace(src='')

                custom_updates[Offset(_node.lineno,
                                      _node.col_offset)] = _fix_null_qvariant
        elif isinstance(_node.func, ast.Name) and _node.func.id == 'QDateTime':
            if len(_node.args) == 8:
                # QDateTime(yyyy, mm, dd, hh, MM, ss, ms, ts) doesn't work anymore,
                # so port to more reliable QDateTime(QDate, QTime, ts) form

                extra_imports['qgis.PyQt.QtCore'].update({'QDate', 'QTime'})

                def _fix_qdatetime_construct(start_index: int, tokens):
                    i = start_index + 1
                    assert tokens[i].src == '('
                    tokens[i] = tokens[i]._replace(src='(QDate(')
                    while tokens[i].offset < Offset(_node.args[2].lineno, _node.args[2].col_offset):
                        i += 1
                    assert tokens[i + 1].src == ','
                    i += 1
                    tokens[i] = tokens[i]._replace(src='), QTime(')
                    i += 1
                    while not tokens[i].src.strip():
                        tokens[i] = tokens[i]._replace(src='')
                        i += 1
                    while tokens[i].offset < Offset(_node.args[6].lineno, _node.args[6].col_offset):
                        i += 1
                    i += 1
                    assert tokens[i].src == ','
                    tokens[i] = tokens[i]._replace(src='),')

                custom_updates[Offset(_node.lineno, _node.col_offset)] = _fix_qdatetime_construct
            elif len(_node.args) == 1 and isinstance(_node.args[0], ast.Call) and _node.args[0].func.id == 'QDate':
                # QDateTime(QDate(..)) doesn't work anymore,
                # so port to more reliable QDateTime(QDate(...), QTime(0,0,0)) form
                extra_imports['qgis.PyQt.QtCore'].update({'QTime'})

                def _fix_qdatetime_construct(start_index: int, tokens):
                    assert tokens[start_index].src == 'QDateTime'
                    assert tokens[start_index + 1].src == '('
                    assert tokens[start_index + 2].src == 'QDate'
                    assert tokens[start_index + 3].src == '('
                    i = start_index + 4
                    while tokens[i].offset < Offset(_node.args[0].end_lineno,
                                                    _node.args[0].end_col_offset):
                        i += 1

                    assert tokens[i - 1].src == ')'
                    tokens[i - 1] = tokens[i - 1]._replace(src='), QTime(0, 0, 0)')

                custom_updates[Offset(_node.lineno,
                                      _node.col_offset)] = _fix_qdatetime_construct

    def visit_attribute(_node: ast.Attribute, _parent):
        if isinstance(_node.value, ast.Name):
            if _node.value.id == 'qApp':
                token_renames[Offset(_node.value.lineno, _node.value.col_offset)] = 'QApplication.instance()'
                extra_imports['qgis.PyQt.QtWidgets'].update({'QApplication'})
                removed_imports['qgis.PyQt.QtWidgets'].update({'qApp'})
            if _node.value.id == 'QVariant' and _node.attr == 'Type':
                def _replace_qvariant_type(start_index: int, tokens):
                    # QVariant.Type.XXX doesn't exist, it should be QVariant.XXX
                    assert tokens[start_index].src == 'QVariant'
                    assert tokens[start_index + 1].src == '.'
                    assert tokens[start_index + 2].src == 'Type'
                    assert tokens[start_index + 3].src == '.'

                    tokens[start_index + 2] = tokens[start_index + 2]._replace(src='')
                    tokens[start_index + 3] = tokens[start_index + 3]._replace(
                        src='')

                custom_updates[Offset(node.lineno, node.col_offset)] = _replace_qvariant_type
        elif isinstance(_node.value, ast.Call):
            if (isinstance(_node.value.func, ast.Attribute) and
                _node.value.func.attr == 'fontMetrics' and
                    _node.attr == 'width'):
                sys.stderr.write(
                    f'{filename}:{_node.lineno}:{_node.col_offset} WARNING: QFontMetrics.width() '
                    'has been removed in Qt6. Use QFontMetrics.horizontalAdvance() if plugin can '
                    'safely require Qt >= 5.11, or QFontMetrics.boundingRect().width() otherwise.\n')

    def visit_subscript(_node: ast.Subscript, _parent):
        if isinstance(_node.value, ast.Attribute):
            if (_node.value.attr == 'activated' and
                isinstance(_node.slice, ast.Name) and
                    _node.slice.id == 'str'):
                sys.stderr.write(
                    f'{filename}:{_node.lineno}:{_node.col_offset} WARNING: activated[str] '
                    'has been removed in Qt6. Consider using QComboBox.activated instead if the string is not required, '
                    'or QComboBox.textActivated if the plugin can '
                    'safely require Qt >= 5.14. Otherwise conditional Qt version code will need to be introduced.\n')

    def visit_import(_node: ast.ImportFrom, _parent):
        import_offsets[Offset(node.lineno, node.col_offset)] = (
            node.module, set(name.name for name in node.names), node.end_lineno,
            node.end_col_offset)
        imported_modules.add(node.module)
        for name in node.names:
            if name.name in import_warnings:
                print(f'{filename}: {import_warnings[name.name]}')
            if name.name == 'resources_rc':
                sys.stderr.write(
                    f'{filename}:{_node.lineno}:{_node.col_offset} WARNING: support for compiled resources '
                    'is removed in Qt6. Directly load icon resources by file path and load UI fields using '
                    'uic.loadUiType by file path instead.\n')
        if _node.module == 'qgis.PyQt.Qt':
            extra_imports['qgis.PyQt.QtCore'].update({'Qt'})
            removed_imports['qgis.PyQt.Qt'].update({'Qt'})

    tree = ast.parse(contents, filename=filename)
    for parent in ast.walk(tree):
        for node in ast.iter_child_nodes(parent):
            if isinstance(node, ast.ImportFrom):
                visit_import(node, parent)

            if (not qgis3_compat and isinstance(node, ast.Attribute) and isinstance(node.value, ast.Name)
                    and node.value.id == "QVariant"):
                fix_qvariant_type.append(Offset(node.lineno, node.col_offset))

            if isinstance(node, ast.Call):
                visit_call(node, parent)
            elif isinstance(node, ast.Attribute):
                visit_attribute(node, parent)
            elif isinstance(node, ast.Subscript):
                visit_subscript(node, parent)

            if isinstance(node, ast.FunctionDef) and node.name in rename_function_definitions:
                function_def_renames[
                    Offset(node.lineno, node.col_offset)] = rename_function_definitions[node.name]

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
            elif (isinstance(node, ast.Attribute) and isinstance(node.value, ast.Name) and not isinstance(parent, ast.Attribute)
                    and (node.value.id, node.attr) in qt_enums):
                fix_qt_enums[Offset(node.lineno, node.col_offset)] = (node.value.id, qt_enums[(node.value.id, node.attr)], node.attr)

            if (isinstance(node, ast.Attribute) and isinstance(node.value, ast.Name)
                    and (node.value.id, node.attr) in deprecated_renamed_enums):
                rename_qt_enums.append(Offset(node.lineno, node.col_offset))

            elif (isinstance(node, ast.ImportFrom) and node.module and node.module.startswith("PyQt5.")):
                fix_pyqt_import.append(Offset(node.lineno, node.col_offset))

    for module, classes in extra_imports.items():
        if module not in imported_modules:
            class_import = ', '.join(classes)
            import_statement = f'from {module} import {class_import}'
            print(f'{filename}: Missing import, manually add \n\t{import_statement}')

    if not any([fix_qvariant_type,
               fix_pyqt_import,
               fix_qt_enums,
               rename_qt_enums,
               member_renames,
               function_def_renames,
               custom_updates,
               extra_imports,
               removed_imports,
               token_renames]):
        return 0

    tokens = src_to_tokens(contents)
    for i, token in reversed_enumerate(tokens):
        if token.offset in import_offsets:
            end_import_offset = Offset(*import_offsets[token.offset][-2:])
            assert tokens[i].src == 'from'
            token_index = i + 1
            while not tokens[token_index].src.strip():
                token_index += 1

            module = ''
            while tokens[token_index].src.strip():
                module += tokens[token_index].src
                token_index += 1

            if extra_imports.get(module) or removed_imports.get(module):
                current_imports = set()
                while True:
                    token_index += 1
                    if tokens[token_index].offset == end_import_offset:
                        break
                    if tokens[token_index].src.strip() in ('', ',', 'import', '(', ')'):
                        continue

                    import_ = tokens[token_index].src
                    if import_ in removed_imports.get(module, set()):
                        tokens[token_index] = tokens[token_index]._replace(src='')
                        prev_token_index = token_index - 1
                        while True:
                            if tokens[prev_token_index].src.strip() in ('', ','):
                                tokens[prev_token_index] = tokens[
                                    prev_token_index]._replace(src='')
                                prev_token_index -= 1
                            else:
                                break

                        none_forward = True
                        current_index = prev_token_index + 1
                        while True:
                            if tokens[current_index].src in ('\n', ')'):
                                break
                            elif tokens[current_index].src.strip():
                                none_forward = False
                                break
                            current_index += 1

                        none_backward = True
                        current_index = prev_token_index
                        while True:
                            if tokens[current_index].src in ('import',):
                                break
                            elif tokens[current_index].src.strip():
                                none_backward = False
                                break
                            current_index -= 1
                        if none_backward and none_forward:
                            # no more imports from this module, remove whole import
                            while True:
                                if tokens[current_index].src in ('from',):
                                    break
                                current_index -= 1

                            while True:
                                if tokens[current_index].src in ('\n',):
                                    tokens[current_index] = tokens[
                                        current_index]._replace(src='')
                                    break
                                tokens[current_index] = tokens[current_index]._replace(src='')
                                current_index += 1

                    else:
                        current_imports.add(import_)

                imports_to_add = extra_imports.get(module, set()) - current_imports
                if imports_to_add:
                    additional_import_string = ', '.join(imports_to_add)
                    if tokens[token_index - 1].src == ')':
                        token_index -= 1
                        while tokens[token_index].src.strip() in ('', ',', ')'):
                            tokens[token_index] = tokens[token_index]._replace(
                                src='')
                            token_index -= 1
                        tokens[token_index + 1] = tokens[token_index + 1]._replace(src=f", {additional_import_string})")
                    else:
                        tokens[token_index] = tokens[token_index]._replace(src=f", {additional_import_string}{tokens[token_index].src}")

        if token.offset in fix_qvariant_type:
            assert tokens[i].src == "QVariant"
            assert tokens[i + 1].src == "."
            tokens[i] = tokens[i]._replace(src="QMetaType.Type")
            attr = tokens[i + 2].src
            if attr in qmetatype_mapping:
                tokens[i + 2] = tokens[i + 2]._replace(src=qmetatype_mapping[attr])

        if token.offset in custom_updates:
            custom_updates[token.offset](i, tokens)

        if token.offset in fix_pyqt_import:
            assert tokens[i + 2].src == "PyQt5"
            tokens[i + 2] = tokens[i + 2]._replace(src="qgis.PyQt")

        if token.offset in function_def_renames and tokens[i].src == "def":
            tokens[i + 2] = tokens[i + 2]._replace(src=function_def_renames[token.offset])

        if token.offset in token_renames:
            tokens[i] = tokens[i]._replace(src=token_renames[token.offset])

        if token.offset in member_renames:
            counter = i
            while tokens[counter].src != '.':
                counter += 1
            tokens[counter + 1] = tokens[counter + 1]._replace(src=member_renames[token.offset])

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

    # enums might be referenced using a subclass instead of their
    # parent class, so we need to loop through all those too...
    def all_subclasses(cls):
        if cls is object:
            return set()
        return {cls}.union(
            s for c in cls.__subclasses__() for s in all_subclasses(c))
    matched_classes = {item}.union(all_subclasses(item))

    for key, value in item.__dict__.items():
        if inspect.isclass(value) and type(value).__name__ == 'EnumType':
            for ekey, evalue in value.__dict__.items():
                for matched_class in matched_classes:
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

                        if (matched_class.__name__, ekey) in ambiguous_enums:
                            if value.__name__ not in ambiguous_enums[(matched_class.__name__, ekey)]:
                                ambiguous_enums[(matched_class.__name__, ekey)].add(value.__name__)
                            continue

                        existing_entry = qt_enums.get((matched_class.__name__, ekey))
                        if existing_entry != value.__name__ and existing_entry:
                            ambiguous_enums[(matched_class.__name__, ekey)].add(existing_entry)
                            ambiguous_enums[(matched_class.__name__, ekey)].add(
                                value.__name__)
                            del qt_enums[(matched_class.__name__, ekey)]
                        else:
                            qt_enums[(matched_class.__name__, ekey)] = f"{value.__name__}"

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
