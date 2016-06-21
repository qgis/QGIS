# -*- coding: utf-8 -*-

"""
***************************************************************************
    Gridify.py
    ---------------------
    Date                 : May 2010
    Copyright            : (C) 2010 by Michael Minn
    Email                : pyqgis at michaelminn dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Michael Minn'
__date__ = 'May 2010'
__copyright__ = '(C) 2010, Michael Minn'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.PyQt.QtCore import QVariant
from qgis.core import QGis, QgsField, QgsGeometry, QgsDistanceArea, QgsFeature, QgsFeatureRequest
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterTableField
from processing.core.parameters import ParameterSelection
from processing.core.outputs import OutputVector

from processing.tools import dataobjects, vector

from math import sqrt


class HubDistance(GeoAlgorithm):
    POINTS = 'POINTS'
    HUBS = 'HUBS'
    FIELD = 'FIELD'
    GEOMETRY = 'GEOMETRY'
    UNIT = 'UNIT'
    OUTPUT = 'OUTPUT'

    GEOMETRIES = ['Point',
                  'Line to hub'
                  ]

    UNITS = ['Meters',
             'Feet',
             'Miles',
             'Kilometers',
             'Layer units'
             ]

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Distance to nearest hub')
        self.group, self.i18n_group = self.trAlgorithm('Vector analysis tools')

        self.units = [self.tr('Meters'),
                      self.tr('Feet'),
                      self.tr('Miles'),
                      self.tr('Kilometers'),
                      self.tr('Layer units')]

        self.addParameter(ParameterVector(self.POINTS,
                                          self.tr('Source points layer'), [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterVector(self.HUBS,
                                          self.tr('Destination hubs layer'), [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterTableField(self.FIELD,
                                              self.tr('Hub layer name attribute'), self.HUBS))
        self.addParameter(ParameterSelection(self.GEOMETRY,
                                             self.tr('Output shape type'), self.GEOMETRIES))
        self.addParameter(ParameterSelection(self.UNIT,
                                             self.tr('Measurement unit'), self.units))

        self.addOutput(OutputVector(self.OUTPUT, self.tr('Hub distance')))

    def processAlgorithm(self, progress):
        layerPoints = dataobjects.getObjectFromUri(
            self.getParameterValue(self.POINTS))
        layerHubs = dataobjects.getObjectFromUri(
            self.getParameterValue(self.HUBS))
        fieldName = self.getParameterValue(self.FIELD)

        addLines = self.getParameterValue(self.GEOMETRY)
        units = self.UNITS[self.getParameterValue(self.UNIT)]

        if layerPoints.source() == layerHubs.source():
            raise GeoAlgorithmExecutionException(
                self.tr('Same layer given for both hubs and spokes'))

        geomType = QGis.WKBPoint
        if addLines:
            geomType = QGis.WKBLineString

        fields = layerPoints.pendingFields()
        fields.append(QgsField('HubName', QVariant.String))
        fields.append(QgsField('HubDist', QVariant.Double))

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(
            fields, geomType, layerPoints.crs())

        index = vector.spatialindex(layerHubs)

        distance = QgsDistanceArea()
        distance.setSourceCrs(layerPoints.crs().srsid())
        distance.setEllipsoidalMode(True)

        # Scan source points, find nearest hub, and write to output file
        features = vector.features(layerPoints)
        total = 100.0 / len(features)
        for current, f in enumerate(features):
            src = f.geometry().boundingBox().center()

            neighbors = index.nearestNeighbor(src, 1)
            ft = layerHubs.getFeatures(QgsFeatureRequest().setFilterFid(neighbors[0])).next()
            closest = ft.geometry().boundingBox().center()
            hubDist = distance.measureLine(src, closest)

            attributes = f.attributes()
            attributes.append(ft[fieldName])
            if units == 'Feet':
                attributes.append(hubDist * 3.2808399)
            elif units == 'Miles':
                attributes.append(hubDist * 0.000621371192)
            elif units == 'Kilometers':
                attributes.append(hubDist / 1000.0)
            elif units != 'Meters':
                attributes.append(sqrt(
                    pow(src.x() - closest.x(), 2.0) +
                    pow(src.y() - closest.y(), 2.0)))
            else:
                attributes.append(hubDist)

            feat = QgsFeature()
            feat.setAttributes(attributes)

            if geomType == QGis.WKBPoint:
                feat.setGeometry(QgsGeometry.fromPoint(src))
            else:
                feat.setGeometry(QgsGeometry.fromPolyline([src, closest]))

            writer.addFeature(feat)
            progress.setPercentage(int(current * total))

        del writer
