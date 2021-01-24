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

import os

from qgis.PyQt.QtGui import QIcon, QColor

from qgis.analysis import QgsRelief
from qgis.core import (QgsProcessingParameterDefinition,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterRasterDestination,
                       QgsProcessingParameterFileDestination,
                       QgsRasterFileWriter,
                       QgsProcessingException)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class ParameterReliefColors(QgsProcessingParameterDefinition):

    def __init__(self, name='', description='', parent=None, optional=True):
        super().__init__(name, description, None, optional)
        self.parent = parent
        self.setMetadata({'widget_wrapper': 'processing.algs.qgis.ui.ReliefColorsWidget.ReliefColorsWidgetWrapper'})

    def type(self):
        return 'relief_colors'

    def clone(self):
        return ParameterReliefColors(self.name(), self.description(), self.parent,
                                     self.flags() & QgsProcessingParameterDefinition.FlagOptional)

    @staticmethod
    def valueToColors(value):
        if value is None:
            return None

        if value == '':
            return None

        if isinstance(value, str):
            return value.split(';')
        else:
            return ParameterReliefColors.colorsToString(value)

    @staticmethod
    def colorsToString(colors):
        return ';'.join('{:f}, {:f}, {:d}, {:d}, {:d}'.format(c[0],
                                                              c[1],
                                                              c[2],
                                                              c[3],
                                                              c[4])
                        for c in colors)


class Relief(QgisAlgorithm):
    INPUT = 'INPUT'
    Z_FACTOR = 'Z_FACTOR'
    AUTO_COLORS = 'AUTO_COLORS'
    COLORS = 'COLORS'
    OUTPUT = 'OUTPUT'
    FREQUENCY_DISTRIBUTION = 'FREQUENCY_DISTRIBUTION'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'dem.png'))

    def group(self):
        return self.tr('Raster terrain analysis')

    def groupId(self):
        return 'rasterterrainanalysis'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterRasterLayer(self.INPUT,
                                                            self.tr('Elevation layer')))
        self.addParameter(QgsProcessingParameterNumber(self.Z_FACTOR,
                                                       self.tr('Z factor'), type=QgsProcessingParameterNumber.Double,
                                                       minValue=0.00, defaultValue=1.0))
        self.addParameter(QgsProcessingParameterBoolean(self.AUTO_COLORS,
                                                        self.tr('Generate relief classes automatically'),
                                                        defaultValue=False))
        self.addParameter(ParameterReliefColors(self.COLORS,
                                                self.tr('Relief colors'),
                                                self.INPUT,
                                                True))
        self.addParameter(QgsProcessingParameterRasterDestination(self.OUTPUT,
                                                                  self.tr('Relief')))
        self.addParameter(QgsProcessingParameterFileDestination(self.FREQUENCY_DISTRIBUTION,
                                                                self.tr('Frequency distribution'),
                                                                'CSV files (*.csv)',
                                                                optional=True,
                                                                createByDefault=False))

    def name(self):
        return 'relief'

    def displayName(self):
        return self.tr('Relief')

    def processAlgorithm(self, parameters, context, feedback):
        inputFile = self.parameterAsRasterLayer(parameters, self.INPUT, context).source()
        zFactor = self.parameterAsDouble(parameters, self.Z_FACTOR, context)
        automaticColors = self.parameterAsBoolean(parameters, self.AUTO_COLORS, context)
        outputFile = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        frequencyDistribution = self.parameterAsFileOutput(parameters, self.FREQUENCY_DISTRIBUTION, context)

        outputFormat = QgsRasterFileWriter.driverForExtension(os.path.splitext(outputFile)[1])

        relief = QgsRelief(inputFile, outputFile, outputFormat)

        if automaticColors:
            reliefColors = relief.calculateOptimizedReliefClasses()
        else:
            colors = ParameterReliefColors.valueToColors(parameters[self.COLORS])
            if colors is None or len(colors) == 0:
                raise QgsProcessingException(
                    self.tr('Specify relief colors or activate "Generate relief classes automatically" option.'))

            reliefColors = []
            for c in colors:
                v = c.split(',')
                color = QgsRelief.ReliefColor(QColor(int(v[2]), int(v[3]), int(v[4])),
                                              float(v[0]),
                                              float(v[1]))
                reliefColors.append(color)

        relief.setReliefColors(reliefColors)
        relief.setZFactor(zFactor)
        if frequencyDistribution:
            relief.exportFrequencyDistributionToCsv(frequencyDistribution)
        res = relief.processRaster(feedback)
        if res == 1:
            raise QgsProcessingException(self.tr('Can not open input file.'))
        elif res == 2:
            raise QgsProcessingException(self.tr('Can not get GDAL driver for output file.'))
        elif res == 3:
            raise QgsProcessingException(self.tr('Can not create output file.'))
        elif res == 4:
            raise QgsProcessingException(self.tr('Can not get input band.'))
        elif res == 5:
            raise QgsProcessingException(self.tr('Can not create output bands.'))
        elif res == 6:
            raise QgsProcessingException(self.tr('Output raster size is too small (at least 3 rows needed).'))
        elif res == 7:
            feedback.pushInfo(self.tr('Canceled.'))

        return {self.OUTPUT: outputFile, self.FREQUENCY_DISTRIBUTION: frequencyDistribution}
