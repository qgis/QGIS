# -*- coding: utf-8 -*-

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

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtGui import QIcon

from qgis.core import (QgsRasterFileWriter,
                       QgsProcessing,
                       QgsProcessingParameterDefinition,
                       QgsProcessingParameterMultipleLayers,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterString,
                       QgsProcessingParameterBoolean,
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
    DATA_TYPE = 'DATA_TYPE'
    OUTPUT = 'OUTPUT'

    TYPES = ['Byte', 'Int16', 'UInt16', 'UInt32', 'Int32', 'Float32', 'Float64', 'CInt16', 'CInt32', 'CFloat32', 'CFloat64']

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterMultipleLayers(self.INPUT,
                                                               self.tr('Input layers'),
                                                               QgsProcessing.TypeRaster))
        self.addParameter(QgsProcessingParameterBoolean(self.PCT,
                                                        self.tr('Grab pseudocolor table from first layer'),
                                                        defaultValue=False))
        self.addParameter(QgsProcessingParameterBoolean(self.SEPARATE,
                                                        self.tr('Place each input file into a separate band'),
                                                        defaultValue=False))

        options_param = QgsProcessingParameterString(self.OPTIONS,
                                                     self.tr('Additional creation parameters'),
                                                     defaultValue='',
                                                     optional=True)
        options_param.setFlags(options_param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        options_param.setMetadata({
            'widget_wrapper': {
                'class': 'processing.algs.gdal.ui.RasterOptionsWidget.RasterOptionsWidgetWrapper'}})
        self.addParameter(options_param)

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

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'gdaltools', 'merge.png'))

    def getConsoleCommands(self, parameters, context, feedback):
        layers = self.parameterAsLayerList(parameters, self.INPUT, context)
        out = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)

        arguments = []
        if self.parameterAsBool(parameters, self.PCT, context):
            arguments.append('-pct')

        if self.parameterAsBool(parameters, self.SEPARATE, context):
            arguments.append('-separate')

        arguments.append('-ot')
        arguments.append(self.TYPES[self.parameterAsEnum(parameters, self.DATA_TYPE, context)])

        arguments.append('-of')
        arguments.append(QgsRasterFileWriter.driverForExtension(os.path.splitext(out)[1]))

        options = self.parameterAsString(parameters, self.OPTIONS, context)
        if options:
            arguments.append('-co')
            arguments.append(options)

        arguments.append('-o')
        arguments.append(out)

        for layer in layers:
            arguments.append(layer.source())

        commands = []
        if isWindows():
            commands = ['cmd.exe', '/C ', 'gdal_merge.bat',
                        GdalUtils.escapeAndJoin(arguments)]
        else:
            commands = ['gdal_merge.py', GdalUtils.escapeAndJoin(arguments)]

        return commands
