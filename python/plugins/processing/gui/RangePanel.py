"""
***************************************************************************
    RangePanel.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Victor Olaya"
__date__ = "August 2012"
__copyright__ = "(C) 2012, Victor Olaya"

import os
import warnings

from qgis.PyQt import uic
from qgis.PyQt.QtCore import pyqtSignal
from qgis.PyQt.QtWidgets import QDialog

from qgis.core import QgsProcessingParameterNumber

pluginPath = os.path.split(os.path.dirname(__file__))[0]

with warnings.catch_warnings():
    warnings.filterwarnings("ignore", category=DeprecationWarning)
    WIDGET, BASE = uic.loadUiType(
        os.path.join(pluginPath, "ui", "widgetRangeSelector.ui")
    )


class RangePanel(BASE, WIDGET):
    hasChanged = pyqtSignal()

    def __init__(self, param):
        super().__init__(None)
        self.setupUi(self)

        self.param = param
        # Integer or Double range
        if self.param.dataType() == QgsProcessingParameterNumber.Type.Integer:
            self.spnMin.setDecimals(0)
            self.spnMax.setDecimals(0)

        if param.defaultValue() is not None:
            self.setValue(param.defaultValue())
            values = self.getValues()

        # Spin range logic
        self.spnMin.valueChanged.connect(lambda: self.setMinMax())
        self.spnMax.valueChanged.connect(lambda: self.setMaxMin())

    def setMinMax(self):
        values = self.getValues()
        if values[0] >= values[1]:
            self.spnMax.setValue(values[0])
            self.hasChanged.emit()

    def setMaxMin(self):
        values = self.getValues()
        if values[0] >= values[1]:
            self.spnMin.setValue(values[1])
            self.hasChanged.emit()

    def getValue(self):
        return f"{self.spnMin.value()},{self.spnMax.value()}"

    def getValues(self):
        value = self.getValue()
        if value:
            return [float(a) for a in value.split(",")]

    def setValue(self, value):
        try:
            values = value.split(",")
            minVal = float(values[0])
            maxVal = float(values[1])
            self.spnMin.setValue(float(minVal))
            self.spnMax.setValue(float(maxVal))
        except:
            return
