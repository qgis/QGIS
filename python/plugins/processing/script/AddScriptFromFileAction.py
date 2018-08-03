# -*- coding: utf-8 -*-

"""
***************************************************************************
    EditScriptAction.py
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

__author__ = 'Victor Olaya'
__date__ = 'April 2014'
__copyright__ = '(C) 201, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import shutil

from qgis.PyQt.QtCore import QCoreApplication
from qgis.PyQt.QtWidgets import QFileDialog

from qgis.core import Qgis, QgsApplication, QgsMessageLog, QgsSettings

from processing.gui.ToolboxAction import ToolboxAction

from processing.script import ScriptUtils


class AddScriptFromFileAction(ToolboxAction):

    def __init__(self):
        self.name = QCoreApplication.translate("AddScriptFromFileAction", "Add Script to Toolbox…")
        self.group = self.tr("Tools")

    def execute(self):
        settings = QgsSettings()
        lastDir = settings.value("processing/lastScriptsDir", "")
        files, _ = QFileDialog.getOpenFileNames(self.toolbox,
                                                self.tr("Add script(s)"),
                                                lastDir,
                                                self.tr("Processing scripts (*.py *.PY)"))
        if files:
            settings.setValue("processing/lastScriptsDir", os.path.dirname(files[0]))

            valid = 0
            for f in files:
                try:
                    shutil.copy(f, ScriptUtils.scriptsFolders()[0])
                    valid += 1
                except OSError as e:
                    QgsMessageLog.logMessage(self.tr("Could not copy script '{}'\n{}").format(f, str(e)),
                                             "Processing",
                                             Qgis.Warning)

            if valid > 0:
                QgsApplication.processingRegistry().providerById("script").refreshAlgorithms()
