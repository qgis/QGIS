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

from PyQt4.QtCore import *
from PyQt4.QtGui import *
import os.path
from sextante.core.SextanteConfig import SextanteConfig, Setting
from sextante.modeler.SaveAsPythonScriptAction import SaveAsPythonScriptAction
from sextante.core.SextanteLog import SextanteLog
from sextante.modeler.ModelerUtils import ModelerUtils
from sextante.modeler.ModelerAlgorithm import ModelerAlgorithm
from sextante.modeler.WrongModelException import WrongModelException
from sextante.modeler.EditModelAction import EditModelAction
from sextante.modeler.CreateNewModelAction import CreateNewModelAction
from sextante.core.AlgorithmProvider import AlgorithmProvider
from PyQt4 import QtGui
from sextante.modeler.DeleteModelAction import DeleteModelAction

class ModelerAlgorithmProvider(AlgorithmProvider):

    def __init__(self):
        AlgorithmProvider.__init__(self)
        #self.actions = [CreateNewModelAction()]
        self.contextMenuActions = [EditModelAction(), DeleteModelAction(), SaveAsPythonScriptAction()]

    def initializeSettings(self):
        AlgorithmProvider.initializeSettings(self)
        SextanteConfig.addSetting(Setting(self.getDescription(), ModelerUtils.MODELS_FOLDER, "Models folder", ModelerUtils.modelsFolder()))

    def setAlgsList(self, algs):
        ModelerUtils.allAlgs = algs

    def modelsFolder(self):
        return ModelerUtils.modelsFolder()

    def getDescription(self):
        return "Models"

    def getName(self):
        return "model"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/../images/model.png")

    def _loadAlgorithms(self):
        folder = ModelerUtils.modelsFolder()
        self.loadFromFolder(folder)
        folder = os.path.join(os.path.dirname(__file__), "models")
        self.loadFromFolder(folder)

    def loadFromFolder(self,folder):
        if not os.path.exists(folder):
            return
        for descriptionFile in os.listdir(folder):
            if descriptionFile.endswith("model"):
                try:
                    alg = ModelerAlgorithm()
                    fullpath = os.path.join(folder ,descriptionFile)
                    alg.openModel(fullpath)
                    if alg.name.strip() != "":
                        alg.provider = self
                        self.algs.append(alg)
                except WrongModelException,e:
                    SextanteLog.addToLog(SextanteLog.LOG_ERROR,"Could not load model " + descriptionFile + "\n" + e.msg)