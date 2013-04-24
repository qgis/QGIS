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

import os
from PyQt4 import QtGui

from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.core.SextanteUtils import SextanteUtils

from sextante.parameters.ParameterRaster import ParameterRaster
from sextante.parameters.ParameterNumber import ParameterNumber
from sextante.outputs.OutputRaster import OutputRaster

from sextante.gdal.GdalUtils import GdalUtils

class rgb2pct(GeoAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    NCOLORS = "NCOLORS"

    def getIcon(self):
        filepath = os.path.dirname(__file__) + "/icons/24-to-8-bits.png"
        return QtGui.QIcon(filepath)

    def defineCharacteristics(self):
        self.name = "rgb2pct"
        self.group = "[GDAL] Conversion"
        self.addParameter(ParameterRaster(rgb2pct.INPUT, "Input layer", False))
        self.addParameter(ParameterNumber(rgb2pct.NCOLORS, "Number of colors", 1, None, 2))
        self.addOutput(OutputRaster(rgb2pct.OUTPUT, "Output layer"))

    def processAlgorithm(self, progress):
        arguments = []
        arguments.append("-n")
        arguments.append(str(self.getParameterValue(rgb2pct.NCOLORS)))
        arguments.append("-of")
        out = self.getOutputValue(rgb2pct.OUTPUT)
        arguments.append(GdalUtils.getFormatShortNameFromFilename(out))
        arguments.append(self.getParameterValue(rgb2pct.INPUT))
        arguments.append(out)

        if SextanteUtils.isWindows():
            commands = ["cmd.exe", "/C ", "rgb2pct.bat", GdalUtils.escapeAndJoin(arguments)]
        else:
            commands = ["rgb2pct.py", GdalUtils.escapeAndJoin(arguments)]

        GdalUtils.runGdal(commands, progress)
