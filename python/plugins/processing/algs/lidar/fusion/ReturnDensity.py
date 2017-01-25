# -*- coding: utf-8 -*-

"""
***************************************************************************
    ReturnDensity.py  #spellok
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
from processing.core.parameters import ParameterString
from processing.core.outputs import OutputFile
from .FusionAlgorithm import FusionAlgorithm
from .FusionUtils import FusionUtils


class ReturnDensity(FusionAlgorithm):  # spellok

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    CELLSIZE = 'CELLSIZE'
    FIRST = 'FIRST'
    ASCII = 'ASCII'
    CLASS = 'CLASS'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Return Density')
        self.group, self.i18n_group = self.trAlgorithm('Surface')
        self.addParameter(ParameterFile(
            self.INPUT, self.tr('Input LAS layer'), optional=False))
        self.addParameter(ParameterNumber(
            self.CELLSIZE, self.tr('Cellsize'), 0, None, 10.0))
        self.addOutput(OutputFile(
            self.OUTPUT, self.tr('Output file')))
        first = ParameterBoolean(
            self.FIRST, self.tr('Use only first returns when computing return counts'), False)
        first.isAdvanced = True
        self.addParameter(first)
        ascii = ParameterBoolean(
            self.ASCII, self.tr('Output raster data in ASCII raster format instead of PLANS DTM format'), False)
        ascii.isAdvanced = True
        self.addParameter(ascii)

        class_var = ParameterString(
            self.CLASS, self.tr('LAS class'), '', False, True)
        class_var.isAdvanced = True
        self.addParameter(class_var)
        self.addAdvancedModifiers()

    def processAlgorithm(self, progress):
        commands = [os.path.join(FusionUtils.FusionPath(), 'ReturnDensity.exe')]  # spellok
        commands.append('/verbose')

        first = self.getParameterValue(self.FIRST)
        if first:
            commands.append('/first')

        ascii = self.getParameterValue(self.ASCII)
        if ascii:
            commands.append('/ascii')

        class_var = self.getParameterValue(self.CLASS)
        if class_var:
            commands.append('/class:' + unicode(class_var))

        self.addAdvancedModifiersToCommand(commands)
        commands.append(self.getOutputValue(self.OUTPUT))
        commands.append(unicode(self.getParameterValue(self.CELLSIZE)))

        files = self.getParameterValue(self.INPUT).split(';')
        if len(files) == 1:
            commands.append(self.getParameterValue(self.INPUT))
        else:
            FusionUtils.createFileList(files)
            commands.append(FusionUtils.tempFileListFilepath())
        FusionUtils.runFusion(commands, progress)
