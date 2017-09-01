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

from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterString
from processing.core.outputs import OutputVector
from processing.tools.system import isWindows
from processing.algs.gdal.GdalUtils import GdalUtils

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class polygonize(GdalAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    FIELD = 'FIELD'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'gdaltools', 'polygonize.png'))

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(ParameterRaster(polygonize.INPUT,
                                          self.tr('Input layer'), False))
        self.addParameter(ParameterString(polygonize.FIELD,
                                          self.tr('Output field name'), 'DN'))
        self.addOutput(OutputVector(polygonize.OUTPUT, self.tr('Vectorized')))

    def name(self):
        return 'polygonize'

    def displayName(self):
        return self.tr('Polygonize (raster to vector)')

    def group(self):
        return self.tr('Raster conversion')

    def getConsoleCommands(self, parameters, context, feedback):
        output = self.getOutputValue(polygonize.OUTPUT)

        arguments = []
        arguments.append(self.getParameterValue(polygonize.INPUT))
        arguments.append('-f')
        arguments.append(GdalUtils.getVectorDriverFromFileName(output))
        arguments.append(output)
        arguments.append(QFileInfo(output).baseName())
        arguments.append(self.getParameterValue(polygonize.FIELD))

        commands = []
        if isWindows():
            commands = ['cmd.exe', '/C ', 'gdal_polygonize.bat',
                        GdalUtils.escapeAndJoin(arguments)]
        else:
            commands = ['gdal_polygonize.py',
                        GdalUtils.escapeAndJoin(arguments)]

        return commands
