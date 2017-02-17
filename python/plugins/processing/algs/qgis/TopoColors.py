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

from collections import defaultdict, deque

from qgis.core import (QgsField,
                       QgsGeometry,
                       QgsSpatialIndex,
                       QgsPointV2,
                       NULL)

from qgis.PyQt.QtCore import (QVariant)

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import (ParameterVector,
                                        ParameterSelection,
                                        ParameterNumber)
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class TopoColor(GeoAlgorithm):
    INPUT_LAYER = 'INPUT_LAYER'
    MIN_COLORS = 'MIN_COLORS'
    BALANCE = 'BALANCE'
    OUTPUT_LAYER = 'OUTPUT_LAYER'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Topological coloring')
        self.group, self.i18n_group = self.trAlgorithm('Cartographic tools')
        self.tags = self.tr('topocolor,colors,graph,adjacent,assign')

        self.addParameter(ParameterVector(self.INPUT_LAYER,
                                          self.tr('Input layer'), [dataobjects.TYPE_VECTOR_POLYGON]))
        self.addParameter(ParameterNumber(self.MIN_COLORS,
                                          self.tr('Minimum number of colors'), 1, 1000, 4))
        balance_by = [self.tr('By feature count'),
                      self.tr('By assigned area'),
                      self.tr('By distance between colors')]
        self.addParameter(ParameterSelection(
            self.BALANCE,
            self.tr('Balance color assignment'),
            balance_by, default=0))

        self.addOutput(OutputVector(self.OUTPUT_LAYER, self.tr('Colored'), datatype=[dataobjects.TYPE_VECTOR_POLYGON]))

    def processAlgorithm(self, feedback):
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT_LAYER))
        min_colors = self.getParameterValue(self.MIN_COLORS)
        balance_by = self.getParameterValue(self.BALANCE)

        fields = layer.fields()
        fields.append(QgsField('color_id', QVariant.Int))

        writer = self.getOutputFromName(
            self.OUTPUT_LAYER).getVectorWriter(
            fields,
            layer.wkbType(),
            layer.crs())

        features = {f.id(): f for f in vector.features(layer)}

        topology, id_graph = self.compute_graph(features, feedback)
        feature_colors = ColoringAlgorithm.balanced(features,
                                                    balance=balance_by,
                                                    graph=topology,
                                                    feedback=feedback,
                                                    min_colors=min_colors)

        max_colors = max(feature_colors.values())
        feedback.pushInfo(self.tr('{} colors required').format(max_colors))

        total = 20.0 / len(features)
        current = 0
        for feature_id, input_feature in features.items():
            output_feature = input_feature
            attributes = input_feature.attributes()
            if feature_id in feature_colors:
                attributes.append(feature_colors[feature_id])
            else:
                attributes.append(NULL)
            output_feature.setAttributes(attributes)

            writer.addFeature(output_feature)
            current += 1
            feedback.setProgress(80 + int(current * total))

        del writer

    @staticmethod
    def compute_graph(features, feedback, create_id_graph=False):
        """ compute topology from a layer/field """
        s = Graph(sort_graph=False)
        id_graph = None
        if create_id_graph:
            id_graph = Graph(sort_graph=True)

        # skip features without geometry
        features_with_geometry = {f_id: f for (f_id, f) in features.items() if f.hasGeometry()}

        total = 70.0 / len(features_with_geometry)
        index = QgsSpatialIndex()

        i = 0
        for feature_id, f in features_with_geometry.items():
            engine = QgsGeometry.createGeometryEngine(f.geometry().geometry())
            engine.prepareGeometry()

            feature_bounds = f.geometry().boundingBox()
            # grow bounds a little so we get touching features
            feature_bounds.grow(feature_bounds.width() * 0.01)
            intersections = index.intersects(feature_bounds)
            for l2 in intersections:
                f2 = features_with_geometry[l2]
                if engine.intersects(f2.geometry().geometry()):
                    s.add_edge(f.id(), f2.id())
                    s.add_edge(f2.id(), f.id())
                    if id_graph:
                        id_graph.add_edge(f.id(), f2.id())

            index.insertFeature(f)
            i += 1
            feedback.setProgress(int(i * total))

        for feature_id, f in features_with_geometry.items():
            if not feature_id in s.node_edge:
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

        total = 10.0 / len(sorted_by_count)
        i = 0

        for (feature_id, n) in sorted_by_count:
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
                    this_feature_centroid = QgsPointV2(features[feature_id].geometry().centroid().geometry())

                    # find features for all available colors
                    other_features = {f_id: c for (f_id, c) in feature_colors.items() if c in available_colors}

                    # loop through these, and calculate the minimum distance from this feature to the nearest
                    # feature with each assigned color
                    for other_feature_id, c in other_features.items():
                        other_geometry = features[other_feature_id].geometry()
                        other_centroid = QgsPointV2(other_geometry.centroid().geometry())

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
