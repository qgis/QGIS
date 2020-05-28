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

import os

from qgis.PyQt import uic
from qgis.PyQt.QtCore import (
    QItemSelectionModel,
    QModelIndex,
    pyqtSlot,
    QCoreApplication,
    QVariant,
)

from qgis.PyQt.QtWidgets import (
    QComboBox,
    QSpacerItem,
    QMessageBox,
    QWidget,
    QVBoxLayout
)

from qgis.core import (
    QgsApplication,
    QgsMapLayerProxyModel,
    QgsProcessingFeatureSourceDefinition,
    QgsProcessingUtils,
    QgsVectorLayer,
    QgsField,
    QgsFields,
)

from processing.gui.wrappers import WidgetWrapper, DIALOG_STANDARD, DIALOG_MODELER
from processing.tools import dataobjects
from processing.algs.qgis.FieldsMapper import FieldsMapper


pluginPath = os.path.dirname(__file__)
WIDGET, BASE = uic.loadUiType(
    os.path.join(pluginPath, 'fieldsmappingpanelbase.ui'))


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

        self.layerCombo.setAllowEmptyLayer(True)
        self.layerCombo.setFilters(QgsMapLayerProxyModel.VectorLayer)
        self.dialogType = None
        self.layer = None

    def configure(self):
        self.model = self.fieldsView.model()
        self.fieldsView.setDestinationEditable(True)

    def setLayer(self, layer):
        if layer is None or self.layer == layer:
            return
        self.layer = layer
        if self.model.rowCount(QModelIndex()) == 0:
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
        # Value is a dict with name, type, length, precision and expression
        mapping = self.fieldsView.mapping()
        results = []
        for f in mapping:
            results.append({
                'name': f.field.name(),
                'type': f.field.type(),
                'length': f.field.length(),
                'precision': f.field.precision(),
                'expression': f.expression,
            })
        return results

    def setValue(self, value):
        if type(value) != dict:
            return
        destinationFields = QgsFields()
        expressions = {}
        for field_def in value:
            f = QgsField(field_def.get('name'),
                         field_def.get('type', QVariant.Invalid),
                         field_def.get(QVariant.typeToName(field_def.get('type', QVariant.Invalid))),
                         field_def.get('length', 0),
                         field_def.get('precision', 0))
            try:
                expressions[f.name()] = field_def['expressions']
            except AttributeError:
                pass
            destinationFields.append(f)

        if len(destinationFields):
            self.fieldsView.setDestinationFields(destinationFields, expressions)

    @pyqtSlot(bool, name='on_addButton_clicked')
    def on_addButton_clicked(self, checked=False):
        rowCount = self.model.rowCount(QModelIndex())
        self.model.appendField(QgsField('new_field'))
        index = self.model.index(rowCount, 0)
        self.fieldsView.selectionModel().select(
            index,
            QItemSelectionModel.SelectionFlags(
                QItemSelectionModel.Clear |
                QItemSelectionModel.Select |
                QItemSelectionModel.Current |
                QItemSelectionModel.Rows))
        self.fieldsView.scrollTo(index)

    @pyqtSlot(bool, name='on_deleteButton_clicked')
    def on_deleteButton_clicked(self, checked=False):
        self.fieldsView.removeSelectedFields()

    @pyqtSlot(bool, name='on_upButton_clicked')
    def on_upButton_clicked(self, checked=False):
        self.fieldsView.moveSelectedFieldsUp()

    @pyqtSlot(bool, name='on_downButton_clicked')
    def on_downButton_clicked(self, checked=False):
        self.fieldsView.moveSelectedFieldsDown()

    @pyqtSlot(bool, name='on_resetButton_clicked')
    def on_resetButton_clicked(self, checked=False):
        """Load fields from layer"""
        if self.layer:
            self.fieldsView.setSourceFields(self.layer.fields())
            self.fieldsView.setDestinationFields(self.layer.fields())

    @pyqtSlot(bool, name='on_loadLayerFieldsButton_clicked')
    def on_loadLayerFieldsButton_clicked(self, checked=False):
        layer = self.layerCombo.currentLayer()
        if layer is None:
            return

        self.fieldsView.setSourceFields(layer.fields())
        self.fieldsView.setDestinationFields(layer.fields())


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
        self.setLayer(self.sender().widgetValue())

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

    def linkedVectorLayer(self):
        return self._layer

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
