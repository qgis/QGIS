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

import os.path

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from processing.core.ProcessingConfig import ProcessingConfig, Setting
from processing.core.ProcessingLog import ProcessingLog
from processing.core.AlgorithmProvider import AlgorithmProvider
from processing.core.ProcessingUtils import ProcessingUtils

from processing.core.SextanteUtils import SextanteUtils

from processing.gui.EditScriptAction import EditScriptAction
from processing.gui.DeleteScriptAction import DeleteScriptAction
from processing.gui.CreateNewScriptAction import CreateNewScriptAction
p
from processing.r.RUtils import RUtils
from processing.r.RAlgorithm import RAlgorithm

from processing.script.WrongScriptException import WrongScriptException
from processing.tools.system import *
import processing.resources_rc

class RAlgorithmProvider(AlgorithmProvider):

    def __init__(self):
        AlgorithmProvider.__init__(self)
        self.activate = False
        #self.actions.append(CreateNewScriptAction("Create new R script", CreateNewScriptAction.SCRIPT_R))
        self.contextMenuActions = [EditScriptAction(EditScriptAction.SCRIPT_R),
                                   DeleteScriptAction(DeleteScriptAction.SCRIPT_R)
                                  ]

    def initializeSettings(self):
        AlgorithmProvider.initializeSettings(self)
        ProcessingConfig.addSetting(Setting(self.getDescription(), RUtils.RSCRIPTS_FOLDER, "R Scripts folder", RUtils.RScriptsFolder()))
        if isWindows():
            ProcessingConfig.addSetting(Setting(self.getDescription(), RUtils.R_FOLDER, "R folder", RUtils.RFolder()))
            ProcessingConfig.addSetting(Setting(self.getDescription(), RUtils.R_USE64, "Use 64 bit version", False))

    def unload(self):
        AlgorithmProvider.unload(self)
        ProcessingConfig.removeSetting(RUtils.RSCRIPTS_FOLDER)
        if isWindows():
            ProcessingConfig.removeSetting(RUtils.R_FOLDER)
            ProcessingConfig.removeSetting(RUtils.R_USE64)

    def getIcon(self):
        return QIcon(":/sextante/images/r.png")

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
                except WrongScriptException, e:
                    ProcessingLog.addToLog(SextanteLog.LOG_ERROR, e.msg)
                except Exception, e:
                    ProcessingLog.addToLog(SextanteLog.LOG_ERROR, "Could not load R script:" + descriptionFile + "\n" + unicode(e))



