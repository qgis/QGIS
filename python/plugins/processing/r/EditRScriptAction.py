# -*- coding: utf-8 -*-

"""
***************************************************************************
    EditRScriptAction.py
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
from processing.r.RAlgorithm import RAlgorithm
from processing.r.EditRScriptDialog import EditRScriptDialog

class EditRScriptAction(ContextAction):

    def __init__(self):
        self.name="Edit R script"

    def isEnabled(self):
        return isinstance(self.alg, RAlgorithm)

    def execute(self):
        dlg = EditRScriptDialog(self.alg)
        dlg.show()
        dlg.exec_()
        if dlg.update:
            self.toolbox.updateTree()