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

from qgis.core import (QgsField,
                       QgsFields,
                       QgsExpression,
                       QgsDistanceArea,
                       QgsFeatureSink,
                       QgsProject,
                       QgsFeature,
                       QgsApplication,
                       QgsProcessingUtils)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import ParameterTable
from processing.core.parameters import Parameter
from processing.core.outputs import OutputVector


class FieldsMapper(QgisAlgorithm):

    INPUT_LAYER = 'INPUT_LAYER'
    FIELDS_MAPPING = 'FIELDS_MAPPING'
    OUTPUT_LAYER = 'OUTPUT_LAYER'

    def __init__(self):
        GeoAlgorithm.__init__(self)
        self.mapping = None

    def group(self):
        return self.tr('Vector table tools')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(ParameterTable(self.INPUT_LAYER,
                                         self.tr('Input layer'),
                                         False))

        class ParameterFieldsMapping(Parameter):

            default_metadata = {
                'widget_wrapper': 'processing.algs.qgis.ui.FieldsMappingPanel.FieldsMappingWidgetWrapper'
            }

            def __init__(self, name='', description='', parent=None):
                Parameter.__init__(self, name, description)
                self.parent = parent
                self.value = []

            def getValueAsCommandLineParameter(self):
                return '"' + str(self.value) + '"'

            def setValue(self, value):
                if value is None:
                    return False
                if isinstance(value, list):
                    self.value = value
                    return True
                if isinstance(value, str):
                    try:
                        self.value = eval(value)
                        return True
                    except Exception as e:
                        # fix_print_with_import
                        print(str(e))  # display error in console
                        return False
                return False

        self.addParameter(ParameterFieldsMapping(self.FIELDS_MAPPING,
                                                 self.tr('Fields mapping'),
                                                 self.INPUT_LAYER))
        self.addOutput(OutputVector(self.OUTPUT_LAYER,
                                    self.tr('Refactored'),
                                    base_input=self.INPUT_LAYER))

    def name(self):
        return 'refactorfields'

    def displayName(self):
        return self.tr('Refactor fields')

    def processAlgorithm(self, parameters, context, feedback):
        layer = self.getParameterValue(self.INPUT_LAYER)
        mapping = self.getParameterValue(self.FIELDS_MAPPING)
        output = self.getOutputFromName(self.OUTPUT_LAYER)

        layer = QgsProcessingUtils.mapLayerFromString(layer, context)
        fields = QgsFields()
        expressions = []

        da = QgsDistanceArea()
        da.setSourceCrs(layer.crs())
        da.setEllipsoid(context.project().ellipsoid())

        exp_context = layer.createExpressionContext()

        for field_def in mapping:
            fields.append(QgsField(field_def['name'],
                                   field_def['type'],
                                   field_def['length'],
                                   field_def['precision']))

            expression = QgsExpression(field_def['expression'])
            expression.setGeomCalculator(da)
            expression.setDistanceUnits(context.project().distanceUnits())
            expression.setAreaUnits(context.project().areaUnits())
            expression.prepare(exp_context)
            if expression.hasParserError():
                raise GeoAlgorithmExecutionException(
                    self.tr(u'Parser error in expression "{}": {}')
                    .format(str(expression.expression()),
                            str(expression.parserErrorString())))
            expressions.append(expression)

        writer = output.getVectorWriter(fields, layer.wkbType(), layer.crs(), context)

        # Create output vector layer with new attributes
        error_exp = None
        inFeat = QgsFeature()
        outFeat = QgsFeature()
        features = QgsProcessingUtils.getFeatures(layer, context)
        count = QgsProcessingUtils.featureCount(layer, context)
        if count > 0:
            total = 100.0 / count
            for current, inFeat in enumerate(features):
                rownum = current + 1

                geometry = inFeat.geometry()
                outFeat.setGeometry(geometry)

                attrs = []
                for i in range(0, len(mapping)):
                    field_def = mapping[i]
                    expression = expressions[i]
                    exp_context.setFeature(inFeat)
                    exp_context.lastScope().setVariable("row_number", rownum)
                    value = expression.evaluate(exp_context)
                    if expression.hasEvalError():
                        error_exp = expression
                        break

                    attrs.append(value)
                outFeat.setAttributes(attrs)

                writer.addFeature(outFeat, QgsFeatureSink.FastInsert)

                feedback.setProgress(int(current * total))
        else:
            feedback.setProgress(100)

        del writer

        if error_exp is not None:
            raise GeoAlgorithmExecutionException(
                self.tr(u'Evaluation error in expression "{}": {}')
                    .format(str(error_exp.expression()),
                            str(error_exp.parserErrorString())))
