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

__author__ = "Alexander Bruy"
__date__ = "April 2014"
__copyright__ = "(C) 2014, Alexander Bruy"

import os
import random

from qgis.PyQt.QtCore import QMetaType
from qgis.core import (
    Qgis,
    QgsApplication,
    QgsField,
    QgsFeatureSink,
    QgsFeature,
    QgsFields,
    QgsGeometry,
    QgsPointXY,
    QgsWkbTypes,
    QgsSpatialIndex,
    QgsExpression,
    QgsDistanceArea,
    QgsPropertyDefinition,
    QgsProcessing,
    QgsProcessingException,
    QgsProcessingParameters,
    QgsProcessingParameterDefinition,
    QgsProcessingParameterNumber,
    QgsProcessingParameterDistance,
    QgsProcessingParameterFeatureSource,
    QgsProcessingParameterFeatureSink,
    QgsProcessingParameterExpression,
    QgsProcessingParameterEnum,
)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.tools import vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class RandomPointsPolygons(QgisAlgorithm):
    INPUT = "INPUT"
    VALUE = "VALUE"
    EXPRESSION = "EXPRESSION"
    MIN_DISTANCE = "MIN_DISTANCE"
    STRATEGY = "STRATEGY"
    OUTPUT = "OUTPUT"

    def icon(self):
        return QgsApplication.getThemeIcon(
            "/algorithms/mAlgorithmRandomPointsWithinPolygon.svg"
        )

    def svgIconPath(self):
        return QgsApplication.iconPath(
            "/algorithms/mAlgorithmRandomPointsWithinPolygon.svg"
        )

    def group(self):
        return self.tr("Vector creation")

    def groupId(self):
        return "vectorcreation"

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.strategies = [self.tr("Points count"), self.tr("Points density")]

        self.addParameter(
            QgsProcessingParameterFeatureSource(
                self.INPUT,
                self.tr("Input layer"),
                [QgsProcessing.SourceType.TypeVectorPolygon],
            )
        )
        self.addParameter(
            QgsProcessingParameterEnum(
                self.STRATEGY, self.tr("Sampling strategy"), self.strategies, False, 0
            )
        )
        value_param = QgsProcessingParameterNumber(
            self.VALUE,
            self.tr("Point count or density"),
            QgsProcessingParameterNumber.Type.Double,
            1,
            minValue=0,
        )
        value_param.setIsDynamic(True)
        value_param.setDynamicLayerParameterName(self.INPUT)
        value_param.setDynamicPropertyDefinition(
            QgsPropertyDefinition(
                "Value",
                self.tr("Point count or density"),
                QgsPropertyDefinition.StandardPropertyTemplate.Double,
            )
        )
        self.addParameter(value_param)

        # deprecated expression parameter - overrides value parameter if set
        exp_param = QgsProcessingParameterExpression(
            self.EXPRESSION,
            self.tr("Expression"),
            optional=True,
            parentLayerParameterName=self.INPUT,
        )
        exp_param.setFlags(
            exp_param.flags() | QgsProcessingParameterDefinition.Flag.FlagHidden
        )
        self.addParameter(exp_param)

        self.addParameter(
            QgsProcessingParameterDistance(
                self.MIN_DISTANCE,
                self.tr("Minimum distance between points"),
                None,
                self.INPUT,
                True,
                0,
                1000000000,
            )
        )
        self.addParameter(
            QgsProcessingParameterFeatureSink(
                self.OUTPUT,
                self.tr("Random points"),
                type=QgsProcessing.SourceType.TypeVectorPoint,
            )
        )

    def name(self):
        return "randompointsinsidepolygons"

    def displayName(self):
        return self.tr("Random points inside polygons")

    def documentationFlags(self):
        return Qgis.ProcessingAlgorithmDocumentationFlag.RegeneratesPrimaryKey

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        if source is None:
            raise QgsProcessingException(
                self.invalidSourceError(parameters, self.INPUT)
            )

        strategy = self.parameterAsEnum(parameters, self.STRATEGY, context)
        if (
            self.MIN_DISTANCE in parameters
            and parameters[self.MIN_DISTANCE] is not None
        ):
            minDistance = self.parameterAsDouble(parameters, self.MIN_DISTANCE, context)
        else:
            minDistance = None

        expressionContext = self.createExpressionContext(parameters, context, source)
        dynamic_value = QgsProcessingParameters.isDynamic(parameters, "VALUE")
        value_property = None
        if self.EXPRESSION in parameters and parameters[self.EXPRESSION] is not None:
            expression = QgsExpression(
                self.parameterAsString(parameters, self.EXPRESSION, context)
            )
            value = None
            if expression.hasParserError():
                raise QgsProcessingException(expression.parserErrorString())
            expression.prepare(expressionContext)
        else:
            expression = None
            if dynamic_value:
                value_property = parameters["VALUE"]
            value = self.parameterAsDouble(parameters, self.VALUE, context)

        fields = QgsFields()
        fields.append(QgsField("id", QMetaType.Type.Int, "", 10, 0))

        (sink, dest_id) = self.parameterAsSink(
            parameters,
            self.OUTPUT,
            context,
            fields,
            QgsWkbTypes.Type.Point,
            source.sourceCrs(),
            QgsFeatureSink.SinkFlag.RegeneratePrimaryKey,
        )
        if sink is None:
            raise QgsProcessingException(self.invalidSinkError(parameters, self.OUTPUT))

        da = QgsDistanceArea()
        da.setSourceCrs(source.sourceCrs(), context.transformContext())
        da.setEllipsoid(context.ellipsoid())

        total = 100.0 / source.featureCount() if source.featureCount() else 0
        current_progress = 0
        pointId = 0
        for current, f in enumerate(source.getFeatures()):
            if feedback.isCanceled():
                break

            if not f.hasGeometry():
                continue

            current_progress = total * current
            feedback.setProgress(current_progress)

            this_value = value
            if value_property is not None or expression is not None:
                expressionContext.setFeature(f)
                if value_property:
                    this_value, _ = value_property.valueAsDouble(
                        expressionContext, value
                    )
                else:
                    this_value = expression.evaluate(expressionContext)
                    if expression.hasEvalError():
                        feedback.pushInfo(
                            self.tr("Evaluation error for feature ID {}: {}").format(
                                f.id(), expression.evalErrorString()
                            )
                        )
                        continue

            fGeom = f.geometry()
            engine = QgsGeometry.createGeometryEngine(fGeom.constGet())
            engine.prepareGeometry()

            bbox = fGeom.boundingBox()
            if strategy == 0:
                pointCount = int(this_value)
            else:
                pointCount = int(round(this_value * da.measureArea(fGeom)))

            if pointCount == 0:
                feedback.pushInfo(
                    self.tr("Skip feature {} as number of points for it is 0.").format(
                        f.id()
                    )
                )
                continue

            index = None
            if minDistance:
                index = QgsSpatialIndex()
            points = {}

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
                if engine.contains(geom.constGet()) and (
                    not minDistance
                    or vector.checkMinDistance(p, index, minDistance, points)
                ):
                    f = QgsFeature(nPoints)
                    f.initAttributes(1)
                    f.setFields(fields)
                    f.setAttribute("id", pointId)
                    f.setGeometry(geom)
                    sink.addFeature(f, QgsFeatureSink.Flag.FastInsert)
                    if minDistance:
                        index.addFeature(f)
                    points[nPoints] = p
                    nPoints += 1
                    pointId += 1
                    feedback.setProgress(
                        current_progress + int(nPoints * feature_total)
                    )
                nIterations += 1

            if nPoints < pointCount:
                feedback.pushInfo(
                    self.tr(
                        "Could not generate requested number of random "
                        "points. Maximum number of attempts exceeded."
                    )
                )

        feedback.setProgress(100)
        sink.finalize()
        return {self.OUTPUT: dest_id}
