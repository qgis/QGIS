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
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import inspect

from qgis.core import QgsProcessingAlgorithm, QgsMessageLog
from qgis.utils import iface
from qgis.PyQt.QtCore import QCoreApplication
from qgis.PyQt.QtWidgets import QMessageBox

from processing.gui.ContextAction import ContextAction

from processing.script.ScriptEditorDialog import ScriptEditorDialog
from processing.script import ScriptUtils


class EditScriptAction(ContextAction):

    def __init__(self):
        super().__init__()
        self.name = QCoreApplication.translate("EditScriptAction", "Edit Script…")

    def isEnabled(self):
        return isinstance(self.itemData, QgsProcessingAlgorithm) and self.itemData.provider().id() == "script"

    def execute(self):
        filePath = ScriptUtils.findAlgorithmSource(self.itemData.name())
        if filePath is not None:
            dlg = ScriptEditorDialog(filePath, iface.mainWindow())
            dlg.show()
        else:
            QMessageBox.warning(None,
                                self.tr("Edit Script"),
                                self.tr("Can not find corresponding script file.")
                                )
