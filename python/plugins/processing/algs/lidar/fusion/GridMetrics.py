# -*- coding: utf-8 -*-

"""
***************************************************************************
    GridMetrics.py
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
from processing.parameters.ParameterFile import ParameterFile
from processing.parameters.ParameterNumber import ParameterNumber
from processing.parameters.ParameterBoolean import ParameterBoolean
from processing.parameters.ParameterString import ParameterString
from processing.outputs.OutputFile import OutputFile
from processing.lidar.fusion.FusionUtils import FusionUtils
from processing.lidar.fusion.FusionAlgorithm import FusionAlgorithm


class GridMetrics(FusionAlgorithm):

    INPUT = 'INPUT'
    OUTPUT_CSV_ELEVATION = 'OUTPUT_CSV_ELEVATION'
    OUTPUT_CSV_INTENSITY = 'OUTPUT_CSV_INTENSITY'
    OUTPUT_TXT_ELEVATION = 'OUTPUT_TXT_ELEVATION'
    OUTPUT_TXT_INTENSITY = 'OUTPUT_TXT_INTENSITY'
    GROUND = 'GROUND'
    HEIGHT = 'HEIGHT'
    CELLSIZE = 'CELLSIZE'
    OUTLIER = 'OUTLIER'
    FIRST = 'FIRST'
    MINHT = 'MINHT'
    CLASS = 'CLASS'

    def defineCharacteristics(self):
        self.name = 'Grid Metrics'
        self.group = 'Points'
        self.addParameter(ParameterFile(self.INPUT, 'Input las layer'))
        self.addParameter(ParameterFile(self.GROUND, 'Input ground DTM layer'))
        self.addParameter(ParameterNumber(self.HEIGHT, 'Height break'))
        self.addParameter(ParameterNumber(self.CELLSIZE, 'Cellsize'))

        self.addOutput(OutputFile(self.OUTPUT_CSV_ELEVATION, 'Output table with grid metrics'))

        output_csv_intensity = OutputFile(self.OUTPUT_CSV_INTENSITY, 'OUTPUT CSV INTENSITY')
        output_csv_intensity.hidden = True
        self.addOutput(output_csv_intensity)

        output_txt_elevation = OutputFile(self.OUTPUT_TXT_ELEVATION, 'OUTPUT CSV INTENSITY')
        output_txt_elevation.hidden = True
        self.addOutput(output_txt_elevation)

        output_txt_intensity = OutputFile(self.OUTPUT_TXT_INTENSITY, 'OUTPUT CSV INTENSITY')
        output_txt_intensity.hidden = True
        self.addOutput(output_txt_intensity)

        outlier = ParameterString(self.OUTLIER, 'Outlier:low,high', '', False, True)
        outlier.isAdvanced = True
        self.addParameter(outlier)
        first = ParameterBoolean(self.FIRST, 'First', 0)
        first.isAdvanced = True
        self.addParameter(first)
        minht = ParameterString(self.MINHT, 'Htmin', '', False, True)
        minht.isAdvanced = True
        self.addParameter(minht)
        class_var = ParameterString(self.CLASS, 'Class - If multiple, separated by comma (set blank if not used)', '', False, True)
        class_var.isAdvanced = True
        self.addParameter(class_var)
        

    def processAlgorithm(self, progress):
        commands = [os.path.join(FusionUtils.FusionPath(), 'GridMetrics.exe')]
        commands.append('/verbose')
        outlier = self.getParameterValue(self.OUTLIER)
        if str(outlier).strip() != '':
            commands.append('/outlier:' + str(outlier))
        first = self.getParameterValue(self.FIRST)
        if first == 1:
            commands.append('/first:' + str(first))
        minht = self.getParameterValue(self.MINHT)
        if str(minht).strip() != '':
            commands.append('/minht:' + str(minht))
        class_var = self.getParameterValue(self.CLASS)
        if str(class_var).strip() != '':
            commands.append('/class:' + str(class_var))
        commands.append(self.getParameterValue(self.GROUND))
        commands.append(str(self.getParameterValue(self.HEIGHT)))
        commands.append(str(self.getParameterValue(self.CELLSIZE)))
        commands.append(self.getOutputValue(self.OUTPUT_CSV_ELEVATION))
        files = self.getParameterValue(self.INPUT).split(';')
        if len(files) == 1:
            commands.append(self.getParameterValue(self.INPUT))
        else:
            FusionUtils.createFileList(files)
            commands.append(FusionUtils.tempFileListFilepath())
        FusionUtils.runFusion(commands, progress)
        self.setOutputValue(self.OUTPUT_CSV_ELEVATION, self.OUTPUT_CSV_ELEVATION + '_all_returns_elevation_stats.csv')
        self.setOutputValue(self.OUTPUT_CSV_INTENSITY, self.OUTPUT_CSV_INTENSITY + '_all_returns_intensity_stats.csv')
        self.setOutputValue(self.OUTPUT_CSV_ELEVATION, self.OUTPUT_TXT_ELEVATION + '_all_returns_elevation_stats_ascii_header.txt')
        self.setOutputValue(self.OUTPUT_CSV_ELEVATION, self.OUTPUT_TXT_INTENSITY + '_all_returns_intensity_stats_ascii_header.txt')
