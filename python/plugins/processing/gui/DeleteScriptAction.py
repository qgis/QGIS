# -*- coding: utf-8 -*-

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

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtWidgets import QMessageBox

from processing.gui.ContextAction import ContextAction

from processing.algs.r.RAlgorithm import RAlgorithm
from processing.script.ScriptAlgorithm import ScriptAlgorithm
from processing.core.alglist import algList


class DeleteScriptAction(ContextAction):

    SCRIPT_PYTHON = 0
    SCRIPT_R = 1

    def __init__(self, scriptType):
        self.name = self.tr('Delete script', 'DeleteScriptAction')
        self.scriptType = scriptType

    def isEnabled(self):
        if self.scriptType == self.SCRIPT_PYTHON:
            return isinstance(self.itemData, ScriptAlgorithm) and self.itemData.allowEdit
        elif self.scriptType == self.SCRIPT_R:
            return isinstance(self.itemData, RAlgorithm)

    def execute(self):
        reply = QMessageBox.question(None,
                                     self.tr('Confirmation', 'DeleteScriptAction'),
                                     self.tr('Are you sure you want to delete this script?',
                                             'DeleteScriptAction'),
                                     QMessageBox.Yes | QMessageBox.No,
                                     QMessageBox.No)
        if reply == QMessageBox.Yes:
            os.remove(self.itemData.descriptionFile)
            if self.scriptType == self.SCRIPT_PYTHON:
                algList.reloadProvider('script')
            elif self.scriptType == self.SCRIPT_R:
                algList.reloadProvider('r')
