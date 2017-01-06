# -*- coding: utf-8 -*-

"""
***************************************************************************
    ServiceAreaFromPoint.py
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

from qgis.core import QgsWkbTypes, QgsUnitTypes, QgsFeature, QgsGeometry, QgsPoint, QgsField, QgsFields
from qgis.analysis import (QgsVectorLayerDirector,
                           QgsNetworkDistanceStrategy,
                           QgsNetworkSpeedStrategy,
                           QgsGraphBuilder,
                           QgsGraphAnalyzer
                           )
from qgis.utils import iface

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import (ParameterVector,
                                        ParameterPoint,
                                        ParameterNumber,
                                        ParameterString,
                                        ParameterTableField,
                                        ParameterSelection
                                        )
from processing.core.outputs import (OutputNumber,
                                     OutputVector
                                     )
from processing.tools import dataobjects

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class ServiceAreaFromPoint(GeoAlgorithm):

    INPUT_VECTOR = 'INPUT_VECTOR'
    START_POINT = 'START_POINT'
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
    OUTPUT_POINTS = 'OUTPUT_POINTS'
    OUTPUT_POLYGON = 'OUTPUT_POLYGON'

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'networkanalysis.svg'))

    def defineCharacteristics(self):
        self.DIRECTIONS = OrderedDict([
            (self.tr('Forward direction'), QgsVectorLayerDirector.DirectionForward),
            (self.tr('Backward direction'), QgsVectorLayerDirector.DirectionForward),
            (self.tr('Both directions'), QgsVectorLayerDirector.DirectionForward)])

        self.STRATEGIES = [self.tr('Shortest'),
                           self.tr('Fastest')
                           ]

        self.name, self.i18n_name = self.trAlgorithm('Service area (from point)')
        self.group, self.i18n_group = self.trAlgorithm('Network analysis')

        self.addParameter(ParameterVector(self.INPUT_VECTOR,
                                          self.tr('Vector layer representing network'),
                                          [dataobjects.TYPE_VECTOR_LINE]))
        self.addParameter(ParameterPoint(self.START_POINT,
                                         self.tr('Start point')))
        self.addParameter(ParameterSelection(self.STRATEGY,
                                             self.tr('Path type to calculate'),
                                             self.STRATEGIES,
                                             default=0))
        self.addParameter(ParameterNumber(self.TRAVEL_COST,
                                          self.tr('Travel cost (distance for "Shortest", time for "Fastest")'),
                                          0.0, 99999999.999999, 0.0))

        params = []
        params.append(ParameterTableField(self.DIRECTION_FIELD,
                                          self.tr('Direction field'),
                                          self.INPUT_VECTOR,
                                          optional=True))
        params.append(ParameterString(self.VALUE_FORWARD,
                                      self.tr('Value for forward direction'),
                                      '',
                                      optional=True))
        params.append(ParameterString(self.VALUE_BACKWARD,
                                      self.tr('Value for backward direction'),
                                      '',
                                      optional=True))
        params.append(ParameterString(self.VALUE_BOTH,
                                      self.tr('Value for both directions'),
                                      '',
                                      optional=True))
        params.append(ParameterSelection(self.DEFAULT_DIRECTION,
                                         self.tr('Default direction'),
                                         list(self.DIRECTIONS.keys()),
                                         default=2))
        params.append(ParameterTableField(self.SPEED_FIELD,
                                          self.tr('Speed field'),
                                          self.INPUT_VECTOR,
                                          optional=True))
        params.append(ParameterNumber(self.DEFAULT_SPEED,
                                      self.tr('Default speed (km/h)'),
                                      0.0, 99999999.999999, 5.0))
        params.append(ParameterNumber(self.TOLERANCE,
                                      self.tr('Topology tolerance'),
                                      0.0, 99999999.999999, 0.0))

        for p in params:
            p.isAdvanced = True
            self.addParameter(p)

        self.addOutput(OutputVector(self.OUTPUT_POINTS,
                                    self.tr('Service area (boundary nodes)'),
                                    datatype=[dataobjects.TYPE_VECTOR_POINT]))
        self.addOutput(OutputVector(self.OUTPUT_POLYGON,
                                    self.tr('Service area (convex hull)'),
                                    datatype=[dataobjects.TYPE_VECTOR_POLYGON]))

    def processAlgorithm(self, feedback):
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT_VECTOR))
        startPoint = self.getParameterValue(self.START_POINT)
        strategy = self.getParameterValue(self.STRATEGY)
        travelCost = self.getParameterValue(self.TRAVEL_COST)

        directionFieldName = self.getParameterValue(self.DIRECTION_FIELD)
        forwardValue = self.getParameterValue(self.VALUE_FORWARD)
        backwardValue = self.getParameterValue(self.VALUE_BACKWARD)
        bothValue = self.getParameterValue(self.VALUE_BOTH)
        defaultDirection = self.getParameterValue(self.DEFAULT_DIRECTION)
        bothValue = self.getParameterValue(self.VALUE_BOTH)
        defaultDirection = self.getParameterValue(self.DEFAULT_DIRECTION)
        speedFieldName = self.getParameterValue(self.SPEED_FIELD)
        defaultSpeed = self.getParameterValue(self.DEFAULT_SPEED)
        tolerance = self.getParameterValue(self.TOLERANCE)

        tmp = startPoint.split(',')
        startPoint = QgsPoint(float(tmp[0]), float(tmp[1]))

        directionField = -1
        if directionFieldName is not None:
            directionField = layer.fields().lookupField(directionFieldName)
        speedField = -1
        if speedFieldName is not None:
            speedField = layer.fields().lookupField(speedFieldName)

        director = QgsVectorLayerDirector(layer,
                                          directionField,
                                          forwardValue,
                                          backwardValue,
                                          bothValue,
                                          defaultDirection)

        distUnit = iface.mapCanvas().mapSettings().destinationCrs().mapUnits()
        multiplier = QgsUnitTypes.fromUnitToUnitFactor(distUnit, QgsUnitTypes.DistanceMeters)
        if strategy == 0:
            strategy = QgsNetworkDistanceStrategy()
        else:
            strategy = QgsNetworkSpeedStrategy(speedField,
                                               defaultSpeed,
                                               multiplier * 1000.0 / 3600.0)

        director.addStrategy(strategy)
        builder = QgsGraphBuilder(iface.mapCanvas().mapSettings().destinationCrs(),
                                  iface.mapCanvas().hasCrsTransformEnabled(),
                                  tolerance)
        feedback.pushInfo(self.tr('Building graph...'))
        snappedPoints = director.makeGraph(builder, [startPoint])

        feedback.pushInfo(self.tr('Calculating service area...'))
        graph = builder.graph()
        idxStart = graph.findVertex(snappedPoints[0])

        tree, cost = QgsGraphAnalyzer.dijkstra(graph, idxStart, 0)
        vertices = []
        for i, v in enumerate(cost):
            if v > travelCost and tree[i] != -1:
                vertexId = graph.edge(tree[i]).outVertex()
                if cost[vertexId] <= travelCost:
                    vertices.append(i)

        upperBoundary = []
        lowerBoundary = []
        for i in vertices:
            upperBoundary.append(graph.vertex(graph.edge(tree[i]).inVertex()).point())
            lowerBoundary.append(graph.vertex(graph.edge(tree[i]).outVertex()).point())

        feedback.pushInfo(self.tr('Writing results...'))

        fields = QgsFields()
        fields.append(QgsField('type', QVariant.String, '', 254, 0))
        fields.append(QgsField('start', QVariant.String, '', 254, 0))

        feat = QgsFeature()
        feat.setFields(fields)

        geomUpper = QgsGeometry.fromMultiPoint(upperBoundary)
        geomLower = QgsGeometry.fromMultiPoint(lowerBoundary)

        writer = self.getOutputFromName(
            self.OUTPUT_POINTS).getVectorWriter(
                fields,
                QgsWkbTypes.MultiPoint,
                layer.crs())

        feat.setGeometry(geomUpper)
        feat['type'] = 'upper'
        feat['start'] = startPoint.toString()
        writer.addFeature(feat)

        feat.setGeometry(geomLower)
        feat['type'] = 'lower'
        feat['start'] = startPoint.toString()
        writer.addFeature(feat)

        del writer

        upperBoundary.append(startPoint)
        lowerBoundary.append(startPoint)
        geomUpper = QgsGeometry.fromMultiPoint(upperBoundary)
        geomLower = QgsGeometry.fromMultiPoint(lowerBoundary)

        writer = self.getOutputFromName(
            self.OUTPUT_POLYGON).getVectorWriter(
                fields,
                QgsWkbTypes.Polygon,
                layer.crs())

        geom = geomUpper.convexHull()
        feat.setGeometry(geom)
        feat['type'] = 'upper'
        feat['start'] = startPoint.toString()
        writer.addFeature(feat)

        geom = geomLower.convexHull()
        feat.setGeometry(geom)
        feat['type'] = 'lower'
        feat['start'] = startPoint.toString()
        writer.addFeature(feat)
        del writer
