# -*- coding: utf-8 -*-

"""
***************************************************************************
    EquivalentNumField.py
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
from qgis.core import QgsField, QgsFeature
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterTableField
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class EquivalentNumField(GeoAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    FIELD = 'FIELD'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Add unique value index field')
        self.group, self.i18n_group = self.trAlgorithm('Vector table tools')
        self.addParameter(ParameterVector(self.INPUT,
                                          self.tr('Input layer'), [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterTableField(self.FIELD,
                                              self.tr('Class field'), self.INPUT))
        self.addOutput(OutputVector(self.OUTPUT, self.tr('Layer with index field')))

    def processAlgorithm(self, progress):
        fieldname = self.getParameterValue(self.FIELD)
        output = self.getOutputFromName(self.OUTPUT)
        vlayer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT))
        vprovider = vlayer.dataProvider()
        fieldindex = vlayer.fieldNameIndex(fieldname)
        fields = vprovider.fields()
        fields.append(QgsField('NUM_FIELD', QVariant.Int))
        writer = output.getVectorWriter(fields, vprovider.geometryType(),
                                        vlayer.crs())
        outFeat = QgsFeature()
        classes = {}

        features = vector.features(vlayer)
        total = 100.0 / len(features)
        for current, feature in enumerate(features):
            progress.setPercentage(int(current * total))
            inGeom = feature.geometry()
            outFeat.setGeometry(inGeom)
            atMap = feature.attributes()
            clazz = atMap[fieldindex]

            if clazz not in classes:
                classes[clazz] = len(classes.keys())

            atMap.append(classes[clazz])
            outFeat.setAttributes(atMap)
            writer.addFeature(outFeat)

        del writer
