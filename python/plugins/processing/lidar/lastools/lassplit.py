# -*- coding: utf-8 -*-

"""
***************************************************************************
    lassplit.py
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
from processing.lidar.lastools.LasToolsUtils import LasToolsUtils
from processing.lidar.lastools.LasToolsAlgorithm import LasToolsAlgorithm
from processing.parameters.ParameterFile import ParameterFile
from processing.outputs.OutputFile import OutputFile
from processing.parameters.ParameterNumber import ParameterNumber

class lassplit(LasToolsAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    NUM_POINTS = "NUM_POINTS"

    def defineCharacteristics(self):
        self.name = "lassplit"
        self.group = "Tools"
        self.addParameter(ParameterFile(lassplit.INPUT, "Input las layer"))
        self.addParameter(ParameterNumber(lassplit.NUM_POINTS, "Point in each output file", 1, None, 1000000))
        self.addOutput(OutputFile(lassplit.OUTPUT, "Output las file basename"))
        self.addCommonParameters()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LasToolsUtils.LasToolsPath(), "bin", "lassplit.exe")]
        commands.append("-i")
        commands.append(self.getParameterValue(lassplit.INPUT))
        commands.append("-o")
        commands.append(self.getOutputValue(lassplit.OUTPUT))
        commands.append("-split")
        commands.append(self.getParameterValue(lassplit.NUM_POINTS))
        self.addCommonParameterValuesToCommand(commands)

        LasToolsUtils.runLasTools(commands, progress)
