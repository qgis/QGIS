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
import shutil
from qgis.PyQt.QtGui import QIcon
from qgis.PyQt.QtWidgets import QFileDialog, QMessageBox
from qgis.PyQt.QtCore import QSettings, QFileInfo
from processing.gui.ToolboxAction import ToolboxAction
from processing.modeler.ModelerAlgorithm import ModelerAlgorithm
from processing.modeler.WrongModelException import WrongModelException
from processing.modeler.ModelerUtils import ModelerUtils
from processing.core.alglist import algList

pluginPath = os.path.split(os.path.dirname(__file__))[0]


class AddModelFromFileAction(ToolboxAction):

    def __init__(self):
        self.name, self.i18n_name = self.trAction('Add model from file')
        self.group, self.i18n_group = self.trAction('Tools')

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'model.png'))

    def execute(self):
        settings = QSettings()
        lastDir = settings.value('Processing/lastModelsDir', '')
        filename = QFileDialog.getOpenFileName(self.toolbox,
                                               self.tr('Open model', 'AddModelFromFileAction'), lastDir,
                                               self.tr('Processing model files (*.model *.MODEL)', 'AddModelFromFileAction'))
        if filename:
            try:
                settings.setValue('Processing/lastModelsDir',
                                  QFileInfo(filename).absoluteDir().absolutePath())

                ModelerAlgorithm.fromFile(filename)
            except WrongModelException:
                QMessageBox.warning(
                    self.toolbox,
                    self.tr('Error reading model', 'AddModelFromFileAction'),
                    self.tr('The selected file does not contain a valid model', 'AddModelFromFileAction'))
                return
            except:
                QMessageBox.warning(self.toolbox,
                                    self.tr('Error reading model', 'AddModelFromFileAction'),
                                    self.tr('Cannot read file', 'AddModelFromFileAction'))
                return
            destFilename = os.path.join(ModelerUtils.modelsFolder(), os.path.basename(filename))
            shutil.copyfile(filename, destFilename)
            algList.reloadProvider('model')
