# -*- coding: utf-8 -*-

"""
***************************************************************************
    BasicStatistics.py
    ---------------------
    Date                 : November 2016
    Copyright            : (C) 2016 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Nyall Dawson'
__date__ = 'November 2016'
__copyright__ = '(C) 2016, Nyall Dawson'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import codecs

from qgis.PyQt.QtCore import QVariant
from qgis.PyQt.QtGui import QIcon

from qgis.core import (QgsStatisticalSummary,
                       QgsStringStatisticalSummary,
                       QgsDateTimeStatisticalSummary,
                       QgsFeatureRequest)

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterTable
from processing.core.parameters import ParameterTableField
from processing.core.outputs import OutputHTML
from processing.core.outputs import OutputNumber
from processing.tools import dataobjects, vector


pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class BasicStatisticsForField(GeoAlgorithm):

    INPUT_LAYER = 'INPUT_LAYER'
    FIELD_NAME = 'FIELD_NAME'
    OUTPUT_HTML_FILE = 'OUTPUT_HTML_FILE'

    MIN = 'MIN'
    MAX = 'MAX'
    COUNT = 'COUNT'
    UNIQUE = 'UNIQUE'
    EMPTY = 'EMPTY'
    FILLED = 'FILLED'
    MIN_LENGTH = 'MIN_LENGTH'
    MAX_LENGTH = 'MAX_LENGTH'
    MEAN_LENGTH = 'MEAN_LENGTH'
    CV = 'CV'
    SUM = 'SUM'
    MEAN = 'MEAN'
    STD_DEV = 'STD_DEV'
    RANGE = 'RANGE'
    MEDIAN = 'MEDIAN'
    MINORITY = 'MINORITY'
    MAJORITY = 'MAJORITY'
    FIRSTQUARTILE = 'FIRSTQUARTILE'
    THIRDQUARTILE = 'THIRDQUARTILE'
    IQR = 'IQR'

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'basic_statistics.png'))

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Basic statistics for fields')
        self.group, self.i18n_group = self.trAlgorithm('Vector table tools')
        self.tags = self.tr('stats,statistics,date,time,datetime,string,number,text,table,layer,maximum,minimum,mean,average,standard,deviation,'
                            'count,distinct,unique,variance,median,quartile,range,majority,minority')

        self.addParameter(ParameterTable(self.INPUT_LAYER,
                                         self.tr('Input table')))
        self.addParameter(ParameterTableField(self.FIELD_NAME,
                                              self.tr('Field to calculate statistics on'),
                                              self.INPUT_LAYER))

        self.addOutput(OutputHTML(self.OUTPUT_HTML_FILE,
                                  self.tr('Statistics')))

        self.addOutput(OutputNumber(self.COUNT, self.tr('Count')))
        self.addOutput(OutputNumber(self.UNIQUE, self.tr('Number of unique values')))
        self.addOutput(OutputNumber(self.EMPTY, self.tr('Number of empty (null) values')))
        self.addOutput(OutputNumber(self.FILLED, self.tr('Number of non-empty values')))
        self.addOutput(OutputNumber(self.MIN, self.tr('Minimum value')))
        self.addOutput(OutputNumber(self.MAX, self.tr('Maximum value')))
        self.addOutput(OutputNumber(self.MIN_LENGTH, self.tr('Minimum length')))
        self.addOutput(OutputNumber(self.MAX_LENGTH, self.tr('Maximum length')))
        self.addOutput(OutputNumber(self.MEAN_LENGTH, self.tr('Mean length')))
        self.addOutput(OutputNumber(self.CV, self.tr('Coefficient of Variation')))
        self.addOutput(OutputNumber(self.SUM, self.tr('Sum')))
        self.addOutput(OutputNumber(self.MEAN, self.tr('Mean value')))
        self.addOutput(OutputNumber(self.STD_DEV, self.tr('Standard deviation')))
        self.addOutput(OutputNumber(self.RANGE, self.tr('Range')))
        self.addOutput(OutputNumber(self.MEDIAN, self.tr('Median')))
        self.addOutput(OutputNumber(self.MINORITY, self.tr('Minority (rarest occurring value)')))
        self.addOutput(OutputNumber(self.MAJORITY, self.tr('Majority (most frequently occurring value)')))
        self.addOutput(OutputNumber(self.FIRSTQUARTILE, self.tr('First quartile')))
        self.addOutput(OutputNumber(self.THIRDQUARTILE, self.tr('Third quartile')))
        self.addOutput(OutputNumber(self.IQR, self.tr('Interquartile Range (IQR)')))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT_LAYER))
        field_name = self.getParameterValue(self.FIELD_NAME)
        field = layer.fields().at(layer.fields().lookupField(field_name))

        output_file = self.getOutputValue(self.OUTPUT_HTML_FILE)

        request = QgsFeatureRequest().setFlags(QgsFeatureRequest.NoGeometry).setSubsetOfAttributes([field_name], layer.fields())
        features = vector.features(layer, request)

        data = []
        data.append(self.tr('Analyzed layer: {}').format(layer.name()))
        data.append(self.tr('Analyzed field: {}').format(field_name))

        if field.isNumeric():
            data.extend(self.calcNumericStats(features, progress, field))
        elif field.type() in (QVariant.Date, QVariant.Time, QVariant.DateTime):
            data.extend(self.calcDateTimeStats(features, progress, field))
        else:
            data.extend(self.calcStringStats(features, progress, field))

        self.createHTML(output_file, data)

    def calcNumericStats(self, features, progress, field):
        count = len(features)
        total = 100.0 / float(count)
        stat = QgsStatisticalSummary()
        for current, ft in enumerate(features):
            stat.addVariant(ft[field.name()])
            progress.setPercentage(int(current * total))
        stat.finalize()

        cv = stat.stDev() / stat.mean() if stat.mean() != 0 else 0

        self.setOutputValue(self.COUNT, stat.count())
        self.setOutputValue(self.UNIQUE, stat.variety())
        self.setOutputValue(self.EMPTY, stat.countMissing())
        self.setOutputValue(self.FILLED, count - stat.countMissing())
        self.setOutputValue(self.MIN, stat.min())
        self.setOutputValue(self.MAX, stat.max())
        self.setOutputValue(self.RANGE, stat.range())
        self.setOutputValue(self.SUM, stat.sum())
        self.setOutputValue(self.MEAN, stat.mean())
        self.setOutputValue(self.MEDIAN, stat.median())
        self.setOutputValue(self.STD_DEV, stat.stDev())
        self.setOutputValue(self.CV, cv)
        self.setOutputValue(self.MINORITY, stat.minority())
        self.setOutputValue(self.MAJORITY, stat.majority())
        self.setOutputValue(self.FIRSTQUARTILE, stat.firstQuartile())
        self.setOutputValue(self.THIRDQUARTILE, stat.thirdQuartile())
        self.setOutputValue(self.IQR, stat.interQuartileRange())

        data = []
        data.append(self.tr('Count: {}').format(stat.count()))
        data.append(self.tr('Unique values: {}').format(stat.variety()))
        data.append(self.tr('NULL (missing) values: {}').format(stat.countMissing()))
        data.append(self.tr('Minimum value: {}').format(stat.min()))
        data.append(self.tr('Maximum value: {}').format(stat.max()))
        data.append(self.tr('Range: {}').format(stat.range()))
        data.append(self.tr('Sum: {}').format(stat.sum()))
        data.append(self.tr('Mean value: {}').format(stat.mean()))
        data.append(self.tr('Median value: {}').format(stat.median()))
        data.append(self.tr('Standard deviation: {}').format(stat.stDev()))
        data.append(self.tr('Coefficient of Variation: {}').format(cv))
        data.append(self.tr('Minority (rarest occurring value): {}').format(stat.minority()))
        data.append(self.tr('Majority (most frequently occurring value): {}').format(stat.majority()))
        data.append(self.tr('First quartile: {}').format(stat.firstQuartile()))
        data.append(self.tr('Third quartile: {}').format(stat.thirdQuartile()))
        data.append(self.tr('Interquartile Range (IQR): {}').format(stat.interQuartileRange()))
        return data

    def calcStringStats(self, features, progress, field):
        count = len(features)
        total = 100.0 / float(count)
        stat = QgsStringStatisticalSummary()
        for current, ft in enumerate(features):
            stat.addValue(ft[field.name()])
            progress.setPercentage(int(current * total))
        stat.finalize()

        self.setOutputValue(self.COUNT, stat.count())
        self.setOutputValue(self.UNIQUE, stat.countDistinct())
        self.setOutputValue(self.EMPTY, stat.countMissing())
        self.setOutputValue(self.FILLED, stat.count() - stat.countMissing())
        self.setOutputValue(self.MIN, stat.min())
        self.setOutputValue(self.MAX, stat.max())
        self.setOutputValue(self.MIN_LENGTH, stat.minLength())
        self.setOutputValue(self.MAX_LENGTH, stat.maxLength())
        self.setOutputValue(self.MEAN_LENGTH, stat.meanLength())

        data = []
        data.append(self.tr('Count: {}').format(count))
        data.append(self.tr('Unique values: {}').format(stat.countDistinct()))
        data.append(self.tr('NULL (missing) values: {}').format(stat.countMissing()))
        data.append(self.tr('Minimum value: {}').format(stat.min()))
        data.append(self.tr('Maximum value: {}').format(stat.max()))
        data.append(self.tr('Minimum length: {}').format(stat.minLength()))
        data.append(self.tr('Maximum length: {}').format(stat.maxLength()))
        data.append(self.tr('Mean length: {}').format(stat.meanLength()))

        return data

    def calcDateTimeStats(self, features, progress, field):
        count = len(features)
        total = 100.0 / float(count)
        stat = QgsDateTimeStatisticalSummary()
        for current, ft in enumerate(features):
            stat.addValue(ft[field.name()])
            progress.setPercentage(int(current * total))
        stat.finalize()

        self.setOutputValue(self.COUNT, stat.count())
        self.setOutputValue(self.UNIQUE, stat.countDistinct())
        self.setOutputValue(self.EMPTY, stat.countMissing())
        self.setOutputValue(self.FILLED, stat.count() - stat.countMissing())
        self.setOutputValue(self.MIN, stat.statistic(QgsDateTimeStatisticalSummary.Min))
        self.setOutputValue(self.MAX, stat.statistic(QgsDateTimeStatisticalSummary.Max))

        data = []
        data.append(self.tr('Count: {}').format(count))
        data.append(self.tr('Unique values: {}').format(stat.countDistinct()))
        data.append(self.tr('NULL (missing) values: {}').format(stat.countMissing()))
        data.append(self.tr('Minimum value: {}').format(field.displayString(stat.statistic(QgsDateTimeStatisticalSummary.Min))))
        data.append(self.tr('Maximum value: {}').format(field.displayString(stat.statistic(QgsDateTimeStatisticalSummary.Max))))

        return data

    def createHTML(self, outputFile, algData):
        with codecs.open(outputFile, 'w', encoding='utf-8') as f:
            f.write('<html><head>\n')
            f.write('<meta http-equiv="Content-Type" content="text/html; \
                    charset=utf-8" /></head><body>\n')
            for s in algData:
                f.write('<p>' + str(s) + '</p>\n')
            f.write('</body></html>\n')
