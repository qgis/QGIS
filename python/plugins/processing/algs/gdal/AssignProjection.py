# -*- coding: utf-8 -*-

"""
***************************************************************************
    self.py
    ---------------------
    Date                 : January 2016
    Copyright            : (C) 2016 by Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Alexander Bruy'
__date__ = 'January 2016'
__copyright__ = '(C) 2016, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtGui import QIcon

from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterCrs
from processing.core.outputs import OutputRaster
from processing.algs.gdal.GdalUtils import GdalUtils

from processing.tools.system import isWindows

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class AssignProjection(GdalAlgorithm):

    INPUT = 'INPUT'
    CRS = 'CRS'
    OUTPUT = 'OUTPUT'

    def __init__(self):
        super().__init__()
        self.addParameter(ParameterRaster(self.INPUT, self.tr('Input layer'), False))
        self.addParameter(ParameterCrs(self.CRS,
                                       self.tr('Desired CRS'), ''))

        self.addOutput(OutputRaster(self.OUTPUT, self.tr('Layer with projection'), True))

    def name(self):
        return 'assignprojection'

    def displayName(self):
        return self.tr('Assign projection')

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'gdaltools', 'projection-add.png'))

    def group(self):
        return self.tr('Raster projections')

    def getConsoleCommands(self, parameters):
        fileName = self.getParameterValue(self.INPUT)
        crs = self.getParameterValue(self.CRS)
        output = self.getOutputValue(self.OUTPUT)  # NOQA

        arguments = []
        arguments.append('-a_srs')
        arguments.append(crs)

        arguments.append(fileName)

        commands = []
        if isWindows():
            commands = ['cmd.exe', '/C ', 'gdal_edit.bat',
                        GdalUtils.escapeAndJoin(arguments)]
        else:
            commands = ['gdal_edit.py', GdalUtils.escapeAndJoin(arguments)]

        self.setOutputValue(self.OUTPUT, fileName)
        return commands
