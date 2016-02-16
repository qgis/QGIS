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

from processing.gui.ContextAction import ContextAction
from processing.gui.ScriptEditorDialog import ScriptEditorDialog
from processing.algs.r.RAlgorithm import RAlgorithm
from processing.script.ScriptAlgorithm import ScriptAlgorithm


class EditScriptAction(ContextAction):

    SCRIPT_PYTHON = 0
    SCRIPT_R = 1

    def __init__(self, scriptType):
        self.name = self.tr('Edit script', 'EditScriptAction')
        self.scriptType = scriptType

    def isEnabled(self):
        if self.scriptType == ScriptEditorDialog.SCRIPT_PYTHON:
            return isinstance(self.alg, ScriptAlgorithm) and self.alg.allowEdit
        elif self.scriptType == ScriptEditorDialog.SCRIPT_R:
            return isinstance(self.alg, RAlgorithm)

    def execute(self):
        dlg = ScriptEditorDialog(self.scriptType, self.alg)
        dlg.show()
        dlg.exec_()
        if dlg.update:
            if self.scriptType == ScriptEditorDialog.SCRIPT_PYTHON:
                self.toolbox.updateProvider('script')
            elif self.scriptType == ScriptEditorDialog.SCRIPT_R:
                self.toolbox.updateProvider('r')
