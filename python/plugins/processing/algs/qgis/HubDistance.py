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

from PyQt4.QtCore import *
from qgis.core import *
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import \
        GeoAlgorithmExecutionException
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterTableField
from processing.core.parameters import ParameterSelection
from processing.core.outputs import OutputVector

from processing.tools import dataobjects, vector

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
        self.name = 'Distance to nearest hub'
        self.group = 'Vector analysis tools'

        self.addParameter(ParameterVector(self.POINTS,
            self.tr('Source points layer'), [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterVector(self.HUBS,
            self.tr('Destination hubs layer'), [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterTableField(self.FIELD,
            self.tr('Hub layer name attribute'), self.HUBS))
        self.addParameter(ParameterSelection(self.GEOMETRY,
            self.tr('Output shape type'), self.GEOMETRIES))
        self.addParameter(ParameterSelection(self.UNIT,
            self.tr('Measurement unit'), self.UNITS))

        self.addOutput(OutputVector(self.OUTPUT, self.tr('Output')))

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

        # Create array of hubs in memory
        hubs = []
        features = vector.features(layerHubs)
        for f in features:
            hubs.append(Hub(f.geometry().boundingBox().center(),
                            unicode(f[fieldName])))

        distance = QgsDistanceArea()
        distance.setSourceCrs(layerPoints.crs().srsid())
        distance.setEllipsoidalMode(True)

        # Scan source points, find nearest hub, and write to output file
        features = vector.features(layerPoints)
        count = len(features)
        total = 100.0 / float(count)
        for count, f in enumerate(features):
            src = f.geometry().boundingBox().center()

            closest = hubs[0]
            hubDist = distance.measureLine(src, closest.point)

            for hub in hubs:
                dist = distance.measureLine(src, hub.point)
                if dist < hubDist:
                    closest = hub
                    hubDist = dist

            attributes = f.attributes()
            attributes.append(closest.name)
            if units == 'Feet':
                attributes.append(hubDist * 3.2808399)
            elif units == 'Miles':
                attributes.append(hubDist * 0.000621371192)
            elif units == 'Kilometers':
                attributes.append(hubDist / 1000.0)
            elif units != 'Meters':
                attributes.append(sqrt(
                    pow(source.x() - closest.point.x(), 2.0) +
                    pow(source.y() - closest.point.y(), 2.0)))
            else:
                attributes.append(hubDist)

            feat = QgsFeature()
            feat.setAttributes(attributes)

            if geomType == QGis.WKBPoint:
                feat.setGeometry(QgsGeometry.fromPoint(src))
            else:
                feat.setGeometry(QgsGeometry.fromPolyline([src, closest.point]))

            writer.addFeature(feat)
            progress.setPercentage(int(count * total))

        del writer


class Hub:
    def __init__(self, point, name):
        self.point = point
        self.name = name
