# -*- coding: utf-8 -*-

"""
***************************************************************************
    RasterCalculatorWidgets.py
    ---------------------
    Date                 : November 2016
    Copyright            : (C) 2016 by Victor Olaya
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
__date__ = 'November 2016'
__copyright__ = '(C) 2016, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
from functools import partial
import re
import json

from qgis.PyQt import uic
from qgis.PyQt.QtGui import QTextCursor
from qgis.PyQt.QtWidgets import (QLineEdit, QPushButton, QLabel,
                                 QComboBox, QSpacerItem, QSizePolicy)

from qgis.core import (QgsProcessingUtils,
                       QgsProcessingParameterDefinition,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingOutputRasterLayer,
                       QgsProject)

from processing.gui.wrappers import WidgetWrapper, DIALOG_STANDARD, DIALOG_BATCH
from processing.gui.BatchInputSelectionPanel import BatchInputSelectionPanel
from processing.tools import dataobjects
from processing.tools.system import userFolder


from processing.gui.wrappers import InvalidParameterValue

pluginPath = os.path.dirname(__file__)
WIDGET_ADD_NEW, BASE_ADD_NEW = uic.loadUiType(
    os.path.join(pluginPath, 'AddNewExpressionDialog.ui'))


class AddNewExpressionDialog(BASE_ADD_NEW, WIDGET_ADD_NEW):

    def __init__(self, expression):
        super(AddNewExpressionDialog, self).__init__()
        self.setupUi(self)

        self.name = None
        self.expression = None
        self.txtExpression.setPlainText(expression)
        self.buttonBox.rejected.connect(self.cancelPressed)
        self.buttonBox.accepted.connect(self.okPressed)

    def cancelPressed(self):
        self.close()

    def okPressed(self):
        self.name = self.txtName.text()
        self.expression = self.txtExpression.toPlainText()
        self.close()


WIDGET_DLG, BASE_DLG = uic.loadUiType(
    os.path.join(pluginPath, 'PredefinedExpressionDialog.ui'))


class PredefinedExpressionDialog(BASE_DLG, WIDGET_DLG):

    def __init__(self, expression, options):
        super(PredefinedExpressionDialog, self).__init__()
        self.setupUi(self)

        self.filledExpression = None
        self.options = options
        self.expression = expression
        self.variables = set(re.findall(r'\[.*?\]', expression))
        self.comboBoxes = {}
        for variable in self.variables:
            label = QLabel(variable[1:-1])
            combo = QComboBox()
            for opt in self.options.keys():
                combo.addItem(opt)
            self.comboBoxes[variable] = combo
            self.groupBox.layout().addWidget(label)
            self.groupBox.layout().addWidget(combo)

        verticalSpacer = QSpacerItem(20, 40, QSizePolicy.Minimum, QSizePolicy.Expanding)
        self.groupBox.layout().addItem(verticalSpacer)

        self.buttonBox.rejected.connect(self.cancelPressed)
        self.buttonBox.accepted.connect(self.okPressed)

    def cancelPressed(self):
        self.close()

    def okPressed(self):
        self.filledExpression = self.expression
        for name, combo in self.comboBoxes.items():
            self.filledExpression = self.filledExpression.replace(name,
                                                                  self.options[combo.currentText()])
        self.close()


WIDGET, BASE = uic.loadUiType(
    os.path.join(pluginPath, 'RasterCalculatorWidget.ui'))


class ExpressionWidget(BASE, WIDGET):

    _expressions = {"NDVI": "([NIR] - [Red]) % ([NIR] + [Red])"}

    def __init__(self, options):
        super(ExpressionWidget, self).__init__(None)
        self.setupUi(self)

        self.setList(options)

        def doubleClicked(item):
            self.text.insertPlainText('"{}"'.format(self.options[item.text()]))

        def addButtonText(text):
            if any(c for c in text if c.islower()):
                self.text.insertPlainText(" {}()".format(text))
                self.text.moveCursor(QTextCursor.PreviousCharacter, QTextCursor.MoveAnchor)
            else:
                self.text.insertPlainText(" {} ".format(text))
        buttons = [b for b in self.buttonsGroupBox.children()if isinstance(b, QPushButton)]
        for button in buttons:
            button.clicked.connect(partial(addButtonText, button.text()))
        self.listWidget.itemDoubleClicked.connect(doubleClicked)

        self.expressions = {}
        if os.path.exists(self.expsFile()):
            with open(self.expsFile()) as f:
                self.expressions.update(json.load(f))
        self.expressions.update(self._expressions)

        self.fillPredefined()
        self.buttonAddPredefined.clicked.connect(self.addPredefined)

        self.buttonSavePredefined.clicked.connect(self.savePredefined)

    def expsFile(self):
        return os.path.join(userFolder(), 'rastercalcexpressions.json')

    def addPredefined(self):
        expression = self.expressions[self.comboPredefined.currentText()]
        dlg = PredefinedExpressionDialog(expression, self.options)
        dlg.exec_()
        if dlg.filledExpression:
            self.text.setPlainText(dlg.filledExpression)

    def savePredefined(self):
        exp = self.text.toPlainText()
        used = [v for v in self.options.values() if v in exp]

        for i, v in enumerate(used):
            exp = exp.replace(v, chr(97 + i))

        dlg = AddNewExpressionDialog(exp)
        dlg.exec_()
        if dlg.name:
            self.expressions[dlg.name] = dlg.expression

        with open(self.expsFile(), "w") as f:
            f.write(json.dumps(self.expressions))

    def fillPredefined(self):
        self.comboPredefined.clear()
        for expression in self.expressions:
            self.comboPredefined.addItem(expression)

    def setList(self, options):
        self.options = options
        self.listWidget.clear()
        for opt in options.keys():
            self.listWidget.addItem(opt)

    def setValue(self, value):
        self.text.setPlainText(value)

    def value(self):
        return self.text.toPlainText()


class ExpressionWidgetWrapper(WidgetWrapper):

    def _panel(self, options):
        return ExpressionWidget(options)

    def createWidget(self):
        if self.dialogType == DIALOG_STANDARD:
            layers = QgsProcessingUtils.compatibleRasterLayers(QgsProject.instance(), False)
            options = {}
            for lyr in layers:
                for n in range(lyr.bandCount()):
                    name = '{:s}@{:d}'.format(lyr.name(), n + 1)
                    options[name] = name
            return self._panel(options)
        elif self.dialogType == DIALOG_BATCH:
            return QLineEdit()
        else:
            layers = self.dialog.getAvailableValuesOfType([QgsProcessingParameterRasterLayer], [QgsProcessingOutputRasterLayer])
            options = {self.dialog.resolveValueDescription(lyr): "{}@1".format(self.dialog.resolveValueDescription(lyr)) for lyr in layers}
            return self._panel(options)

    def refresh(self):
        # TODO: check if avoid code duplication with self.createWidget
        layers = QgsProcessingUtils.compatibleRasterLayers(QgsProject.instance())
        options = {}
        for lyr in layers:
            for n in range(lyr.bandCount()):
                options[lyr.name()] = '{:s}@{:d}'.format(lyr.name(), n + 1)
        self.widget.setList(options)

    def setValue(self, value):
        if self.dialogType == DIALOG_STANDARD:
            pass  # TODO
        elif self.dialogType == DIALOG_BATCH:
            return self.widget.setText(value)
        else:
            self.widget.setValue(value)

    def value(self):
        if self.dialogType in DIALOG_STANDARD:
            return self.widget.value()
        elif self.dialogType == DIALOG_BATCH:
            return self.widget.text()
        else:
            return self.widget.value()


class LayersListWidgetWrapper(WidgetWrapper):

    def createWidget(self):
        if self.dialogType == DIALOG_BATCH:
            widget = BatchInputSelectionPanel(self.param, self.row, self.col, self.dialog)
            widget.valueChanged.connect(lambda: self.widgetValueHasChanged.emit(self))
            return widget
        else:
            return None

    def setValue(self, value):
        if self.dialogType == DIALOG_BATCH:
            return self.widget.setText(value)

    def value(self):
        if self.dialogType == DIALOG_STANDARD:
            if self.param.datatype == dataobjects.TYPE_FILE:
                return self.param.setValue(self.widget.selectedoptions)
            else:
                if self.param.datatype == dataobjects.TYPE_RASTER:
                    options = QgsProcessingUtils.compatibleRasterLayers(QgsProject.instance(), False)
                elif self.param.datatype == dataobjects.TYPE_VECTOR_ANY:
                    options = QgsProcessingUtils.compatibleVectorLayers(QgsProject.instance(), [], False)
                else:
                    options = QgsProcessingUtils.compatibleVectorLayers(QgsProject.instance(), [self.param.datatype], False)
                return [options[i] for i in self.widget.selectedoptions]
        elif self.dialogType == DIALOG_BATCH:
            return self.widget.getText()
        else:
            options = self._getOptions()
            values = [options[i] for i in self.widget.selectedoptions]
            if len(values) == 0 and not self.param.flags() & QgsProcessingParameterDefinition.FlagOptional:
                raise InvalidParameterValue()
            return values
