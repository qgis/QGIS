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

from qgis.PyQt.QtCore import QVariant
from qgis.core import (QgsField,
                       QgsFeature,
                       QgsApplication,
                       QgsProcessingUtils)
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

    TYPES = [QVariant.Int, QVariant.Double, QVariant.String]

    def icon(self):
        return QgsApplication.getThemeIcon("/providerQgis.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("providerQgis.svg")

    def group(self):
        return self.tr('Vector table tools')

    def name(self):
        return 'addfieldtoattributestable'

    def displayName(self):
        return self.tr('Add field to attributes table')

    def defineCharacteristics(self):

        self.type_names = [self.tr('Integer'),
                           self.tr('Float'),
                           self.tr('String')]

        self.addParameter(ParameterVector(self.INPUT_LAYER,
                                          self.tr('Input layer')))
        self.addParameter(ParameterString(self.FIELD_NAME,
                                          self.tr('Field name')))
        self.addParameter(ParameterSelection(self.FIELD_TYPE,
                                             self.tr('Field type'), self.type_names))
        self.addParameter(ParameterNumber(self.FIELD_LENGTH,
                                          self.tr('Field length'), 1, 255, 10))
        self.addParameter(ParameterNumber(self.FIELD_PRECISION,
                                          self.tr('Field precision'), 0, 10, 0))
        self.addOutput(OutputVector(
            self.OUTPUT_LAYER, self.tr('Added')))

    def processAlgorithm(self, context, feedback):
        fieldType = self.getParameterValue(self.FIELD_TYPE)
        fieldName = self.getParameterValue(self.FIELD_NAME)
        fieldLength = self.getParameterValue(self.FIELD_LENGTH)
        fieldPrecision = self.getParameterValue(self.FIELD_PRECISION)
        output = self.getOutputFromName(self.OUTPUT_LAYER)

        layer = dataobjects.getLayerFromString(
            self.getParameterValue(self.INPUT_LAYER))

        fields = layer.fields()
        fields.append(QgsField(fieldName, self.TYPES[fieldType], '',
                               fieldLength, fieldPrecision))
        writer = output.getVectorWriter(fields, layer.wkbType(),
                                        layer.crs())
        outFeat = QgsFeature()
        features = QgsProcessingUtils.getFeatures(layer, context)
        total = 100.0 / QgsProcessingUtils.featureCount(layer, context)
        for current, feat in enumerate(features):
            feedback.setProgress(int(current * total))
            geom = feat.geometry()
            outFeat.setGeometry(geom)
            atMap = feat.attributes()
            atMap.append(None)
            outFeat.setAttributes(atMap)
            writer.addFeature(outFeat)
        del writer
