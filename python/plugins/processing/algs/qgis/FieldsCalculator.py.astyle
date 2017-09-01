# -*- coding: utf-8 -*-

"""
***************************************************************************
    FieldsCalculator.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.PyQt.QtCore import QVariant
from qgis.core import (QgsExpression,
                       QgsExpressionContext,
                       QgsExpressionContextUtils,
                       QgsFeatureSink,
                       QgsField,
                       QgsDistanceArea,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterExpression,
                       QgsProcessingParameterString,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingException)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm

from .ui.FieldsCalculatorDialog import FieldsCalculatorDialog


class FieldsCalculator(QgisAlgorithm):
    INPUT = 'INPUT'
    NEW_FIELD = 'NEW_FIELD'
    FIELD_NAME = 'FIELD_NAME'
    FIELD_TYPE = 'FIELD_TYPE'
    FIELD_LENGTH = 'FIELD_LENGTH'
    FIELD_PRECISION = 'FIELD_PRECISION'
    FORMULA = 'FORMULA'
    OUTPUT = 'OUTPUT'

    TYPES = [QVariant.Double, QVariant.Int, QVariant.String, QVariant.Date]

    def group(self):
        return self.tr('Vector table')

    def __init__(self):
        super().__init__()
        self.type_names = [self.tr('Float'),
                           self.tr('Integer'),
                           self.tr('String'),
                           self.tr('Date')]

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT, self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterString(self.FIELD_NAME,
                                                       self.tr('Result field name')))
        self.addParameter(QgsProcessingParameterEnum(self.FIELD_TYPE,
                                                     self.tr('Field type'), options=self.type_names))
        self.addParameter(QgsProcessingParameterNumber(self.FIELD_LENGTH,
                                                       self.tr('Field length'), minValue=1, maxValue=255, defaultValue=10))
        self.addParameter(QgsProcessingParameterNumber(self.FIELD_PRECISION,
                                                       self.tr('Field precision'), minValue=0, maxValue=15, defaultValue=3))
        self.addParameter(QgsProcessingParameterBoolean(self.NEW_FIELD,
                                                        self.tr('Create new field'), defaultValue=True))
        self.addParameter(QgsProcessingParameterExpression(self.FORMULA, self.tr('Formula')))
        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT,
                                                            self.tr('Calculated')))

    def name(self):
        return 'fieldcalculator'

    def displayName(self):
        return self.tr('Field calculator')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        layer = self.parameterAsVectorLayer(parameters, self.INPUT, context)
        field_name = self.parameterAsString(parameters, self.FIELD_NAME, context)
        field_type = self.TYPES[self.parameterAsEnum(parameters, self.FIELD_TYPE, context)]
        width = self.parameterAsInt(parameters, self.FIELD_LENGTH, context)
        precision = self.parameterAsInt(parameters, self.FIELD_PRECISION, context)
        new_field = self.parameterAsBool(parameters, self.NEW_FIELD, context)
        formula = self.parameterAsString(parameters, self.FORMULA, context)

        expression = QgsExpression(formula)
        da = QgsDistanceArea()
        da.setSourceCrs(source.sourceCrs())
        da.setEllipsoid(context.project().ellipsoid())
        expression.setGeomCalculator(da)

        expression.setDistanceUnits(context.project().distanceUnits())
        expression.setAreaUnits(context.project().areaUnits())

        fields = source.fields()
        field_index = fields.lookupField(field_name)
        if new_field or field_index < 0:
            fields.append(QgsField(field_name, field_type, '', width, precision))

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, source.wkbType(), source.sourceCrs())

        exp_context = self.createExpressionContext(parameters, context)
        if layer is not None:
            exp_context.appendScope(QgsExpressionContextUtils.layerScope(layer))

        if not expression.prepare(exp_context):
            raise QgsProcessingException(
                self.tr('Evaluation error: {0}').format(expression.parserErrorString()))

        features = source.getFeatures()
        total = 100.0 / source.featureCount() if source.featureCount() else 0

        for current, f in enumerate(features):
            if feedback.isCanceled():
                break

            rownum = current + 1
            exp_context.setFeature(f)
            exp_context.lastScope().setVariable("row_number", rownum)
            value = expression.evaluate(exp_context)
            if expression.hasEvalError():
                feedback.reportError(expression.evalErrorString())
            else:
                attrs = f.attributes()
                if new_field or field_index < 0:
                    attrs.append(value)
                else:
                    attrs[field_index] = value
                f.setAttributes(attrs)
                sink.addFeature(f, QgsFeatureSink.FastInsert)
            feedback.setProgress(int(current * total))

        return {self.OUTPUT: dest_id}

    def checkParameterValues(self, parameters, context):
        newField = self.parameterAsBool(parameters, self.NEW_FIELD, context)
        fieldName = self.parameterAsString(parameters, self.FIELD_NAME, context).strip()
        if newField and len(fieldName) == 0:
            return False, self.tr('Field name is not set. Please enter a field name')
        return super(FieldsCalculator, self).checkParameterValues(parameters, context)

    def createCustomParametersWidget(self, parent):
        return FieldsCalculatorDialog(self)
