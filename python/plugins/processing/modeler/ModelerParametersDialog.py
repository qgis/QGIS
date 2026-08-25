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
    QgsColorButton,
    QgsGui,
    QgsHelp,
    QgsPanelWidgetStack,
    QgsProcessingModelConfigWidget,
    QgsProcessingModelerParametersPanelWidget,
)
from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtGui import QColor
from qgis.PyQt.QtWidgets import (
    QDialog,
    QDialogButtonBox,
    QHBoxLayout,
    QLabel,
    QTabWidget,
    QTextEdit,
    QVBoxLayout,
    QWidget,
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

        self.widget = ModelerParametersWidget(
            alg, model, algName, configuration, context=self.context, dialog=self
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


class ModelerParametersWidget(QgsProcessingModelConfigWidget):
    def __init__(
        self, alg, model, algName=None, configuration=None, dialog=None, context=None
    ):
        super().__init__()
        self._alg = alg  # The algorithm to define in this dialog. It is an instance of QgsProcessingAlgorithm
        self.model = model  # The model this algorithm is going to be added to. It is an instance of QgsProcessingModelAlgorithm
        self.childId = algName  # The name of the algorithm in the model, in case we are editing it and not defining it for the first time
        self.configuration = configuration
        self.context = context
        self.dialog = dialog

        self.widget = QgsProcessingModelerParametersPanelWidget(
            self._alg,
            self.model,
            self.context,
            self.childId,
            self.configuration,
            self,
            self.dialog,
        )
        self.widget.widgetChanged.connect(self.widgetChanged)

        self.setupUi()
        self.params = None

    def algorithm(self):
        return self._alg

    def switchToCommentTab(self):
        self.tab.setCurrentIndex(1)
        self.commentEdit.setFocus()
        self.commentEdit.selectAll()

    def setupUi(self):
        self.mainLayout = QVBoxLayout()
        self.mainLayout.setContentsMargins(0, 0, 0, 0)
        self.tab = QTabWidget()
        self.mainLayout.addWidget(self.tab)

        self.param_widget = QgsPanelWidgetStack()
        self.widget.setDockMode(True)
        self.param_widget.setMainPanel(self.widget)

        self.tab.addTab(self.param_widget, self.tr("Properties"))

        self.commentLayout = QVBoxLayout()
        self.commentEdit = QTextEdit()
        self.commentEdit.setAcceptRichText(False)
        self.commentLayout.addWidget(self.commentEdit, 1)

        hl = QHBoxLayout()
        hl.setContentsMargins(0, 0, 0, 0)
        hl.addWidget(QLabel(self.tr("Color")))
        self.comment_color_button = QgsColorButton()
        self.comment_color_button.setAllowOpacity(True)
        self.comment_color_button.setWindowTitle(self.tr("Comment Color"))
        self.comment_color_button.setShowNull(True, self.tr("Default"))
        hl.addWidget(self.comment_color_button)
        self.commentLayout.addLayout(hl)

        w2 = QWidget()
        w2.setLayout(self.commentLayout)
        self.tab.addTab(w2, self.tr("Comments"))

        self.setLayout(self.mainLayout)

    def setComments(self, text):
        self.commentEdit.setPlainText(text)

    def comments(self):
        return self.commentEdit.toPlainText()

    def setCommentColor(self, color):
        if color.isValid():
            self.comment_color_button.setColor(color)
        else:
            self.comment_color_button.setToNull()

    def commentColor(self):
        return (
            self.comment_color_button.color()
            if not self.comment_color_button.isNull()
            else QColor()
        )

    def setPreviousValues(self):
        self.widget.setPreviousValues()

    def createAlgorithm(self):
        alg = self.widget.createAlgorithm()
        if alg:
            alg.comment().setDescription(self.comments())
            alg.comment().setColor(self.commentColor())
        return alg
