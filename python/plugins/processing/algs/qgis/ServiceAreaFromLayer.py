# -*- coding: utf-8 -*-

"""
***************************************************************************
    ServiceAreaFromLayer.py
    ---------------------
    Date                 : December 2016
    Copyright            : (C) 2016 by Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Alexander Bruy'
__date__ = 'December 2016'
__copyright__ = '(C) 2016, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
from collections import OrderedDict

from qgis.PyQt.QtCore import QVariant, QCoreApplication
from qgis.PyQt.QtGui import QIcon

from qgis.core import (QgsWkbTypes,
                       QgsUnitTypes,
                       QgsFeature,
                       QgsFeatureSink,
                       QgsFeatureRequest,
                       QgsGeometry,
                       QgsGeometryUtils,
                       QgsFields,
                       QgsPointXY,
                       QgsField,
                       QgsProcessing,
                       QgsProcessingException,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterDistance,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterPoint,
                       QgsProcessingParameterField,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterString,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingParameterDefinition)
from qgis.analysis import (QgsVectorLayerDirector,
                           QgsNetworkDistanceStrategy,
                           QgsNetworkSpeedStrategy,
                           QgsGraphBuilder,
                           QgsGraphAnalyzer
                           )

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class ServiceAreaFromLayer(QgisAlgorithm):

    INPUT = 'INPUT'
    START_POINTS = 'START_POINTS'
    STRATEGY = 'STRATEGY'
    TRAVEL_COST = 'TRAVEL_COST'
    DIRECTION_FIELD = 'DIRECTION_FIELD'
    VALUE_FORWARD = 'VALUE_FORWARD'
    VALUE_BACKWARD = 'VALUE_BACKWARD'
    VALUE_BOTH = 'VALUE_BOTH'
    DEFAULT_DIRECTION = 'DEFAULT_DIRECTION'
    SPEED_FIELD = 'SPEED_FIELD'
    DEFAULT_SPEED = 'DEFAULT_SPEED'
    TOLERANCE = 'TOLERANCE'
    INCLUDE_BOUNDS = 'INCLUDE_BOUNDS'
    OUTPUT = 'OUTPUT'
    OUTPUT_LINES = 'OUTPUT_LINES'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'networkanalysis.svg'))

    def group(self):
        return self.tr('Network analysis')

    def groupId(self):
        return 'networkanalysis'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.DIRECTIONS = OrderedDict([
            (self.tr('Forward direction'), QgsVectorLayerDirector.DirectionForward),
            (self.tr('Backward direction'), QgsVectorLayerDirector.DirectionBackward),
            (self.tr('Both directions'), QgsVectorLayerDirector.DirectionBoth)])

        self.STRATEGIES = [self.tr('Shortest'),
                           self.tr('Fastest')
                           ]

        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Vector layer representing network'),
                                                              [QgsProcessing.TypeVectorLine]))
        self.addParameter(QgsProcessingParameterFeatureSource(self.START_POINTS,
                                                              self.tr('Vector layer with start points'),
                                                              [QgsProcessing.TypeVectorPoint]))
        self.addParameter(QgsProcessingParameterEnum(self.STRATEGY,
                                                     self.tr('Path type to calculate'),
                                                     self.STRATEGIES,
                                                     defaultValue=0))
        self.addParameter(QgsProcessingParameterNumber(self.TRAVEL_COST,
                                                       self.tr('Travel cost (distance for "Shortest", time for "Fastest")'),
                                                       QgsProcessingParameterNumber.Double,
                                                       0.0, False, 0))

        params = []
        params.append(QgsProcessingParameterField(self.DIRECTION_FIELD,
                                                  self.tr('Direction field'),
                                                  None,
                                                  self.INPUT,
                                                  optional=True))
        params.append(QgsProcessingParameterString(self.VALUE_FORWARD,
                                                   self.tr('Value for forward direction'),
                                                   optional=True))
        params.append(QgsProcessingParameterString(self.VALUE_BACKWARD,
                                                   self.tr('Value for backward direction'),
                                                   optional=True))
        params.append(QgsProcessingParameterString(self.VALUE_BOTH,
                                                   self.tr('Value for both directions'),
                                                   optional=True))
        params.append(QgsProcessingParameterEnum(self.DEFAULT_DIRECTION,
                                                 self.tr('Default direction'),
                                                 list(self.DIRECTIONS.keys()),
                                                 defaultValue=2))
        params.append(QgsProcessingParameterField(self.SPEED_FIELD,
                                                  self.tr('Speed field'),
                                                  None,
                                                  self.INPUT,
                                                  optional=True))
        params.append(QgsProcessingParameterNumber(self.DEFAULT_SPEED,
                                                   self.tr('Default speed (km/h)'),
                                                   QgsProcessingParameterNumber.Double,
                                                   5.0, False, 0))
        params.append(QgsProcessingParameterDistance(self.TOLERANCE,
                                                     self.tr('Topology tolerance'),
                                                     0.0, self.INPUT, False, 0))
        params.append(QgsProcessingParameterBoolean(self.INCLUDE_BOUNDS,
                                                    self.tr('Include upper/lower bound points'),
                                                    defaultValue=False))
        for p in params:
            p.setFlags(p.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
            self.addParameter(p)

        lines_output = QgsProcessingParameterFeatureSink(self.OUTPUT_LINES,
                                                         self.tr('Service area (lines)'),
                                                         QgsProcessing.TypeVectorLine, optional=True)
        lines_output.setCreateByDefault(True)
        self.addParameter(lines_output)

        nodes_output = QgsProcessingParameterFeatureSink(self.OUTPUT,
                                                         self.tr('Service area (boundary nodes)'),
                                                         QgsProcessing.TypeVectorPoint, optional=True)
        nodes_output.setCreateByDefault(False)
        self.addParameter(nodes_output)

    def name(self):
        return 'serviceareafromlayer'

    def displayName(self):
        return self.tr('Service area (from layer)')

    def processAlgorithm(self, parameters, context, feedback):
        network = self.parameterAsSource(parameters, self.INPUT, context)
        if network is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.INPUT))

        startPoints = self.parameterAsSource(parameters, self.START_POINTS, context)
        if startPoints is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.START_POINTS))

        strategy = self.parameterAsEnum(parameters, self.STRATEGY, context)
        travelCost = self.parameterAsDouble(parameters, self.TRAVEL_COST, context)

        directionFieldName = self.parameterAsString(parameters, self.DIRECTION_FIELD, context)
        forwardValue = self.parameterAsString(parameters, self.VALUE_FORWARD, context)
        backwardValue = self.parameterAsString(parameters, self.VALUE_BACKWARD, context)
        bothValue = self.parameterAsString(parameters, self.VALUE_BOTH, context)
        defaultDirection = self.parameterAsEnum(parameters, self.DEFAULT_DIRECTION, context)
        speedFieldName = self.parameterAsString(parameters, self.SPEED_FIELD, context)
        defaultSpeed = self.parameterAsDouble(parameters, self.DEFAULT_SPEED, context)
        tolerance = self.parameterAsDouble(parameters, self.TOLERANCE, context)

        include_bounds = True # default to true to maintain 3.0 API
        if self.INCLUDE_BOUNDS in parameters:
            include_bounds = self.parameterAsBool(parameters, self.INCLUDE_BOUNDS, context)

        fields = startPoints.fields()
        fields.append(QgsField('type', QVariant.String, '', 254, 0))
        fields.append(QgsField('start', QVariant.String, '', 254, 0))

        feat = QgsFeature()
        feat.setFields(fields)

        directionField = -1
        if directionFieldName:
            directionField = network.fields().lookupField(directionFieldName)
        speedField = -1
        if speedFieldName:
            speedField = network.fields().lookupField(speedFieldName)

        director = QgsVectorLayerDirector(network,
                                          directionField,
                                          forwardValue,
                                          backwardValue,
                                          bothValue,
                                          defaultDirection)

        distUnit = context.project().crs().mapUnits()
        multiplier = QgsUnitTypes.fromUnitToUnitFactor(distUnit, QgsUnitTypes.DistanceMeters)
        if strategy == 0:
            strategy = QgsNetworkDistanceStrategy()
        else:
            strategy = QgsNetworkSpeedStrategy(speedField,
                                               defaultSpeed,
                                               multiplier * 1000.0 / 3600.0)

        director.addStrategy(strategy)
        builder = QgsGraphBuilder(network.sourceCrs(),
                                  True,
                                  tolerance)

        feedback.pushInfo(QCoreApplication.translate('ServiceAreaFromLayer', 'Loading start points…'))
        request = QgsFeatureRequest()
        request.setDestinationCrs(network.sourceCrs(), context.transformContext())
        features = startPoints.getFeatures(request)
        total = 100.0 / startPoints.featureCount() if startPoints.featureCount() else 0

        points = []
        source_attributes = {}
        i = 0
        for current, f in enumerate(features):
            if feedback.isCanceled():
                break

            if not f.hasGeometry():
                continue

            for p in f.geometry().vertices():
                points.append(QgsPointXY(p))
                source_attributes[i] = f.attributes()
                i += 1

            feedback.setProgress(int(current * total))

        feedback.pushInfo(QCoreApplication.translate('ServiceAreaFromLayer', 'Building graph…'))
        snappedPoints = director.makeGraph(builder, points, feedback)

        feedback.pushInfo(QCoreApplication.translate('ServiceAreaFromLayer', 'Calculating service areas…'))
        graph = builder.graph()

        (point_sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                                     fields, QgsWkbTypes.MultiPoint, network.sourceCrs())
        (line_sink, line_dest_id) = self.parameterAsSink(parameters, self.OUTPUT_LINES, context,
                                                         fields, QgsWkbTypes.MultiLineString, network.sourceCrs())

        total = 100.0 / len(snappedPoints) if snappedPoints else 1
        for i, p in enumerate(snappedPoints):
            if feedback.isCanceled():
                break

            idxStart = graph.findVertex(snappedPoints[i])
            origPoint = points[i].toString()

            tree, cost = QgsGraphAnalyzer.dijkstra(graph, idxStart, 0)

            vertices = set()
            area_points = []
            lines = []
            for vertex, start_vertex_cost in enumerate(cost):
                inbound_edge_index = tree[vertex]
                if inbound_edge_index == -1 and vertex != idxStart:
                    # unreachable vertex
                    continue

                if start_vertex_cost > travelCost:
                    # vertex is too expensive, discard
                    continue

                vertices.add(vertex)
                start_point = graph.vertex(vertex).point()

                # find all edges coming from this vertex
                for edge_id in graph.vertex(vertex).outgoingEdges():
                    edge = graph.edge(edge_id)
                    end_vertex_cost = start_vertex_cost + edge.cost(0)
                    end_point = graph.vertex(edge.toVertex()).point()
                    if end_vertex_cost <= travelCost:
                        # end vertex is cheap enough to include
                        vertices.add(edge.toVertex())
                        lines.append([start_point, end_point])
                    else:
                        # travelCost sits somewhere on this edge, interpolate position
                        interpolated_end_point = QgsGeometryUtils.interpolatePointOnLineByValue(start_point.x(), start_point.y(), start_vertex_cost,
                                                                                                end_point.x(), end_point.y(), end_vertex_cost, travelCost)
                        area_points.append(interpolated_end_point)
                        lines.append([start_point, interpolated_end_point])

            for v in vertices:
                area_points.append(graph.vertex(v).point())

            feat = QgsFeature()
            if point_sink is not None:
                geomPoints = QgsGeometry.fromMultiPointXY(area_points)
                feat.setGeometry(geomPoints)
                attrs = source_attributes[i]
                attrs.extend(['within', origPoint])
                feat.setAttributes(attrs)
                point_sink.addFeature(feat, QgsFeatureSink.FastInsert)

                if include_bounds:
                    upperBoundary = []
                    lowerBoundary = []

                    vertices = []
                    for vertex, c in enumerate(cost):
                        if c > travelCost and tree[vertex] != -1:
                            vertexId = graph.edge(tree[vertex]).fromVertex()
                            if cost[vertexId] <= travelCost:
                                vertices.append(vertex)

                    for v in vertices:
                        upperBoundary.append(graph.vertex(graph.edge(tree[v]).toVertex()).point())
                        lowerBoundary.append(graph.vertex(graph.edge(tree[v]).fromVertex()).point())

                    geomUpper = QgsGeometry.fromMultiPointXY(upperBoundary)
                    geomLower = QgsGeometry.fromMultiPointXY(lowerBoundary)

                    feat.setGeometry(geomUpper)
                    attrs[-2] = 'upper'
                    feat.setAttributes(attrs)
                    point_sink.addFeature(feat, QgsFeatureSink.FastInsert)

                    feat.setGeometry(geomLower)
                    attrs[-2] = 'lower'
                    feat.setAttributes(attrs)
                    point_sink.addFeature(feat, QgsFeatureSink.FastInsert)

            if line_sink is not None:
                geom_lines = QgsGeometry.fromMultiPolylineXY(lines)
                feat.setGeometry(geom_lines)
                attrs = source_attributes[i]
                attrs.extend(['lines', origPoint])
                feat.setAttributes(attrs)
                line_sink.addFeature(feat, QgsFeatureSink.FastInsert)

            feedback.setProgress(int(i * total))

        results = {}
        if point_sink is not None:
            results[self.OUTPUT] = dest_id
        if line_sink is not None:
            results[self.OUTPUT_LINES] = line_dest_id
        return results
