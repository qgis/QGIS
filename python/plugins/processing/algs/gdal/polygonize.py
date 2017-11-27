# -*- coding: utf-8 -*-

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

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtGui import QIcon
from qgis.PyQt.QtCore import QFileInfo

from qgis.core import (QgsProcessing,
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
    OUTPUT = 'OUTPUT'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterRasterLayer(self.INPUT, self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterBand(self.BAND,
                                                     self.tr('Band number'),
                                                     parentLayerParameterName=self.INPUT))
        self.addParameter(QgsProcessingParameterString(self.FIELD,
                                                       self.tr('Name of the field to create'),
                                                       defaultValue='DN'))
        self.addParameter(QgsProcessingParameterBoolean(self.EIGHT_CONNECTEDNESS,
                                                        self.tr('Use 8-connectedness'),
                                                        defaultValue=False))

        self.addParameter(QgsProcessingParameterVectorDestination(self.OUTPUT,
                                                                  self.tr('Vectorized'),
                                                                  QgsProcessing.TypeVectorPolygon))

    def name(self):
        return 'polygonize'

    def displayName(self):
        return self.tr('Polygonize (raster to vector)')

    def group(self):
        return self.tr('Raster conversion')

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'gdaltools', 'polygonize.png'))

    def getConsoleCommands(self, parameters, context, feedback):
        arguments = []
        inLayer = self.parameterAsRasterLayer(parameters, self.INPUT, context)
        arguments.append(inLayer.source())

        outFile = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        output, outFormat = GdalUtils.ogrConnectionStringAndFormat(outFile, context)
        arguments.append(output)

        if self.parameterAsBool(parameters, self.EIGHT_CONNECTEDNESS, context):
            arguments.append('-8')

        arguments.append('-b')
        arguments.append(str(self.parameterAsInt(parameters, self.BAND, context)))

        if outFormat:
            arguments.append('-f {}'.format(outFormat))

        arguments.append(GdalUtils.ogrLayerName(output))
        arguments.append(self.parameterAsString(parameters, self.FIELD, context))

        commands = []
        if isWindows():
            commands = ['cmd.exe', '/C ', 'gdal_polygonize.bat',
                        GdalUtils.escapeAndJoin(arguments)]
        else:
            commands = ['gdal_polygonize.py',
                        GdalUtils.escapeAndJoin(arguments)]

        return commands
