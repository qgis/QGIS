"""
***************************************************************************
    FileSelectionPanel.py
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
from qgis.core import QgsSettings

from processing.tools.system import isWindows

pluginPath = os.path.split(os.path.dirname(__file__))[0]

with warnings.catch_warnings():
    warnings.filterwarnings("ignore", category=DeprecationWarning)
    WIDGET, BASE = uic.loadUiType(
        os.path.join(pluginPath, "ui", "widgetBaseSelector.ui")
    )


class FileSelectionPanel(BASE, WIDGET):

    def __init__(self, isFolder, ext=None):
        super().__init__(None)
        self.setupUi(self)

        self.ext = ext or "*"
        self.isFolder = isFolder

        self.btnSelect.clicked.connect(self.showSelectionDialog)

    def showSelectionDialog(self):
        # Find the file dialog's working directory
        settings = QgsSettings()
        text = self.leText.text()
        if os.path.isdir(text):
            path = text
        elif os.path.isdir(os.path.dirname(text)):
            path = os.path.dirname(text)
        elif settings.contains("/Processing/LastInputPath"):
            path = settings.value("/Processing/LastInputPath")
        else:
            path = ""

        if self.isFolder:
            folder = QFileDialog.getExistingDirectory(
                self, self.tr("Select Folder"), path
            )
            if folder:
                self.leText.setText(folder)
                settings.setValue("/Processing/LastInputPath", os.path.dirname(folder))
        else:
            filenames, selected_filter = QFileDialog.getOpenFileNames(
                self,
                self.tr("Select File"),
                path,
                self.tr("{} files").format(self.ext.upper())
                + " (*."
                + self.ext
                + self.tr(");;All files (*.*)"),
            )
            if filenames:
                self.leText.setText(";".join(filenames))
                settings.setValue(
                    "/Processing/LastInputPath", os.path.dirname(filenames[0])
                )

    def getValue(self):
        s = self.leText.text()
        if isWindows():
            s = s.replace("\\", "/")
        return s

    def setText(self, text):
        self.leText.setText(text)
