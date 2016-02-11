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

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import QgsFeature

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterTable
from processing.core.parameters import ParameterTableField
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class JoinAttributes(GeoAlgorithm):

    OUTPUT_LAYER = 'OUTPUT_LAYER'
    INPUT_LAYER = 'INPUT_LAYER'
    INPUT_LAYER_2 = 'INPUT_LAYER_2'
    TABLE_FIELD = 'TABLE_FIELD'
    TABLE_FIELD_2 = 'TABLE_FIELD_2'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Join attributes table')
        self.group, self.i18n_group = self.trAlgorithm('Vector general tools')
        self.addParameter(ParameterVector(self.INPUT_LAYER,
                                          self.tr('Input layer'), [ParameterVector.VECTOR_TYPE_ANY], False))
        self.addParameter(ParameterTable(self.INPUT_LAYER_2,
                                         self.tr('Input layer 2'), False))
        self.addParameter(ParameterTableField(self.TABLE_FIELD,
                                              self.tr('Table field'), self.INPUT_LAYER))
        self.addParameter(ParameterTableField(self.TABLE_FIELD_2,
                                              self.tr('Table field 2'), self.INPUT_LAYER_2))
        self.addOutput(OutputVector(self.OUTPUT_LAYER,
                                    self.tr('Joined layer')))

    def processAlgorithm(self, progress):
        input = self.getParameterValue(self.INPUT_LAYER)
        input2 = self.getParameterValue(self.INPUT_LAYER_2)
        output = self.getOutputFromName(self.OUTPUT_LAYER)
        field = self.getParameterValue(self.TABLE_FIELD)
        field2 = self.getParameterValue(self.TABLE_FIELD_2)

        # Layer 1
        layer = dataobjects.getObjectFromUri(input)
        provider = layer.dataProvider()
        joinField1Index = layer.fieldNameIndex(field)

        # Layer 2
        layer2 = dataobjects.getObjectFromUri(input2)

        joinField2Index = layer2.fieldNameIndex(field2)

        # Output
        outFields = vector.combineVectorFields(layer, layer2)

        writer = output.getVectorWriter(outFields, provider.geometryType(),
                                        layer.crs())

        inFeat = QgsFeature()
        inFeat2 = QgsFeature()
        outFeat = QgsFeature()

        # Cache attributes of Layer 2
        cache = {}
        features2 = vector.features(layer2)
        for inFeat2 in features2:
            attrs2 = inFeat2.attributes()
            joinValue2 = unicode(attrs2[joinField2Index])
            # Put the attributes into the dict if the join key is not contained in the keys of the dict.
            # Note: This behavior is same as previous behavior of this function,
            # but different from the attribute cache function of QGIS core.
            if joinValue2 not in cache:
                cache[joinValue2] = attrs2

        # Create output vector layer with additional attribute
        features = vector.features(layer)
        for inFeat in features:
            outFeat.setGeometry(inFeat.geometry())
            attrs = inFeat.attributes()
            joinValue1 = unicode(attrs[joinField1Index])
            attrs.extend(cache.get(joinValue1, []))
            outFeat.setAttributes(attrs)
            writer.addFeature(outFeat)
        del writer
