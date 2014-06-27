# -*- coding: utf-8 -*- 

"""
***************************************************************************
    Catalog.py
    ---------------------
    Date                 : June 2014
    Copyright            : (C) 2014 by Agresta S. Coop
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

__author__ = 'Agresta S. Coop - www.agresta.org'
__date__ = 'June 2014'
__copyright__ = '(C) 2014, Agresta S. Coop'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
import subprocess
from processing.parameters.ParameterFile import ParameterFile
from processing.parameters.ParameterString import ParameterString
from processing.parameters.ParameterNumber import ParameterNumber
from processing.parameters.ParameterBoolean import ParameterBoolean
from processing.outputs.OutputFile import OutputFile
from processing.lidar.fusion.FusionUtils import FusionUtils
from processing.lidar.fusion.FusionAlgorithm import FusionAlgorithm


class Catalog(FusionAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    DENSITY = 'DENSITY'
    FIRSTDENSITY = 'FIRSTDENSITY'
    INTENSITY = 'INTENSITY'
    ADVANCED_MODIFIERS = 'ADVANCED_MODIFIERS'

    def defineCharacteristics(self):
        self.name = 'Catalog'
        self.group = 'Points'
        self.addParameter(ParameterFile(self.INPUT, 'Input las layer'))       
        self.addOutput(OutputFile(self.OUTPUT, 'Output files'))
        density = ParameterString(self.DENSITY, 'Density - area, min, max (set blank if not used)', '', False, True)
        density.isAdvanced = True
        self.addParameter(density)
        firest_density = ParameterString(self.FIRSTDENSITY, 'First Density - area, min, max (set blank if not used)', '', False, True)
        firest_density.isAdvanced = True
        self.addParameter(firest_density)
        intensity = ParameterString(self.INTENSITY, 'Intensity - area, min, max (set blank if not used)', '', False, True)
        intensity.isAdvanced = True
        self.addParameter(intensity)
        advance_modifiers = ParameterString(self.ADVANCED_MODIFIERS, 'Additional modifiers', '', False, True)
        advance_modifiers.isAdvanced = True
        self.addParameter(advance_modifiers)

        
    def processAlgorithm(self, progress):
        commands = [os.path.join(FusionUtils.FusionPath(), 'Catalog.exe')]
        commands.append('/verbose')
        intensity = self.getParameterValue(self.INTENSITY)
        if str(intensity).strip() != '':
            commands.append('/intensity:' + str(intensity))
        density = self.getParameterValue(self.DENSITY)
        if str(density).strip() != '':
            commands.append('/density:' + str(density))
        firstdensity = self.getParameterValue(self.FIRSTDENSITY)
        if str(firstdensity).strip() != '':
            commands.append('/firstdensity:' + str(firstdensity))
        advance_modifiers = str(self.getParameterValue(self.ADVANCED_MODIFIERS)).strip()
        if advance_modifiers != '':
            commands.append(s)
        files = self.getParameterValue(self.INPUT).split(';')
        if len(files) == 1:
            commands.append(self.getParameterValue(self.INPUT))
        else:
            FusionUtils.createFileList(files)
            commands.append(FusionUtils.tempFileListFilepath())                
        commands.append(self.getOutputValue(self.OUTPUT))        
        FusionUtils.runFusion(commands, progress)