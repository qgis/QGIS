# -*- coding: utf-8 -*-

"""
***************************************************************************
    PointsFromPolygons.py
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
from qgis.core import *
from PyQt4.QtCore import *
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.parameters.ParameterRaster import ParameterRaster
from processing.parameters.ParameterVector import ParameterVector
from processing.outputs.OutputVector import OutputVector
from processing.tools import dataobjects, vector
from processing.tools.general import *

class PointsFromPolygons(GeoAlgorithm):

    INPUT_RASTER = "INPUT_RASTER"
    RASTER_BAND = "RASTER_BAND"
    INPUT_VECTOR = "INPUT_VECTOR"
    OUTPUT_LAYER = "OUTPUT_LAYER"

    def defineCharacteristics(self):
        self.name = "Points from polygons"
        self.group = "Vector geometry tools"

        self.addParameter(ParameterRaster(self.INPUT_RASTER, "Raster layer"))
        self.addParameter(ParameterVector(self.INPUT_VECTOR, "Vector layer", [ParameterVector.VECTOR_TYPE_POLYGON]))
        self.addOutput(OutputVector(self.OUTPUT_LAYER, "Output layer"))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(self.getParameterValue(self.INPUT_VECTOR))

        rasterPath = unicode(self.getParameterValue(self.INPUT_RASTER))

        rasterDS = gdal.Open(rasterPath, gdal.GA_ReadOnly)
        geoTransform = rasterDS.GetGeoTransform()
        rasterDS = None

        fields = QgsFields()
        fields.append(QgsField("id", QVariant.Int, "", 10, 0))
        fields.append(QgsField("poly_id", QVariant.Int, "", 10, 0))
        fields.append(QgsField("point_id", QVariant.Int, "", 10, 0))

        writer = self.getOutputFromName(self.OUTPUT_LAYER).getVectorWriter(fields.toList(), QGis.WKBPoint, layer.crs())

        outFeature = QgsFeature()
        outFeature.setFields(fields)
        point = QgsPoint()

        fid = 0
        polyId = 0
        pointId = 0

        current = 0
        features = vector.features(layer)
        total = 100.0 / len(features)
        for f in features:
            geom = f.geometry()
            bbox = geom.boundingBox()

            xMin = bbox.xMinimum()
            xMax = bbox.xMaximum()
            yMin = bbox.yMinimum()
            yMax = bbox.yMaximum()

            startRow, startColumn = mapToPixel(xMin, yMax, geoTransform)
            endRow, endColumn = mapToPixel(xMax, yMin, geoTransform)

            for row in xrange(startRow, endRow + 1):
                for col in xrange(startColumn, endColumn + 1):
                    x, y = pixelToMap(row, col, geoTransform)
                    point.setX(x)
                    point.setY(y)

                    if geom.contains(point):
                        outFeature.setGeometry(QgsGeometry.fromPoint(point))
                        outFeature["id"] = fid
                        outFeature["poly_id"] = polyId
                        outFeature["point_id"] = pointId

                        fid += 1
                        pointId +=1

                        writer.addFeature(outFeature)

            pointId = 0
            polyId += 1

            current += 1
            progress.setPercentage(int(current * total))

        del writer

