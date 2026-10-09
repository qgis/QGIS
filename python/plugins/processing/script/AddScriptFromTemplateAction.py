"""
***************************************************************************
    AddScriptFromTemplateAction.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2018 by Matteo Ghetta
    Email                : matteo dot ghetta at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Matteo Ghetta"
__date__ = "March 2018"
__copyright__ = "(C) 2018, Matteo Ghetta"

import os

from qgis.gui import QgsProcessingToolboxAction
from qgis.PyQt.QtCore import QCoreApplication
from qgis.utils import iface

from processing.script.ScriptEditorDialog import ScriptEditorDialog


class AddScriptFromTemplateAction(QgsProcessingToolboxAction):
    def __init__(self):
        super().__init__(
            QCoreApplication.translate(
                "AddScriptFromTemplate", "Create New Script from Template…"
            ),
            QCoreApplication.translate("ToolboxAction", "Tools"),
        )

    def trigger(self, context):
        dlg = ScriptEditorDialog(parent=iface.mainWindow())

        pluginPath = os.path.split(os.path.dirname(__file__))[0]
        templatePath = os.path.join(pluginPath, "script", "ScriptTemplate.py")

        with open(templatePath, encoding="utf-8") as f:
            templateTxt = f.read()
            dlg.codeEditor().setText(templateTxt)

        dlg.show()
