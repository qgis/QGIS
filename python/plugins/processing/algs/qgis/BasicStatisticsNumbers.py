# -*- coding: utf-8 -*-

"""
***************************************************************************
    BasicStatisticsNumbers.py
    ---------------------
    Date                 : September 2012
    Copyright            : (C) 2012 by Victor Olaya
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
__date__ = 'September 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import codecs

from qgis.PyQt.QtGui import QIcon

from qgis.core import QgsStatisticalSummary

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterTableField
from processing.core.outputs import OutputHTML
from processing.core.outputs import OutputNumber
from processing.tools import dataobjects, vector


pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class BasicStatisticsNumbers(GeoAlgorithm):

    INPUT_LAYER = 'INPUT_LAYER'
    FIELD_NAME = 'FIELD_NAME'
    OUTPUT_HTML_FILE = 'OUTPUT_HTML_FILE'

    CV = 'CV'
    MIN = 'MIN'
    MAX = 'MAX'
    SUM = 'SUM'
    MEAN = 'MEAN'
    COUNT = 'COUNT'
    STD_DEV = 'STD_DEV'
    RANGE = 'RANGE'
    MEDIAN = 'MEDIAN'
    UNIQUE = 'UNIQUE'
    MINORITY = 'MINORITY'
    MAJORITY = 'MAJORITY'
    FIRSTQUARTILE = 'FIRSTQUARTILE'
    THIRDQUARTILE = 'THIRDQUARTILE'
    NULLVALUES = 'NULLVALUES'
    IQR = 'IQR'

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'basic_statistics.png'))

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Basic statistics for numeric fields')
        self.group, self.i18n_group = self.trAlgorithm('Vector table tools')

        self.addParameter(ParameterVector(self.INPUT_LAYER,
                                          self.tr('Input vector layer'), ParameterVector.VECTOR_TYPE_ANY, False))
        self.addParameter(ParameterTableField(self.FIELD_NAME,
                                              self.tr('Field to calculate statistics on'),
                                              self.INPUT_LAYER, ParameterTableField.DATA_TYPE_NUMBER))

        self.addOutput(OutputHTML(self.OUTPUT_HTML_FILE,
                                  self.tr('Statistics')))

        self.addOutput(OutputNumber(self.CV, self.tr('Coefficient of Variation')))
        self.addOutput(OutputNumber(self.MIN, self.tr('Minimum value')))
        self.addOutput(OutputNumber(self.MAX, self.tr('Maximum value')))
        self.addOutput(OutputNumber(self.SUM, self.tr('Sum')))
        self.addOutput(OutputNumber(self.MEAN, self.tr('Mean value')))
        self.addOutput(OutputNumber(self.STD_DEV, self.tr('Standard deviation')))
        self.addOutput(OutputNumber(self.COUNT, self.tr('Count')))
        self.addOutput(OutputNumber(self.RANGE, self.tr('Range')))
        self.addOutput(OutputNumber(self.MEDIAN, self.tr('Median')))
        self.addOutput(OutputNumber(self.UNIQUE, self.tr('Number of unique values')))
        self.addOutput(OutputNumber(self.MINORITY, self.tr('Minority (rarest occurring value)')))
        self.addOutput(OutputNumber(self.MAJORITY, self.tr('Majority (most frequently occurring value)')))
        self.addOutput(OutputNumber(self.FIRSTQUARTILE, self.tr('First quartile')))
        self.addOutput(OutputNumber(self.THIRDQUARTILE, self.tr('Third quartile')))
        self.addOutput(OutputNumber(self.NULLVALUES, self.tr('NULL (missed) values')))
        self.addOutput(OutputNumber(self.IQR, self.tr('Interquartile Range (IQR)')))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT_LAYER))
        fieldName = self.getParameterValue(self.FIELD_NAME)

        outputFile = self.getOutputValue(self.OUTPUT_HTML_FILE)

        cvValue = 0
        minValue = 0
        maxValue = 0
        sumValue = 0
        meanValue = 0
        medianValue = 0
        stdDevValue = 0
        minority = 0
        majority = 0
        firstQuartile = 0
        thirdQuartile = 0
        nullValues = 0
        iqr = 0

        values = []

        features = vector.features(layer)
        count = len(features)
        total = 100.0 / float(count)
        for current, ft in enumerate(features):
            value = ft[fieldName]
            if value or value == 0:
                values.append(float(value))
            else:
                nullValues += 1

            progress.setPercentage(int(current * total))

        stat = QgsStatisticalSummary()
        stat.calculate(values)

        count = stat.count()
        uniqueValue = stat.variety()
        minValue = stat.min()
        maxValue = stat.max()
        rValue = stat.range()
        sumValue = stat.sum()
        meanValue = stat.mean()
        medianValue = stat.median()
        stdDevValue = stat.stDev()
        if meanValue != 0.00:
            cvValue = stdDevValue / meanValue
        minority = stat.minority()
        majority = stat.majority()
        firstQuartile = stat.firstQuartile()
        thirdQuartile = stat.thirdQuartile()
        iqr = stat.interQuartileRange()

        data = []
        data.append(self.tr('Analyzed layer: {}').format(layer.name()))
        data.append(self.tr('Analyzed field: {}').format(fieldName))
        data.append(self.tr('Count: {}').format(count))
        data.append(self.tr('Unique values: {}').format(uniqueValue))
        data.append(self.tr('Minimum value: {}').format(minValue))
        data.append(self.tr('Maximum value: {}').format(maxValue))
        data.append(self.tr('Range: {}').format(rValue))
        data.append(self.tr('Sum: {}').format(sumValue))
        data.append(self.tr('Mean value: {}').format(meanValue))
        data.append(self.tr('Median value: {}').format(medianValue))
        data.append(self.tr('Standard deviation: {}').format(stdDevValue))
        data.append(self.tr('Coefficient of Variation: {}').format(cvValue))
        data.append(self.tr('Minority (rarest occurring value): {}').format(minority))
        data.append(self.tr('Majority (most frequently occurring value): {}').format(majority))
        data.append(self.tr('First quartile: {}').format(firstQuartile))
        data.append(self.tr('Third quartile: {}').format(thirdQuartile))
        data.append(self.tr('NULL (missing) values: {}').format(nullValues))
        data.append(self.tr('Interquartile Range (IQR): {}').format(iqr))

        self.createHTML(outputFile, data)

        self.setOutputValue(self.COUNT, count)
        self.setOutputValue(self.UNIQUE, uniqueValue)
        self.setOutputValue(self.MIN, minValue)
        self.setOutputValue(self.MAX, maxValue)
        self.setOutputValue(self.RANGE, rValue)
        self.setOutputValue(self.SUM, sumValue)
        self.setOutputValue(self.MEAN, meanValue)
        self.setOutputValue(self.MEDIAN, medianValue)
        self.setOutputValue(self.STD_DEV, stdDevValue)
        self.setOutputValue(self.MINORITY, minority)
        self.setOutputValue(self.MAJORITY, majority)
        self.setOutputValue(self.FIRSTQUARTILE, firstQuartile)
        self.setOutputValue(self.THIRDQUARTILE, thirdQuartile)
        self.setOutputValue(self.NULLVALUES, nullValues)
        self.setOutputValue(self.IQR, iqr)

    def createHTML(self, outputFile, algData):
        f = codecs.open(outputFile, 'w', encoding='utf-8')
        f.write('<html><head>\n')
        f.write('<meta http-equiv="Content-Type" content="text/html; \
                charset=utf-8" /></head><body>\n')
        for s in algData:
            f.write('<p>' + unicode(s) + '</p>\n')
        f.write('</body></html>\n')
        f.close()
