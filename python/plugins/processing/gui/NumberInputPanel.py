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
from builtins import str

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import math

from qgis.PyQt import uic
from qgis.PyQt.QtCore import pyqtSignal
from qgis.PyQt.QtWidgets import QDialog

from qgis.core import (QgsExpression,
                       QgsProcessingParameterNumber)
from qgis.gui import QgsExpressionBuilderDialog
from processing.core.parameters import ParameterNumber, ParameterVector, ParameterRaster
from processing.core.outputs import OutputNumber, OutputVector, OutputRaster
from processing.modeler.ModelerAlgorithm import ValueFromInput, ValueFromOutput, CompoundValue

pluginPath = os.path.split(os.path.dirname(__file__))[0]
NUMBER_WIDGET, NUMBER_BASE = uic.loadUiType(
    os.path.join(pluginPath, 'ui', 'widgetNumberSelector.ui'))
WIDGET, BASE = uic.loadUiType(
    os.path.join(pluginPath, 'ui', 'widgetBaseSelector.ui'))


class ModellerNumberInputPanel(BASE, WIDGET):
    """
    Number input panel for use inside the modeller - this input panel
    is based off the base input panel and includes a text based line input
    for entering values. This allows expressions and other non-numeric
    values to be set, which are later evalauted to numbers when the model
    is run.
    """

    hasChanged = pyqtSignal()

    def __init__(self, param, modelParametersDialog):
        super(ModellerNumberInputPanel, self).__init__(None)
        self.setupUi(self)

        self.param = param
        self.modelParametersDialog = modelParametersDialog
        if param.defaultValue():
            self.setValue(param.defaultValue())
        self.btnSelect.clicked.connect(self.showExpressionsBuilder)
        self.leText.textChanged.connect(lambda: self.hasChanged.emit())

    def showExpressionsBuilder(self):
        context = self.param.expressionContext()
        dlg = QgsExpressionBuilderDialog(None, str(self.leText.text()), self, 'generic', context)

        context.popScope()
        values = self.modelParametersDialog.getAvailableValuesOfType(ParameterNumber, OutputNumber)
        variables = {}
        for value in values:
            if isinstance(value, ValueFromInput):
                name = value.name
                element = self.modelParametersDialog.model.inputs[name].param
                desc = element.description
            else:
                name = "%s_%s" % (value.alg, value.output)
                alg = self.modelParametersDialog.model.algs[value.alg]
                out = alg.algorithm.getOutputFromName(value.output)
                desc = self.tr("Output '{0}' from algorithm '{1}'").format(out.description, alg.description)
            variables[name] = desc
        values = self.modelParametersDialog.getAvailableValuesOfType(ParameterVector, OutputVector)
        values.extend(self.modelParametersDialog.getAvailableValuesOfType(ParameterRaster, OutputRaster))
        for value in values:
            if isinstance(value, ValueFromInput):
                name = value.name
                element = self.modelParametersDialog.model.inputs[name].param
                desc = element.description
            else:
                name = "%s_%s" % (value.alg, value.output)
                alg = self.modelParametersDialog.model.algs[value.alg]
                element = alg.algorithm.getOutputFromName(value.output)
                desc = self.tr("Output '{0}' from algorithm '{1}'").format(element.description, alg.description)
            variables['%s_minx' % name] = self.tr("Minimum X of {0}").format(desc)
            variables['%s_miny' % name] = self.tr("Minimum Y of {0}").format(desc)
            variables['%s_maxx' % name] = self.tr("Maximum X of {0}").format(desc)
            variables['%s_maxy' % name] = self.tr("Maximum Y of {0}").format(desc)
            if isinstance(element, (ParameterRaster, OutputRaster)):
                variables['%s_min' % name] = self.tr("Minimum value of {0}").format(desc)
                variables['%s_max' % name] = self.tr("Maximum value of {0}").format(desc)
                variables['%s_avg' % name] = self.tr("Mean value of {0}").format(desc)
                variables['%s_stddev' % name] = self.tr("Standard deviation of {0}").format(desc)
        for variable, desc in variables.items():
            dlg.expressionBuilder().registerItem("Modeler", variable, "@" + variable, desc, highlightedItem=True)

        dlg.setWindowTitle(self.tr('Expression based input'))
        if dlg.exec_() == QDialog.Accepted:
            exp = QgsExpression(dlg.expressionText())
            if not exp.hasParserError():
                self.setValue(dlg.expressionText())

    def getValue(self):
        value = self.leText.text()
        values = []
        for param in self.modelParametersDialog.model.parameters:
            if isinstance(param, ParameterNumber):
                if "@" + param.name in value:
                    values.append(ValueFromInput(param.name))
        for alg in list(self.modelParametersDialog.model.algs.values()):
            for out in alg.algorithm.outputs:
                if isinstance(out, OutputNumber) and "@%s_%s" % (alg.name(), out.name) in value:
                    values.append(ValueFromOutput(alg.name(), out.name))
        if values:
            return CompoundValue(values, value)
        else:
            return value

    def setValue(self, value):
        self.leText.setText(str(value))


class NumberInputPanel(NUMBER_BASE, NUMBER_WIDGET):
    """
    Number input panel for use outside the modeller - this input panel
    contains a user friendly spin box for entering values. It also
    allows expressions to be evaluated, but these expressions are evaluated
    immediately after entry and are not stored anywhere.
    """

    hasChanged = pyqtSignal()

    def __init__(self, param):
        super(NumberInputPanel, self).__init__(None)
        self.setupUi(self)

        self.spnValue.setExpressionsEnabled(True)

        self.param = param
        if self.param.dataType() == QgsProcessingParameterNumber.Integer:
            self.spnValue.setDecimals(0)
        else:
            # Guess reasonable step value
            if self.param.maximum() is not None and self.param.minimum() is not None:
                try:
                    self.spnValue.setSingleStep(self.calculateStep(float(self.param.minimum()), float(self.param.maximum())))
                except:
                    pass

        if self.param.maximum() is not None:
            self.spnValue.setMaximum(self.param.maximum())
        else:
            self.spnValue.setMaximum(999999999)
        if self.param.minimum() is not None:
            self.spnValue.setMinimum(self.param.minimum())
        else:
            self.spnValue.setMinimum(-999999999)

        # set default value
        if param.defaultValue() is not None:
            self.setValue(param.defaultValue())
            try:
                self.spnValue.setClearValue(float(param.defaultValue()))
            except:
                pass
        elif self.param.minimum() is not None:
            try:
                self.setValue(float(self.param.minimum()))
                self.spnValue.setClearValue(float(self.param.minimum()))
            except:
                pass
        else:
            self.setValue(0)
            self.spnValue.setClearValue(0)
        self.btnSelect.setFixedHeight(self.spnValue.height())

        self.btnSelect.clicked.connect(self.showExpressionsBuilder)
        self.spnValue.valueChanged.connect(lambda: self.hasChanged.emit())

    def showExpressionsBuilder(self):
        context = self.param.expressionContext()
        dlg = QgsExpressionBuilderDialog(None, str(self.spnValue.value()), self, 'generic', context)

        dlg.setWindowTitle(self.tr('Expression based input'))
        if dlg.exec_() == QDialog.Accepted:
            exp = QgsExpression(dlg.expressionText())
            if not exp.hasParserError():
                try:
                    val = float(exp.evaluate(context))
                    self.setValue(val)
                except:
                    return

    def getValue(self):
        return self.spnValue.value()

    def setValue(self, value):
        try:
            self.spnValue.setValue(float(value))
        except:
            return

    def calculateStep(self, minimum, maximum):
        value_range = maximum - minimum
        if value_range <= 1.0:
            step = value_range / 10.0
            # round to 1 significant figrue
            return round(step, -int(math.floor(math.log10(step))))
        else:
            return 1.0
