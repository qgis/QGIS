# -*- coding: utf-8 -*-

"""
***************************************************************************
    RasterLayerStatistics.py
    ---------------------
    Date                 : January 2013
    Copyright            : (C) 2013 by Victor Olaya
    Email                : volayaf at gmail dot com
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
__date__ = 'January 2013'
__copyright__ = '(C) 2013, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import codecs

from qgis.core import (QgsRectangle,
                       QgsRasterBandStats,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterBand,
                       QgsProcessingParameterFileDestination,
                       QgsProcessingOutputNumber)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class RasterLayerStatistics(QgisAlgorithm):

    INPUT = 'INPUT'
    BAND = 'BAND'
    OUTPUT_HTML_FILE = 'OUTPUT_HTML_FILE'

    MIN = 'MIN'
    MAX = 'MAX'
    RANGE = 'RANGE'
    SUM = 'SUM'
    MEAN = 'MEAN'
    STD_DEV = 'STD_DEV'
    SUM_OF_SQUARES = 'SUM_OF_SQUARES'

    def group(self):
        return self.tr('Raster analysis')

    def groupId(self):
        return 'rasteranalysis'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterRasterLayer(self.INPUT,
                                                            self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterBand(self.BAND,
                                                     self.tr('Band number'),
                                                     1,
                                                     self.INPUT))
        self.addParameter(QgsProcessingParameterFileDestination(self.OUTPUT_HTML_FILE, self.tr('Statistics'), self.tr('HTML files (*.html)'), None, True))

        self.addOutput(QgsProcessingOutputNumber(self.MIN, self.tr('Minimum value')))
        self.addOutput(QgsProcessingOutputNumber(self.MAX, self.tr('Maximum value')))
        self.addOutput(QgsProcessingOutputNumber(self.RANGE, self.tr('Range')))
        self.addOutput(QgsProcessingOutputNumber(self.SUM, self.tr('Sum')))
        self.addOutput(QgsProcessingOutputNumber(self.MEAN, self.tr('Mean value')))
        self.addOutput(QgsProcessingOutputNumber(self.STD_DEV, self.tr('Standard deviation')))
        self.addOutput(QgsProcessingOutputNumber(self.SUM_OF_SQUARES, self.tr('Sum of the squares')))

    def name(self):
        return 'rasterlayerstatistics'

    def displayName(self):
        return self.tr('Raster layer statistics')

    def processAlgorithm(self, parameters, context, feedback):
        layer = self.parameterAsRasterLayer(parameters, self.INPUT, context)
        band = self.parameterAsInt(parameters, self.BAND, context)
        outputFile = self.parameterAsFileOutput(parameters, self.OUTPUT_HTML_FILE, context)

        stat = layer.dataProvider().bandStatistics(band, QgsRasterBandStats.All, QgsRectangle(), 0)

        data = []
        data.append(self.tr('Analyzed file: {} (band {})').format(layer.source(), band))
        data.append(self.tr('Minimum value: {}').format(stat.minimumValue))
        data.append(self.tr('Maximum value: {}').format(stat.maximumValue))
        data.append(self.tr('Range: {}').format(stat.range))
        data.append(self.tr('Sum: {}').format(stat.sum))
        data.append(self.tr('Mean value: {}').format(stat.mean))
        data.append(self.tr('Standard deviation: {}').format(stat.stdDev))
        data.append(self.tr('Sum of the squares: {}').format(stat.sumOfSquares))

        results = {self.MIN: stat.minimumValue,
                   self.MAX: stat.maximumValue,
                   self.RANGE: stat.range,
                   self.SUM: stat.sum,
                   self.MEAN: stat.mean,
                   self.STD_DEV: stat.stdDev,
                   self.SUM_OF_SQUARES: stat.sumOfSquares}

        if outputFile:
            self.createHTML(outputFile, data)
            results[self.OUTPUT_HTML_FILE] = outputFile

        return results

    def createHTML(self, outputFile, algData):
        with codecs.open(outputFile, 'w', encoding='utf-8') as f:
            f.write('<html><head>\n')
            f.write('<meta http-equiv="Content-Type" content="text/html; \
                    charset=utf-8" /></head><body>\n')
            for s in algData:
                f.write('<p>' + str(s) + '</p>\n')
            f.write('</body></html>\n')
