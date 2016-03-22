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
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterFile
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterString
from processing.core.outputs import OutputFile
from .FusionAlgorithm import FusionAlgorithm
from .FusionUtils import FusionUtils


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
    ASCII = 'ASCII'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Canopy Model')
        self.group, self.i18n_group = self.trAlgorithm('Points')
        self.addParameter(ParameterFile(
            self.INPUT, self.tr('Input LAS layer')))
        self.addParameter(ParameterNumber(
            self.CELLSIZE, self.tr('Cellsize'), 0, None, 10.0))
        self.addParameter(ParameterSelection(
            self.XYUNITS, self.tr('XY Units'), self.UNITS))
        self.addParameter(ParameterSelection(
            self.ZUNITS, self.tr('Z Units'), self.UNITS))
        self.addOutput(OutputFile(
            self.OUTPUT_DTM, self.tr('.dtm output surface'), 'dtm'))
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
        class_var = ParameterString(
            self.CLASS, self.tr('Class'), '', False, True)
        class_var.isAdvanced = True
        self.addParameter(class_var)
        slope = ParameterBoolean(
            self.SLOPE, self.tr('Calculate slope'), False)
        slope.isAdvanced = True
        self.addParameter(slope)
        self.addParameter(ParameterBoolean(
            self.ASCII, self.tr('Add an ASCII output'), False))
        self.addAdvancedModifiers()

    def processAlgorithm(self, progress):
        commands = [os.path.join(FusionUtils.FusionPath(), 'CanopyModel.exe')]
        commands.append('/verbose')
        ground = self.getParameterValue(self.GROUND)
        if unicode(ground).strip():
            commands.append('/ground:' + unicode(ground))
        median = self.getParameterValue(self.MEDIAN)
        if unicode(median).strip():
            commands.append('/median:' + unicode(median))
        smooth = self.getParameterValue(self.SMOOTH)
        if unicode(smooth).strip():
            commands.append('/smooth:' + unicode(smooth))
        slope = self.getParameterValue(self.SLOPE)
        if slope:
            commands.append('/slope')
        class_var = self.getParameterValue(self.CLASS)
        if unicode(class_var).strip():
            commands.append('/class:' + unicode(class_var))
        ascii = self.getParameterValue(self.ASCII)
        if ascii:
            commands.append('/ascii')
        self.addAdvancedModifiersToCommand(commands)
        commands.append(self.getOutputValue(self.OUTPUT_DTM))
        commands.append(unicode(self.getParameterValue(self.CELLSIZE)))
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
