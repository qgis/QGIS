# -*- coding: utf-8 -*-

"""
***************************************************************************
    ImageCreate.py
    ---------------------
    Date                 : January 2016
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
__date__ = 'January 2016'
__copyright__ = "(C) 2016 by Niccolo' Marchi"

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterFile
from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterNumber
from processing.core.outputs import OutputFile
from .FusionAlgorithm import FusionAlgorithm
from .FusionUtils import FusionUtils


class ImageCreate(FusionAlgorithm):

    INPUT = 'INPUT'
    COLOROPTION = 'COLOROPTION'
    GROUND = 'GROUND'
    PIXEL = 'PIXEL'
    RGB = 'RGB'
    SWITCH = 'SWITCH'
    OUTPUT = 'OUTPUT'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('ImageCreate')
        self.group, self.i18n_group = self.trAlgorithm('Points')
        self.addParameter(ParameterFile(
            self.INPUT, self.tr('Input LAS')))
        self.addParameter(ParameterSelection(
            self.COLOROPTION, self.tr('Method to assign color'),
            ['Intensity', 'Elevation', 'Height']))
        self.addParameter(ParameterFile(
            self.GROUND, self.tr("Ground file (used with 'Height' method)"), 'dtm'))
        self.addParameter(ParameterBoolean(
            self.RGB, self.tr('Use RGB color model to create the color ramp'), False))
        self.addParameter(ParameterNumber(
            self.PIXEL, self.tr('Pixel size'), 0, None, 1.0))
        self.addParameter(ParameterSelection(
            self.SWITCH, self.tr('Output format'), ['JPEG', 'Bitmap']))
        self.addOutput(OutputFile(self.OUTPUT, 'Output image'))
        self.addAdvancedModifiers()

    def processAlgorithm(self, progress):
        commands = [os.path.join(FusionUtils.FusionPath(), 'ImageCreate.exe')]
        commands.append('/verbose')
        commands.append('/coloroption:' + unicode(self.getParameterValue(self.COLOROPTION)))
        ground = self.getParameterValue(self.GROUND)
        if unicode(ground).strip():
            commands.append('/dtm:' + unicode(ground))
        if self.getParameterValue(self.RGB):
            commands.append('/rgb')
        if self.getParameterValue(self.SWITCH) == 0:
            commands.append('/jpg')
        else:
            commands.append('/bmp')
        outFile = self.getOutputValue(self.OUTPUT)
        commands.append(outFile)
        commands.append(unicode(self.getParameterValue(self.PIXEL)))
        files = self.getParameterValue(self.INPUT).split(';')
        if len(files) == 1:
            commands.append(self.getParameterValue(self.INPUT))
        else:
            FusionUtils.createFileList(files)
            commands.append(FusionUtils.tempFileListFilepath())
        FusionUtils.runFusion(commands, progress)
