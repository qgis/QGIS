# -*- coding: utf-8 -*-

"""
***************************************************************************
    PointsFromLines.py
    ---------------------
    Date                 : August 2013
    Copyright            : (C) 2013 by Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Alexander Bruy'
__date__ = 'August 2013'
__copyright__ = '(C) 2013, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from osgeo import gdal
from PyQt4.QtCore import QVariant
from qgis.core import QGis, QgsFeature, QgsFields, QgsField, QgsGeometry, QgsPoint
from processing.tools import vector, raster, dataobjects
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterVector
from processing.core.outputs import OutputVector


class PointsFromLines(GeoAlgorithm):

    INPUT_RASTER = 'INPUT_RASTER'
    RASTER_BAND = 'RASTER_BAND'
    INPUT_VECTOR = 'INPUT_VECTOR'
    OUTPUT_LAYER = 'OUTPUT_LAYER'

    def defineCharacteristics(self):
        self.name = 'Generate points (pixel centroids) along line'
        self.group = 'Vector analysis tools'

        self.addParameter(ParameterRaster(self.INPUT_RASTER,
            self.tr('Raster layer')))
        self.addParameter(ParameterVector(self.INPUT_VECTOR,
            self.tr('Vector layer'), [ParameterVector.VECTOR_TYPE_LINE]))
        self.addOutput(OutputVector(self.OUTPUT_LAYER, self.tr('Output layer')))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(self.getParameterValue(self.INPUT_VECTOR))

        rasterPath = unicode(self.getParameterValue(self.INPUT_RASTER))

        rasterDS = gdal.Open(rasterPath, gdal.GA_ReadOnly)
        geoTransform = rasterDS.GetGeoTransform()
        rasterDS = None

        fields = QgsFields()
        fields.append(QgsField('id', QVariant.Int, '', 10, 0))
        fields.append(QgsField('line_id', QVariant.Int, '', 10, 0))
        fields.append(QgsField('point_id', QVariant.Int, '', 10, 0))

        writer = self.getOutputFromName(self.OUTPUT_LAYER).getVectorWriter(
            fields.toList(), QGis.WKBPoint, layer.crs())

        outFeature = QgsFeature()
        outFeature.setFields(fields)

        self.fid = 0
        self.lineId = 0
        self.pointId = 0

        current = 0
        features = vector.features(layer)
        total = 100.0 / len(features)
        for f in features:
            geom = f.geometry()
            if geom.isMultipart():
                lines = geom.asMultiPolyline()
                for line in lines:
                    for i in xrange(len(line) - 1):
                        p1 = line[i]
                        p2 = line[i + 1]

                        (x1, y1) = raster.mapToPixel(p1.x(), p1.y(),
                                geoTransform)
                        (x2, y2) = raster.mapToPixel(p2.x(), p2.y(),
                                geoTransform)

                        self.buildLine(x1, y1, x2, y2, geoTransform,
                                       writer, outFeature)
            else:
                points = geom.asPolyline()
                for i in xrange(len(points) - 1):
                    p1 = points[i]
                    p2 = points[i + 1]

                    (x1, y1) = raster.mapToPixel(p1.x(), p1.y(), geoTransform)
                    (x2, y2) = raster.mapToPixel(p2.x(), p2.y(), geoTransform)

                    self.buildLine(x1, y1, x2, y2, geoTransform, writer,
                                   outFeature)

            self.pointId = 0
            self.lineId += 1

            current += 1
            progress.setPercentage(int(current * total))

        del writer

    def buildLine(self, startX, startY, endX, endY, geoTransform, writer, feature):
        if startX == endX:
            if startY > endY:
                (startY, endY) = (endY, startY)
            row = startX
            for col in xrange(startY, endY + 1):
                self.createPoint(row, col, geoTransform, writer, feature)
        elif startY == endY:
            if startX > endX:
                (startX, endX) = (endX, startX)
            col = startY
            for row in xrange(startX, endX + 1):
                self.createPoint(row, col, geoTransform, writer, feature)
        else:
            width = endX - startX
            height = endY - startY

            if width < 0:
                dx1 = -1
                dx2 = -1
            else:
                dx1 = 1
                dx2 = 1

            if height < 0:
                dy1 = -1
            else:
                dy1 = 1
            dy2 = 0

            longest = abs(width)
            shortest = abs(height)
            if not longest > shortest:
                (longest, shortest) = (shortest, longest)
                if height < 0:
                    dy2 = -1
                else:
                    dy2 = 1
                dx2 = 0

            err = longest / 2
            for i in xrange(longest + 1):
                self.createPoint(startX, startY, geoTransform, writer, feature)

                err += shortest
                if not err < longest:
                    err = err - longest
                    startX += dx1
                    startY += dy1
                else:
                    startX += dx2
                    startY += dy2

    def createPoint(self, pX, pY, geoTransform, writer, feature):
        (x, y) = raster.pixelToMap(pX, pY, geoTransform)

        feature.setGeometry(QgsGeometry.fromPoint(QgsPoint(x, y)))
        feature['id'] = self.fid
        feature['line_id'] = self.lineId
        feature['point_id'] = self.pointId

        self.fid += 1
        self.pointId += 1

        writer.addFeature(feature)
