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

__author__ = "Victor Olaya"
__date__ = "May 2015"
__copyright__ = "(C) 2015, Victor Olaya"

from processing.core.exceptions import InvalidOutputExtension, InvalidParameterValue
from processing.gui.algorithm_widget import AlgorithmWidget
from processing.gui.ParametersPanel import ParametersPanel
from processing.tools.dataobjects import createContext
from qgis.core import (
    QgsProcessingException,
    QgsProcessingFeedback,
    QgsProcessingParameterDefinition,
)
from qgis.PyQt.QtWidgets import (
    QLabel,
    QPlainTextEdit,
    QVBoxLayout,
    QWidget,
)


class GdalAlgorithmWidget(AlgorithmWidget):
    """
    Custom algorithm widget for showing GDAL command line arguments
    """

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

    def parametersHaveChanged(self):
        context = createContext()
        feedback = QgsProcessingFeedback()
        try:
            # messy as all heck, but we don't want to call the dialog's implementation of
            # createProcessingParameters as we want to catch the exceptions raised by the
            # parameter panel instead...
            parameters = (
                {}
                if self.dialog.mainWidget() is None
                else self.dialog.mainWidget().createProcessingParameters()
            )
            for output in self.algorithm().destinationParameterDefinitions():
                if not output.name() in parameters or parameters[output.name()] is None:
                    if (
                        not output.flags()
                        & QgsProcessingParameterDefinition.Flag.FlagOptional
                    ):
                        parameters[output.name()] = self.tr("[temporary file]")
            for p in self.algorithm().parameterDefinitions():
                if p.flags() & QgsProcessingParameterDefinition.Flag.FlagHidden:
                    continue

                if (
                    p.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
                    and p.name() not in parameters
                ):
                    continue

                if p.name() not in parameters or not p.checkValueIsAcceptable(
                    parameters[p.name()]
                ):
                    # not ready yet
                    self.text.setPlainText("")
                    return

            try:
                commands = self.algorithm().getConsoleCommands(
                    parameters, context, feedback, executing=False
                )
                commands = [c for c in commands if c not in ["cmd.exe", "/C "]]
                self.text.setPlainText(" ".join(commands))
            except QgsProcessingException as e:
                self.text.setPlainText(str(e))
        except InvalidParameterValue as e:
            self.text.setPlainText(
                self.tr("Invalid value for parameter '{0}'").format(
                    e.parameter.description()
                )
            )
        except InvalidOutputExtension as e:
            self.text.setPlainText(e.message)
