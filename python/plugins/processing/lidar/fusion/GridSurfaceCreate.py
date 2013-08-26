# -*- coding: utf-8 -*-

"""
***************************************************************************
    GridSurfaceCreate.py
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
from processing.parameters.ParameterFile import ParameterFile
from processing.lidar.fusion.FusionUtils import FusionUtils
from processing.parameters.ParameterNumber import ParameterNumber
from processing.parameters.ParameterSelection import ParameterSelection
from processing.lidar.fusion.FusionAlgorithm import FusionAlgorithm
from processing.outputs.OutputFile import OutputFile

class GridSurfaceCreate(FusionAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    CELLSIZE = "CELLSIZE"
    XYUNITS = "XYUNITS"
    ZUNITS = "ZUNITS"
    UNITS = ["Meter", "Feet"]

    def defineCharacteristics(self):
        self.name = "Create Grid Surface"
        self.group = "Surface"
        self.addParameter(ParameterFile(self.INPUT, "Input las layer"))
        self.addParameter(ParameterNumber(self.CELLSIZE, "Cellsize", 0, None, 10.0))
        self.addParameter(ParameterSelection(self.XYUNITS, "XY Units", self.UNITS))
        self.addParameter(ParameterSelection(self.ZUNITS, "Z Units", self.UNITS))
        self.addOutput(OutputFile(self.OUTPUT, "PLANS DTM surface"))
        self.addAdvancedModifiers()

    def processAlgorithm(self, progress):
        commands = [os.path.join(FusionUtils.FusionPath(), "GridSurfaceCreate.exe")]
        commands.append("/verbose")
        self.addAdvancedModifiersToCommand(commands)
        commands.append(self.getOutputValue(self.OUTPUT))
        commands.append(str(self.getParameterValue(self.CELLSIZE)))
        commands.append(self.UNITS[self.getParameterValue(self.XYUNITS)][0])
        commands.append(self.UNITS[self.getParameterValue(self.ZUNITS)][0])
        commands.append("0")
        commands.append("0")
        commands.append("0")
        commands.append("0")
        files = self.getParameterValue(self.INPUT).split(";")
        if len(files) == 1:
            commands.append(self.getParameterValue(self.INPUT))
        else:
            FusionUtils.createFileList(files)
            commands.append(FusionUtils.tempFileListFilepath())
        FusionUtils.runFusion(commands, progress)
