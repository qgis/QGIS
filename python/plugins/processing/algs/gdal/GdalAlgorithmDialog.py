# -*- coding: utf-8 -*-

"""
***************************************************************************
    GdalAlgorithmDialog.py
    ---------------------
    Date                 : May 2015
    Copyright            : (C) 2015 by Victor Olaya
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
__date__ = 'May 2015'
__copyright__ = '(C) 2015, Victor Olaya'

from qgis.PyQt.QtCore import QCoreApplication
from qgis.PyQt.QtWidgets import (QWidget,
                                 QVBoxLayout,
                                 QPushButton,
                                 QLabel,
                                 QPlainTextEdit,
                                 QLineEdit,
                                 QComboBox,
                                 QCheckBox,
                                 QSizePolicy,
                                 QDialogButtonBox)

from qgis.core import (QgsProcessingFeedback,
                       QgsProcessingParameterDefinition)
from qgis.gui import (QgsMessageBar,
                      QgsProjectionSelectionWidget,
                      QgsProcessingAlgorithmDialogBase,
                      QgsProcessingLayerOutputDestinationWidget)

from processing.gui.AlgorithmDialog import AlgorithmDialog
from processing.gui.AlgorithmDialogBase import AlgorithmDialogBase
from processing.gui.ParametersPanel import ParametersPanel
from processing.gui.MultipleInputPanel import MultipleInputPanel
from processing.gui.NumberInputPanel import NumberInputPanel
from processing.gui.wrappers import WidgetWrapper
from processing.tools.dataobjects import createContext


class GdalAlgorithmDialog(AlgorithmDialog):

    def __init__(self, alg, parent=None):
        super().__init__(alg, parent=parent)
        self.mainWidget().parametersHaveChanged()

    def getParametersPanel(self, alg, parent):
        return GdalParametersPanel(parent, alg)


class GdalParametersPanel(ParametersPanel):

    def __init__(self, parent, alg):
        super().__init__(parent, alg)

        self.dialog = parent
        w = QWidget()
        layout = QVBoxLayout()
        layout.setMargin(0)
        layout.setSpacing(6)
        label = QLabel()
        label.setText(self.tr("GDAL/OGR console call"))
        layout.addWidget(label)
        self.text = QPlainTextEdit()
        self.text.setReadOnly(True)
        layout.addWidget(self.text)
        w.setLayout(layout)
        self.addExtraWidget(w)

        self.connectParameterSignals()
        self.parametersHaveChanged()

    def connectParameterSignals(self):
        for wrapper in list(self.wrappers.values()):
            wrapper.widgetValueHasChanged.connect(self.parametersHaveChanged)

            # TODO - remove when all wrappers correctly emit widgetValueHasChanged!

            # For compatibility with 3.x API, we need to check whether the wrapper is
            # the deprecated WidgetWrapper class. If not, it's the newer
            # QgsAbstractProcessingParameterWidgetWrapper class
            # TODO QGIS 4.0 - remove
            if issubclass(wrapper.__class__, WidgetWrapper):
                w = wrapper.widget
            else:
                w = wrapper.wrappedWidget()

            self.connectWidgetChangedSignals(w)
            for c in w.findChildren(QWidget):
                self.connectWidgetChangedSignals(c)

    def connectWidgetChangedSignals(self, w):
        if isinstance(w, QLineEdit):
            w.textChanged.connect(self.parametersHaveChanged)
        elif isinstance(w, QComboBox):
            w.currentIndexChanged.connect(self.parametersHaveChanged)
        elif isinstance(w, QgsProjectionSelectionWidget):
            w.crsChanged.connect(self.parametersHaveChanged)
        elif isinstance(w, QCheckBox):
            w.stateChanged.connect(self.parametersHaveChanged)
        elif isinstance(w, MultipleInputPanel):
            w.selectionChanged.connect(self.parametersHaveChanged)
        elif isinstance(w, NumberInputPanel):
            w.hasChanged.connect(self.parametersHaveChanged)
        elif isinstance(w, QgsProcessingLayerOutputDestinationWidget):
            w.destinationChanged.connect(self.parametersHaveChanged)

    def parametersHaveChanged(self):
        context = createContext()
        feedback = QgsProcessingFeedback()
        try:
            # messy as all heck, but we don't want to call the dialog's implementation of
            # createProcessingParameters as we want to catch the exceptions raised by the
            # parameter panel instead...
            parameters = {} if self.dialog.mainWidget() is None else self.dialog.mainWidget().createProcessingParameters()
            for output in self.algorithm().destinationParameterDefinitions():
                if not output.name() in parameters or parameters[output.name()] is None:
                    if not output.flags() & QgsProcessingParameterDefinition.FlagOptional:
                        parameters[output.name()] = self.tr("[temporary file]")
            for p in self.algorithm().parameterDefinitions():
                if p.flags() & QgsProcessingParameterDefinition.FlagHidden:
                    continue

                if p.flags() & QgsProcessingParameterDefinition.FlagOptional and p.name() not in parameters:
                    continue

                if p.name() not in parameters or not p.checkValueIsAcceptable(parameters[p.name()]):
                    # not ready yet
                    self.text.setPlainText('')
                    return

            commands = self.algorithm().getConsoleCommands(parameters, context, feedback, executing=False)
            commands = [c for c in commands if c not in ['cmd.exe', '/C ']]
            self.text.setPlainText(" ".join(commands))
        except AlgorithmDialogBase.InvalidParameterValue as e:
            self.text.setPlainText(self.tr("Invalid value for parameter '{0}'").format(e.parameter.description()))
        except AlgorithmDialogBase.InvalidOutputExtension as e:
            self.text.setPlainText(e.message)
