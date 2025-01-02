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

__author__ = "Victor Olaya"
__date__ = "August 2012"
__copyright__ = "(C) 2012, Victor Olaya"

import os

from qgis.core import (
    Qgis,
    QgsApplication,
    QgsProcessingProvider,
    QgsMessageLog,
    QgsProcessingModelAlgorithm,
    QgsRuntimeProfiler,
)

from processing.core.ProcessingConfig import ProcessingConfig, Setting

from processing.gui.ContextAction import ContextAction
from processing.gui.ProviderActions import ProviderActions, ProviderContextMenuActions

from processing.modeler.AddModelFromFileAction import AddModelFromFileAction
from processing.modeler.CreateNewModelAction import CreateNewModelAction
from processing.modeler.DeleteModelAction import DeleteModelAction
from processing.modeler.EditModelAction import EditModelAction
from processing.modeler.ExportModelAsPythonScriptAction import (
    ExportModelAsPythonScriptAction,
)
from processing.modeler.OpenModelFromFileAction import OpenModelFromFileAction
from processing.modeler.ModelerUtils import ModelerUtils

pluginPath = os.path.split(os.path.dirname(__file__))[0]


class ModelerAlgorithmProvider(QgsProcessingProvider):

    def __init__(self):
        super().__init__()
        self.actions = [
            CreateNewModelAction(),
            OpenModelFromFileAction(),
            AddModelFromFileAction(),
        ]
        sep_action = ContextAction()
        sep_action.is_separator = True
        self.contextMenuActions = [
            EditModelAction(),
            DeleteModelAction(),
            sep_action,
            ExportModelAsPythonScriptAction(),
        ]
        self.algs = []
        self.isLoading = False

        # must reload models if providers list is changed - previously unavailable algorithms
        # which models depend on may now be available
        QgsApplication.processingRegistry().providerAdded.connect(self.onProviderAdded)

    def onProviderAdded(self, provider_id):
        if provider_id == self.id():
            return

        self.refreshAlgorithms()

    def load(self):
        with QgsRuntimeProfiler.profile("Model Provider"):
            ProcessingConfig.settingIcons[self.name()] = self.icon()
            ProcessingConfig.addSetting(
                Setting(
                    self.name(),
                    ModelerUtils.MODELS_FOLDER,
                    self.tr("Models folder", "ModelerAlgorithmProvider"),
                    ModelerUtils.defaultModelsFolder(),
                    valuetype=Setting.MULTIPLE_FOLDERS,
                )
            )
            ProviderActions.registerProviderActions(self, self.actions)
            ProviderContextMenuActions.registerProviderContextMenuActions(
                self.contextMenuActions
            )
            ProcessingConfig.readSettings()
            self.refreshAlgorithms()

        return True

    def unload(self):
        ProviderActions.deregisterProviderActions(self)
        ProviderContextMenuActions.deregisterProviderContextMenuActions(
            self.contextMenuActions
        )

    def modelsFolder(self):
        return ModelerUtils.modelsFolders()[0]

    def name(self):
        return self.tr("Models", "ModelerAlgorithmProvider")

    def id(self):
        return "model"

    def icon(self):
        return QgsApplication.getThemeIcon("/processingModel.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("processingModel.svg")

    def supportsNonFileBasedOutput(self):
        return True

    def loadAlgorithms(self):
        with QgsRuntimeProfiler.profile("Load model algorithms"):
            if self.isLoading:
                return
            self.isLoading = True
            self.algs = []
            folders = ModelerUtils.modelsFolders()
            for f in folders:
                self.loadFromFolder(f)
            for a in self.algs:
                self.addAlgorithm(a)
            self.isLoading = False

    def loadFromFolder(self, folder):
        if not os.path.exists(folder):
            return
        for path, subdirs, files in os.walk(folder):
            for descriptionFile in files:
                if descriptionFile.endswith("model3"):
                    fullpath = os.path.join(path, descriptionFile)

                    alg = QgsProcessingModelAlgorithm()
                    if alg.fromFile(fullpath):
                        if alg.name():
                            alg.setSourceFilePath(fullpath)
                            self.algs.append(alg)
                    else:
                        QgsMessageLog.logMessage(
                            self.tr(
                                "Could not load model {0}", "ModelerAlgorithmProvider"
                            ).format(descriptionFile),
                            self.tr("Processing"),
                            Qgis.MessageLevel.Critical,
                        )
