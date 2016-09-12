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

from qgis.core import (QgsDataSourceUri,
                       QgsCredentials,
                       QgsExpression,
                       QgsRasterLayer,
                       QgsExpressionContextScope)
from qgis.gui import QgsEncodingFileDialog, QgsExpressionBuilderDialog
from qgis.utils import iface
from processing.core.parameters import ParameterNumber
from processing.core.outputs import OutputNumber
from processing.modeler.ModelerAlgorithm import ValueFromInput, ValueFromOutput, CompoundValue

pluginPath = os.path.split(os.path.dirname(__file__))[0]
WIDGET, BASE = uic.loadUiType(
    os.path.join(pluginPath, 'ui', 'widgetBaseSelector.ui'))


class NumberInputPanel(BASE, WIDGET):

    hasChanged = pyqtSignal()

    def __init__(self, param, modelParametersDialog=None):
        super(NumberInputPanel, self).__init__(None)
        self.setupUi(self)

        self.param = param
        self.modelParametersDialog = modelParametersDialog
        if param.default:
            self.setValue(param.default)
        self.btnSelect.clicked.connect(self.showExpressionsBuilder)
        self.leText.textChanged.connect(lambda: self.hasChanged.emit())

    def showExpressionsBuilder(self):
        context = self.param.expressionContext()
        if self.modelParametersDialog is not None:
            context.popScope()
            values = self.modelParametersDialog.getAvailableValuesOfType(ParameterNumber, OutputNumber)
            modelerScope = QgsExpressionContextScope()
            for value in values:
                name = value.name if isinstance(value, ValueFromInput) else "%s_%s" % (value.alg, value.output)
                modelerScope.setVariable(name, 1)
            context.appendScope(modelerScope) 
        dlg = QgsExpressionBuilderDialog(None, self.leText.text(), self, 'generic', context)
        dlg.setWindowTitle(self.tr('Expression based input'))
        if dlg.exec_() == QDialog.Accepted:
            exp = QgsExpression(dlg.expressionText())
            if not exp.hasParserError():
                self.setValue(dlg.expressionText())

    
    def getValue(self):
        if self.modelParametersDialog:
            value = self.leText.text()
            values = []
            for param in self.modelParametersDialog.model.parameters:
                if isinstance(param, ParameterNumber):
                    if "@" + param.name in value:
                        values.append(ValueFromInput(param.name))
            for alg in self.modelParametersDialog.model.algs.values():
                for out in alg.algorithm.outputs:
                    if isinstance(out, OutputNumber) and "@%s_%s" % (alg.name, out.name) in value:
                        values.append(ValueFromOutput(alg.name, out.name))
            if values:
                return CompoundValue(values, value)
            else:
                return value
        else:
            return self.leText.text()

    def setValue(self, value):
        self.leText.setText(unicode(value))
