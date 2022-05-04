# -*- coding: utf-8 -*-

"""
***************************************************************************
    InterpolationDataWidget.py
    ---------------------
    Date                 : December 2016
    Copyright            : (C) 2016 by Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Alexander Bruy'
__date__ = 'December 2016'
__copyright__ = '(C) 2016, Alexander Bruy'

import os

from qgis.PyQt import uic
from qgis.PyQt.QtCore import pyqtSignal
from qgis.PyQt.QtWidgets import (QTreeWidgetItem,
                                 QComboBox)
from qgis.core import (QgsApplication,
                       QgsMapLayerProxyModel,
                       QgsWkbTypes,
                       QgsRectangle,
                       QgsReferencedRectangle,
                       QgsCoordinateReferenceSystem,
                       QgsProcessingUtils,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterDefinition,
                       QgsFieldProxyModel)
from qgis.gui import QgsDoubleSpinBox
from qgis.analysis import QgsInterpolator

from processing.gui.wrappers import WidgetWrapper, DIALOG_STANDARD
from processing.tools import dataobjects

pluginPath = os.path.dirname(__file__)


class ParameterInterpolationData(QgsProcessingParameterDefinition):

    def __init__(self, name='', description=''):
        super().__init__(name, description)
        self.setMetadata({
            'widget_wrapper': 'processing.algs.qgis.ui.InterpolationWidgets.InterpolationDataWidgetWrapper'
        })

    def type(self):
        return 'idw_interpolation_data'

    def clone(self):
        return ParameterInterpolationData(self.name(), self.description())

    @staticmethod
    def parseValue(value):
        if value is None:
            return None

        if value == '':
            return None

        if isinstance(value, str):
            return value if value != '' else None
        else:
            return ParameterInterpolationData.dataToString(value)

    @staticmethod
    def dataToString(data):
        s = ''
        for c in data:
            s += '{}::~::{}::~::{:d}::~::{:d};'.format(c[0],
                                                       c[1],
                                                       c[2],
                                                       c[3])
        return s[:-1]


WIDGET, BASE = uic.loadUiType(os.path.join(pluginPath, 'interpolationdatawidgetbase.ui'))


class InterpolationDataWidget(BASE, WIDGET):
    hasChanged = pyqtSignal()

    def __init__(self):
        super(InterpolationDataWidget, self).__init__(None)
        self.setupUi(self)

        self.btnAdd.setIcon(QgsApplication.getThemeIcon('/symbologyAdd.svg'))
        self.btnRemove.setIcon(QgsApplication.getThemeIcon('/symbologyRemove.svg'))

        self.btnAdd.clicked.connect(self.addLayer)
        self.btnRemove.clicked.connect(self.removeLayer)

        self.cmbLayers.layerChanged.connect(self.layerChanged)
        self.cmbLayers.setFilters(QgsMapLayerProxyModel.VectorLayer)
        self.cmbFields.setFilters(QgsFieldProxyModel.Numeric)
        self.cmbFields.setLayer(self.cmbLayers.currentLayer())

    def addLayer(self):
        layer = self.cmbLayers.currentLayer()

        attribute = ''
        if self.chkUseZCoordinate.isChecked():
            attribute = 'Z_COORD'
        else:
            attribute = self.cmbFields.currentField()

        self._addLayerData(layer.name(), attribute)
        self.hasChanged.emit()

    def removeLayer(self):
        item = self.layersTree.currentItem()
        if not item:
            return
        self.layersTree.invisibleRootItem().removeChild(item)
        self.hasChanged.emit()

    def layerChanged(self, layer):
        self.chkUseZCoordinate.setEnabled(False)
        self.chkUseZCoordinate.setChecked(False)

        if layer is None or not layer.isValid():
            return

        provider = layer.dataProvider()
        if not provider:
            return

        if QgsWkbTypes.hasZ(provider.wkbType()):
            self.chkUseZCoordinate.setEnabled(True)

        self.cmbFields.setLayer(layer)

    def _addLayerData(self, layerName, attribute):
        item = QTreeWidgetItem()
        item.setText(0, layerName)
        item.setText(1, attribute)
        self.layersTree.addTopLevelItem(item)

        comboBox = QComboBox()
        comboBox.addItem(self.tr('Points'))
        comboBox.addItem(self.tr('Structure lines'))
        comboBox.addItem(self.tr('Break lines'))
        comboBox.setCurrentIndex(0)
        self.layersTree.setItemWidget(item, 2, comboBox)

    def setValue(self, value):
        self.layersTree.clear()
        rows = value.split('::|::')
        for i, r in enumerate(rows):
            v = r.split('::~::')
            layer = QgsProcessingUtils.mapLayerFromString(v[0], dataobjects.createContext())
            field_index = int(v[2])

            if field_index == -1:
                field_name = 'Z_COORD'
            else:
                field_name = layer.fields().at(field_index).name()

            self._addLayerData(v[0], field_name)

            comboBox = self.layersTree.itemWidget(self.layersTree.topLevelItem(i), 2)
            comboBox.setCurrentIndex(comboBox.findText((int(v[3]))))

        self.hasChanged.emit()

    def value(self):
        layers = ''
        context = dataobjects.createContext()
        for i in range(self.layersTree.topLevelItemCount()):
            item = self.layersTree.topLevelItem(i)
            if item:
                layerName = item.text(0)
                layer = QgsProcessingUtils.mapLayerFromString(layerName, context)
                if not layer:
                    continue

                provider = layer.dataProvider()
                if not provider:
                    continue

                interpolationAttribute = item.text(1)
                interpolationSource = QgsInterpolator.ValueAttribute
                if interpolationAttribute == 'Z_COORD':
                    interpolationSource = QgsInterpolator.ValueZ
                    fieldIndex = -1
                else:
                    fieldIndex = layer.fields().indexFromName(interpolationAttribute)

                comboBox = self.layersTree.itemWidget(self.layersTree.topLevelItem(i), 2)
                inputTypeName = comboBox.currentText()
                if inputTypeName == self.tr('Points'):
                    inputType = QgsInterpolator.SourcePoints
                elif inputTypeName == self.tr('Structure lines'):
                    inputType = QgsInterpolator.SourceStructureLines
                else:
                    inputType = QgsInterpolator.SourceBreakLines

                layers += '{}::~::{:d}::~::{:d}::~::{:d}::|::'.format(layer.source(),
                                                                      interpolationSource,
                                                                      fieldIndex,
                                                                      inputType)
        return layers[:-len('::|::')]


class InterpolationDataWidgetWrapper(WidgetWrapper):

    def createWidget(self):
        widget = InterpolationDataWidget()
        widget.hasChanged.connect(lambda: self.widgetValueHasChanged.emit(self))
        return widget

    def setValue(self, value):
        self.widget.setValue(value)

    def value(self):
        return self.widget.value()


class ParameterPixelSize(QgsProcessingParameterNumber):

    def __init__(self, name='', description='', layersData=None, extent=None, minValue=None, default=None, optional=False):
        QgsProcessingParameterNumber.__init__(self, name, description, QgsProcessingParameterNumber.Double, default, optional, minValue)
        self.setMetadata({
            'widget_wrapper': 'processing.algs.qgis.ui.InterpolationWidgets.PixelSizeWidgetWrapper'
        })

        self.layersData = layersData
        self.extent = extent
        self.layers = []

    def clone(self):
        copy = ParameterPixelSize(self.name(), self.description(), self.layersData, self.extent, self.minimum(), self.defaultValue(), self.flags() & QgsProcessingParameterDefinition.FlagOptional)
        return copy


WIDGET, BASE = uic.loadUiType(os.path.join(pluginPath, 'RasterResolutionWidget.ui'))


class PixelSizeWidget(BASE, WIDGET):

    def __init__(self):
        super(PixelSizeWidget, self).__init__(None)
        self.setupUi(self)
        self.context = dataobjects.createContext()

        self.extent = QgsRectangle()
        self.layers = []

        self.mCellXSpinBox.setShowClearButton(False)
        self.mCellYSpinBox.setShowClearButton(False)
        self.mRowsSpinBox.setShowClearButton(False)
        self.mColumnsSpinBox.setShowClearButton(False)

        self.mCellYSpinBox.valueChanged.connect(self.mCellXSpinBox.setValue)
        self.mCellXSpinBox.valueChanged.connect(self.pixelSizeChanged)
        self.mRowsSpinBox.valueChanged.connect(self.rowsChanged)
        self.mColumnsSpinBox.valueChanged.connect(self.columnsChanged)

    def setLayers(self, layersData):
        self.extent = QgsRectangle()
        self.layers = []
        for row in layersData.split(';'):
            v = row.split('::~::')
            # need to keep a reference until interpolation is complete
            layer = QgsProcessingUtils.variantToSource(v[0], self.context)
            if layer:
                self.layers.append(layer)
                bbox = layer.sourceExtent()
                if self.extent.isEmpty():
                    self.extent = bbox
                else:
                    self.extent.combineExtentWith(bbox)

        self.pixelSizeChanged()

    def setExtent(self, extent):
        if extent is not None:
            tokens = extent.split(' ')[0].split(',')
            ext = QgsRectangle(float(tokens[0]), float(tokens[2]), float(tokens[1]), float(tokens[3]))
            if len(tokens) > 1:
                self.extent = QgsReferencedRectangle(ext, QgsCoordinateReferenceSystem(tokens[1][1:-1]))
            else:
                self.extent = ext
        self.pixelSizeChanged()

    def pixelSizeChanged(self):
        cell_size = self.mCellXSpinBox.value()
        if cell_size <= 0:
            return

        self.mCellYSpinBox.blockSignals(True)
        self.mCellYSpinBox.setValue(cell_size)
        self.mCellYSpinBox.blockSignals(False)
        rows = max(round(self.extent.height() / cell_size) + 1, 1)
        cols = max(round(self.extent.width() / cell_size) + 1, 1)
        self.mRowsSpinBox.blockSignals(True)
        self.mRowsSpinBox.setValue(rows)
        self.mRowsSpinBox.blockSignals(False)
        self.mColumnsSpinBox.blockSignals(True)
        self.mColumnsSpinBox.setValue(cols)
        self.mColumnsSpinBox.blockSignals(False)

    def rowsChanged(self):
        rows = self.mRowsSpinBox.value()
        if rows <= 0:
            return
        cell_size = self.extent.height() / rows
        cols = max(round(self.extent.width() / cell_size) + 1, 1)
        self.mColumnsSpinBox.blockSignals(True)
        self.mColumnsSpinBox.setValue(cols)
        self.mColumnsSpinBox.blockSignals(False)
        for w in [self.mCellXSpinBox, self.mCellYSpinBox]:
            w.blockSignals(True)
            w.setValue(cell_size)
            w.blockSignals(False)

    def columnsChanged(self):
        cols = self.mColumnsSpinBox.value()
        if cols < 2:
            return
        cell_size = self.extent.width() / (cols - 1)
        rows = max(round(self.extent.height() / cell_size), 1)
        self.mRowsSpinBox.blockSignals(True)
        self.mRowsSpinBox.setValue(rows)
        self.mRowsSpinBox.blockSignals(False)
        for w in [self.mCellXSpinBox, self.mCellYSpinBox]:
            w.blockSignals(True)
            w.setValue(cell_size)
            w.blockSignals(False)

    def setValue(self, value):
        try:
            numeric_value = float(value)
        except:
            return False

        self.mCellXSpinBox.setValue(numeric_value)
        self.mCellYSpinBox.setValue(numeric_value)
        return True

    def value(self):
        return self.mCellXSpinBox.value()


class PixelSizeWidgetWrapper(WidgetWrapper):

    def __init__(self, param, dialog, row=0, col=0, **kwargs):
        super().__init__(param, dialog, row, col, **kwargs)
        self.context = dataobjects.createContext()

    def _panel(self):
        return PixelSizeWidget()

    def createWidget(self):
        if self.dialogType == DIALOG_STANDARD:
            return self._panel()
        else:
            w = QgsDoubleSpinBox()
            w.setShowClearButton(False)
            w.setMinimum(0)
            w.setMaximum(99999999999)
            w.setDecimals(6)
            w.setToolTip(self.tr('Resolution of each pixel in output raster, in layer units'))
            return w

    def postInitialize(self, wrappers):
        if self.dialogType != DIALOG_STANDARD:
            return

        for wrapper in wrappers:
            if wrapper.parameterDefinition().name() == self.param.layersData:
                self.setLayers(wrapper.parameterValue())
                wrapper.widgetValueHasChanged.connect(self.layersChanged)
            elif wrapper.parameterDefinition().name() == self.param.extent:
                self.setExtent(wrapper.parameterValue())
                wrapper.widgetValueHasChanged.connect(self.extentChanged)

    def layersChanged(self, wrapper):
        self.setLayers(wrapper.parameterValue())

    def setLayers(self, layersData):
        self.widget.setLayers(layersData)

    def extentChanged(self, wrapper):
        self.setExtent(wrapper.parameterValue())

    def setExtent(self, extent):
        self.widget.setExtent(extent)

    def setValue(self, value):
        return self.widget.setValue(value)

    def value(self):
        return self.widget.value()
