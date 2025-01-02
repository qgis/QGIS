"""
***************************************************************************
    OpenScriptFromFileAction.py
    ---------------------
    Date                 : May 2018
    Copyright            : (C) 2018 by Mathieu Pellerin
    Email                : nirvn dot asia at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Nyall Dawson"
__date__ = "February 2018"
__copyright__ = "(C) 2018, Nyall Dawson"

import os
from qgis.PyQt.QtWidgets import QFileDialog
from qgis.PyQt.QtCore import QFileInfo, QCoreApplication

from qgis.core import QgsApplication, QgsSettings
from qgis.utils import iface

from processing.gui.ToolboxAction import ToolboxAction
from processing.script.ScriptEditorDialog import ScriptEditorDialog

pluginPath = os.path.split(os.path.dirname(__file__))[0]


class OpenScriptFromFileAction(ToolboxAction):

    def __init__(self):
        self.name = QCoreApplication.translate(
            "OpenScriptFromFileAction", "Open Existing Script…"
        )
        self.group = self.tr("Tools")

    def getIcon(self):
        return QgsApplication.getThemeIcon("/processingScript.svg")

    def execute(self):
        settings = QgsSettings()
        lastDir = settings.value("Processing/lastScriptsDir", "")
        filename, selected_filter = QFileDialog.getOpenFileName(
            self.toolbox,
            self.tr("Open Script", "AddScriptFromFileAction"),
            lastDir,
            self.tr("Processing scripts (*.py *.PY)", "AddScriptFromFileAction"),
        )
        if filename:
            settings.setValue(
                "Processing/lastScriptsDir",
                QFileInfo(filename).absoluteDir().absolutePath(),
            )

            dlg = ScriptEditorDialog(filePath=filename, parent=iface.mainWindow())
            dlg.show()
