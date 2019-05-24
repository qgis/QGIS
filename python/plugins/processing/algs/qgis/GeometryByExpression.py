# -*- coding: utf-8 -*-

"""
***************************************************************************
    GeometryByExpression.py
    -----------------------
    Date                 : October 2016
    Copyright            : (C) 2016 by Nyall Dawson
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
__date__ = 'October 2016'
__copyright__ = '(C) 2016, Nyall Dawson'

from qgis.core import (QgsWkbTypes,
                       QgsExpression,
                       QgsGeometry,
                       QgsProcessing,
                       QgsProcessingAlgorithm,
                       QgsProcessingException,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterExpression,
                       QgsProcessingFeatureSource)

from processing.algs.qgis.QgisAlgorithm import QgisFeatureBasedAlgorithm


class GeometryByExpression(QgisFeatureBasedAlgorithm):

    OUTPUT_GEOMETRY = 'OUTPUT_GEOMETRY'
    WITH_Z = 'WITH_Z'
    WITH_M = 'WITH_M'
    EXPRESSION = 'EXPRESSION'

    def group(self):
        return self.tr('Vector geometry')

    def groupId(self):
        return 'vectorgeometry'

    def flags(self):
        return super().flags() & ~QgsProcessingAlgorithm.FlagSupportsInPlaceEdits

    def __init__(self):
        super().__init__()
        self.geometry_types = [self.tr('Polygon'),
                               'Line',
                               'Point']

    def initParameters(self, config=None):
        self.addParameter(QgsProcessingParameterEnum(
            self.OUTPUT_GEOMETRY,
            self.tr('Output geometry type'),
            options=self.geometry_types, defaultValue=0))
        self.addParameter(QgsProcessingParameterBoolean(self.WITH_Z,
                                                        self.tr('Output geometry has z dimension'), defaultValue=False))
        self.addParameter(QgsProcessingParameterBoolean(self.WITH_M,
                                                        self.tr('Output geometry has m values'), defaultValue=False))

        self.addParameter(QgsProcessingParameterExpression(self.EXPRESSION,
                                                           self.tr("Geometry expression"), defaultValue='$geometry', parentLayerParameterName='INPUT'))

    def name(self):
        return 'geometrybyexpression'

    def displayName(self):
        return self.tr('Geometry by expression')

    def outputName(self):
        return self.tr('Modified geometry')

    def prepareAlgorithm(self, parameters, context, feedback):
        self.geometry_type = self.parameterAsEnum(parameters, self.OUTPUT_GEOMETRY, context)
        self.wkb_type = None
        if self.geometry_type == 0:
            self.wkb_type = QgsWkbTypes.Polygon
        elif self.geometry_type == 1:
            self.wkb_type = QgsWkbTypes.LineString
        else:
            self.wkb_type = QgsWkbTypes.Point
        if self.parameterAsBoolean(parameters, self.WITH_Z, context):
            self.wkb_type = QgsWkbTypes.addZ(self.wkb_type)
        if self.parameterAsBoolean(parameters, self.WITH_M, context):
            self.wkb_type = QgsWkbTypes.addM(self.wkb_type)

        self.expression = QgsExpression(self.parameterAsString(parameters, self.EXPRESSION, context))
        if self.expression.hasParserError():
            feedback.reportError(self.expression.parserErrorString())
            return False

        self.expression_context = self.createExpressionContext(parameters, context)
        self.expression.prepare(self.expression_context)

        return True

    def outputWkbType(self, input_wkb_type):
        return self.wkb_type

    def inputLayerTypes(self):
        return [QgsProcessing.TypeVector]

    def sourceFlags(self):
        return QgsProcessingFeatureSource.FlagSkipGeometryValidityChecks

    def processFeature(self, feature, context, feedback):
        self.expression_context.setFeature(feature)
        value = self.expression.evaluate(self.expression_context)
        if self.expression.hasEvalError():
            raise QgsProcessingException(
                self.tr('Evaluation error: {0}').format(self.expression.evalErrorString()))

        if not value:
            feature.setGeometry(QgsGeometry())
        else:
            if not isinstance(value, QgsGeometry):
                raise QgsProcessingException(
                    self.tr('{} is not a geometry').format(value))
            feature.setGeometry(value)
        return [feature]
