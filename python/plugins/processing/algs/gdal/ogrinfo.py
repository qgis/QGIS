"""
***************************************************************************
    ogrinfo.py
    ---------------------
    Date                 : November 2012
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
__date__ = "November 2012"
__copyright__ = "(C) 2012, Victor Olaya"

from qgis.core import (
    QgsProcessingException,
    QgsProcessingParameterVectorLayer,
    QgsProcessingParameterBoolean,
    QgsProcessingParameterDefinition,
    QgsProcessingParameterString,
    QgsProcessingParameterFileDestination,
)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils


class ogrinfo(GdalAlgorithm):
    INPUT = "INPUT"
    ALL_LAYERS = "ALL_LAYERS"
    SUMMARY_ONLY = "SUMMARY_ONLY"
    NO_METADATA = "NO_METADATA"
    EXTRA = "EXTRA"
    OUTPUT = "OUTPUT"

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(
            QgsProcessingParameterVectorLayer(self.INPUT, self.tr("Input layer"))
        )
        self.addParameter(
            QgsProcessingParameterBoolean(
                self.ALL_LAYERS,
                self.tr("Enable listing of all layers in the dataset"),
                defaultValue=False,
            )
        )
        if self.name() == "ogrinfo":
            self.addParameter(
                QgsProcessingParameterBoolean(
                    self.SUMMARY_ONLY, self.tr("Summary output only"), defaultValue=True
                )
            )
        else:
            self.addParameter(
                QgsProcessingParameterBoolean(
                    self.FEATURES,
                    self.tr("Enable listing of features"),
                    defaultValue=False,
                )
            )

        self.addParameter(
            QgsProcessingParameterBoolean(
                self.NO_METADATA, self.tr("Suppress metadata info"), defaultValue=False
            )
        )

        extra_param = QgsProcessingParameterString(
            self.EXTRA,
            self.tr("Additional command-line parameters"),
            defaultValue=None,
            optional=True,
        )
        extra_param.setFlags(
            extra_param.flags() | QgsProcessingParameterDefinition.Flag.FlagAdvanced
        )
        self.addParameter(extra_param)

        if self.name() == "ogrinfo":
            self.addParameter(
                QgsProcessingParameterFileDestination(
                    self.OUTPUT,
                    self.tr("Layer information"),
                    self.tr("HTML files (*.html)"),
                )
            )
        else:
            self.addParameter(
                QgsProcessingParameterFileDestination(
                    self.OUTPUT,
                    self.tr("Layer information"),
                    self.tr("JSON files (*.json)"),
                )
            )

    def name(self):
        return "ogrinfo"

    def displayName(self):
        return self.tr("Vector information")

    def group(self):
        return self.tr("Vector miscellaneous")

    def groupId(self):
        return "vectormiscellaneous"

    def commandName(self):
        return "ogrinfo"

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        if self.name() == "ogrinfo":
            arguments = ["-al"]
        else:
            arguments = ["-json"]

        if self.name() == "ogrinfo":
            if self.parameterAsBoolean(parameters, self.SUMMARY_ONLY, context):
                arguments.append("-so")
        else:
            if self.parameterAsBoolean(parameters, self.FEATURES, context):
                arguments.append("-features")

        if self.parameterAsBoolean(parameters, self.NO_METADATA, context):
            arguments.append("-nomd")

        if self.EXTRA in parameters and parameters[self.EXTRA] not in (None, ""):
            extra = self.parameterAsString(parameters, self.EXTRA, context)
            arguments.append(extra)

        inLayer = self.parameterAsVectorLayer(parameters, self.INPUT, context)
        if inLayer is None:
            raise QgsProcessingException(
                self.invalidSourceError(parameters, self.INPUT)
            )

        input_details = self.getOgrCompatibleSource(
            self.INPUT, parameters, context, feedback, executing
        )
        arguments.append(input_details.connection_string)
        if not self.parameterAsBoolean(parameters, self.ALL_LAYERS, context):
            arguments.append(input_details.layer_name)

        if input_details.open_options:
            arguments.extend(input_details.open_options_as_arguments())

        if input_details.credential_options:
            arguments.extend(input_details.credential_options_as_arguments())

        return [self.commandName(), GdalUtils.escapeAndJoin(arguments)]

    def processAlgorithm(self, parameters, context, feedback):
        console_output = GdalUtils.runGdal(
            self.getConsoleCommands(parameters, context, feedback), feedback
        )
        output = self.parameterAsFileOutput(parameters, self.OUTPUT, context)
        with open(output, "w") as f:
            f.write("<pre>")
            for s in console_output[1:]:
                f.write(str(s))
            f.write("</pre>")

        return {self.OUTPUT: output}


class ogrinfojson(ogrinfo):
    FEATURES = "FEATURES"

    def name(self):
        return "ogrinfojson"

    def displayName(self):
        return self.tr("Vector information (JSON)")

    def processAlgorithm(self, parameters, context, feedback):
        console_output = GdalUtils.runGdal(
            self.getConsoleCommands(parameters, context, feedback)
        )
        output = self.parameterAsFileOutput(parameters, self.OUTPUT, context)
        with open(output, "w", newline="") as f:
            for s in console_output[1:]:
                f.write(str(s))

        return {self.OUTPUT: output}
