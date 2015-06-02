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


from qgis.core import QgsField, QgsExpression, QgsFeature
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import ParameterVector
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector

from .fieldsmapping import ParameterFieldsMapping
from .ui.FieldsMapperDialogs import (FieldsMapperParametersDialog,
                                     FieldsMapperModelerParametersDialog)

class FieldsMapper(GeoAlgorithm):

    INPUT_LAYER = 'INPUT_LAYER'
    FIELDS_MAPPING = 'FIELDS_MAPPING'
    OUTPUT_LAYER = 'OUTPUT_LAYER'

    def __init__(self):
        GeoAlgorithm.__init__(self)
        self.mapping = None

    def defineCharacteristics(self):
        self.name = 'Refactor fields'
        self.group = 'Vector table tools'
        self.addParameter(ParameterVector(self.INPUT_LAYER,
            self.tr('Input layer'),
            [ParameterVector.VECTOR_TYPE_ANY], False))
        self.addParameter(ParameterFieldsMapping(self.FIELDS_MAPPING,
            self.tr('Fields mapping'), self.INPUT_LAYER))
        self.addOutput(OutputVector(self.OUTPUT_LAYER,
            self.tr('Refactored')))

    def getCustomParametersDialog(self):
        return FieldsMapperParametersDialog(self)

    def getCustomModelerParametersDialog(self, modelAlg, algIndex=None):
        return FieldsMapperModelerParametersDialog(self, modelAlg, algIndex)

    def processAlgorithm(self, progress):
        layer = self.getParameterValue(self.INPUT_LAYER)
        mapping = self.getParameterValue(self.FIELDS_MAPPING)
        output = self.getOutputFromName(self.OUTPUT_LAYER)

        layer = dataobjects.getObjectFromUri(layer)
        provider = layer.dataProvider()
        fields = []
        expressions = []
        for field_def in mapping:
            fields.append(QgsField(name=field_def['name'],
                                   type=field_def['type'],
                                   len=field_def['length'],
                                   prec=field_def['precision']))

            expression = QgsExpression(field_def['expression'])
            if expression.hasParserError():
                raise GeoAlgorithmExecutionException(
                    self.tr(u'Parser error in expression "{}": {}')
                    .format(unicode(field_def['expression']),
                            unicode(expression.parserErrorString())))
            expression.prepare(provider.fields())
            if expression.hasEvalError():
                raise GeoAlgorithmExecutionException(
                    self.tr(u'Evaluation error in expression "{}": {}')
                    .format(unicode(field_def['expression']),
                            unicode(expression.evalErrorString())))
            expressions.append(expression)

        writer = output.getVectorWriter(fields,
                                        provider.geometryType(),
                                        layer.crs())

        # Create output vector layer with new attributes
        error = ''
        calculationSuccess = True
        inFeat = QgsFeature()
        outFeat = QgsFeature()
        features = vector.features(layer)
        count = len(features)
        for current, inFeat in enumerate(features):
            rownum = current + 1

            outFeat.setGeometry(inFeat.geometry())

            attrs = []
            for i in xrange(0, len(mapping)):
                field_def = mapping[i]
                expression = expressions[i]
                expression.setCurrentRowNumber(rownum)
                value = expression.evaluate(inFeat)
                if expression.hasEvalError():
                    calculationSuccess = False
                    error = expression.evalErrorString()
                    break

                attrs.append(value)
            outFeat.setAttributes(attrs)

            writer.addFeature(outFeat)

            current += 1
            progress.setPercentage(100 * current / float(count))

        del writer

        if not calculationSuccess:
            raise GeoAlgorithmExecutionException(
                self.tr('An error occurred while evaluating the calculation'
                        ' string:\n') + error)
