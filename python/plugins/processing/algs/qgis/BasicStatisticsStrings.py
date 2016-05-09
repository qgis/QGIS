# -*- coding: utf-8 -*-

"""
***************************************************************************
    BasicStatisticsStrings.py
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

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterTableField
from processing.core.outputs import OutputHTML
from processing.core.outputs import OutputNumber
from processing.tools import dataobjects, vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class BasicStatisticsStrings(GeoAlgorithm):

    INPUT_LAYER = 'INPUT_LAYER'
    FIELD_NAME = 'FIELD_NAME'
    OUTPUT_HTML_FILE = 'OUTPUT_HTML_FILE'

    MIN_LEN = 'MIN_LEN'
    MAX_LEN = 'MAX_LEN'
    MEAN_LEN = 'MEAN_LEN'
    COUNT = 'COUNT'
    EMPTY = 'EMPTY'
    FILLED = 'FILLED'
    UNIQUE = 'UNIQUE'

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'basic_statistics.png'))

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Basic statistics for text fields')
        self.group, self.i18n_group = self.trAlgorithm('Vector table tools')

        self.addParameter(ParameterVector(self.INPUT_LAYER,
                                          self.tr('Input vector layer'),
                                          ParameterVector.VECTOR_TYPE_ANY, False))
        self.addParameter(ParameterTableField(self.FIELD_NAME,
                                              self.tr('Field to calculate statistics on'),
                                              self.INPUT_LAYER, ParameterTableField.DATA_TYPE_STRING))

        self.addOutput(OutputHTML(self.OUTPUT_HTML_FILE,
                                  self.tr('Statistics for text')))

        self.addOutput(OutputNumber(self.MIN_LEN, self.tr('Minimum length')))
        self.addOutput(OutputNumber(self.MAX_LEN, self.tr('Maximum length')))
        self.addOutput(OutputNumber(self.MEAN_LEN, self.tr('Mean length')))
        self.addOutput(OutputNumber(self.COUNT, self.tr('Count')))
        self.addOutput(OutputNumber(self.EMPTY, self.tr('Number of empty values')))
        self.addOutput(OutputNumber(self.FILLED, self.tr('Number of non-empty values')))
        self.addOutput(OutputNumber(self.UNIQUE, self.tr('Number of unique values')))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT_LAYER))
        fieldName = self.getParameterValue(self.FIELD_NAME)

        outputFile = self.getOutputValue(self.OUTPUT_HTML_FILE)

        index = layer.fieldNameIndex(fieldName)

        sumValue = 0
        minValue = 0
        maxValue = 0
        meanValue = 0
        nullValues = 0
        filledValues = 0

        isFirst = True
        values = []

        features = vector.features(layer)
        count = len(features)
        total = 100.0 / count
        for current, ft in enumerate(features):
            value = ft[fieldName]
            if value:
                length = float(len(value))
                filledValues += 1
            else:
                nullValues += 1
                progress.setPercentage(int(current * total))
                continue

            if isFirst:
                minValue = length
                maxValue = length
                isFirst = False
            else:
                if length < minValue:
                    minValue = length
                if length > maxValue:
                    maxValue = length

            values.append(length)
            sumValue += length

            progress.setPercentage(int(current * total))

        n = float(len(values))
        if n > 0:
            meanValue = sumValue / n

        uniqueValues = vector.getUniqueValuesCount(layer, index)

        data = []
        data.append(self.tr('Analyzed layer: {}').format(layer.name()))
        data.append(self.tr('Analyzed field: {}').format(fieldName))
        data.append(self.tr('Minimum length: {}').format(minValue))
        data.append(self.tr('Maximum length: {}').format(maxValue))
        data.append(self.tr('Mean length: {}').format(meanValue))
        data.append(self.tr('Filled values: {}').format(filledValues))
        data.append(self.tr('NULL (missing) values: {}').format(nullValues))
        data.append(self.tr('Count: {}').format(count))
        data.append(self.tr('Unique: {}').format(uniqueValues))

        self.createHTML(outputFile, data)

        self.setOutputValue(self.MIN_LEN, minValue)
        self.setOutputValue(self.MAX_LEN, maxValue)
        self.setOutputValue(self.MEAN_LEN, meanValue)
        self.setOutputValue(self.FILLED, filledValues)
        self.setOutputValue(self.EMPTY, nullValues)
        self.setOutputValue(self.COUNT, count)
        self.setOutputValue(self.UNIQUE, uniqueValues)

    def createHTML(self, outputFile, algData):
        f = codecs.open(outputFile, 'w', encoding='utf-8')
        f.write('<html><head>\n')
        f.write('<meta http-equiv="Content-Type" content="text/html; \
                charset=utf-8" /></head><body>\n')
        for s in algData:
            f.write('<p>' + unicode(s) + '</p>\n')
        f.write('</body></html>')
        f.close()
