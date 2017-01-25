# -*- coding: utf-8 -*-

"""
***************************************************************************
    MergeRaster.py
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
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterString
from processing.core.outputs import OutputFile
from .FusionAlgorithm import FusionAlgorithm
from .FusionUtils import FusionUtils


class MergeRaster(FusionAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    OVERL = 'OVERL'
    COMP = 'COMP'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Merge ASCII files')
        self.group, self.i18n_group = self.trAlgorithm('Points')
        self.addParameter(ParameterFile(
            self.INPUT, self.tr('Input ASCII files'), optional=False))
        self.addOutput(OutputFile(
            self.OUTPUT, self.tr('Output file'), 'asc'))

        overl = ParameterString(
            self.OVERL, self.tr('Specify how overlap areas should be treated'), '', False, True)
        overl.isAdvanced = True
        self.addParameter(overl)
        comp = ParameterBoolean(
            self.COMP, self.tr('Compare values in cells common to two or more input files'), False)
        comp.isAdvanced = True
        self.addParameter(comp)
        self.addAdvancedModifiers()

    def processAlgorithm(self, progress):
        commands = [os.path.join(FusionUtils.FusionPath(), 'MergeRaster.exe')]
        commands.append('/verbose')
        overl = self.getParameterValue(self.OVERL)
        if overl:
            commands.append('/overlap:' + unicode(overl))
        comp = self.getParameterValue(self.COMP)
        if comp:
            commands.append('/compare')
        self.addAdvancedModifiersToCommand(commands)
        outFile = self.getOutputValue(self.OUTPUT)
        commands.append(outFile)
        files = self.getParameterValue(self.INPUT).split(';')
        if len(files) == 1:
            commands.append(self.getParameterValue(self.INPUT))
        else:
            FusionUtils.createFileList(files)
            commands.append(FusionUtils.tempFileListFilepath())
        FusionUtils.runFusion(commands, progress)
