"""
***************************************************************************
    DeleteScriptAction.py
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

from qgis.PyQt.QtCore import QCoreApplication
from qgis.PyQt.QtWidgets import QMessageBox

from qgis.core import QgsApplication, QgsProcessingAlgorithm

from processing.gui.ContextAction import ContextAction

from processing.script import ScriptUtils


class DeleteScriptAction(ContextAction):

    def __init__(self):
        super().__init__()
        self.name = QCoreApplication.translate("DeleteScriptAction", "Delete Script…")

    def isEnabled(self):
        return (
            isinstance(self.itemData, QgsProcessingAlgorithm)
            and self.itemData.provider().id() == "script"
        )

    def execute(self):
        reply = QMessageBox.question(
            None,
            self.tr("Delete Script"),
            self.tr("Are you sure you want to delete this script?"),
            QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No,
            QMessageBox.StandardButton.No,
        )
        if reply == QMessageBox.StandardButton.Yes:
            filePath = ScriptUtils.findAlgorithmSource(self.itemData.name())
            if filePath is not None:
                os.remove(filePath)
                QgsApplication.processingRegistry().providerById(
                    "script"
                ).refreshAlgorithms()
            else:
                QMessageBox.warning(
                    None,
                    self.tr("Delete Script"),
                    self.tr("Can not find corresponding script file."),
                )
