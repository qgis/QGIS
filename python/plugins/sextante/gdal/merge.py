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

from PyQt4 import QtGui
from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.outputs.OutputRaster import OutputRaster
import os
from sextante.gdal.GdalUtils import GdalUtils
from sextante.core.SextanteUtils import SextanteUtils
from sextante.parameters.ParameterBoolean import ParameterBoolean
from sextante.parameters.ParameterMultipleInput import ParameterMultipleInput

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
        if SextanteUtils.isWindows():
            commands = ["cmd.exe", "/C ", "gdal_merge.bat"]
        else:
            commands = ["gdal_merge.py"]
        if self.getParameterValue(merge.SEPARATE):
            commands.append("-separate")
        if self.getParameterValue(merge.PCT):
            commands.append("-pct")
        commands.append("-o")
        out = self.getOutputValue(merge.OUTPUT)
        commands.append(out)
        commands.append("-of")
        commands.append(GdalUtils.getFormatShortNameFromFilename(out))
        commands.append(self.getParameterValue(merge.INPUT).replace(";", " "))


        GdalUtils.runGdal(commands, progress)
