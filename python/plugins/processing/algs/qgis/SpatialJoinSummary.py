# -*- coding: utf-8 -*-

"""
***************************************************************************
    SpatialJoinSummary.py
    ---------------------
    Date                 : September 2017
    Copyright            : (C) 2017 by Nyall Dawson
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
__date__ = 'September 2017'
__copyright__ = '(C) 2017, Nyall Dawson'

import os
import math

from collections import defaultdict

from qgis.PyQt.QtGui import QIcon
from qgis.PyQt.QtCore import QVariant
from qgis.core import (NULL,
                       QgsApplication,
                       QgsField,
                       QgsFields,
                       QgsFeatureSink,
                       QgsFeatureRequest,
                       QgsGeometry,
                       QgsFeatureSource,
                       QgsCoordinateTransform,
                       QgsStatisticalSummary,
                       QgsDateTimeStatisticalSummary,
                       QgsStringStatisticalSummary,
                       QgsProcessing,
                       QgsProcessingUtils,
                       QgsProcessingException,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterField,
                       QgsProcessingParameterFeatureSink)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.tools import vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class SpatialJoinSummary(QgisAlgorithm):
    INPUT = "INPUT"
    JOIN = "JOIN"
    PREDICATE = "PREDICATE"
    JOIN_FIELDS = "JOIN_FIELDS"
    SUMMARIES = "SUMMARIES"
    DISCARD_NONMATCHING = "DISCARD_NONMATCHING"
    OUTPUT = "OUTPUT"

    def group(self):
        return self.tr('Vector general')

    def groupId(self):
        return 'vectorgeneral'

    def __init__(self):
        super().__init__()

    def icon(self):
        return QgsApplication.getThemeIcon("/algorithms/mAlgorithmBasicStatistics.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("/algorithms/mAlgorithmBasicStatistics.svg")

    def initAlgorithm(self, config=None):
        self.predicates = (
            ('intersects', self.tr('intersect')),
            ('contains', self.tr('contain')),
            ('isEqual', self.tr('equal')),
            ('touches', self.tr('touch')),
            ('overlaps', self.tr('overlap')),
            ('within', self.tr('are within')),
            ('crosses', self.tr('cross')))

        self.statistics = [
            ('count', self.tr('count')),
            ('unique', self.tr('unique')),
            ('min', self.tr('min')),
            ('max', self.tr('max')),
            ('range', self.tr('range')),
            ('sum', self.tr('sum')),
            ('mean', self.tr('mean')),
            ('median', self.tr('median')),
            ('stddev', self.tr('stddev')),
            ('minority', self.tr('minority')),
            ('majority', self.tr('majority')),
            ('q1', self.tr('q1')),
            ('q3', self.tr('q3')),
            ('iqr', self.tr('iqr')),
            ('empty', self.tr('empty')),
            ('filled', self.tr('filled')),
            ('min_length', self.tr('min_length')),
            ('max_length', self.tr('max_length')),
            ('mean_length', self.tr('mean_length'))]

        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Join to features in'),
                                                              [QgsProcessing.TypeVectorAnyGeometry]))
        predicate = QgsProcessingParameterEnum(self.PREDICATE,
                                               self.tr('Where the features'),
                                               options=[p[1] for p in self.predicates],
                                               allowMultiple=True, defaultValue=[0])
        predicate.setMetadata({
            'widget_wrapper': {
                'useCheckBoxes': True,
                'columns': 2}})
        self.addParameter(predicate)
        self.addParameter(QgsProcessingParameterFeatureSource(self.JOIN,
                                                              self.tr('By comparing to'),
                                                              [QgsProcessing.TypeVectorAnyGeometry]))
        self.addParameter(QgsProcessingParameterField(self.JOIN_FIELDS,
                                                      self.tr('Fields to summarise (leave empty to use all fields)'),
                                                      parentLayerParameterName=self.JOIN,
                                                      allowMultiple=True, optional=True))
        self.addParameter(QgsProcessingParameterEnum(self.SUMMARIES,
                                                     self.tr(
                                                         'Summaries to calculate (leave empty to use all available)'),
                                                     options=[p[1] for p in self.statistics],
                                                     allowMultiple=True, optional=True))
        self.addParameter(QgsProcessingParameterBoolean(self.DISCARD_NONMATCHING,
                                                        self.tr('Discard records which could not be joined'),
                                                        defaultValue=False))
        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT,
                                                            self.tr('Joined layer')))

    def name(self):
        return 'joinbylocationsummary'

    def displayName(self):
        return self.tr('Join attributes by location (summary)')

    def tags(self):
        return self.tr(
            "summary,aggregate,join,intersects,intersecting,touching,within,contains,overlaps,relation,spatial,"
            "stats,statistics,sum,maximum,minimum,mean,average,standard,deviation,"
            "count,distinct,unique,variance,median,quartile,range,majority,minority,histogram,distinct").split(',')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        if source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.INPUT))

        join_source = self.parameterAsSource(parameters, self.JOIN, context)
        if join_source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.JOIN))

        if join_source.hasSpatialIndex() == QgsFeatureSource.SpatialIndexNotPresent:
            feedback.reportError(self.tr("No spatial index exists for join layer, performance will be severely degraded"))

        join_fields = self.parameterAsFields(parameters, self.JOIN_FIELDS, context)
        discard_nomatch = self.parameterAsBoolean(parameters, self.DISCARD_NONMATCHING, context)
        summaries = [self.statistics[i][0] for i in
                     sorted(self.parameterAsEnums(parameters, self.SUMMARIES, context))]

        if not summaries:
            # none selected, so use all
            summaries = [s[0] for s in self.statistics]

        source_fields = source.fields()
        fields_to_join = QgsFields()
        join_field_indexes = []
        if not join_fields:
            # no fields selected, use all
            join_fields = [join_source.fields().at(i).name() for i in range(len(join_source.fields()))]

        def addFieldKeepType(original, stat):
            """
            Adds a field to the output, keeping the same data type as the original
            """
            field = QgsField(original)
            field.setName(field.name() + '_' + stat)
            fields_to_join.append(field)

        def addField(original, stat, type):
            """
            Adds a field to the output, with a specified type
            """
            field = QgsField(original)
            field.setName(field.name() + '_' + stat)
            field.setType(type)
            if type == QVariant.Double:
                field.setLength(20)
                field.setPrecision(6)
            fields_to_join.append(field)

        numeric_fields = (
            ('count', QVariant.Int, 'count'),
            ('unique', QVariant.Int, 'variety'),
            ('min', QVariant.Double, 'min'),
            ('max', QVariant.Double, 'max'),
            ('range', QVariant.Double, 'range'),
            ('sum', QVariant.Double, 'sum'),
            ('mean', QVariant.Double, 'mean'),
            ('median', QVariant.Double, 'median'),
            ('stddev', QVariant.Double, 'stDev'),
            ('minority', QVariant.Double, 'minority'),
            ('majority', QVariant.Double, 'majority'),
            ('q1', QVariant.Double, 'firstQuartile'),
            ('q3', QVariant.Double, 'thirdQuartile'),
            ('iqr', QVariant.Double, 'interQuartileRange')
        )

        datetime_fields = (
            ('count', QVariant.Int, 'count'),
            ('unique', QVariant.Int, 'countDistinct'),
            ('empty', QVariant.Int, 'countMissing'),
            ('filled', QVariant.Int),
            ('min', None),
            ('max', None)
        )

        string_fields = (
            ('count', QVariant.Int, 'count'),
            ('unique', QVariant.Int, 'countDistinct'),
            ('empty', QVariant.Int, 'countMissing'),
            ('filled', QVariant.Int),
            ('min', None, 'min'),
            ('max', None, 'max'),
            ('min_length', QVariant.Int, 'minLength'),
            ('max_length', QVariant.Int, 'maxLength'),
            ('mean_length', QVariant.Double, 'meanLength')
        )

        field_types = []
        for f in join_fields:
            idx = join_source.fields().lookupField(f)
            if idx >= 0:
                join_field_indexes.append(idx)

                join_field = join_source.fields().at(idx)
                if join_field.isNumeric():
                    field_types.append('numeric')
                    field_list = numeric_fields
                elif join_field.type() in (QVariant.Date, QVariant.Time, QVariant.DateTime):
                    field_types.append('datetime')
                    field_list = datetime_fields
                else:
                    field_types.append('string')
                    field_list = string_fields

                for f in field_list:
                    if f[0] in summaries:
                        if f[1] is not None:
                            addField(join_field, f[0], f[1])
                        else:
                            addFieldKeepType(join_field, f[0])

        out_fields = QgsProcessingUtils.combineFields(source_fields, fields_to_join)

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               out_fields, source.wkbType(), source.sourceCrs())
        if sink is None:
            raise QgsProcessingException(self.invalidSinkError(parameters, self.OUTPUT))

        # do the join
        predicates = [self.predicates[i][0] for i in self.parameterAsEnums(parameters, self.PREDICATE, context)]

        features = source.getFeatures()
        total = 100.0 / source.featureCount() if source.featureCount() else 0

        for current, f in enumerate(features):
            if feedback.isCanceled():
                break

            if not f.hasGeometry():
                if not discard_nomatch:
                    # ensure consistent count of attributes - otherwise non matching
                    # features will have incorrect attribute length
                    # and provider may reject them
                    attrs = f.attributes()
                    if len(attrs) < len(out_fields):
                        attrs += [NULL] * (len(out_fields) - len(attrs))
                    f.setAttributes(attrs)
                    sink.addFeature(f, QgsFeatureSink.FastInsert)
                continue

            engine = None

            values = []

            request = QgsFeatureRequest().setFilterRect(f.geometry().boundingBox()).setSubsetOfAttributes(join_field_indexes).setDestinationCrs(source.sourceCrs(), context.transformContext())
            for test_feat in join_source.getFeatures(request):
                if feedback.isCanceled():
                    break

                join_attributes = []
                for a in join_field_indexes:
                    join_attributes.append(test_feat[a])

                if engine is None:
                    engine = QgsGeometry.createGeometryEngine(f.geometry().constGet())
                    engine.prepareGeometry()

                for predicate in predicates:
                    if getattr(engine, predicate)(test_feat.geometry().constGet()):
                        values.append(join_attributes)
                        break

            feedback.setProgress(int(current * total))

            if len(values) == 0:
                if discard_nomatch:
                    continue
                else:
                    # ensure consistent count of attributes - otherwise non matching
                    # features will have incorrect attribute length
                    # and provider may reject them
                    attrs = f.attributes()
                    if len(attrs) < len(out_fields):
                        attrs += [NULL] * (len(out_fields) - len(attrs))
                    f.setAttributes(attrs)
                    sink.addFeature(f, QgsFeatureSink.FastInsert)
            else:
                attrs = f.attributes()
                for i in range(len(join_field_indexes)):
                    attribute_values = [v[i] for v in values]
                    field_type = field_types[i]
                    if field_type == 'numeric':
                        stat = QgsStatisticalSummary()
                        for v in attribute_values:
                            stat.addVariant(v)
                        stat.finalize()
                        for s in numeric_fields:
                            if s[0] in summaries:
                                val = getattr(stat, s[2])()
                                attrs.append(val if not math.isnan(val) else NULL)
                    elif field_type == 'datetime':
                        stat = QgsDateTimeStatisticalSummary()
                        stat.calculate(attribute_values)
                        for s in datetime_fields:
                            if s[0] in summaries:
                                if s[0] == 'filled':
                                    attrs.append(stat.count() - stat.countMissing())
                                elif s[0] == 'min':
                                    attrs.append(stat.statistic(QgsDateTimeStatisticalSummary.Min))
                                elif s[0] == 'max':
                                    attrs.append(stat.statistic(QgsDateTimeStatisticalSummary.Max))
                                else:
                                    attrs.append(getattr(stat, s[2])())
                    else:
                        stat = QgsStringStatisticalSummary()
                        for v in attribute_values:
                            if v == NULL:
                                stat.addString('')
                            else:
                                stat.addString(str(v))
                        stat.finalize()
                        for s in string_fields:
                            if s[0] in summaries:
                                if s[0] == 'filled':
                                    attrs.append(stat.count() - stat.countMissing())
                                else:
                                    attrs.append(getattr(stat, s[2])())

                f.setAttributes(attrs)
                sink.addFeature(f, QgsFeatureSink.FastInsert)

        return {self.OUTPUT: dest_id}
