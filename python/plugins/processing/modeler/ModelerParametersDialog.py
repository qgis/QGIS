"""
***************************************************************************
    ModelerParametersDialog.py
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

__author__ = "Victor Olaya"
__date__ = "August 2012"
__copyright__ = "(C) 2012, Victor Olaya"

import webbrowser
from typing import Optional

from qgis.core import (
    QgsProcessingAlgorithm,
    QgsProcessingModelAlgorithm,
)
from qgis.gui import (
    QgsGui,
    QgsHelp,
    QgsProcessingModelerParametersWidget,
)
from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtWidgets import (
    QDialog,
    QDialogButtonBox,
    QVBoxLayout,
)
from qgis.utils import iface

from processing.tools.dataobjects import createContext


class ModelerParametersDialog(QDialog):
    def __init__(
        self,
        alg: QgsProcessingAlgorithm,
        model: QgsProcessingModelAlgorithm,
        algName: Optional[str] = None,
        configuration: Optional[dict[str, object]] = None,
    ):
        super().__init__()
        self.setObjectName("ModelerParametersDialog")

        self.setStyleSheet(QgsGui.applicationStyleSheet())
        QgsGui.instance().applicationStyleSheetChanged.connect(self.setStyleSheet)

        # dammit this is SUCH as mess... stupid stable API
        self._alg = alg  # The algorithm to define in this dialog. It is an instance of QgsProcessingAlgorithm
        self.model = model  # The model this algorithm is going to be added to. It is an instance of QgsProcessingModelAlgorithm
        self.childId = algName  # The name of the algorithm in the model, in case we are editing it and not defining it for the first time
        self.configuration = configuration
        self.context = createContext()

        self.setWindowTitle(
            " - ".join([self._alg.group(), self._alg.displayName()])
            if self._alg.group()
            else self._alg.displayName()
        )

        self.widget = QgsProcessingModelerParametersWidget(
            alg, model, self.context, algName, configuration, dialog=self
        )
        QgsGui.enableAutoGeometryRestore(self)

        self.buttonBox = QDialogButtonBox()
        self.buttonBox.setOrientation(Qt.Orientation.Horizontal)
        self.buttonBox.setStandardButtons(
            QDialogButtonBox.StandardButton.Cancel
            | QDialogButtonBox.StandardButton.Ok
            | QDialogButtonBox.StandardButton.Help
        )

        self.buttonBox.accepted.connect(self.okPressed)
        self.buttonBox.rejected.connect(self.reject)
        self.buttonBox.helpRequested.connect(self.openHelp)

        mainLayout = QVBoxLayout()
        mainLayout.addWidget(self.widget, 1)
        mainLayout.addWidget(self.buttonBox)
        self.setLayout(mainLayout)

    def algorithm(self):
        return self._alg

    def setComments(self, text):
        self.widget.setComments(text)

    def comments(self):
        return self.widget.comments()

    def setCommentColor(self, color):
        self.widget.setCommentColor(color)

    def commentColor(self):
        return self.widget.commentColor()

    def switchToCommentTab(self):
        self.widget.switchToCommentTab()

    def setPreviousValues(self):
        self.widget.setPreviousValues()

    def createAlgorithm(self):
        return self.widget.createAlgorithm()

    def okPressed(self):
        if self.createAlgorithm() is not None:
            self.accept()

    def openHelp(self):
        algHelp = self.widget.algorithm().helpUrl()
        if not algHelp:
            algHelp = QgsHelp.helpUrl(
                "processing_algs/{}/{}.html#{}".format(
                    self.widget.algorithm().provider().helpId(),
                    self.algorithm().groupId(),
                    f"{self.algorithm().provider().helpId()}{self.algorithm().name().replace('_', '-')}",
                )
            ).toString()

        if algHelp not in [None, ""]:
            webbrowser.open(algHelp)
