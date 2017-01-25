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
from future import standard_library
standard_library.install_aliases()
from builtins import str

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
from processing.core.parameters import ParameterFile
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterBoolean
from processing.core.outputs import OutputFile
from .FusionUtils import FusionUtils
from .FusionAlgorithm import FusionAlgorithm


class CanopyMaxima(FusionAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    THRESHOLD = 'THRESHOLD'
    GROUND = 'GROUND'
    SUMMARY = 'SUMMARY'
    PARAM_A = 'PARAM_A'
    PARAM_C = 'PARAM_C'
    SHAPE = 'SHAPE'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Canopy Maxima')
        self.group, self.i18n_group = self.trAlgorithm('Points')
        self.addParameter(ParameterFile(
            self.INPUT, self.tr('Input PLANS DTM canopy height model'),
            optional=False))
        self.addParameter(ParameterFile(
            self.GROUND, self.tr('Input ground PLANS DTM layer [optional]')))
        self.addParameter(ParameterNumber(
            self.THRESHOLD, self.tr('Limit analysis to areas above this height threshold'), 0, None, 10.0))

        self.addParameter(ParameterNumber(
            self.PARAM_A, self.tr('Variable window size: parameter A'), 0, None, 2.51503))
        self.addParameter(ParameterNumber(
            self.PARAM_C, self.tr('Parameter C'), 0, None, 0.00901))
        summary = ParameterBoolean(
            self.SUMMARY, self.tr('Tree height summary statistics'), False)
        summary.isAdvanced = True
        self.addParameter(summary)
        shape = ParameterBoolean(
            self.SHAPE, self.tr('Create output shapefiles'), False)
        shape.isAdvanced = True
        self.addParameter(shape)

        self.addOutput(OutputFile(
            self.OUTPUT, self.tr('Output file with maxima'), 'csv'))

        self.addAdvancedModifiers()

    def processAlgorithm(self, feedback):
        commands = [os.path.join(FusionUtils.FusionPath(), 'CanopyMaxima.exe')]
        commands.append('/verbose')
        commands.append('/wse:' + unicode(self.getParameterValue(self.PARAM_A)) + ',0,' + unicode(self.getParameterValue(self.PARAM_C)) + ',0')
        ground = self.getParameterValue(self.GROUND)
        if ground:
            gfiles = self.getParameterValue(self.GROUND).split(';')
            if len(gfiles) == 1:
                commands.append('/ground:' + str(ground))
            else:
                FusionUtils.createGroundList(gfiles)
                commands.append('/ground:' + str(FusionUtils.tempGroundListFilepath()))
        commands.append('/threshold:' + str(self.getParameterValue(self.THRESHOLD)))
        if self.getParameterValue(self.SUMMARY):
            commands.append('/summary')
        self.addAdvancedModifiersToCommand(commands)
        files = self.getParameterValue(self.INPUT).split(';')
        if len(files) == 1:
            commands.append(self.getParameterValue(self.INPUT))
        else:
            FusionUtils.createFileList(files)
            commands.append(FusionUtils.tempFileListFilepath())
        commands.append(self.getOutputValue(self.OUTPUT))

        FusionUtils.runFusion(commands, feedback)
