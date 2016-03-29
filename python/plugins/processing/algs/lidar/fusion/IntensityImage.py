# -*- coding: utf-8 -*-

"""
***************************************************************************
    IntensityImage.py
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


class IntensityImage(FusionAlgorithm):

    INPUT = 'INPUT'
    ALLRET = 'ALLRET'
    LOWEST = 'LOWEST'
    HIST = 'HIST'
    PIXEL = 'PIXEL'
    SWITCH = 'SWITCH'
    OUTPUT = 'OUTPUT'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('IntensityImage')
        self.group, self.i18n_group = self.trAlgorithm('Points')

        self.addParameter(ParameterFile(
            self.INPUT, self.tr('Input file')))
        self.addParameter(ParameterBoolean(
            self.ALLRET, self.tr('Use all returns instead of only first'), False))
        self.addParameter(ParameterBoolean(
            self.LOWEST, self.tr('Use the lowest return in pixel area to assign the intensity value'), False))
        self.addParameter(ParameterBoolean(
            self.HIST, self.tr('Produce a CSV intensity histogram data file'), False))
        self.addParameter(ParameterNumber(
            self.PIXEL, self.tr('Pixel size'), 0, None, 1.0))
        self.addParameter(ParameterSelection(
            self.SWITCH, self.tr('Output format'), ['Bitmap', 'JPEG']))
        self.addOutput(OutputFile(self.OUTPUT, 'Output image'))
        self.addAdvancedModifiers()

    def processAlgorithm(self, progress):
        commands = [os.path.join(FusionUtils.FusionPath(), 'IntensityImage.exe')]
        commands.append('/verbose')
        if self.getParameterValue(self.ALLRET):
            commands.append('/allreturns')
        if self.getParameterValue(self.LOWEST):
            commands.append('/lowest')
        if self.getParameterValue(self.HIST):
            commands.append('/hist')
        if self.getParameterValue(self.SWITCH) == 1:
            commands.append('/jpg')
        commands.append(unicode(self.getParameterValue(self.PIXEL)))
        outFile = self.getOutputValue(self.OUTPUT)
        commands.append(outFile)
        files = self.getParameterValue(self.INPUT).split(';')
        if len(files) == 1:
            commands.append(self.getParameterValue(self.INPUT))
        else:
            FusionUtils.createFileList(files)
            commands.append(FusionUtils.tempFileListFilepath())
        FusionUtils.runFusion(commands, progress)
