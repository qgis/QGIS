# -*- coding: utf-8 -*-

"""
***************************************************************************
    Cover.py
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
from processing.core.parameters import ParameterSelection
from processing.core.outputs import OutputRaster
from FusionAlgorithm import FusionAlgorithm
from FusionUtils import FusionUtils


class Cover(FusionAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    CELLSIZE = 'CELLSIZE'
    HEIGHTBREAK = 'HEIGHTREAK'
    GROUND = 'GROUND'
    XYUNITS = 'XYUNITS'
    ZUNITS = 'ZUNITS'
    UNITS = ['Meter', 'Feet']

    def defineCharacteristics(self):
        self.name = 'Cover'
        self.group = 'Points'
        self.addParameter(ParameterFile(
            self.INPUT, self.tr('Input LAS layer')))
        self.addParameter(ParameterFile(
            self.GROUND, self.tr('Input ground DTM layer')))
        self.addParameter(ParameterNumber(
            self.CELLSIZE, self.tr('Cellsize'), 0, None, 10.0))
        self.addParameter(ParameterNumber(
            self.HEIGHTBREAK, self.tr('Heightbreak'), 0, None, 10.0))
        self.addParameter(ParameterSelection(
            self.XYUNITS, self.tr('XY Units'), self.UNITS))
        self.addParameter(ParameterSelection(
            self.ZUNITS, self.tr('Z Units'), self.UNITS))
        self.addOutput(OutputRaster(self.OUTPUT, self.tr('Cover')))
        self.addAdvancedModifiers()

    def processAlgorithm(self, progress):
        commands = [os.path.join(FusionUtils.FusionPath(), 'Cover.exe')]
        commands.append('/verbose')
        self.addAdvancedModifiersToCommand(commands)
        ground = self.getParameterValue(self.GROUND)
        if str(ground).strip() != '':
            commands.append('/ground:' + str(ground))
        outFile = self.getOutputValue(self.OUTPUT) + '.dtm'
        commands.append(outFile)
        commands.append(str(self.getParameterValue(self.CELLSIZE)))
        commands.append(self.UNITS[self.getParameterValue(self.XYUNITS)][0])
        commands.append(self.UNITS[self.getParameterValue(self.ZUNITS)][0])
        commands.append('0')
        commands.append('0')
        commands.append('0')
        commands.append('0')
        files = self.getParameterValue(self.INPUT).split(';')
        if len(files) == 1:
            commands.append(self.getParameterValue(self.INPUT))
        else:
            FusionUtils.createFileList(files)
            commands.append(FusionUtils.tempFileListFilepath())
        FusionUtils.runFusion(commands, progress)
        commands = [os.path.join(FusionUtils.FusionPath(), 'DTM2TIF.exe')]
        commands.append(outFile)
        commands.append(self.getOutputValue(self.OUTPUT))
        p = subprocess.Popen(commands, shell=True)
        p.wait()
