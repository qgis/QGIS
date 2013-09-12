# -*- coding: utf-8 -*-

"""
***************************************************************************
    merge.py
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
from processing.tools.system import *

from processing.outputs.OutputRaster import OutputRaster
from processing.parameters.ParameterBoolean import ParameterBoolean
from processing.parameters.ParameterMultipleInput import ParameterMultipleInput

from processing.gdal.GdalUtils import GdalUtils

class merge(GeoAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    PCT = "PCT"
    SEPARATE = "SEPARATE"

    def getIcon(self):
        filepath = os.path.dirname(__file__) + "/icons/merge.png"
        return QtGui.QIcon(filepath)

    def defineCharacteristics(self):
        self.name = "Merge"
        self.group = "[GDAL] Miscellaneous"
        self.addParameter(ParameterMultipleInput(merge.INPUT, "Input layers", ParameterMultipleInput.TYPE_RASTER))
        self.addParameter(ParameterBoolean(merge.PCT, "Grab pseudocolor table from first layer", False))
        self.addParameter(ParameterBoolean(merge.SEPARATE, "Layer stack", False))
        self.addOutput(OutputRaster(merge.OUTPUT, "Output layer"))

    def processAlgorithm(self, progress):
        arguments = []
        if self.getParameterValue(merge.SEPARATE):
            arguments.append("-separate")
        if self.getParameterValue(merge.PCT):
            arguments.append("-pct")
        arguments.append("-o")
        out = self.getOutputValue(merge.OUTPUT)
        arguments.append(out)
        arguments.append("-of")
        arguments.append(GdalUtils.getFormatShortNameFromFilename(out))
        arguments.extend(self.getParameterValue(merge.INPUT).split(";"))

        commands = []
        if isWindows():
            commands = ["cmd.exe", "/C ", "gdal_merge.bat", GdalUtils.escapeAndJoin(arguments)]
        else:
            commands = ["gdal_merge.py", GdalUtils.escapeAndJoin(arguments)]

        GdalUtils.runGdal(commands, progress)
