# -*- coding: utf-8 -*-

"""
***************************************************************************
    ZonalStatistics.py
    ---------------------
    Date                 : September 2016
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
__date__ = 'September 2016'
__copyright__ = '(C) 2016, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtGui import QIcon

from qgis.analysis import QgsZonalStatistics
from qgis.core import (QgsFeatureSink,
                       QgsProcessingUtils,
                       QgsProcessingParameterDefinition,
                       QgsProcessingParameterVectorLayer,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterString,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterEnum,
                       QgsProcessingOutputVectorLayer)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class ZonalStatistics(QgisAlgorithm):

    INPUT_RASTER = 'INPUT_RASTER'
    RASTER_BAND = 'RASTER_BAND'
    INPUT_VECTOR = 'INPUT_VECTOR'
    COLUMN_PREFIX = 'COLUMN_PREFIX'
    STATISTICS = 'STATS'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'zonalstats.png'))

    def group(self):
        return self.tr('Raster tools')

    def __init__(self):
        super().__init__()
        self.STATS = {self.tr('Count'): QgsZonalStatistics.Count,
                      self.tr('Sum'): QgsZonalStatistics.Sum,
                      self.tr('Mean'): QgsZonalStatistics.Mean,
                      self.tr('Median'): QgsZonalStatistics.Median,
                      self.tr('Std. dev.'): QgsZonalStatistics.StDev,
                      self.tr('Min'): QgsZonalStatistics.Min,
                      self.tr('Max'): QgsZonalStatistics.Max,
                      self.tr('Range'): QgsZonalStatistics.Range,
                      self.tr('Minority'): QgsZonalStatistics.Minority,
                      self.tr('Majority (mode)'): QgsZonalStatistics.Majority,
                      self.tr('Variety'): QgsZonalStatistics.Variety,
                      self.tr('Variance'): QgsZonalStatistics.Variance,
                      self.tr('All'): QgsZonalStatistics.All
                      }

        self.addParameter(QgsProcessingParameterRasterLayer(self.INPUT_RASTER,
                                                            self.tr('Raster layer')))
        self.addParameter(QgsProcessingParameterNumber(self.RASTER_BAND,
                                                       self.tr('Raster band'),
                                                       minValue=1, maxValue=999, defaultValue=1))
        self.addParameter(QgsProcessingParameterVectorLayer(self.INPUT_VECTOR,
                                                            self.tr('Vector layer containing zones'),
                                                            [QgsProcessingParameterDefinition.TypeVectorPolygon]))
        self.addParameter(QgsProcessingParameterString(self.COLUMN_PREFIX,
                                                       self.tr('Output column prefix'), '_'))
        self.addParameter(QgsProcessingParameterEnum(self.STATISTICS,
                                                     self.tr('Statistics to calculate'),
                                                     list(self.STATS.keys()),
                                                     allowMultiple=True))
        self.addOutput(QgsProcessingOutputVectorLayer(self.INPUT_VECTOR,
                                                      self.tr('Zonal statistics'),
                                                      QgsProcessingParameterDefinition.TypeVectorPolygon))

    def name(self):
        return 'zonalstatistics'

    def displayName(self):
        return self.tr('Zonal Statistics')

    def processAlgorithm(self, parameters, context, feedback):
        bandNumber = self.parameterAsInt(parameters, self.RASTER_BAND, context)
        columnPrefix = self.parameterAsString(parameters, self.COLUMN_PREFIX, context)
        st = self.parameterAsEnums(parameters, self.STATISTICS, context)

        vectorLayer = self.parameterAsVectorLayer(parameters, self.INPUT_VECTOR, context)
        rasterLayer = self.parameterAsRasterLayer(parameters, self.INPUT_RASTER, context)

        keys = list(self.STATS.keys())
        selectedStats = 0
        for i in st:
            selectedStats |= self.STATS[keys[i]]

        zs = QgsZonalStatistics(vectorLayer,
                                rasterLayer,
                                columnPrefix,
                                bandNumber,
                                selectedStats)
        zs.calculateStatistics(feedback)

        return {self.INPUT_VECTOR: vectorLayer}
