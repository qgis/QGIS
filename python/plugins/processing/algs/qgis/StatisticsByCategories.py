# -*- coding: utf-8 -*-

"""
***************************************************************************
    StatisticsByCategories.py
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

from qgis.core import QgsStatisticalSummary
from processing.core.outputs import OutputTable
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.tools import dataobjects, vector
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterTableField


class StatisticsByCategories(GeoAlgorithm):

    INPUT_LAYER = 'INPUT_LAYER'
    VALUES_FIELD_NAME = 'VALUES_FIELD_NAME'
    CATEGORIES_FIELD_NAME = 'CATEGORIES_FIELD_NAME'
    OUTPUT = 'OUTPUT'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Statistics by categories')
        self.group, self.i18n_group = self.trAlgorithm('Vector table tools')

        self.addParameter(ParameterVector(self.INPUT_LAYER,
                                          self.tr('Input vector layer'), [ParameterVector.VECTOR_TYPE_ANY], False))
        self.addParameter(ParameterTableField(self.VALUES_FIELD_NAME,
                                              self.tr('Field to calculate statistics on'),
                                              self.INPUT_LAYER, ParameterTableField.DATA_TYPE_NUMBER))
        self.addParameter(ParameterTableField(self.CATEGORIES_FIELD_NAME,
                                              self.tr('Field with categories'),
                                              self.INPUT_LAYER, ParameterTableField.DATA_TYPE_ANY))

        self.addOutput(OutputTable(self.OUTPUT, self.tr('Statistics by category')))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(self.getParameterValue(self.INPUT_LAYER))
        valuesFieldName = self.getParameterValue(self.VALUES_FIELD_NAME)
        categoriesFieldName = self.getParameterValue(self.CATEGORIES_FIELD_NAME)

        output = self.getOutputFromName(self.OUTPUT)
        valuesField = layer.fieldNameIndex(valuesFieldName)
        categoriesField = layer.fieldNameIndex(categoriesFieldName)

        features = vector.features(layer)
        total = 100.0 / len(features)
        values = {}
        for current, feat in enumerate(features):
            progress.setPercentage(int(current * total))
            attrs = feat.attributes()
            try:
                value = float(attrs[valuesField])
                cat = unicode(attrs[categoriesField])
                if cat not in values:
                    values[cat] = []
                values[cat].append(value)
            except:
                pass

        fields = ['category', 'min', 'max', 'mean', 'stddev', 'sum', 'count']
        writer = output.getTableWriter(fields)
        stat = QgsStatisticalSummary(QgsStatisticalSummary.Min | QgsStatisticalSummary.Max |
                                     QgsStatisticalSummary.Mean | QgsStatisticalSummary.StDevSample |
                                     QgsStatisticalSummary.Sum | QgsStatisticalSummary.Count)

        for (cat, v) in values.items():
            stat.calculate(v)
            record = [cat, stat.min(), stat.max(), stat.mean(), stat.sampleStDev(), stat.sum(), stat.count()]
            writer.addRecord(record)
