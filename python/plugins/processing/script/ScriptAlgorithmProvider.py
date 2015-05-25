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

import os

from PyQt4.QtGui import QIcon

from processing.core.ProcessingConfig import ProcessingConfig, Setting
from processing.core.AlgorithmProvider import AlgorithmProvider
from processing.gui.EditScriptAction import EditScriptAction
from processing.gui.DeleteScriptAction import DeleteScriptAction
from processing.gui.CreateNewScriptAction import CreateNewScriptAction
from processing.script.ScriptUtils import ScriptUtils
from processing.script.AddScriptFromFileAction import AddScriptFromFileAction
from processing.gui.GetScriptsAndModels import GetScriptsAction

pluginPath = os.path.split(os.path.dirname(__file__))[0]


class ScriptAlgorithmProvider(AlgorithmProvider):

    def __init__(self):
        AlgorithmProvider.__init__(self)
        self.actions.extend([CreateNewScriptAction(self.tr('Create new script', 'ScriptAlgorithmProvider'),
                            CreateNewScriptAction.SCRIPT_PYTHON),
                            AddScriptFromFileAction(),
                            GetScriptsAction()])
        self.contextMenuActions = \
            [EditScriptAction(EditScriptAction.SCRIPT_PYTHON),
             DeleteScriptAction(DeleteScriptAction.SCRIPT_PYTHON)]

    def initializeSettings(self):
        AlgorithmProvider.initializeSettings(self)
        ProcessingConfig.addSetting(Setting(self.getDescription(),
                                    ScriptUtils.SCRIPTS_FOLDER,
                                    self.tr('Scripts folder', 'ScriptAlgorithmProvider'),
                                    ScriptUtils.scriptsFolder()))

    def unload(self):
        AlgorithmProvider.unload(self)
        ProcessingConfig.addSetting(ScriptUtils.SCRIPTS_FOLDER)

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'script.png'))

    def getName(self):
        return 'script'

    def getDescription(self):
        return self.tr('Scripts', 'ScriptAlgorithmProvider')

    def _loadAlgorithms(self):
        folder = ScriptUtils.scriptsFolder()
        self.algs = ScriptUtils.loadFromFolder(folder)
