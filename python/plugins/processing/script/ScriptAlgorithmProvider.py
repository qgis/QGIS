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

import os.path

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from processing.core.ProcessingConfig import ProcessingConfig, Setting
from processing.core.ProcessingLog import ProcessingLog
from processing.core.AlgorithmProvider import AlgorithmProvider

from processing.gui.EditScriptAction import EditScriptAction
from processing.gui.DeleteScriptAction import DeleteScriptAction
from processing.gui.CreateNewScriptAction import CreateNewScriptAction

from processing.script.ScriptAlgorithm import ScriptAlgorithm
from processing.script.ScriptUtils import ScriptUtils
from processing.script.WrongScriptException import WrongScriptException

import processing.resources_rc

class ScriptAlgorithmProvider(AlgorithmProvider):

    def __init__(self):
        AlgorithmProvider.__init__(self)
        #self.actions.append(CreateNewScriptAction("Create new script", CreateNewScriptAction.SCRIPT_PYTHON))
        self.contextMenuActions = [EditScriptAction(EditScriptAction.SCRIPT_PYTHON),
                                   DeleteScriptAction(DeleteScriptAction.SCRIPT_PYTHON)
                                  ]

    def initializeSettings(self):
        AlgorithmProvider.initializeSettings(self)
        ProcessingConfig.addSetting(Setting(self.getDescription(), ScriptUtils.SCRIPTS_FOLDER, "Scripts folder", ScriptUtils.scriptsFolder()))

    def unload(self):
        AlgorithmProvider.unload(self)
        ProcessingConfig.addSetting(ScriptUtils.SCRIPTS_FOLDER)

    def getIcon(self):
        return QIcon(":/processing/images/script.png")

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
                except WrongScriptException, e:
                    ProcessingLog.addToLog(ProcessingLog.LOG_ERROR, e.msg)
                except Exception, e:
                    ProcessingLog.addToLog(ProcessingLog.LOG_ERROR, "Could not load script:" + descriptionFile + "\n" + unicode(e))
