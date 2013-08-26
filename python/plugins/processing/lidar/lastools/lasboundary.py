# -*- coding: utf-8 -*-

"""
***************************************************************************
    lasboundary.py
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
from processing.parameters.ParameterString import ParameterString
from processing.outputs.OutputVector import OutputVector
from processing.lidar.lastools.LasToolsUtils import LasToolsUtils
from processing.parameters.ParameterBoolean import ParameterBoolean
from processing.parameters.ParameterNumber import ParameterNumber
from processing.lidar.lastools.LasToolsAlgorithm import LasToolsAlgorithm
from processing.parameters.ParameterFile import ParameterFile

class lasboundary(LasToolsAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    CONCAVITY = "CONCAVITY"
    DISJOINT = "DISJOINT"
    HOLES = "HOLES"

    def defineCharacteristics(self):
        self.name = "lasboundary"
        self.group = "Tools"
        self.addParameter(ParameterFile(lasboundary.INPUT, "Input las layer"))
        self.addParameter(ParameterNumber(lasboundary.CONCAVITY, "Concavity threshold", 0, None, 50.0))
        self.addParameter(ParameterBoolean(lasboundary.HOLES, "Compute also interior holes", False))
        self.addParameter(ParameterBoolean(lasboundary.DISJOINT, "Compute disjoint hull", False))
        self.addOutput(OutputVector(lasboundary.OUTPUT, "Output boundary layer"))
        self.addCommonParameters()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LasToolsUtils.LasToolsPath(), "bin", "lasboundary.exe")]
        commands.append("-i")
        commands.append(self.getParameterValue(lasboundary.INPUT))
        commands.append("-o")
        commands.append(self.getOutputValue(lasboundary.OUTPUT))
        commands.append("-concavity")
        commands.append(str(self.getParameterValue(lasboundary.CONCAVITY)))
        if self.getParameterValue(lasboundary.HOLES):
            commands.append("-holes")
        if self.getParameterValue(lasboundary.DISJOINT):
            commands.append("-disjoint")
        self.addCommonParameterValuesToCommand(commands)

        LasToolsUtils.runLasTools(commands, progress)
