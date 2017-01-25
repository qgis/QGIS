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
from future import standard_library
standard_library.install_aliases()
from builtins import str

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import subprocess
from processing.core.parameters import ParameterFile
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterSelection
from processing.core.outputs import OutputRaster
from .FusionAlgorithm import FusionAlgorithm
from .FusionUtils import FusionUtils


class Cover(FusionAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    CELLSIZE = 'CELLSIZE'
    HEIGHTBREAK = 'HEIGHTREAK'
    GROUND = 'GROUND'
    ALLRETS = 'ALLRETS'
    PENETRATION = 'PENETRATION'
    XYUNITS = 'XYUNITS'
    ZUNITS = 'ZUNITS'
    UNITS = ['Meter', 'Feet']

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Cover')
        self.group, self.i18n_group = self.trAlgorithm('Points')
        self.addParameter(ParameterFile(
            self.INPUT, self.tr('Input LAS layer'),
            optional=False))
        self.addParameter(ParameterFile(
            self.GROUND, self.tr('Input ground PLANS DTM layer'),
            optional=False))
        self.addParameter(ParameterNumber(
            self.CELLSIZE, self.tr('Cell Size'), 0, None, 10.0))
        self.addParameter(ParameterNumber(
            self.HEIGHTBREAK, self.tr('Heightbreak for the cover calculation (see help)'), 0, None, 10.0))
        self.addParameter(ParameterSelection(
            self.XYUNITS, self.tr('XY Units'), self.UNITS))
        self.addParameter(ParameterSelection(
            self.ZUNITS, self.tr('Z Units'), self.UNITS))
        self.addParameter(ParameterBoolean(
            self.ALLRETS, self.tr('Use all returns instead of only first'), False))
        self.addParameter(ParameterBoolean(
            self.PENETRATION, self.tr('Compute the proportion of returns close to the ground surface'), False))
        self.addOutput(OutputFile(self.OUTPUT, self.tr('Cover output file')))
        self.addAdvancedModifiers()

    def processAlgorithm(self, feedback):
        commands = [os.path.join(FusionUtils.FusionPath(), 'Cover.exe')]
        commands.append('/verbose')
        allrets = self.getParameterValue(self.ALLRETS)
        if str(allrets).strip() != '':
            commands.append('/all')
        penetration = self.getParameterValue(self.PENETRATION)
        if penetration:
            commands.append('/penetration')
        self.addAdvancedModifiersToCommand(commands)
        ground = self.getParameterValue(self.GROUND).split(';')
        if len(ground) == 1:
            commands.append(self.getParameterValue(self.GROUND))
        else:
            FusionUtils.createGroundList(ground)
            commands.append(FusionUtils.tempGroundListFilepath())
        outFile = self.getOutputValue(self.OUTPUT) + '.dtm'
        commands.append(outFile)
        commands.append(str(self.getParameterValue(self.HEIGHTBREAK)))
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
        FusionUtils.runFusion(commands, feedback)
        commands = [os.path.join(FusionUtils.FusionPath(), 'DTM2ASCII.exe')]
        commands.append(outFile)
        commands.append(self.getOutputValue(self.OUTPUT))
        p = subprocess.Popen(commands, shell=True)
        p.wait()
