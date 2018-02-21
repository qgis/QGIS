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
import math
import sip

from qgis.PyQt import uic
from qgis.PyQt.QtCore import pyqtSignal
from qgis.PyQt.QtWidgets import QDialog

from qgis.core import (QgsExpression,
                       QgsProperty,
                       QgsProcessingParameterNumber,
                       QgsProcessingOutputNumber,
                       QgsProcessingParameterDefinition,
                       QgsProcessingModelChildParameterSource,
                       QgsProcessingFeatureSourceDefinition,
                       QgsProcessingUtils)
from qgis.gui import QgsExpressionBuilderDialog
from processing.tools.dataobjects import createExpressionContext, createContext

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
        context = createExpressionContext()
        processing_context = createContext()
        scope = self.modelParametersDialog.model.createExpressionContextScopeForChildAlgorithm(self.modelParametersDialog.childId, processing_context)
        context.appendScope(scope)

        highlighted = scope.variableNames()
        context.setHighlightedVariables(highlighted)

        dlg = QgsExpressionBuilderDialog(None, str(self.leText.text()), self, 'generic', context)

        dlg.setWindowTitle(self.tr('Expression Based Input'))
        if dlg.exec_() == QDialog.Accepted:
            exp = QgsExpression(dlg.expressionText())
            if not exp.hasParserError():
                self.setValue(dlg.expressionText())

    def getValue(self):
        value = self.leText.text()
        for param in self.modelParametersDialog.model.parameterDefinitions():
            if isinstance(param, QgsProcessingParameterNumber):
                if "@" + param.name() == value.strip():
                    return QgsProcessingModelChildParameterSource.fromModelParameter(param.name())

        for alg in list(self.modelParametersDialog.model.childAlgorithms().values()):
            for out in alg.algorithm().outputDefinitions():
                if isinstance(out, QgsProcessingOutputNumber) and "@%s_%s" % (alg.childId(), out.name()) == value.strip():
                    return QgsProcessingModelChildParameterSource.fromChildOutput(alg.childId(), out.outputName())

        try:
            return float(value.strip())
        except:
            return QgsProcessingModelChildParameterSource.fromExpression(self.leText.text())

    def setValue(self, value):
        if isinstance(value, QgsProcessingModelChildParameterSource):
            if value.source() == QgsProcessingModelChildParameterSource.ModelParameter:
                self.leText.setText('@' + value.parameterName())
            elif value.source() == QgsProcessingModelChildParameterSource.ChildOutput:
                name = "%s_%s" % (value.outputChildId(), value.outputName())
                self.leText.setText(name)
            elif value.source() == QgsProcessingModelChildParameterSource.Expression:
                self.leText.setText(value.expression())
            else:
                self.leText.setText(str(value.staticValue()))
        else:
            self.leText.setText(str(value))


class NumberInputPanel(NUMBER_BASE, NUMBER_WIDGET):

    """
    Number input panel for use outside the modeller - this input panel
    contains a user friendly spin box for entering values.
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

        self.allowing_null = False
        # set default value
        if param.flags() & QgsProcessingParameterDefinition.FlagOptional:
            self.spnValue.setShowClearButton(True)
            min = self.spnValue.minimum() - 1
            self.spnValue.setMinimum(min)
            self.spnValue.setValue(min)
            self.spnValue.setSpecialValueText(self.tr('Not set'))
            self.allowing_null = True

        if param.defaultValue() is not None:
            self.setValue(param.defaultValue())
            if not self.allowing_null:
                try:
                    self.spnValue.setClearValue(float(param.defaultValue()))
                except:
                    pass
        elif self.param.minimum() is not None:
            try:
                self.setValue(float(self.param.minimum()))
                if not self.allowing_null:
                    self.spnValue.setClearValue(float(self.param.minimum()))
            except:
                pass
        elif not self.allowing_null:
            self.setValue(0)
            self.spnValue.setClearValue(0)

        # we don't show the expression button outside of modeler
        self.layout().removeWidget(self.btnSelect)
        sip.delete(self.btnSelect)
        self.btnSelect = None

        if not self.param.isDynamic():
            # only show data defined button for dynamic properties
            self.layout().removeWidget(self.btnDataDefined)
            sip.delete(self.btnDataDefined)
            self.btnDataDefined = None
        else:
            self.btnDataDefined.init(0, QgsProperty(), self.param.dynamicPropertyDefinition())
            self.btnDataDefined.registerEnabledWidget(self.spnValue, False)

        self.spnValue.valueChanged.connect(lambda: self.hasChanged.emit())

    def setDynamicLayer(self, layer):
        context = createContext()
        try:
            if isinstance(layer, QgsProcessingFeatureSourceDefinition):
                layer, ok = layer.source.valueAsString(context.expressionContext())
            if isinstance(layer, str):
                layer = QgsProcessingUtils.mapLayerFromString(layer, context)
            self.btnDataDefined.setVectorLayer(layer)
        except:
            pass

    def getValue(self):
        if self.btnDataDefined is not None and self.btnDataDefined.isActive():
            return self.btnDataDefined.toProperty()
        elif self.allowing_null and self.spnValue.value() == self.spnValue.minimum():
            return None
        else:
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
