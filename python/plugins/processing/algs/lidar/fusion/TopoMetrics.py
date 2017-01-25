# -*- coding: utf-8 -*-

"""
***************************************************************************
    TopoMetrics.py
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
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterFile
from processing.core.parameters import ParameterNumber
from processing.core.outputs import OutputFile
from .FusionAlgorithm import FusionAlgorithm
from .FusionUtils import FusionUtils


class TopoMetrics(FusionAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    CELLSIZE = 'CELLSIZE'
    POINTSP = 'POINTSP'
    LATITUDE = 'LATITUDE'
    TPI = 'TPI'
    SQUARE = 'SQUARE'
    DISK = 'DISK'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Topographic Metrics')
        self.group, self.i18n_group = self.trAlgorithm('Points')
        self.addParameter(ParameterFile(
            self.INPUT, self.tr('Input PLANS DTM surface files'), optional=False))
        self.addOutput(OutputFile(
            self.OUTPUT, self.tr('Output file')))

        self.addParameter(ParameterNumber(
            self.CELLSIZE, self.tr('Size of the cell used to report topographic metrics'), 0, None, 5.0))
        self.addParameter(ParameterNumber(
            self.POINTSP, self.tr('Spacing for the 3 by 3 array of points used to compute the metrics'), 0, None, 0.0))
        self.addParameter(ParameterNumber(
            self.LATITUDE, self.tr('Latitude'), 2, None, 45.0))
        self.addParameter(ParameterNumber(
            self.TPI, self.tr('TPI window size'), 0, None, 5.0))

        square = ParameterBoolean(
            self.SQUARE, self.tr('Use a square window for TPI'), False)
        square.isAdvanced = True
        self.addParameter(square)

        disk = ParameterBoolean(
            self.DISK, self.tr('Do not load ground surface models into memory'), False)
        disk.isAdvanced = True
        self.addParameter(disk)

        self.addAdvancedModifiers()

    def processAlgorithm(self, progress):
        commands = [os.path.join(FusionUtils.FusionPath(), 'TopoMetrics.exe')]
        commands.append('/verbose')

        square = self.getParameterValue(self.SQUARE)
        if square:
            commands.append('/square')
        disk = self.getParameterValue(self.DISK)
        if disk:
            commands.append('/disk')
        self.addAdvancedModifiersToCommand(commands)
        files = self.getParameterValue(self.INPUT).split(';')
        if len(files) == 1:
            commands.append(self.getParameterValue(self.INPUT))
        else:
            FusionUtils.createFileList(files)
            commands.append(FusionUtils.tempFileListFilepath())

        commands.append(unicode(self.getParameterValue(self.CELLSIZE)))
        commands.append(unicode(self.getParameterValue(self.POINTSP)))
        commands.append(unicode(self.getParameterValue(self.LATITUDE)))
        commands.append(unicode(self.getParameterValue(self.TPI)))

        outFile = self.getOutputValue(self.OUTPUT)
        commands.append(outFile)
        FusionUtils.runFusion(commands, progress)
