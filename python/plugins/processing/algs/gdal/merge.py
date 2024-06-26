"""
***************************************************************************
    merge.py
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
                       QgsProcessing,
                       QgsProcessingException,
                       QgsProcessingParameterDefinition,
                       QgsProcessingParameterMultipleLayers,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterString,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterRasterDestination)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

from processing.tools.system import isWindows

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class merge(GdalAlgorithm):
    INPUT = 'INPUT'
    PCT = 'PCT'
    SEPARATE = 'SEPARATE'
    OPTIONS = 'OPTIONS'
    EXTRA = 'EXTRA'
    DATA_TYPE = 'DATA_TYPE'
    NODATA_INPUT = 'NODATA_INPUT'
    NODATA_OUTPUT = 'NODATA_OUTPUT'
    OUTPUT = 'OUTPUT'

    TYPES = ['Byte', 'Int16', 'UInt16', 'UInt32', 'Int32', 'Float32', 'Float64', 'CInt16', 'CInt32', 'CFloat32', 'CFloat64', 'Int8']

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterMultipleLayers(self.INPUT,
                                                               self.tr('Input layers'),
                                                               QgsProcessing.SourceType.TypeRaster))
        self.addParameter(QgsProcessingParameterBoolean(self.PCT,
                                                        self.tr('Grab pseudocolor table from first layer'),
                                                        defaultValue=False))
        self.addParameter(QgsProcessingParameterBoolean(self.SEPARATE,
                                                        self.tr('Place each input file into a separate band'),
                                                        defaultValue=False))

        nodata_param = QgsProcessingParameterNumber(self.NODATA_INPUT,
                                                    self.tr('Input pixel value to treat as NoData'),
                                                    type=QgsProcessingParameterNumber.Type.Double,
                                                    defaultValue=None,
                                                    optional=True)
        nodata_param.setFlags(nodata_param.flags() | QgsProcessingParameterDefinition.Flag.FlagAdvanced)
        self.addParameter(nodata_param)

        nodata_out_param = QgsProcessingParameterNumber(self.NODATA_OUTPUT,
                                                        self.tr('Assign specified NoData value to output'),
                                                        type=QgsProcessingParameterNumber.Type.Double,
                                                        defaultValue=None,
                                                        optional=True)
        nodata_out_param.setFlags(nodata_out_param.flags() | QgsProcessingParameterDefinition.Flag.FlagAdvanced)
        self.addParameter(nodata_out_param)

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

        self.addParameter(QgsProcessingParameterEnum(self.DATA_TYPE,
                                                     self.tr('Output data type'),
                                                     self.TYPES,
                                                     allowMultiple=False,
                                                     defaultValue=5))

        self.addParameter(QgsProcessingParameterRasterDestination(self.OUTPUT,
                                                                  self.tr('Merged')))

    def name(self):
        return 'merge'

    def displayName(self):
        return self.tr('Merge')

    def group(self):
        return self.tr('Raster miscellaneous')

    def groupId(self):
        return 'rastermiscellaneous'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'gdaltools', 'merge.png'))

    def commandName(self):
        return 'gdal_merge'

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        out = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        self.setOutputValue(self.OUTPUT, out)

        arguments = []
        if self.parameterAsBoolean(parameters, self.PCT, context):
            arguments.append('-pct')

        if self.parameterAsBoolean(parameters, self.SEPARATE, context):
            arguments.append('-separate')

        if self.NODATA_INPUT in parameters and parameters[self.NODATA_INPUT] is not None:
            nodata_input = self.parameterAsDouble(parameters, self.NODATA_INPUT, context)
            arguments.append('-n')
            arguments.append(str(nodata_input))

        if self.NODATA_OUTPUT in parameters and parameters[self.NODATA_OUTPUT] is not None:
            nodata_output = self.parameterAsDouble(parameters, self.NODATA_OUTPUT, context)
            arguments.append('-a_nodata')
            arguments.append(str(nodata_output))

        data_type = self.parameterAsEnum(parameters, self.DATA_TYPE, context)
        if self.TYPES[data_type] == 'Int8' and GdalUtils.version() < 3070000:
            raise QgsProcessingException(self.tr('Int8 data type requires GDAL version 3.7 or later'))

        arguments.append('-ot ' + self.TYPES[data_type])

        output_format = QgsRasterFileWriter.driverForExtension(os.path.splitext(out)[1])
        if not output_format:
            raise QgsProcessingException(self.tr('Output format is invalid'))

        arguments.append('-of')
        arguments.append(output_format)

        options = self.parameterAsString(parameters, self.OPTIONS, context)
        if options:
            arguments.extend(GdalUtils.parseCreationOptions(options))

        if self.EXTRA in parameters and parameters[self.EXTRA] not in (None, ''):
            extra = self.parameterAsString(parameters, self.EXTRA, context)
            arguments.append(extra)

        arguments.append('-o')
        arguments.append(out)

        # Always write input files to a text file in case there are many of them and the
        # length of the command will be longer then allowed in command prompt
        list_file = GdalUtils.writeLayerParameterToTextFile(filename='mergeInputFiles.txt', alg=self, parameters=parameters, parameter_name=self.INPUT, context=context, quote=True, executing=executing)
        arguments.append('--optfile')
        arguments.append(list_file)

        return [self.commandName() + ('.bat' if isWindows() else '.py'), GdalUtils.escapeAndJoin(arguments)]
