# -*- coding: utf-8 -*-

"""
***************************************************************************
    sieve.py
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

from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm

from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterNumber
from processing.core.outputs import OutputRaster

from processing.tools.system import isWindows

from processing.algs.gdal.GdalUtils import GdalUtils

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class sieve(GdalAlgorithm):

    INPUT = 'INPUT'
    THRESHOLD = 'THRESHOLD'
    CONNECTIONS = 'CONNECTIONS'
    OUTPUT = 'OUTPUT'

    PIXEL_CONNECTIONS = ['4', '8']

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'gdaltools', 'sieve.png'))

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(ParameterRaster(self.INPUT, self.tr('Input layer'), False))
        self.addParameter(ParameterNumber(self.THRESHOLD,
                                          self.tr('Threshold'), 0, 9999, 2))
        self.addParameter(ParameterSelection(self.CONNECTIONS,
                                             self.tr('Pixel connection'), self.PIXEL_CONNECTIONS, 0))

        self.addOutput(OutputRaster(self.OUTPUT, self.tr('Sieved')))

    def name(self):
        return 'sieve'

    def displayName(self):
        return self.tr('Sieve')

    def group(self):
        return self.tr('Raster analysis')

    def getConsoleCommands(self, parameters, context, feedback):
        output = self.getOutputValue(self.OUTPUT)

        arguments = []
        arguments.append('-st')
        arguments.append(str(self.getParameterValue(self.THRESHOLD)))

        arguments.append('-' +
                         self.PIXEL_CONNECTIONS[self.getParameterValue(
                                                self.CONNECTIONS)])

        arguments.append('-of')
        arguments.append(GdalUtils.getFormatShortNameFromFilename(output))

        arguments.append(self.getParameterValue(self.INPUT))
        arguments.append(output)

        commands = []
        if isWindows():
            commands = ['cmd.exe', '/C ', 'gdal_sieve.bat',
                        GdalUtils.escapeAndJoin(arguments)]
        else:
            commands = ['gdal_sieve.py', GdalUtils.escapeAndJoin(arguments)]

        return commands
