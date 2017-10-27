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
from builtins import str
from builtins import zip
from builtins import range

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
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterField,
                       QgsProcessingParameterFeatureSink)

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
    OUTPUT = "OUTPUT"

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'join_location.png'))

    def group(self):
        return self.tr('Vector general')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.predicates = (
            ('intersects', self.tr('intersects')),
            ('contains', self.tr('contains')),
            ('equals', self.tr('equals')),
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
            self.tr('Create separate feature for each located feature'),
            self.tr('Take attributes of the first located feature only')
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
                'class': 'processing.gui.wrappers.EnumWidgetWrapper',
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
        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT,
                                                            self.tr('Joined layer')))

    def name(self):
        return 'joinattributesbylocation'

    def displayName(self):
        return self.tr('Join attributes by location')

    def tags(self):
        return self.tr("join,intersects,intersecting,touching,within,contains,overlaps,relation,spatial").split(',')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        join_source = self.parameterAsSource(parameters, self.JOIN, context)
        join_fields = self.parameterAsFields(parameters, self.JOIN_FIELDS, context)
        method = self.parameterAsEnum(parameters, self.METHOD, context)
        discard_nomatch = self.parameterAsBool(parameters, self.DISCARD_NONMATCHING, context)

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

        out_fields = QgsProcessingUtils.combineFields(source_fields, fields_to_join)

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               out_fields, source.wkbType(), source.sourceCrs())

        # do the join

        # build a list of 'reversed' predicates, because in this function
        # we actually test the reverse of what the user wants (allowing us
        # to prepare geometries and optimise the algorithm)
        predicates = [self.reversed_predicates[self.predicates[i][0]] for i in
                      self.parameterAsEnums(parameters, self.PREDICATE, context)]

        remaining = set()
        if not discard_nomatch:
            remaining = set(source.allFeatureIds())

        added_set = set()

        request = QgsFeatureRequest().setSubsetOfAttributes(join_field_indexes).setDestinationCrs(source.sourceCrs())
        features = join_source.getFeatures(request)
        total = 100.0 / join_source.featureCount() if join_source.featureCount() else 0

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
                    join_attributes.append(f.attributes()[a])

                if engine is None:
                    engine = QgsGeometry.createGeometryEngine(f.geometry().constGet())
                    engine.prepareGeometry()

                for predicate in predicates:
                    if getattr(engine, predicate)(test_feat.geometry().constGet()):
                        added_set.add(test_feat.id())

                        # join attributes and add
                        attributes = test_feat.attributes()
                        attributes.extend(join_attributes)
                        output_feature = test_feat
                        output_feature.setAttributes(attributes)
                        sink.addFeature(output_feature, QgsFeatureSink.FastInsert)
                        break

            feedback.setProgress(int(current * total))

        if not discard_nomatch:
            remaining = remaining.difference(added_set)
            for f in source.getFeatures(QgsFeatureRequest().setFilterFids(list(remaining))):
                if feedback.isCanceled():
                    break
                sink.addFeature(f, QgsFeatureSink.FastInsert)

        return {self.OUTPUT: dest_id}
