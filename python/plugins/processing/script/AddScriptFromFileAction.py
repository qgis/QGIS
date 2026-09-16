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

__author__ = "Victor Olaya"
__date__ = "April 2014"
__copyright__ = "(C) 201, Victor Olaya"

import os
import shutil

from qgis.core import Qgis, QgsApplication, QgsMessageLog, QgsSettings
from qgis.gui import QgsProcessingToolboxAction
from qgis.PyQt.QtCore import QCoreApplication
from qgis.PyQt.QtWidgets import QFileDialog

from processing.script import ScriptUtils


class AddScriptFromFileAction(QgsProcessingToolboxAction):
    def __init__(self):
        super().__init__(
            QCoreApplication.translate(
                "AddScriptFromFileAction", "Add Script to Toolbox…"
            ),
            QCoreApplication.translate("ToolboxAction", "Tools"),
        )

    def trigger(self, context):
        settings = QgsSettings()
        lastDir = settings.value("processing/lastScriptsDir", "")
        files, _ = QFileDialog.getOpenFileNames(
            context.parentWidget(),
            QCoreApplication.translate("AddScriptFromFileAction", "Add script(s)"),
            lastDir,
            QCoreApplication.translate(
                "AddScriptFromFileAction", "Processing scripts (*.py *.PY)"
            ),
        )
        if files:
            settings.setValue("processing/lastScriptsDir", os.path.dirname(files[0]))

            valid = 0
            for f in files:
                try:
                    shutil.copy(f, ScriptUtils.scriptsFolders()[0])
                    valid += 1
                except OSError as e:
                    QgsMessageLog.logMessage(
                        QCoreApplication.translate(
                            "AddScriptFromFileAction", "Could not copy script '{}'\n{}"
                        ).format(f, str(e)),
                        "Processing",
                        Qgis.MessageLevel.Warning,
                    )

            if valid > 0:
                QgsApplication.processingRegistry().providerById(
                    "script"
                ).refreshAlgorithms()
