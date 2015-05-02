
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

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterNumber
from processing.core.outputs import OutputRaster
from processing.tools.system import isWindows
from processing.algs.gdal.GdalUtils import GdalUtils


class rgb2pct(GdalAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    NCOLORS = 'NCOLORS'


    def defineCharacteristics(self):
        self.name = 'RGB to PCT'
        self.group = '[GDAL] Conversion'
        self.addParameter(ParameterRaster(rgb2pct.INPUT,
            self.tr('Input layer'), False))
        self.addParameter(ParameterNumber(rgb2pct.NCOLORS,
            self.tr('Number of colors'), 1, None, 2))
        self.addOutput(OutputRaster(rgb2pct.OUTPUT, self.tr('RGB to PCT')))

    def processAlgorithm(self, progress):
        arguments = []
        arguments.append('-n')
        arguments.append(str(self.getParameterValue(rgb2pct.NCOLORS)))
        arguments.append('-of')
        out = self.getOutputValue(rgb2pct.OUTPUT)
        arguments.append(GdalUtils.getFormatShortNameFromFilename(out))
        arguments.append(self.getParameterValue(rgb2pct.INPUT))
        arguments.append(out)

        if isWindows():
            commands = ['cmd.exe', '/C ', 'rgb2pct.bat',
                        GdalUtils.escapeAndJoin(arguments)]
        else:
            commands = ['rgb2pct.py', GdalUtils.escapeAndJoin(arguments)]

        GdalUtils.runGdal(commands, progress)
