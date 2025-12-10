"""
***************************************************************************
    DatasetIdentify.py
    ------------------
    Date                 : December 2025
    Copyright            : (C) 2025 by Even Rouault
    Email                : even dot rouault at spatialys dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Even Rouault"
__date__ = "December 2025"
__copyright__ = "(C) 2025, Even Rouault"

import os

from qgis.core import (
    Qgis,
    QgsProcessing,
    QgsProcessingException,
    QgsProcessingParameterFile,
    QgsProcessingParameterBoolean,
    QgsProcessingParameterVectorDestination,
)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class DatasetIdentify(GdalAlgorithm):
    INPUT = "INPUT"
    RECURSIVE = "RECURSIVE"
    DETAILS = "DETAILS"
    OUTPUT = "OUTPUT"

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(
            QgsProcessingParameterFile(
                self.INPUT,
                self.tr("Input folder"),
                Qgis.ProcessingFileParameterBehavior.Folder,
            )
        )

        self.addParameter(
            QgsProcessingParameterBoolean(
                self.RECURSIVE,
                self.tr(
                    "Perform recursive exploration of the input folder and its subfolders"
                ),
                defaultValue=True,
            )
        )

        self.addParameter(
            QgsProcessingParameterBoolean(
                self.DETAILS,
                self.tr("Add some details about identified datasets in the output"),
                defaultValue=True,
            )
        )

        self.addParameter(
            QgsProcessingParameterVectorDestination(
                self.OUTPUT,
                self.tr("Output file"),
                QgsProcessing.SourceType.TypeVector,
            )
        )

    def name(self):
        return "DatasetIdentify"

    def displayName(self):
        return self.tr("Dataset identification")

    def group(self):
        return self.tr("Miscellaneous")

    def groupId(self):
        return "miscellaneous"

    def commandName(self):
        return "gdal dataset identify"

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        arguments = []
        if self.parameterAsBoolean(parameters, self.RECURSIVE, context):
            arguments.append("--recursive")
        if self.parameterAsBoolean(parameters, self.DETAILS, context):
            arguments.append("--detailed")

        inDirectory = self.parameterAsFile(parameters, self.INPUT, context)
        if not os.path.exists(inDirectory):
            raise QgsProcessingException(
                self.tr(f"Directory {inDirectory} does not exist")
            )
        arguments.append("--input")
        arguments.append(inDirectory)

        outFile = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        self.setOutputValue(self.OUTPUT, outFile)
        arguments.append("--output")
        arguments.append(outFile)

        return [self.commandName(), GdalUtils.escapeAndJoin(arguments)]

    def shortHelpString(self):
        return self.tr(
            "The algorithm reports the name of GDAL drivers that can open "
            "files contained in a folder, with optional additional details, "
            "and write the result into an output vector layer.\n"
        )
