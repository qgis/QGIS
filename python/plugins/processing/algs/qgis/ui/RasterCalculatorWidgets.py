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

__author__ = "Victor Olaya"
__date__ = "November 2016"
__copyright__ = "(C) 2016, Victor Olaya"

import os
from functools import partial
import re
import json

from qgis.utils import iface
from qgis.PyQt import uic
from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtGui import QTextCursor
from qgis.PyQt.QtWidgets import (
    QLineEdit,
    QPushButton,
    QLabel,
    QComboBox,
    QSpacerItem,
    QSizePolicy,
    QListWidgetItem,
)

from qgis.core import (
    QgsProcessingUtils,
    QgsProcessingParameterDefinition,
    QgsProcessingParameterRasterLayer,
    QgsProcessingOutputRasterLayer,
    QgsProject,
)

from processing.gui.wrappers import WidgetWrapper, DIALOG_STANDARD, DIALOG_BATCH
from processing.gui.BatchInputSelectionPanel import BatchInputSelectionPanel
from processing.tools import dataobjects
from processing.tools.system import userFolder

from processing.gui.wrappers import InvalidParameterValue

from qgis.analysis import QgsRasterCalculatorEntry, QgsRasterCalcNode

pluginPath = os.path.dirname(__file__)
WIDGET_ADD_NEW, BASE_ADD_NEW = uic.loadUiType(
    os.path.join(pluginPath, "AddNewExpressionDialog.ui")
)


class AddNewExpressionDialog(BASE_ADD_NEW, WIDGET_ADD_NEW):

    def __init__(self, expression):
        super().__init__()
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
    os.path.join(pluginPath, "PredefinedExpressionDialog.ui")
)


class PredefinedExpressionDialog(BASE_DLG, WIDGET_DLG):

    def __init__(self, expression, options):
        super().__init__()
        self.setupUi(self)

        self.filledExpression = None
        self.options = options
        self.expression = expression
        self.variables = set(re.findall(r"\[.*?\]", expression))
        self.comboBoxes = {}
        for variable in self.variables:
            label = QLabel(variable[1:-1])
            combo = QComboBox()
            for opt in self.options.keys():
                combo.addItem(opt)
            self.comboBoxes[variable] = combo
            self.groupBox.layout().addWidget(label)
            self.groupBox.layout().addWidget(combo)

        verticalSpacer = QSpacerItem(
            20, 40, QSizePolicy.Policy.Minimum, QSizePolicy.Policy.Expanding
        )
        self.groupBox.layout().addItem(verticalSpacer)

        self.buttonBox.rejected.connect(self.cancelPressed)
        self.buttonBox.accepted.connect(self.okPressed)

    def cancelPressed(self):
        self.close()

    def okPressed(self):
        self.filledExpression = self.expression
        for name, combo in self.comboBoxes.items():
            self.filledExpression = self.filledExpression.replace(
                name, self.options[combo.currentText()]
            )
        self.close()


WIDGET, BASE = uic.loadUiType(os.path.join(pluginPath, "RasterCalculatorWidget.ui"))


class ExpressionWidget(BASE, WIDGET):
    _expressions = {"NDVI": "([NIR] - [Red]) / ([NIR] + [Red])"}

    def __init__(self, options):
        super().__init__(None)
        self.setupUi(self)

        self.setList(options)

        def doubleClicked(item):
            self.text.insertPlainText(f'"{self.options[item.text()]}"')

        def addButtonText(text):
            if any(c for c in text if c.islower()):
                self.text.insertPlainText(f" {text}()")
                self.text.moveCursor(
                    QTextCursor.MoveOperation.PreviousCharacter,
                    QTextCursor.MoveMode.MoveAnchor,
                )
            else:
                self.text.insertPlainText(f" {text} ")

        buttons = [
            b for b in self.buttonsGroupBox.children() if isinstance(b, QPushButton)
        ]
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
        self.text.textChanged.connect(self.expressionValid)

    def expressionValid(self):
        errorString = ""
        testNode = QgsRasterCalcNode.parseRasterCalcString(
            self.text.toPlainText(), errorString
        )

        if not self.text.toPlainText():
            self.expressionErrorLabel.setText(self.tr("Expression is empty"))
            self.expressionErrorLabel.setStyleSheet("QLabel { color: black; }")
            return False

        if testNode:
            self.expressionErrorLabel.setText(self.tr("Expression is valid"))
            self.expressionErrorLabel.setStyleSheet(
                "QLabel { color: green; font-weight: bold; }"
            )
            return True

        self.expressionErrorLabel.setText(
            self.tr("Expression is not valid ") + errorString
        )
        self.expressionErrorLabel.setStyleSheet(
            "QLabel { color : red; font-weight: bold; }"
        )
        return False

    def expsFile(self):
        return os.path.join(userFolder(), "rastercalcexpressions.json")

    def addPredefined(self):
        expression = self.expressions[self.comboPredefined.currentText()]
        dlg = PredefinedExpressionDialog(expression, self.options)
        dlg.exec()
        if dlg.filledExpression:
            self.text.setPlainText(dlg.filledExpression)

    def savePredefined(self):
        exp = self.text.toPlainText()
        used = [v for v in self.options.values() if v in exp]

        for i, v in enumerate(used):
            exp = exp.replace(v, f"[{chr(97 + i)}]")

        dlg = AddNewExpressionDialog(exp)
        dlg.exec()
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
        entries = QgsRasterCalculatorEntry.rasterEntries()

        def _find_source(name):
            for entry in entries:
                if entry.ref == name:
                    return entry.raster.source()
            return ""

        for name in options.keys():
            item = QListWidgetItem(name, self.listWidget)
            tooltip = _find_source(name)
            if tooltip:
                item.setData(Qt.ItemDataRole.ToolTipRole, tooltip)
            self.listWidget.addItem(item)

    def setValue(self, value):
        self.text.setPlainText(value)

    def value(self):
        return self.text.toPlainText()


class ExpressionWidgetWrapper(WidgetWrapper):

    def _panel(self, options):
        return ExpressionWidget(options)

    def _get_options(self):
        entries = QgsRasterCalculatorEntry.rasterEntries()
        options = {}
        for entry in entries:
            options[entry.ref] = entry.ref
        return options

    def createWidget(self):
        if self.dialogType == DIALOG_STANDARD:
            if (
                iface is not None
                and iface.layerTreeView() is not None
                and iface.layerTreeView().layerTreeModel() is not None
            ):
                iface.layerTreeView().layerTreeModel().dataChanged.connect(self.refresh)
            return self._panel(self._get_options())
        elif self.dialogType == DIALOG_BATCH:
            return QLineEdit()
        else:
            layers = self.dialog.getAvailableValuesOfType(
                [QgsProcessingParameterRasterLayer], [QgsProcessingOutputRasterLayer]
            )
            options = {
                self.dialog.resolveValueDescription(
                    lyr
                ): f"{self.dialog.resolveValueDescription(lyr)}@1"
                for lyr in layers
            }
            self.widget = self._panel(options)
            return self.widget

    def refresh(self, *args):
        self.widget.setList(self._get_options())

    def setValue(self, value):
        if self.dialogType == DIALOG_STANDARD:
            pass  # TODO
        elif self.dialogType == DIALOG_BATCH:
            return self.widget.setText(value)
        else:
            self.widget.setValue(value)

    def value(self):
        if self.dialogType == DIALOG_STANDARD:
            return self.widget.value()
        elif self.dialogType == DIALOG_BATCH:
            return self.widget.text()
        else:
            return self.widget.value()


class LayersListWidgetWrapper(WidgetWrapper):

    def createWidget(self):
        if self.dialogType == DIALOG_BATCH:
            widget = BatchInputSelectionPanel(
                self.parameterDefinition(), self.row, self.col, self.dialog
            )
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
                    options = QgsProcessingUtils.compatibleRasterLayers(
                        QgsProject.instance(), False
                    )
                elif self.param.datatype == dataobjects.TYPE_VECTOR_ANY:
                    options = QgsProcessingUtils.compatibleVectorLayers(
                        QgsProject.instance(), [], False
                    )
                else:
                    options = QgsProcessingUtils.compatibleVectorLayers(
                        QgsProject.instance(), [self.param.datatype], False
                    )
                return [options[i] for i in self.widget.selectedoptions]
        elif self.dialogType == DIALOG_BATCH:
            return self.widget.getText()
        else:
            options = self._getOptions()
            values = [options[i] for i in self.widget.selectedoptions]
            if (
                len(values) == 0
                and not self.parameterDefinition().flags()
                & QgsProcessingParameterDefinition.Flag.FlagOptional
            ):
                raise InvalidParameterValue()
            return values
