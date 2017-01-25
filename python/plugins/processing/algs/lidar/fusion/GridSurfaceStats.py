# -*- coding: utf-8 -*-

"""
***************************************************************************
    GridSurfaceStats.py
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
from processing.core.parameters import ParameterBoolean
from processing.core.outputs import OutputFile
from .FusionAlgorithm import FusionAlgorithm
from .FusionUtils import FusionUtils


class GridSurfaceStats(FusionAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    SAFACT = 'SAFACT'
    AREA = 'AREA'
    ASCII = 'ASCII'
    SVONLY = 'SVONLY'
    GROUND = 'GROUND'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Grid Surface Stats')
        self.group, self.i18n_group = self.trAlgorithm('Surface')
        self.addParameter(ParameterFile(
            self.INPUT, self.tr('Input PLANS DTM canopy height model'), optional=False))
        self.addParameter(ParameterNumber(
            self.SAFACT, self.tr('Multiplier for outputfile cell size'), 0, None, 1.0))
        self.addOutput(OutputFile(
            self.OUTPUT, self.tr('Output file')))
        area = ParameterBoolean(
            self.AREA, self.tr('Compute the surface area of inputfile instead of the surface area divided by the flat cell area'), False)
        area.isAdvanced = True
        self.addParameter(area)
        ascii = ParameterBoolean(
            self.ASCII, self.tr('Output all files in ASCII raster format instead of PLANS DTM ones'), False)
        ascii.isAdvanced = True
        self.addParameter(ascii)
        svonly = ParameterBoolean(
            self.SVONLY, self.tr('Output only the surface volume metric layer'), False)
        svonly.isAdvanced = True
        self.addParameter(svonly)

        ground = ParameterFile(
            self.GROUND, self.tr('Use the specified surface model to represent the ground surface'), False, True)
        ground.isAdvanced = True
        self.addParameter(ground)

        self.addAdvancedModifiers()

    def processAlgorithm(self, progress):
        commands = [os.path.join(FusionUtils.FusionPath(), 'GridSurfaceStats.exe')]
        commands.append('/verbose')
        area = self.getParameterValue(self.AREA)
        if area:
            commands.append('/area')
        ascii = self.getParameterValue(self.ASCII)
        if ascii:
            commands.append('/ascii')
        svonly = self.getParameterValue(self.SVONLY)
        if svonly:
            commands.append('/svonly')
        ground = self.getParameterValue(self.GROUND)
        if ground:
            gfiles = self.getParameterValue(self.GROUND).split(';')
            if len(gfiles) == 1:
                commands.append('/ground:' + unicode(ground))
            else:
                FusionUtils.createGroundList(gfiles)
                commands.append('/ground:' + unicode(FusionUtils.tempGroundListFilepath()))
        self.addAdvancedModifiersToCommand(commands)
        files = self.getParameterValue(self.INPUT).split(';')
        if len(files) == 1:
            commands.append(self.getParameterValue(self.INPUT))
        else:
            FusionUtils.createFileList(files)
            commands.append(FusionUtils.tempFileListFilepath())
        commands.append(self.getOutputValue(self.OUTPUT))
        commands.append(unicode(self.getParameterValue(self.SAFACT)))
        FusionUtils.runFusion(commands, progress)
