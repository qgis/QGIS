# -*- coding: utf-8 -*-

"""
***************************************************************************
    GroundFilter.py
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
import subprocess
from processing.core.parameters import ParameterFile
from processing.core.parameters import ParameterNumber
from processing.core.outputs import OutputFile
from FusionUtils import FusionUtils
from FusionAlgorithm import FusionAlgorithm


class GroundFilter(FusionAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    CELLSIZE = 'CELLSIZE'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Ground Filter')
        self.group, self.i18n_group = self.trAlgorithm('Points')
        self.addParameter(ParameterFile(
            self.INPUT, self.tr('Input LAS layer')))
        self.addParameter(ParameterNumber(self.CELLSIZE,
            self.tr('Cellsize for intermediate surfaces'), 0, None, 10))
        self.addOutput(OutputFile(
            self.OUTPUT, self.tr('Output ground LAS file')))
        self.addAdvancedModifiers()

    def processAlgorithm(self, progress):
        commands = [os.path.join(FusionUtils.FusionPath(), 'GroundFilter.exe')]
        commands.append('/verbose')
        self.addAdvancedModifiersToCommand(commands)
        outFile = self.getOutputValue(self.OUTPUT) + '.lda'
        commands.append(str(self.getParameterValue(self.CELLSIZE)))
        commands.append(outFile)
        files = self.getParameterValue(self.INPUT).split(';')
        if len(files) == 1:
            commands.append(self.getParameterValue(self.INPUT))
        else:
            FusionUtils.createFileList(files)
            commands.append(FusionUtils.tempFileListFilepath())
        FusionUtils.runFusion(commands, progress)
        commands = [os.path.join(FusionUtils.FusionPath(), 'LDA2LAS.exe')]
        commands.append(outFile)
        commands.append(self.getOutputValue(self.OUTPUT))
        p = subprocess.Popen(commands, shell=True)
        p.wait()
