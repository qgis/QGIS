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
    QgsProcessingParameterDefinition,
    QgsProcessingParameterType,
    NULL)

from PyQt5.QtCore import QCoreApplication

from processing.algs.qgis.QgisAlgorithm import QgisFeatureBasedAlgorithm


class FieldsMapper(QgisFeatureBasedAlgorithm):

    INPUT_LAYER = 'INPUT_LAYER'
    FIELDS_MAPPING = 'FIELDS_MAPPING'
    OUTPUT_LAYER = 'OUTPUT_LAYER'

    def group(self):
        return self.tr('Vector table')

    def groupId(self):
        return 'vectortable'

    def tags(self):
        return self.tr('attributes,table').split(',')

    def initParameters(self, config=None):
        fields_mapping = FieldsMapper.ParameterFieldsMapping(self.FIELDS_MAPPING,
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

    def supportInPlaceEdit(self, layer):
        return False

    def prepareAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, 'INPUT', context)
        if source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, 'INPUT'))

        mapping = self.parameterAsFieldsMapping(parameters, self.FIELDS_MAPPING, context)

        self.fields = QgsFields()
        self.expressions = []

        da = QgsDistanceArea()
        da.setSourceCrs(source.sourceCrs(), context.transformContext())
        da.setEllipsoid(context.project().ellipsoid())

        # create an expression context using thread safe processing context
        self.expr_context = self.createExpressionContext(parameters, context, source)

        for field_def in mapping:
            self.fields.append(QgsField(name=field_def['name'],
                                        type=field_def['type'],
                                        typeName="",
                                        len=field_def.get('length', 0),
                                        prec=field_def.get('precision', 0)))
            if field_def['expression']:
                expression = QgsExpression(field_def['expression'])
                expression.setGeomCalculator(da)
                expression.setDistanceUnits(context.project().distanceUnits())
                expression.setAreaUnits(context.project().areaUnits())
                if expression.hasParserError():
                    feedback.reportError(
                        self.tr(u'Parser error in expression "{}": {}')
                        .format(expression.expression(),
                                expression.parserErrorString()))
                    return False
                self.expressions.append(expression)
            else:
                self.expressions.append(None)
        return True

    def outputFields(self, inputFields):
        return self.fields

    def processAlgorithm(self, parameters, context, feeback):
        for expression in self.expressions:
            if expression is not None:
                expression.prepare(self.expr_context)
        self._row_number = 0
        return super().processAlgorithm(parameters, context, feeback)

    def processFeature(self, feature, context, feedback):
        attributes = []
        for expression in self.expressions:
            if expression is not None:
                self.expr_context.setFeature(feature)
                self.expr_context.lastScope().setVariable("row_number", self._row_number)
                value = expression.evaluate(self.expr_context)
                if expression.hasEvalError():
                    raise QgsProcessingException(
                        self.tr(u'Evaluation error in expression "{}": {}')
                            .format(expression.expression(),
                                    expression.evalErrorString()))
                attributes.append(value)
            else:
                attributes.append(NULL)
        feature.setAttributes(attributes)
        self._row_number += 1
        return [feature]

    class ParameterFieldsMappingType(QgsProcessingParameterType):

        def __init__(self):
            super().__init__()

        def create(self, name):
            return FieldsMapper.ParameterFieldsMapping(name)

        def metadata(self):
            return {'widget_wrapper': 'processing.algs.qgis.ui.FieldsMappingPanel.FieldsMappingWidgetWrapper'}

        def name(self):
            return QCoreApplication.translate('Processing', 'Fields Mapper')

        def id(self):
            return 'fields_mapping'

        def pythonImportString(self):
            return 'from processing.algs.qgis.FieldsMapper import FieldsMapper'

        def className(self):
            return 'FieldsMapper.ParameterFieldsMapping'

        def description(self):
            return QCoreApplication.translate('Processing', 'A mapping of field names to field type definitions and expressions. Used for the refactor fields algorithm.')

    class ParameterFieldsMapping(QgsProcessingParameterDefinition):

        def __init__(self, name, description='', parentLayerParameterName='INPUT'):
            super().__init__(name, description)
            self._parentLayerParameter = parentLayerParameterName

        def clone(self):
            copy = FieldsMapper.ParameterFieldsMapping(self.name(), self.description(), self._parentLayerParameter)
            return copy

        def type(self):
            return self.typeName()

        @staticmethod
        def typeName():
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
