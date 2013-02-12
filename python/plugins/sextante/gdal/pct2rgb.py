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

from PyQt4 import QtGui
from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.parameters.ParameterRaster import ParameterRaster
from sextante.outputs.OutputRaster import OutputRaster
import os
from sextante.gdal.GdalUtils import GdalUtils
from sextante.parameters.ParameterSelection import ParameterSelection
from sextante.core.SextanteUtils import SextanteUtils

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
        if SextanteUtils.isWindows():
            commands = ["cmd.exe", "/C ", "pct2rgb.bat"]
        else:
            commands = ["pct2rgb.py"]
        commands.append("-b")
        commands.append(str(self.getParameterValue(pct2rgb.NBAND) + 1))
        commands.append("-of")
        out = self.getOutputValue(pct2rgb.OUTPUT)
        commands.append(GdalUtils.getFormatShortNameFromFilename(out))
        commands.append(self.getParameterValue(pct2rgb.INPUT))
        commands.append(out)

        GdalUtils.runGdal(commands, progress)
