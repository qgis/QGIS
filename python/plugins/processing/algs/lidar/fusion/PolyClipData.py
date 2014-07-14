# -*- coding: utf-8 -*-

"""
***************************************************************************
    PolyClipData.py
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
import subprocess
from PyQt4 import QtGui
from processing.core.parameters import ParameterFile
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterString
from processing.core.outputs import OutputFile
from FusionAlgorithm import FusionAlgorithm
from FusionUtils import FusionUtils


class PolyClipData(FusionAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    SHAPE = 'SHAPE'
    MASK = 'MASK'
    FIELD ='FIELD'
    VALUE = 'VALUE'

    def defineCharacteristics(self):
        self.name = 'Poly Clip Data'
        self.group = 'Points'
        self.addParameter(ParameterFile(self.INPUT, 'Input .las layer'))
        self.addParameter(ParameterFile(self.MASK, 'Mask layer'))
        self.addOutput(OutputFile(self.OUTPUT, 'Output clipped .las file', 'las'))
        self.addParameter(ParameterBoolean(self.SHAPE, 'Use Shape attribute ', False))
        ##  'field' e 'value' box should appear or get activated if Shape attribute is switched ON
        self.addParameter(ParameterString(self.FIELD, 'Shape field index'))
        self.addParameter(ParameterString(self.VALUE, "Shape value"))
        self.addAdvancedModifiers()


    def processAlgorithm(self, progress):
        commands = [os.path.join(FusionUtils.FusionPath(), 'PolyClipData.exe')]
        commands.append('/verbose')
        if self.getParameterValue(self.SHAPE):
            commands.append('/shape:' + str(self.getParameterValue(self.FIELD)) + ',' + str(self.getParameterValue(self.VALUE)))
        self.addAdvancedModifiersToCommand(commands)
        commands.append(self.getParameterValue(self.MASK))
        outFile = self.getOutputValue(self.OUTPUT)
        commands.append(outFile)
        files = self.getParameterValue(self.INPUT).split(';')
        if len(files) == 1:
            commands.append(self.getParameterValue(self.INPUT))
        else:
            FusionUtils.createFileList(files)
            commands.append(FusionUtils.tempFileListFilepath())

        FusionUtils.runFusion(commands, progress)