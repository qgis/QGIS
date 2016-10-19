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
from shutil import copyfile

from qgis.PyQt.QtWidgets import QFileDialog, QMessageBox
from qgis.PyQt.QtGui import QIcon
from qgis.PyQt.QtCore import QSettings, QFileInfo

from processing.script.ScriptAlgorithm import ScriptAlgorithm
from processing.gui.ToolboxAction import ToolboxAction
from processing.script.WrongScriptException import WrongScriptException
from processing.script.ScriptUtils import ScriptUtils
from processing.core.alglist import algList

pluginPath = os.path.split(os.path.dirname(__file__))[0]


class AddScriptFromFileAction(ToolboxAction):

    def __init__(self):
        self.name, self.i18n_name = self.trAction('Add script from file')
        self.group, self.i18n_group = self.trAction('Tools')

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'scriptfile.svg'))

    def execute(self):
        settings = QSettings()
        lastDir = settings.value('Processing/lastScriptsDir', '')
        filename, selected_filter = QFileDialog.getOpenFileNames(self.toolbox,
                                                        self.tr('Script files', 'AddScriptFromFileAction')), (lastDir,
                                                        self.tr('Script files (*.py *.PY)', 'AddScriptFromFileAction'))

        if filename:
            try:
                # settings.setValue('Processing/lastScriptsDir',
                                #   QFileInfo(filename).absoluteDir().absolutePath())
                for f in filename:
                    if f.endswith('.py'):
                        script = ScriptAlgorithm(f)
            except WrongScriptException:
                QMessageBox.warning(self.toolbox,
                                    self.tr('Error reading script', 'AddScriptFromFileAction'),
                                    self.tr('The selected file does not contain a valid script', 'AddScriptFromFileAction'))
                return

            destFilename = []
            for scr in filename:
                if scr.endswith('.py'):
                    destFilename.append(os.path.join(ScriptUtils.defaultScriptsFolder(), os.path.basename(scr)))

            for j in destFilename:
                with open(j, 'w') as f:
                    f.write(script.script)

            for file in filename:
                if file.endswith('.help'):
                    copyfile(file, os.path.join(ScriptUtils.defaultScriptsFolder(), os.path.basename(os.path.normpath(file))))

            algList.reloadProvider('script')
