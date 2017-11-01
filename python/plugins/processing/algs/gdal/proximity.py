# -*- coding: utf-8 -*-

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

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtGui import QIcon

from qgis.core import (QgsRasterFileWriter,
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
    DATA_TYPE = 'DATA_TYPE'
    OUTPUT = 'OUTPUT'

    TYPES = ['Byte', 'Int16', 'UInt16', 'UInt32', 'Int32', 'Float32', 'Float64', 'CInt16', 'CInt32', 'CFloat32', 'CFloat64']

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
                                                       type=QgsProcessingParameterNumber.Double,
                                                       minValue=0.0,
                                                       defaultValue=0.0,
                                                       optional=True))
        self.addParameter(QgsProcessingParameterNumber(self.REPLACE,
                                                       self.tr('Value to be applied to all pixels that are within the -maxdist of target pixels'),
                                                       type=QgsProcessingParameterNumber.Double,
                                                       defaultValue=0.0,
                                                       optional=True))
        self.addParameter(QgsProcessingParameterNumber(self.NODATA,
                                                       self.tr('Nodata value to use for the destination proximity raster'),
                                                       type=QgsProcessingParameterNumber.Double,
                                                       defaultValue=0.0,
                                                       optional=True))

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
                                                                  self.tr('Proximity map')))

    def name(self):
        return 'proximity'

    def displayName(self):
        return self.tr('Proximity (raster distance)')

    def group(self):
        return self.tr('Raster analysis')

    def getConsoleCommands(self, parameters, context, feedback):
        inLayer = self.parameterAsRasterLayer(parameters, self.INPUT, context)
        distance = self.parameterAsDouble(parameters, self.MAX_DISTANCE, context)
        replaceValue = self.parameterAsDouble(parameters, self.REPLACE, context)
        nodata = self.parameterAsDouble(parameters, self.NODATA, context)
        options = self.parameterAsString(parameters, self.OPTIONS, context)
        out = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)

        arguments = []
        arguments.append('-srcband')
        arguments.append(str(self.parameterAsInt(parameters, self.BAND, context)))

        arguments.append('-distunits')
        arguments.append(self.distanceUnits[self.parameterAsEnum(parameters, self.UNITS, context)][1])

        values = self.parameterAsString(parameters, self.VALUES, context)
        if values:
            arguments.append('-values')
            arguments.append(values)

        if distance:
            arguments.append('-maxdist')
            arguments.append(str(distance))

        if nodata:
            arguments.append('-nodata')
            arguments.append(str(nodata))

        if replaceValue:
            arguments.append('-fixed-buf-val')
            arguments.append(str(replaceValue))

        arguments.append('-ot')
        arguments.append(self.TYPES[self.parameterAsEnum(parameters, self.DATA_TYPE, context)])

        arguments.append('-of')
        arguments.append(QgsRasterFileWriter.driverForExtension(os.path.splitext(out)[1]))

        if options:
            arguments.append('-co')
            arguments.append(options)

        arguments.append(inLayer.source())
        arguments.append(out)

        commands = []
        if isWindows():
            commands = ['cmd.exe', '/C ', 'gdal_proximity.bat',
                        GdalUtils.escapeAndJoin(arguments)]
        else:
            commands = ['gdal_proximity.py',
                        GdalUtils.escapeAndJoin(arguments)]

        return commands
