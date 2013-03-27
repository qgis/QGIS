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

from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.outputs.OutputVector import OutputVector
from sextante.parameters.ParameterVector import ParameterVector
from qgis.core import *
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from sextante.parameters.ParameterString import ParameterString
from sextante.core.QGisLayers import QGisLayers


class FieldsCalculator(GeoAlgorithm):

    INPUT_LAYER = "INPUT_LAYER"
    FIELD_NAME = "FIELD_NAME"
    FORMULA = "FORMULA"
    OUTPUT_LAYER = "OUTPUT_LAYER"

    #===========================================================================
    # def getIcon(self):
    #    return QtGui.QIcon(os.path.dirname(__file__) + "/../images/qgis.png")
    #===========================================================================

    def defineCharacteristics(self):
        self.name = "Field calculator"
        self.group = "Vector table tools"
        self.addParameter(ParameterVector(self.INPUT_LAYER, "Input layer", ParameterVector.VECTOR_TYPE_ANY, False))
        self.addParameter(ParameterString(self.FIELD_NAME, "Result field name"))
        self.addParameter(ParameterString(self.FORMULA, "Formula"))
        self.addOutput(OutputVector(self.OUTPUT_LAYER, "Output layer"))

    def processAlgorithm(self, progress):
        fieldname = self.getParameterValue(self.FIELD_NAME)
        formula = self.getParameterValue(self.FORMULA)
        output = self.getOutputFromName(self.OUTPUT_LAYER)
        vlayer = QGisLayers.getObjectFromUri(self.getParameterValue(self.INPUT_LAYER))
        vprovider = vlayer.dataProvider()
        fields = vprovider.fields()
        fields.append(QgsField(fieldname, QVariant.Double))
        writer = output.getVectorWriter(fields, vprovider.geometryType(), vlayer.crs())
        outFeat = QgsFeature()
        inGeom = QgsGeometry()
        nFeat = vprovider.featureCount()
        nElement = 0
        features = QGisLayers.features(vlayer)
        for inFeat in features:
            progress.setPercentage(int((100 * nElement) / nFeat))
            attrs = inFeat.attributes()
            expression = formula
            k = 0
            for attr in attrs:
                expression = expression.replace(str(fields[k].name()), str(attr.toString()))
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

