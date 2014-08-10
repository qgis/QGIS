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
__date__ = 'April 2014'
__copyright__ = '(C) 201, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
from PyQt4 import QtGui
from processing.script.ScriptAlgorithm import ScriptAlgorithm
from processing.gui.ToolboxAction import ToolboxAction
from processing.script.WrongScriptException import WrongScriptException
from processing.script.ScriptUtils import ScriptUtils

class AddScriptFromFileAction(ToolboxAction):

    def __init__(self):
        self.name = "Add script from file"
        self.group = 'Tools'

    def getIcon(self):
        return QtGui.QIcon(':/processing/images/script.png')

    def execute(self):
        filename = QtGui.QFileDialog.getOpenFileName(self.toolbox, 'Script files', None,
                '*.py')
        if filename:
            try:
                script = ScriptAlgorithm(filename)
            except WrongScriptException:
                QtGui.QMessageBox.warning(self.toolbox, "Error reading script", "The selected file does not contain a valid script")
                return
            destFilename = os.path.join(ScriptUtils.scriptsFolder(), os.path.basename(filename))
            with open(destFilename, "w") as f:
                f.write(script.script)
            self.toolbox.updateProvider('script')