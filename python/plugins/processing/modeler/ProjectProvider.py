"""
***************************************************************************
    ProjectProvider.py
    ------------------------
    Date                 : July 2018
    Copyright            : (C) 2018 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Nyall Dawson"
__date__ = "July 2018"
__copyright__ = "(C) 2018, Nyall Dawson"

from qgis.core import (
    Qgis,
    QgsApplication,
    QgsProcessingProvider,
    QgsMessageLog,
    QgsProcessingModelAlgorithm,
    QgsProject,
    QgsXmlUtils,
    QgsRuntimeProfiler,
)

PROJECT_PROVIDER_ID = "project"


class ProjectProvider(QgsProcessingProvider):

    def __init__(self, project=None):
        super().__init__()
        if project is None:
            self.project = QgsProject.instance()
        else:
            self.project = project

        self.model_definitions = {}  # dict of models in project
        self.is_loading = False

        # must reload models if providers list is changed - previously unavailable algorithms
        # which models depend on may now be available
        QgsApplication.processingRegistry().providerAdded.connect(
            self.on_provider_added
        )

        self.project.readProject.connect(self.read_project)
        self.project.writeProject.connect(self.write_project)
        self.project.cleared.connect(self.clear)

    def on_provider_added(self, provider_id):
        if provider_id == self.id():
            return

        self.refreshAlgorithms()

    def load(self):
        with QgsRuntimeProfiler.profile("Project Provider"):
            self.refreshAlgorithms()

        return True

    def clear(self):
        """
        Remove all algorithms from the provider
        """
        self.model_definitions = {}
        self.refreshAlgorithms()

    def add_model(self, model):
        """
        Adds a model to the provider
        :type model: QgsProcessingModelAlgorithm
        :param model: model to add
        """
        definition = model.toVariant()
        self.model_definitions[model.name()] = definition
        self.refreshAlgorithms()

    def remove_model(self, model):
        """
        Removes a model from the project
        :type model: QgsProcessingModelAlgorithm
        :param model: model to remove
        """
        if model is None:
            return

        if model.name() in self.model_definitions:
            del self.model_definitions[model.name()]

        self.refreshAlgorithms()

    def read_project(self, doc):
        """
        Reads the project model definitions from the project DOM document
        :param doc: DOM document
        """
        self.model_definitions = {}
        project_models_nodes = doc.elementsByTagName("projectModels")
        if project_models_nodes:
            project_models_node = project_models_nodes.at(0)
            model_nodes = project_models_node.childNodes()
            for n in range(model_nodes.count()):
                model_element = model_nodes.at(n).toElement()
                definition = QgsXmlUtils.readVariant(model_element)
                algorithm = QgsProcessingModelAlgorithm()
                if algorithm.loadVariant(definition):
                    self.model_definitions[algorithm.name()] = definition

        self.refreshAlgorithms()

    def write_project(self, doc):
        """
        Writes out the project model definitions into the project DOM document
        :param doc: DOM document
        """
        qgis_nodes = doc.elementsByTagName("qgis")
        if not qgis_nodes:
            return

        qgis_node = qgis_nodes.at(0)
        project_models_node = doc.createElement("projectModels")

        for a in self.algorithms():
            definition = a.toVariant()
            element = QgsXmlUtils.writeVariant(definition, doc)
            project_models_node.appendChild(element)
        qgis_node.appendChild(project_models_node)

    def name(self):
        return self.tr("Project models", "ProjectProvider")

    def longName(self):
        return self.tr("Models embedded in the current project", "ProjectProvider")

    def id(self):
        return PROJECT_PROVIDER_ID

    def icon(self):
        return QgsApplication.getThemeIcon("/mIconQgsProjectFile.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("mIconQgsProjectFile.svg")

    def supportsNonFileBasedOutput(self):
        return True

    def loadAlgorithms(self):
        if self.is_loading:
            return
        self.is_loading = True

        for definition in self.model_definitions.values():
            algorithm = QgsProcessingModelAlgorithm()
            if algorithm.loadVariant(definition):
                self.addAlgorithm(algorithm)
            else:
                QgsMessageLog.logMessage(
                    self.tr("Could not load model from project", "ProjectProvider"),
                    self.tr("Processing"),
                    Qgis.MessageLevel.Critical,
                )

        self.is_loading = False
