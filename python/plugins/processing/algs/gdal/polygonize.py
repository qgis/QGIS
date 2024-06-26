"""
***************************************************************************
    polygonize.py
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

from qgis.core import (QgsProcessing,
                       QgsProcessingException,
                       QgsProcessingParameterDefinition,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterBand,
                       QgsProcessingParameterString,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterVectorDestination)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.tools.system import isWindows
from processing.algs.gdal.GdalUtils import GdalUtils

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class polygonize(GdalAlgorithm):
    INPUT = 'INPUT'
    BAND = 'BAND'
    FIELD = 'FIELD'
    EIGHT_CONNECTEDNESS = 'EIGHT_CONNECTEDNESS'
    EXTRA = 'EXTRA'
    OUTPUT = 'OUTPUT'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterRasterLayer(self.INPUT, self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterBand(self.BAND,
                                                     self.tr('Band number'),
                                                     1,
                                                     parentLayerParameterName=self.INPUT))
        self.addParameter(QgsProcessingParameterString(self.FIELD,
                                                       self.tr('Name of the field to create'),
                                                       defaultValue='DN'))
        self.addParameter(QgsProcessingParameterBoolean(self.EIGHT_CONNECTEDNESS,
                                                        self.tr('Use 8-connectedness'),
                                                        defaultValue=False))

        extra_param = QgsProcessingParameterString(self.EXTRA,
                                                   self.tr('Additional command-line parameters'),
                                                   defaultValue=None,
                                                   optional=True)
        extra_param.setFlags(extra_param.flags() | QgsProcessingParameterDefinition.Flag.FlagAdvanced)
        self.addParameter(extra_param)

        self.addParameter(QgsProcessingParameterVectorDestination(self.OUTPUT,
                                                                  self.tr('Vectorized'),
                                                                  QgsProcessing.SourceType.TypeVectorPolygon))

    def name(self):
        return 'polygonize'

    def displayName(self):
        return self.tr('Polygonize (raster to vector)')

    def group(self):
        return self.tr('Raster conversion')

    def groupId(self):
        return 'rasterconversion'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'gdaltools', 'polygonize.png'))

    def commandName(self):
        return 'gdal_polygonize'

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        arguments = []

        if self.parameterAsBoolean(parameters, self.EIGHT_CONNECTEDNESS, context):
            arguments.append('-8')

        if self.EXTRA in parameters and parameters[self.EXTRA] not in (None, ''):
            extra = self.parameterAsString(parameters, self.EXTRA, context)
            arguments.append(extra)

        inLayer = self.parameterAsRasterLayer(parameters, self.INPUT, context)
        if inLayer is None:
            raise QgsProcessingException(self.invalidRasterError(parameters, self.INPUT))
        input_details = GdalUtils.gdal_connection_details_from_layer(
            inLayer)

        arguments.append(input_details.connection_string)

        arguments.append('-b')
        arguments.append(str(self.parameterAsInt(parameters, self.BAND, context)))

        outFile = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        self.setOutputValue(self.OUTPUT, outFile)
        output_details = GdalUtils.gdal_connection_details_from_uri(outFile, context)

        if output_details.format:
            arguments.append(f'-f {output_details.format}')

        arguments.append(output_details.connection_string)

        layerName = GdalUtils.ogrOutputLayerName(output_details.connection_string)
        if layerName:
            arguments.append(layerName)
        arguments.append(self.parameterAsString(parameters, self.FIELD, context))

        if input_details.credential_options:
            arguments.extend(input_details.credential_options_as_arguments())

        return [self.commandName() + ('.bat' if isWindows() else '.py'), GdalUtils.escapeAndJoin(arguments)]
