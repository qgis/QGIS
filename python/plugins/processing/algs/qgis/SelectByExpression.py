# -*- coding: utf-8 -*-

"""
***************************************************************************
    SelectByExpression.py
    ---------------------
    Date                 : July 2014
    Copyright            : (C) 2014 by Michaël Douchin
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Michael Douchin'
__date__ = 'July 2014'
__copyright__ = '(C) 2014, Michael Douchin'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import processing
from qgis.core import QgsExpression, QgsFeatureRequest
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterSelection
from processing.core.outputs import OutputVector
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterString

class SelectByExpression(GeoAlgorithm):

    LAYERNAME = 'LAYERNAME'
    EXPRESSION= 'EXPRESSION'
    RESULT = 'RESULT'
    METHOD = 'METHOD'
    METHODS = ['creating new selection', 'adding to current selection',
               'removing from current selection']

    def defineCharacteristics(self):
        self.name = 'Select by expression'
        self.group = 'Vector selection tools'

        self.addParameter(ParameterVector(self.LAYERNAME,
            self.tr('Input Layer'), [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterString(self.EXPRESSION,
            self.tr("Expression")))
        self.addParameter(ParameterSelection(self.METHOD,
            self.tr('Modify current selection by'), self.METHODS, 0))
        self.addOutput(OutputVector(self.RESULT, self.tr('Output'), True))

    def processAlgorithm(self, progress):
        filename = self.getParameterValue(self.LAYERNAME)
        layer = processing.getObject(filename)
        oldSelection = set(layer.selectedFeaturesIds())
        method = self.getParameterValue(self.METHOD)

        # Build QGIS request with expression
        expression = self.getParameterValue(self.EXPRESSION)
        qExp = QgsExpression(expression)
        if not qExp.hasParserError():
            qReq = QgsFeatureRequest(qExp)
        else:
            raise GeoAlgorithmExecutionException(qExp.parserErrorString())
        selected = [f.id() for f in layer.getFeatures(qReq)]

        if method == 1:
            selected = list(oldSelection.union(selected))
        elif method == 2:
            selected = list(oldSelection.difference(selected))

        # Set the selection
        layer.setSelectedFeatures(selected)

        self.setOutputValue(self.RESULT, filename)
