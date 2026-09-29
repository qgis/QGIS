"""
***************************************************************************
    CreateNewScriptAction.py
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

from qgis.gui import QgsProcessingToolboxAction
from qgis.PyQt.QtCore import QCoreApplication
from qgis.utils import iface

from processing.script.ScriptEditorDialog import ScriptEditorDialog


class CreateNewScriptAction(QgsProcessingToolboxAction):
    def __init__(self):
        super().__init__(
            QCoreApplication.translate("CreateNewScriptAction", "Create New Script…"),
            QCoreApplication.translate("ToolboxAction", "Tools"),
        )

    def trigger(self, context):
        dlg = ScriptEditorDialog(parent=iface.mainWindow())
        dlg.show()
