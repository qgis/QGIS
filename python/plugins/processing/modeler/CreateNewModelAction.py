# -*- coding: utf-8 -*-

"""
***************************************************************************
    CreateNewModelAction.py
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
from PyQt4 import QtGui
from processing.gui.ToolboxAction import ToolboxAction
from processing.modeler.ModelerDialog import ModelerDialog

class CreateNewModelAction(ToolboxAction):

    def __init__(self):
        self.name = 'Create new model'
        self.group = 'Tools'

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + '/../images/model.png')

    def execute(self):
        dlg = ModelerDialog()
        dlg.show()
        dlg.exec_()
        if dlg.update:
            self.toolbox.updateProvider('model')
