# -*- coding: utf-8 -*-

"""
***************************************************************************
    SplitDTM.py
    ---------------------
    Date                 : November 2016
    Copyright            : (C) 2016 by Niccolo' Marchi
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
__date__ = 'November 2016'
__copyright__ = "(C) 2016 by Niccolo' Marchi"

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
from processing.core.parameters import ParameterFile
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterString
from processing.core.outputs import OutputFile
from .FusionAlgorithm import FusionAlgorithm
from .FusionUtils import FusionUtils


class SplitDTM(FusionAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    COLUMNS = 'COLUMNS'
    ROWS = 'ROWS'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Split PLANS DTM files')
        self.group, self.i18n_group = self.trAlgorithm('Points')
        self.addParameter(ParameterFile(
            self.INPUT, self.tr('Input PLANS DTM file'), optional=False))
        self.addParameter(ParameterNumber(
            self.COLUMNS, self.tr('Number of columns of tiles'), 0, None, 1))
        self.addParameter(ParameterNumber(
            self.ROWS, self.tr('Number of rows of tiles'), 0, None, 1))
        self.addOutput(OutputFile(
            self.OUTPUT, self.tr('Output files')))
        self.addAdvancedModifiers()

    def processAlgorithm(self, progress):
        commands = [os.path.join(FusionUtils.FusionPath(), 'SplitDTM.exe')]
        commands.append('/verbose')
        self.addAdvancedModifiersToCommand(commands)
        files = self.getParameterValue(self.INPUT).split(';')
        if len(files) == 1:
            commands.append(self.getParameterValue(self.INPUT))
        else:
            FusionUtils.createFileList(files)
            commands.append(FusionUtils.tempFileListFilepath())
        outFile = self.getOutputValue(self.OUTPUT)
        commands.append(outFile)
        commands.append(unicode(self.getParameterValue(self.COLUMNS)))
        commands.append(unicode(self.getParameterValue(self.ROWS)))
        FusionUtils.runFusion(commands, progress)
