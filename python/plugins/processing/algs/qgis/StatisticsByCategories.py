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

from qgis.core import (QgsProcessingParameterFeatureSource,
                       QgsStatisticalSummary,
                       QgsDateTimeStatisticalSummary,
                       QgsStringStatisticalSummary,
                       QgsFeatureRequest,
                       QgsApplication,
                       QgsProcessingException,
                       QgsProcessingParameterField,
                       QgsProcessingParameterFeatureSink,
                       QgsFields,
                       QgsField,
                       QgsWkbTypes,
                       QgsCoordinateReferenceSystem,
                       QgsFeature,
                       QgsFeatureSink,
                       QgsProcessing,
                       QgsProcessingFeatureSource,
                       NULL)
from qgis.PyQt.QtCore import QMetaType
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm

from collections import defaultdict


class StatisticsByCategories(QgisAlgorithm):
    INPUT = 'INPUT'
    VALUES_FIELD_NAME = 'VALUES_FIELD_NAME'
    CATEGORIES_FIELD_NAME = 'CATEGORIES_FIELD_NAME'
    OUTPUT = 'OUTPUT'

    def group(self):
        return self.tr('Vector analysis')

    def groupId(self):
        return 'vectoranalysis'

    def tags(self):
        return self.tr('groups,stats,statistics,table,layer,sum,maximum,minimum,mean,average,standard,deviation,'
                       'count,distinct,unique,variance,median,quartile,range,majority,minority,histogram,distinct,summary').split(',')

    def icon(self):
        return QgsApplication.getThemeIcon("/algorithms/mAlgorithmBasicStatistics.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("/algorithms/mAlgorithmBasicStatistics.svg")

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input vector layer'),
                                                              types=[QgsProcessing.SourceType.TypeVector]))
        self.addParameter(QgsProcessingParameterField(self.VALUES_FIELD_NAME,
                                                      self.tr(
                                                          'Field to calculate statistics on (if empty, only count is calculated)'),
                                                      parentLayerParameterName=self.INPUT, optional=True))
        self.addParameter(QgsProcessingParameterField(self.CATEGORIES_FIELD_NAME,
                                                      self.tr('Field(s) with categories'),
                                                      parentLayerParameterName=self.INPUT,
                                                      type=QgsProcessingParameterField.DataType.Any, allowMultiple=True))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Statistics by category')))

    def name(self):
        return 'statisticsbycategories'

    def displayName(self):
        return self.tr('Statistics by categories')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        if source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.INPUT))

        value_field_name = self.parameterAsString(parameters, self.VALUES_FIELD_NAME, context)
        category_field_names = self.parameterAsFields(parameters, self.CATEGORIES_FIELD_NAME, context)

        value_field_index = source.fields().lookupField(value_field_name)
        if value_field_index >= 0:
            value_field = source.fields().at(value_field_index)
        else:
            value_field = None
        category_field_indexes = list()

        # generate output fields
        fields = QgsFields()
        for field_name in category_field_names:
            c = source.fields().lookupField(field_name)
            if c == -1:
                raise QgsProcessingException(self.tr('Field "{field_name}" does not exist.').format(field_name=field_name))
            category_field_indexes.append(c)
            fields.append(source.fields().at(c))

        def addField(name):
            """
            Adds a field to the output, keeping the same data type as the value_field
            """
            field = QgsField(value_field)
            field.setName(name)
            fields.append(field)

        if value_field is None:
            field_type = 'none'
            fields.append(QgsField('count', QMetaType.Type.Int))
        elif value_field.isNumeric():
            field_type = 'numeric'
            fields.append(QgsField('count', QMetaType.Type.Int))
            fields.append(QgsField('unique', QMetaType.Type.Int))
            fields.append(QgsField('min', QMetaType.Type.Double))
            fields.append(QgsField('max', QMetaType.Type.Double))
            fields.append(QgsField('range', QMetaType.Type.Double))
            fields.append(QgsField('sum', QMetaType.Type.Double))
            fields.append(QgsField('mean', QMetaType.Type.Double))
            fields.append(QgsField('median', QMetaType.Type.Double))
            fields.append(QgsField('stddev', QMetaType.Type.Double))
            fields.append(QgsField('minority', QMetaType.Type.Double))
            fields.append(QgsField('majority', QMetaType.Type.Double))
            fields.append(QgsField('q1', QMetaType.Type.Double))
            fields.append(QgsField('q3', QMetaType.Type.Double))
            fields.append(QgsField('iqr', QMetaType.Type.Double))
        elif value_field.type() in (QMetaType.Type.QDate, QMetaType.Type.QTime, QMetaType.Type.QDateTime):
            field_type = 'datetime'
            fields.append(QgsField('count', QMetaType.Type.Int))
            fields.append(QgsField('unique', QMetaType.Type.Int))
            fields.append(QgsField('empty', QMetaType.Type.Int))
            fields.append(QgsField('filled', QMetaType.Type.Int))
            # keep same data type for these fields
            addField('min')
            addField('max')
        else:
            field_type = 'string'
            fields.append(QgsField('count', QMetaType.Type.Int))
            fields.append(QgsField('unique', QMetaType.Type.Int))
            fields.append(QgsField('empty', QMetaType.Type.Int))
            fields.append(QgsField('filled', QMetaType.Type.Int))
            # keep same data type for these fields
            addField('min')
            addField('max')
            fields.append(QgsField('min_length', QMetaType.Type.Int))
            fields.append(QgsField('max_length', QMetaType.Type.Int))
            fields.append(QgsField('mean_length', QMetaType.Type.Double))

        request = QgsFeatureRequest().setFlags(QgsFeatureRequest.Flag.NoGeometry)
        if value_field is not None:
            attrs = [value_field_index]
        else:
            attrs = []
        attrs.extend(category_field_indexes)
        request.setSubsetOfAttributes(attrs)
        features = source.getFeatures(request, QgsProcessingFeatureSource.Flag.FlagSkipGeometryValidityChecks)
        total = 50.0 / source.featureCount() if source.featureCount() else 0
        if field_type == 'none':
            values = defaultdict(lambda: 0)
        else:
            values = defaultdict(list)
        for current, feat in enumerate(features):
            if feedback.isCanceled():
                break

            feedback.setProgress(int(current * total))
            attrs = feat.attributes()
            cat = tuple([attrs[c] for c in category_field_indexes])
            if field_type == 'none':
                values[cat] += 1
                continue
            if field_type == 'numeric':
                if attrs[value_field_index] == NULL:
                    continue
                else:
                    value = float(attrs[value_field_index])
            elif field_type == 'string':
                if attrs[value_field_index] == NULL:
                    value = ''
                else:
                    value = str(attrs[value_field_index])
            elif attrs[value_field_index] == NULL:
                value = NULL
            else:
                value = attrs[value_field_index]
            values[cat].append(value)

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, QgsWkbTypes.Type.NoGeometry, QgsCoordinateReferenceSystem())
        if sink is None:
            raise QgsProcessingException(self.invalidSinkError(parameters, self.OUTPUT))

        if field_type == 'none':
            self.saveCounts(values, sink, feedback)
        elif field_type == 'numeric':
            self.calcNumericStats(values, sink, feedback)
        elif field_type == 'datetime':
            self.calcDateTimeStats(values, sink, feedback)
        else:
            self.calcStringStats(values, sink, feedback)

        sink.finalize()
        return {self.OUTPUT: dest_id}

    def saveCounts(self, values, sink, feedback):
        total = 50.0 / len(values) if values else 0
        current = 0
        for cat, v in values.items():
            if feedback.isCanceled():
                break

            feedback.setProgress(int(current * total) + 50)
            f = QgsFeature()
            f.setAttributes(list(cat) + [v])
            sink.addFeature(f, QgsFeatureSink.Flag.FastInsert)
            current += 1

    def calcNumericStats(self, values, sink, feedback):
        stat = QgsStatisticalSummary()

        total = 50.0 / len(values) if values else 0
        current = 0
        for cat, v in values.items():
            if feedback.isCanceled():
                break

            feedback.setProgress(int(current * total) + 50)

            stat.calculate(v)
            f = QgsFeature()
            f.setAttributes(list(cat) + [stat.count(),
                                         stat.variety(),
                                         stat.min(),
                                         stat.max(),
                                         stat.range(),
                                         stat.sum(),
                                         stat.mean(),
                                         stat.median(),
                                         stat.stDev(),
                                         stat.minority(),
                                         stat.majority(),
                                         stat.firstQuartile(),
                                         stat.thirdQuartile(),
                                         stat.interQuartileRange()])

            sink.addFeature(f, QgsFeatureSink.Flag.FastInsert)
            current += 1

    def calcDateTimeStats(self, values, sink, feedback):
        stat = QgsDateTimeStatisticalSummary()

        total = 50.0 / len(values) if values else 0
        current = 0
        for cat, v in values.items():
            if feedback.isCanceled():
                break

            feedback.setProgress(int(current * total) + 50)

            stat.calculate(v)
            f = QgsFeature()
            f.setAttributes(list(cat) + [stat.count(),
                                         stat.countDistinct(),
                                         stat.countMissing(),
                                         stat.count() - stat.countMissing(),
                                         stat.statistic(QgsDateTimeStatisticalSummary.Statistic.Min),
                                         stat.statistic(QgsDateTimeStatisticalSummary.Statistic.Max)
                                         ])

            sink.addFeature(f, QgsFeatureSink.Flag.FastInsert)
            current += 1

    def calcStringStats(self, values, sink, feedback):
        stat = QgsStringStatisticalSummary()

        total = 50.0 / len(values) if values else 0
        current = 0
        for cat, v in values.items():
            if feedback.isCanceled():
                break

            feedback.setProgress(int(current * total) + 50)

            stat.calculate(v)
            f = QgsFeature()
            f.setAttributes(list(cat) + [stat.count(),
                                         stat.countDistinct(),
                                         stat.countMissing(),
                                         stat.count() - stat.countMissing(),
                                         stat.min(),
                                         stat.max(),
                                         stat.minLength(),
                                         stat.maxLength(),
                                         stat.meanLength()
                                         ])

            sink.addFeature(f, QgsFeatureSink.Flag.FastInsert)
            current += 1
