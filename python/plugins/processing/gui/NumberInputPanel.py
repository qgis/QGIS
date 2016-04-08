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

from PyQt import uic
from PyQt.QtCore import pyqtSignal
from PyQt.QtGui import QDialog

from math import log10, floor
from qgis.core import (QgsDataSourceURI, QgsCredentials,  QgsExpressionContext,
                        QgsExpressionContextUtils, QgsExpression)
from qgis.gui import QgsEncodingFileDialog, QgsExpressionBuilderDialog

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
        context = QgsExpressionContext()
        context.appendScope(QgsExpressionContextUtils.globalScope())
        context.appendScope(QgsExpressionContextUtils.projectScope())
        dlg = QgsExpressionBuilderDialog(None, self.spnValue.text(), self, "generic", context)
        dlg.setWindowTitle(self.tr("Expression based input"));
        if dlg.exec_() == QDialog.Accepted:
            exp = QgsExpression(dlg.expressionText())
            if not exp.hasParserError():
                result =  exp.evaluate(context)
                if not exp.hasEvalError():
                    try:
                        self.spnValue.setValue(float(result))
                    except:
                        pass

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
