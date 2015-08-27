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

from PyQt4 import QtCore
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterString
from processing.core.outputs import OutputVector
from processing.tools.system import isWindows
from processing.algs.gdal.GdalUtils import GdalUtils


class polygonize(GdalAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    FIELD = 'FIELD'

    def commandLineName(self):
        return "gdalogr:polygonize"

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Polygonize (raster to vector)')
        self.group, self.i18n_group = self.trAlgorithm('[GDAL] Conversion')
        self.addParameter(ParameterRaster(polygonize.INPUT,
                                          self.tr('Input layer'), False))
        self.addParameter(ParameterString(polygonize.FIELD,
                                          self.tr('Output field name'), 'DN'))
        self.addOutput(OutputVector(polygonize.OUTPUT, self.tr('Vectorized')))

    def getConsoleCommands(self):
        arguments = []
        arguments.append(self.getParameterValue(polygonize.INPUT))
        arguments.append('-f')
        arguments.append('ESRI Shapefile')
        output = self.getOutputValue(polygonize.OUTPUT)
        arguments.append(output)
        arguments.append(QtCore.QFileInfo(output).baseName())
        arguments.append(self.getParameterValue(polygonize.FIELD))

        commands = []
        if isWindows():
            commands = ['cmd.exe', '/C ', 'gdal_polygonize.bat',
                        GdalUtils.escapeAndJoin(arguments)]
        else:
            commands = ['gdal_polygonize.py',
                        GdalUtils.escapeAndJoin(arguments)]

        return commands
