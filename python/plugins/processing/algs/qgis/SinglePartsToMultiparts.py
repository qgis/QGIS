# -*- coding: utf-8 -*-

"""
***************************************************************************
    SinglePartsToMultiparts.py
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

from qgis.PyQt.QtGui import QIcon

from qgis.core import QgsFeature, QgsGeometry, QgsWkbTypes, QgsProcessingUtils

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterTableField
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class SinglePartsToMultiparts(GeoAlgorithm):

    INPUT = 'INPUT'
    FIELD = 'FIELD'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'single_to_multi.png'))

    def group(self):
        return self.tr('Vector geometry tools')

    def name(self):
        return 'singlepartstomultipart'

    def displayName(self):
        return self.tr('Singleparts to multipart')

    def defineCharacteristics(self):
        self.addParameter(ParameterVector(self.INPUT, self.tr('Input layer')))
        self.addParameter(ParameterTableField(self.FIELD,
                                              self.tr('Unique ID field'), self.INPUT))

        self.addOutput(OutputVector(self.OUTPUT, self.tr('Multipart')))

    def processAlgorithm(self, context, feedback):
        layer = dataobjects.getLayerFromString(self.getParameterValue(self.INPUT))
        fieldName = self.getParameterValue(self.FIELD)

        geomType = QgsWkbTypes.multiType(layer.wkbType())

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(
            layer.fields().toList(), geomType, layer.crs())

        outFeat = QgsFeature()
        inGeom = QgsGeometry()

        index = layer.fields().lookupField(fieldName)

        collection_geom = {}
        collection_attrs = {}

        features = QgsProcessingUtils.getFeatures(layer, context)
        total = 100.0 / QgsProcessingUtils.featureCount(layer, context)
        for current, feature in enumerate(features):
            atMap = feature.attributes()
            idVar = atMap[index]
            key = str(idVar).strip()
            if key not in collection_geom:
                collection_geom[key] = []
                collection_attrs[key] = atMap

            inGeom = feature.geometry()
            collection_geom[key].append(inGeom)

            feedback.setProgress(int(current * total))

        for key, geoms in collection_geom.items():
            outFeat.setAttributes(collection_attrs[key])
            outFeat.setGeometry(QgsGeometry.collectGeometry(geoms))
            writer.addFeature(outFeat)

        del writer
