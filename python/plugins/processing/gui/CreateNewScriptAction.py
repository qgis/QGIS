# -*- coding: utf-8 -*-

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

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtGui import QIcon

from processing.gui.ToolboxAction import ToolboxAction
from processing.gui.ScriptEditorDialog import ScriptEditorDialog
from processing.core.alglist import algList

pluginPath = os.path.split(os.path.dirname(__file__))[0]


class CreateNewScriptAction(ToolboxAction):

    SCRIPT_PYTHON = 0
    SCRIPT_R = 1

    def __init__(self, actionName, scriptType):
        self.name, self.i18n_name = self.trAction(actionName)
        self.group, self.i18n_group = self.trAction('Tools')

        self.scriptType = scriptType

    def getIcon(self):
        if self.scriptType == self.SCRIPT_PYTHON:
            return QIcon(os.path.join(pluginPath, 'images', 'script.png'))
        elif self.scriptType == self.SCRIPT_R:
            return QIcon(os.path.join(pluginPath, 'images', 'r.svg'))

    def execute(self):
        dlg = None
        if self.scriptType == self.SCRIPT_PYTHON:
            dlg = ScriptEditorDialog(ScriptEditorDialog.SCRIPT_PYTHON, None)
        if self.scriptType == self.SCRIPT_R:
            dlg = ScriptEditorDialog(ScriptEditorDialog.SCRIPT_R, None)
        dlg.show()
        dlg.exec_()
        if dlg.update:
            if self.scriptType == self.SCRIPT_PYTHON:
                algList.reloadProvider('script')
            elif self.scriptType == self.SCRIPT_R:
                algList.reloadProvider('r')
