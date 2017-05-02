# -*- coding: utf-8 -*-

"""
***************************************************************************
    EditScriptAction.py
    ---------------------
    Date                 : March 2017
    Copyright            : (C) 2017 by Matteo Ghetta
    Email                : matteo dot ghetta at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Matteo Ghetta'
__date__ = 'March 2017'
__copyright__ = '(C) 2017, Matteo Ghetta'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
from shutil import copyfile

from qgis.PyQt.QtWidgets import QFileDialog, QMessageBox
from qgis.PyQt.QtCore import QFileInfo

from qgis.core import QgsApplication, QgsSettings

from processing.algs.r.RAlgorithm import RAlgorithm
from processing.gui.ToolboxAction import ToolboxAction
from processing.script.WrongScriptException import WrongScriptException
from processing.core.alglist import algList
from .RUtils import RUtils

pluginPath = os.path.split(os.path.dirname(__file__))[0]


class AddRScriptFromFileAction(ToolboxAction):

    def __init__(self):
        self.name, self.i18n_name = self.trAction('Add R script from file')
        self.group, self.i18n_group = self.trAction('Tools')

    def getIcon(self):
        return QgsApplication.getThemeIcon("/rfile.svg")

    def execute(self):
        settings = QgsSettings()
        lastDir = settings.value('Processing/lastScriptsDir', '')
        filenames, selected_filter = QFileDialog.getOpenFileNames(self.toolbox,
                                                                  self.tr('R Script files', 'AddRScriptFromFileAction'), lastDir,
                                                                  self.tr('R Script files (*.rsx)', 'AddRScriptFromFileAction'))
        if filenames:
            validAlgs = 0
            wrongAlgs = []
            for filename in filenames:
                try:
                    settings.setValue('Processing/lastScriptsDir',
                                      QFileInfo(filename).absoluteDir().absolutePath())
                    script = RAlgorithm(filename)
                    destFilename = os.path.join(RUtils.defaultRScriptsFolder(), os.path.basename(filename))
                    with open(destFilename, 'w') as f:
                        f.write(script.script)
                    helpscript = '{}.{}'.format(filename, 'help')
                    if os.path.exists(helpscript):
                        destHelpFilename = '{}.{}'.format(destFilename, 'help')
                        print(destHelpFilename)
                        copyfile(helpscript, os.path.join(RUtils.defaultRScriptsFolder(), os.path.basename(destHelpFilename)))
                    validAlgs += 1
                except WrongScriptException:
                    wrongAlgs.append(os.path.basename(filename))
            if validAlgs:
                algList.reloadProvider('r')
            if wrongAlgs:
                QMessageBox.warning(self.toolbox,
                                    self.tr('Error reading scripts', 'AddRScriptFromFileAction'),
                                    self.tr('The following files do not contain a valid script:\n-', 'AddRScriptFromFileAction') +
                                    "\n-".join(wrongAlgs))
