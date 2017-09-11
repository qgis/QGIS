# -*- coding: utf-8 -*-

"""
***************************************************************************
    ExtractByExpression.py
    ---------------------
    Date                 : September 2017
    Copyright            : (C) 2017 by Alexander Bruy
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

__author__ = 'Alexander Bruy'
__date__ = 'September 2017'
__copyright__ = '(C) 2017, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import QgsExpression, QgsExpressionContext, QgsExpressionContextUtils, QgsFeatureRequest
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterString
from processing.core.outputs import OutputVector
from processing.tools import dataobjects


class ExtractByExpression(GeoAlgorithm):

    INPUT = 'INPUT'
    EXPRESSION = 'EXPRESSION'
    OUTPUT = 'OUTPUT'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Extract by expression')
        self.group, self.i18n_group = self.trAlgorithm('Vector selection tools')

        self.addParameter(ParameterVector(self.INPUT,
                                          self.tr('Input Layer'), [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterString(self.EXPRESSION,
                                          self.tr("Expression")))
        self.addOutput(OutputVector(self.OUTPUT, self.tr('Extracted (expression)')))

    def processAlgorithm(self, progress):
        layer = layer = dataobjects.getObjectFromUri(self.getParameterValue(self.INPUT))

        expression = self.getParameterValue(self.EXPRESSION)
        qExp = QgsExpression(expression)
        if qExp.hasParserError():
            raise GeoAlgorithmExecutionException(qExp.parserErrorString())

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(
            layer.fields(), layer.wkbType(), layer.crs())

        context = QgsExpressionContext()
        context.appendScope(QgsExpressionContextUtils.globalScope())
        context.appendScope(QgsExpressionContextUtils.projectScope())
        context.appendScope(QgsExpressionContextUtils.layerScope(layer))

        count = layer.featureCount()
        step = 100.0 / count if count else 1

        request = QgsFeatureRequest(qExp, context)

        for current, f in enumerate(layer.getFeatures(request)):
            writer.addFeature(f)
            progress.setPercentage(int(current * step))

        del writer
