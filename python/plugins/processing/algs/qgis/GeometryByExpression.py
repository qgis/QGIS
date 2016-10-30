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

# This will get replaced with a git SHA1 when you do a git archive323

__revision__ = '$Format:%H$'

from qgis.core import QgsWkbTypes, QgsExpression, QgsExpressionContext, QgsExpressionContextUtils, QgsGeometry

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.ProcessingLog import ProcessingLog
from processing.core.parameters import ParameterVector, ParameterSelection, ParameterBoolean, ParameterString
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class GeometryByExpression(GeoAlgorithm):

    INPUT_LAYER = 'INPUT_LAYER'
    OUTPUT_LAYER = 'OUTPUT_LAYER'
    OUTPUT_GEOMETRY = 'OUTPUT_GEOMETRY'
    WITH_Z = 'WITH_Z'
    WITH_M = 'WITH_M'
    EXPRESSION = 'EXPRESSION'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Geometry by expression')
        self.group, self.i18n_group = self.trAlgorithm('Vector geometry tools')

        self.addParameter(ParameterVector(self.INPUT_LAYER,
                                          self.tr('Input layer')))

        self.geometry_types = [self.tr('Polygon'),
                               'Line',
                               'Point']
        self.addParameter(ParameterSelection(
            self.OUTPUT_GEOMETRY,
            self.tr('Output geometry type'),
            self.geometry_types, default=0))
        self.addParameter(ParameterBoolean(self.WITH_Z,
                                           self.tr('Output geometry has z dimension'), False))
        self.addParameter(ParameterBoolean(self.WITH_M,
                                           self.tr('Output geometry has m values'), False))

        self.addParameter(ParameterString(self.EXPRESSION,
                                          self.tr("Geometry expression"), '$geometry'))

        self.addOutput(OutputVector(self.OUTPUT_LAYER, self.tr('Modified geometry')))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT_LAYER))

        geometry_type = self.getParameterValue(self.OUTPUT_GEOMETRY)
        wkb_type = None
        if geometry_type == 0:
            wkb_type = QgsWkbTypes.Polygon
        elif geometry_type == 1:
            wkb_type = QgsWkbTypes.LineString
        else:
            wkb_type = QgsWkbTypes.Point
        if self.getParameterValue(self.WITH_Z):
            wkb_type = QgsWkbTypes.addZ(wkb_type)
        if self.getParameterValue(self.WITH_M):
            wkb_type = QgsWkbTypes.addM(wkb_type)

        writer = self.getOutputFromName(
            self.OUTPUT_LAYER).getVectorWriter(
                layer.fields(),
                wkb_type,
                layer.crs())

        expression = QgsExpression(self.getParameterValue(self.EXPRESSION))
        if expression.hasParserError():
            raise GeoAlgorithmExecutionException(expression.parserErrorString())

        exp_context = QgsExpressionContext()
        exp_context.appendScope(QgsExpressionContextUtils.globalScope())
        exp_context.appendScope(QgsExpressionContextUtils.projectScope())
        exp_context.appendScope(QgsExpressionContextUtils.layerScope(layer))

        if not expression.prepare(exp_context):
            raise GeoAlgorithmExecutionException(
                self.tr('Evaluation error: %s' % expression.evalErrorString()))

        features = vector.features(layer)
        total = 100.0 / len(features)
        for current, input_feature in enumerate(features):
            output_feature = input_feature

            exp_context.setFeature(input_feature)
            value = expression.evaluate(exp_context)
            if expression.hasEvalError():
                raise GeoAlgorithmExecutionException(
                    self.tr('Evaluation error: %s' % expression.evalErrorString()))

            if not value:
                output_feature.setGeometry(QgsGeometry())
            else:
                if not isinstance(value, QgsGeometry):
                    raise GeoAlgorithmExecutionException(
                        self.tr('{} is not a geometry').format(value))
                output_feature.setGeometry(value)

            writer.addFeature(output_feature)
            progress.setPercentage(int(current * total))

        del writer
