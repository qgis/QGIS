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

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt import uic
from qgis.PyQt.QtCore import pyqtSlot
from qgis.PyQt.QtWidgets import (QTreeWidgetItem,
                                 QComboBox
                                 )
from qgis.core import (QgsApplication,
                       QgsMapLayer,
                       QgsMapLayerProxyModel,
                       QgsWkbTypes,
                       QgsProcessingUtils,
                       QgsProcessingParameterDefinition
                       )
from qgis.core import QgsFieldProxyModel
from qgis.analysis import QgsInterpolator

from processing.gui.wrappers import WidgetWrapper
from processing.tools import dataobjects

pluginPath = os.path.dirname(__file__)


class ParameterInterpolationData(QgsProcessingParameterDefinition):

    def __init__(self, name='', description=''):
        super().__init__(name, description)
        self.setMetadata({
            'widget_wrapper': 'processing.algs.qgis.ui.InterpolationDataWidget.InterpolationDataWidgetWrapper'
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

    hasChanged = pyqtSlot()

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
        rows = value.split(';')
        for i, r in enumerate(rows):
            v = r.split('::~::')
            self.addLayerData(v[0], v[1])

            comboBox = self.layersTree.itemWidget(self.layersTree.topLevelItem(i), 2)
            comboBox.setCurrentIndex(comboBox.findText(v[3]))
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

            layers += '{}::~::{:d}::~::{:d}::~::{:d};'.format(layer.source(),
                                                              interpolationSource,
                                                              fieldIndex,
                                                              inputType)
        return layers[:-1]


class InterpolationDataWidgetWrapper(WidgetWrapper):

    def createWidget(self):
        widget = InterpolationDataWidget()
        widget.hasChanged.connect(lambda: self.widgetValueHasChanged.emit(self))
        return widget

    def setValue(self, value):
        self.widget.setValue(value)

    def value(self):
        return self.widget.value()
