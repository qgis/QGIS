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

from PyQt4.QtCore import *

from qgis.core import *

from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.core.QGisLayers import QGisLayers

from sextante.parameters.ParameterVector import ParameterVector
from sextante.parameters.ParameterString import ParameterString
from sextante.parameters.ParameterNumber import ParameterNumber
from sextante.parameters.ParameterSelection import ParameterSelection

from sextante.outputs.OutputVector import OutputVector

class FieldsCalculator(GeoAlgorithm):

    INPUT_LAYER = "INPUT_LAYER"
    FIELD_NAME = "FIELD_NAME"
    FIELD_TYPE = "FIELD_TYPE"
    FIELD_LENGTH = "FIELD_LENGTH"
    FIELD_PRECISION = "FIELD_PRECISION"
    FORMULA = "FORMULA"
    OUTPUT_LAYER = "OUTPUT_LAYER"

    TYPE_NAMES = ["Integer", "Float", "String"]
    TYPES = [QVariant.Int, QVariant.Double, QVariant.String]

    #===========================================================================
    # def getIcon(self):
    #    return QtGui.QIcon(os.path.dirname(__file__) + "/../images/qgis.png")
    #===========================================================================

    def defineCharacteristics(self):
        self.name = "Field calculator"
        self.group = "Vector table tools"
        self.addParameter(ParameterVector(self.INPUT_LAYER, "Input layer", ParameterVector.VECTOR_TYPE_ANY, False))
        self.addParameter(ParameterString(self.FIELD_NAME, "Result field name"))
        self.addParameter(ParameterSelection(self.FIELD_TYPE, "Field type", self.TYPE_NAMES))
        self.addParameter(ParameterNumber(self.FIELD_LENGTH, "Field lenght", 1, 255, 10))
        self.addParameter(ParameterNumber(self.FIELD_PRECISION, "Field precision", 0, 10, 0))
        self.addParameter(ParameterString(self.FORMULA, "Formula"))
        self.addOutput(OutputVector(self.OUTPUT_LAYER, "Output layer"))

    def processAlgorithm(self, progress):
        fieldName = self.getParameterValue(self.FIELD_NAME)
        fieldType = self.getParameterValue(self.FIELD_TYPE)
        fieldLength = self.getParameterValue(self.FIELD_LENGTH)
        fieldPrecision = self.getParameterValue(self.FIELD_PRECISION)
        formula = self.getParameterValue(self.FORMULA)
        output = self.getOutputFromName(self.OUTPUT_LAYER)

        layer = QGisLayers.getObjectFromUri(self.getParameterValue(self.INPUT_LAYER))
        provider = layer.dataProvider()
        fields = provider.fields()
        fields.append(QgsField(fieldName, self.TYPES[fieldType], "", fieldLength, fieldPrecision))
        writer = output.getVectorWriter(fields, provider.geometryType(), layer.crs())

        outFeat = QgsFeature()
        inGeom = QgsGeometry()
        nFeat = provider.featureCount()
        nElement = 0
        features = QGisLayers.features(layer)
        for inFeat in features:
            progress.setPercentage(int((100 * nElement) / nFeat))
            attrs = inFeat.attributes()
            expression = formula
            k = 0
            for attr in attrs:
                expression = expression.replace(unicode(fields[k].name()), unicode(attr.toString()))
                k += 1
            try:
                result = eval(expression)
            except Exception:
                result = None
                #raise GeoAlgorithmExecutionException("Problem evaluation formula: Wrong field values or formula")
            nElement += 1
            inGeom = inFeat.geometry()
            outFeat.setGeometry(inGeom)
            attrs = inFeat.attributes()
            attrs.append(QVariant(result))
            outFeat.setAttributes(attrs)
            writer.addFeature(outFeat)
        del writer

    def checkParameterValuesBeforeExecuting(self):
        ##TODO check that formula is correct and fields exist
        pass
