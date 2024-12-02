"""
***************************************************************************
    Heatmap.py
    ---------------------
    Date                 : December 2016
    Copyright            : (C) 2016 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Nyall Dawson"
__date__ = "December 2016"
__copyright__ = "(C) 2016, Nyall Dawson"

from processing.gui.wrappers import WidgetWrapper, DIALOG_STANDARD
from processing.tools import dataobjects

import os
from typing import Optional

from qgis.PyQt import uic
from qgis.gui import QgsDoubleSpinBox
from qgis.core import QgsRectangle, QgsProcessingUtils

pluginPath = os.path.dirname(__file__)
WIDGET, BASE = uic.loadUiType(os.path.join(pluginPath, "RasterResolutionWidget.ui"))


class HeatmapPixelSizeWidget(BASE, WIDGET):

    def __init__(self):
        super().__init__(None)
        self.setupUi(self)

        self.layer_bounds: QgsRectangle = QgsRectangle()
        self.source = None
        self.raster_bounds: QgsRectangle = QgsRectangle()
        self.radius: float = 100
        self.radius_field: Optional[str] = None

        self.mCellXSpinBox.setShowClearButton(False)
        self.mCellYSpinBox.setShowClearButton(False)
        self.mRowsSpinBox.setShowClearButton(False)
        self.mColumnsSpinBox.setShowClearButton(False)

        self.mCellYSpinBox.valueChanged.connect(self.mCellXSpinBox.setValue)
        self.mCellXSpinBox.valueChanged.connect(self.pixelSizeChanged)
        self.mRowsSpinBox.valueChanged.connect(self.rowsChanged)
        self.mColumnsSpinBox.valueChanged.connect(self.columnsChanged)

    def setRadius(self, radius: float):
        self.radius = radius
        self.recalculate_bounds()

    def setRadiusField(self, radius_field: Optional[str]):
        self.radius_field = radius_field
        self.recalculate_bounds()

    def setSource(self, source):
        if not source:
            return
        bounds = source.sourceExtent()
        if bounds.isNull():
            return

        self.source = source
        self.layer_bounds = bounds
        self.recalculate_bounds()

    def recalculate_bounds(self):
        self.raster_bounds = QgsRectangle(self.layer_bounds)

        if not self.source:
            return

        max_radius = self.radius
        if self.radius_field:
            idx = self.source.fields().lookupField(self.radius_field)
            try:
                max_radius = float(self.source.maximumValue(idx))
            except:
                pass

        self.raster_bounds.setXMinimum(self.raster_bounds.xMinimum() - max_radius)
        self.raster_bounds.setYMinimum(self.raster_bounds.yMinimum() - max_radius)
        self.raster_bounds.setXMaximum(self.raster_bounds.xMaximum() + max_radius)
        self.raster_bounds.setYMaximum(self.raster_bounds.yMaximum() + max_radius)

        self.pixelSizeChanged()

    def pixelSizeChanged(self):
        cell_size = self.mCellXSpinBox.value()
        if cell_size <= 0 or self.raster_bounds.isNull():
            return

        self.mCellYSpinBox.blockSignals(True)
        self.mCellYSpinBox.setValue(cell_size)
        self.mCellYSpinBox.blockSignals(False)
        rows = max(round(self.raster_bounds.height() / cell_size) + 1, 1)
        cols = max(round(self.raster_bounds.width() / cell_size) + 1, 1)
        self.mRowsSpinBox.blockSignals(True)
        self.mRowsSpinBox.setValue(rows)
        self.mRowsSpinBox.blockSignals(False)
        self.mColumnsSpinBox.blockSignals(True)
        self.mColumnsSpinBox.setValue(cols)
        self.mColumnsSpinBox.blockSignals(False)

    def rowsChanged(self):
        rows = self.mRowsSpinBox.value()
        if rows <= 0 or self.raster_bounds.isNull():
            return

        cell_size = self.raster_bounds.height() / rows
        if cell_size == 0:
            return

        cols = max(round(self.raster_bounds.width() / cell_size) + 1, 1)
        self.mColumnsSpinBox.blockSignals(True)
        self.mColumnsSpinBox.setValue(cols)
        self.mColumnsSpinBox.blockSignals(False)
        for w in [self.mCellXSpinBox, self.mCellYSpinBox]:
            w.blockSignals(True)
            w.setValue(cell_size)
            w.blockSignals(False)

    def columnsChanged(self):
        cols = self.mColumnsSpinBox.value()
        if cols < 2 or self.raster_bounds.isNull():
            return

        cell_size = self.raster_bounds.width() / (cols - 1)
        if cell_size == 0:
            return

        rows = max(round(self.raster_bounds.height() / cell_size), 1)
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


class HeatmapPixelSizeWidgetWrapper(WidgetWrapper):

    def __init__(self, param, dialog, row=0, col=0, **kwargs):
        super().__init__(param, dialog, row, col, **kwargs)
        self.context = dataobjects.createContext()

    def _panel(self):
        return HeatmapPixelSizeWidget()

    def createWidget(self):
        if self.dialogType == DIALOG_STANDARD:
            return self._panel()
        else:
            w = QgsDoubleSpinBox()
            w.setShowClearButton(False)
            w.setMinimum(0)
            w.setMaximum(99999999999)
            w.setDecimals(6)
            w.setToolTip(
                self.tr("Resolution of each pixel in output raster, in layer units")
            )
            return w

    def postInitialize(self, wrappers):
        if self.dialogType != DIALOG_STANDARD:
            return

        for wrapper in wrappers:
            if wrapper.parameterDefinition().name() == self.param.parent_layer:
                self.setSource(wrapper.parameterValue())
                wrapper.widgetValueHasChanged.connect(self.parentLayerChanged)
            elif wrapper.parameterDefinition().name() == self.param.radius_param:
                self.setRadius(wrapper.parameterValue())
                wrapper.widgetValueHasChanged.connect(self.radiusChanged)
            elif wrapper.parameterDefinition().name() == self.param.radius_field_param:
                self.setSource(wrapper.parameterValue())
                wrapper.widgetValueHasChanged.connect(self.radiusFieldChanged)

    def parentLayerChanged(self, wrapper):
        self.setSource(wrapper.parameterValue())

    def setSource(self, source):
        source = QgsProcessingUtils.variantToSource(source, self.context)
        self.widget.setSource(source)

    def radiusChanged(self, wrapper):
        self.setRadius(wrapper.parameterValue())

    def setRadius(self, radius: float):
        self.widget.setRadius(radius)

    def radiusFieldChanged(self, wrapper):
        self.setRadiusField(wrapper.parameterValue())

    def setRadiusField(self, radius_field: Optional[str]):
        self.widget.setRadiusField(radius_field)

    def setValue(self, value):
        return self.widget.setValue(value)

    def value(self):
        return self.widget.value()
