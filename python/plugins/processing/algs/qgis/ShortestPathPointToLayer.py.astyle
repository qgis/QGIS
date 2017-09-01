# -*- coding: utf-8 -*-

"""
***************************************************************************
    ShortestPathPointToLayer.py
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
                       QgsGeometry,
                       QgsFields,
                       QgsField,
                       QgsMessageLog,
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


class ShortestPathPointToLayer(QgisAlgorithm):

    INPUT = 'INPUT'
    START_POINT = 'START_POINT'
    END_POINTS = 'END_POINTS'
    STRATEGY = 'STRATEGY'
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
            (self.tr('Backward direction'), QgsVectorLayerDirector.DirectionForward),
            (self.tr('Both directions'), QgsVectorLayerDirector.DirectionForward)])

        self.STRATEGIES = [self.tr('Shortest'),
                           self.tr('Fastest')
                           ]

        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Vector layer representing network'),
                                                              [QgsProcessing.TypeVectorLine]))
        self.addParameter(QgsProcessingParameterPoint(self.START_POINT,
                                                      self.tr('Start point')))
        self.addParameter(QgsProcessingParameterFeatureSource(self.END_POINTS,
                                                              self.tr('Vector layer with end points'),
                                                              [QgsProcessing.TypeVectorPoint]))
        self.addParameter(QgsProcessingParameterEnum(self.STRATEGY,
                                                     self.tr('Path type to calculate'),
                                                     self.STRATEGIES,
                                                     defaultValue=0))

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
                                                            self.tr('Shortest path'),
                                                            QgsProcessing.TypeVectorLine))

    def name(self):
        return 'shortestpathpointtolayer'

    def displayName(self):
        return self.tr('Shortest path (point to layer)')

    def processAlgorithm(self, parameters, context, feedback):
        network = self.parameterAsSource(parameters, self.INPUT, context)
        startPoint = self.parameterAsPoint(parameters, self.START_POINT, context)
        endPoints = self.parameterAsSource(parameters, self.END_POINTS, context)
        strategy = self.parameterAsEnum(parameters, self.STRATEGY, context)

        directionFieldName = self.parameterAsString(parameters, self.DIRECTION_FIELD, context)
        forwardValue = self.parameterAsString(parameters, self.VALUE_FORWARD, context)
        backwardValue = self.parameterAsString(parameters, self.VALUE_BACKWARD, context)
        bothValue = self.parameterAsString(parameters, self.VALUE_BOTH, context)
        defaultDirection = self.parameterAsEnum(parameters, self.DEFAULT_DIRECTION, context)
        speedFieldName = self.parameterAsString(parameters, self.SPEED_FIELD, context)
        defaultSpeed = self.parameterAsDouble(parameters, self.DEFAULT_SPEED, context)
        tolerance = self.parameterAsDouble(parameters, self.TOLERANCE, context)

        fields = QgsFields()
        fields.append(QgsField('start', QVariant.String, '', 254, 0))
        fields.append(QgsField('end', QVariant.String, '', 254, 0))
        fields.append(QgsField('cost', QVariant.Double, '', 20, 7))

        feat = QgsFeature()
        feat.setFields(fields)

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, QgsWkbTypes.LineString, network.sourceCrs())

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
            multiplier = 3600

        director.addStrategy(strategy)
        builder = QgsGraphBuilder(context.project().crs(),
                                  True,
                                  tolerance)

        feedback.pushInfo(self.tr('Loading end points...'))
        request = QgsFeatureRequest()
        request.setFlags(request.flags() ^ QgsFeatureRequest.SubsetOfAttributes)
        request.setDestinationCrs(network.sourceCrs())
        features = endPoints.getFeatures(request)
        total = 100.0 / endPoints.featureCount() if endPoints.featureCount() else 0

        points = [startPoint]
        for current, f in enumerate(features):
            if feedback.isCanceled():
                break

            points.append(f.geometry().asPoint())
            feedback.setProgress(int(current * total))

        feedback.pushInfo(self.tr('Building graph...'))
        snappedPoints = director.makeGraph(builder, points, feedback)

        feedback.pushInfo(self.tr('Calculating shortest paths...'))
        graph = builder.graph()

        idxStart = graph.findVertex(snappedPoints[0])
        tree, cost = QgsGraphAnalyzer.dijkstra(graph, idxStart, 0)
        route = []

        nPoints = len(snappedPoints)
        total = 100.0 / nPoints if nPoints else 1
        for i in range(1, count + 1):
            if feedback.isCanceled():
                break

            idxEnd = graph.findVertex(snappedPoints[i])

            if tree[idxEnd] == -1:
                msg = self.tr('There is no route from start point ({}) to end point ({}).'.format(startPoint.toString(), points[i].toString()))
                feedback.setProgressText(msg)
                QgsMessageLog.logMessage(msg, self.tr('Processing'), QgsMessageLog.WARNING)
                continue

            cost = 0.0
            current = idxEnd
            while current != idxStart:
                cost += graph.edge(tree[current]).cost(0)
                route.append(graph.vertex(graph.edge(tree[current]).inVertex()).point())
                current = graph.edge(tree[current]).outVertex()

            route.append(snappedPoints[0])
            route.reverse()

            geom = QgsGeometry.fromPolyline(route)
            feat.setGeometry(geom)
            feat['start'] = startPoint.toString()
            feat['end'] = points[i].toString()
            feat['cost'] = cost / multiplier
            sink.addFeature(feat, QgsFeatureSink.FastInsert)

            route[:] = []

            feedback.setProgress(int(i * total))

        return {self.OUTPUT: dest_id}
