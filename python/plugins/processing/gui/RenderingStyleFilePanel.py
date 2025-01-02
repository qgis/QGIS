"""
***************************************************************************
    RenderingStyleFilePanel.py
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

import os
import warnings

from qgis.PyQt import uic
from qgis.PyQt.QtWidgets import QFileDialog

from processing.tools.system import isWindows

pluginPath = os.path.split(os.path.dirname(__file__))[0]

with warnings.catch_warnings():
    warnings.filterwarnings("ignore", category=DeprecationWarning)
    WIDGET, BASE = uic.loadUiType(
        os.path.join(pluginPath, "ui", "widgetBaseSelector.ui")
    )


class RenderingStyleFilePanel(BASE, WIDGET):

    def __init__(self):
        super().__init__(None)
        self.setupUi(self)

        self.btnSelect.clicked.connect(self.showSelectionDialog)

    def showSelectionDialog(self):
        filename, selected_filter = QFileDialog.getOpenFileName(
            self,
            self.tr("Select Style File"),
            "",
            self.tr("QGIS Layer Style File (*.qml *.QML)"),
        )
        if filename:
            self.leText.setText(filename)

    def setText(self, text):
        self.leText.setText(text)

    def getValue(self):
        s = self.leText.text()
        if isWindows():
            s = s.replace("\\", "/")
        return s
