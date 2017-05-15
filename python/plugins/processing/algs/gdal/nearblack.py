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

from qgis.PyQt.QtGui import QIcon

from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterBoolean
from processing.core.outputs import OutputRaster
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
        self.addParameter(ParameterRaster(self.INPUT,
                                          self.tr('Input layer'), False))
        self.addParameter(ParameterNumber(self.NEAR,
                                          self.tr('How far from black (white)'), 0, None, 15))
        self.addParameter(ParameterBoolean(self.WHITE,
                                           self.tr('Search for nearly white pixels instead of nearly black'),
                                           False))
        self.addOutput(OutputRaster(self.OUTPUT, self.tr('Nearblack')))

    def name(self):
        return 'nearblack'

    def displayName(self):
        return self.tr('Near black')

    def group(self):
        return self.tr('Raster analysis')

    def getConsoleCommands(self, parameters):
        arguments = []
        arguments.append('-o')
        output = str(self.getOutputValue(self.OUTPUT))
        arguments.append(output)
        arguments.append('-of')
        arguments.append(GdalUtils.getFormatShortNameFromFilename(output))
        arguments.append('-near')
        arguments.append(str(self.getParameterValue(self.NEAR)))
        if self.getParameterValue(self.WHITE):
            arguments.append('-white')
        arguments.append(self.getParameterValue(self.INPUT))
        return ['nearblack', GdalUtils.escapeAndJoin(arguments)]
