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

import os
from collections import OrderedDict

from qgis.PyQt import uic
from qgis.PyQt.QtCore import (
    QItemSelectionModel,
    QAbstractTableModel,
    QModelIndex,
    QVariant,
    Qt,
    pyqtSlot,
    QCoreApplication
)
from qgis.PyQt.QtWidgets import (
    QComboBox,
    QHeaderView,
    QLineEdit,
    QSpacerItem,
    QMessageBox,
    QSpinBox,
    QStyledItemDelegate,
    QWidget,
    QVBoxLayout
)

from qgis.core import (
    QgsApplication,
    QgsExpression,
    QgsMapLayerProxyModel,
    QgsProcessingFeatureSourceDefinition,
    QgsProcessingUtils,
    QgsProject,
    QgsVectorLayer,
)
from qgis.gui import QgsFieldExpressionWidget

from processing.gui.wrappers import WidgetWrapper, DIALOG_STANDARD, DIALOG_MODELER, DIALOG_BATCH
from processing.tools import dataobjects
from processing.algs.qgis.FieldsMapper import FieldsMapper


pluginPath = os.path.dirname(__file__)
WIDGET, BASE = uic.loadUiType(
    os.path.join(pluginPath, 'fieldsmappingpanelbase.ui'))


class FieldsMappingModel(QAbstractTableModel):

    fieldTypes = OrderedDict([
        (QVariant.Date, "Date"),
        (QVariant.DateTime, "DateTime"),
        (QVariant.Double, "Double"),
        (QVariant.Int, "Integer"),
        (QVariant.LongLong, "Integer64"),
        (QVariant.String, "String"),
        (QVariant.Bool, "Boolean")])

    def __init__(self, parent=None):
        super(FieldsMappingModel, self).__init__(parent)
        self._mapping = []
        self._layer = None
        self.configure()

    def configure(self):
        self.columns = [{
            'name': 'expression',
            'type': QgsExpression,
            'header': self.tr("Source expression"),
            'persistentEditor': True
        }, {
            'name': 'name',
            'type': QVariant.String,
            'header': self.tr("Field name")
        }, {
            'name': 'type',
            'type': QVariant.Type,
            'header': self.tr("Type"),
            'persistentEditor': True
        }, {
            'name': 'length',
            'type': QVariant.Int,
            'header': self.tr("Length")
        }, {
            'name': 'precision',
            'type': QVariant.Int,
            'header': self.tr("Precision")
        }]

    def columnIndex(self, column_name):
        for index, column in enumerate(self.columns):
            if column['name'] == column_name:
                return index

    def mapping(self):
        return self._mapping

    def setMapping(self, value):
        self.beginResetModel()
        self._mapping = value
        self.endResetModel()

    def contextGenerator(self):
        if self._layer:
            return self._layer
        return QgsProject.instance()

    def layer(self):
        return self._layer

    def setLayer(self, layer):
        self._layer = layer

    def columnCount(self, parent=QModelIndex()):
        if parent.isValid():
            return 0
        return len(self.columns)

    def rowCount(self, parent=QModelIndex()):
        if parent.isValid():
            return 0
        try:
            return len(self._mapping)
        except TypeError:
            return 0

    def headerData(self, section, orientation, role=Qt.DisplayRole):
        if role == Qt.DisplayRole:
            if orientation == Qt.Horizontal:
                return self.columns[section]['header']
            if orientation == Qt.Vertical:
                return section

    def flags(self, index):
        return Qt.ItemFlags(Qt.ItemIsSelectable |
                            Qt.ItemIsEditable |
                            Qt.ItemIsEnabled)

    def data(self, index, role=Qt.DisplayRole):
        field = self._mapping[index.row()]
        column_def = self.columns[index.column()]

        if role == Qt.DisplayRole:
            value = field[column_def['name']]
            if column_def['type'] == QVariant.Type:
                if value == QVariant.Invalid:
                    return ''
                return self.fieldTypes[value]
            return value

        if role == Qt.EditRole:
            return field[column_def['name']]

        if role == Qt.TextAlignmentRole:
            if column_def['type'] in [QVariant.Int]:
                hAlign = Qt.AlignRight
            else:
                hAlign = Qt.AlignLeft
            return hAlign + Qt.AlignVCenter

    def setData(self, index, value, role=Qt.EditRole):
        field = self._mapping[index.row()]
        column_def = self.columns[index.column()]

        if role == Qt.EditRole:
            field[column_def['name']] = value
            self.dataChanged.emit(index, index)

        return True

    def insertRows(self, row, count, index=QModelIndex()):
        self.beginInsertRows(index, row, row + count - 1)

        for i in range(count):
            field = self.newField()
            self._mapping.insert(row + i, field)

        self.endInsertRows()
        return True

    def removeRows(self, row, count, index=QModelIndex()):
        self.beginRemoveRows(index, row, row + count - 1)

        for i in range(row + count - 1, row + 1):
            self._mapping.pop(i)

        self.endRemoveRows()
        return True

    def newField(self, field=None):
        if field is None:
            return {'name': '',
                    'type': QVariant.Invalid,
                    'length': 0,
                    'precision': 0,
                    'expression': ''}

        return {'name': field.name(),
                'type': field.type(),
                'length': field.length(),
                'precision': field.precision(),
                'expression': QgsExpression.quotedColumnRef(field.name())}

    def loadLayerFields(self, layer):
        self.beginResetModel()

        self._mapping = []
        if layer is not None:
            for field in layer.fields():
                self._mapping.append(self.newField(field))

        self.endResetModel()


class FieldTypeDelegate(QStyledItemDelegate):

    def createEditor(self, parent, option, index):
        editor = QComboBox(parent)
        for key, text in FieldsMappingModel.fieldTypes.items():
            editor.addItem(text, key)
        return editor

    def setEditorData(self, editor, index):
        if not editor:
            return
        value = index.model().data(index, Qt.EditRole)
        editor.setCurrentIndex(editor.findData(value))

    def setModelData(self, editor, model, index):
        if not editor:
            return
        value = editor.currentData()
        if value is None:
            value = QVariant.Invalid
        model.setData(index, value)


class ExpressionDelegate(QStyledItemDelegate):

    def createEditor(self, parent, option, index):
        editor = QgsFieldExpressionWidget(parent)
        editor.setLayer(index.model().layer())
        editor.registerExpressionContextGenerator(index.model().contextGenerator())
        editor.fieldChanged.connect(self.on_expression_fieldChange)
        editor.setAutoFillBackground(True)
        editor.setAllowEvalErrors(self.parent().dialogType == DIALOG_MODELER)
        return editor

    def setEditorData(self, editor, index):
        if not editor:
            return
        value = index.model().data(index, Qt.EditRole)
        editor.setField(value)

    def setModelData(self, editor, model, index):
        if not editor:
            return
        (value, isExpression, isValid) = editor.currentField()
        if isExpression is True:
            model.setData(index, value)
        else:
            model.setData(index, QgsExpression.quotedColumnRef(value))

    def on_expression_fieldChange(self, fieldName):
        self.commitData.emit(self.sender())


class FieldsMappingPanel(BASE, WIDGET):

    def __init__(self, parent=None):
        super(FieldsMappingPanel, self).__init__(parent)
        self.setupUi(self)

        self.addButton.setIcon(QgsApplication.getThemeIcon("/mActionNewAttribute.svg"))
        self.deleteButton.setIcon(QgsApplication.getThemeIcon('/mActionDeleteAttribute.svg'))
        self.upButton.setIcon(QgsApplication.getThemeIcon('/mActionArrowUp.svg'))
        self.downButton.setIcon(QgsApplication.getThemeIcon('/mActionArrowDown.svg'))
        self.resetButton.setIcon(QgsApplication.getThemeIcon('/mIconClearText.svg'))

        self.configure()

        self.model.modelReset.connect(self.on_model_modelReset)
        self.model.rowsInserted.connect(self.on_model_rowsInserted)

        self.layerCombo.setAllowEmptyLayer(True)
        self.layerCombo.setFilters(QgsMapLayerProxyModel.VectorLayer)
        self.dialogType = None

    def configure(self):
        self.model = FieldsMappingModel()
        self.fieldsView.setModel(self.model)

        self.setDelegate('expression', ExpressionDelegate(self))
        self.setDelegate('type', FieldTypeDelegate(self))

    def setDelegate(self, column_name, delegate):
        self.fieldsView.setItemDelegateForColumn(
            self.model.columnIndex(column_name),
            delegate)

    def setLayer(self, layer):
        if self.model.layer() == layer:
            return
        self.model.setLayer(layer)
        if layer is None:
            return
        if self.model.rowCount() == 0:
            self.on_resetButton_clicked()
            return
        dlg = QMessageBox(self)
        dlg.setText(self.tr("Do you want to reset the field mapping?"))
        dlg.setStandardButtons(
            QMessageBox.StandardButtons(QMessageBox.Yes |
                                        QMessageBox.No))
        dlg.setDefaultButton(QMessageBox.No)
        if dlg.exec_() == QMessageBox.Yes:
            self.on_resetButton_clicked()

    def value(self):
        return self.model.mapping()

    def setValue(self, value):
        self.model.setMapping(value)

    @pyqtSlot(bool, name='on_addButton_clicked')
    def on_addButton_clicked(self, checked=False):
        rowCount = self.model.rowCount()
        self.model.insertRows(rowCount, 1)
        index = self.model.index(rowCount, 0)
        self.fieldsView.selectionModel().select(
            index,
            QItemSelectionModel.SelectionFlags(
                QItemSelectionModel.Clear |
                QItemSelectionModel.Select |
                QItemSelectionModel.Current |
                QItemSelectionModel.Rows))
        self.fieldsView.scrollTo(index)
        self.fieldsView.scrollTo(index)

    @pyqtSlot(bool, name='on_deleteButton_clicked')
    def on_deleteButton_clicked(self, checked=False):
        sel = self.fieldsView.selectionModel()
        if not sel.hasSelection():
            return

        indexes = sel.selectedRows()
        for index in indexes:
            self.model.removeRows(index.row(), 1)

    @pyqtSlot(bool, name='on_upButton_clicked')
    def on_upButton_clicked(self, checked=False):
        sel = self.fieldsView.selectionModel()
        if not sel.hasSelection():
            return

        row = sel.selectedRows()[0].row()
        if row == 0:
            return

        self.model.insertRows(row - 1, 1)

        for column in range(self.model.columnCount()):
            srcIndex = self.model.index(row + 1, column)
            dstIndex = self.model.index(row - 1, column)
            value = self.model.data(srcIndex, Qt.EditRole)
            self.model.setData(dstIndex, value, Qt.EditRole)

        self.model.removeRows(row + 1, 1)

        sel.select(
            self.model.index(row - 1, 0),
            QItemSelectionModel.SelectionFlags(
                QItemSelectionModel.Clear |
                QItemSelectionModel.Select |
                QItemSelectionModel.Current |
                QItemSelectionModel.Rows))

    @pyqtSlot(bool, name='on_downButton_clicked')
    def on_downButton_clicked(self, checked=False):
        sel = self.fieldsView.selectionModel()
        if not sel.hasSelection():
            return

        row = sel.selectedRows()[0].row()
        if row == self.model.rowCount() - 1:
            return

        self.model.insertRows(row + 2, 1)

        for column in range(self.model.columnCount()):
            srcIndex = self.model.index(row, column)
            dstIndex = self.model.index(row + 2, column)
            value = self.model.data(srcIndex, Qt.EditRole)
            self.model.setData(dstIndex, value, Qt.EditRole)

        self.model.removeRows(row, 1)

        sel.select(
            self.model.index(row + 1, 0),
            QItemSelectionModel.SelectionFlags(
                QItemSelectionModel.Clear |
                QItemSelectionModel.Select |
                QItemSelectionModel.Current |
                QItemSelectionModel.Rows))

    @pyqtSlot(bool, name='on_resetButton_clicked')
    def on_resetButton_clicked(self, checked=False):
        self.model.loadLayerFields(self.model.layer())

    def resizeColumns(self):
        header = self.fieldsView.horizontalHeader()
        header.resizeSections(QHeaderView.ResizeToContents)
        for section in range(header.count()):
            size = header.sectionSize(section)
            fieldType = self.model.columns[section]['type']
            if fieldType == QgsExpression:
                header.resizeSection(section, size + 100)
            else:
                header.resizeSection(section, size + 20)

    def openPersistentEditors(self, row):
        for index, column in enumerate(self.model.columns):
            if 'persistentEditor' in column.keys() and column['persistentEditor']:
                self.fieldsView.openPersistentEditor(self.model.index(row, index))
                continue

                editor = self.fieldsView.indexWidget(self.model.index(row, index))
                if isinstance(editor, QLineEdit):
                    editor.deselect()
                if isinstance(editor, QSpinBox):
                    lineEdit = editor.findChild(QLineEdit)
                    lineEdit.setAlignment(Qt.AlignRight or Qt.AlignVCenter)
                    lineEdit.deselect()

    def on_model_modelReset(self):
        for row in range(0, self.model.rowCount()):
            self.openPersistentEditors(row)
        self.resizeColumns()

    def on_model_rowsInserted(self, parent, start, end):
        for row in range(start, end + 1):
            self.openPersistentEditors(row)

    @pyqtSlot(bool, name='on_loadLayerFieldsButton_clicked')
    def on_loadLayerFieldsButton_clicked(self, checked=False):
        layer = self.layerCombo.currentLayer()
        if layer is None:
            return
        self.model.loadLayerFields(layer)


class FieldsMappingWidgetWrapper(WidgetWrapper):

    def __init__(self, *args, **kwargs):
        super(FieldsMappingWidgetWrapper, self).__init__(*args, **kwargs)
        self._layer = None

    def createPanel(self):
        return FieldsMappingPanel()

    def createWidget(self):
        self.panel = self.createPanel()
        self.panel.dialogType = self.dialogType

        if self.dialogType == DIALOG_MODELER:
            self.combobox = QComboBox()
            self.combobox.addItem(QCoreApplication.translate('Processing', '[Preconfigure]'), None)
            fieldsMappingInputs = self.dialog.getAvailableValuesOfType(FieldsMapper.ParameterFieldsMapping)
            for input in fieldsMappingInputs:
                self.combobox.addItem(self.dialog.resolveValueDescription(input), input)

            def updatePanelEnabledState():
                if self.combobox.currentData() is None:
                    self.panel.setEnabled(True)
                else:
                    self.panel.setEnabled(False)

            self.combobox.currentIndexChanged.connect(updatePanelEnabledState)

            widget = QWidget()
            widget.setLayout(QVBoxLayout())
            widget.layout().addWidget(self.combobox)
            widget.layout().addWidget(self.panel)
            return widget
        else:
            return self.panel

    def postInitialize(self, wrappers):
        for wrapper in wrappers:
            if wrapper.parameterDefinition().name() == self.parameterDefinition().parentLayerParameter():
                if wrapper.parameterValue():
                    self.setLayer(wrapper.parameterValue())
                wrapper.widgetValueHasChanged.connect(self.parentLayerChanged)
                break

        # remove exiting spacers to get FieldsMappingPanel fully expanded
        if self.dialogType in (DIALOG_STANDARD, DIALOG_MODELER):
            layout = self.widget.parent().layout()
            spacer = layout.itemAt(layout.count() - 1)
            if isinstance(spacer, QSpacerItem):
                layout.removeItem(spacer)

    def parentLayerChanged(self, layer=None):
        self.setLayer(self.sender().value())

    def setLayer(self, layer):
        context = dataobjects.createContext()
        if layer == self._layer:
            return
        if isinstance(layer, QgsProcessingFeatureSourceDefinition):
            layer, ok = layer.source.valueAsString(context.expressionContext())
        if isinstance(layer, str):
            layer = QgsProcessingUtils.mapLayerFromString(layer, context)
        if not isinstance(layer, QgsVectorLayer):
            layer = None
        self._layer = layer
        self.panel.setLayer(self._layer)

    def setValue(self, value):
        self.panel.setValue(value)

    def value(self):
        if self.dialogType == DIALOG_MODELER:
            if self.combobox.currentData() is None:
                return self.panel.value()
            else:
                return self.comboValue(combobox=self.combobox)
        else:
            return self.panel.value()
