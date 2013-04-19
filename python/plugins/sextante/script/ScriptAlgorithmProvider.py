# -*- coding: utf-8 -*-

"""
***************************************************************************
    ScriptAlgorithmProvider.py
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

from sextante.script.CreateNewScriptAction import CreateNewScriptAction
from sextante.script.EditScriptAction import EditScriptAction
from PyQt4.QtCore import *
from PyQt4.QtGui import *
import os.path
from sextante.script.DeleteScriptAction import DeleteScriptAction
from sextante.script.ScriptAlgorithm import ScriptAlgorithm
from sextante.script.ScriptUtils import ScriptUtils
from sextante.script.WrongScriptException import WrongScriptException
from sextante.core.SextanteConfig import SextanteConfig, Setting
from sextante.core.SextanteLog import SextanteLog
from sextante.core.AlgorithmProvider import AlgorithmProvider
from PyQt4 import QtGui

class ScriptAlgorithmProvider(AlgorithmProvider):

    def __init__(self):
        AlgorithmProvider.__init__(self)
        #self.actions.append(CreateNewScriptAction())
        self.contextMenuActions = [EditScriptAction(), DeleteScriptAction()]

    def initializeSettings(self):
        AlgorithmProvider.initializeSettings(self)
        SextanteConfig.addSetting(Setting(self.getDescription(), ScriptUtils.SCRIPTS_FOLDER, "Scripts folder", ScriptUtils.scriptsFolder()))

    def unload(self):
        AlgorithmProvider.unload(self)
        SextanteConfig.addSetting(ScriptUtils.SCRIPTS_FOLDER)

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/../images/script.png")

    def getName(self):
        return "script"

    def getDescription(self):
        return "Scripts"

    def _loadAlgorithms(self):
        folder = ScriptUtils.scriptsFolder()
        self.loadFromFolder(folder)
        folder = os.path.join(os.path.dirname(__file__), "scripts")
        self.loadFromFolder(folder)

    def loadFromFolder(self, folder):
        if not os.path.exists(folder):
            return
        for descriptionFile in os.listdir(folder):
            if descriptionFile.endswith("py"):
                try:
                    fullpath = os.path.join(folder, descriptionFile)
                    alg = ScriptAlgorithm(fullpath)
                    if alg.name.strip() != "":
                        self.algs.append(alg)
                except WrongScriptException,e:
                    SextanteLog.addToLog(SextanteLog.LOG_ERROR,e.msg)



