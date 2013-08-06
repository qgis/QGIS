# -*- coding: utf-8 -*-

"""
***************************************************************************
    lasclip.py
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
from sextante.lidar.lastools.LasToolsUtils import LasToolsUtils
from sextante.lidar.lastools.LasToolsAlgorithm import LasToolsAlgorithm
from sextante.parameters.ParameterFile import ParameterFile
from sextante.outputs.OutputFile import OutputFile
from sextante.parameters.ParameterVector import ParameterVector

class lasclip(LasToolsAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    POLYGON = "POLYGON"

    def defineCharacteristics(self):
        self.name = "lasclip"
        self.group = "Tools"
        self.addParameter(ParameterFile(lasclip.INPUT, "Input las layer"))
        self.addParameter(ParameterVector(lasclip.POLYGON, "Input polygons", [ParameterVector.VECTOR_TYPE_POLYGON]))
        self.addOutput(OutputFile(lasclip.OUTPUT, "Output classified las file"))
        self.addCommonParameters()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LasToolsUtils.LasToolsPath(), "bin", "lasclip.exe")]
        commands.append("-i")
        commands.append(self.getParameterValue(lasclip.INPUT))
        commands.append("-poly")
        commands.append(self.getParameterValue(lasclip.POLYGON))
        commands.append("-o")
        commands.append(self.getOutputValue(lasclip.OUTPUT))
        self.addCommonParameterValuesToCommand(commands)

        LasToolsUtils.runLasTools(commands, progress)
