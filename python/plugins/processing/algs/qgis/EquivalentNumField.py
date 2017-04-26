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
from qgis.core import (QgsField,
                       QgsFeature,
                       QgsApplication,
                       QgsProcessingUtils)
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterTableField
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class EquivalentNumField(GeoAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    FIELD = 'FIELD'

    def icon(self):
        return QgsApplication.getThemeIcon("/providerQgis.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("providerQgis.svg")

    def group(self):
        return self.tr('Vector table tools')

    def name(self):
        return 'adduniquevalueindexfield'

    def displayName(self):
        return self.tr('Add unique value index field')

    def defineCharacteristics(self):
        self.addParameter(ParameterVector(self.INPUT,
                                          self.tr('Input layer')))
        self.addParameter(ParameterTableField(self.FIELD,
                                              self.tr('Class field'), self.INPUT))
        self.addOutput(OutputVector(self.OUTPUT, self.tr('Layer with index field')))

    def processAlgorithm(self, context, feedback):
        fieldname = self.getParameterValue(self.FIELD)
        output = self.getOutputFromName(self.OUTPUT)
        vlayer = dataobjects.getLayerFromString(
            self.getParameterValue(self.INPUT))
        fieldindex = vlayer.fields().lookupField(fieldname)
        fields = vlayer.fields()
        fields.append(QgsField('NUM_FIELD', QVariant.Int))
        writer = output.getVectorWriter(fields, vlayer.wkbType(),
                                        vlayer.crs())
        outFeat = QgsFeature()
        classes = {}

        features = QgsProcessingUtils.getFeatures(vlayer, context)
        total = 100.0 / QgsProcessingUtils.featureCount(vlayer, context)
        for current, feature in enumerate(features):
            feedback.setProgress(int(current * total))
            inGeom = feature.geometry()
            outFeat.setGeometry(inGeom)
            atMap = feature.attributes()
            clazz = atMap[fieldindex]

            if clazz not in classes:
                classes[clazz] = len(list(classes.keys()))

            atMap.append(classes[clazz])
            outFeat.setAttributes(atMap)
            writer.addFeature(outFeat)

        del writer
