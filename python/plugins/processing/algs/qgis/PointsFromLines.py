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

from osgeo import gdal
from qgis.PyQt.QtCore import QVariant
from qgis.core import (QgsFeature,
                       QgsFeatureSink,
                       QgsFields,
                       QgsField,
                       QgsGeometry,
                       QgsPointXY,
                       QgsWkbTypes,
                       QgsProcessing,
                       QgsProcessingException,
                       QgsFeatureRequest,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink)
from processing.tools import raster
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class PointsFromLines(QgisAlgorithm):

    INPUT_RASTER = 'INPUT_RASTER'
    RASTER_BAND = 'RASTER_BAND'
    INPUT_VECTOR = 'INPUT_VECTOR'
    OUTPUT = 'OUTPUT'

    def group(self):
        return self.tr('Vector creation')

    def groupId(self):
        return 'vectorcreation'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterRasterLayer(self.INPUT_RASTER,
                                                            self.tr('Raster layer')))
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT_VECTOR,
                                                              self.tr('Vector layer'), [QgsProcessing.TypeVectorLine]))
        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Points along lines'), QgsProcessing.TypeVectorPoint))

    def name(self):
        return 'generatepointspixelcentroidsalongline'

    def displayName(self):
        return self.tr('Generate points (pixel centroids) along line')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT_VECTOR, context)
        if source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.INPUT_VECTOR))

        raster_layer = self.parameterAsRasterLayer(parameters, self.INPUT_RASTER, context)
        rasterPath = raster_layer.source()

        rasterDS = gdal.Open(rasterPath, gdal.GA_ReadOnly)
        geoTransform = rasterDS.GetGeoTransform()
        rasterDS = None

        fields = QgsFields()
        fields.append(QgsField('id', QVariant.Int, '', 10, 0))
        fields.append(QgsField('line_id', QVariant.Int, '', 10, 0))
        fields.append(QgsField('point_id', QVariant.Int, '', 10, 0))

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, QgsWkbTypes.Point, raster_layer.crs())
        if sink is None:
            raise QgsProcessingException(self.invalidSinkError(parameters, self.OUTPUT))

        outFeature = QgsFeature()
        outFeature.setFields(fields)

        self.fid = 0
        self.lineId = 0
        self.pointId = 0

        features = source.getFeatures(QgsFeatureRequest().setDestinationCrs(raster_layer.crs(), context.transformContext()))
        total = 100.0 / source.featureCount() if source.featureCount() else 0
        for current, f in enumerate(features):
            if feedback.isCanceled():
                break

            if not f.hasGeometry():
                continue

            geom = f.geometry()
            if geom.isMultipart():
                lines = geom.asMultiPolyline()
                for line in lines:
                    for i in range(len(line) - 1):
                        p1 = line[i]
                        p2 = line[i + 1]

                        (x1, y1) = raster.mapToPixel(p1.x(), p1.y(),
                                                     geoTransform)
                        (x2, y2) = raster.mapToPixel(p2.x(), p2.y(),
                                                     geoTransform)

                        self.buildLine(x1, y1, x2, y2, geoTransform,
                                       sink, outFeature)
            else:
                points = geom.asPolyline()
                for i in range(len(points) - 1):
                    p1 = points[i]
                    p2 = points[i + 1]

                    (x1, y1) = raster.mapToPixel(p1.x(), p1.y(), geoTransform)
                    (x2, y2) = raster.mapToPixel(p2.x(), p2.y(), geoTransform)

                    self.buildLine(x1, y1, x2, y2, geoTransform, sink,
                                   outFeature)

            self.pointId = 0
            self.lineId += 1

            feedback.setProgress(int(current * total))

        return {self.OUTPUT: dest_id}

    def buildLine(self, startX, startY, endX, endY, geoTransform, writer, feature):
        if startX == endX:
            if startY > endY:
                (startY, endY) = (endY, startY)
            row = startX
            for col in range(startY, endY + 1):
                self.createPoint(row, col, geoTransform, writer, feature)
        elif startY == endY:
            if startX > endX:
                (startX, endX) = (endX, startX)
            col = startY
            for row in range(startX, endX + 1):
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
            for i in range(longest + 1):
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

        feature.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(x, y)))
        feature['id'] = self.fid
        feature['line_id'] = self.lineId
        feature['point_id'] = self.pointId

        self.fid += 1
        self.pointId += 1

        writer.addFeature(feature, QgsFeatureSink.FastInsert)
