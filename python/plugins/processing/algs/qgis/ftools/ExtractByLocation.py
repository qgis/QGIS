# -*- coding: utf-8 -*-

"""
***************************************************************************
    ExtractByLocation.py
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
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.parameters.ParameterVector import ParameterVector
from processing.outputs.OutputVector import OutputVector
from processing.tools import dataobjects, vector


class ExtractByLocation(GeoAlgorithm):

    INPUT = 'INPUT'
    INTERSECT = 'INTERSECT'
    OUTPUT = 'OUTPUT'

    def defineCharacteristics(self):
        self.name = 'Extract by location'
        self.group = 'Vector selection tools'
        self.addParameter(ParameterVector(self.INPUT, 'Layer to select from',
                          [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterVector(self.INTERSECT,
                          'Additional layer (intersection layer)',
                          [ParameterVector.VECTOR_TYPE_ANY]))
        self.addOutput(OutputVector(self.OUTPUT, 'Selection'))

    def processAlgorithm(self, progress):
        filename = self.getParameterValue(self.INPUT)
        layer = dataobjects.getObjectFromUri(filename)
        filename = self.getParameterValue(self.INTERSECT)
        selectLayer = dataobjects.getObjectFromUri(filename)
        index = vector.spatialindex(layer)

        geom = QgsGeometry()
        selectedSet = []
        current = 0
        features = vector.features(selectLayer)
        featureCount = len(features)
        total = 100.0 / float(len(features))
        for current,f in enumerate(features):
            geom = QgsGeometry(f.geometry())
            intersects = index.intersects(geom.boundingBox())
            for i in intersects:
                request = QgsFeatureRequest().setFilterFid(i)
                feat = layer.getFeatures(request).next()
                tmpGeom = QgsGeometry(feat.geometry())
                if geom.intersects(tmpGeom):
                    selectedSet.append(feat.id())
            progress.setPercentage(int(current * total))

        output = self.getOutputFromName(self.OUTPUT)
        writer = output.getVectorWriter(layer.layer.pendingFields().toList(),
                layer.geometryType(), layer.crs())

        for (i, feat) in enumerate(features):
            if feat.id() in selectedSet:
                writer.addFeature(feat)
            progress.setPercentage(100 * i / float(featureCount))
        del writer
