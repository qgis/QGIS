# -*- coding: utf-8 -*-

"""
***************************************************************************
    JoinAttributes.py
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
from builtins import str

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.core import (QgsFeature,
                       QgsFeatureSink,
                       QgsApplication,
                       QgsProcessingUtils)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterTable
from processing.core.parameters import ParameterTableField
from processing.core.outputs import OutputVector
from processing.tools import vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class JoinAttributes(QgisAlgorithm):

    OUTPUT_LAYER = 'OUTPUT_LAYER'
    INPUT_LAYER = 'INPUT_LAYER'
    INPUT_LAYER_2 = 'INPUT_LAYER_2'
    TABLE_FIELD = 'TABLE_FIELD'
    TABLE_FIELD_2 = 'TABLE_FIELD_2'

    def icon(self):
        return QgsApplication.getThemeIcon("/providerQgis.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("providerQgis.svg")

    def group(self):
        return self.tr('Vector general tools')

    def __init__(self):
        super().__init__()
        self.addParameter(ParameterVector(self.INPUT_LAYER,
                                          self.tr('Input layer')))
        self.addParameter(ParameterTable(self.INPUT_LAYER_2,
                                         self.tr('Input layer 2'), False))
        self.addParameter(ParameterTableField(self.TABLE_FIELD,
                                              self.tr('Table field'), self.INPUT_LAYER))
        self.addParameter(ParameterTableField(self.TABLE_FIELD_2,
                                              self.tr('Table field 2'), self.INPUT_LAYER_2))
        self.addOutput(OutputVector(self.OUTPUT_LAYER,
                                    self.tr('Joined layer')))

    def name(self):
        return 'joinattributestable'

    def displayName(self):
        return self.tr('Join attributes table')

    def processAlgorithm(self, parameters, context, feedback):
        input = self.getParameterValue(self.INPUT_LAYER)
        input2 = self.getParameterValue(self.INPUT_LAYER_2)
        output = self.getOutputFromName(self.OUTPUT_LAYER)
        field = self.getParameterValue(self.TABLE_FIELD)
        field2 = self.getParameterValue(self.TABLE_FIELD_2)

        layer = QgsProcessingUtils.mapLayerFromString(input, context)
        joinField1Index = layer.fields().lookupField(field)

        layer2 = QgsProcessingUtils.mapLayerFromString(input2, context)
        joinField2Index = layer2.fields().lookupField(field2)

        outFields = vector.combineVectorFields(layer, layer2)
        writer = output.getVectorWriter(outFields, layer.wkbType(), layer.crs(), context)

        # Cache attributes of Layer 2
        cache = {}
        features = QgsProcessingUtils.getFeatures(layer2, context)
        total = 100.0 / layer2.featureCount() if layer2.featureCount() else 0
        for current, feat in enumerate(features):
            attrs = feat.attributes()
            joinValue2 = str(attrs[joinField2Index])
            if joinValue2 not in cache:
                cache[joinValue2] = attrs
            feedback.setProgress(int(current * total))

        # Create output vector layer with additional attribute
        outFeat = QgsFeature()
        features = QgsProcessingUtils.getFeatures(layer, context)
        total = 100.0 / layer.featureCount() if layer.featureCount() else 0
        for current, feat in enumerate(features):
            outFeat.setGeometry(feat.geometry())
            attrs = feat.attributes()
            joinValue1 = str(attrs[joinField1Index])
            attrs.extend(cache.get(joinValue1, []))
            outFeat.setAttributes(attrs)
            writer.addFeature(outFeat, QgsFeatureSink.FastInsert)
            feedback.setProgress(int(current * total))
        del writer
