# -*- coding: utf-8 -*-

"""
***************************************************************************
    DensifyGeometriesInterval.py by Anita Graser, Dec 2012
    based on DensifyGeometries.py
    ---------------------
    Date                 : October 2012
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

__author__ = 'Anita Graser'
__date__ = 'Dec 2012'
__copyright__ = '(C) 2012, Anita Graser'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from math import sqrt

from qgis.core import QGis, QgsPoint, QgsGeometry, QgsFeature

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterNumber
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class DensifyGeometriesInterval(GeoAlgorithm):

    INPUT = 'INPUT'
    INTERVAL = 'INTERVAL'
    OUTPUT = 'OUTPUT'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Densify geometries given an interval')
        self.group, self.i18n_group = self.trAlgorithm('Vector geometry tools')

        self.addParameter(ParameterVector(self.INPUT,
            self.tr('Input layer'),
            [ParameterVector.VECTOR_TYPE_POLYGON, ParameterVector.VECTOR_TYPE_LINE]))
        self.addParameter(ParameterNumber(self.INTERVAL,
            self.tr('Interval between vertices to add'), 0.0, 10000000.0, 1.0))

        self.addOutput(OutputVector(self.OUTPUT, self.tr('Densified')))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(self.getParameterValue(self.INPUT))
        interval = self.getParameterValue(self.INTERVAL)

        isPolygon = layer.geometryType() == QGis.Polygon

        writer = self.getOutputFromName(
            self.OUTPUT).getVectorWriter(layer.pendingFields().toList(),
                                         layer.wkbType(), layer.crs())

        features = vector.features(layer)
        total = 100.0 / float(len(features))
        current = 0
        for f in features:
            featGeometry = QgsGeometry(f.geometry())
            attrs = f.attributes()
            newGeometry = self.densifyGeometry(featGeometry, interval,
                    isPolygon)
            feature = QgsFeature()
            feature.setGeometry(newGeometry)
            feature.setAttributes(attrs)
            writer.addFeature(feature)

            current += 1
            progress.setPercentage(int(current * total))

        del writer

    def densifyGeometry(self, geometry, interval, isPolygon):
        output = []
        if isPolygon:
            if geometry.isMultipart():
                polygons = geometry.asMultiPolygon()
                for poly in polygons:
                    p = []
                    for ring in poly:
                        p.append(self.densify(ring, interval))
                    output.append(p)
                return QgsGeometry.fromMultiPolygon(output)
            else:
                rings = geometry.asPolygon()
                for ring in rings:
                    output.append(self.densify(ring, interval))
                return QgsGeometry.fromPolygon(output)
        else:
            if geometry.isMultipart():
                lines = geometry.asMultiPolyline()
                for points in lines:
                    output.append(self.densify(points, interval))
                return QgsGeometry.fromMultiPolyline(output)
            else:
                points = geometry.asPolyline()
                output = self.densify(points, interval)
                return QgsGeometry.fromPolyline(output)

    def densify(self, polyline, interval):
        output = []
        for i in xrange(len(polyline) - 1):
            p1 = polyline[i]
            p2 = polyline[i + 1]
            output.append(p1)

            # Calculate necessary number of points between p1 and p2
            pointsNumber = sqrt(p1.sqrDist(p2)) / interval
            if pointsNumber > 1:
                multiplier = 1.0 / float(pointsNumber)
            else:
                multiplier = 1
            for j in xrange(int(pointsNumber)):
                delta = multiplier * (j + 1)
                x = p1.x() + delta * (p2.x() - p1.x())
                y = p1.y() + delta * (p2.y() - p1.y())
                output.append(QgsPoint(x, y))
                if j + 1 == pointsNumber:
                    break
        output.append(polyline[len(polyline) - 1])
        return output
