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

import os
import shutil
from qgis.PyQt.QtWidgets import QFileDialog, QMessageBox
from qgis.PyQt.QtCore import QFileInfo, QCoreApplication

from qgis.core import QgsApplication, QgsSettings, QgsProcessingModelAlgorithm

from processing.gui.ToolboxAction import ToolboxAction
from processing.modeler.exceptions import WrongModelException
from processing.modeler.ModelerUtils import ModelerUtils

pluginPath = os.path.split(os.path.dirname(__file__))[0]


class AddModelFromFileAction(ToolboxAction):

    def __init__(self):
        self.name = QCoreApplication.translate('AddModelFromFileAction', 'Add Model to Toolbox…')
        self.group = self.tr('Tools')

    def getIcon(self):
        return QgsApplication.getThemeIcon("/processingModel.svg")

    def execute(self):
        settings = QgsSettings()
        lastDir = settings.value('Processing/lastModelsDir', '')
        filename, selected_filter = QFileDialog.getOpenFileName(self.toolbox,
                                                                self.tr('Open Model', 'AddModelFromFileAction'), lastDir,
                                                                self.tr('Processing models (*.model3 *.MODEL3)', 'AddModelFromFileAction'))
        if filename:
            settings.setValue('Processing/lastModelsDir',
                              QFileInfo(filename).absoluteDir().absolutePath())

            alg = QgsProcessingModelAlgorithm()
            if not alg.fromFile(filename):
                QMessageBox.warning(
                    self.toolbox,
                    self.tr('Open Model', 'AddModelFromFileAction'),
                    self.tr('The selected file does not contain a valid model', 'AddModelFromFileAction'))
                return
            destFilename = os.path.join(ModelerUtils.modelsFolders()[0], os.path.basename(filename))
            shutil.copyfile(filename, destFilename)
            QgsApplication.processingRegistry().providerById('model').refreshAlgorithms()
