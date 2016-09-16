# -*- coding: utf-8 -*-

"""
***************************************************************************
    wrappers.py
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

from qgis.core import QgsCoordinateReferenceSystem
from qgis.PyQt.QtWidgets import QCheckBox, QComboBox, QLineEdit, QPlainTextEdit
from qgis.PyQt.QtCore import pyqtSignal, QObject, QVariant

from processing.gui.NumberInputPanel import NumberInputPanel
from processing.gui.InputLayerSelectorPanel import InputLayerSelectorPanel
from processing.modeler.MultilineTextPanel import MultilineTextPanel
from processing.gui.CrsSelectionPanel import CrsSelectionPanel
from processing.gui.PointSelectionPanel import PointSelectionPanel
from processing.core.parameters import (ParameterBoolean, ParameterPoint, ParameterFile,
    ParameterRaster, ParameterVector, ParameterNumber, ParameterString, ParameterTable,
    ParameterTableField, ParameterExtent, ParameterFixedTable, ParameterCrs)
from processing.core.ProcessingConfig import ProcessingConfig
from processing.gui.FileSelectionPanel import FileSelectionPanel
from processing.core.outputs import (OutputFile, OutputRaster, OutputVector, OutputNumber,
    OutputString, OutputTable, OutputExtent)
from processing.tools import dataobjects
from processing.gui.MultipleInputPanel import MultipleInputPanel
from processing.gui.BatchInputSelectionPanel import BatchInputSelectionPanel
from processing.gui.FixedTablePanel import FixedTablePanel
from processing.gui.ExtentSelectionPanel import ExtentSelectionPanel
from processing.gui.StringInputPanel import StringInputPanel
from processing.gui.GeometryPredicateSelectionPanel import GeometryPredicateSelectionPanel


DIALOG_STANDARD = 'standard'
DIALOG_BATCH = 'batch'
DIALOG_MODELER = 'modeler'

class InvalidParameterValue(Exception):
    pass


dialogTypes = {"AlgorithmDialog":DIALOG_STANDARD,
               "ModelerParametersDialog":DIALOG_MODELER,
               "BatchAlgorithmDialog": DIALOG_BATCH}

def getExtendedLayerName(layer):
    authid = layer.crs().authid()
    if ProcessingConfig.getSetting(ProcessingConfig.SHOW_CRS_DEF) and authid is not None:
        return u'{} [{}]'.format(layer.name(), authid)
    else:
        return layer.name()

class WidgetWrapper(QObject):

    widgetValueHasChanged = pyqtSignal(object)

    def __init__(self, param, dialog, row=0, col=0):
        QObject.__init__(self)
        self.param = param
        self.dialog = dialog
        self.row = row
        self.col = col
        self.dialogType = dialogTypes.get(dialog.__class__.__name__, DIALOG_STANDARD)
        self.widget = self.createWidget()
        if param.default is not None:
            self.setValue(param.default)

    def comboValue(self, validator=None):
        idx = self.widget.findText(self.widget.currentText())
        if idx < 0:
            v = self.widget.currentText().strip()
            if validator is not None and not validator(v):
                raise InvalidParameterValue()
            return v
        else:
            return self.widget.itemData(self.widget.currentIndex())

    def createWidget(self):
        pass

    def setValue(self, value):
        pass

    def setComboValue(self, value):
        if isinstance(value, list):
            value = value[0]
        values = [self.widget.itemData(i) for i in range(self.widget.count())]
        try:
            idx = values.index(value)
            self.widget.setCurrentIndex(idx)
            return
        except ValueError:
            pass
        if self.widget.isEditable():
            if value is not None:
                self.widget.setEditText(unicode(value))
        else:
            self.widget.setCurrentIndex(0)

    def value(self):
        pass

    def anotherParameterWidgetHasChanged(self, wrapper):
        pass

    def postInitialize(self, wrappers):
        pass

    def refresh(self):
        pass

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
            widget.addItem(self.tr('Yes'))
            widget.addItem(self.tr('No'))
            if self.param.default:
                widget.setCurrentIndex(0)
            else:
                widget.setCurrentIndex(1)
            return widget
        else:
            widget = QComboBox()
            widget.addItem('Yes', True)
            widget.addItem('No', False)
            bools = self.dialog.getAvailableValuesOfType(ParameterBoolean, None)
            for b in bools:
                widget.addItem(self.dialog.resolveValueDescription(b), b)
            if self.param.default:
                widget.setCurrentIndex(0)
            else:
                widget.setCurrentIndex(1)
            return widget

    def setValue(self, value):
        if self.dialogType == DIALOG_STANDARD:
            self.widget.setChecked(value)
        else:
            self.setComboValue(value)

    def value(self):
        if self.dialogType == DIALOG_STANDARD:
            return self.widget.isChecked()
        elif self.dialogType == DIALOG_BATCH:
            return self.widget.currentIndex == 0
        else:
            return self.comboValue()


class CrsWidgetWrapper(WidgetWrapper):

    def createWidget(self):
        if self.dialogType == DIALOG_MODELER:
            widget = QComboBox()
            widget.setEditable(True)
            crss = self.dialog.getAvailableValuesOfType(ParameterCrs)
            for crs in crss:
                widget.addItem(self.dialog.resolveValueDescription(crs), crs)
            raster = self.dialog.getAvailableValuesOfType(ParameterRaster, OutputRaster)
            vector = self.dialog.getAvailableValuesOfType(ParameterVector, OutputVector)
            for r in raster:
                widget.addItem("Crs of layer " + self.dialog.resolveValueDescription(r), r)
            for v in vector:
                widget.addItem("Crs of layer " + self.dialog.resolveValueDescription(v), v)
            if not self.param.default:
                widget.setEditText(self.param.default)
            return widget
        else:
            return CrsSelectionPanel()

    def setValue(self, value):
        if self.dialogType == DIALOG_MODELER:
            self.setComboValue(value)
        else:
            if isinstance(value, basestring):  # authId
                self.widget.crs = value
            else:
                self.widget.crs = QgsCoordinateReferenceSystem(value).authid()
            self.widget.updateText()

    def value(self):
        if self.dialogType == DIALOG_MODELER:
            return self.comboValue()
        else:
            return self.widget.getValue()

class ExtentWidgetWrapper(WidgetWrapper):

    USE_MIN_COVERING_EXTENT = "[Use min covering extent]"

    def createWidget(self):
        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            return ExtentSelectionPanel(self.dialog, self.param)
        else:
            widget = QComboBox()
            widget.setEditable(True)
            extents = self.dialog.getAvailableValuesOfType(ParameterExtent, OutputExtent)
            if self.param.optional:
                widget.addItem(self.USE_MIN_COVERING_EXTENT, None)
            raster = self.dialog.getAvailableValuesOfType(ParameterRaster, OutputRaster)
            vector = self.dialog.getAvailableValuesOfType(ParameterVector, OutputVector)
            for ex in extents:
                widget.addItem(self.dialog.resolveValueDescription(ex), ex)
            for r in raster:
                widget.addItem("Extent of " + self.dialog.resolveValueDescription(r), r)
            for v in vector:
                widget.addItem("Extent of " + self.dialog.resolveValueDescription(v), v)
            if not self.param.default:
                widget.setEditText(self.param.default)
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
                s = unicode(self.widget.currentText()).strip()
                if s:
                    try:
                        tokens = s.split(',')
                        if len(tokens) != 4:
                            raise InvalidParameterValue()
                        for token in tokens:
                            float(token)
                    except:
                        raise InvalidParameterValue()
                elif self.param.optional:
                    s = None
                else:
                    raise InvalidParameterValue()
                return s
            else:
                return self.widget.itemData(self.widget.currentIndex())


class PointWidgetWrapper(WidgetWrapper):

    def createWidget(self):
        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            return PointSelectionPanel(self.dialog, self.param.default)
        else:
            item = QComboBox()
            item.setEditable(True)
            points = self.dialog.getAvailableValuesOfType(ParameterPoint)
            for p in points:
                item.addItem(self.dialog.resolveValueDescription(p), p)
            item.setEditText(unicode(self.param.default))

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
                s = unicode(self.widget.currentText()).strip()
                if s:
                    try:
                        tokens = s.split(',')
                        if len(tokens) != 2:
                            raise InvalidParameterValue()
                        for token in tokens:
                            float(token)
                    except:
                        raise InvalidParameterValue()
                elif self.param.optional:
                    s = None
                else:
                    raise InvalidParameterValue()
                return s
            else:
                return self.widget.itemData(self.widget.currentIndex())


class FileWidgetWrapper(WidgetWrapper):

    def createWidget(self):
        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            return FileSelectionPanel(self.param.isFolder, self.param.ext)
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
            if not bool(table) and not self.param.optional:
                raise InvalidParameterValue()
            return ParameterFixedTable.tableToString(table)
        else:
            return self.widget.table


class MultipleInputWidgetWrapper(WidgetWrapper):

    def _getOptions(self):
        if self.param.datatype == dataobjects.TYPE_VECTOR_ANY:
            options = self.dialog.getAvailableValuesOfType(ParameterVector, OutputVector)
        elif self.param.datatype == dataobjects.TYPE_VECTOR_POINT:
            options = self.dialog.getAvailableValuesOfType(ParameterVector, OutputVector,
                                                           [dataobjects.TYPE_VECTOR_POINT, dataobjects.TYPE_VECTOR_ANY])
        elif self.param.datatype == dataobjects.TYPE_VECTOR_LINE:
            options = self.dialog.getAvailableValuesOfType(ParameterVector, OutputVector,
                                                           [dataobjects.TYPE_VECTOR_LINE, dataobjects.TYPE_VECTOR_ANY])
        elif self.param.datatype == dataobjects.TYPE_VECTOR_POLYGON:
            options = self.dialog.getAvailableValuesOfType(ParameterVector, OutputVector,
                                                           [dataobjects.TYPE_VECTOR_POLYGON, dataobjects.TYPE_VECTOR_ANY])
        elif self.param.datatype == dataobjects.TYPE_RASTER:
            options = self.dialog.getAvailableValuesOfType(ParameterRaster, OutputRaster)
        else:
            options = self.dialog.getAvailableValuesOfType(ParameterFile, OutputFile)
        return options

    def createWidget(self):
        if self.dialogType == DIALOG_STANDARD:
            if self.param.datatype == dataobjects.TYPE_FILE:
                return MultipleInputPanel(datatype=dataobjects.TYPE_FILE)
            else:
                if self.param.datatype == dataobjects.TYPE_RASTER:
                    options = dataobjects.getRasterLayers(sorting=False)
                elif self.param.datatype == dataobjects.TYPE_VECTOR_ANY:
                    options = dataobjects.getVectorLayers(sorting=False)
                else:
                    options = dataobjects.getVectorLayers([self.param.datatype], sorting=False)
                opts = [getExtendedLayerName(opt) for opt in options]
                return MultipleInputPanel(opts)
        elif self.dialogType == DIALOG_BATCH:
            return BatchInputSelectionPanel(self.param, self.row, self.col, self.dialog)
        else:
            options = [self.dialog.resolveValueDescription(opt) for opt in self._getOptions()]
            return MultipleInputPanel(options)

    def refresh(self):
        if self.param.datatype != dataobjects.TYPE_FILE:
            if self.param.datatype == dataobjects.TYPE_RASTER:
                options = dataobjects.getRasterLayers(sorting=False)
            elif self.param.datatype == dataobjects.TYPE_VECTOR_ANY:
                options = dataobjects.getVectorLayers(sorting=False)
            else:
                options = dataobjects.getVectorLayers([self.param.datatype], sorting=False)
            opts = [self.getExtendedLayerName(opt) for opt in options]
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
            if self.param.datatype == dataobjects.TYPE_FILE:
                return self.param.setValue(self.widget.selectedoptions)
            else:
                if self.param.datatype == dataobjects.TYPE_RASTER:
                    options = dataobjects.getRasterLayers(sorting=False)
                elif self.param.datatype == dataobjects.TYPE_VECTOR_ANY:
                    options = dataobjects.getVectorLayers(sorting=False)
                else:
                    options = dataobjects.getVectorLayers([self.param.datatype], sorting=False)
                return [options[i] for i in self.widget.selectedoptions]
        elif self.dialogType == DIALOG_BATCH:
            return self.widget.getText()
        else:
            options = self._getOptions()
            values = [options[i] for i in self.widget.selectedoptions]
            if len(values) == 0 and not self.param.optional:
                raise InvalidParameterValue()
            return values


class NumberWidgetWrapper(WidgetWrapper):

    def createWidget(self):
        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            return NumberInputPanel(self.param, None)
        else:
            return NumberInputPanel(self.param, self.dialog)

    def setValue(self, value):
        self.widget.setValue(value)

    def value(self):
        return self.widget.getValue()

class RasterWidgetWrapper(WidgetWrapper):

    NOT_SELECTED = '[Not selected]'

    def createWidget(self):
        if self.dialogType == DIALOG_STANDARD:
            layers = dataobjects.getRasterLayers()
            items = []
            if self.param.optional:
                items.append((self.NOT_SELECTED, None))
            for layer in layers:
                items.append((getExtendedLayerName(layer), layer))
            return InputLayerSelectorPanel(items, self.param)
        elif self.dialogType == DIALOG_BATCH:
            return BatchInputSelectionPanel(self.param, self.row, self.col, self.dialog)
        else:
            widget = QComboBox()
            widget.setEditable(True)
            files = self.dialog.getAvailableValuesOfType(ParameterRaster, OutputRaster)
            for f in files:
                widget.addItem(self.dialog.resolveValueDescription(f), f)
            return widget

    def refresh(self):
        self.widget.cmbText.clear()
        layers = dataobjects.getRasterLayers(self)
        layers.sort(key=lambda lay: lay.name())
        if self.param.optional:
            self.widget.cmbText.addItem(self.NOT_SELECTED, None)
        for layer in layers:
            self.widget.cmbText.addItem(getExtendedLayerName(layer), layer)

    def setValue(self, value):
        if self.dialogType == DIALOG_STANDARD:
            pass  # TODO
        elif self.dialogType == DIALOG_BATCH:
            self.widget.setText(value)
        else:
            self.setComboValue(value)

    def value(self):
        if self.dialogType == DIALOG_STANDARD:
            return self.widget.getValue()
        elif self.dialogType == DIALOG_BATCH:
            return self.widget.getText()
        else:
            def validator(v):
                return bool(v) or self.param.optional
            return self.comboValue(validator)


class SelectionWidgetWrapper(WidgetWrapper):

    def createWidget(self):
        if self.param.multiple:
            return MultipleInputPanel(options=self.param.options)
        else:
            widget = QComboBox()
            widget.addItems(self.param.options)
            if self.param.default:
                widget.setCurrentIndex(self.param.default)
            return widget

    def setValue(self, value):
        if self.param.multiple:
            self.widget.setSelectedItems(value)
        else:
            self.widget.setCurrentIndex(int(value))

    def value(self):
        if self.param.multiple:
                return self.widget.selectedoptions
        else:
            return self.widget.currentIndex()


class VectorWidgetWrapper(WidgetWrapper):

    NOT_SELECTED = '[Not selected]'

    def createWidget(self):
        if self.dialogType == DIALOG_STANDARD:
            widget = QComboBox()
            self._populate(widget)
            return widget
        elif self.dialogType == DIALOG_BATCH:
            return BatchInputSelectionPanel(self.param, self.row, self.col, self.dialog)
        else:
            widget = QComboBox()
            layers = self.dialog.getAvailableValuesOfType(ParameterVector, OutputVector)
            if self.param.optional:
                widget.addItem(self.NOT_SELECTED, None)
            for layer in layers:
                widget.addItem(self.dialog.resolveValueDescription(layer), layer)
            return widget

    def _populate(self, widget):
        widget.clear()
        layers = dataobjects.getVectorLayers(self.param.datatype)
        layers.sort(key=lambda lay: lay.name())
        if self.param.optional:
            widget.addItem(self.NOT_SELECTED, None)
        for layer in layers:
            widget.addItem(getExtendedLayerName(layer), layer)
        widget.currentIndexChanged.connect(lambda: self.widgetValueHasChanged.emit(self))
        widget.name = self.param.name

    def refresh(self):
        self._populate(self.widget)

    def setValue(self, value):
        if self.dialogType == DIALOG_STANDARD:
            pass  # TODO
        elif self.dialogType == DIALOG_BATCH:
            self.widget.setText(value)
        else:
            self.setComboValue(value)


    def value(self):
        if self.dialogType == DIALOG_STANDARD:
            try:
                return self.widget.itemData(self.widget.currentIndex())
            except:
                return self.widget.getValue()
        elif self.dialogType == DIALOG_BATCH:
            return self.widget.getText()
        else:
            def validator(v):
                return bool(v) or self.param.optional
            return self.comboValue(validator)

class StringWidgetWrapper(WidgetWrapper):

    def createWidget(self):
        if self.dialogType == DIALOG_STANDARD:
            if self.param.multiline:
                widget = QPlainTextEdit()
                if self.param.default:
                    widget.setPlainText(self.param.default)
            else:
                widget = StringInputPanel(self.param)
                if self.param.default:
                    widget.setValue(self.param.default)
        elif self.dialogType == DIALOG_BATCH:
                widget = QLineEdit()
                if self.param.default:
                    widget.setText(self.param.default)
        else:
            strings = self.dialog.getAvailableValuesOfType(ParameterString, OutputString)
            options = [(self.dialog.resolveValueDescription(s), s) for s in strings]
            if self.param.multiline:
                widget = MultilineTextPanel(options)
                widget.setText(self.param.default or "")
            else:
                widget = QComboBox()
                widget.setEditable(True)
                for desc, val in options:
                    widget.addItem(desc, val)
                widget.setEditText(self.param.default or "")
        return widget

    def setValue(self, value):
        if self.dialogType == DIALOG_STANDARD:
            pass  # TODO
        elif self.dialogType == DIALOG_BATCH:
            self.widget.setText(value)
        else:
            if self.param.multiline:
                self.widget.setValue(value)
            else:
                self.setComboValue(value)

    def value(self):
        if self.dialogType in DIALOG_STANDARD:
            if self.param.multiline:
                text = self.widget.toPlainText()
            else:
                text = self.widget.getValue()
            return text
        if self.dialogType == DIALOG_BATCH:
            text = self.widget.text()
        else:
            if self.param.multiline:
                value = self.widget.getValue()
                option = self.widget.getOption()
                if option == MultilineTextPanel.USE_TEXT:
                    if value == '':
                        if self.param.optional:
                            return None
                        else:
                            raise InvalidParameterValue()
                    else:
                        return value
                else:
                    return value
            else:
                def validator(v):
                    return bool(v) or self.param.optional
                return self.comboValue(validator)


class TableWidgetWrapper(WidgetWrapper):

    NOT_SELECTED = '[Not selected]'

    def createWidget(self):
        if self.dialogType == DIALOG_STANDARD:
            widget = QComboBox()
            layers = dataobjects.getTables()
            layers.sort(key=lambda lay: lay.name())
            if self.param.optional:
                widget.addItem(self.NOT_SELECTED, None)
            for layer in layers:
                widget.addItem(layer.name(), layer)
            widget.currentIndexChanged.connect(lambda: self.widgetValueHasChanged.emit(self))
            widget.name = self.param.name
            return widget
        elif self.dialogType == DIALOG_BATCH:
            return BatchInputSelectionPanel(self.param, self.row, self.col, self.dialog)
        else:
            widget = QComboBox()
            tables = self.dialog.getAvailableValuesOfType(ParameterTable, OutputTable)
            layers = self.dialog.getAvailableValuesOfType(ParameterVector, OutputVector)
            if self.param.optional:
                widget.addItem(self.NOT_SELECTED, None)
            for table in tables:
                widget.addItem(self.dialog.resolveValueDescription(table), table)
            for layer in layers:
                widget.addItem(self.dialog.resolveValueDescription(layer), layer)
            return widget

    def setValue(self, value):
        if self.dialogType == DIALOG_STANDARD:
            pass  # TODO
        elif self.dialogType == DIALOG_BATCH:
            return self.widget.setText(value)
        else:
            self.setComboValue(value)


    def value(self):
        if self.dialogType == DIALOG_STANDARD:
            try:
                return self.widget.itemData(self.widget.currentIndex())
            except:
                return self.widget.getValue()
        elif self.dialogType == DIALOG_BATCH:
            return self.widget.getText()
        else:
            def validator(v):
                return bool(v) or self.param.optional
            return self.comboValue(validator)

class TableFieldWidgetWrapper(WidgetWrapper):

    NOT_SET = '[Not set]'

    def createWidget(self):
        if self.param.multiple:
            if self.dialogType == DIALOG_STANDARD:
                    return MultipleInputPanel(options=[])
            else:
                return QLineEdit()
        else:
            if self.dialogType == DIALOG_STANDARD:
                widget = QComboBox()
                return widget
            elif self.dialogType == DIALOG_BATCH:
                item = QLineEdit()
                if self.param.default is not None:
                    item.setText(self.param.default)
            else:
                widget = QComboBox()
                widget.setEditable(True)
                fields = self.dialog.getAvailableValuesOfType(ParameterTableField, None)
                if self.param.optional:
                    widget.addItem(self.NOT_SET, None)
                for f in fields:
                    widget.addItem(self.dialog.resolveValueDescription(f), f)
                return widget

    def postInitialize(self, wrappers):
        for wrapper in wrappers:
            if wrapper.param.name == self.param.parent:
                layer = wrapper.widget.itemData(wrapper.widget.currentIndex())
                if layer is not None:
                    fields = self.getFields(layer, wrapper.param.datatype)
                    if self.param.multiple:
                        self.widget.updateForOptions(fields)
                    else:
                        self.widget.clear()
                        if self.param.optional:
                            self.widget.addItem(self.tr(self.NOT_SET))
                        self.widget.addItems(fields)
                break

    def getFields(self, layer, datatype):
        fieldTypes = []
        if datatype == ParameterTableField.DATA_TYPE_STRING:
            fieldTypes = [QVariant.String]
        elif datatype == ParameterTableField.DATA_TYPE_NUMBER:
            fieldTypes = [QVariant.Int, QVariant.Double, QVariant.LongLong,
                          QVariant.UInt, QVariant.ULongLong]

        fieldNames = set()
        for field in layer.fields():
            if not fieldTypes or field.type() in fieldTypes:
                fieldNames.add(unicode(field.name()))
        return sorted(list(fieldNames), cmp=locale.strcoll)

    def setValue(self, value):
        if self.param.multiple:
            if self.dialogType == DIALOG_STANDARD:
                options = self.widget.options
                selected = []
                for i, opt in enumerate(options):
                    if opt in value:
                        selected.append(i)
                self.widget.setSelectedItems(selected)
            else:
                self.widget.setText(value)
        else:
            if self.dialogType == DIALOG_STANDARD:
                pass  # TODO
            elif self.dialogType == DIALOG_BATCH:
                return self.widget.setText(value)
            else:
                self.setComboValue(value)


    def value(self):
        if self.param.multiple:
            if self.dialogType == DIALOG_STANDARD:
                return  [self.widget.options[i] for i in self.widget.selectedoptions]
            elif self.dialogType == DIALOG_BATCH:
                return self.widget.text()
            else:
                text = self.widget.text()
                if not bool(text) and not self.param.optional:
                    raise InvalidParameterValue()
                return text
        else:
            if self.dialogType == DIALOG_STANDARD:
                if self.param.optional and self.widget.currentIndex() == 0:
                    return None
                return self.widget.currentText()
            elif self.dialogType == DIALOG_BATCH:
                return self.widget.text()
            else:
                def validator(v):
                    return bool(v) or self.param.optional
                return self.comboValue(validator)

    def anotherParameterWidgetHasChanged(self, wrapper):
        if wrapper.param.name == self.param.parent:
            layer = wrapper.value()
            if layer is not None:
                fields = self.getFields(layer, wrapper.param.datatype)
                if self.param.multiple:
                    self.widget.updateForOptions(fields)
                else:
                    self.widget.clear()
                    if self.param.optional:
                        self.widget.addItem(self.tr(self.NOT_SET))
                    self.widget.addItems(fields)


def GeometryPredicateWidgetWrapper(WidgetWrapper):

    def createWidget(self):
        return GeometryPredicateSelectionPanel()

    def setValue(self, value):
        self.widget.setValue(value)

    def value(self):
        return self.widget.value()
