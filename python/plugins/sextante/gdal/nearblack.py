# -*- coding: utf-8 -*-

"""
***************************************************************************
    nearblack.py
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
from sextante.parameters.ParameterRaster import ParameterRaster
from sextante.parameters.ParameterNumber import ParameterNumber
from sextante.parameters.ParameterBoolean import ParameterBoolean
from sextante.outputs.OutputRaster import OutputRaster

from sextante.gdal.GdalUtils import GdalUtils

class nearblack(GeoAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    NEAR = "NEAR"
    WHITE = "WHITE"

    def getIcon(self):
        filepath = os.path.dirname(__file__) + "/icons/nearblack.png"
        return QtGui.QIcon(filepath)

    def defineCharacteristics(self):
        self.name = "Nearblack"
        self.group = "[GDAL] Analysis"
        self.addParameter(ParameterRaster(nearblack.INPUT, "Input layer", False))
        self.addParameter(ParameterNumber(nearblack.NEAR, "How far from black (white)", 0, None, 15))
        self.addParameter(ParameterBoolean(nearblack.WHITE, "Search for nearly white pixels instead of nearly black", False))
        self.addOutput(OutputRaster(nearblack.OUTPUT, "Output layer"))

    def processAlgorithm(self, progress):
        arguments = []
        arguments.append("-o")
        arguments.append(self.getOutputValue(nearblack.OUTPUT))
        arguments.append("-near")
        arguments.append(str(self.getParameterValue(nearblack.NEAR)))
        if self.getParameterValue(nearblack.WHITE):
            arguments.append("-white")
        arguments.append(self.getParameterValue(nearblack.INPUT))
        GdalUtils.runGdal(["nearblack", GdalUtils.escapeAndJoin(arguments)], progress)
