# -*- coding: utf-8 -*-

"""
***************************************************************************
    lasclassify.py
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

class lasclassify(LasToolsAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"

    def defineCharacteristics(self):
        self.name = "lasclassify"
        self.group = "Tools"
        self.addParameter(ParameterFile(lasclassify.INPUT, "Input las layer"))
        self.addOutput(OutputFile(lasclassify.OUTPUT, "Output classified las file"))
        self.addCommonParameters()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LasToolsUtils.LasToolsPath(), "bin", "lasclassify.exe")]
        commands.append("-i")
        commands.append(self.getParameterValue(lasclassify.INPUT))
        commands.append("-o")
        commands.append(self.getOutputValue(lasclassify.OUTPUT))
        self.addCommonParameterValuesToCommand(commands)

        LasToolsUtils.runLasTools(commands, progress)
