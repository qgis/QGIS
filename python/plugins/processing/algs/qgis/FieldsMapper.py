# -*- coding: utf-8 -*-

"""
***************************************************************************
    FieldsMapper.py
    ---------------------
    Date                 : October 2014
    Copyright            : (C) 2014 by Arnaud Morvan
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
__date__ = 'October 2014'
__copyright__ = '(C) 2014, Arnaud Morvan'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import (
    QgsDistanceArea,
    QgsExpression,
    QgsField,
    QgsFields,
    QgsProcessing,
    QgsProcessingException,
    QgsProcessingParameterDefinition)

from processing.algs.qgis.QgisAlgorithm import QgisFeatureBasedAlgorithm


class FieldsMapper(QgisFeatureBasedAlgorithm):

    INPUT_LAYER = 'INPUT_LAYER'
    FIELDS_MAPPING = 'FIELDS_MAPPING'
    OUTPUT_LAYER = 'OUTPUT_LAYER'

    def group(self):
        return self.tr('Vector table')

    def initParameters(self, config=None):

        class ParameterFieldsMapping(QgsProcessingParameterDefinition):

            def __init__(self, name, description, parentLayerParameterName='INPUT'):
                super().__init__(name, description)
                self._parentLayerParameter = parentLayerParameterName

            def clone(self):
                copy = ParameterFieldsMapping(self.name(), self.description(), self._parentLayerParameter)
                return copy

            def type(self):
                return 'fields_mapping'

            def checkValueIsAcceptable(self, value, context=None):
                if not isinstance(value, list):
                    return False
                for field_def in value:
                    if not isinstance(field_def, dict):
                        return False
                    if 'name' not in field_def.keys():
                        return False
                    if 'type' not in field_def.keys():
                        return False
                    if 'expression' not in field_def.keys():
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

        fields_mapping = ParameterFieldsMapping(self.FIELDS_MAPPING,
                                                description=self.tr('Fields mapping'))
        fields_mapping.setMetadata({
            'widget_wrapper': 'processing.algs.qgis.ui.FieldsMappingPanel.FieldsMappingWidgetWrapper'
        })
        self.addParameter(fields_mapping)

    def name(self):
        return 'refactorfields'

    def displayName(self):
        return self.tr('Refactor fields')

    def outputName(self):
        return self.tr('Refactored')

    def inputLayerTypes(self):
        return [QgsProcessing.TypeVector]

    def parameterAsFieldsMapping(self, parameters, name, context):
        return parameters[name]

    def prepareAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, 'INPUT', context)
        mapping = self.parameterAsFieldsMapping(parameters, self.FIELDS_MAPPING, context)

        self.fields = QgsFields()
        self.expressions = []

        da = QgsDistanceArea()
        da.setSourceCrs(source.sourceCrs())
        da.setEllipsoid(context.project().ellipsoid())

        for field_def in mapping:
            self.fields.append(QgsField(name=field_def['name'],
                                        type=field_def['type'],
                                        typeName="",
                                        len=field_def.get('length', 0),
                                        prec=field_def.get('precision', 0)))
            expression = QgsExpression(field_def['expression'])
            expression.setGeomCalculator(da)
            expression.setDistanceUnits(context.project().distanceUnits())
            expression.setAreaUnits(context.project().areaUnits())
            if expression.hasParserError():
                raise QgsProcessingException(
                    self.tr(u'Parser error in expression "{}": {}')
                    .format(str(expression.expression()),
                            str(expression.parserErrorString())))
            self.expressions.append(expression)
        return True

    def outputFields(self, inputFields):
        return self.fields

    def processAlgorithm(self, parameters, context, feeback):
        # create an expression context using thead safe processing context
        self.expr_context = self.createExpressionContext(parameters, context)
        for expression in self.expressions:
            expression.prepare(self.expr_context)
        self._row_number = 0
        return super().processAlgorithm(parameters, context, feeback)

    def processFeature(self, feature, feedback):
        attributes = []
        for expression in self.expressions:
            self.expr_context.setFeature(feature)
            self.expr_context.lastScope().setVariable("row_number", self._row_number)
            value = expression.evaluate(self.expr_context)
            if expression.hasEvalError():
                raise QgsProcessingException(
                    self.tr(u'Evaluation error in expression "{}": {}')
                        .format(str(expression.expression()),
                                str(expression.parserErrorString())))
            attributes.append(value)
        feature.setAttributes(attributes)
        self._row_number += 1
        return feature
