# -*- coding: utf-8 -*-

"""
***************************************************************************
    las2dem.py
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
from processing.parameters.ParameterString import ParameterString
from processing.lidar.lastools.LasToolsUtils import LasToolsUtils
from processing.parameters.ParameterBoolean import ParameterBoolean
from processing.outputs.OutputRaster import OutputRaster
from processing.lidar.lastools.LasToolsAlgorithm import LasToolsAlgorithm
from processing.parameters.ParameterFile import ParameterFile

class las2dem(LasToolsAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    INTENSITY = "INTENSITY"

    def defineCharacteristics(self):
        self.name = "las2dem"
        self.group = "Tools"
        self.addParameter(ParameterFile(las2dem.INPUT, "Input las layer"))
        self.addParameter(ParameterBoolean(las2dem.INTENSITY, "Use intensity instead of elevation", False))
        self.addOutput(OutputRaster(las2dem.OUTPUT, "Output dem layer"))
        self.addCommonParameters()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LasToolsUtils.LasToolsPath(), "bin", "las2dem.exe")]
        commands.append("-i")
        commands.append(self.getParameterValue(las2dem.INPUT))
        commands.append("-o")
        commands.append(self.getOutputValue(las2dem.OUTPUT))
        if self.getParameterValue(las2dem.INTENSITY):
            commands.append("-intensity")
        self.addCommonParameterValuesToCommand(commands)

        LasToolsUtils.runLasTools(commands, progress)
