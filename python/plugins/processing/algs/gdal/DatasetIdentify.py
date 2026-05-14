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

from osgeo import gdal
from qgis.core import (
    Qgis,
    QgsProcessing,
    QgsProcessingException,
    QgsProcessingParameterBoolean,
    QgsProcessingParameterFile,
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
                self.tr("Perform recursive exploration of the input folder"),
                defaultValue=True,
            )
        )

        detailsParam = QgsProcessingParameterBoolean(
            self.DETAILS,
            self.tr("Add details about identified datasets in the output"),
            defaultValue=True,
        )
        detailsParam.setHelp(
            self.tr(
                "Reports if the file is a Cloud Optimized GeoTIFF (COG), the list of files that compose the dataset, if it has georeferencing (has_geotransform and has_crs fields) and overviews"
            )
        )
        self.addParameter(detailsParam)

        self.addParameter(
            QgsProcessingParameterVectorDestination(
                self.OUTPUT,
                self.tr("Output file"),
                QgsProcessing.SourceType.TypeVector,
            )
        )

    def name(self):
        return "dataset_identify"

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
                self.tr("Directory {} does not exist").format(inDirectory)
            )
        arguments.append("--input")
        arguments.append(inDirectory)

        outFile = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        self.setOutputValue(self.OUTPUT, outFile)
        arguments.append("--output")
        arguments.append(outFile)

        return [self.commandName(), GdalUtils.escapeAndJoin(arguments)]

    def processAlgorithm(self, parameters, context, feedback):
        if int(gdal.VersionInfo()) < 3130000:
            raise QgsProcessingException(
                self.tr("GDAL 3.13 or later is needed to run this algorithm")
            )
        return super().processAlgorithm(parameters, context, feedback)

    def shortHelpString(self):
        return self.tr(
            "This algorithm reports the name of GDAL drivers that can open "
            "files contained in a folder, with optional additional details, "
            "and write the result into an output vector layer."
        )

    def shortDescription(self):
        return self.tr(
            "Reports the name of GDAL drivers that can open files contained in a folder, "
            "with optional details."
        )
