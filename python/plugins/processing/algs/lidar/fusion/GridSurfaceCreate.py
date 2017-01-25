# -*- coding: utf-8 -*-

"""
***************************************************************************
    GridSurfaceCreate.py
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
from processing.core.parameters import ParameterSelection
from processing.core.outputs import OutputFile
from .FusionAlgorithm import FusionAlgorithm
from .FusionUtils import FusionUtils
from processing.core.parameters import ParameterString


class GridSurfaceCreate(FusionAlgorithm):

    INPUT = 'INPUT'
    OUTPUT_DTM = 'OUTPUT_DTM'
    CELLSIZE = 'CELLSIZE'
    XYUNITS = 'XYUNITS'
    ZUNITS = 'ZUNITS'
    UNITS = ['Meter', 'Feet']
    SPIKE = 'SPIKE'
    MEDIAN = 'MEDIAN'
    SMOOTH = 'SMOOTH'
    SLOPE = 'SLOPE'
    MINIMUM = 'MINIMUM'
    CLASS = 'CLASS'
    ADVANCED_MODIFIERS = 'ADVANCED_MODIFIERS'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Grid Surface Create')
        self.group, self.i18n_group = self.trAlgorithm('Surface')
        self.addParameter(ParameterFile(
            self.INPUT, self.tr('Input LAS layer'),
            optional=False))
        self.addParameter(ParameterNumber(
            self.CELLSIZE, self.tr('Cell Size'), 0, None, 10.0))
        self.addParameter(ParameterSelection(
            self.XYUNITS, self.tr('XY Units'), self.UNITS))
        self.addParameter(ParameterSelection(
            self.ZUNITS, self.tr('Z Units'), self.UNITS))
        self.addOutput(OutputFile(
            self.OUTPUT_DTM, self.tr('DTM Output Surface'), 'dtm'))
        spike = ParameterNumber(
            self.SPIKE, self.tr('Filter final surface to remove spikes with slopes greater than # percent'), 0, None, 0.0)
        spike.isAdvanced = True
        self.addParameter(spike)
        median = ParameterNumber(
            self.MEDIAN, self.tr('Apply median filter to model using # by # neighbor window'), 0, None, 0.0)
        median.isAdvanced = True
        self.addParameter(median)
        smooth = ParameterNumber(
            self.SMOOTH, self.tr('Apply mean filter to model using # by # neighbor window'), 0, None, 0.0)
        smooth.isAdvanced = True
        self.addParameter(smooth)
        slope = ParameterNumber(
            self.SLOPE, self.tr('Filter areas from the surface with slope greater than # percent'), 0, None, 0.0)
        slope.isAdvanced = True
        self.addParameter(slope)
        minimum = ParameterBoolean(
            self.MINIMUM, self.tr('Use the lowest point in each cell as the surface elevation'), False)
        minimum.isAdvanced = True
        self.addParameter(minimum)
        class_var = ParameterString(
            self.CLASS, self.tr('Class(es)'), '', False, True)
        class_var.isAdvanced = True
        self.addParameter(class_var)
        advance_modifiers = ParameterString(
            self.ADVANCED_MODIFIERS, self.tr('Additional modifiers'), '', False, True)
        advance_modifiers.isAdvanced = True
        self.addParameter(advance_modifiers)

    def processAlgorithm(self, feedback):
        commands = [os.path.join(FusionUtils.FusionPath(), 'GridSurfaceCreate.exe')]
        commands.append('/verbose')
        spike = self.getParameterValue(self.SPIKE)
        if spike != 0.0:
            commands.append('/spike:' + str(spike))
        median = self.getParameterValue(self.MEDIAN)
        if median != 0.0:
            commands.append('/median:' + str(median))
        smooth = self.getParameterValue(self.SMOOTH)
        if smooth != 0.0:
            commands.append('/smooth:' + str(smooth))
        slope = self.getParameterValue(self.SLOPE)
        if slope != 0.0:
            commands.append('/slope:' + str(slope))
        minimum = self.getParameterValue(self.MINIMUM)
        if str(minimum).strip():
            commands.append('/minimum:' + str(minimum))
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
        FusionUtils.runFusion(commands, feedback)
