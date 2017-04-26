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
                       QgsApplication)

from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import ParameterVector
from processing.core.outputs import OutputVector
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterExpression
from processing.tools import dataobjects


class ExtractByExpression(GeoAlgorithm):

    INPUT = 'INPUT'
    EXPRESSION = 'EXPRESSION'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QgsApplication.getThemeIcon("/providerQgis.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("providerQgis.svg")

    def tags(self):
        return self.tr('extract,filter,expression,field').split(',')

    def group(self):
        return self.tr('Vector selection tools')

    def name(self):
        return 'extractbyexpression'

    def displayName(self):
        return self.tr('Extract by expression')

    def defineCharacteristics(self):
        self.addParameter(ParameterVector(self.INPUT,
                                          self.tr('Input Layer')))
        self.addParameter(ParameterExpression(self.EXPRESSION,
                                              self.tr("Expression"), parent_layer=self.INPUT))
        self.addOutput(OutputVector(self.OUTPUT, self.tr('Extracted (expression)')))

    def processAlgorithm(self, context, feedback):
        layer = dataobjects.getLayerFromString(self.getParameterValue(self.INPUT))
        expression_string = self.getParameterValue(self.EXPRESSION)
        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(layer.fields(), layer.wkbType(), layer.crs())

        expression = QgsExpression(expression_string)
        if not expression.hasParserError():
            req = QgsFeatureRequest().setFilterExpression(expression_string)
        else:
            raise GeoAlgorithmExecutionException(expression.parserErrorString())

        for f in layer.getFeatures(req):
            writer.addFeature(f)

        del writer
