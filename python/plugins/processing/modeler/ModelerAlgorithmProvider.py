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

import os.path
from PyQt4.QtCore import *
from PyQt4.QtGui import *
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


class ModelerAlgorithmProvider(AlgorithmProvider):

    def __init__(self):
        AlgorithmProvider.__init__(self)
        self.actions = [CreateNewModelAction(), AddModelFromFileAction(), GetModelsAction()]
        self.contextMenuActions = [EditModelAction(), DeleteModelAction()]

    def initializeSettings(self):
        AlgorithmProvider.initializeSettings(self)
        ProcessingConfig.addSetting(Setting(self.getDescription(),
                                    ModelerUtils.MODELS_FOLDER, 'Models folder'
                                    , ModelerUtils.modelsFolder()))

    def setAlgsList(self, algs):
        ModelerUtils.allAlgs = algs

    def modelsFolder(self):
        return ModelerUtils.modelsFolder()

    def getDescription(self):
        return 'Models'

    def getName(self):
        return 'model'

    def getIcon(self):
        return QIcon(os.path.dirname(__file__) + '/../images/model.png')

    def _loadAlgorithms(self):
        folder = ModelerUtils.modelsFolder()
        self.loadFromFolder(folder)

    def loadFromFolder(self, folder):
        if not os.path.exists(folder):
            return
        for path, subdirs, files in os.walk(folder):
            for descriptionFile in files:
                if descriptionFile.endswith('model'):
                    try:
                        fullpath = os.path.join(path, descriptionFile)
                        alg = ModelerAlgorithm.fromFile(fullpath)
                        alg.provider = self
                        self.algs.append(alg)
                    except WrongModelException, e:
                        ProcessingLog.addToLog(ProcessingLog.LOG_ERROR,
	                            'Could not load model ' + descriptionFile + '\n'
	                            + e.msg)
