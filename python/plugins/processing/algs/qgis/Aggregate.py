# -*- coding: utf-8 -*-

"""
***************************************************************************
    Aggregate.py
    ---------------------
    Date                 : February 2017
    Copyright            : (C) 2017 by Arnaud Morvan
    Email                : arnaud dot morvan at camptocamp dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Arnaud Morvan'
__date__ = 'February 2017'
__copyright__ = '(C) 2017, Arnaud Morvan'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import (
    QgsDistanceArea,
    QgsExpression,
    QgsExpressionContextUtils,
    QgsFeature,
    QgsFeatureSink,
    QgsField,
    QgsFields,
    QgsGeometry,
    QgsProcessing,
    QgsProcessingParameterDefinition,
    QgsProcessingParameterExpression,
    QgsProcessingParameterFeatureSink,
    QgsProcessingParameterFeatureSource,
    QgsProcessingException,
    QgsProcessingUtils,
    QgsWkbTypes,
)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class Aggregate(QgisAlgorithm):

    INPUT = 'INPUT'
    GROUP_BY = 'GROUP_BY'
    AGGREGATES = 'AGGREGATES'
    DISSOLVE = 'DISSOLVE'
    OUTPUT = 'OUTPUT'

    def group(self):
        return self.tr('Vector geometry')

    def groupId(self):
        return 'vectorgeometry'

    def name(self):
        return 'aggregate'

    def displayName(self):
        return self.tr('Aggregate')

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer'),
                                                              types=[QgsProcessing.TypeVector]))
        self.addParameter(QgsProcessingParameterExpression(self.GROUP_BY,
                                                           self.tr('Group by expression (NULL to group all features)'),
                                                           defaultValue='NULL',
                                                           optional=False,
                                                           parentLayerParameterName=self.INPUT))

        class ParameterAggregates(QgsProcessingParameterDefinition):

            def __init__(self, name, description, parentLayerParameterName='INPUT'):
                super().__init__(name, description)
                self._parentLayerParameter = parentLayerParameterName

            def clone(self):
                copy = ParameterAggregates(self.name(), self.description(), self._parentLayerParameter)
                return copy

            def type(self):
                return 'aggregates'

            def checkValueIsAcceptable(self, value, context=None):
                if not isinstance(value, list):
                    return False
                for field_def in value:
                    if not isinstance(field_def, dict):
                        return False
                    if not field_def.get('input', False):
                        return False
                    if not field_def.get('aggregate', False):
                        return False
                    if not field_def.get('name', False):
                        return False
                    if not field_def.get('type', False):
                        return False
                return True

            def valueAsPythonString(self, value, context):
                return str(value)

            def asScriptCode(self):
                raise NotImplementedError()

            @classmethod
            def fromScriptCode(cls, name, description, isOptional, definition):
                raise NotImplementedError()

            def parentLayerParameter(self):
                return self._parentLayerParameter

        self.addParameter(ParameterAggregates(self.AGGREGATES,
                                              description=self.tr('Aggregates')))
        self.parameterDefinition(self.AGGREGATES).setMetadata({
            'widget_wrapper': 'processing.algs.qgis.ui.AggregatesPanel.AggregatesWidgetWrapper'
        })

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT,
                                                            self.tr('Aggregated')))

    def parameterAsAggregates(self, parameters, name, context):
        return parameters[name]

    def prepareAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        if source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.INPUT))

        group_by = self.parameterAsExpression(parameters, self.GROUP_BY, context)
        aggregates = self.parameterAsAggregates(parameters, self.AGGREGATES, context)

        da = QgsDistanceArea()
        da.setSourceCrs(source.sourceCrs(), context.transformContext())
        da.setEllipsoid(context.project().ellipsoid())

        self.source = source
        self.group_by = group_by
        self.group_by_expr = self.createExpression(group_by, da, context)
        self.geometry_expr = self.createExpression('collect($geometry, {})'.format(group_by), da, context)

        self.fields = QgsFields()
        self.fields_expr = []
        for field_def in aggregates:
            self.fields.append(QgsField(name=field_def['name'],
                                        type=field_def['type'],
                                        typeName="",
                                        len=field_def['length'],
                                        prec=field_def['precision']))
            aggregate = field_def['aggregate']
            if aggregate == 'first_value':
                expression = field_def['input']
            elif aggregate == 'concatenate' or aggregate == 'concatenate_unique':
                expression = ('{}({}, {}, {}, \'{}\')'
                              .format(field_def['aggregate'],
                                      field_def['input'],
                                      group_by,
                                      'TRUE',
                                      field_def['delimiter']))
            else:
                expression = '{}({}, {})'.format(field_def['aggregate'],
                                                 field_def['input'],
                                                 group_by)
            expr = self.createExpression(expression, da, context)
            self.fields_expr.append(expr)
        return True

    def processAlgorithm(self, parameters, context, feedback):
        expr_context = self.createExpressionContext(parameters, context, self.source)
        self.group_by_expr.prepare(expr_context)

        # Group features in memory layers
        source = self.source
        count = self.source.featureCount()
        if count:
            progress_step = 50.0 / count
        current = 0
        groups = {}
        keys = []  # We need deterministic order for the tests
        feature = QgsFeature()
        for feature in self.source.getFeatures():
            expr_context.setFeature(feature)
            group_by_value = self.evaluateExpression(self.group_by_expr, expr_context)

            # Get an hashable key for the dict
            key = group_by_value
            if isinstance(key, list):
                key = tuple(key)

            group = groups.get(key, None)
            if group is None:
                sink, id = QgsProcessingUtils.createFeatureSink(
                    'memory:',
                    context,
                    source.fields(),
                    source.wkbType(),
                    source.sourceCrs())
                layer = QgsProcessingUtils.mapLayerFromString(id, context)
                group = {
                    'sink': sink,
                    'layer': layer,
                    'feature': feature
                }
                groups[key] = group
                keys.append(key)

            group['sink'].addFeature(feature, QgsFeatureSink.FastInsert)

            current += 1
            feedback.setProgress(int(current * progress_step))
            if feedback.isCanceled():
                return

        (sink, dest_id) = self.parameterAsSink(parameters,
                                               self.OUTPUT,
                                               context,
                                               self.fields,
                                               QgsWkbTypes.multiType(source.wkbType()),
                                               source.sourceCrs())
        if sink is None:
            raise QgsProcessingException(self.invalidSinkError(parameters, self.OUTPUT))

        # Calculate aggregates on memory layers
        if len(keys):
            progress_step = 50.0 / len(keys)
        for current, key in enumerate(keys):
            group = groups[key]
            expr_context = self.createExpressionContext(parameters, context)
            expr_context.appendScope(QgsExpressionContextUtils.layerScope(group['layer']))
            expr_context.setFeature(group['feature'])

            geometry = self.evaluateExpression(self.geometry_expr, expr_context)
            if geometry is not None and not geometry.isEmpty():
                geometry = QgsGeometry.unaryUnion(geometry.asGeometryCollection())
                if geometry.isEmpty():
                    raise QgsProcessingException(
                        'Impossible to combine geometries for {} = {}'
                        .format(self.group_by, group_by_value))

            attrs = []
            for fields_expr in self.fields_expr:
                attrs.append(self.evaluateExpression(fields_expr, expr_context))

            # Write output feature
            outFeat = QgsFeature()
            if geometry is not None:
                outFeat.setGeometry(geometry)
            outFeat.setAttributes(attrs)
            sink.addFeature(outFeat, QgsFeatureSink.FastInsert)

            feedback.setProgress(50 + int(current * progress_step))
            if feedback.isCanceled():
                return

        return {self.OUTPUT: dest_id}

    def createExpression(self, text, da, context):
        expr = QgsExpression(text)
        expr.setGeomCalculator(da)
        expr.setDistanceUnits(context.project().distanceUnits())
        expr.setAreaUnits(context.project().areaUnits())
        if expr.hasParserError():
            raise QgsProcessingException(
                self.tr(u'Parser error in expression "{}": {}')
                .format(text, expr.parserErrorString()))
        return expr

    def evaluateExpression(self, expr, context):
        value = expr.evaluate(context)
        if expr.hasEvalError():
            raise QgsProcessingException(
                self.tr(u'Evaluation error in expression "{}": {}')
                .format(expr.expression(), expr.evalErrorString()))
        return value
