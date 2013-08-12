# -*- coding: utf-8 -*-

"""
***************************************************************************
    RAlgorithmProvider.py
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
from processing.script.WrongScriptException import WrongScriptException
from processing.r.DeleteRScriptAction import DeleteRScriptAction
from processing.core.ProcessingConfig import ProcessingConfig, Setting
from processing.core.ProcessingLog import ProcessingLog
from processing.core.AlgorithmProvider import AlgorithmProvider
from PyQt4 import QtGui
from processing.r.RUtils import RUtils
from processing.r.RAlgorithm import RAlgorithm
from processing.r.CreateNewRScriptAction import CreateNewRScriptAction
from processing.r.EditRScriptAction import EditRScriptAction
from processing.core.ProcessingUtils import ProcessingUtils

class RAlgorithmProvider(AlgorithmProvider):

    def __init__(self):
        AlgorithmProvider.__init__(self)
        self.activate = False
        self.actions.append(CreateNewRScriptAction())
        self.contextMenuActions = [EditRScriptAction(), DeleteRScriptAction()]

    def initializeSettings(self):
        AlgorithmProvider.initializeSettings(self)
        ProcessingConfig.addSetting(Setting(self.getDescription(), RUtils.RSCRIPTS_FOLDER, "R Scripts folder", RUtils.RScriptsFolder()))
        if ProcessingUtils.isWindows():
            ProcessingConfig.addSetting(Setting(self.getDescription(), RUtils.R_FOLDER, "R folder", RUtils.RFolder()))
            ProcessingConfig.addSetting(Setting(self.getDescription(), RUtils.R_USE64, "Use 64 bit version", False))

    def unload(self):
        AlgorithmProvider.unload(self)
        ProcessingConfig.removeSetting(RUtils.RSCRIPTS_FOLDER)
        if ProcessingUtils.isWindows():
            ProcessingConfig.removeSetting(RUtils.R_FOLDER)
            ProcessingConfig.removeSetting(RUtils.R_USE64)

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/../images/r.png")


    def getDescription(self):
        return "R scripts"

    def getName(self):
        return "r"

    def _loadAlgorithms(self):
        folder = RUtils.RScriptsFolder()
        self.loadFromFolder(folder)
        folder = os.path.join(os.path.dirname(__file__), "scripts")
        self.loadFromFolder(folder)


    def loadFromFolder(self, folder):
        if not os.path.exists(folder):
            return
        for descriptionFile in os.listdir(folder):
            if descriptionFile.endswith("rsx"):
                try:
                    fullpath = os.path.join(folder, descriptionFile)
                    alg = RAlgorithm(fullpath)
                    if alg.name.strip() != "":
                        self.algs.append(alg)
                except WrongScriptException,e:
                    ProcessingLog.addToLog(ProcessingLog.LOG_ERROR,e.msg)
                except Exception, e:
                    ProcessingLog.addToLog(ProcessingLog.LOG_ERROR,"Could not load R script:" + descriptionFile + "\n" + str(e))



