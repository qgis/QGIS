# -*- coding: utf-8 -*-

"""
***************************************************************************
    RandomPointsOnLines.py
    ----------------------------
    Date                 : February 2020
    Copyright            : (C) 2020 by Håvard Tveite
    Email                : havard dot tveite at nmbu dot no
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
    Starting point: RandomPointsAlongLines.py (C) 2014 by Alexander Bruy
"""

__author__ = 'Håvard Tveite'
__date__ = 'February 2020'
__copyright__ = '(C) 2020, Håvard Tveite'

import random

from qgis.PyQt.QtCore import QVariant
from qgis.core import (QgsFeature,
                       QgsFeatureSink,
                       QgsField,
                       QgsFields,
                       QgsGeometry,
                       QgsPointXY,
                       QgsProcessing,
                       QgsProcessingException,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterDistance,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingOutputNumber,
                       QgsSpatialIndex,
                       QgsWkbTypes)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.tools import vector


class RandomPointsOnLines(QgisAlgorithm):

    INPUT = 'INPUT'
    POINTS_NUMBER = 'POINTS_NUMBER'
    MIN_DISTANCE = 'MIN_DISTANCE'
    OUTPUT = 'OUTPUT'
    MAXTRIESPERPOINT = 'MAX_TRIES_PER_POINT'
    OUTPUT_POINTS = 'POINTS_GENERATED'
    MISSED_POINTS = 'POINTS_MISSED'
    MISSED_LINES = 'LINES_WITH_MISSED_POINTS'
    EMPTY_OR_NO_GEOM = 'LINES_WITH_EMPTY_OR_NO_GEOMETRY'
    RANDOM_SEED = 'SEED'
    INCLUDE_LINE_ATTRIBUTES = 'INCLUDE_LINE_ATTRIBUTES'
    POINT_ID_FIELD_NAME = 'point_num'

    def name(self):
        return 'randompointsonlines'

    def displayName(self):
        return self.tr('Random points on lines')

    def group(self):
        return self.tr('Vector creation')

    def groupId(self):
        return 'vectorcreation'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer'),
                                                              [QgsProcessing.TypeVectorLine]))
        self.addParameter(QgsProcessingParameterNumber(self.POINTS_NUMBER,
                                                       self.tr('Number of points on each feature'),
                                                       QgsProcessingParameterNumber.Integer,
                                                       1, False, 1, 1000000000))
        self.addParameter(QgsProcessingParameterDistance(self.MIN_DISTANCE,
                                                         self.tr('Minimum distance between points'),
                                                         0, self.INPUT, False, 0, 1000000000))
        self.addParameter(QgsProcessingParameterNumber(self.MAXTRIESPERPOINT,
                                                       self.tr('Maximum number of attempts per point'),
                                                       QgsProcessingParameterNumber.Integer,
                                                       10, False, 1, 1000))
        self.addParameter(QgsProcessingParameterNumber(self.RANDOM_SEED,
                                                       self.tr('Random number seed'),
                                                       QgsProcessingParameterNumber.Integer,
                                                       None, True, 1, 1000000000))
        self.addParameter(QgsProcessingParameterBoolean(self.INCLUDE_LINE_ATTRIBUTES,
                                                        self.tr('Include line attributes'),
                                                        True, True))
        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT,
                                                            self.tr('Random points'),
                                                            type=QgsProcessing.TypeVectorPoint))
        self.addOutput(QgsProcessingOutputNumber(self.OUTPUT_POINTS,
                                                 self.tr('Total number of points generated')
                                                 ))
        self.addOutput(QgsProcessingOutputNumber(self.MISSED_POINTS,
                                                 self.tr('Number of missed points')
                                                 ))
        self.addOutput(QgsProcessingOutputNumber(self.MISSED_LINES,
                                                 self.tr('Number of lines with missed points')
                                                 ))
        self.addOutput(QgsProcessingOutputNumber(self.EMPTY_OR_NO_GEOM,
                                                 self.tr('Number of features with empty or no geometry')
                                                 ))

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        if source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters,
                                                                 self.INPUT))
        pointCount = self.parameterAsInt(parameters, self.POINTS_NUMBER,
                                         context)
        minDistance = self.parameterAsDouble(parameters, self.MIN_DISTANCE,
                                             context)
        randSeed = self.parameterAsInt(parameters, self.RANDOM_SEED, context)
        maxTriesPerPoint = self.parameterAsInt(parameters,
                                               self.MAXTRIESPERPOINT,
                                               context)
        includelineattr = self.parameterAsBoolean(parameters,
                                                  self.INCLUDE_LINE_ATTRIBUTES,
                                                  context)
        fields = QgsFields()
        if includelineattr:
            fields = source.fields()
        fields.append(QgsField(self.POINT_ID_FIELD_NAME,
                               QVariant.Int, '', 10, 0))
        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT,
                                               context, fields,
                                               QgsWkbTypes.Point,
                                               source.sourceCrs(),
                                               QgsFeatureSink.RegeneratePrimaryKey)
        if sink is None:
            raise QgsProcessingException(
                self.invalidSinkError(parameters, self.OUTPUT))
        totNPoints = 0  # Total number of points generated
        missedPoints = 0  # Number of misses (too close)
        missedLines = 0  # Number of features with missed points
        emptyOrNullGeom = 0  # Number of features with empty or no geometry
        featureCount = source.featureCount()
        total = 100.0 / (pointCount * featureCount) if (pointCount and
                                                        featureCount) else 1
        if randSeed:
            random.seed(randSeed)
        else:
            random.seed()
        index = QgsSpatialIndex()
        points = dict()
        # Go through all the features of the layer
        for feat in source.getFeatures():
            if feedback.isCanceled():
                break
            fGeom = feat.geometry()  # get the geometry
            if fGeom is None:
                emptyOrNullGeom += 1
                feedback.pushInfo('Null geometry - skipping!')
                continue
            if fGeom.isEmpty():
                emptyOrNullGeom += 1
                feedback.pushInfo('Empty geometry - skipping!')
                continue
            totLineLength = fGeom.length()

            # Generate points on the (multi)line geometry
            nPoints = 0  # Number of points generated for this geometry
            nIterations = 0  # number of attempts for this geometry
            while nIterations < maxTriesPerPoint and nPoints < pointCount:
                # Try to generate a point
                if feedback.isCanceled():
                    break
                # Get the random "position" along the line for this point
                randomLength = random.random() * totLineLength
                # Get the point on the line
                randPoint = fGeom.interpolate(randomLength)
                # vector.checkMinDistance requires PointXY:
                randPointXY = randPoint.asPoint()
                if (minDistance == 0 or
                    vector.checkMinDistance(randPointXY, index, minDistance,
                                            points)):
                    f = QgsFeature(totNPoints)
                    f.setFields(fields)
                    attrs = []
                    if includelineattr:
                        attrs.extend(feat.attributes())
                    attrs.append(totNPoints)
                    f.setAttributes(attrs)
                    f.setGeometry(randPoint)
                    sink.addFeature(f, QgsFeatureSink.FastInsert)
                    index.addFeature(f)
                    points[totNPoints] = randPointXY
                    nPoints += 1
                    totNPoints += 1
                    feedback.setProgress(int((totNPoints + missedPoints) *
                                             total))
                nIterations += 1
            if nPoints < pointCount:
                missedPoints += pointCount - nPoints
                missedLines += 1
        if totNPoints < pointCount * featureCount:
            feedback.pushInfo(str(totNPoints) + ' (out of ' +
                              str(pointCount * featureCount) +
                              ') requested points were generated.')
        if emptyOrNullGeom > 0:
            feedback.pushInfo('Number of features with empty or no geometry: '
                              + str(emptyOrNullGeom))
        return {self.OUTPUT: dest_id, self.OUTPUT_POINTS: totNPoints,
                self.MISSED_POINTS: missedPoints,
                self.MISSED_LINES: missedLines,
                self.EMPTY_OR_NO_GEOM: emptyOrNullGeom}
