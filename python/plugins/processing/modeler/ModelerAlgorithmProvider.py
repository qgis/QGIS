# -*- coding: utf-8 -*-

"""
***************************************************************************
    ModelerAlgorithmProvider.py
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

from qgis.PyQt.QtGui import QIcon

from processing.core.AlgorithmProvider import AlgorithmProvider
from processing.core.ProcessingConfig import ProcessingConfig, Setting
from processing.core.ProcessingLog import ProcessingLog
from processing.modeler.ModelerUtils import ModelerUtils
from processing.modeler.ModelerAlgorithm import ModelerAlgorithm
from processing.modeler.WrongModelException import WrongModelException
from processing.modeler.EditModelAction import EditModelAction
from processing.modeler.CreateNewModelAction import CreateNewModelAction
from processing.modeler.DeleteModelAction import DeleteModelAction
from processing.modeler.AddModelFromFileAction import AddModelFromFileAction
from processing.gui.GetScriptsAndModels import GetModelsAction

pluginPath = os.path.split(os.path.dirname(__file__))[0]


class ModelerAlgorithmProvider(AlgorithmProvider):

    def __init__(self):
        super().__init__()
        self.actions = [CreateNewModelAction(), AddModelFromFileAction(), GetModelsAction()]
        self.contextMenuActions = [EditModelAction(), DeleteModelAction()]

    def initializeSettings(self):
        AlgorithmProvider.initializeSettings(self)
        ProcessingConfig.addSetting(Setting(self.name(),
                                            ModelerUtils.MODELS_FOLDER, self.tr('Models folder', 'ModelerAlgorithmProvider'),
                                            ModelerUtils.defaultModelsFolder(), valuetype=Setting.MULTIPLE_FOLDERS))

    def modelsFolder(self):
        return ModelerUtils.modelsFolders()[0]

    def name(self):
        return self.tr('Models', 'ModelerAlgorithmProvider')

    def id(self):
        return 'model'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'model.svg'))

    def _loadAlgorithms(self):
        folders = ModelerUtils.modelsFolders()
        self.algs = []
        for f in folders:
            self.loadFromFolder(f)

    def loadFromFolder(self, folder):
        if not os.path.exists(folder):
            return
        for path, subdirs, files in os.walk(folder):
            for descriptionFile in files:
                if descriptionFile.endswith('model'):
                    try:
                        fullpath = os.path.join(path, descriptionFile)
                        alg = ModelerAlgorithm.fromFile(fullpath)
                        if alg.name:
                            alg.provider = self
                            alg.descriptionFile = fullpath
                            self.algs.append(alg)
                        else:
                            ProcessingLog.addToLog(ProcessingLog.LOG_ERROR,
                                                   self.tr('Could not load model %s', 'ModelerAlgorithmProvider') % descriptionFile)
                    except WrongModelException as e:
                        ProcessingLog.addToLog(ProcessingLog.LOG_ERROR,
                                               self.tr('Could not load model %s\n%s', 'ModelerAlgorithmProvider') % (descriptionFile, e.msg))
