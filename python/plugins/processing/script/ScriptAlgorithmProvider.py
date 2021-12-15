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

import os

from qgis.core import (Qgis,
                       QgsMessageLog,
                       QgsApplication,
                       QgsProcessingProvider,
                       QgsRuntimeProfiler)

from processing.core.ProcessingConfig import ProcessingConfig, Setting

from processing.gui.ProviderActions import (ProviderActions,
                                            ProviderContextMenuActions)

from processing.script.AddScriptFromFileAction import AddScriptFromFileAction
from processing.script.CreateNewScriptAction import CreateNewScriptAction
from processing.script.AddScriptFromTemplateAction import AddScriptFromTemplateAction
from processing.script.DeleteScriptAction import DeleteScriptAction
from processing.script.EditScriptAction import EditScriptAction
from processing.script.OpenScriptFromFileAction import OpenScriptFromFileAction
from processing.script import ScriptUtils
from processing.tools.system import userFolder


class ScriptAlgorithmProvider(QgsProcessingProvider):

    def __init__(self):
        super().__init__()
        self.algs = []
        self.additional_algorithm_classes = []
        self.actions = [CreateNewScriptAction(),
                        AddScriptFromTemplateAction(),
                        OpenScriptFromFileAction(),
                        AddScriptFromFileAction()
                        ]
        self.contextMenuActions = [EditScriptAction(),
                                   DeleteScriptAction()]

    def load(self):
        with QgsRuntimeProfiler.profile('Script Provider'):
            ProcessingConfig.settingIcons[self.name()] = self.icon()
            ProcessingConfig.addSetting(Setting(self.name(),
                                                ScriptUtils.SCRIPTS_FOLDERS,
                                                self.tr("Scripts folder(s)"),
                                                ScriptUtils.defaultScriptsFolder(),
                                                valuetype=Setting.MULTIPLE_FOLDERS))

            ProviderActions.registerProviderActions(self, self.actions)
            ProviderContextMenuActions.registerProviderContextMenuActions(self.contextMenuActions)

            ProcessingConfig.readSettings()
            self.refreshAlgorithms()

        return True

    def unload(self):
        ProcessingConfig.removeSetting(ScriptUtils.SCRIPTS_FOLDERS)

        ProviderActions.deregisterProviderActions(self)
        ProviderContextMenuActions.deregisterProviderContextMenuActions(self.contextMenuActions)

    def icon(self):
        return QgsApplication.getThemeIcon("/processingScript.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("processingScript.svg")

    def id(self):
        return "script"

    def name(self):
        return self.tr("Scripts")

    def supportsNonFileBasedOutput(self):
        # TODO - this may not be strictly true. We probably need a way for scripts
        # to indicate whether individual outputs support non-file based outputs,
        # but for now allow it. At best we expose nice features to users, at worst
        # they'll get an error if they use them with incompatible outputs...
        return True

    def add_algorithm_class(self, algorithm_class):
        """
        Adds an algorithm class to the provider
        """
        self.additional_algorithm_classes.append(algorithm_class)
        self.refreshAlgorithms()

    def loadAlgorithms(self):
        self.algs = []
        folders = ScriptUtils.scriptsFolders()
        # always add default script folder to the list
        defaultScriptFolder = ScriptUtils.defaultScriptsFolder()
        if defaultScriptFolder not in folders:
            folders.append(defaultScriptFolder)
        # load all scripts
        for folder in folders:
            folder = ScriptUtils.resetScriptFolder(folder)
            if not folder:
                continue

            for path, subdirs, files in os.walk(folder):
                for entry in files:
                    if entry.lower().endswith(".py"):
                        moduleName = os.path.splitext(os.path.basename(entry))[0]
                        filePath = os.path.abspath(os.path.join(path, entry))
                        alg = ScriptUtils.loadAlgorithm(moduleName, filePath)
                        if alg is not None:
                            self.algs.append(alg)

        for alg_class in self.additional_algorithm_classes:
            self.algs.append(alg_class())

        for a in self.algs:
            self.addAlgorithm(a)
