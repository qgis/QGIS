# -*- coding: utf-8 -*-

"""
***************************************************************************
    CanopyMaxima.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
from PyQt4 import QtGui
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterFile
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterBoolean
from processing.core.outputs import OutputTable
from FusionUtils import FusionUtils
from FusionAlgorithm import FusionAlgorithm


class CanopyMaxima(FusionAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    THRESHOLD = 'THRESHOLD'
    GROUND = 'GROUND'
    SUMMARY = 'SUMMARY'
    PARAM_A = 'PARAM_A'
    PARAM_C = 'PARAM_C'

    def defineCharacteristics(self):
        self.name = 'Canopy Maxima'
        self.group = 'Points'
        self.addParameter(ParameterFile(self.INPUT, 'Input FUSION canopy height model'))
        self.addParameter(ParameterFile(self.GROUND, 'Input ground .dtm layer [optional]'))
        self.addParameter(ParameterNumber(self.THRESHOLD, 'Height threshold',
                          0, None, 10.0))
        ### begin
        self.addParameter(ParameterNumber(self.PARAM_A, 'Variable window size: parameter A',
                          0, None, 2.51503))
        self.addParameter(ParameterNumber(self.PARAM_C, 'Parameter C',
                          0, None, 0.00901))
        self.addParameter(ParameterBoolean(self.SUMMARY, 'Summary (tree height summary statistics)',
                        False))
        ### end
        self.addOutput(OutputTable(self.OUTPUT, 'Output file with maxima'))
        self.addAdvancedModifiers()


    def processAlgorithm(self, progress):
        commands = [os.path.join(FusionUtils.FusionPath(), 'CanopyMaxima.exe')]
        commands.append('/verbose')
        ### begin
        commands.append('/wse:' + str(self.getParameterValue(self.PARAM_A)) + ',0,' + str(self.getParameterValue(self.PARAM_C)) + ',0')
        if self.getParameterValue(self.SUMMARY) == True:
            commands.append('/summary')
        ### end
        self.addAdvancedModifiersToCommand(commands)
        ground = self.getParameterValue(self.GROUND)
        ## here it's necessary to have the support for multiple files like for INPUT.
        if str(ground).strip():
            commands.append('/ground:' + str(ground))
        commands.append('/threshold:'
                        + str(self.getParameterValue(self.THRESHOLD)))
        files = self.getParameterValue(self.INPUT).split(';')
        if len(files) == 1:
            commands.append(self.getParameterValue(self.INPUT))
        else:
            FusionUtils.createFileList(files)
            commands.append(FusionUtils.tempFileListFilepath())
        commands.append(self.getOutputValue(self.OUTPUT))

        FusionUtils.runFusion(commands, progress)
