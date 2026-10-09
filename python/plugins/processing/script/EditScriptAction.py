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
__date__ = "August 2012"
__copyright__ = "(C) 2012, Victor Olaya"

import inspect

from qgis.core import QgsMessageLog, QgsProcessingAlgorithm
from qgis.gui import QgsProcessingToolboxContextAction
from qgis.PyQt.QtCore import QCoreApplication
from qgis.PyQt.QtWidgets import QMessageBox
from qgis.utils import iface

from processing.script import ScriptUtils
from processing.script.ScriptEditorDialog import ScriptEditorDialog


class EditScriptAction(QgsProcessingToolboxContextAction):
    def __init__(self):
        super().__init__(QCoreApplication.translate("EditScriptAction", "Edit Script…"))

    def isCompatibleWithAlgorithm(self, provider_id: str, algorithm_name: str):
        return provider_id == "script"

    def trigger(self, context):
        filePath = ScriptUtils.findAlgorithmSource(context.algorithmName())
        if filePath is not None:
            dlg = ScriptEditorDialog(filePath, parent=iface.mainWindow())
            dlg.show()
        else:
            QMessageBox.warning(
                None,
                QCoreApplication.translate("EditScriptAction", "Edit Script"),
                QCoreApplication.translate(
                    "EditScriptAction", "Can not find corresponding script file."
                ),
            )
