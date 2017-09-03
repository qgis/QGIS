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
from builtins import str

__author__ = 'Victor Olaya'
__date__ = 'September 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import (QgsProcessingParameterFeatureSource,
                       QgsStatisticalSummary,
                       QgsFeatureRequest,
                       QgsProcessingParameterField,
                       QgsProcessingParameterFeatureSink,
                       QgsFields,
                       QgsField,
                       QgsWkbTypes,
                       QgsCoordinateReferenceSystem,
                       QgsFeature,
                       QgsFeatureSink)
from qgis.PyQt.QtCore import QVariant
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class StatisticsByCategories(QgisAlgorithm):

    INPUT = 'INPUT'
    VALUES_FIELD_NAME = 'VALUES_FIELD_NAME'
    CATEGORIES_FIELD_NAME = 'CATEGORIES_FIELD_NAME'
    OUTPUT = 'OUTPUT'

    def group(self):
        return self.tr('Vector analysis')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input vector layer')))
        self.addParameter(QgsProcessingParameterField(self.VALUES_FIELD_NAME,
                                                      self.tr('Field to calculate statistics on'),
                                                      parentLayerParameterName=self.INPUT, type=QgsProcessingParameterField.Numeric))
        self.addParameter(QgsProcessingParameterField(self.CATEGORIES_FIELD_NAME,
                                                      self.tr('Field with categories'),
                                                      parentLayerParameterName=self.INPUT, type=QgsProcessingParameterField.Any))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Statistics by category')))

    def name(self):
        return 'statisticsbycategories'

    def displayName(self):
        return self.tr('Statistics by categories')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        value_field_name = self.parameterAsString(parameters, self.VALUES_FIELD_NAME, context)
        category_field_name = self.parameterAsString(parameters, self.CATEGORIES_FIELD_NAME, context)

        value_field_index = source.fields().lookupField(value_field_name)
        category_field_index = source.fields().lookupField(category_field_name)

        features = source.getFeatures(QgsFeatureRequest().setFlags(QgsFeatureRequest.NoGeometry))
        total = 100.0 / source.featureCount() if source.featureCount() else 0
        values = {}
        for current, feat in enumerate(features):
            if feedback.isCanceled():
                break

            feedback.setProgress(int(current * total))
            attrs = feat.attributes()
            try:
                value = float(attrs[value_field_index])
                cat = attrs[category_field_index]
                if cat not in values:
                    values[cat] = []
                values[cat].append(value)
            except:
                pass

        fields = QgsFields()
        fields.append(source.fields().at(category_field_index))
        fields.append(QgsField('min', QVariant.Double))
        fields.append(QgsField('max', QVariant.Double))
        fields.append(QgsField('mean', QVariant.Double))
        fields.append(QgsField('stddev', QVariant.Double))
        fields.append(QgsField('sum', QVariant.Double))
        fields.append(QgsField('count', QVariant.Int))

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, QgsWkbTypes.NoGeometry, QgsCoordinateReferenceSystem())

        stat = QgsStatisticalSummary(QgsStatisticalSummary.Min | QgsStatisticalSummary.Max |
                                     QgsStatisticalSummary.Mean | QgsStatisticalSummary.StDevSample |
                                     QgsStatisticalSummary.Sum | QgsStatisticalSummary.Count)

        for (cat, v) in list(values.items()):
            stat.calculate(v)
            f = QgsFeature()
            f.setAttributes([cat, stat.min(), stat.max(), stat.mean(), stat.sampleStDev(), stat.sum(), stat.count()])
            sink.addFeature(f, QgsFeatureSink.FastInsert)

        return {self.OUTPUT: dest_id}
