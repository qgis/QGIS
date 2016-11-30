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
from builtins import str

__author__ = 'Victor Olaya'
__date__ = 'September 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import codecs

from qgis.PyQt.QtGui import QIcon

from qgis.core import (QgsStringStatisticalSummary,
                       QgsFeatureRequest)

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterTable
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
    MIN_VALUE = 'MIN_VALUE'
    MAX_VALUE = 'MAX_VALUE'

    def __init__(self):
        GeoAlgorithm.__init__(self)
        # this algorithm is deprecated - use BasicStatistics instead
        self.showInToolbox = False

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'basic_statistics.png'))

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Basic statistics for text fields')
        self.group, self.i18n_group = self.trAlgorithm('Vector table tools')
        self.tags = self.tr('stats,statistics,string,table,layer')

        self.addParameter(ParameterTable(self.INPUT_LAYER,
                                         self.tr('Input vector layer')))
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
        self.addOutput(OutputNumber(self.MIN_VALUE, self.tr('Minimum string value')))
        self.addOutput(OutputNumber(self.MAX_VALUE, self.tr('Maximum string value')))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT_LAYER))
        fieldName = self.getParameterValue(self.FIELD_NAME)

        outputFile = self.getOutputValue(self.OUTPUT_HTML_FILE)

        request = QgsFeatureRequest().setFlags(QgsFeatureRequest.NoGeometry).setSubsetOfAttributes([fieldName],
                                                                                                   layer.fields())
        stat = QgsStringStatisticalSummary()
        features = vector.features(layer, request)
        count = len(features)
        total = 100.0 / float(count)
        for current, ft in enumerate(features):
            stat.addValue(ft[fieldName])
            progress.setPercentage(int(current * total))

        stat.finalize()

        data = []
        data.append(self.tr('Analyzed layer: {}').format(layer.name()))
        data.append(self.tr('Analyzed field: {}').format(fieldName))
        data.append(self.tr('Minimum length: {}').format(stat.minLength()))
        data.append(self.tr('Maximum length: {}').format(stat.maxLength()))
        data.append(self.tr('Mean length: {}').format(stat.meanLength()))
        data.append(self.tr('Filled values: {}').format(stat.count() - stat.countMissing()))
        data.append(self.tr('NULL (missing) values: {}').format(stat.countMissing()))
        data.append(self.tr('Count: {}').format(stat.count()))
        data.append(self.tr('Unique: {}').format(stat.countDistinct()))
        data.append(self.tr('Minimum string value: {}').format(stat.min()))
        data.append(self.tr('Maximum string value: {}').format(stat.max()))

        self.createHTML(outputFile, data)

        self.setOutputValue(self.MIN_LEN, stat.minLength())
        self.setOutputValue(self.MAX_LEN, stat.maxLength())
        self.setOutputValue(self.MEAN_LEN, stat.meanLength())
        self.setOutputValue(self.FILLED, stat.count() - stat.countMissing())
        self.setOutputValue(self.EMPTY, stat.countMissing())
        self.setOutputValue(self.COUNT, stat.count())
        self.setOutputValue(self.UNIQUE, stat.countDistinct())
        self.setOutputValue(self.MIN_VALUE, stat.min())
        self.setOutputValue(self.MAX_VALUE, stat.max())

    def createHTML(self, outputFile, algData):
        with codecs.open(outputFile, 'w', encoding='utf-8') as f:
            f.write('<html><head>\n')
            f.write('<meta http-equiv="Content-Type" content="text/html; charset=utf-8" /></head><body>\n')
            for s in algData:
                f.write('<p>' + str(s) + '</p>\n')
            f.write('</body></html>\n')
