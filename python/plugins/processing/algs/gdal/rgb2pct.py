
# -*- coding: utf-8 -*-

"""
***************************************************************************
    rgb2pct.py
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

from qgis.PyQt.QtGui import QIcon

from qgis.core import (QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterRasterDestination)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.tools.system import isWindows
from processing.algs.gdal.GdalUtils import GdalUtils

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class rgb2pct(GdalAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    NCOLORS = 'NCOLORS'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'gdaltools', '24-to-8-bits.png'))

    def group(self):
        return self.tr('Raster conversion')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterRasterLayer(rgb2pct.INPUT,
                                                            self.tr('Input layer'), optional=False))
        self.addParameter(QgsProcessingParameterNumber(rgb2pct.NCOLORS,
                                                       self.tr('Number of colors'), minValue=1, defaultValue=2))
        self.addParameter(QgsProcessingParameterRasterDestination(self.OUTPUT, self.tr('RGB to PCT')))

    def name(self):
        return 'rgbtopct'

    def displayName(self):
        return self.tr('RGB to PCT')

    def getConsoleCommands(self, parameters, context, feedback):
        arguments = []
        arguments.append('-n')
        arguments.append(str(self.parameterAsInt(parameters, rgb2pct.NCOLORS, context)))
        arguments.append('-of')
        out = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        arguments.append(GdalUtils.getFormatShortNameFromFilename(out))
        arguments.append(self.parameterAsRasterLayer(parameters, self.INPUT, context).source())
        arguments.append(out)

        if isWindows():
            commands = ['cmd.exe', '/C ', 'rgb2pct.bat',
                        GdalUtils.escapeAndJoin(arguments)]
        else:
            commands = ['rgb2pct.py', GdalUtils.escapeAndJoin(arguments)]

        return commands
