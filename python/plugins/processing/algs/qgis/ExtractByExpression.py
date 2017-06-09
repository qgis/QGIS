# -*- coding: utf-8 -*-

"""
***************************************************************************
    ExtractByExpression.py
    ---------------------
    Date                 : October 2016
    Copyright            : (C) 2016 by Nyall Dawson
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

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import (QgsExpression,
                       QgsFeatureRequest,
                       QgsApplication,
                       QgsProcessingUtils,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterExpression,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingOutputVectorLayer,
                       QgsProcessingParameterDefinition)

from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import ParameterVector
from processing.core.outputs import OutputVector
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.core.parameters import ParameterExpression


class ExtractByExpression(QgisAlgorithm):

    INPUT = 'INPUT'
    EXPRESSION = 'EXPRESSION'
    OUTPUT = 'OUTPUT'
    FAIL_OUTPUT = 'FAIL_OUTPUT'

    def icon(self):
        return QgsApplication.getThemeIcon("/providerQgis.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("providerQgis.svg")

    def tags(self):
        return self.tr('extract,filter,expression,field').split(',')

    def group(self):
        return self.tr('Vector selection tools')

    def __init__(self):
        super().__init__()
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterExpression(self.EXPRESSION,
                                                           self.tr('Expression'), None, self.INPUT))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Matching features')))
        self.addOutput(QgsProcessingOutputVectorLayer(self.OUTPUT, self.tr('Matching (expression)')))
        self.addParameter(QgsProcessingParameterFeatureSink(self.FAIL_OUTPUT, self.tr('Non-matching'),
                                                            QgsProcessingParameterDefinition.TypeVectorAny, None, True))
        self.addOutput(QgsProcessingOutputVectorLayer(self.FAIL_OUTPUT, self.tr('Non-matching (expression)')))

    def name(self):
        return 'extractbyexpression'

    def displayName(self):
        return self.tr('Extract by expression')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        expression_string = self.parameterAsExpression(parameters, self.EXPRESSION, context)

        (matching_sink, matching_sink_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                                                 source.fields(), source.wkbType(), source.sourceCrs())
        (nonmatching_sink, non_matching_sink_id) = self.parameterAsSink(parameters, self.FAIL_OUTPUT, context,
                                                                        source.fields(), source.wkbType(), source.sourceCrs())

        expression = QgsExpression(expression_string)
        if expression.hasParserError():
            raise GeoAlgorithmExecutionException(expression.parserErrorString())
        expression_context = self.createExpressionContext(parameters, context)

        if not nonmatching_sink:
            # not saving failing features - so only fetch good features
            req = QgsFeatureRequest().setFilterExpression(expression_string)
            req.setExpressionContext(expression_context)

            for f in source.getFeatures(req):
                if feedback.isCanceled():
                    break
                matching_sink.addFeature(f)
        else:
            # saving non-matching features, so we need EVERYTHING
            expression_context.setFields(source.fields())
            expression.prepare(expression_context)

            total = 100.0 / source.featureCount()

            for current, f in enumerate(source.getFeatures()):
                if feedback.isCanceled():
                    break

                expression_context.setFeature(f)
                if expression.evaluate(expression_context):
                    matching_sink.addFeature(f)
                else:
                    nonmatching_sink.addFeature(f)

                feedback.setProgress(int(current * total))

        results = {self.OUTPUT: matching_sink_id}
        if nonmatching_sink:
            results[self.FAIL_OUTPUT] = non_matching_sink_id
        return results
