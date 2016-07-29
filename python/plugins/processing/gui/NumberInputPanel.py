# -*- coding: utf-8 -*-

"""
***************************************************************************
    NumberInputPanel.py
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

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt import uic
from qgis.PyQt.QtCore import pyqtSignal
from qgis.PyQt.QtWidgets import QDialog

from math import log10, floor
from qgis.core import (QgsDataSourceURI,
                       QgsCredentials,
                       QgsExpressionContext,
                       QgsExpressionContextUtils,
                       QgsExpression,
                       QgsRasterLayer,
                       QgsExpressionContextScope)
from qgis.gui import QgsEncodingFileDialog, QgsExpressionBuilderDialog
from qgis.utils import iface
from processing.tools import dataobjects

pluginPath = os.path.split(os.path.dirname(__file__))[0]
WIDGET, BASE = uic.loadUiType(
    os.path.join(pluginPath, 'ui', 'widgetNumberSelector.ui'))


class NumberInputPanel(BASE, WIDGET):

    hasChanged = pyqtSignal()

    def __init__(self, number, minimum, maximum, isInteger):
        super(NumberInputPanel, self).__init__(None)
        self.setupUi(self)

        self.spnValue.setExpressionsEnabled(True)
        self.isInteger = isInteger
        if self.isInteger:
            self.spnValue.setDecimals(0)
        else:
            #Guess reasonable step value
            if (maximum == 0 or maximum) and (minimum == 0 or minimum):
                self.spnValue.setSingleStep(self.calculateStep(minimum, maximum))

        if maximum == 0 or maximum:
            self.spnValue.setMaximum(maximum)
        else:
            self.spnValue.setMaximum(99999999)
        if minimum == 0 or minimum:
            self.spnValue.setMinimum(minimum)
        else:
            self.spnValue.setMinimum(-99999999)

        #Set default value
        if number == 0 or number:
            self.spnValue.setValue(float(number))
            self.spnValue.setClearValue(float(number))
        elif minimum == 0 or minimum:
            self.spnValue.setValue(float(minimum))
            self.spnValue.setClearValue(float(minimum))
        else:
            self.spnValue.setValue(0)
            self.spnValue.setClearValue(0)

        self.btnCalc.setFixedHeight(self.spnValue.height())

        self.btnCalc.clicked.connect(self.showExpressionsBuilder)

        self.spnValue.valueChanged.connect(lambda: self.hasChanged.emit())

    def showExpressionsBuilder(self):
        context = self.expressionContext()
        dlg = QgsExpressionBuilderDialog(None, self.spnValue.text(), self, 'generic', context)
        dlg.setWindowTitle(self.tr('Expression based input'))
        if dlg.exec_() == QDialog.Accepted:
            exp = QgsExpression(dlg.expressionText())
            if not exp.hasParserError():
                result = exp.evaluate(context)
                if not exp.hasEvalError():
                    try:
                        self.spnValue.setValue(float(result))
                    except:
                        pass

    def expressionContext(self):
        context = QgsExpressionContext()
        context.appendScope(QgsExpressionContextUtils.globalScope())
        context.appendScope(QgsExpressionContextUtils.projectScope())
        processingScope = QgsExpressionContextScope()
        layers = dataobjects.getAllLayers()
        for layer in layers:
            name = layer.name()
            processingScope.setVariable('%s_minx' % name, layer.extent().xMinimum())
            processingScope.setVariable('%s_miny' % name, layer.extent().yMinimum())
            processingScope.setVariable('%s_maxx' % name, layer.extent().xMaximum())
            processingScope.setVariable('%s_maxy' % name, layer.extent().yMaximum())
            if isinstance(layer, QgsRasterLayer):
                cellsize = (layer.extent().xMaximum()
                            - layer.extent().xMinimum()) / layer.width()
                processingScope.setVariable('%s_cellsize' % name, cellsize)

        layers = dataobjects.getRasterLayers()
        for layer in layers:
            for i in range(layer.bandCount()):
                stats = layer.dataProvider().bandStatistics(i + 1)
                processingScope.setVariable('%s_band%i_avg' % (name, i + 1), stats.mean)
                processingScope.setVariable('%s_band%i_stddev' % (name, i + 1), stats.stdDev)
                processingScope.setVariable('%s_band%i_min' % (name, i + 1), stats.minimumValue)
                processingScope.setVariable('%s_band%i_max' % (name, i + 1), stats.maximumValue)

        extent = iface.mapCanvas().extent()
        processingScope.setVariable('canvasextent_minx', extent.xMinimum())
        processingScope.setVariable('canvasextent_miny', extent.yMinimum())
        processingScope.setVariable('canvasextent_maxx', extent.xMaximum())
        processingScope.setVariable('canvasextent_maxy', extent.yMaximum())

        extent = iface.mapCanvas().fullExtent()
        processingScope.setVariable('fullextent_minx', extent.xMinimum())
        processingScope.setVariable('fullextent_miny', extent.yMinimum())
        processingScope.setVariable('fullextent_maxx', extent.xMaximum())
        processingScope.setVariable('fullextent_maxy', extent.yMaximum())
        context.appendScope(processingScope)
        return context

    def getValue(self):
        return self.spnValue.value()

    def calculateStep(self, minimum, maximum):
        valueRange = maximum - minimum
        if valueRange <= 1.0:
            step = valueRange / 10.0
            # round to 1 significant figure
            return round(step, -int(floor(log10(step))))
        else:
            return 1.0
