# -*- coding: utf-8 -*-

"""
***************************************************************************
    FirstLastReturn.py
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
from processing.core.parameters import ParameterBoolean
from processing.core.outputs import OutputFile
from FusionAlgorithm import FusionAlgorithm
from FusionUtils import FusionUtils


class FirstLastReturn(FusionAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    SWITCH = 'SWITCH'


    def defineCharacteristics(self):
        self.name = 'First&Last Return'
        self.group = 'Points'
        self.addParameter(ParameterFile(self.INPUT, self.tr('Input .las')))
        self.addParameter(ParameterBoolean(
            self.SWITCH, self.tr('Use LAS info'), True))
        self.addOutput(OutputFile(self.OUTPUT, self.tr('Output layers')))
        self.addAdvancedModifiers()

    def processAlgorithm(self, progress):
        commands = [os.path.join(FusionUtils.FusionPath(), 'FirstLastReturn.exe')]
        commands.append('/verbose')
        if self.getParameterValue(self.SWITCH):
            commands.append('/uselas')
        self.addAdvancedModifiersToCommand(commands)
        outFile = self.getOutputValue(self.OUTPUT)
        commands.append(outFile)
        files = self.getParameterValue(self.INPUT).split(';')
        if len(files) == 1:
            commands.append(self.getParameterValue(self.INPUT))
        else:
            FusionUtils.createFileList(files)
            commands.append(FusionUtils.tempFileListFilepath())
        FusionUtils.runFusion(commands, progress)
