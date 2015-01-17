# -*- coding: utf-8 -*-

"""
***************************************************************************
    DTM2TIF.py
    ---------------------
    Date                 : May 2014
    Copyright            : (C) 2014 by Niccolo' Marchi
    Email                : sciurusurbanus at hotmail dot it
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Niccolo' Marchi"
__date__ = 'May 2014'
__copyright__ = "(C) 2014 by Niccolo' Marchi"

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
from processing.core.parameters import ParameterFile
from processing.core.outputs import OutputRaster
from FusionAlgorithm import FusionAlgorithm
from FusionUtils import FusionUtils


class DTM2TIF(FusionAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    CSV = 'CSV'


    def defineCharacteristics(self):
        self.name = "DTM to TIF"
        self.group = "Conversion"
        self.addParameter(ParameterFile(
            self.INPUT, self.tr("Input .dtm layer")))
        self.addOutput(OutputRaster(self.OUTPUT, self.tr('Output file name')))
        self.addAdvancedModifiers()

    def processAlgorithm(self, progress):
        commands = [os.path.join(FusionUtils.FusionPath(), "DTM2TIF.exe")]
        commands.append("/verbose")
        self.addAdvancedModifiersToCommand(commands)
        files = self.getParameterValue(self.INPUT).split(";")
        if len(files) == 1:
            commands.append(self.getParameterValue(self.INPUT))
        else:
            FusionUtils.createFileList(files)
            commands.append(FusionUtils.tempFileListFilepath())
        outFile = self.getOutputValue(self.OUTPUT)
        commands.append(outFile)

        FusionUtils.runFusion(commands, progress)
