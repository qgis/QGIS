"""
***************************************************************************
    rasterize.py
    ---------------------
    Date                 : September 2013
    Copyright            : (C) 2013 by Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Alexander Bruy'
__date__ = 'September 2013'
__copyright__ = '(C) 2013, Alexander Bruy'

import os

from qgis.PyQt.QtGui import QIcon

from qgis.core import (QgsRasterFileWriter,
                       QgsProcessingException,
                       QgsProcessingParameterDefinition,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterField,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterString,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterExtent,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterRasterDestination, NULL)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class rasterize(GdalAlgorithm):
    INPUT = 'INPUT'
    FIELD = 'FIELD'
    BURN = 'BURN'
    USE_Z = 'USE_Z'
    WIDTH = 'WIDTH'
    HEIGHT = 'HEIGHT'
    UNITS = 'UNITS'
    NODATA = 'NODATA'
    EXTENT = 'EXTENT'
    INIT = 'INIT'
    INVERT = 'INVERT'
    ALL_TOUCH = 'ALL_TOUCH'
    OPTIONS = 'OPTIONS'
    DATA_TYPE = 'DATA_TYPE'
    EXTRA = 'EXTRA'
    OUTPUT = 'OUTPUT'

    TYPES = ['Byte', 'Int16', 'UInt16', 'UInt32', 'Int32', 'Float32', 'Float64', 'CInt16', 'CInt32', 'CFloat32', 'CFloat64', 'Int8']

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.units = [self.tr("Pixels"),
                      self.tr("Georeferenced units")]

        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterField(self.FIELD,
                                                      self.tr('Field to use for a burn-in value'),
                                                      None,
                                                      self.INPUT,
                                                      QgsProcessingParameterField.DataType.Numeric,
                                                      optional=True))
        self.addParameter(QgsProcessingParameterNumber(self.BURN,
                                                       self.tr('A fixed value to burn'),
                                                       type=QgsProcessingParameterNumber.Type.Double,
                                                       defaultValue=0.0,
                                                       optional=True))
        self.addParameter(QgsProcessingParameterBoolean(self.USE_Z,
                                                        self.tr('Burn value extracted from the "Z" values of the feature'),
                                                        defaultValue=False,
                                                        optional=True))
        self.addParameter(QgsProcessingParameterEnum(self.UNITS,
                                                     self.tr('Output raster size units'),
                                                     self.units))
        self.addParameter(QgsProcessingParameterNumber(self.WIDTH,
                                                       self.tr('Width/Horizontal resolution'),
                                                       type=QgsProcessingParameterNumber.Type.Double,
                                                       minValue=0.0,
                                                       defaultValue=0.0))
        self.addParameter(QgsProcessingParameterNumber(self.HEIGHT,
                                                       self.tr('Height/Vertical resolution'),
                                                       type=QgsProcessingParameterNumber.Type.Double,
                                                       minValue=0.0,
                                                       defaultValue=0.0))
        self.addParameter(QgsProcessingParameterExtent(self.EXTENT,
                                                       self.tr('Output extent'),
                                                       optional=True))
        nodataParam = QgsProcessingParameterNumber(self.NODATA,
                                                   self.tr('Assign a specified NoData value to output bands'),
                                                   type=QgsProcessingParameterNumber.Type.Double,
                                                   optional=True)
        nodataParam.setGuiDefaultValueOverride(NULL)
        self.addParameter(nodataParam)

        options_param = QgsProcessingParameterString(self.OPTIONS,
                                                     self.tr('Additional creation options'),
                                                     defaultValue='',
                                                     optional=True)
        options_param.setFlags(options_param.flags() | QgsProcessingParameterDefinition.Flag.FlagAdvanced)
        options_param.setMetadata({'widget_wrapper': {'widget_type': 'rasteroptions'}})
        self.addParameter(options_param)

        dataType_param = QgsProcessingParameterEnum(self.DATA_TYPE,
                                                    self.tr('Output data type'),
                                                    self.TYPES,
                                                    allowMultiple=False,
                                                    defaultValue=5)
        dataType_param.setFlags(dataType_param.flags() | QgsProcessingParameterDefinition.Flag.FlagAdvanced)
        self.addParameter(dataType_param)

        init_param = QgsProcessingParameterNumber(self.INIT,
                                                  self.tr('Pre-initialize the output image with value'),
                                                  type=QgsProcessingParameterNumber.Type.Double,
                                                  optional=True)
        init_param.setFlags(init_param.flags() | QgsProcessingParameterDefinition.Flag.FlagAdvanced)
        self.addParameter(init_param)

        invert_param = QgsProcessingParameterBoolean(self.INVERT,
                                                     self.tr('Invert rasterization'),
                                                     defaultValue=False)
        invert_param.setFlags(invert_param.flags() | QgsProcessingParameterDefinition.Flag.FlagAdvanced)
        self.addParameter(invert_param)

        extra_param = QgsProcessingParameterString(self.EXTRA,
                                                   self.tr('Additional command-line parameters'),
                                                   defaultValue=None,
                                                   optional=True)
        extra_param.setFlags(extra_param.flags() | QgsProcessingParameterDefinition.Flag.FlagAdvanced)
        self.addParameter(extra_param)

        self.addParameter(QgsProcessingParameterRasterDestination(self.OUTPUT,
                                                                  self.tr('Rasterized')))

    def name(self):
        return 'rasterize'

    def displayName(self):
        return self.tr('Rasterize (vector to raster)')

    def group(self):
        return self.tr('Vector conversion')

    def groupId(self):
        return 'vectorconversion'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'gdaltools', 'rasterize.png'))

    def commandName(self):
        return 'gdal_rasterize'

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        if source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.INPUT))

        input_details = self.getOgrCompatibleSource(self.INPUT, parameters, context, feedback, executing)
        arguments = [
            '-l',
            input_details.layer_name
        ]
        fieldName = self.parameterAsString(parameters, self.FIELD, context)
        use_z = self.parameterAsBoolean(parameters, self.USE_Z, context)
        if use_z:
            arguments.append('-3d')
        elif fieldName:
            arguments.append('-a')
            arguments.append(fieldName)
        else:
            arguments.append('-burn')
            arguments.append(self.parameterAsDouble(parameters, self.BURN, context))

        units = self.parameterAsEnum(parameters, self.UNITS, context)
        if units == 0:
            arguments.append('-ts')
        else:
            arguments.append('-tr')
        arguments.append(self.parameterAsDouble(parameters, self.WIDTH, context))
        arguments.append(self.parameterAsDouble(parameters, self.HEIGHT, context))

        if self.INIT in parameters and parameters[self.INIT] is not None:
            initValue = self.parameterAsDouble(parameters, self.INIT, context)
            arguments.append('-init')
            arguments.append(initValue)

        if self.parameterAsBoolean(parameters, self.INVERT, context):
            arguments.append('-i')

        if self.parameterAsBoolean(parameters, self.ALL_TOUCH, context):
            arguments.append('-at')

        if self.NODATA in parameters and parameters[self.NODATA] is not None:
            nodata = self.parameterAsDouble(parameters, self.NODATA, context)
            arguments.append('-a_nodata')
            arguments.append(nodata)

        extent = self.parameterAsExtent(parameters, self.EXTENT, context, source.sourceCrs())
        if not extent.isNull():
            arguments.append('-te')
            arguments.append(extent.xMinimum())
            arguments.append(extent.yMinimum())
            arguments.append(extent.xMaximum())
            arguments.append(extent.yMaximum())

        data_type = self.parameterAsEnum(parameters, self.DATA_TYPE, context)
        if self.TYPES[data_type] == 'Int8' and GdalUtils.version() < 3070000:
            raise QgsProcessingException(self.tr('Int8 data type requires GDAL version 3.7 or later'))

        arguments.append('-ot ' + self.TYPES[data_type])

        out = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        self.setOutputValue(self.OUTPUT, out)

        output_format = QgsRasterFileWriter.driverForExtension(os.path.splitext(out)[1])
        if not output_format:
            raise QgsProcessingException(self.tr('Output format is invalid'))

        arguments.append('-of')
        arguments.append(output_format)

        if input_details.open_options:
            if GdalUtils.version() < 3070000:
                raise QgsProcessingException(self.tr(
                    'Open options are not supported by gdal_rasterize version {} (requires GDAL version 3.7 or later)'.format(
                        GdalUtils.readableVersion())))

            arguments.extend(input_details.open_options_as_arguments())

        if input_details.credential_options:
            arguments.extend(input_details.credential_options_as_arguments())

        options = self.parameterAsString(parameters, self.OPTIONS, context)
        if options:
            arguments.extend(GdalUtils.parseCreationOptions(options))

        if self.EXTRA in parameters and parameters[self.EXTRA] not in (None, ''):
            extra = self.parameterAsString(parameters, self.EXTRA, context)
            arguments.append(extra)

        arguments.append(input_details.connection_string)
        arguments.append(out)

        return [self.commandName(), GdalUtils.escapeAndJoin(arguments)]
