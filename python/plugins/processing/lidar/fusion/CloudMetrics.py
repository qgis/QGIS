# -*- coding: utf-8 -*-

"""
***************************************************************************
    CloudMetrics.py
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
from processing.lidar.fusion.FusionUtils import FusionUtils
from processing.lidar.fusion.FusionAlgorithm import FusionAlgorithm

class CloudMetrics(FusionAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"

    def defineCharacteristics(self):
        self.name = "Cloud Metrics"
        self.group = "Points"
        self.addParameter(ParameterFile(self.INPUT, "Input las layer"))
        self.addOutput(OutputTable(self.OUTPUT, "Output file with tabular metric information"))
        self.addAdvancedModifiers()

    def processAlgorithm(self, progress):
        commands = [os.path.join(FusionUtils.FusionPath(), "CloudMetrics.exe")]
        commands.append("/verbose")
        self.addAdvancedModifiersToCommand(commands)
        files = self.getParameterValue(self.INPUT).split(";")
        if len(files) == 1:
            commands.append(self.getParameterValue(self.INPUT))
        else:
            FusionUtils.createFileList(files)
            commands.append(FusionUtils.tempFileListFilepath())
        commands.append(self.getOutputValue(self.OUTPUT))
        #self.addCommonParameterValuesToCommand(commands)

        FusionUtils.runFusion(commands, progress)
