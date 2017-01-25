# -*- coding: utf-8 -*-

"""
***************************************************************************
    SurfaceStats.py
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
from processing.core.outputs import OutputFile
from .FusionAlgorithm import FusionAlgorithm
from .FusionUtils import FusionUtils


class SurfaceStats(FusionAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    GROUND = 'GROUND'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Surface Statistics')
        self.group, self.i18n_group = self.trAlgorithm('Surface')
        self.addParameter(ParameterFile(
            self.INPUT, self.tr('Input PLANS DTM layer'), optional=False))
        self.addOutput(OutputFile(self.OUTPUT, self.tr('Output file name'), 'csv'))
        ground = ParameterFile(
            self.GROUND, self.tr('Use the specified surface model to represent the ground surface'))
        ground.isAdvanced = True
        self.addParameter(ground)
        self.addAdvancedModifiers()

    def processAlgorithm(self, progress):
        commands = [os.path.join(FusionUtils.FusionPath(), "SurfaceStats.exe")]
        commands.append('/verbose')
        ground = self.getParameterValue(self.GROUND)
        if ground:
            gfiles = self.getParameterValue(self.GROUND).split(';')
            if len(gfiles) == 1:
                commands.append('/ground:' + unicode(ground))
            else:
                FusionUtils.createGroundList(gfiles)
                commands.append('/ground:' + unicode(FusionUtils.tempGroundListFilepath()))
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
