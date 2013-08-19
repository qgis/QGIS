# -*- coding: utf-8 -*-

"""
***************************************************************************
    pct2rgb.py
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

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.ProcessingUtils import ProcessingUtils

from processing.parameters.ParameterRaster import ParameterRaster
from processing.parameters.ParameterSelection import ParameterSelection
from processing.outputs.OutputRaster import OutputRaster

from processing.gdal.GdalUtils import GdalUtils

class pct2rgb(GeoAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    NBAND = "NBAND"

    def getIcon(self):
        filepath = os.path.dirname(__file__) + "/icons/8-to-24-bits.png"
        return QtGui.QIcon(filepath)

    def defineCharacteristics(self):
        self.name = "pct2rgb"
        self.group = "[GDAL] Conversion"
        self.addParameter(ParameterRaster(pct2rgb.INPUT, "Input layer", False))
        options = []
        for i in range(25):
            options.append(str(i + 1))
        self.addParameter(ParameterSelection(pct2rgb.NBAND, "Band to convert", options))
        self.addOutput(OutputRaster(pct2rgb.OUTPUT, "Output layer"))

    def processAlgorithm(self, progress):
        arguments = []
        arguments.append("-b")
        arguments.append(str(self.getParameterValue(pct2rgb.NBAND) + 1))
        arguments.append("-of")
        out = self.getOutputValue(pct2rgb.OUTPUT)
        arguments.append(GdalUtils.getFormatShortNameFromFilename(out))
        arguments.append(self.getParameterValue(pct2rgb.INPUT))
        arguments.append(out)

        commands = []
        if ProcessingUtils.isWindows():
            commands = ["cmd.exe", "/C ", "pct2rgb.bat", GdalUtils.escapeAndJoin(arguments)]
        else:
            commands = ["pct2rgb.py", GdalUtils.escapeAndJoin(arguments)]

        GdalUtils.runGdal(commands, progress)
