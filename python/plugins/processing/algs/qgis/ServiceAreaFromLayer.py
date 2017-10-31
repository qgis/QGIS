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

from qgis.PyQt.QtCore import QVariant
from qgis.PyQt.QtGui import QIcon

from qgis.core import (QgsWkbTypes,
                       QgsUnitTypes,
                       QgsFeature,
                       QgsFeatureSink,
                       QgsFeatureRequest,
                       QgsGeometry,
                       QgsFields,
                       QgsPointXY,
                       QgsField,
                       QgsProcessing,
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
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'networkanalysis.svg'))

    def group(self):
        return self.tr('Network analysis')

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
                                                       0.0, False, 0, 99999999.99))

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
                                                   5.0, False, 0, 99999999.99))
        params.append(QgsProcessingParameterNumber(self.TOLERANCE,
                                                   self.tr('Topology tolerance'),
                                                   QgsProcessingParameterNumber.Double,
                                                   0.0, False, 0, 99999999.99))

        for p in params:
            p.setFlags(p.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
            self.addParameter(p)

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT,
                                                            self.tr('Service area (boundary nodes)'),
                                                            QgsProcessing.TypeVectorPoint))

    def name(self):
        return 'serviceareafromlayer'

    def displayName(self):
        return self.tr('Service area (from layer)')

    def processAlgorithm(self, parameters, context, feedback):
        network = self.parameterAsSource(parameters, self.INPUT, context)
        startPoints = self.parameterAsSource(parameters, self.START_POINTS, context)
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

        feedback.pushInfo(self.tr('Loading start points...'))
        request = QgsFeatureRequest()
        request.setDestinationCrs(network.sourceCrs())
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

        feedback.pushInfo(self.tr('Building graph...'))
        snappedPoints = director.makeGraph(builder, points, feedback)

        feedback.pushInfo(self.tr('Calculating service areas...'))
        graph = builder.graph()

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, QgsWkbTypes.MultiPoint, network.sourceCrs())

        vertices = []
        upperBoundary = []
        lowerBoundary = []
        total = 100.0 / len(snappedPoints) if snappedPoints else 1
        for i, p in enumerate(snappedPoints):
            if feedback.isCanceled():
                break

            idxStart = graph.findVertex(snappedPoints[i])
            origPoint = points[i].toString()

            tree, cost = QgsGraphAnalyzer.dijkstra(graph, idxStart, 0)
            for j, v in enumerate(cost):
                if v > travelCost and tree[j] != -1:
                    vertexId = graph.edge(tree[j]).fromVertex()
                    if cost[vertexId] <= travelCost:
                        vertices.append(j)

            for j in vertices:
                upperBoundary.append(graph.vertex(graph.edge(tree[j]).toVertex()).point())
                lowerBoundary.append(graph.vertex(graph.edge(tree[j]).fromVertex()).point())

            geomUpper = QgsGeometry.fromMultiPointXY(upperBoundary)
            geomLower = QgsGeometry.fromMultiPointXY(lowerBoundary)

            feat.setGeometry(geomUpper)

            attrs = source_attributes[i]
            attrs.extend(['upper', origPoint])
            feat.setAttributes(attrs)
            sink.addFeature(feat, QgsFeatureSink.FastInsert)

            feat.setGeometry(geomLower)
            attrs[-2] = 'lower'
            feat.setAttributes(attrs)
            sink.addFeature(feat, QgsFeatureSink.FastInsert)

            vertices[:] = []
            upperBoundary[:] = []
            lowerBoundary[:] = []

            feedback.setProgress(int(i * total))

        return {self.OUTPUT: dest_id}
