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
from qgis.core import (QgsApplication,
                       QgsField,
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
                       QgsProcessingParameterDistance,
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
        return QgsApplication.getThemeIcon("/algorithms/mAlgorithmRandomPointsWithinPolygon.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("/algorithms/mAlgorithmRandomPointsWithinPolygon.svg")

    def group(self):
        return self.tr('Vector creation')

    def groupId(self):
        return 'vectorcreation'

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
        self.addParameter(QgsProcessingParameterDistance(self.MIN_DISTANCE,
                                                         self.tr('Minimum distance between points'),
                                                         0, self.INPUT, False, 0, 1000000000))
        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT,
                                                            self.tr('Random points'),
                                                            type=QgsProcessing.TypeVectorPoint))

    def name(self):
        return 'randompointsinsidepolygons'

    def displayName(self):
        return self.tr('Random points inside polygons')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        if source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.INPUT))

        strategy = self.parameterAsEnum(parameters, self.STRATEGY, context)
        minDistance = self.parameterAsDouble(parameters, self.MIN_DISTANCE, context)

        expression = QgsExpression(self.parameterAsString(parameters, self.EXPRESSION, context))
        if expression.hasParserError():
            raise QgsProcessingException(expression.parserErrorString())

        expressionContext = self.createExpressionContext(parameters, context, source)
        expression.prepare(expressionContext)

        fields = QgsFields()
        fields.append(QgsField('id', QVariant.Int, '', 10, 0))

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, QgsWkbTypes.Point, source.sourceCrs())
        if sink is None:
            raise QgsProcessingException(self.invalidSinkError(parameters, self.OUTPUT))

        da = QgsDistanceArea()
        da.setSourceCrs(source.sourceCrs(), context.transformContext())
        da.setEllipsoid(context.project().ellipsoid())

        total = 100.0 / source.featureCount() if source.featureCount() else 0
        current_progress = 0
        for current, f in enumerate(source.getFeatures()):
            if feedback.isCanceled():
                break

            if not f.hasGeometry():
                continue

            current_progress = total * current
            feedback.setProgress(current_progress)

            expressionContext.setFeature(f)
            value = expression.evaluate(expressionContext)
            if expression.hasEvalError():
                feedback.pushInfo(
                    self.tr('Evaluation error for feature ID {}: {}').format(f.id(), expression.evalErrorString()))
                continue

            fGeom = f.geometry()
            engine = QgsGeometry.createGeometryEngine(fGeom.constGet())
            engine.prepareGeometry()

            bbox = fGeom.boundingBox()
            if strategy == 0:
                pointCount = int(value)
            else:
                pointCount = int(round(value * da.measureArea(fGeom)))

            if pointCount == 0:
                feedback.pushInfo("Skip feature {} as number of points for it is 0.".format(f.id()))
                continue

            index = QgsSpatialIndex()
            points = dict()

            nPoints = 0
            nIterations = 0
            maxIterations = pointCount * 200
            feature_total = total / pointCount if pointCount else 1

            random.seed()

            while nIterations < maxIterations and nPoints < pointCount:
                if feedback.isCanceled():
                    break

                rx = bbox.xMinimum() + bbox.width() * random.random()
                ry = bbox.yMinimum() + bbox.height() * random.random()

                p = QgsPointXY(rx, ry)
                geom = QgsGeometry.fromPointXY(p)
                if engine.contains(geom.constGet()) and \
                        vector.checkMinDistance(p, index, minDistance, points):
                    f = QgsFeature(nPoints)
                    f.initAttributes(1)
                    f.setFields(fields)
                    f.setAttribute('id', nPoints)
                    f.setGeometry(geom)
                    sink.addFeature(f, QgsFeatureSink.FastInsert)
                    index.addFeature(f)
                    points[nPoints] = p
                    nPoints += 1
                    feedback.setProgress(current_progress + int(nPoints * feature_total))
                nIterations += 1

            if nPoints < pointCount:
                feedback.pushInfo(self.tr('Could not generate requested number of random '
                                          'points. Maximum number of attempts exceeded.'))

        feedback.setProgress(100)

        return {self.OUTPUT: dest_id}
