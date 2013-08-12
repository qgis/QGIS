# -*- coding: utf-8 -*-

"""
***************************************************************************
    CanopyMaxima.py
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
from processing.outputs.OutputTable import OutputTable
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.lidar.fusion.FusionUtils import FusionUtils
from PyQt4 import QtGui
from processing.parameters.ParameterNumber import ParameterNumber
from processing.lidar.fusion.FusionAlgorithm import FusionAlgorithm

class CanopyMaxima(FusionAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    THRESHOLD = "THRESHOLD"
    GROUND = "GROUND"


    def defineCharacteristics(self):
        self.name = "Canopy Maxima"
        self.group = "Points"
        self.addParameter(ParameterFile(self.INPUT, "Input las layer"))
        self.addParameter(ParameterFile(self.GROUND, "Input ground DTM layer [optional, leave blank if not using it]"))
        self.addParameter(ParameterNumber(self.THRESHOLD, "Minimum threshold", 0, None, 10.0))
        self.addOutput(OutputTable(self.OUTPUT, "Output file with maxima"))
        self.addAdvancedModifiers()

    def processAlgorithm(self, progress):
        commands = [os.path.join(FusionUtils.FusionPath(), "CanopyMaxima.exe")]
        commands.append("/verbose")
        self.addAdvancedModifiersToCommand(commands)
        ground = self.getParameterValue(self.GROUND)
        if str(ground).strip() != "":
            commands.append("/ground:" + str(ground))
        commands.append("/threshold:" + str(self.getParameterValue(self.THRESHOLD)))
        files = self.getParameterValue(self.INPUT).split(";")
        if len(files) == 1:
            commands.append(self.getParameterValue(self.INPUT))
        else:
            FusionUtils.createFileList(files)
            commands.append(FusionUtils.tempFileListFilepath())
        commands.append(self.getOutputValue(self.OUTPUT))

        FusionUtils.runFusion(commands, progress)
