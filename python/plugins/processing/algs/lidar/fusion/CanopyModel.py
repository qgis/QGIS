# -*- coding: utf-8 -*-

"""
***************************************************************************
    CanopyModel.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
    ---------------------
    Date                 : June 2014
    Copyright            : (C) 2014 by Agresta S. Coop.
    Email                : iescamochero at agresta dot org
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
from processing.core.parameters import ParameterFile
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterString
from processing.core.outputs import OutputFile
from FusionAlgorithm import FusionAlgorithm
from FusionUtils import FusionUtils


class CanopyModel(FusionAlgorithm):

    INPUT = 'INPUT'
    OUTPUT_DTM = 'OUTPUT_DTM'
    CELLSIZE = 'CELLSIZE'
    XYUNITS = 'XYUNITS'
    ZUNITS = 'ZUNITS'
    UNITS = ['Meter', 'Feet']
    GROUND = 'GROUND'
    MEDIAN = 'MEDIAN'
    SMOOTH = 'SMOOTH'
    SLOPE = 'SLOPE'
    CLASS = 'CLASS'
    ADVANCED_MODIFIERS = 'ADVANCED_MODIFIERS'

    def defineCharacteristics(self):
        self.name = 'Canopy Model'
        self.group = 'Points'
        self.addParameter(ParameterFile(
            self.INPUT, self.tr('Input las layer')))
        self.addParameter(ParameterNumber(
            self.CELLSIZE, self.tr('Cellsize'), 0, None, 10.0))
        self.addParameter(ParameterSelection(
            self.XYUNITS, self.tr('XY Units'), self.UNITS))
        self.addParameter(ParameterSelection(
            self.ZUNITS, self.tr('Z Units'), self.UNITS))
        self.addOutput(OutputFile(
            self.OUTPUT_DTM, self.tr('DTM Output Surface'), 'dtm'))
        ground = ParameterFile(
            self.GROUND, self.tr('Input ground DTM layer'), False, True)
        ground.isAdvanced = True
        self.addParameter(ground)
        median = ParameterString(
            self.MEDIAN, self.tr('Median'), '', False, True)
        median.isAdvanced = True
        self.addParameter(median)
        smooth = ParameterString(
            self.SMOOTH, self.tr('Smooth'), '', False, True)
        smooth.isAdvanced = True
        self.addParameter(smooth)
        slope = ParameterString(
            self.SLOPE, self.tr('Slope'), '', False, True)
        slope.isAdvanced = True
        self.addParameter(slope)
        class_var = ParameterString(
            self.CLASS, self.tr('Class'), '', False, True)
        class_var.isAdvanced = True
        self.addParameter(class_var)
        advance_modifiers = ParameterString(
            self.ADVANCED_MODIFIERS, self.tr('Additional modifiers'), '', False, True)
        advance_modifiers.isAdvanced = True
        self.addParameter(advance_modifiers)

    def processAlgorithm(self, progress):
        commands = [os.path.join(FusionUtils.FusionPath(), 'CanopyModel.exe')]
        commands.append('/verbose')
        ground = self.getParameterValue(self.GROUND)
        if str(ground).strip():
            commands.append('/ground:' + str(ground))
        median = self.getParameterValue(self.MEDIAN)
        if str(median).strip():
            commands.append('/median:' + str(median))
        smooth = self.getParameterValue(self.SMOOTH)
        if str(smooth).strip():
            commands.append('/smooth:' + str(smooth))
        slope = self.getParameterValue(self.SLOPE)
        if str(slope).strip():
            commands.append('/slope:' + str(slope))
        class_var = self.getParameterValue(self.CLASS)
        if str(class_var).strip():
            commands.append('/class:' + str(class_var))
        advance_modifiers = str(self.getParameterValue(self.ADVANCED_MODIFIERS)).strip()
        if advance_modifiers:
            commands.append(advance_modifiers)
        commands.append(self.getOutputValue(self.OUTPUT_DTM))
        commands.append(str(self.getParameterValue(self.CELLSIZE)))
        commands.append(self.UNITS[self.getParameterValue(self.XYUNITS)][0])
        commands.append(self.UNITS[self.getParameterValue(self.ZUNITS)][0])
        commands.append('0')
        commands.append('0')
        commands.append('0')
        commands.append('0')
        files = self.getParameterValue(self.INPUT).split(';')
        if len(files) == 1:
            commands.append(self.getParameterValue(self.INPUT))
        else:
            FusionUtils.createFileList(files)
            commands.append(FusionUtils.tempFileListFilepath())
        FusionUtils.runFusion(commands, progress)
