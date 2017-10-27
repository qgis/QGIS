# -*- coding: utf-8 -*-

"""
***************************************************************************
    TopoColors.py
    --------------
    Date                 : February 2017
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
__date__ = 'February 2017'
__copyright__ = '(C) 2017, Nyall Dawson'

# This will get replaced with a git SHA1 when you do a git archive323

__revision__ = '$Format:%H$'

import os
import operator
import sys

from collections import defaultdict

from qgis.core import (QgsField,
                       QgsFeatureSink,
                       QgsGeometry,
                       QgsSpatialIndex,
                       QgsPointXY,
                       NULL,
                       QgsProcessing,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterFeatureSink)

from qgis.PyQt.QtCore import (QVariant)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class TopoColor(QgisAlgorithm):
    INPUT = 'INPUT'
    MIN_COLORS = 'MIN_COLORS'
    MIN_DISTANCE = 'MIN_DISTANCE'
    BALANCE = 'BALANCE'
    OUTPUT = 'OUTPUT'

    def tags(self):
        return self.tr('topocolor,colors,graph,adjacent,assign').split(',')

    def group(self):
        return self.tr('Cartography')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):

        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer'), [QgsProcessing.TypeVectorPolygon]))
        self.addParameter(QgsProcessingParameterNumber(self.MIN_COLORS,
                                                       self.tr('Minimum number of colors'), minValue=1, maxValue=1000, defaultValue=4))
        self.addParameter(QgsProcessingParameterNumber(self.MIN_DISTANCE,
                                                       self.tr('Minimum distance between features'), type=QgsProcessingParameterNumber.Double,
                                                       minValue=0.0, maxValue=999999999.0, defaultValue=0.0))
        balance_by = [self.tr('By feature count'),
                      self.tr('By assigned area'),
                      self.tr('By distance between colors')]
        self.addParameter(QgsProcessingParameterEnum(
            self.BALANCE,
            self.tr('Balance color assignment'),
            options=balance_by, defaultValue=0))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Colored'), QgsProcessing.TypeVectorPolygon))

    def name(self):
        return 'topologicalcoloring'

    def displayName(self):
        return self.tr('Topological coloring')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        min_colors = self.parameterAsInt(parameters, self.MIN_COLORS, context)
        balance_by = self.parameterAsEnum(parameters, self.BALANCE, context)
        min_distance = self.parameterAsDouble(parameters, self.MIN_DISTANCE, context)

        fields = source.fields()
        fields.append(QgsField('color_id', QVariant.Int))

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, source.wkbType(), source.sourceCrs())

        features = {f.id(): f for f in source.getFeatures()}

        topology, id_graph = self.compute_graph(features, feedback, min_distance=min_distance)
        feature_colors = ColoringAlgorithm.balanced(features,
                                                    balance=balance_by,
                                                    graph=topology,
                                                    feedback=feedback,
                                                    min_colors=min_colors)

        if len(feature_colors) == 0:
            return {self.OUTPUT: dest_id}

        max_colors = max(feature_colors.values())
        feedback.pushInfo(self.tr('{} colors required').format(max_colors))

        total = 20.0 / len(features)
        current = 0
        for feature_id, input_feature in features.items():
            if feedback.isCanceled():
                break

            output_feature = input_feature
            attributes = input_feature.attributes()
            if feature_id in feature_colors:
                attributes.append(feature_colors[feature_id])
            else:
                attributes.append(NULL)
            output_feature.setAttributes(attributes)

            sink.addFeature(output_feature, QgsFeatureSink.FastInsert)
            current += 1
            feedback.setProgress(80 + int(current * total))

        return {self.OUTPUT: dest_id}

    @staticmethod
    def compute_graph(features, feedback, create_id_graph=False, min_distance=0):
        """ compute topology from a layer/field """
        s = Graph(sort_graph=False)
        id_graph = None
        if create_id_graph:
            id_graph = Graph(sort_graph=True)

        # skip features without geometry
        features_with_geometry = {f_id: f for (f_id, f) in features.items() if f.hasGeometry()}

        total = 70.0 / len(features_with_geometry) if features_with_geometry else 1
        index = QgsSpatialIndex()

        i = 0
        for feature_id, f in features_with_geometry.items():
            if feedback.isCanceled():
                break

            g = f.geometry()
            if min_distance > 0:
                g = g.buffer(min_distance, 5)

            engine = QgsGeometry.createGeometryEngine(g.constGet())
            engine.prepareGeometry()

            feature_bounds = g.boundingBox()
            # grow bounds a little so we get touching features
            feature_bounds.grow(feature_bounds.width() * 0.01)
            intersections = index.intersects(feature_bounds)
            for l2 in intersections:
                f2 = features_with_geometry[l2]
                if engine.intersects(f2.geometry().constGet()):
                    s.add_edge(f.id(), f2.id())
                    s.add_edge(f2.id(), f.id())
                    if id_graph:
                        id_graph.add_edge(f.id(), f2.id())

            index.insertFeature(f)
            i += 1
            feedback.setProgress(int(i * total))

        for feature_id, f in features_with_geometry.items():
            if feedback.isCanceled():
                break

            if feature_id not in s.node_edge:
                s.add_edge(feature_id, None)

        return s, id_graph


class ColoringAlgorithm:

    @staticmethod
    def balanced(features, graph, feedback, balance=0, min_colors=4):
        feature_colors = {}
        # start with minimum number of colors in pool
        color_pool = set(range(1, min_colors + 1))

        # calculate count of neighbours
        neighbour_count = defaultdict(int)
        for feature_id, neighbours in graph.node_edge.items():
            neighbour_count[feature_id] += len(neighbours)

        # sort features by neighbour count - we want to handle those with more neighbours first
        sorted_by_count = [feature_id for feature_id in sorted(neighbour_count.items(),
                                                               key=operator.itemgetter(1),
                                                               reverse=True)]
        # counts for each color already assigned
        color_counts = defaultdict(int)
        color_areas = defaultdict(float)
        for c in color_pool:
            color_counts[c] = 0
            color_areas[c] = 0

        total = 10.0 / len(sorted_by_count) if sorted_by_count else 1
        i = 0

        for (feature_id, n) in sorted_by_count:
            if feedback.isCanceled():
                break

            # first work out which already assigned colors are adjacent to this feature
            adjacent_colors = set()
            for neighbour in graph.node_edge[feature_id]:
                if neighbour in feature_colors:
                    adjacent_colors.add(feature_colors[neighbour])

            # from the existing colors, work out which are available (ie non-adjacent)
            available_colors = color_pool.difference(adjacent_colors)

            feature_color = -1
            if len(available_colors) == 0:
                # no existing colors available for this feature, so add new color to pool and repeat
                min_colors += 1
                return ColoringAlgorithm.balanced(features, graph, feedback, balance, min_colors)
            else:
                if balance == 0:
                    # choose least used available color
                    counts = [(c, v) for c, v in color_counts.items() if c in available_colors]
                    feature_color = sorted(counts, key=operator.itemgetter(1))[0][0]
                    color_counts[feature_color] += 1
                elif balance == 1:
                    areas = [(c, v) for c, v in color_areas.items() if c in available_colors]
                    feature_color = sorted(areas, key=operator.itemgetter(1))[0][0]
                    color_areas[feature_color] += features[feature_id].geometry().area()
                elif balance == 2:
                    min_distances = {c: sys.float_info.max for c in available_colors}
                    this_feature_centroid = features[feature_id].geometry().centroid().constGet()

                    # find features for all available colors
                    other_features = {f_id: c for (f_id, c) in feature_colors.items() if c in available_colors}

                    # loop through these, and calculate the minimum distance from this feature to the nearest
                    # feature with each assigned color
                    for other_feature_id, c in other_features.items():
                        if feedback.isCanceled():
                            break

                        other_geometry = features[other_feature_id].geometry()
                        other_centroid = other_geometry.centroid().constGet()

                        distance = this_feature_centroid.distanceSquared(other_centroid)
                        if distance < min_distances[c]:
                            min_distances[c] = distance

                    # choose color such that minimum distance is maximised! ie we want MAXIMAL separation between
                    # features with the same color
                    feature_color = sorted(min_distances, key=min_distances.__getitem__, reverse=True)[0]

            feature_colors[feature_id] = feature_color

            i += 1
            feedback.setProgress(70 + int(i * total))

        return feature_colors


class Graph:

    def __init__(self, sort_graph=True):
        self.sort_graph = sort_graph
        self.node_edge = {}

    def add_edge(self, i, j):
        ij = [i, j]
        if self.sort_graph:
            ij.sort()
        (i, j) = ij
        if i in self.node_edge:
            self.node_edge[i].add(j)
        else:
            self.node_edge[i] = {j}

    def make_full(self):
        g = Graph(sort_graph=False)
        for k in self.node_edge.keys():
            for v in self.node_edge[k]:
                g.add_edge(v, k)
                g.add_edge(k, v)
        return g
