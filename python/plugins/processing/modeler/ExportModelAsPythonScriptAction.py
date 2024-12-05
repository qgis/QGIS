"""
***************************************************************************
    EditModelAction.py
    ---------------------
    Date                 : February 2019
    Copyright            : (C) 2019 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
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
__date__ = "February 2019"
__copyright__ = "(C) 2019, Nyall Dawson"

from qgis.PyQt.QtCore import QCoreApplication
from qgis.core import QgsProcessingAlgorithm, QgsProcessing, QgsApplication
from qgis.utils import iface

from processing.gui.ContextAction import ContextAction
from processing.script.ScriptEditorDialog import ScriptEditorDialog


class ExportModelAsPythonScriptAction(ContextAction):

    def __init__(self):
        super().__init__()
        self.name = QCoreApplication.translate(
            "ExportModelAsPythonScriptAction", "Export Model as Python Algorithm…"
        )

    def isEnabled(self):
        return isinstance(
            self.itemData, QgsProcessingAlgorithm
        ) and self.itemData.provider().id() in ("model", "project")

    def icon(self):
        return QgsApplication.getThemeIcon("/mActionSaveAsPython.svg")

    def execute(self):
        alg = self.itemData
        dlg = ScriptEditorDialog(parent=iface.mainWindow())

        dlg.editor.setText(
            "\n".join(
                alg.asPythonCode(
                    QgsProcessing.PythonOutputType.PythonQgsProcessingAlgorithmSubclass,
                    4,
                )
            )
        )
        dlg.show()
