# -*- coding: utf-8 -*-

"""
***************************************************************************
    DTM2ASCII.py
    ---------------------
    Date                 : May 2014
    Copyright            : (C) 2014 by Niccolo' Marchi
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
__date__ = 'May 2014'
__copyright__ = "(C) 2014 by Niccolo' Marchi"

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
from processing.core.parameters import ParameterFile
from processing.core.parameters import ParameterSelection
from .FusionAlgorithm import FusionAlgorithm
from .FusionUtils import FusionUtils


class DTM2ASCII(FusionAlgorithm):

    INPUT = 'INPUT'
    SWITCH = 'SWITCH'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('DTM to ASCII')
        self.group, self.i18n_group = self.trAlgorithm('Points')
        self.addParameter(ParameterFile(
            self.INPUT, self.tr('Input canopy surface (.dtm)')))
        self.addParameter(ParameterSelection(
            self.SWITCH, self.tr('Output format'), ['raster (ASCII)', 'csv']))

    def processAlgorithm(self, progress):
        commands = [os.path.join(FusionUtils.FusionPath(), 'DTM2ASCII.exe')]
        commands.append('/verbose')
        if self.getParameterValue(self.SWITCH) == 0:
            commands.append('/raster')
        else:
            commands.append('/csv')
        files = self.getParameterValue(self.INPUT).split(';')
        if len(files) == 1:
            commands.append(self.getParameterValue(self.INPUT))
        else:
            FusionUtils.createFileList(files)
            commands.append(FusionUtils.tempFileListFilepath())
        FusionUtils.runFusion(commands, progress)
