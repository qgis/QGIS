# -*- coding: utf-8 -*-

"""
***************************************************************************
    PointsInPolygon.py
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

import os

from qgis.PyQt.QtGui import QIcon
from qgis.PyQt.QtCore import QVariant

from qgis.core import QgsGeometry, QgsFeatureRequest, QgsFeature, QgsField, QgsProcessingUtils

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterString
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class PointsInPolygon(GeoAlgorithm):

    POLYGONS = 'POLYGONS'
    POINTS = 'POINTS'
    OUTPUT = 'OUTPUT'
    FIELD = 'FIELD'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'sum_points.png'))

    def group(self):
        return self.tr('Vector analysis tools')

    def name(self):
        return 'countpointsinpolygon'

    def displayName(self):
        return self.tr('Count points in polygon')

    def defineCharacteristics(self):
        self.addParameter(ParameterVector(self.POLYGONS,
                                          self.tr('Polygons'), [dataobjects.TYPE_VECTOR_POLYGON]))
        self.addParameter(ParameterVector(self.POINTS,
                                          self.tr('Points'), [dataobjects.TYPE_VECTOR_POINT]))
        self.addParameter(ParameterString(self.FIELD,
                                          self.tr('Count field name'), 'NUMPOINTS'))
        self.addOutput(OutputVector(self.OUTPUT, self.tr('Count'), datatype=[dataobjects.TYPE_VECTOR_POLYGON]))

    def processAlgorithm(self, context, feedback):
        polyLayer = dataobjects.getLayerFromString(self.getParameterValue(self.POLYGONS))
        pointLayer = dataobjects.getLayerFromString(self.getParameterValue(self.POINTS))
        fieldName = self.getParameterValue(self.FIELD)

        fields = polyLayer.fields()
        fields.append(QgsField(fieldName, QVariant.Int))

        (idxCount, fieldList) = vector.findOrCreateField(polyLayer,
                                                         polyLayer.fields(), fieldName)

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(
            fields.toList(), polyLayer.wkbType(), polyLayer.crs())

        spatialIndex = vector.spatialindex(pointLayer)

        ftPoly = QgsFeature()
        ftPoint = QgsFeature()
        outFeat = QgsFeature()
        geom = QgsGeometry()

        features = QgsProcessingUtils.getFeatures(polyLayer, context)
        total = 100.0 / QgsProcessingUtils.featureCount(polyLayer, context)
        for current, ftPoly in enumerate(features):
            geom = ftPoly.geometry()
            engine = QgsGeometry.createGeometryEngine(geom.geometry())
            engine.prepareGeometry()

            attrs = ftPoly.attributes()

            count = 0
            points = spatialIndex.intersects(geom.boundingBox())
            if len(points) > 0:
                request = QgsFeatureRequest().setFilterFids(points).setSubsetOfAttributes([])
                fit = pointLayer.getFeatures(request)
                ftPoint = QgsFeature()
                while fit.nextFeature(ftPoint):
                    tmpGeom = ftPoint.geometry()
                    if engine.contains(tmpGeom.geometry()):
                        count += 1

            outFeat.setGeometry(geom)
            if idxCount == len(attrs):
                attrs.append(count)
            else:
                attrs[idxCount] = count
            outFeat.setAttributes(attrs)
            writer.addFeature(outFeat)

            feedback.setProgress(int(current * total))

        del writer
