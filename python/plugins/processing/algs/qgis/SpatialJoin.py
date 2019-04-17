# -*- coding: utf-8 -*-

"""
***************************************************************************
    SpatialJoin.py
    ---------------------
    Date                 : October 2013
    Copyright            : (C) 2013 by Joshua Arnott
    Email                : josh at snorfalorpagus dot net
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Joshua Arnott'
__date__ = 'October 2013'
__copyright__ = '(C) 2013, Joshua Arnott'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtGui import QIcon

from qgis.core import (QgsFields,
                       QgsFeatureSink,
                       QgsFeatureRequest,
                       QgsGeometry,
                       QgsProcessing,
                       QgsProcessingUtils,
                       QgsProcessingException,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterField,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingParameterString,
                       QgsProcessingOutputNumber)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.tools import vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class SpatialJoin(QgisAlgorithm):
    INPUT = "INPUT"
    JOIN = "JOIN"
    PREDICATE = "PREDICATE"
    JOIN_FIELDS = "JOIN_FIELDS"
    METHOD = "METHOD"
    DISCARD_NONMATCHING = "DISCARD_NONMATCHING"
    PREFIX = "PREFIX"
    OUTPUT = "OUTPUT"
    NON_MATCHING = "NON_MATCHING"
    JOINED_COUNT = "JOINED_COUNT"

    def group(self):
        return self.tr('Vector general')

    def groupId(self):
        return 'vectorgeneral'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.predicates = (
            ('intersects', self.tr('intersects')),
            ('contains', self.tr('contains')),
            ('isEqual', self.tr('equals')),
            ('touches', self.tr('touches')),
            ('overlaps', self.tr('overlaps')),
            ('within', self.tr('within')),
            ('crosses', self.tr('crosses')))

        self.reversed_predicates = {'intersects': 'intersects',
                                    'contains': 'within',
                                    'isEqual': 'isEqual',
                                    'touches': 'touches',
                                    'overlaps': 'overlaps',
                                    'within': 'contains',
                                    'crosses': 'crosses'}

        self.methods = [
            self.tr('Create separate feature for each located feature (one-to-many)'),
            self.tr('Take attributes of the first located feature only (one-to-one)')
        ]

        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer'),
                                                              [QgsProcessing.TypeVectorAnyGeometry]))
        self.addParameter(QgsProcessingParameterFeatureSource(self.JOIN,
                                                              self.tr('Join layer'),
                                                              [QgsProcessing.TypeVectorAnyGeometry]))

        predicate = QgsProcessingParameterEnum(self.PREDICATE,
                                               self.tr('Geometric predicate'),
                                               options=[p[1] for p in self.predicates],
                                               allowMultiple=True, defaultValue=[0])
        predicate.setMetadata({
            'widget_wrapper': {
                'useCheckBoxes': True,
                'columns': 2}})
        self.addParameter(predicate)
        self.addParameter(QgsProcessingParameterField(self.JOIN_FIELDS,
                                                      self.tr('Fields to add (leave empty to use all fields)'),
                                                      parentLayerParameterName=self.JOIN,
                                                      allowMultiple=True, optional=True))
        self.addParameter(QgsProcessingParameterEnum(self.METHOD,
                                                     self.tr('Join type'), self.methods))
        self.addParameter(QgsProcessingParameterBoolean(self.DISCARD_NONMATCHING,
                                                        self.tr('Discard records which could not be joined'),
                                                        defaultValue=False))
        self.addParameter(QgsProcessingParameterString(self.PREFIX,
                                                       self.tr('Joined field prefix'), optional=True))
        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT,
                                                            self.tr('Joined layer'),
                                                            QgsProcessing.TypeVectorAnyGeometry,
                                                            defaultValue=None, optional=True, createByDefault=True))

        non_matching = QgsProcessingParameterFeatureSink(self.NON_MATCHING,
                                                         self.tr('Unjoinable features from first layer'),
                                                         QgsProcessing.TypeVectorAnyGeometry,
                                                         defaultValue=None, optional=True, createByDefault=False)
        # TODO GUI doesn't support advanced outputs yet
        # non_matching.setFlags(non_matching.flags() | QgsProcessingParameterDefinition.FlagAdvanced )
        self.addParameter(non_matching)

        self.addOutput(QgsProcessingOutputNumber(self.JOINED_COUNT, self.tr("Number of joined features from input table")))

    def name(self):
        return 'joinattributesbylocation'

    def displayName(self):
        return self.tr('Join attributes by location')

    def tags(self):
        return self.tr("join,intersects,intersecting,touching,within,contains,overlaps,relation,spatial").split(',')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        if source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.INPUT))

        join_source = self.parameterAsSource(parameters, self.JOIN, context)
        if join_source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.JOIN))

        join_fields = self.parameterAsFields(parameters, self.JOIN_FIELDS, context)
        method = self.parameterAsEnum(parameters, self.METHOD, context)
        discard_nomatch = self.parameterAsBoolean(parameters, self.DISCARD_NONMATCHING, context)
        prefix = self.parameterAsString(parameters, self.PREFIX, context)

        source_fields = source.fields()
        fields_to_join = QgsFields()
        join_field_indexes = []
        if not join_fields:
            fields_to_join = join_source.fields()
            join_field_indexes = [i for i in range(len(fields_to_join))]
        else:
            for f in join_fields:
                idx = join_source.fields().lookupField(f)
                join_field_indexes.append(idx)
                if idx >= 0:
                    fields_to_join.append(join_source.fields().at(idx))

        if prefix:
            prefixed_fields = QgsFields()
            for i in range(len(fields_to_join)):
                field = fields_to_join[i]
                field.setName(prefix + field.name())
                prefixed_fields.append(field)
            fields_to_join = prefixed_fields

        out_fields = QgsProcessingUtils.combineFields(source_fields, fields_to_join)

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               out_fields, source.wkbType(), source.sourceCrs(), QgsFeatureSink.RegeneratePrimaryKey)
        if self.OUTPUT in parameters and parameters[self.OUTPUT] is not None and sink is None:
            raise QgsProcessingException(self.invalidSinkError(parameters, self.OUTPUT))

        (non_matching_sink, non_matching_dest_id) = self.parameterAsSink(parameters, self.NON_MATCHING, context,
                                                                         source.fields(), source.wkbType(), source.sourceCrs(), QgsFeatureSink.RegeneratePrimaryKey)
        if self.NON_MATCHING in parameters and parameters[self.NON_MATCHING] is not None and non_matching_sink is None:
            raise QgsProcessingException(self.invalidSinkError(parameters, self.NON_MATCHING))

        # do the join

        # build a list of 'reversed' predicates, because in this function
        # we actually test the reverse of what the user wants (allowing us
        # to prepare geometries and optimise the algorithm)
        predicates = [self.reversed_predicates[self.predicates[i][0]] for i in
                      self.parameterAsEnums(parameters, self.PREDICATE, context)]

        remaining = set()
        if not discard_nomatch or non_matching_sink is not None:
            remaining = set(source.allFeatureIds())

        added_set = set()

        request = QgsFeatureRequest().setSubsetOfAttributes(join_field_indexes).setDestinationCrs(source.sourceCrs(), context.transformContext())
        features = join_source.getFeatures(request)
        total = 100.0 / join_source.featureCount() if join_source.featureCount() else 0

        joined_count = 0
        unjoined_count = 0

        for current, f in enumerate(features):
            if feedback.isCanceled():
                break

            if not f.hasGeometry():
                continue

            bbox = f.geometry().boundingBox()
            engine = None

            request = QgsFeatureRequest().setFilterRect(bbox)
            for test_feat in source.getFeatures(request):
                if feedback.isCanceled():
                    break
                if method == 1 and test_feat.id() in added_set:
                    # already added this feature, and user has opted to only output first match
                    continue

                join_attributes = []
                for a in join_field_indexes:
                    join_attributes.append(f[a])

                if engine is None:
                    engine = QgsGeometry.createGeometryEngine(f.geometry().constGet())
                    engine.prepareGeometry()

                for predicate in predicates:
                    if getattr(engine, predicate)(test_feat.geometry().constGet()):
                        added_set.add(test_feat.id())

                        if sink is not None:
                            # join attributes and add
                            attributes = test_feat.attributes()
                            attributes.extend(join_attributes)
                            output_feature = test_feat
                            output_feature.setAttributes(attributes)
                            sink.addFeature(output_feature, QgsFeatureSink.FastInsert)
                        break

            feedback.setProgress(int(current * total))

        if not discard_nomatch or non_matching_sink is not None:
            remaining = remaining.difference(added_set)
            for f in source.getFeatures(QgsFeatureRequest().setFilterFids(list(remaining))):
                if feedback.isCanceled():
                    break
                if sink is not None:
                    sink.addFeature(f, QgsFeatureSink.FastInsert)
                if non_matching_sink is not None:
                    non_matching_sink.addFeature(f, QgsFeatureSink.FastInsert)

        result = {}
        if sink is not None:
            result[self.OUTPUT] = dest_id
        if non_matching_sink is not None:
            result[self.NON_MATCHING] = non_matching_dest_id

        result[self.JOINED_COUNT] = len(added_set)

        return result
