# -*- coding: utf-8 -*-

"""
***************************************************************************
    MergeDTM.py
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


class MergeDTM(FusionAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    CELLSIZE = 'CELLSIZE'
    EXTENT = 'EXTENT'
    DISK = 'DISK'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Merge PLANS DTM files')
        self.group, self.i18n_group = self.trAlgorithm('Points')
        self.addParameter(ParameterFile(
            self.INPUT, self.tr('Input PLANS DTM files'), optional=False))
        self.addOutput(OutputFile(
            self.OUTPUT, self.tr('Output merged file')))

        cellsize = ParameterNumber(
            self.CELLSIZE, self.tr('Resample the input DTM data to the following cellsize'), 0, None, 0.0)
        cellsize.isAdvanced = True
        self.addParameter(cellsize)

        extent = ParameterBoolean(
            self.EXTENT, self.tr('Preserve the exact extent of the input models'), False)
        extent.isAdvanced = True
        self.addParameter(extent)

        disk = ParameterBoolean(
            self.DISK, self.tr('Merge the files to a disk file. USE ONLY IF DEFAULT METHOD FAILS'), False)
        disk.isAdvanced = True
        self.addParameter(disk)

        self.addAdvancedModifiers()

    def processAlgorithm(self, progress):
        commands = [os.path.join(FusionUtils.FusionPath(), 'MergeDTM.exe')]
        commands.append('/verbose')

        cellsize = self.getParameterValue(self.CELLSIZE)
        if cellsize != 0.0:
            commands.append('/cellsize:' + unicode(self.getParameterValue(self.CELLSIZE)))
        extent = self.getParameterValue(self.EXTENT)
        if extent:
            commands.append('/exactextent')
        disk = self.getParameterValue(self.DISK)
        if disk:
            commands.append('/disk')
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
