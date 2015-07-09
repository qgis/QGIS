# -*- coding: utf-8 -*-

"""
***************************************************************************
    AddTableField.py
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

from PyQt4.QtCore import QVariant
from qgis.core import QgsField, QgsFeature, QgsGeometry
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterString
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterSelection
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class AddTableField(GeoAlgorithm):

    OUTPUT_LAYER = 'OUTPUT_LAYER'
    INPUT_LAYER = 'INPUT_LAYER'
    FIELD_NAME = 'FIELD_NAME'
    FIELD_TYPE = 'FIELD_TYPE'
    FIELD_LENGTH = 'FIELD_LENGTH'
    FIELD_PRECISION = 'FIELD_PRECISION'

    TYPE_NAMES = ['Integer', 'Float', 'String']
    TYPES = [QVariant.Int, QVariant.Double, QVariant.String]

    def defineCharacteristics(self):
        self.name = 'Add field to attributes table'
        self.group = 'Vector table tools'

        self.addParameter(ParameterVector(self.INPUT_LAYER,
            self.tr('Input layer'), [ParameterVector.VECTOR_TYPE_ANY], False))
        self.addParameter(ParameterString(self.FIELD_NAME,
            self.tr('Field name')))
        self.addParameter(ParameterSelection(self.FIELD_TYPE,
            self.tr('Field type'), self.TYPE_NAMES))
        self.addParameter(ParameterNumber(self.FIELD_LENGTH,
            self.tr('Field length'), 1, 255, 10))
        self.addParameter(ParameterNumber(self.FIELD_PRECISION,
            self.tr('Field precision'), 0, 10, 0))
        self.addOutput(OutputVector(
            self.OUTPUT_LAYER, self.tr('Added')))

    def processAlgorithm(self, progress):
        fieldType = self.getParameterValue(self.FIELD_TYPE)
        fieldName = self.getParameterValue(self.FIELD_NAME)
        fieldLength = self.getParameterValue(self.FIELD_LENGTH)
        fieldPrecision = self.getParameterValue(self.FIELD_PRECISION)
        output = self.getOutputFromName(self.OUTPUT_LAYER)

        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT_LAYER))

        provider = layer.dataProvider()
        fields = provider.fields()
        fields.append(QgsField(fieldName, self.TYPES[fieldType], '',
                      fieldLength, fieldPrecision))
        writer = output.getVectorWriter(fields, provider.geometryType(),
                layer.crs())
        outFeat = QgsFeature()
        inGeom = QgsGeometry()
        nElement = 0
        features = vector.features(layer)
        nFeat = len(features)
        for inFeat in features:
            progress.setPercentage(int(100 * nElement / nFeat))
            nElement += 1
            inGeom = inFeat.geometry()
            outFeat.setGeometry(inGeom)
            atMap = inFeat.attributes()
            atMap.append(None)
            outFeat.setAttributes(atMap)
            writer.addFeature(outFeat)
        del writer
