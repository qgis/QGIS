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

import os
from collections import OrderedDict

from qgis.PyQt.QtGui import QIcon

from qgis.analysis import QgsZonalStatistics
from qgis.core import (QgsProcessing,
                       QgsProcessingAlgorithm,
                       QgsProcessingParameterVectorLayer,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterString,
                       QgsProcessingParameterBand,
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
        return self.tr('Raster analysis')

    def groupId(self):
        return 'rasteranalysis'

    def flags(self):
        return super().flags() | QgsProcessingAlgorithm.FlagNoThreading

    def __init__(self):
        super().__init__()
        self.bandNumber = None
        self.columnPrefix = None
        self.selectedStats = None
        self.vectorLayer = None
        self.raster_interface = None
        self.raster_crs = None
        self.raster_units_per_pixel_x = None
        self.raster_units_per_pixel_y = None

    def initAlgorithm(self, config=None):
        self.STATS = OrderedDict([(self.tr('Count'), QgsZonalStatistics.Count),
                                  (self.tr('Sum'), QgsZonalStatistics.Sum),
                                  (self.tr('Mean'), QgsZonalStatistics.Mean),
                                  (self.tr('Median'), QgsZonalStatistics.Median),
                                  (self.tr('Std. dev.'), QgsZonalStatistics.StDev),
                                  (self.tr('Min'), QgsZonalStatistics.Min),
                                  (self.tr('Max'), QgsZonalStatistics.Max),
                                  (self.tr('Range'), QgsZonalStatistics.Range),
                                  (self.tr('Minority'), QgsZonalStatistics.Minority),
                                  (self.tr('Majority (mode)'), QgsZonalStatistics.Majority),
                                  (self.tr('Variety'), QgsZonalStatistics.Variety),
                                  (self.tr('Variance'), QgsZonalStatistics.Variance),
                                  (self.tr('All'), QgsZonalStatistics.All)])

        self.addParameter(QgsProcessingParameterRasterLayer(self.INPUT_RASTER,
                                                            self.tr('Raster layer')))
        self.addParameter(QgsProcessingParameterBand(self.RASTER_BAND,
                                                     self.tr('Raster band'),
                                                     1,
                                                     self.INPUT_RASTER))
        self.addParameter(QgsProcessingParameterVectorLayer(self.INPUT_VECTOR,
                                                            self.tr('Vector layer containing zones'),
                                                            [QgsProcessing.TypeVectorPolygon]))
        self.addParameter(QgsProcessingParameterString(self.COLUMN_PREFIX,
                                                       self.tr('Output column prefix'), '_'))
        keys = list(self.STATS.keys())
        self.addParameter(QgsProcessingParameterEnum(self.STATISTICS,
                                                     self.tr('Statistics to calculate'),
                                                     keys,
                                                     allowMultiple=True, defaultValue=[0, 1, 2]))
        self.addOutput(QgsProcessingOutputVectorLayer(self.INPUT_VECTOR,
                                                      self.tr('Zonal statistics'),
                                                      QgsProcessing.TypeVectorPolygon))

    def name(self):
        return 'zonalstatistics'

    def displayName(self):
        return self.tr('Zonal statistics')

    def prepareAlgorithm(self, parameters, context, feedback):
        self.bandNumber = self.parameterAsInt(parameters, self.RASTER_BAND, context)
        self.columnPrefix = self.parameterAsString(parameters, self.COLUMN_PREFIX, context)
        st = self.parameterAsEnums(parameters, self.STATISTICS, context)

        keys = list(self.STATS.keys())
        self.selectedStats = 0
        for i in st:
            self.selectedStats |= self.STATS[keys[i]]

        self.vectorLayer = self.parameterAsVectorLayer(parameters, self.INPUT_VECTOR, context)
        rasterLayer = self.parameterAsRasterLayer(parameters, self.INPUT_RASTER, context)
        self.raster_interface = rasterLayer.dataProvider().clone()
        self.raster_crs = rasterLayer.crs()
        self.raster_units_per_pixel_x = rasterLayer.rasterUnitsPerPixelX()
        self.raster_units_per_pixel_y = rasterLayer.rasterUnitsPerPixelY()
        return True

    def processAlgorithm(self, parameters, context, feedback):
        zs = QgsZonalStatistics(self.vectorLayer,
                                self.raster_interface,
                                self.raster_crs,
                                self.raster_units_per_pixel_x,
                                self.raster_units_per_pixel_y,
                                self.columnPrefix,
                                self.bandNumber,
                                QgsZonalStatistics.Statistics(self.selectedStats))
        zs.calculateStatistics(feedback)
        return {self.INPUT_VECTOR: self.vectorLayer}
