# -*- coding: utf-8 -*-

"""
***************************************************************************
    Relief.py
    ---------------------
    Date                 : December 2016
    Copyright            : (C) 2016 by Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Alexander Bruy'
__date__ = 'December 2016'
__copyright__ = '(C) 2016, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtGui import QIcon, QColor

from qgis.analysis import QgsRelief

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import (Parameter,
                                        ParameterRaster,
                                        ParameterNumber,
                                        ParameterBoolean,
                                        _splitParameterOptions,
                                        _createDescriptiveName)
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.outputs import OutputRaster, OutputTable
from processing.tools import raster

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class Relief(GeoAlgorithm):

    INPUT_LAYER = 'INPUT_LAYER'
    Z_FACTOR = 'Z_FACTOR'
    AUTO_COLORS = 'AUTO_COLORS'
    COLORS = 'COLORS'
    OUTPUT_LAYER = 'OUTPUT_LAYER'
    FREQUENCY_DISTRIBUTION = 'FREQUENCY_DISTRIBUTION'

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'dem.png'))

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Relief')
        self.group, self.i18n_group = self.trAlgorithm('Raster terrain analysis')

        class ParameterReliefColors(Parameter):
            default_metadata = {
                'widget_wrapper': 'processing.algs.qgis.ui.ReliefColorsWidget.ReliefColorsWidgetWrapper'
            }

            def __init__(self, name='', description='', parent=None, optional=True):
                Parameter.__init__(self, name, description, None, optional)
                self.parent = parent

            def setValue(self, value):
                if value is None:
                    if not self.optional:
                        return False
                    self.value = None
                    return True

                if value == '':
                    if not self.optional:
                        return False

                if isinstance(value, str):
                    self.value = value if value != '' else None
                else:
                    self.value = ParameterReliefColors.colorsToString(value)
                return True

            def getValueAsCommandLineParameter(self):
                return '"{}"'.format(self.value)

            def getAsScriptCode(self):
                param_type = ''
                param_type += 'relief colors '
                return '##' + self.name + '=' + param_type

            @classmethod
            def fromScriptCode(self, line):
                isOptional, name, definition = _splitParameterOptions(line)
                descName = _createDescriptiveName(name)
                parent = definition.lower().strip()[len('relief colors') + 1:]
                return ParameterReliefColors(name, descName, parent)

            @staticmethod
            def colorsToString(colors):
                s = ''
                for c in colors:
                    s += '{:f}, {:f}, {:d}, {:d}, {:d};'.format(c[0],
                                                                c[1],
                                                                c[2],
                                                                c[3],
                                                                c[4])
                return s[:-1]

        self.addParameter(ParameterRaster(self.INPUT_LAYER,
                                          self.tr('Elevation layer')))
        self.addParameter(ParameterNumber(self.Z_FACTOR,
                                          self.tr('Z factor'),
                                          1.0, 999999.99, 1.0))
        self.addParameter(ParameterBoolean(self.AUTO_COLORS,
                                           self.tr('Generate relief classes automatically'),
                                           False))
        self.addParameter(ParameterReliefColors(self.COLORS,
                                                self.tr('Relief colors'),
                                                self.INPUT_LAYER,
                                                True))
        self.addOutput(OutputRaster(self.OUTPUT_LAYER,
                                    self.tr('Relief')))
        self.addOutput(OutputTable(self.FREQUENCY_DISTRIBUTION,
                                   self.tr('Frequency distribution')))

    def processAlgorithm(self, feedback):
        inputFile = self.getParameterValue(self.INPUT_LAYER)
        zFactor = self.getParameterValue(self.Z_FACTOR)
        automaticColors = self.getParameterValue(self.AUTO_COLORS)
        colors = self.getParameterValue(self.COLORS)
        outputFile = self.getOutputValue(self.OUTPUT_LAYER)
        frequencyDistribution = self.getOutputValue(self.FREQUENCY_DISTRIBUTION)

        outputFormat = raster.formatShortNameFromFileName(outputFile)

        relief = QgsRelief(inputFile, outputFile, outputFormat)

        if automaticColors:
            reliefColors = relief.calculateOptimizedReliefClasses()
        else:
            if colors is None:
                raise GeoAlgorithmExecutionException(
                    self.tr('Specify relief colors or activate "Generate relief classes automatically" option.'))

            reliefColors = []
            for c in colors.split(';'):
                v = c.split(',')
                color = QgsRelief.ReliefColor(QColor(int(v[2]), int(v[3]), int(v[4])),
                                              float(v[0]),
                                              float(v[1]))
                reliefColors.append(color)

        relief.setReliefColors(reliefColors)
        relief.setZFactor(zFactor)
        relief.exportFrequencyDistributionToCsv(frequencyDistribution)
        relief.processRaster(None)
