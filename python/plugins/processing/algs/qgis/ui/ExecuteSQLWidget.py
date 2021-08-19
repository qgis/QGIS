# -*- coding: utf-8 -*-

"""
***************************************************************************
    ExecuteSQLWidget.py
    ---------------------
    Date                 : November 2017
    Copyright            : (C) 2017 by Paul Blottiere
    Email                : blottiere dot paul at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Paul Blottiere'
__date__ = 'November 2018'
__copyright__ = '(C) 2018, Paul Blottiere'

import os

from qgis.PyQt import uic

from qgis.core import (QgsExpressionContextScope,
                       QgsProcessingParameterString,
                       QgsProcessingParameterNumber,
                       QgsExpression,
                       QgsProcessingModelChildParameterSource)

from qgis.gui import QgsFieldExpressionWidget

from processing.gui.wrappers import (WidgetWrapper,
                                     dialogTypes,
                                     DIALOG_MODELER)

pluginPath = os.path.dirname(__file__)
WIDGET, BASE = uic.loadUiType(os.path.join(pluginPath, 'ExecuteSQLWidgetBase.ui'))


class ExecuteSQLWidget(BASE, WIDGET):

    def __init__(self, dialog):
        super(ExecuteSQLWidget, self).__init__(None)
        self.setupUi(self)
        self.dialog = dialog
        self.dialogType = dialogTypes[dialog.__class__.__name__]

        self.mExpressionWidget = QgsFieldExpressionWidget()

        # add model parameters in context scope if called from modeler
        if self.dialogType == DIALOG_MODELER:
            strings = dialog.getAvailableValuesOfType(
                [QgsProcessingParameterString, QgsProcessingParameterNumber], [])
            model_params = [dialog.resolveValueDescription(s) for s in strings]

            scope = QgsExpressionContextScope()
            for param in model_params:
                var = QgsExpressionContextScope.StaticVariable(param)
                scope.addVariable(var)

            self.mExpressionWidget.appendScope(scope)

        self.mHLayout.insertWidget(0, self.mExpressionWidget)

        self.mInsert.clicked.connect(self.insert)

    def insert(self):
        if self.mExpressionWidget.currentText():
            exp = '[%{}%]'.format(self.mExpressionWidget.currentText())
            self.mText.insertPlainText(exp)

    def setValue(self, value):
        text = value

        if self.dialogType == DIALOG_MODELER:
            if isinstance(value, list):
                for v in value:
                    if isinstance(v, QgsProcessingModelChildParameterSource) \
                            and v.source() == QgsProcessingModelChildParameterSource.ExpressionText:
                        text = v.expressionText()

                        # replace parameter's name by expression (diverging after model save)
                        names = QgsExpression.referencedVariables(text)

                        strings = self.dialog.getAvailableValuesOfType(
                            [QgsProcessingParameterString, QgsProcessingParameterNumber], [])
                        model_params = [(self.dialog.resolveValueDescription(s), s) for s in strings]

                        for k, v in model_params:
                            if v.parameterName() in names:
                                text = text.replace('[% @{} %]'.format(v.parameterName()), '[% @{} %]'.format(k))

        self.mText.setPlainText(text)

    def value(self):
        value = self.mText.toPlainText()

        if self.dialogType == DIALOG_MODELER:
            expression_values = self._expressionValues(value)
            if len(expression_values) > 1:
                value = expression_values

        return value

    def _expressionValues(self, text):
        strings = self.dialog.getAvailableValuesOfType(
            [QgsProcessingParameterString, QgsProcessingParameterNumber], [])
        model_params = [(self.dialog.resolveValueDescription(s), s) for s in strings]

        variables = QgsExpression.referencedVariables(text)

        # replace description by parameter's name (diverging after model save)
        descriptions = QgsExpression.referencedVariables(text)

        for k, v in model_params:
            if k in descriptions:
                text = text.replace('[% @{} %]'.format(k), '[% @{} %]'.format(v.parameterName()))

        src = QgsProcessingModelChildParameterSource.fromExpressionText(text)

        # add parameters currently used by the expression
        expression_values = [src]

        for k, v in model_params:
            if k in variables:
                expression_values.append(v)

        return expression_values


class ExecuteSQLWidgetWrapper(WidgetWrapper):

    def createWidget(self):
        return ExecuteSQLWidget(self.dialog)

    def setValue(self, value):
        self.widget.setValue(value)

    def value(self):
        return self.widget.value()
