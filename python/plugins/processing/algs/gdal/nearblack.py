# -*- coding: utf-8 -*-

"""
***************************************************************************
    nearblack.py
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
from builtins import str

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.core import (QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterRasterDestination)

from qgis.PyQt.QtGui import QIcon

from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class nearblack(GdalAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    NEAR = 'NEAR'
    WHITE = 'WHITE'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'gdaltools', 'nearblack.png'))

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterRasterLayer(self.INPUT, self.tr('Input layer'), False))
        self.addParameter(QgsProcessingParameterNumber(self.NEAR,
                                                       self.tr('How far from black (white)'),
                                                       type=QgsProcessingParameterNumber.Integer, minValue=0, maxValue=15, defaultValue=None))
        self.addParameter(QgsProcessingParameterBoolean(self.WHITE,
                                                        self.tr('Search for nearly white pixels instead of nearly black'),
                                                        defaultValue=False))
        self.addParameter(QgsProcessingParameterRasterDestination(self.OUTPUT, self.tr('Nearblack')))

    def name(self):
        return 'nearblack'

    def displayName(self):
        return self.tr('Near black')

    def group(self):
        return self.tr('Raster analysis')

    def getConsoleCommands(self, parameters, context, feedback):
        out = str(self.parameterAsOutputLayer(parameters, self.OUTPUT, context))

        arguments = []
        arguments.append('-o')
        arguments.append(out)
        arguments.append('-of')
        arguments.append(GdalUtils.getFormatShortNameFromFilename(out))
        arguments.append('-near')
        arguments.append(str(self.parameterAsInt(parameters, self.NEAR, context)))
        if self.parameterAsBool(parameters, self.WHITE, context):
            arguments.append('-white')
        arguments.append(self.parameterAsRasterLayer(parameters, self.INPUT, context).source())

        return ['nearblack', GdalUtils.escapeAndJoin(arguments)]
