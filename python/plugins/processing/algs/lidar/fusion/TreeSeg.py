# -*- coding: utf-8 -*-

"""
***************************************************************************
    TreeSeg.py
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
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterFile
from processing.core.parameters import ParameterNumber
from processing.core.outputs import OutputFile
from .FusionAlgorithm import FusionAlgorithm
from .FusionUtils import FusionUtils


class TreeSeg(FusionAlgorithm):

    INPUT = 'INPUT'
    GROUND = 'GROUND'
    HTTH = 'HTTH'
    OUTPUT = 'OUTPUT'
    HEIGHT = 'HEIGHT'
    SHAPE = 'SHAPE'
    ALIGN = 'ALIGN'
    BUFF = 'BUFF'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Tree Segmentation')
        self.group, self.i18n_group = self.trAlgorithm('Points')
        self.addParameter(ParameterFile(
            self.INPUT, self.tr('Input Canopy Height Model (in PLANS DTM format)'), optional=False))
        self.addParameter(ParameterNumber(
            self.HTTH, self.tr('Minimum height for object segmentation'), 0, None, 2.0))
        self.addOutput(OutputFile(
            self.OUTPUT, self.tr('Base name for output files')))

        ground = ParameterFile(
            self.GROUND, self.tr('Ground file for height normalization'))
        ground.isAdvanced = True
        self.addParameter(ground)
        height = ParameterBoolean(
            self.HEIGHT, self.tr("Normalize height model using ground model (select if a ground file is provided)"), False)
        height.isAdvanced = True
        self.addParameter(height)
        buff = ParameterNumber(
            self.BUFF, self.tr('Add a buffer to the data extent when segmenting'), 0, None, 0.0)
        buff.isAdvanced
        self.addParameter(buff)
        shape = ParameterBoolean(
            self.SHAPE, self.tr('Create output shapefiles'), False)
        shape.isAdvanced = True
        self.addParameter(shape)
        align = ParameterBoolean(
            self.ALIGN, self.tr('Align output grid to the input extent'), False)
        align.isAdvanced = True
        self.addParameter(align)

        self.addAdvancedModifiers()

    def processAlgorithm(self, progress):
        commands = [os.path.join(FusionUtils.FusionPath(), 'TreeSeg.exe')]
        commands.append('/verbose')

        ground = self.getParameterValue(self.GROUND)
        if ground:
            gfiles = self.getParameterValue(self.GROUND).split(';')
            if len(gfiles) == 1:
                commands.append('/ground:' + unicode(ground))
            else:
                FusionUtils.createGroundList(gfiles)
                commands.append('/ground:' + unicode(FusionUtils.tempGroundListFilepath()))
        height = self.getParameterValue(self.HEIGHT)
        if height:
            commands.append('/height')
        buff = self.getParameterValue(self.BUFF)
        if buff != 0.0:
            commands.append('/buffer:' + unicode(self.getParameterValue(self.BUFF)))
        shape = self.getParameterValue(self.SHAPE)
        if shape:
            commands.append('/shape')
        align = self.getParameterValue(self.ALIGN)
        if align:
            commands.append('/align:' + unicode(self.getParameterValue(self.INPUT)))
        self.addAdvancedModifiersToCommand(commands)

        files = self.getParameterValue(self.INPUT).split(';')
        if len(files) == 1:
            commands.append(self.getParameterValue(self.INPUT))
        else:
            FusionUtils.createFileList(files)
            commands.append(FusionUtils.tempFileListFilepath())

        commands.append(unicode(self.getParameterValue(self.HTTH)))

        outFile = self.getOutputValue(self.OUTPUT)
        commands.append(outFile)
        FusionUtils.runFusion(commands, progress)
