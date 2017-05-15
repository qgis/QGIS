# -*- coding: utf-8 -*-

"""
***************************************************************************
    wrappers.py - Standard parameters widget wrappers
    ---------------------
    Date                 : May 2016
    Copyright            : (C) 2016 by Arnaud Morvan, Victor Olaya
    Email                : arnaud dot morvan at camptocamp dot com
                           volayaf at gmail dot com
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
__date__ = 'May 2016'
__copyright__ = '(C) 2016, Arnaud Morvan'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'


import locale
import os
from functools import cmp_to_key
from inspect import isclass
from copy import deepcopy

from qgis.core import (
    QgsApplication,
    QgsCoordinateReferenceSystem,
    QgsExpression,
    QgsFieldProxyModel,
    QgsMapLayerProxyModel,
    QgsWkbTypes,
    QgsSettings,
    QgsProject,
    QgsMapLayer,
    QgsProcessingUtils,
    QgsProcessingParameterDefinition,
    QgsProcessingParameterBoolean,
    QgsProcessingParameterCrs,
    QgsProcessingParameterExtent,
    QgsProcessingParameterPoint,
    QgsProcessingParameterFile,
    QgsProcessingParameterMultipleLayers,
    QgsProcessingParameterNumber,
    QgsProcessingParameterRasterLayer,
    QgsProcessingParameterEnum,
    QgsProcessingParameterString,
    QgsProcessingParameterExpression,
    QgsProcessingParameterTable,
    QgsProcessingParameterTableField,
    QgsProcessingParameterVectorLayer)

from qgis.PyQt.QtWidgets import (
    QCheckBox,
    QComboBox,
    QDialog,
    QFileDialog,
    QHBoxLayout,
    QLineEdit,
    QPlainTextEdit,
    QToolButton,
    QWidget,
)
from qgis.gui import (
    QgsExpressionLineEdit,
    QgsExpressionBuilderDialog,
    QgsFieldComboBox,
    QgsFieldExpressionWidget,
    QgsProjectionSelectionDialog,
    QgsMapLayerComboBox,
    QgsProjectionSelectionWidget,
)
from qgis.PyQt.QtCore import pyqtSignal, QObject, QVariant
from qgis.utils import iface

from processing.gui.NumberInputPanel import NumberInputPanel, ModellerNumberInputPanel
from processing.modeler.MultilineTextPanel import MultilineTextPanel
from processing.gui.PointSelectionPanel import PointSelectionPanel
from processing.core.parameters import (ParameterBoolean,
                                        ParameterPoint,
                                        ParameterFile,
                                        ParameterRaster,
                                        ParameterVector,
                                        ParameterNumber,
                                        ParameterString,
                                        ParameterExpression,
                                        ParameterTable,
                                        ParameterTableField,
                                        ParameterExtent,
                                        ParameterFixedTable,
                                        ParameterCrs,
                                        _resolveLayers)
from processing.core.ProcessingConfig import ProcessingConfig
from processing.gui.FileSelectionPanel import FileSelectionPanel
from processing.core.outputs import (OutputFile, OutputRaster, OutputVector,
                                     OutputString, OutputTable, OutputExtent)
from processing.tools import dataobjects
from processing.gui.MultipleInputPanel import MultipleInputPanel
from processing.gui.BatchInputSelectionPanel import BatchInputSelectionPanel
from processing.gui.FixedTablePanel import FixedTablePanel
from processing.gui.ExtentSelectionPanel import ExtentSelectionPanel

DIALOG_STANDARD = 'standard'
DIALOG_BATCH = 'batch'
DIALOG_MODELER = 'modeler'


class InvalidParameterValue(Exception):
    pass


dialogTypes = {"AlgorithmDialog": DIALOG_STANDARD,
               "ModelerParametersDialog": DIALOG_MODELER,
               "BatchAlgorithmDialog": DIALOG_BATCH}


def getExtendedLayerName(layer):
    authid = layer.crs().authid()
    if ProcessingConfig.getSetting(ProcessingConfig.SHOW_CRS_DEF) and authid is not None:
        return u'{} [{}]'.format(layer.name(), authid)
    else:
        return layer.name()


class WidgetWrapper(QObject):

    widgetValueHasChanged = pyqtSignal(object)

    def __init__(self, param, dialog, row=0, col=0, **kwargs):
        QObject.__init__(self)
        self.param = param
        self.dialog = dialog
        self.row = row
        self.col = col
        self.dialogType = dialogTypes.get(dialog.__class__.__name__, DIALOG_STANDARD)
        self.widget = self.createWidget(**kwargs)
        if param.defaultValue() is not None:
            self.setValue(param.defaultValue())

    def comboValue(self, validator=None, combobox=None):
        if combobox is None:
            combobox = self.widget
        idx = combobox.findText(combobox.currentText())
        if idx < 0:
            v = combobox.currentText().strip()
            if validator is not None and not validator(v):
                raise InvalidParameterValue()
            return v
        return combobox.currentData()

    def createWidget(self, **kwargs):
        pass

    def setValue(self, value):
        pass

    def setComboValue(self, value, combobox=None):
        if combobox is None:
            combobox = self.widget
        if isinstance(value, list):
            value = value[0]
        values = [combobox.itemData(i) for i in range(combobox.count())]
        try:
            idx = values.index(value)
            combobox.setCurrentIndex(idx)
            return
        except ValueError:
            pass
        if combobox.isEditable():
            if value is not None:
                combobox.setEditText(str(value))
        else:
            combobox.setCurrentIndex(0)

    def value(self):
        pass

    def postInitialize(self, wrappers):
        pass

    def refresh(self):
        pass

    def getFileName(self, initial_value=''):
        """Shows a file open dialog"""
        settings = QgsSettings()
        if os.path.isdir(initial_value):
            path = initial_value
        elif os.path.isdir(os.path.dirname(initial_value)):
            path = os.path.dirname(initial_value)
        elif settings.contains('/Processing/LastInputPath'):
            path = str(settings.value('/Processing/LastInputPath'))
        else:
            path = ''

        filename, selected_filter = QFileDialog.getOpenFileName(self.widget, self.tr('Select file'),
                                                                path, self.tr(
            'All files (*.*);;') + self.param.getFileFilter())
        if filename:
            settings.setValue('/Processing/LastInputPath',
                              os.path.dirname(str(filename)))
        return filename, selected_filter


class ExpressionWidgetWrapperMixin():

    def wrapWithExpressionButton(self, basewidget):
        expr_button = QToolButton()
        expr_button.clicked.connect(self.showExpressionsBuilder)
        expr_button.setText('...')

        layout = QHBoxLayout()
        layout.setContentsMargins(0, 0, 0, 0)
        layout.addWidget(basewidget)
        layout.addWidget(expr_button)

        widget = QWidget()
        widget.setLayout(layout)

        return widget

    def showExpressionsBuilder(self):
        context = self.param.expressionContext()
        value = self.value()
        if not isinstance(value, str):
            value = ''
        dlg = QgsExpressionBuilderDialog(None, value, self.widget, 'generic', context)
        dlg.setWindowTitle(self.tr('Expression based input'))
        if dlg.exec_() == QDialog.Accepted:
            exp = QgsExpression(dlg.expressionText())
            if not exp.hasParserError():
                self.setValue(dlg.expressionText())


class BasicWidgetWrapper(WidgetWrapper):

    def createWidget(self):
        return QLineEdit()

    def setValue(self, value):
        self.widget.setText(value)

    def value(self):
        return self.widget.text()


class BooleanWidgetWrapper(WidgetWrapper):

    def createWidget(self):
        if self.dialogType == DIALOG_STANDARD:
            return QCheckBox()
        elif self.dialogType == DIALOG_BATCH:
            widget = QComboBox()
            widget.addItem(self.tr('Yes'), True)
            widget.addItem(self.tr('No'), False)
            return widget
        else:
            widget = QComboBox()
            widget.addItem(self.tr('Yes'), True)
            widget.addItem(self.tr('No'), False)
            bools = self.dialog.getAvailableValuesOfType(ParameterBoolean, None)
            for b in bools:
                widget.addItem(self.dialog.resolveValueDescription(b), b)
            return widget

    def setValue(self, value):
        if self.dialogType == DIALOG_STANDARD:
            self.widget.setChecked(value)
        else:
            self.setComboValue(value)

    def value(self):
        if self.dialogType == DIALOG_STANDARD:
            return self.widget.isChecked()
        else:
            return self.comboValue()


class CrsWidgetWrapper(WidgetWrapper):

    def createWidget(self):
        if self.dialogType == DIALOG_MODELER:
            self.combo = QComboBox()
            widget = QWidget()
            layout = QHBoxLayout()
            layout.setMargin(0)
            layout.setContentsMargins(0, 0, 0, 0)
            layout.setSpacing(1)
            layout.addWidget(self.combo)
            btn = QToolButton()
            btn.setIcon(QgsApplication.getThemeIcon("mActionSetProjection.svg"))
            btn.setToolTip(self.tr("Select CRS"))
            btn.clicked.connect(self.selectProjection)
            layout.addWidget(btn)

            widget.setLayout(layout)
            self.combo.setEditable(True)
            crss = self.dialog.getAvailableValuesOfType(ParameterCrs)
            for crs in crss:
                self.combo.addItem(self.dialog.resolveValueDescription(crs), crs)
            raster = self.dialog.getAvailableValuesOfType(ParameterRaster, OutputRaster)
            vector = self.dialog.getAvailableValuesOfType(ParameterVector, OutputVector)
            for r in raster:
                self.combo.addItem("Crs of layer " + self.dialog.resolveValueDescription(r), r)
            for v in vector:
                self.combo.addItem("Crs of layer " + self.dialog.resolveValueDescription(v), v)
            if self.param.defaultValue():
                self.combo.setEditText(self.param.defaultValue())
            return widget
        else:
            widget = QgsProjectionSelectionWidget()
            if self.param.flags() & QgsProcessingParameterDefinition.FlagOptional:
                widget.setOptionVisible(QgsProjectionSelectionWidget.CrsNotSet, True)

            if self.param.defaultValue():
                if self.param.defaultValue() == 'ProjectCrs':
                    crs = QgsProject.instance().crs()
                else:
                    crs = QgsCoordinateReferenceSystem(self.param.defaultValue())
                widget.setCrs(crs)
            else:
                widget.setOptionVisible(QgsProjectionSelectionWidget.CrsNotSet, True)

            return widget

    def selectProjection(self):
        dialog = QgsProjectionSelectionDialog(self.widget)
        current_crs = QgsCoordinateReferenceSystem(self.combo.currentText())
        if current_crs.isValid():
            dialog.setCrs(current_crs)

        if dialog.exec_():
            self.setValue(dialog.crs().authid())

    def setValue(self, value):
        if self.dialogType == DIALOG_MODELER:
            self.setComboValue(value, self.combo)
        elif value == 'ProjectCrs':
            self.widget.setCrs(QgsProject.instance().crs())
        else:
            self.widget.setCrs(QgsCoordinateReferenceSystem(value))

    def value(self):
        if self.dialogType == DIALOG_MODELER:
            return self.comboValue(combobox=self.combo)
        else:
            crs = self.widget.crs()
            if crs.isValid():
                return self.widget.crs().authid()
            else:
                return None


class ExtentWidgetWrapper(WidgetWrapper):

    USE_MIN_COVERING_EXTENT = "[Use min covering extent]"

    def createWidget(self):
        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            return ExtentSelectionPanel(self.dialog, self.param)
        else:
            widget = QComboBox()
            widget.setEditable(True)
            extents = self.dialog.getAvailableValuesOfType(ParameterExtent, OutputExtent)
            if self.param.flags() & QgsProcessingParameterDefinition.FlagOptional:
                widget.addItem(self.USE_MIN_COVERING_EXTENT, None)
            raster = self.dialog.getAvailableValuesOfType(ParameterRaster, OutputRaster)
            vector = self.dialog.getAvailableValuesOfType(ParameterVector, OutputVector)
            for ex in extents:
                widget.addItem(self.dialog.resolveValueDescription(ex), ex)
            for r in raster:
                widget.addItem("Extent of " + self.dialog.resolveValueDescription(r), r)
            for v in vector:
                widget.addItem("Extent of " + self.dialog.resolveValueDescription(v), v)
            if not self.param.defaultValue():
                widget.setEditText(self.param.defaultValue())
            return widget

    def setValue(self, value):
        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            self.widget.setExtentFromString(value)
        else:
            self.setComboValue(value)

    def value(self):
        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            return self.widget.getValue()
        else:
            idx = self.widget.findText(self.widget.currentText())
            if idx < 0:
                s = str(self.widget.currentText()).strip()
                if s:
                    try:
                        tokens = s.split(',')
                        if len(tokens) != 4:
                            raise InvalidParameterValue()
                        for token in tokens:
                            float(token)
                    except:
                        raise InvalidParameterValue()
                elif self.param.flags() & QgsProcessingParameterDefinition.FlagOptional:
                    s = None
                else:
                    raise InvalidParameterValue()
                return s
            else:
                return self.widget.currentData()


class PointWidgetWrapper(WidgetWrapper):

    def createWidget(self):
        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            return PointSelectionPanel(self.dialog, self.param.defaultValue())
        else:
            item = QComboBox()
            item.setEditable(True)
            points = self.dialog.getAvailableValuesOfType(ParameterPoint)
            for p in points:
                item.addItem(self.dialog.resolveValueDescription(p), p)
            item.setEditText(str(self.param.defaultValue()))
            return item

    def setValue(self, value):
        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            self.widget.setPointFromString(value)
        else:
            self.setComboValue(value)

    def value(self):
        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            return self.widget.getValue()
        else:
            idx = self.widget.findText(self.widget.currentText())
            if idx < 0:
                s = str(self.widget.currentText()).strip()
                if s:
                    try:
                        tokens = s.split(',')
                        if len(tokens) != 2:
                            raise InvalidParameterValue()
                        for token in tokens:
                            float(token)
                    except:
                        raise InvalidParameterValue()
                elif self.param.flags() & QgsProcessingParameterDefinition.FlagOptional:
                    s = None
                else:
                    raise InvalidParameterValue()
                return s
            else:
                return self.widget.currentData()


class FileWidgetWrapper(WidgetWrapper):

    def createWidget(self):
        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            return FileSelectionPanel(self.param.behavior() == QgsProcessingParameterFile.Folder, self.param.extension())
        else:
            widget = QComboBox()
            widget.setEditable(True)
            files = self.dialog.getAvailableValuesOfType(ParameterFile, OutputFile)
            for f in files:
                widget.addItem(self.dialog.resolveValueDescription(f), f)
            return widget

    def setValue(self, value):
        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            self.widget.setText(value)
        else:
            self.setComboValue(value)

    def value(self):
        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            return self.widget.getValue()
        else:
            return self.comboValue()


class FixedTableWidgetWrapper(WidgetWrapper):

    def createWidget(self):
        return FixedTablePanel(self.param)

    def setValue(self, value):
        pass

    def value(self):
        if self.dialogType == DIALOG_MODELER:
            table = self.widget.table
            if not bool(table) and not self.param.flags() & QgsProcessingParameterDefinition.FlagOptional:
                raise InvalidParameterValue()
            return ParameterFixedTable.tableToString(table)
        else:
            return self.widget.table


class MultipleInputWidgetWrapper(WidgetWrapper):

    def _getOptions(self):
        if self.param.layerType() == QgsProcessingParameterDefinition.TypeVectorAny:
            options = self.dialog.getAvailableValuesOfType(ParameterVector, OutputVector)
        elif self.param.layerType() == QgsProcessingParameterDefinition.TypeVectorPoint:
            options = self.dialog.getAvailableValuesOfType(ParameterVector, OutputVector,
                                                           [QgsProcessingParameterDefinition.TypeVectorPoint, QgsProcessingParameterDefinition.TypeVectorAny])
        elif self.param.layerType() == QgsProcessingParameterDefinition.TypeVectorLine:
            options = self.dialog.getAvailableValuesOfType(ParameterVector, OutputVector,
                                                           [QgsProcessingParameterDefinition.TypeVectorLine, QgsProcessingParameterDefinition.TypeVectorAny])
        elif self.param.layerType() == QgsProcessingParameterDefinition.TypeVectorPolygon:
            options = self.dialog.getAvailableValuesOfType(ParameterVector, OutputVector,
                                                           [QgsProcessingParameterDefinition.TypeVectorPolygon, QgsProcessingParameterDefinition.TypeVectorAny])
        elif self.param.layerType() == QgsProcessingParameterDefinition.TypeRaster:
            options = self.dialog.getAvailableValuesOfType(ParameterRaster, OutputRaster)
        elif self.param.layerType() == QgsProcessingParameterDefinition.TypeTable:
            options = self.dialog.getAvailableValuesOfType(ParameterTable, OutputTable)
        else:
            options = self.dialog.getAvailableValuesOfType(ParameterFile, OutputFile)
        options = sorted(options, key=lambda opt: self.dialog.resolveValueDescription(opt))
        return options

    def createWidget(self):
        if self.dialogType == DIALOG_STANDARD:
            if self.param.layerType() == QgsProcessingParameterDefinition.TypeFile:
                return MultipleInputPanel(datatype=dataobjects.TYPE_FILE)
            else:
                if self.param.layerType() == QgsProcessingParameterDefinition.TypeRaster:
                    options = QgsProcessingUtils.compatibleRasterLayers(QgsProject.instance(), False)
                elif self.param.layerType() in (QgsProcessingParameterDefinition.TypeVectorAny, QgsProcessingParameterDefinition.TypeTable):
                    options = QgsProcessingUtils.compatibleVectorLayers(QgsProject.instance(), [], False)
                else:
                    options = QgsProcessingUtils.compatibleVectorLayers(QgsProject.instance(), [self.param.layerType()], False)
                opts = [getExtendedLayerName(opt) for opt in options]
                return MultipleInputPanel(opts)
        elif self.dialogType == DIALOG_BATCH:
            widget = BatchInputSelectionPanel(self.param, self.row, self.col, self.dialog)
            widget.valueChanged.connect(lambda: self.widgetValueHasChanged.emit(self))
            return widget
        else:
            options = [self.dialog.resolveValueDescription(opt) for opt in self._getOptions()]
            return MultipleInputPanel(options)

    def refresh(self):
        if self.param.layerType() != QgsProcessingParameterDefinition.TypeFile:
            if self.param.layerType() == QgsProcessingParameterDefinition.TypeRaster:
                options = QgsProcessingUtils.compatibleRasterLayers(QgsProject.instance(), False)
            elif self.param.layerType() in (QgsProcessingParameterDefinition.TypeVectorAny, QgsProcessingParameterDefinition.TypeTable):
                options = QgsProcessingUtils.compatibleVectorLayers(QgsProject.instance(), [], False)
            else:
                options = QgsProcessingUtils.compatibleVectorLayers(QgsProject.instance(), [self.param.layerType()], False)
            opts = [getExtendedLayerName(opt) for opt in options]
            self.widget.updateForOptions(opts)

    def setValue(self, value):
        if self.dialogType == DIALOG_STANDARD:
            pass  # TODO
        elif self.dialogType == DIALOG_BATCH:
            return self.widget.setText(value)
        else:
            options = self._getOptions()
            selected = []
            for i, opt in enumerate(options):
                if opt in value:
                    selected.append(i)
            self.widget.setSelectedItems(selected)

    def value(self):
        if self.dialogType == DIALOG_STANDARD:
            if self.param.layerType() == QgsProcessingParameterDefinition.TypeFile:
                return self.param.setValue(self.widget.selectedoptions)
            else:
                if self.param.layerType() == QgsProcessingParameterDefinition.TypeRaster:
                    options = QgsProcessingUtils.compatibleRasterLayers(QgsProject.instance(), False)
                elif self.param.layerType() in (QgsProcessingParameterDefinition.TypeVectorAny, QgsProcessingParameterDefinition.TypeTable):
                    options = QgsProcessingUtils.compatibleVectorLayers(QgsProject.instance(), [], False)
                else:
                    options = QgsProcessingUtils.compatibleVectorLayers(QgsProject.instance(), [self.param.layerType()], False)
                return [options[i] for i in self.widget.selectedoptions]
        elif self.dialogType == DIALOG_BATCH:
            return self.widget.getText()
        else:
            options = self._getOptions()
            values = [options[i] for i in self.widget.selectedoptions]
            if len(values) == 0 and not self.param.flags() & QgsProcessingParameterDefinition.FlagOptional:
                raise InvalidParameterValue()
            return values


class NumberWidgetWrapper(WidgetWrapper):

    def createWidget(self):
        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            widget = NumberInputPanel(self.param)
            widget.hasChanged.connect(lambda: self.widgetValueHasChanged.emit(self))
            return widget
        else:
            return ModellerNumberInputPanel(self.param, self.dialog)

    def setValue(self, value):
        self.widget.setValue(value)

    def value(self):
        return self.widget.getValue()


class RasterWidgetWrapper(WidgetWrapper):

    NOT_SELECTED = '[Not selected]'

    def createWidget(self):
        if self.dialogType == DIALOG_STANDARD:
            widget = QWidget()
            layout = QHBoxLayout()
            layout.setMargin(0)
            layout.setContentsMargins(0, 0, 0, 0)
            layout.setSpacing(2)
            self.combo = QgsMapLayerComboBox()
            layout.addWidget(self.combo)
            btn = QToolButton()
            btn.setText('...')
            btn.setToolTip(self.tr("Select file"))
            btn.clicked.connect(self.selectFile)
            layout.addWidget(btn)

            widget.setLayout(layout)
            if self.param.flags() & QgsProcessingParameterDefinition.FlagOptional:
                self.combo.setAllowEmptyLayer(True)
            if ProcessingConfig.getSetting(ProcessingConfig.SHOW_CRS_DEF):
                self.combo.setShowCrs(True)

            self.combo.setFilters(QgsMapLayerProxyModel.RasterLayer)
            self.combo.setExcludedProviders(['grass'])
            try:
                if iface.activeLayer().type() == QgsMapLayer.RasterLayer:
                    self.combo.setLayer(iface.activeLayer())
            except:
                pass

            self.combo.currentIndexChanged.connect(lambda: self.widgetValueHasChanged.emit(self))
            self.combo.currentTextChanged.connect(lambda: self.widgetValueHasChanged.emit(self))
            return widget
        elif self.dialogType == DIALOG_BATCH:
            return BatchInputSelectionPanel(self.param, self.row, self.col, self.dialog)
        else:
            self.combo = QComboBox()
            layers = self.dialog.getAvailableValuesOfType(ParameterRaster, OutputRaster)
            self.combo.setEditable(True)
            for layer in layers:
                self.combo.addItem(self.dialog.resolveValueDescription(layer), layer)
            if self.param.flags() & QgsProcessingParameterDefinition.FlagOptional:
                self.combo.setEditText("")

            widget = QWidget()
            layout = QHBoxLayout()
            layout.setMargin(0)
            layout.setContentsMargins(0, 0, 0, 0)
            layout.setSpacing(2)
            layout.addWidget(self.combo)
            btn = QToolButton()
            btn.setText('...')
            btn.setToolTip(self.tr("Select file"))
            btn.clicked.connect(self.selectFile)
            layout.addWidget(btn)
            widget.setLayout(layout)
            return widget

    def selectFile(self):
        filename, selected_filter = self.getFileName(self.combo.currentText())
        if filename:
            filename = dataobjects.getRasterSublayer(filename, self.param)
            if isinstance(self.combo, QgsMapLayerComboBox):
                items = self.combo.additionalItems()
                items.append(filename)
                self.combo.setAdditionalItems(items)
                self.combo.setCurrentIndex(self.combo.findText(filename))
            else:
                self.combo.setEditText(filename)

    def setValue(self, value):
        if self.dialogType == DIALOG_STANDARD:
            pass  # TODO
        elif self.dialogType == DIALOG_BATCH:
            self.widget.setText(value)
        else:
            self.setComboValue(value, combobox=self.combo)

    def value(self):
        if self.dialogType == DIALOG_STANDARD:
            try:
                layer = self.combo.currentLayer()
                if layer:
                    return layer
                else:
                    return self.combo.currentText()
            except:
                return self.combo.currentText()
        elif self.dialogType == DIALOG_BATCH:
            return self.widget.getText()
        else:
            def validator(v):
                if not bool(v):
                    return self.param.flags() & QgsProcessingParameterDefinition.FlagOptional
                else:
                    return os.path.exists(v)
            return self.comboValue(validator, combobox=self.combo)


class SelectionWidgetWrapper(WidgetWrapper):

    def createWidget(self):
        if self.param.allowMultiple():
            return MultipleInputPanel(options=self.param.options())
        else:
            widget = QComboBox()
            for i, option in enumerate(self.param.options()):
                widget.addItem(option, i)
            if self.param.defaultValue():
                widget.setCurrentIndex(widget.findData(self.param.defaultValue()))
            return widget

    def setValue(self, value):
        if self.param.allowMultiple():
            self.widget.setSelectedItems(value)
        else:
            self.widget.setCurrentIndex(self.widget.findData(value))

    def value(self):
        if self.param.allowMultiple():
            return self.widget.selectedoptions
        else:
            return self.widget.currentData()


class VectorWidgetWrapper(WidgetWrapper):

    NOT_SELECTED = '[Not selected]'

    def createWidget(self):
        if self.dialogType == DIALOG_STANDARD:
            widget = QWidget()
            layout = QHBoxLayout()
            layout.setMargin(0)
            layout.setContentsMargins(0, 0, 0, 0)
            layout.setSpacing(2)
            self.combo = QgsMapLayerComboBox()
            layout.addWidget(self.combo)
            btn = QToolButton()
            btn.setText('...')
            btn.setToolTip(self.tr("Select file"))
            btn.clicked.connect(self.selectFile)
            layout.addWidget(btn)

            widget.setLayout(layout)

            filters = QgsMapLayerProxyModel.Filters()
            if QgsProcessingParameterDefinition.TypeVectorAny == self.param.dataType():
                filters = QgsMapLayerProxyModel.HasGeometry
            if QgsProcessingParameterDefinition.TypeVectorPoint == self.param.dataType():
                filters |= QgsMapLayerProxyModel.PointLayer
            if QgsProcessingParameterDefinition.TypeVectorLine == self.param.dataType():
                filters |= QgsMapLayerProxyModel.LineLayer
            if QgsProcessingParameterDefinition.TypeVectorPolygon == self.param.dataType():
                filters |= QgsMapLayerProxyModel.PolygonLayer

            try:
                if iface.activeLayer().type() == QgsMapLayer.VectorLayer:
                    self.combo.setLayer(iface.activeLayer())
            except:
                pass

            if self.param.flags() & QgsProcessingParameterDefinition.FlagOptional:
                self.combo.setAllowEmptyLayer(True)
            if ProcessingConfig.getSetting(ProcessingConfig.SHOW_CRS_DEF):
                self.combo.setShowCrs(True)

            self.combo.setFilters(filters)
            self.combo.setExcludedProviders(['grass'])

            self.combo.currentIndexChanged.connect(lambda: self.widgetValueHasChanged.emit(self))
            self.combo.currentTextChanged.connect(lambda: self.widgetValueHasChanged.emit(self))
            return widget

        elif self.dialogType == DIALOG_BATCH:
            widget = BatchInputSelectionPanel(self.param, self.row, self.col, self.dialog)
            widget.valueChanged.connect(lambda: self.widgetValueHasChanged.emit(self))
            return widget
        else:
            self.combo = QComboBox()
            layers = self.dialog.getAvailableValuesOfType(ParameterVector, OutputVector)
            self.combo.setEditable(True)
            for layer in layers:
                self.combo.addItem(self.dialog.resolveValueDescription(layer), layer)
            if self.param.flags() & QgsProcessingParameterDefinition.FlagOptional:
                self.combo.setEditText("")

            widget = QWidget()
            layout = QHBoxLayout()
            layout.setMargin(0)
            layout.setContentsMargins(0, 0, 0, 0)
            layout.setSpacing(2)
            layout.addWidget(self.combo)
            btn = QToolButton()
            btn.setText('...')
            btn.setToolTip(self.tr("Select file"))
            btn.clicked.connect(self.selectFile)
            layout.addWidget(btn)
            widget.setLayout(layout)
            return widget

    def selectFile(self):
        filename, selected_filter = self.getFileName(self.combo.currentText())
        if filename:
            filename = dataobjects.getRasterSublayer(filename, self.param)
            if isinstance(self.combo, QgsMapLayerComboBox):
                items = self.combo.additionalItems()
                items.append(filename)
                self.combo.setAdditionalItems(items)
                self.combo.setCurrentIndex(self.combo.findText(filename))
            else:
                self.combo.setEditText(filename)

    def setValue(self, value):
        if self.dialogType == DIALOG_STANDARD:
            pass  # TODO
        elif self.dialogType == DIALOG_BATCH:
            self.widget.setValue(value)
        else:
            self.setComboValue(value, combobox=self.combo)

    def value(self):
        if self.dialogType == DIALOG_STANDARD:
            try:
                layer = self.combo.currentLayer()
                if layer:
                    return layer.id()
                else:
                    return self.combo.currentText()
            except:
                return self.combo.currentText()
        elif self.dialogType == DIALOG_BATCH:
            return self.widget.value()
        else:
            def validator(v):
                if not bool(v):
                    return self.param.flags() & QgsProcessingParameterDefinition.FlagOptional
                else:
                    return os.path.exists(v)
            return self.comboValue(validator, combobox=self.combo)


class StringWidgetWrapper(WidgetWrapper, ExpressionWidgetWrapperMixin):

    def createWidget(self):
        if self.dialogType == DIALOG_STANDARD:
            if self.param.multiLine():
                widget = QPlainTextEdit()
            else:
                self._lineedit = QLineEdit()
                return self.wrapWithExpressionButton(self._lineedit)

        elif self.dialogType == DIALOG_BATCH:
            widget = QLineEdit()

        else:
            # strings, numbers, files and table fields are all allowed input types
            strings = self.dialog.getAvailableValuesOfType([ParameterString, ParameterNumber, ParameterFile,
                                                            ParameterTableField, ParameterExpression], OutputString)
            options = [(self.dialog.resolveValueDescription(s), s) for s in strings]
            if self.param.multiLine():
                widget = MultilineTextPanel(options)
            else:
                widget = QComboBox()
                widget.setEditable(True)
                for desc, val in options:
                    widget.addItem(desc, val)
        return widget

    def setValue(self, value):
        if self.dialogType == DIALOG_STANDARD:
            if self.param.multiLine():
                self.widget.setPlainText(value)
            else:
                self._lineedit.setText(value)

        elif self.dialogType == DIALOG_BATCH:
            self.widget.setText(value)

        else:
            if self.param.multiLine():
                self.widget.setValue(value)
            else:
                self.setComboValue(value)

    def value(self):
        if self.dialogType in DIALOG_STANDARD:
            if self.param.multiLine():
                text = self.widget.toPlainText()
            else:
                text = self._lineedit.text()
            return text

        elif self.dialogType == DIALOG_BATCH:
            return self.widget.text()

        else:
            if self.param.multiLine():
                value = self.widget.getValue()
                option = self.widget.getOption()
                if option == MultilineTextPanel.USE_TEXT:
                    if value == '':
                        if self.param.flags() & QgsProcessingParameterDefinition.FlagOptional:
                            return None
                        else:
                            raise InvalidParameterValue()
                    else:
                        return value
                else:
                    return value
            else:
                def validator(v):
                    return bool(v) or self.param.flags() & QgsProcessingParameterDefinition.FlagOptional
                return self.comboValue(validator)


class ExpressionWidgetWrapper(WidgetWrapper):

    def createWidget(self):
        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            if self.param.parentLayerParameter():
                widget = QgsFieldExpressionWidget()
            else:
                widget = QgsExpressionLineEdit()
            if self.param.defaultValue():
                widget.setExpression(self.param.defaultValue())
        else:
            strings = self.dialog.getAvailableValuesOfType([ParameterExpression, ParameterString, ParameterNumber], OutputString)
            options = [(self.dialog.resolveValueDescription(s), s) for s in strings]
            widget = QComboBox()
            widget.setEditable(True)
            for desc, val in options:
                widget.addItem(desc, val)
            widget.setEditText(self.param.default or "")
        return widget

    def postInitialize(self, wrappers):
        for wrapper in wrappers:
            if wrapper.param.name() == self.param.parentLayerParameter():
                if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
                    self.setLayer(wrapper.value())
                    wrapper.widgetValueHasChanged.connect(self.parentLayerChanged)
                break

    def parentLayerChanged(self, wrapper):
        self.setLayer(wrapper.value())

    def setLayer(self, layer):
        context = dataobjects.createContext()
        if isinstance(layer, str):
            layer = QgsProcessingUtils.mapLayerFromString(_resolveLayers(layer), context)
        self.widget.setLayer(layer)

    def setValue(self, value):
        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            self.widget.setExpression(value)
        else:
            self.setComboValue(value)

    def value(self):
        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            try:
                return self.widget.asExpression()
            except:
                return self.widget.expression()
        else:
            def validator(v):
                return bool(v) or self.param.flags() & QgsProcessingParameterDefinition.FlagOptional
            return self.comboValue(validator)


class TableWidgetWrapper(WidgetWrapper):

    NOT_SELECTED = '[Not selected]'

    def createWidget(self):
        if self.dialogType == DIALOG_STANDARD:
            widget = QWidget()
            layout = QHBoxLayout()
            layout.setMargin(0)
            layout.setContentsMargins(0, 0, 0, 0)
            layout.setSpacing(2)
            self.combo = QgsMapLayerComboBox()
            layout.addWidget(self.combo)
            btn = QToolButton()
            btn.setText('...')
            btn.setToolTip(self.tr("Select file"))
            btn.clicked.connect(self.selectFile)
            layout.addWidget(btn)

            widget.setLayout(layout)

            if self.param.flags() & QgsProcessingParameterDefinition.FlagOptional:
                self.combo.setAllowEmptyLayer(True)

            self.combo.setFilters(QgsMapLayerProxyModel.VectorLayer)
            self.combo.setExcludedProviders(['grass'])
            try:
                if iface.activeLayer().type() == QgsMapLayer.VectorLayer:
                    self.combo.setLayer(iface.activeLayer())
            except:
                pass

            self.combo.currentIndexChanged.connect(lambda: self.widgetValueHasChanged.emit(self))
            self.combo.currentTextChanged.connect(lambda: self.widgetValueHasChanged.emit(self))
            return widget

        elif self.dialogType == DIALOG_BATCH:
            return BatchInputSelectionPanel(self.param, self.row, self.col, self.dialog)
        else:
            self.combo = QComboBox()
            layers = self.dialog.getAvailableValuesOfType(ParameterRaster, OutputRaster)
            self.combo.setEditable(True)
            tables = self.dialog.getAvailableValuesOfType(ParameterTable, OutputTable)
            layers = self.dialog.getAvailableValuesOfType(ParameterVector, OutputVector)
            if self.param.flags() & QgsProcessingParameterDefinition.FlagOptional:
                self.combo.addItem(self.NOT_SELECTED, None)
            for table in tables:
                self.combo.addItem(self.dialog.resolveValueDescription(table), table)
            for layer in layers:
                self.combo.addItem(self.dialog.resolveValueDescription(layer), layer)

            widget = QWidget()
            layout = QHBoxLayout()
            layout.setMargin(0)
            layout.setContentsMargins(0, 0, 0, 0)
            layout.setSpacing(2)
            layout.addWidget(self.combo)
            btn = QToolButton()
            btn.setText('...')
            btn.setToolTip(self.tr("Select file"))
            btn.clicked.connect(self.selectFile)
            layout.addWidget(btn)
            widget.setLayout(layout)
            return widget

    def selectFile(self):
        filename, selected_filter = self.getFileName(self.combo.currentText())
        if filename:
            filename = dataobjects.getRasterSublayer(filename, self.param)
            if isinstance(self.combo, QgsMapLayerComboBox):
                items = self.combo.additionalItems()
                items.append(filename)
                self.combo.setAdditionalItems(items)
                self.combo.setCurrentIndex(self.combo.findText(filename))
            else:
                self.combo.setEditText(filename)

    def setValue(self, value):
        if self.dialogType == DIALOG_STANDARD:
            pass  # TODO
        elif self.dialogType == DIALOG_BATCH:
            return self.widget.setText(value)
        else:
            self.setComboValue(value, combobox=self.combo)

    def value(self):
        if self.dialogType == DIALOG_STANDARD:
            try:
                layer = self.combo.currentLayer()
                if layer:
                    return layer
                else:
                    return self.combo.currentText()
            except:
                return self.combo.currentText()
        elif self.dialogType == DIALOG_BATCH:
            return self.widget.value()
        else:
            def validator(v):
                return bool(v) or self.param.flags() & QgsProcessingParameterDefinition.FlagOptional
            return self.comboValue(validator, combobox=self.combo)


class TableFieldWidgetWrapper(WidgetWrapper):

    NOT_SET = '[Not set]'

    def createWidget(self):
        self._layer = None

        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            if self.param.allowMultiple():
                return MultipleInputPanel(options=[])
            else:
                widget = QgsFieldComboBox()
                widget.setAllowEmptyFieldName(self.param.flags() & QgsProcessingParameterDefinition.FlagOptional)
                widget.fieldChanged.connect(lambda: self.widgetValueHasChanged.emit(self))
                if self.param.dataType() == QgsProcessingParameterTableField.Numeric:
                    widget.setFilters(QgsFieldProxyModel.Numeric)
                elif self.param.dataType() == QgsProcessingParameterTableField.String:
                    widget.setFilters(QgsFieldProxyModel.String)
                elif self.param.dataType() == QgsProcessingParameterTableField.DateTime:
                    widget.setFilters(QgsFieldProxyModel.Date | QgsFieldProxyModel.Time)
                return widget
        else:
            widget = QComboBox()
            widget.setEditable(True)
            fields = self.dialog.getAvailableValuesOfType(ParameterTableField, None)
            if self.param.flags() & QgsProcessingParameterDefinition.FlagOptional:
                widget.addItem(self.NOT_SET, None)
            for f in fields:
                widget.addItem(self.dialog.resolveValueDescription(f), f)
            return widget

    def postInitialize(self, wrappers):
        for wrapper in wrappers:
            if wrapper.param.name() == self.param.parentLayerParameter():
                if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
                    self.setLayer(wrapper.value())
                    wrapper.widgetValueHasChanged.connect(self.parentValueChanged)
                break

    def parentValueChanged(self, wrapper):
        self.setLayer(wrapper.value())

    def setLayer(self, layer):
        context = dataobjects.createContext()
        if isinstance(layer, str):
            layer = QgsProcessingUtils.mapLayerFromString(_resolveLayers(layer), context)
        self._layer = layer
        self.refreshItems()

    def refreshItems(self):
        if self.param.allowMultiple():
            self.widget.updateForOptions(self.getFields())
        else:
            self.widget.setLayer(self._layer)
            self.widget.setCurrentIndex(0)

    def getFields(self):
        if self._layer is None:
            return []
        fieldTypes = []
        if self.param.dataType() == QgsProcessingParameterTableField.String:
            fieldTypes = [QVariant.String]
        elif self.param.dataType() == QgsProcessingParameterTableField.Numeric:
            fieldTypes = [QVariant.Int, QVariant.Double, QVariant.LongLong,
                          QVariant.UInt, QVariant.ULongLong]
        elif self.param.dataType() == QgsProcessingParameterTableField.DateTime:
            fieldTypes = [QVariant.Date, QVariant.Time, QVariant.DateTime]

        fieldNames = set()
        for field in self._layer.fields():
            if not fieldTypes or field.type() in fieldTypes:
                fieldNames.add(str(field.name()))
        return sorted(list(fieldNames), key=cmp_to_key(locale.strcoll))

    def setValue(self, value):
        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            if self.param.allowMultiple():
                options = self.widget.options
                selected = []
                for i, opt in enumerate(options):
                    if opt in value:
                        selected.append(i)
                self.widget.setSelectedItems(selected)
            else:
                self.widget.setField(value)
        else:
            self.setComboValue(value)

    def value(self):
        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            if self.param.allowMultiple():
                return [self.widget.options[i] for i in self.widget.selectedoptions]
            else:
                f = self.widget.currentField()
                if self.param.flags() & QgsProcessingParameterDefinition.FlagOptional and not f:
                    return None
                return f
        else:
            def validator(v):
                return bool(v) or self.param.flags() & QgsProcessingParameterDefinition.FlagOptional
            return self.comboValue(validator)


class WidgetWrapperFactory:
    """
    Factory for parameter widget wrappers
    """

    @staticmethod
    def create_wrapper(param, dialog, row=0, col=0):

        if hasattr(param, 'metadata'):
            return WidgetWrapperFactory.create_wrapper_from_metadata(param, dialog, row, col)
        else:
            return WidgetWrapperFactory.create_wrapper_from_class(param, dialog, row, col)

    @staticmethod
    def create_wrapper_from_metadata(param, dialog, row=0, col=0):
        wrapper = param.metadata.get('widget_wrapper', None)
        params = {}
        # wrapper metadata should be a dict with class key
        if isinstance(wrapper, dict):
            params = deepcopy(wrapper)
            wrapper = params.pop('class')
        # wrapper metadata should be a class path
        if isinstance(wrapper, str):
            tokens = wrapper.split('.')
            mod = __import__('.'.join(tokens[:-1]), fromlist=[tokens[-1]])
            wrapper = getattr(mod, tokens[-1])
        # or directly a class object
        if isclass(wrapper):
            wrapper = wrapper(param, dialog, row, col, **params)
        # or a wrapper instance
        return wrapper

    @staticmethod
    def create_wrapper_from_class(param, dialog, row=0, col=0):
        wrapper = None
        if param.type() == 'boolean':
            wrapper = BooleanWidgetWrapper
        elif param.type() == 'crs':
            wrapper = CrsWidgetWrapper
        elif param.type() == 'extent':
            wrapper = ExtentWidgetWrapper
        elif param.type() == 'point':
            wrapper = PointWidgetWrapper
        elif param.type() == 'file':
            wrapper = FileWidgetWrapper
        elif param.type() == 'multilayer':
            wrapper = MultipleInputWidgetWrapper
        elif param.type() == 'number':
            wrapper = NumberWidgetWrapper
        elif param.type() == 'raster':
            wrapper = RasterWidgetWrapper
        elif param.type() == 'enum':
            wrapper = SelectionWidgetWrapper
        elif param.type() == 'string':
            wrapper = StringWidgetWrapper
        elif param.type() == 'expression':
            wrapper = ExpressionWidgetWrapper
        elif param.type() == 'table':
            wrapper = TableWidgetWrapper
        elif param.type() == 'field':
            wrapper = TableFieldWidgetWrapper
        elif param.type() == 'vector':
            wrapper = VectorWidgetWrapper
        else:
            assert False, param.type()
        return wrapper(param, dialog, row, col)
