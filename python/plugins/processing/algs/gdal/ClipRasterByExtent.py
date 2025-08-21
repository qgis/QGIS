"""
***************************************************************************
    ClipRasterByExtent.py
    ---------------------
    Date                 : September 2013
    Copyright            : (C) 2013 by Alexander Bruy
    Email                : alexander bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Alexander Bruy"
__date__ = "September 2013"
__copyright__ = "(C) 2013, Alexander Bruy"

import os
import tempfile

from qgis.PyQt.QtGui import QIcon

from qgis.core import (
    QgsDistanceArea,
    QgsRasterFileWriter,
    QgsProcessingException,
    QgsProcessingParameterDefinition,
    QgsProcessingParameterRasterLayer,
    QgsProcessingParameterEnum,
    QgsProcessingParameterExtent,
    QgsProcessingParameterString,
    QgsProcessingParameterNumber,
    QgsProcessingParameterBoolean,
    QgsProcessingParameterRasterDestination,
    QgsProcessingRasterLayerDefinition,
)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalConnectionDetails, GdalUtils

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class ClipRasterByExtent(GdalAlgorithm):
    INPUT = "INPUT"
    EXTENT = "PROJWIN"
    OVERCRS = "OVERCRS"
    NODATA = "NODATA"
    OPTIONS = "OPTIONS"
    CREATION_OPTIONS = "CREATION_OPTIONS"
    DATA_TYPE = "DATA_TYPE"
    EXTRA = "EXTRA"
    OUTPUT = "OUTPUT"

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):

        self.TYPES = [
            self.tr("Use Input Layer Data Type"),
            "Byte",
            "Int16",
            "UInt16",
            "UInt32",
            "Int32",
            "Float32",
            "Float64",
            "CInt16",
            "CInt32",
            "CFloat32",
            "CFloat64",
            "Int8",
        ]

        self.addParameter(
            QgsProcessingParameterRasterLayer(self.INPUT, self.tr("Input layer"))
        )
        self.addParameter(
            QgsProcessingParameterExtent(self.EXTENT, self.tr("Clipping extent"))
        )
        self.addParameter(
            QgsProcessingParameterBoolean(
                self.OVERCRS,
                self.tr("Override the projection for the output file"),
                defaultValue=False,
            )
        )
        self.addParameter(
            QgsProcessingParameterNumber(
                self.NODATA,
                self.tr("Assign a specified NoData value to output bands"),
                type=QgsProcessingParameterNumber.Type.Double,
                defaultValue=None,
                optional=True,
            )
        )

        # backwards compatibility parameter
        # TODO QGIS 4: remove parameter and related logic
        options_param = QgsProcessingParameterString(
            self.OPTIONS,
            self.tr("Additional creation options"),
            defaultValue="",
            optional=True,
        )
        options_param.setFlags(
            options_param.flags() | QgsProcessingParameterDefinition.Flag.Hidden
        )
        options_param.setMetadata({"widget_wrapper": {"widget_type": "rasteroptions"}})
        self.addParameter(options_param)

        creation_options_param = QgsProcessingParameterString(
            self.CREATION_OPTIONS,
            self.tr("Additional creation options"),
            defaultValue="",
            optional=True,
        )
        creation_options_param.setFlags(
            creation_options_param.flags()
            | QgsProcessingParameterDefinition.Flag.FlagAdvanced
        )
        creation_options_param.setMetadata(
            {"widget_wrapper": {"widget_type": "rasteroptions"}}
        )
        self.addParameter(creation_options_param)

        dataType_param = QgsProcessingParameterEnum(
            self.DATA_TYPE,
            self.tr("Output data type"),
            self.TYPES,
            allowMultiple=False,
            defaultValue=0,
        )
        dataType_param.setFlags(
            dataType_param.flags() | QgsProcessingParameterDefinition.Flag.FlagAdvanced
        )
        self.addParameter(dataType_param)

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

        self.addParameter(
            QgsProcessingParameterRasterDestination(
                self.OUTPUT, self.tr("Clipped (extent)")
            )
        )

    def name(self):
        return "cliprasterbyextent"

    def displayName(self):
        return self.tr("Clip raster by extent")

    def group(self):
        return self.tr("Raster extraction")

    def groupId(self):
        return "rasterextraction"

    def icon(self):
        return QIcon(os.path.join(pluginPath, "images", "gdaltools", "raster-clip.png"))

    def commandName(self):
        return "gdal_translate"

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        inLayer = self.parameterAsRasterLayer(parameters, self.INPUT, context)
        if inLayer is None:
            raise QgsProcessingException(
                "Invalid input layer {}".format(
                    parameters[self.INPUT] if self.INPUT in parameters else "INPUT"
                )
            )

        bbox = self.parameterAsExtent(parameters, self.EXTENT, context, inLayer.crs())
        if bbox.isNull() or bbox.isEmpty():
            raise QgsProcessingException(
                "Invalid extent {}".format(
                    parameters[self.EXTENT] if self.EXTENT in parameters else "EXTENT"
                )
            )

        arguments = []

        if inLayer.providerType() == "wms" and isinstance(
            parameters[self.INPUT], QgsProcessingRasterLayerDefinition
        ):
            # If the scale is greater than 0, we'll have a QgsProcessingRasterLayerDefinition
            param_def = parameters[self.INPUT]
            scale = param_def.referenceScale
            dpi = param_def.dpi

            distanceArea = None
            if inLayer.crs().isGeographic():
                distanceArea = QgsDistanceArea()
                distanceArea.setSourceCrs(inLayer.crs(), context.transformContext())
                distanceArea.setEllipsoid(context.ellipsoid())

            width, height = GdalUtils._wms_dimensions_for_scale(
                bbox, inLayer.crs(), scale, dpi, distanceArea
            )
            wms_description_file_path = tempfile.mktemp("_wms_description_file.xml")
            res_xml_wms, xml_wms_error = GdalUtils.gdal_wms_xml_description_file(
                inLayer.publicSource(),
                GdalUtils._get_wms_version(inLayer),
                bbox,
                width,
                height,
                wms_description_file_path,
            )
            if not res_xml_wms:
                raise QgsProcessingException(
                    "Cannot create XML description file for WMS layer. Details: {}".format(
                        xml_wms_error
                    )
                )
            input_details = GdalConnectionDetails(
                connection_string=wms_description_file_path
            )
        else:
            input_details = GdalUtils.gdal_connection_details_from_layer(inLayer)
            arguments.extend(
                [
                    "-projwin",
                    str(bbox.xMinimum()),
                    str(bbox.yMaximum()),
                    str(bbox.xMaximum()),
                    str(bbox.yMinimum()),
                ]
            )

        override_crs = self.parameterAsBoolean(parameters, self.OVERCRS, context)
        if self.NODATA in parameters and parameters[self.NODATA] is not None:
            nodata = self.parameterAsDouble(parameters, self.NODATA, context)
        else:
            nodata = None
        out = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        self.setOutputValue(self.OUTPUT, out)

        crs = inLayer.crs()
        if override_crs and crs.isValid():
            arguments.append(f"-a_srs {GdalUtils.gdal_crs_string(crs)}")

        if nodata is not None:
            arguments.append(f"-a_nodata {nodata}")

        data_type = self.parameterAsEnum(parameters, self.DATA_TYPE, context)
        if data_type:
            if self.TYPES[data_type] == "Int8" and GdalUtils.version() < 3070000:
                raise QgsProcessingException(
                    self.tr("Int8 data type requires GDAL version 3.7 or later")
                )

            arguments.append("-ot " + self.TYPES[data_type])

        output_format = QgsRasterFileWriter.driverForExtension(os.path.splitext(out)[1])
        if not output_format:
            raise QgsProcessingException(self.tr("Output format is invalid"))

        arguments.append("-of")
        arguments.append(output_format)

        options = self.parameterAsString(parameters, self.CREATION_OPTIONS, context)
        # handle backwards compatibility parameter OPTIONS
        if self.OPTIONS in parameters and parameters[self.OPTIONS] not in (None, ""):
            options = self.parameterAsString(parameters, self.OPTIONS, context)
        if options:
            arguments.extend(GdalUtils.parseCreationOptions(options))

        if self.EXTRA in parameters and parameters[self.EXTRA] not in (None, ""):
            extra = self.parameterAsString(parameters, self.EXTRA, context)
            arguments.append(extra)

        arguments.append(input_details.connection_string)
        arguments.append(out)

        if input_details.open_options:
            arguments.extend(input_details.open_options_as_arguments())

        if input_details.credential_options:
            arguments.extend(input_details.credential_options_as_arguments())

        return [self.commandName(), GdalUtils.escapeAndJoin(arguments)]
