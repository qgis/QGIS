# -*- coding: utf-8 -*-

"""
***************************************************************************
    OpenModelFromFileAction.py
    ---------------------
    Date                 : February 2018
    Copyright            : (C) 2018 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Nyall Dawson'
__date__ = 'February 2018'
__copyright__ = '(C) 2018, Nyall Dawson'

import os
from qgis.PyQt.QtWidgets import QFileDialog
from qgis.PyQt.QtCore import QFileInfo, QCoreApplication, QDir

from qgis.core import QgsApplication, QgsSettings
from qgis.utils import iface
from processing.gui.ToolboxAction import ToolboxAction
from processing.modeler.ModelerDialog import ModelerDialog

pluginPath = os.path.split(os.path.dirname(__file__))[0]


class OpenModelFromFileAction(ToolboxAction):

    def __init__(self):
        self.name = QCoreApplication.translate('OpenModelFromFileAction', 'Open Existing Model…')
        self.group = self.tr('Tools')

    def getIcon(self):
        return QgsApplication.getThemeIcon("/processingModel.svg")

    def execute(self):
        settings = QgsSettings()
        lastDir = settings.value('Processing/lastModelsDir', QDir.homePath())
        filename, selected_filter = QFileDialog.getOpenFileName(self.toolbox,
                                                                self.tr('Open Model', 'AddModelFromFileAction'), lastDir,
                                                                self.tr('Processing models (*.model3 *.MODEL3)', 'AddModelFromFileAction'))
        if filename:
            settings.setValue('Processing/lastModelsDir',
                              QFileInfo(filename).absoluteDir().absolutePath())

            dlg = ModelerDialog.create()
            dlg.loadModel(filename)
            dlg.show()
            dlg.activate()
