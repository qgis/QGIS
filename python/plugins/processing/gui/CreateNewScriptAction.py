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

from PyQt4.QtGui import *
from processing.gui.ToolboxAction import ToolboxAction
from processing.gui.ScriptEditorDialog import ScriptEditorDialog
import processing.resources_rc

class CreateNewScriptAction(ToolboxAction):

    SCRIPT_PYTHON = 0
    SCRIPT_R = 1

    def __init__(self, actionName, scriptType):
        self.name = actionName
        self.group = self.tr('Tools', 'CreateNewScriptAction')
        self.scriptType = scriptType

    def getIcon(self):
        if self.scriptType == self.SCRIPT_PYTHON:
            return QIcon(':/processing/images/script.png')
        elif self.scriptType == self.SCRIPT_R:
            return QIcon(':/processing/images/r.png')

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
                self.toolbox.updateProvider('script')
            elif self.scriptType == self.SCRIPT_R:
                self.toolbox.updateProvider('r')
