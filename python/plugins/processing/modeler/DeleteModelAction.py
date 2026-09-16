"""
***************************************************************************
    DeleteModelAction.py
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

from qgis.core import QgsApplication, QgsProcessing, QgsProject
from qgis.gui import QgsProcessingToolboxContextAction
from qgis.PyQt.QtCore import QCoreApplication
from qgis.PyQt.QtWidgets import QMessageBox


class DeleteModelAction(QgsProcessingToolboxContextAction):
    def __init__(self):
        super().__init__(
            QCoreApplication.translate("DeleteModelAction", "Delete Model…")
        )

    def isCompatibleWithAlgorithm(self, provider_id: str, algorithm_name: str):
        return provider_id in ("model", "project")

    def trigger(self, context):
        algorithm_name = context.algorithmName()

        is_project_provider = context.providerId() == QgsProcessing.PROJECT_PROVIDER_ID
        provider = QgsApplication.processingRegistry().providerById(
            context.providerId()
        )
        if provider is None:
            return

        model = provider.algorithm(algorithm_name)
        if model is None:
            return  # shouldn't happen, but let's be safe

        if is_project_provider:
            msg = QCoreApplication.translate(
                "DeleteModelAction",
                "Are you sure you want to delete this model from the current project?",
            )
        else:
            msg = QCoreApplication.translate(
                "DeleteModelAction", "Are you sure you want to delete this model?"
            )

        reply = QMessageBox.question(
            None,
            QCoreApplication.translate("DeleteModelAction", "Delete Model"),
            msg,
            QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No,
            QMessageBox.StandardButton.No,
        )

        if reply == QMessageBox.StandardButton.Yes:
            if is_project_provider:
                provider.removeModel(model)
                QgsProject.instance().setDirty(True)
            else:
                os.remove(model.sourceFilePath())
                QgsApplication.processingRegistry().providerById(
                    "model"
                ).refreshAlgorithms()
