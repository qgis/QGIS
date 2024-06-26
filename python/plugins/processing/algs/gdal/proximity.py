"""
***************************************************************************
    proximity.py
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

from qgis.PyQt.QtGui import QIcon

from qgis.core import (QgsRasterFileWriter,
                       QgsProcessingException,
                       QgsProcessingParameterDefinition,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterBand,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterString,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterRasterDestination)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

from processing.tools.system import isWindows

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class proximity(GdalAlgorithm):
    INPUT = 'INPUT'
    BAND = 'BAND'
    VALUES = 'VALUES'
    MAX_DISTANCE = 'MAX_DISTANCE'
    REPLACE = 'REPLACE'
    UNITS = 'UNITS'
    NODATA = 'NODATA'
    OPTIONS = 'OPTIONS'
    EXTRA = 'EXTRA'
    DATA_TYPE = 'DATA_TYPE'
    OUTPUT = 'OUTPUT'

    TYPES = ['Byte', 'Int16', 'UInt16', 'UInt32', 'Int32', 'Float32', 'Float64', 'CInt16', 'CInt32', 'CFloat32', 'CFloat64', 'Int8']

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'gdaltools', 'proximity.png'))

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.distanceUnits = ((self.tr('Georeferenced coordinates'), 'GEO'),
                              (self.tr('Pixel coordinates'), 'PIXEL'))

        self.addParameter(QgsProcessingParameterRasterLayer(self.INPUT,
                                                            self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterBand(self.BAND,
                                                     self.tr('Band number'),
                                                     1,
                                                     parentLayerParameterName=self.INPUT))
        self.addParameter(QgsProcessingParameterString(self.VALUES,
                                                       self.tr('A list of pixel values in the source image to be considered target pixels'),
                                                       optional=True))
        self.addParameter(QgsProcessingParameterEnum(self.UNITS,
                                                     self.tr('Distance units'),
                                                     options=[i[0] for i in self.distanceUnits],
                                                     allowMultiple=False,
                                                     defaultValue=1))
        self.addParameter(QgsProcessingParameterNumber(self.MAX_DISTANCE,
                                                       self.tr('The maximum distance to be generated'),
                                                       type=QgsProcessingParameterNumber.Type.Double,
                                                       minValue=0.0,
                                                       defaultValue=0.0,
                                                       optional=True))
        self.addParameter(QgsProcessingParameterNumber(self.REPLACE,
                                                       self.tr('Value to be applied to all pixels that are within the -maxdist of target pixels'),
                                                       type=QgsProcessingParameterNumber.Type.Double,
                                                       defaultValue=0.0,
                                                       optional=True))
        self.addParameter(QgsProcessingParameterNumber(self.NODATA,
                                                       self.tr('Nodata value to use for the destination proximity raster'),
                                                       type=QgsProcessingParameterNumber.Type.Double,
                                                       defaultValue=0.0,
                                                       optional=True))

        options_param = QgsProcessingParameterString(self.OPTIONS,
                                                     self.tr('Additional creation options'),
                                                     defaultValue='',
                                                     optional=True)
        options_param.setFlags(options_param.flags() | QgsProcessingParameterDefinition.Flag.FlagAdvanced)
        options_param.setMetadata({'widget_wrapper': {'widget_type': 'rasteroptions'}})
        self.addParameter(options_param)

        extra_param = QgsProcessingParameterString(self.EXTRA,
                                                   self.tr('Additional command-line parameters'),
                                                   defaultValue=None,
                                                   optional=True)
        extra_param.setFlags(extra_param.flags() | QgsProcessingParameterDefinition.Flag.FlagAdvanced)
        self.addParameter(extra_param)

        dataType_param = QgsProcessingParameterEnum(self.DATA_TYPE,
                                                    self.tr('Output data type'),
                                                    self.TYPES,
                                                    allowMultiple=False,
                                                    defaultValue=5)
        dataType_param.setFlags(dataType_param.flags() | QgsProcessingParameterDefinition.Flag.FlagAdvanced)
        self.addParameter(dataType_param)

        self.addParameter(QgsProcessingParameterRasterDestination(self.OUTPUT,
                                                                  self.tr('Proximity map')))

    def name(self):
        return 'proximity'

    def displayName(self):
        return self.tr('Proximity (raster distance)')

    def group(self):
        return self.tr('Raster analysis')

    def groupId(self):
        return 'rasteranalysis'

    def commandName(self):
        return 'gdal_proximity'

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        inLayer = self.parameterAsRasterLayer(parameters, self.INPUT, context)
        if inLayer is None:
            raise QgsProcessingException(self.invalidRasterError(parameters, self.INPUT))
        input_details = GdalUtils.gdal_connection_details_from_layer(
            inLayer)

        distance = self.parameterAsDouble(parameters, self.MAX_DISTANCE, context)
        replaceValue = self.parameterAsDouble(parameters, self.REPLACE, context)
        if self.NODATA in parameters and parameters[self.NODATA] is not None:
            nodata = self.parameterAsDouble(parameters, self.NODATA, context)
        else:
            nodata = None
        options = self.parameterAsString(parameters, self.OPTIONS, context)
        out = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        self.setOutputValue(self.OUTPUT, out)

        arguments = [
            '-srcband',
            str(self.parameterAsInt(parameters, self.BAND, context)),
            '-distunits',

            self.distanceUnits[self.parameterAsEnum(parameters, self.UNITS, context)][1]
        ]

        values = self.parameterAsString(parameters, self.VALUES, context)
        if values:
            arguments.append('-values')
            arguments.append(values)

        if distance:
            arguments.append('-maxdist')
            arguments.append(str(distance))

        if nodata is not None:
            arguments.append('-nodata')
            arguments.append(str(nodata))

        if replaceValue:
            arguments.append('-fixed-buf-val')
            arguments.append(str(replaceValue))

        data_type = self.parameterAsEnum(parameters, self.DATA_TYPE, context)
        if self.TYPES[data_type] == 'Int8' and GdalUtils.version() < 3070000:
            raise QgsProcessingException(self.tr('Int8 data type requires GDAL version 3.7 or later'))

        arguments.append('-ot ' + self.TYPES[data_type])

        output_format = QgsRasterFileWriter.driverForExtension(os.path.splitext(out)[1])
        if not output_format:
            raise QgsProcessingException(self.tr('Output format is invalid'))

        arguments.append('-of')
        arguments.append(output_format)

        if options:
            arguments.extend(GdalUtils.parseCreationOptions(options))

        if self.EXTRA in parameters and parameters[self.EXTRA] not in (None, ''):
            extra = self.parameterAsString(parameters, self.EXTRA, context)
            arguments.append(extra)

        arguments.append(input_details.connection_string)
        arguments.append(out)

        if input_details.credential_options:
            arguments.extend(input_details.credential_options_as_arguments())

        return [self.commandName() + ('.bat' if isWindows() else '.py'), GdalUtils.escapeAndJoin(arguments)]
