"""
***************************************************************************
    EditScriptAction.py
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
__date__ = "April 2014"
__copyright__ = "(C) 201, Victor Olaya"

import os
import shutil

from qgis.core import (
    QgsApplication,
    QgsProcessingModelAlgorithm,
    QgsProcessingUtils,
    QgsSettings,
)
from qgis.gui import QgsProcessingToolboxAction
from qgis.PyQt.QtCore import QCoreApplication, QDir, QFileInfo
from qgis.PyQt.QtWidgets import QFileDialog, QMessageBox

pluginPath = os.path.split(os.path.dirname(__file__))[0]


class AddModelFromFileAction(QgsProcessingToolboxAction):
    def __init__(self):
        super().__init__(
            QCoreApplication.translate(
                "AddModelFromFileAction", "Add Model to Toolbox…"
            ),
            QCoreApplication.translate("ToolboxAction", "Tools"),
        )

    def icon(self):
        return QgsApplication.getThemeIcon("/processingModel.svg")

    def trigger(self, context):
        settings = QgsSettings()
        lastDir = settings.value("Processing/lastModelsDir", QDir.homePath())
        filename, selected_filter = QFileDialog.getOpenFileName(
            context.parentWidget(),
            QCoreApplication.translate("AddModelFromFileAction", "Open Model"),
            lastDir,
            QCoreApplication.translate(
                "AddModelFromFileAction", "Processing models (*.model3 *.MODEL3)"
            ),
        )
        if filename:
            settings.setValue(
                "Processing/lastModelsDir",
                QFileInfo(filename).absoluteDir().absolutePath(),
            )

            alg = QgsProcessingModelAlgorithm()
            if not alg.fromFile(filename):
                QMessageBox.warning(
                    context.parentWidget(),
                    QCoreApplication.translate("AddModelFromFileAction", "Open Model"),
                    QCoreApplication.translate(
                        "AddModelFromFileAction",
                        "The selected file does not contain a valid model",
                    ),
                )
                return

            if (
                QgsApplication.instance()
                .processingRegistry()
                .algorithmById(f"model:{alg.id()}")
            ):
                QMessageBox.warning(
                    context.parentWidget(),
                    QCoreApplication.translate("AddModelFromFileAction", "Open Model"),
                    QCoreApplication.translate(
                        "AddModelFromFileAction",
                        "Model with the same name already exists",
                    ),
                )
                return

            destFilename = os.path.join(
                QgsProcessingUtils.modelFolders()[0], os.path.basename(filename)
            )
            if os.path.exists(destFilename):
                reply = QMessageBox.question(
                    context.parentWidget(),
                    QCoreApplication.translate("AddModelFromFileAction", "Open Model"),
                    QCoreApplication.translate(
                        "AddModelFromFileAction",
                        "There is already a model file with the same name. Overwrite?",
                    ),
                    QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No,
                    QMessageBox.StandardButton.No,
                )

                if reply == QMessageBox.StandardButton.No:
                    return

            shutil.copyfile(filename, destFilename)
            QgsApplication.processingRegistry().providerById(
                "model"
            ).refreshAlgorithms()
