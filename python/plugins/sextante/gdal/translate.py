# -*- coding: utf-8 -*-

"""
***************************************************************************
    translate.py
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
from sextante.parameters.ParameterString import ParameterString

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

class translate(GeoAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    EXTRA = "EXTRA"

    def getIcon(self):
        filepath = os.path.dirname(__file__) + "/icons/translate.png"
        return QtGui.QIcon(filepath)

    def defineCharacteristics(self):
        self.name = "Translate (convert format)"
        self.group = "[GDAL] Conversion"
        self.addParameter(ParameterRaster(translate.INPUT, "Input layer", False))
        self.addParameter(ParameterString(translate.EXTRA, "Additional creation parameters"))
        self.addOutput(OutputRaster(translate.OUTPUT, "Output layer"))

    def processAlgorithm(self, progress):
        commands = ["gdal_translate"]
        commands.append("-of")
        out = self.getOutputValue(translate.OUTPUT)
        extra = self.getOutputValue(translate.EXTRA)
        commands.append(GdalUtils.getFormatShortNameFromFilename(out))
        commands.append(extra)
        commands.append(self.getParameterValue(translate.INPUT))
        commands.append(out)


        GdalUtils.runGdal(commands, progress)
