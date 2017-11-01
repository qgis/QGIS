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

from qgis.core import (QgsApplication,
                       QgsProcessingProvider)

from processing.core.ProcessingConfig import ProcessingConfig, Setting
from processing.gui.EditScriptAction import EditScriptAction
from processing.gui.DeleteScriptAction import DeleteScriptAction
from processing.gui.CreateNewScriptAction import CreateNewScriptAction
from processing.script.ScriptUtils import ScriptUtils
from processing.script.AddScriptFromFileAction import AddScriptFromFileAction
from processing.gui.GetScriptsAndModels import GetScriptsAction
from processing.gui.ProviderActions import (ProviderActions,
                                            ProviderContextMenuActions)
from processing.script.CreateScriptCollectionPluginAction import CreateScriptCollectionPluginAction

pluginPath = os.path.split(os.path.dirname(__file__))[0]


class ScriptAlgorithmProvider(QgsProcessingProvider):

    def __init__(self):
        super().__init__()
        self.algs = []
        self.folder_algorithms = []
        self.actions = [CreateNewScriptAction('Create new script',
                                              CreateNewScriptAction.SCRIPT_PYTHON),
                        AddScriptFromFileAction(),
                        GetScriptsAction(),
                        CreateScriptCollectionPluginAction()]
        self.contextMenuActions = \
            [EditScriptAction(EditScriptAction.SCRIPT_PYTHON),
             DeleteScriptAction(DeleteScriptAction.SCRIPT_PYTHON)]

    def load(self):
        ProcessingConfig.settingIcons[self.name()] = self.icon()
        ProcessingConfig.addSetting(Setting(self.name(),
                                            ScriptUtils.SCRIPTS_FOLDER,
                                            self.tr('Scripts folder', 'ScriptAlgorithmProvider'),
                                            ScriptUtils.defaultScriptsFolder(), valuetype=Setting.MULTIPLE_FOLDERS))
        ProviderActions.registerProviderActions(self, self.actions)
        ProviderContextMenuActions.registerProviderContextMenuActions(self.contextMenuActions)
        ProcessingConfig.readSettings()
        self.refreshAlgorithms()
        return True

    def unload(self):
        ProcessingConfig.removeSetting(ScriptUtils.SCRIPTS_FOLDER)
        ProviderActions.deregisterProviderActions(self)
        ProviderContextMenuActions.deregisterProviderContextMenuActions(self.contextMenuActions)

    def icon(self):
        return QgsApplication.getThemeIcon("/processingScript.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("processingScript.svg")

    def id(self):
        return 'script'

    def name(self):
        return self.tr('Scripts', 'ScriptAlgorithmProvider')

    def loadAlgorithms(self):
        self.algs = []
        folders = ScriptUtils.scriptsFolders()
        for f in folders:
            self.algs.extend(ScriptUtils.loadFromFolder(f))
        self.algs.extend(self.folder_algorithms)
        for a in self.algs:
            self.addAlgorithm(a)

    def addAlgorithmsFromFolder(self, folder):
        self.folder_algorithms.extend(ScriptUtils.loadFromFolder(folder))
