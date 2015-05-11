# -*- coding: utf-8 -*-

"""
***************************************************************************
    FieldsMappingWidget.py
    ---------------------
    Date                 : October 2014
    Copyright            : (C) 2014 by Arnaud Morvan
    Email                : arnaud dot morvan at camptocamp dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Arnaud Morvan'
__date__ = 'October 2014'
__copyright__ = '(C) 2014, Arnaud Morvan'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'


from collections import OrderedDict

from PyQt4 import QtCore, QtGui

from qgis.core import QgsExpression
from qgis.gui import QgsFieldExpressionWidget

from processing.tools import dataobjects

from .ui_widgetFieldsMapping import Ui_Form


class FieldsMappingModel(QtCore.QAbstractTableModel):

    fieldTypes = OrderedDict([
        (QtCore.QVariant.Int, "Integer"),
        (QtCore.QVariant.Double, "Double"),
        (QtCore.QVariant.String, "String"),
        (QtCore.QVariant.DateTime, "Date"),
        (QtCore.QVariant.LongLong, "Double"),        
        (QtCore.QVariant.Date, "Date")])

    columns = [
        {'name': 'name', 'type': QtCore.QVariant.String},
        {'name': 'type', 'type': QtCore.QVariant.Type},
        {'name': 'length', 'type': QtCore.QVariant.Int},
        {'name': 'precision', 'type': QtCore.QVariant.Int},
        # {'name': 'comment', 'type': QtCore.QVariant.String},
        {'name': 'expression', 'type': QgsExpression}]

    def __init__(self, parent=None):
        super(FieldsMappingModel, self).__init__(parent)
        self._mapping = []
        self._errors = []
        self._layer = None

    def mapping(self):
        return self._mapping

    def setMapping(self, value):
        self.beginResetModel()
        self._mapping = value
        self.testAllExpressions()
        self.endResetModel()

    def testAllExpressions(self):
        self._errors = [None for i in xrange(0, len(self._mapping))]
        for row in xrange(0, len(self._mapping)):
            self.testExpression(row)

    def testExpression(self, row):
        self._errors[row] = None
        field = self._mapping[row]
        expression = QgsExpression(field['expression'])
        if expression.hasParserError():
            self._errors[row] = expression.parserErrorString()
            return
        if self._layer is None:
            return
        dp = self._layer.dataProvider()
        for feature in dp.getFeatures():
            expression.evaluate(feature, dp.fields())
            if expression.hasEvalError():
                self._errors[row] = expression.evalErrorString()
                return
            break

    def layer(self):
        return self._layer

    def setLayer(self, layer):
        self._layer = layer
        self.testAllExpressions()

    def columnCount(self, parent=QtCore.QModelIndex()):
        if parent.isValid():
            return 0
        return len(self.columns)

    def rowCount(self, parent=QtCore.QModelIndex()):
        if parent.isValid():
            return 0
        return self._mapping.__len__()

    def headerData(self, section, orientation, role=QtCore.Qt.DisplayRole):
        if role == QtCore.Qt.DisplayRole:
            if orientation == QtCore.Qt.Horizontal:
                return self.columns[section]['name'].title()
            if orientation == QtCore.Qt.Vertical:
                return section

    def flags(self, index):
        flags = (QtCore.Qt.ItemIsSelectable
                 + QtCore.Qt.ItemIsEditable
                 + QtCore.Qt.ItemIsEnabled)

        return QtCore.Qt.ItemFlags(flags)

    def data(self, index, role=QtCore.Qt.DisplayRole):
        column = index.column()

        if role == QtCore.Qt.DisplayRole:
            field = self._mapping[index.row()]
            column_def = self.columns[column]
            value = field[column_def['name']]

            fieldType = column_def['type']
            if fieldType == QtCore.QVariant.Type:
                if value == QtCore.QVariant.Invalid:
                    return ''
                return self.fieldTypes[value]
            return value

        if role == QtCore.Qt.EditRole:
            field = self._mapping[index.row()]
            column_def = self.columns[column]
            value = field[column_def['name']]
            return value

        if role == QtCore.Qt.TextAlignmentRole:
            fieldType = self.columns[column]['type']
            if fieldType in [QtCore.QVariant.Int]:
                hAlign = QtCore.Qt.AlignRight
            else:
                hAlign = QtCore.Qt.AlignLeft
            return hAlign + QtCore.Qt.AlignVCenter

        if role == QtCore.Qt.ForegroundRole:
            column_def = self.columns[column]
            if column_def['name'] == 'expression':
                brush = QtGui.QBrush()
                if self._errors[index.row()]:
                    brush.setColor(QtCore.Qt.red)
                else:
                    brush.setColor(QtCore.Qt.black)
                return brush

        if role == QtCore.Qt.ToolTipRole:
            column_def = self.columns[column]
            if column_def['name'] == 'expression':
                return self._errors[index.row()]

    def setData(self, index, value, role=QtCore.Qt.EditRole):
        if role == QtCore.Qt.EditRole:
            field = self._mapping[index.row()]
            column = index.column()
            column_def = self.columns[column]
            field[column_def['name']] = value
            if column_def['name'] == 'expression':
                self.testExpression(index.row())
            self.dataChanged.emit(index, index)
        return True

    def insertRows(self, row, count, index=QtCore.QModelIndex()):
        self.beginInsertRows(index, row, row + count - 1)

        for i in xrange(0, count):
            field = self.newField()
            self._mapping.insert(row + i, field)
            self._errors.insert(row + i, None)
            self.testExpression(row)

        self.endInsertRows()
        return True

    def removeRows(self, row, count, index=QtCore.QModelIndex()):
        self.beginRemoveRows(index, row, row + count - 1)

        for i in xrange(row + count - 1, row + 1):
            self._mapping.pop(i)
            self._errors.pop(i)

        self.endRemoveRows()
        return True

    def newField(self, field=None):
        if field is None:
            return {'name': '',
                    'type': QtCore.QVariant.Invalid,
                    'length': 0,
                    'precision': 0,
                    'expression': ''}

        return {'name': field.name(),
                'type': field.type(),
                'length': field.length(),
                'precision': field.precision(),
                'expression': field.name()}

    def loadLayerFields(self, layer):
        self.beginResetModel()

        self._mapping = []
        if layer is not None:
            dp = layer.dataProvider()
            for field in dp.fields():
                self._mapping.append(self.newField(field))
        self.testAllExpressions()

        self.endResetModel()


class FieldDelegate(QtGui.QStyledItemDelegate):

    def __init__(self, parent=None):
        super(FieldDelegate, self).__init__(parent)

    def createEditor(self, parent, option, index):
        column = index.column()

        fieldType = FieldsMappingModel.columns[column]['type']
        if fieldType == QtCore.QVariant.Type:
            editor = QtGui.QComboBox(parent)
            for key, text in FieldsMappingModel.fieldTypes.iteritems():
                editor.addItem(text, key)

        elif fieldType == QgsExpression:
            editor = QgsFieldExpressionWidget(parent)
            editor.setLayer(index.model().layer())
            editor.fieldChanged.connect(self.on_expression_fieldChange)

        else:
            editor = QtGui.QStyledItemDelegate.createEditor(self, parent, option, index)

        editor.setAutoFillBackground(True)
        return editor

    def setEditorData(self, editor, index):
        if not editor:
            return

        column = index.column()
        value = index.model().data(index, QtCore.Qt.EditRole)

        fieldType = FieldsMappingModel.columns[column]['type']
        if fieldType == QtCore.QVariant.Type:
            editor.setCurrentIndex(editor.findData(value))

        elif fieldType == QgsExpression:
            editor.setField(value)

        else:
            QtGui.QStyledItemDelegate.setEditorData(self, editor, index)

    def setModelData(self, editor, model, index):
        if not editor:
            return

        column = index.column()

        fieldType = FieldsMappingModel.columns[column]['type']
        if fieldType == QtCore.QVariant.Type:
            value = editor.itemData(editor.currentIndex())
            if value is None:
                value = QtCore.QVariant.Invalid
            model.setData(index, value)

        elif fieldType == QgsExpression:
            (value, isExpression, isValid) = editor.currentField()
            model.setData(index, value)

        else:
            QtGui.QStyledItemDelegate.setModelData(self, editor, model, index)

    def updateEditorGeometry(self, editor, option, index):
        editor.setGeometry(option.rect)

    def on_expression_fieldChange(self, fieldName):
        self.commitData.emit(self.sender())


class FieldsMappingPanel(QtGui.QWidget, Ui_Form):

    def __init__(self, parent=None):
        QtGui.QWidget.__init__(self, parent)
        self.setupUi(self)

        self.addButton.setIcon(
            QtGui.QIcon(':/images/themes/default/mActionAdd.svg'))
        self.deleteButton.setIcon(
            QtGui.QIcon(':/images/themes/default/mActionRemove.svg'))
        self.upButton.setIcon(
            QtGui.QIcon(':/images/themes/default/mActionArrowUp.png'))
        self.downButton.setIcon(
            QtGui.QIcon(':/images/themes/default/mActionArrowDown.png'))
        self.resetButton.setIcon(
            QtGui.QIcon(':/images/themes/default/mIconClear.png'))

        self.model = FieldsMappingModel()
        self.fieldsView.setModel(self.model)

        self.model.rowsInserted.connect(self.on_model_rowsInserted)
        self.fieldsView.setItemDelegate(FieldDelegate())

        self.updateLayerCombo()

    def setLayer(self, layer):
        self.model.setLayer(layer)
        if self.model.rowCount() == 0:
            self.on_resetButton_clicked()
        else:
            dlg = QtGui.QMessageBox(self)
            dlg.setText("Do you want to reset the field mapping?")
            dlg.setStandardButtons(
                QtGui.QMessageBox.StandardButtons(QtGui.QMessageBox.Yes
                                                  + QtGui.QMessageBox.No))
            dlg.setDefaultButton(QtGui.QMessageBox.No)
            if dlg.exec_() == QtGui.QMessageBox.Yes:
                self.on_resetButton_clicked()

    def value(self):
        return self.model.mapping()

    def setValue(self, value):
        self.model.setMapping(value)

    @QtCore.pyqtSlot(bool, name='on_addButton_clicked')
    def on_addButton_clicked(self, checked=False):
        rowCount = self.model.rowCount()
        self.model.insertRows(rowCount, 1)
        index = self.model.index(rowCount, 0)
        self.fieldsView.selectionModel().select(index,
            QtGui.QItemSelectionModel.SelectionFlags(QtGui.QItemSelectionModel.Clear
                                                     + QtGui.QItemSelectionModel.Select
                                                     + QtGui.QItemSelectionModel.Current
                                                     + QtGui.QItemSelectionModel.Rows))
        self.fieldsView.scrollTo(index)
        self.fieldsView.scrollTo(index)

    @QtCore.pyqtSlot(bool, name='on_deleteButton_clicked')
    def on_deleteButton_clicked(self, checked=False):
        sel = self.fieldsView.selectionModel()
        if not sel.hasSelection():
            return

        indexes = sel.selectedRows()
        for index in indexes:
            self.model.removeRows(index.row(), 1)

    @QtCore.pyqtSlot(bool, name='on_upButton_clicked')
    def on_upButton_clicked(self, checked=False):
        sel = self.fieldsView.selectionModel()
        if not sel.hasSelection():
            return

        row = sel.selectedRows()[0].row()
        if row == 0:
            return

        self.model.insertRows(row - 1, 1)

        for column in xrange(0, self.model.columnCount()):
            srcIndex = self.model.index(row + 1, column)
            dstIndex = self.model.index(row - 1, column)
            value = self.model.data(srcIndex, QtCore.Qt.EditRole)
            self.model.setData(dstIndex, value, QtCore.Qt.EditRole)

        self.model.removeRows(row + 1, 1)

        sel.select(self.model.index(row - 1, 0),
                   QtGui.QItemSelectionModel.SelectionFlags(QtGui.QItemSelectionModel.Clear
                                                            + QtGui.QItemSelectionModel.Select
                                                            + QtGui.QItemSelectionModel.Current
                                                            + QtGui.QItemSelectionModel.Rows))

    @QtCore.pyqtSlot(bool, name='on_downButton_clicked')
    def on_downButton_clicked(self, checked=False):
        sel = self.fieldsView.selectionModel()
        if not sel.hasSelection():
            return

        row = sel.selectedRows()[0].row()
        if row == self.model.rowCount() - 1:
            return

        self.model.insertRows(row + 2, 1)

        for column in xrange(0, self.model.columnCount()):
            srcIndex = self.model.index(row, column)
            dstIndex = self.model.index(row + 2, column)
            value = self.model.data(srcIndex, QtCore.Qt.EditRole)
            self.model.setData(dstIndex, value, QtCore.Qt.EditRole)

        self.model.removeRows(row, 1)

        sel.select(self.model.index(row + 1, 0),
                   QtGui.QItemSelectionModel.SelectionFlags(QtGui.QItemSelectionModel.Clear
                                                            + QtGui.QItemSelectionModel.Select
                                                            + QtGui.QItemSelectionModel.Current
                                                            + QtGui.QItemSelectionModel.Rows))

    @QtCore.pyqtSlot(bool, name='on_resetButton_clicked')
    def on_resetButton_clicked(self, checked=False):
        self.model.loadLayerFields(self.model.layer())
        self.openPersistentEditor(
            self.model.index(0, 0),
            self.model.index(self.model.rowCount() - 1,
                             self.model.columnCount() - 1))
        self.resizeColumns()

    def resizeColumns(self):
        header = self.fieldsView.horizontalHeader()
        header.resizeSections(QtGui.QHeaderView.ResizeToContents)
        for section in xrange(0, header.count()):
            size = header.sectionSize(section)
            fieldType = FieldsMappingModel.columns[section]['type']
            if fieldType == QgsExpression:
                header.resizeSection(section, size + 100)
            else:
                header.resizeSection(section, size + 20)

    def openPersistentEditor(self, topLeft, bottomRight):
        return
        for row in xrange(topLeft.row(), bottomRight.row() + 1):
            for column in xrange(topLeft.column(), bottomRight.column() + 1):
                self.fieldsView.openPersistentEditor(self.model.index(row, column))
                editor = self.fieldsView.indexWidget(self.model.index(row, column))
                if isinstance(editor, QtGui.QLineEdit):
                    editor.deselect()
                if isinstance(editor, QtGui.QSpinBox):
                    lineEdit = editor.findChild(QtGui.QLineEdit)
                    lineEdit.setAlignment(QtCore.Qt.AlignRight or QtCore.Qt.AlignVCenter)
                    lineEdit.deselect()

    def on_model_rowsInserted(self, parent, start, end):
        self.openPersistentEditor(
            self.model.index(start, 0),
            self.model.index(end, self.model.columnCount() - 1))

    def updateLayerCombo(self):
        layers = dataobjects.getVectorLayers()
        layers.sort(key=lambda lay: lay.name())
        for layer in layers:
            self.layerCombo.addItem(layer.name(), layer)

    @QtCore.pyqtSlot(bool, name='on_loadLayerFieldsButton_clicked')
    def on_loadLayerFieldsButton_clicked(self, checked=False):
        layer = self.layerCombo.itemData(self.layerCombo.currentIndex())
        if layer is None:
            return
        self.model.loadLayerFields(layer)
