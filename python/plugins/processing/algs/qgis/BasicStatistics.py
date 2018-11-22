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

from qgis.core import (QgsApplication,
                       QgsStatisticalSummary,
                       QgsStringStatisticalSummary,
                       QgsDateTimeStatisticalSummary,
                       QgsFeatureRequest,
                       QgsProcessing,
                       QgsProcessingException,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterField,
                       QgsProcessingParameterFileDestination,
                       QgsProcessingOutputNumber,
                       QgsProcessingFeatureSource)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class BasicStatisticsForField(QgisAlgorithm):
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

    def icon(self):
        return QgsApplication.getThemeIcon("/algorithms/mAlgorithmBasicStatistics.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("/algorithms/mAlgorithmBasicStatistics.svg")

    def tags(self):
        return self.tr(
            'stats,statistics,date,time,datetime,string,number,text,table,layer,sum,maximum,minimum,mean,average,standard,deviation,'
            'count,distinct,unique,variance,median,quartile,range,majority,minority,summary').split(',')

    def group(self):
        return self.tr('Vector analysis')

    def groupId(self):
        return 'vectoranalysis'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT_LAYER,
                                                              self.tr('Input layer'),
                                                              types=[QgsProcessing.TypeVector]))

        self.addParameter(QgsProcessingParameterField(self.FIELD_NAME,
                                                      self.tr('Field to calculate statistics on'),
                                                      None, self.INPUT_LAYER, QgsProcessingParameterField.Any))

        self.addParameter(QgsProcessingParameterFileDestination(self.OUTPUT_HTML_FILE, self.tr('Statistics'),
                                                                self.tr('HTML files (*.html)'), None, True))

        self.addOutput(QgsProcessingOutputNumber(self.COUNT, self.tr('Count')))
        self.addOutput(QgsProcessingOutputNumber(self.UNIQUE, self.tr('Number of unique values')))
        self.addOutput(QgsProcessingOutputNumber(self.EMPTY, self.tr('Number of empty (null) values')))
        self.addOutput(QgsProcessingOutputNumber(self.FILLED, self.tr('Number of non-empty values')))
        self.addOutput(QgsProcessingOutputNumber(self.MIN, self.tr('Minimum value')))
        self.addOutput(QgsProcessingOutputNumber(self.MAX, self.tr('Maximum value')))
        self.addOutput(QgsProcessingOutputNumber(self.MIN_LENGTH, self.tr('Minimum length')))
        self.addOutput(QgsProcessingOutputNumber(self.MAX_LENGTH, self.tr('Maximum length')))
        self.addOutput(QgsProcessingOutputNumber(self.MEAN_LENGTH, self.tr('Mean length')))
        self.addOutput(QgsProcessingOutputNumber(self.CV, self.tr('Coefficient of Variation')))
        self.addOutput(QgsProcessingOutputNumber(self.SUM, self.tr('Sum')))
        self.addOutput(QgsProcessingOutputNumber(self.MEAN, self.tr('Mean value')))
        self.addOutput(QgsProcessingOutputNumber(self.STD_DEV, self.tr('Standard deviation')))
        self.addOutput(QgsProcessingOutputNumber(self.RANGE, self.tr('Range')))
        self.addOutput(QgsProcessingOutputNumber(self.MEDIAN, self.tr('Median')))
        self.addOutput(QgsProcessingOutputNumber(self.MINORITY, self.tr('Minority (rarest occurring value)')))
        self.addOutput(QgsProcessingOutputNumber(self.MAJORITY, self.tr('Majority (most frequently occurring value)')))
        self.addOutput(QgsProcessingOutputNumber(self.FIRSTQUARTILE, self.tr('First quartile')))
        self.addOutput(QgsProcessingOutputNumber(self.THIRDQUARTILE, self.tr('Third quartile')))
        self.addOutput(QgsProcessingOutputNumber(self.IQR, self.tr('Interquartile Range (IQR)')))

    def name(self):
        return 'basicstatisticsforfields'

    def displayName(self):
        return self.tr('Basic statistics for fields')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT_LAYER, context)
        if source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.INPUT_LAYER))

        field_name = self.parameterAsString(parameters, self.FIELD_NAME, context)
        field = source.fields().at(source.fields().lookupField(field_name))

        output_file = self.parameterAsFileOutput(parameters, self.OUTPUT_HTML_FILE, context)

        request = QgsFeatureRequest().setFlags(QgsFeatureRequest.NoGeometry).setSubsetOfAttributes([field_name],
                                                                                                   source.fields())
        features = source.getFeatures(request, QgsProcessingFeatureSource.FlagSkipGeometryValidityChecks)
        count = source.featureCount()

        data = []
        data.append(self.tr('Analyzed field: {}').format(field_name))
        results = {}

        if field.isNumeric():
            d, results = self.calcNumericStats(features, feedback, field, count)
            data.extend(d)
        elif field.type() in (QVariant.Date, QVariant.Time, QVariant.DateTime):
            d, results = self.calcDateTimeStats(features, feedback, field, count)
            data.extend(d)
        else:
            d, results = self.calcStringStats(features, feedback, field, count)
            data.extend(d)

        if output_file:
            self.createHTML(output_file, data)
            results[self.OUTPUT_HTML_FILE] = output_file

        return results

    def calcNumericStats(self, features, feedback, field, count):
        total = 100.0 / count if count else 0
        stat = QgsStatisticalSummary()
        for current, ft in enumerate(features):
            if feedback.isCanceled():
                break
            stat.addVariant(ft[field.name()])
            feedback.setProgress(int(current * total))
        stat.finalize()

        cv = stat.stDev() / stat.mean() if stat.mean() != 0 else 0

        results = {self.COUNT: stat.count(),
                   self.UNIQUE: stat.variety(),
                   self.EMPTY: stat.countMissing(),
                   self.FILLED: count - stat.countMissing(),
                   self.MIN: stat.min(),
                   self.MAX: stat.max(),
                   self.RANGE: stat.range(),
                   self.SUM: stat.sum(),
                   self.MEAN: stat.mean(),
                   self.MEDIAN: stat.median(),
                   self.STD_DEV: stat.stDev(),
                   self.CV: cv,
                   self.MINORITY: stat.minority(),
                   self.MAJORITY: stat.majority(),
                   self.FIRSTQUARTILE: stat.firstQuartile(),
                   self.THIRDQUARTILE: stat.thirdQuartile(),
                   self.IQR: stat.interQuartileRange()}

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
        return data, results

    def calcStringStats(self, features, feedback, field, count):
        total = 100.0 / count if count else 1
        stat = QgsStringStatisticalSummary()
        for current, ft in enumerate(features):
            if feedback.isCanceled():
                break
            stat.addValue(ft[field.name()])
            feedback.setProgress(int(current * total))
        stat.finalize()

        results = {self.COUNT: stat.count(),
                   self.UNIQUE: stat.countDistinct(),
                   self.EMPTY: stat.countMissing(),
                   self.FILLED: stat.count() - stat.countMissing(),
                   self.MIN: stat.min(),
                   self.MAX: stat.max(),
                   self.MIN_LENGTH: stat.minLength(),
                   self.MAX_LENGTH: stat.maxLength(),
                   self.MEAN_LENGTH: stat.meanLength()}

        data = []
        data.append(self.tr('Count: {}').format(count))
        data.append(self.tr('Unique values: {}').format(stat.countDistinct()))
        data.append(self.tr('NULL (missing) values: {}').format(stat.countMissing()))
        data.append(self.tr('Minimum value: {}').format(stat.min()))
        data.append(self.tr('Maximum value: {}').format(stat.max()))
        data.append(self.tr('Minimum length: {}').format(stat.minLength()))
        data.append(self.tr('Maximum length: {}').format(stat.maxLength()))
        data.append(self.tr('Mean length: {}').format(stat.meanLength()))

        return data, results

    def calcDateTimeStats(self, features, feedback, field, count):
        total = 100.0 / count if count else 1
        stat = QgsDateTimeStatisticalSummary()
        for current, ft in enumerate(features):
            if feedback.isCanceled():
                break
            stat.addValue(ft[field.name()])
            feedback.setProgress(int(current * total))
        stat.finalize()

        results = {self.COUNT: stat.count(),
                   self.UNIQUE: stat.countDistinct(),
                   self.EMPTY: stat.countMissing(),
                   self.FILLED: stat.count() - stat.countMissing(),
                   self.MIN: stat.statistic(QgsDateTimeStatisticalSummary.Min),
                   self.MAX: stat.statistic(QgsDateTimeStatisticalSummary.Max)}

        data = []
        data.append(self.tr('Count: {}').format(count))
        data.append(self.tr('Unique values: {}').format(stat.countDistinct()))
        data.append(self.tr('NULL (missing) values: {}').format(stat.countMissing()))
        data.append(
            self.tr('Minimum value: {}').format(field.displayString(stat.statistic(QgsDateTimeStatisticalSummary.Min))))
        data.append(
            self.tr('Maximum value: {}').format(field.displayString(stat.statistic(QgsDateTimeStatisticalSummary.Max))))

        return data, results

    def createHTML(self, outputFile, algData):
        with codecs.open(outputFile, 'w', encoding='utf-8') as f:
            f.write('<html><head>\n')
            f.write('<meta http-equiv="Content-Type" content="text/html; \
                    charset=utf-8" /></head><body>\n')
            for s in algData:
                f.write('<p>' + str(s) + '</p>\n')
            f.write('</body></html>\n')
