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
from future import standard_library
standard_library.install_aliases()
from builtins import str

__author__ = 'Agresta S. Coop - www.agresta.org'
__date__ = 'June 2014'
__copyright__ = '(C) 2014, Agresta S. Coop'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
from processing.core.parameters import ParameterFile
from processing.core.parameters import ParameterString
from processing.core.parameters import ParameterBoolean
from processing.core.outputs import OutputFile
from .FusionUtils import FusionUtils
from .FusionAlgorithm import FusionAlgorithm


class Catalog(FusionAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    DENSITY = 'DENSITY'
    FIRSTDENSITY = 'FIRSTDENSITY'
    INTENSITY = 'INTENSITY'
    INDEX = 'INDEX'
    IMAGE = 'IMAGE'
    DRAWTILES = 'DRAWTILES'
    COVERAGE = 'COVERAGE'
    CRETURNS = 'CRETURNS'
    ADVANCED_MODIFIERS = 'ADVANCED_MODIFIERS'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Catalog')
        self.group, self.i18n_group = self.trAlgorithm('Points')
        self.addParameter(ParameterFile(
            self.INPUT, self.tr('Input LAS layer'),
            optional=False))
        self.addOutput(OutputFile(self.OUTPUT, self.tr('Output files')))
        density = ParameterString(
            self.DENSITY,
            self.tr('Density - area, min, max (set blank if not used)'),
            '', False, True)
        density.isAdvanced = True
        self.addParameter(density)
        firest_density = ParameterString(
            self.FIRSTDENSITY,
            self.tr('First Density - area, min, max (set blank if not used)'),
            '', False, True)
        firest_density.isAdvanced = True
        self.addParameter(firest_density)
        intensity = ParameterString(
            self.INTENSITY,
            self.tr('Intensity - area, min, max (set blank if not used)'),
            '', False, True)
        intensity.isAdvanced = True
        self.addParameter(intensity)
        self.addParameter(ParameterBoolean(self.INDEX,
                                           self.tr('Create LIDAR data file indexes'), False))
        self.addParameter(ParameterBoolean(self.IMAGE,
                                           self.tr('Create image files showing the coverage area for each LIDAR file'), False))
        self.addParameter(ParameterBoolean(self.DRAWTILES,
                                           self.tr('Draw data file extents and names on the intensity image'), False))
        self.addParameter(ParameterBoolean(self.COVERAGE,
                                           self.tr('Create one image that shows the nominal coverage area'), False))
        self.addParameter(ParameterBoolean(self.CRETURNS,
                                           self.tr('Adds count return columns in the CSV and HTML output'), False))
        advanced_modifiers = ParameterString(
            self.ADVANCED_MODIFIERS,
            self.tr('Additional modifiers'), '', False, True)
        advanced_modifiers.isAdvanced = True
        self.addParameter(advanced_modifiers)

    def processAlgorithm(self, feedback):
        commands = [os.path.join(FusionUtils.FusionPath(), 'Catalog.exe')]
        commands.append('/verbose')
        intensity = self.getParameterValue(self.INTENSITY)
        if str(intensity).strip():
            commands.append('/intensity:' + str(intensity))
        density = self.getParameterValue(self.DENSITY)
        if str(density).strip():
            commands.append('/density:' + str(density))
        firstdensity = self.getParameterValue(self.FIRSTDENSITY)
        if str(firstdensity).strip():
            commands.append('/firstdensity:' + str(firstdensity))
        index = self.getParameterValue(self.INDEX)
        if str(index).strip():
            commands.append('/index')
        drawtiles = self.getParameterValue(self.IMAGE)
        if str(drawtiles).strip():
            commands.append('/drawtiles')
        coverage = self.getParameterValue(self.DRAWTILES)
        if str(coverage).strip():
            commands.append('/coverage')
        image = self.getParameterValue(self.COVERAGE)
        if str(image).strip():
            commands.append('/image')
        creturns = self.getParameterValue(self.COVERAGE)
        if str(creturns).strip():
            commands.append('/countreturns')
        advanced_modifiers = str(self.getParameterValue(self.ADVANCED_MODIFIERS)).strip()
        if advanced_modifiers:
            commands.append(advanced_modifiers)
        files = self.getParameterValue(self.INPUT).split(';')
        if len(files) == 1:
            commands.append(self.getParameterValue(self.INPUT))
        else:
            FusionUtils.createFileList(files)
            commands.append(FusionUtils.tempFileListFilepath())
        commands.append(self.getOutputValue(self.OUTPUT))
        FusionUtils.runFusion(commands, feedback)
