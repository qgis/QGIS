# -*- coding: utf-8 -*-

"""
***************************************************************************
    DensityMetrics.py
    ---------------------
    Date                 : August 2016
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
__date__ = 'August 2016'
__copyright__ = "(C) 2016 by Niccolo' Marchi"

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import subprocess
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterFile
from processing.core.parameters import ParameterNumber
from processing.core.outputs import OutputFile
from .FusionAlgorithm import FusionAlgorithm
from .FusionUtils import FusionUtils


class DensityMetrics(FusionAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    CELLSIZE = 'CELLSIZE'
    SLICE = 'SLICE'
    GROUND = 'GROUND'
    FIRST = 'FIRST'
    NOCSV = 'NOCSV'
    HTLIM = 'HTLIM'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Density Metrics')
        self.group, self.i18n_group = self.trAlgorithm('Points')
        self.addParameter(ParameterFile(
            self.INPUT, self.tr('Input LAS layer'), optional=False))
        self.addParameter(ParameterFile(
            self.GROUND, self.tr('Input ground PLANS DTM layer'), optional=False))
        self.addParameter(ParameterNumber(
            self.CELLSIZE, self.tr('Cellsize'), 0, None, 5.0))
        self.addParameter(ParameterNumber(
            self.SLICE, self.tr('Slice thickness'), 0, None, 2.0))
        self.addParameter(ParameterNumber(
            self.HTLIM, self.tr('Maximum height limit'), 0, None, 50.0))
        self.addParameter(ParameterBoolean(
            self.FIRST, self.tr('Use only first returns'), False))
        self.addParameter(ParameterBoolean(
            self.NOCSV, self.tr('Do not create a CSV output file for cell metrics'), False))
        self.addOutput(OutputFile(
            self.OUTPUT, self.tr('Base name for output files')))
        self.addAdvancedModifiers()

    def processAlgorithm(self, progress):
        commands = [os.path.join(FusionUtils.FusionPath(), 'DensityMetrics.exe')]
        commands.append('/verbose')
        first = self.getParameterValue(self.FIRST)
        if first:
            commands.append('/first')
        nocsv = self.getParameterValue(self.NOCSV)
        if nocsv:
            commands.append('/nocsv')
        commands.append('/maxsliceht:' + unicode(self.getParameterValue(self.HTLIM)))
        self.addAdvancedModifiersToCommand(commands)
        ground = self.getParameterValue(self.GROUND).split(';')
        if len(ground) == 1:
            commands.append(self.getParameterValue(self.GROUND))
        else:
            FusionUtils.createGroundList(ground)
            commands.append(FusionUtils.tempGroundListFilepath())
        commands.append(unicode(self.getParameterValue(self.CELLSIZE)))
        commands.append(unicode(self.getParameterValue(self.SLICE)))
        outFile = self.getOutputValue(self.OUTPUT)
        commands.append(outFile)
        files = self.getParameterValue(self.INPUT).split(';')
        if len(files) == 1:
            commands.append(self.getParameterValue(self.INPUT))
        else:
            FusionUtils.createFileList(files)
            commands.append(FusionUtils.tempFileListFilepath())
        FusionUtils.runFusion(commands, progress)
