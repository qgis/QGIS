# -*- coding: utf-8 -*-

"""
***************************************************************************
    FilterData.py
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
import subprocess
from processing.outputs.OutputFile import OutputFile
from processing.lidar.fusion.FusionAlgorithm import FusionAlgorithm
from processing.parameters.ParameterNumber import ParameterNumber

class FilterData(FusionAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    VALUE = "VALUE"
    SHAPE = "SHAPE"
    WINDOWSIZE = "WINDOWSIZE"


    def defineCharacteristics(self):
        self.name = "Filter Data outliers"
        self.group = "Points"
        self.addParameter(ParameterFile(self.INPUT, "Input las layer"))
        self.addParameter(ParameterNumber(self.VALUE, "Standard Deviation multiplier"))
        self.addParameter(ParameterNumber(self.VALUE, "Window size", None, None, 10))
        self.addOutput(OutputFile(self.OUTPUT, "Output filtered las file"))
        self.addAdvancedModifiers()

    def processAlgorithm(self, progress):
        commands = [os.path.join(FusionUtils.FusionPath(), "FilterData.exe")]
        commands.append("/verbose")
        self.addAdvancedModifiersToCommand(commands)
        commands.append("outlier")
        commands.append(str(self.getParameterValue(self.VALUE)))
        commands.append(str(self.getParameterValue(self.WINDOWSIZE)))
        outFile = self.getOutputValue(self.OUTPUT) + ".lda"
        commands.append(outFile)
        files = self.getParameterValue(self.INPUT).split(";")
        if len(files) == 1:
            commands.append(self.getParameterValue(self.INPUT))
        else:
            FusionUtils.createFileList(files)
            commands.append(FusionUtils.tempFileListFilepath())
        FusionUtils.runFusion(commands, progress)
        commands = [os.path.join(FusionUtils.FusionPath(), "LDA2LAS.exe")]
        commands.append(outFile)
        commands.append(self.getOutputValue(self.OUTPUT))
        p = subprocess.Popen(commands, shell=True)
        p.wait()
