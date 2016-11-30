# -*- coding: utf-8 -*-

"""
***************************************************************************
    BasicStatisticsDateTime.py
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

from qgis.core import (QgsDateTimeStatisticalSummary,
                       QgsFeatureRequest)

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterTable
from processing.core.parameters import ParameterTableField
from processing.core.outputs import OutputHTML
from processing.core.outputs import OutputNumber
from processing.tools import dataobjects, vector


pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class BasicStatisticsDateTime(GeoAlgorithm):

    INPUT_LAYER = 'INPUT_LAYER'
    FIELD_NAME = 'FIELD_NAME'
    OUTPUT_HTML_FILE = 'OUTPUT_HTML_FILE'

    MIN = 'MIN'
    MAX = 'MAX'
    COUNT = 'COUNT'
    UNIQUE = 'UNIQUE'
    EMPTY = 'EMPTY'

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'basic_statistics.png'))

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Basic statistics for date/time fields')
        self.group, self.i18n_group = self.trAlgorithm('Vector table tools')
        self.tags = self.tr('stats,statistics,date,time,datetime,table,layer')

        self.addParameter(ParameterTable(self.INPUT_LAYER,
                                         self.tr('Input vector layer')))
        self.addParameter(ParameterTableField(self.FIELD_NAME,
                                              self.tr('Field to calculate statistics on'),
                                              self.INPUT_LAYER, ParameterTableField.DATA_TYPE_DATETIME))

        self.addOutput(OutputHTML(self.OUTPUT_HTML_FILE,
                                  self.tr('Statistics')))

        self.addOutput(OutputNumber(self.COUNT, self.tr('Count')))
        self.addOutput(OutputNumber(self.UNIQUE, self.tr('Number of unique values')))
        self.addOutput(OutputNumber(self.EMPTY, self.tr('Number of empty values')))
        self.addOutput(OutputNumber(self.MIN, self.tr('Minimum date/time value')))
        self.addOutput(OutputNumber(self.MAX, self.tr('Maximum date/time value')))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT_LAYER))
        fieldName = self.getParameterValue(self.FIELD_NAME)
        field = layer.fields().at(layer.fields().lookupField(fieldName))

        outputFile = self.getOutputValue(self.OUTPUT_HTML_FILE)

        request = QgsFeatureRequest().setFlags(QgsFeatureRequest.NoGeometry).setSubsetOfAttributes([fieldName], layer.fields())
        stat = QgsDateTimeStatisticalSummary()
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
        data.append(self.tr('Count: {}').format(stat.count()))
        data.append(self.tr('Unique values: {}').format(stat.countDistinct()))
        data.append(self.tr('NULL (missing) values: {}').format(stat.countMissing()))
        data.append(self.tr('Minimum value: {}').format(field.displayString(stat.statistic(QgsDateTimeStatisticalSummary.Min))))
        data.append(self.tr('Maximum value: {}').format(field.displayString(stat.statistic(QgsDateTimeStatisticalSummary.Max))))

        self.createHTML(outputFile, data)

        self.setOutputValue(self.COUNT, stat.count())
        self.setOutputValue(self.UNIQUE, stat.countDistinct())
        self.setOutputValue(self.MIN, stat.statistic(QgsDateTimeStatisticalSummary.Min))
        self.setOutputValue(self.MAX, stat.statistic(QgsDateTimeStatisticalSummary.Max))
        self.setOutputValue(self.EMPTY, stat.countMissing())

    def createHTML(self, outputFile, algData):
        with codecs.open(outputFile, 'w', encoding='utf-8') as f:
            f.write('<html><head>\n')
            f.write('<meta http-equiv="Content-Type" content="text/html; \
                    charset=utf-8" /></head><body>\n')
            for s in algData:
                f.write('<p>' + str(s) + '</p>\n')
            f.write('</body></html>\n')
