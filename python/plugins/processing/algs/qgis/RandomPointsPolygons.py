# -*- coding: utf-8 -*-

"""
***************************************************************************
    RandomPointsPolygons.py
    ---------------------
    Date                 : April 2014
    Copyright            : (C) 2014 by Alexander Bruy
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
__date__ = 'April 2014'
__copyright__ = '(C) 2014, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import random

from qgis.PyQt.QtGui import QIcon
from qgis.PyQt.QtCore import QVariant
from qgis.core import (QgsField,
                       QgsFeatureSink,
                       QgsFeature,
                       QgsFields,
                       QgsGeometry,
                       QgsPointXY,
                       QgsWkbTypes,
                       QgsSpatialIndex,
                       QgsFeatureRequest,
                       QgsExpression,
                       QgsDistanceArea,
                       QgsProject,
                       QgsProcessing,
                       QgsProcessingException,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingParameterExpression,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterDefinition)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.tools import vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class RandomPointsPolygons(QgisAlgorithm):

    INPUT = 'INPUT'
    EXPRESSION = 'EXPRESSION'
    MIN_DISTANCE = 'MIN_DISTANCE'
    STRATEGY = 'STRATEGY'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'random_points.png'))

    def group(self):
        return self.tr('Vector creation')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.strategies = [self.tr('Points count'),
                           self.tr('Points density')]

        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer'),
                                                              [QgsProcessing.TypeVectorPolygon]))
        self.addParameter(QgsProcessingParameterEnum(self.STRATEGY,
                                                     self.tr('Sampling strategy'),
                                                     self.strategies,
                                                     False,
                                                     0))
        self.addParameter(QgsProcessingParameterExpression(self.EXPRESSION,
                                                           self.tr('Expression'),
                                                           parentLayerParameterName=self.INPUT))
        self.addParameter(QgsProcessingParameterNumber(self.MIN_DISTANCE,
                                                       self.tr('Minimum distance between points'),
                                                       QgsProcessingParameterNumber.Double,
                                                       0, False, 0, 1000000000))
        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT,
                                                            self.tr('Random points'),
                                                            type=QgsProcessing.TypeVectorPoint))

    def name(self):
        return 'randompointsinsidepolygons'

    def displayName(self):
        return self.tr('Random points inside polygons')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        strategy = self.parameterAsEnum(parameters, self.STRATEGY, context)
        minDistance = self.parameterAsDouble(parameters, self.MIN_DISTANCE, context)

        expression = QgsExpression(self.parameterAsString(parameters, self.EXPRESSION, context))
        if expression.hasParserError():
            raise ProcessingException(expression.parserErrorString())

        expressionContext = self.createExpressionContext(parameters, context)
        if not expression.prepare(expressionContext):
            raise ProcessingException(
                self.tr('Evaluation error: {0}').format(expression.evalErrorString()))

        fields = QgsFields()
        fields.append(QgsField('id', QVariant.Int, '', 10, 0))

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, QgsWkbTypes.Point, source.sourceCrs())

        da = QgsDistanceArea()
        da.setSourceCrs(source.sourceCrs())
        da.setEllipsoid(context.project().ellipsoid())

        total = 100.0 / source.featureCount() if source.featureCount() else 0
        for current, f in enumerate(source.getFeatures()):
            if feedback.isCanceled():
                break

            expressionContext.setFeature(f)
            value = expression.evaluate(expressionContext)
            if expression.hasEvalError():
                feedback.pushInfo(
                    self.tr('Evaluation error for feature ID {}: {}').format(f.id(), expression.evalErrorString()))
                continue

            fGeom = f.geometry()
            bbox = fGeom.boundingBox()
            if strategy == 0:
                pointCount = int(value)
            else:
                pointCount = int(round(value * da.measureArea(fGeom)))

            if pointCount == 0:
                feedback.pushInfo("Skip feature {} as number of points for it is 0.")
                continue

            index = QgsSpatialIndex()
            points = dict()

            nPoints = 0
            nIterations = 0
            maxIterations = pointCount * 200
            total = 100.0 / pointCount if pointCount else 1

            random.seed()

            while nIterations < maxIterations and nPoints < pointCount:
                if feedback.isCanceled():
                    break

                rx = bbox.xMinimum() + bbox.width() * random.random()
                ry = bbox.yMinimum() + bbox.height() * random.random()

                p = QgsPointXY(rx, ry)
                geom = QgsGeometry.fromPointXY(p)
                if geom.within(fGeom) and \
                        vector.checkMinDistance(p, index, minDistance, points):
                    f = QgsFeature(nPoints)
                    f.initAttributes(1)
                    f.setFields(fields)
                    f.setAttribute('id', nPoints)
                    f.setGeometry(geom)
                    sink.addFeature(f, QgsFeatureSink.FastInsert)
                    index.insertFeature(f)
                    points[nPoints] = p
                    nPoints += 1
                    feedback.setProgress(int(nPoints * total))
                nIterations += 1

            if nPoints < pointCount:
                feedback.pushInfo(self.tr('Could not generate requested number of random '
                                          'points. Maximum number of attempts exceeded.'))

            feedback.setProgress(0)

        return {self.OUTPUT: dest_id}
