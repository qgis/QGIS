from processing.gui.wrappers import WidgetWrapper, DIALOG_STANDARD, DIALOG_BATCH
from processing.tools import dataobjects
from processing.gui.BatchInputSelectionPanel import BatchInputSelectionPanel
from qgis.PyQt.QtWidgets import (QListWidget, QLineEdit, QPushButton, QLabel,
                                 QComboBox, QSpacerItem, QSizePolicy)
from qgis.PyQt.QtGui import QTextCursor
from processing.core.outputs import OutputRaster
from processing.core.parameters import ParameterRaster
from processing.gui.wrappers import InvalidParameterValue
import os
from qgis.PyQt import uic
from functools import partial
import re

pluginPath = os.path.dirname(__file__)
WIDGET_DLG, BASE_DLG = uic.loadUiType(
    os.path.join(pluginPath, 'PredefinedExpressionDialog.ui'))

class PredefinedExpressionDialog(BASE_DLG, WIDGET_DLG):

    def __init__(self, expression, options):
        super(PredefinedExpressionDialog, self).__init__()
        self.setupUi(self)

        self.filledExpression = None
        self.options = options
        self.expression = expression
        self.variables = set(re.findall('\[.*?\]', expression))
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
    os.path.join(pluginPath, 'ExpressionWidget.ui'))

class ExpressionWidget(BASE, WIDGET):

    expressions = {"NDVI": "([NIR] - [Red]) % ([NIR] + [Red])"}

    def __init__(self, options):
        super(ExpressionWidget, self).__init__(None)
        self.setupUi(self)

        self.setList(options)

        def doubleClicked(item):
            self.text.insertPlainText(self.options[item.text()])

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

        self.fillPredefined()
        self.buttonAddPredefined.clicked.connect(self.addPredefined)

    def addPredefined(self):
        expression = self.expressions[self.comboPredefined.currentText()]
        dlg = PredefinedExpressionDialog(expression, self.options)
        dlg.exec_()
        if dlg.filledExpression:
            self.text.setPlainText(dlg.filledExpression)

    def fillPredefined(self):
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
            layers = dataobjects.getRasterLayers(sorting=False)
            options = {}
            for lyr in layers:
                for n in range(lyr.bandCount()):
                    name = '{:s}@{:d}'.format(lyr.name(), n + 1)
                    options[name] = name
            return self._panel(options)
        elif self.dialogType == DIALOG_BATCH:
            return QLineEdit()
        else:
            layers = self.dialog.getAvailableValuesOfType(ParameterRaster, OutputRaster)
            options = {self.dialog.resolveValueDescription(lyr): "{}@1".format(lyr) for lyr in layers}
            return self._panel(options)

    def refresh(self):
        layers = dataobjects.getRasterLayers()
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
                    options = dataobjects.getRasterLayers(sorting=False)
                elif self.param.datatype == dataobjects.TYPE_VECTOR_ANY:
                    options = dataobjects.getVectorLayers(sorting=False)
                else:
                    options = dataobjects.getVectorLayers([self.param.datatype], sorting=False)
                return [options[i] for i in self.widget.selectedoptions]
        elif self.dialogType == DIALOG_BATCH:
            return self.widget.getText()
        else:
            options = self._getOptions()
            values = [options[i] for i in self.widget.selectedoptions]
            if len(values) == 0 and not self.param.optional:
                raise InvalidParameterValue()
            return values
