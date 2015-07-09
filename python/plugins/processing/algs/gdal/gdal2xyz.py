# -*- coding: utf-8 -*-

"""
***************************************************************************
    gdal2xyz.py
    ---------------------
    Date                 : September 2013
    Copyright            : (C) 2013 by Alexander Bruy
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
__date__ = 'September 2013'
__copyright__ = '(C) 2013, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm

from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterNumber
from processing.core.outputs import OutputTable

from processing.tools.system import isWindows

from processing.algs.gdal.GdalUtils import GdalUtils


class gdal2xyz(GdalAlgorithm):

    INPUT = 'INPUT'
    BAND = 'BAND'
    OUTPUT = 'OUTPUT'

    def defineCharacteristics(self):
        self.name = 'gdal2xyz'
        self.group = '[GDAL] Conversion'
        self.addParameter(ParameterRaster(
            self.INPUT, self.tr('Input layer'), False))
        self.addParameter(ParameterNumber(self.BAND,
            self.tr('Band number'), 1, 9999, 1))

        self.addOutput(OutputTable(self.OUTPUT, self.tr('xyz')))

    def getConsoleCommands(self):
        arguments = []
        arguments.append('-band')
        arguments.append(str(self.getParameterValue(self.BAND)))

        arguments.append('-csv')
        arguments.append(self.getParameterValue(self.INPUT))
        arguments.append(self.getOutputValue(self.OUTPUT))

        commands = []
        if isWindows():
            commands = ['cmd.exe', '/C ', 'gdal2xyz.bat',
                        GdalUtils.escapeAndJoin(arguments)]
        else:
            commands = ['gdal2xyz.py', GdalUtils.escapeAndJoin(arguments)]

        return commands
