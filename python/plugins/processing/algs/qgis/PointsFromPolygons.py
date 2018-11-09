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
from qgis.core import (QgsFeatureRequest,
                       QgsFields,
                       QgsField,
                       QgsFeature,
                       QgsFeatureSink,
                       QgsGeometry,
                       QgsWkbTypes,
                       QgsPoint,
                       QgsProcessing,
                       QgsProcessingException,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink)
from qgis.PyQt.QtCore import QVariant
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.tools import raster


class PointsFromPolygons(QgisAlgorithm):

    INPUT_RASTER = 'INPUT_RASTER'
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
                                                              self.tr('Vector layer'), [QgsProcessing.TypeVectorPolygon]))
        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Points inside polygons'), QgsProcessing.TypeVectorPoint))

    def name(self):
        return 'generatepointspixelcentroidsinsidepolygons'

    def displayName(self):
        return self.tr('Generate points (pixel centroids) inside polygons')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT_VECTOR, context)
        if source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.INPUT_VECTOR))

        raster_layer = self.parameterAsRasterLayer(parameters, self.INPUT_RASTER, context)
        rasterPath = raster_layer.source()

        rasterDS = gdal.Open(rasterPath, gdal.GA_ReadOnly)
        geoTransform = rasterDS.GetGeoTransform()

        fields = QgsFields()
        fields.append(QgsField('id', QVariant.Int, '', 10, 0))
        fields.append(QgsField('poly_id', QVariant.Int, '', 10, 0))
        fields.append(QgsField('point_id', QVariant.Int, '', 10, 0))

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, QgsWkbTypes.Point, raster_layer.crs())
        if sink is None:
            raise QgsProcessingException(self.invalidSinkError(parameters, self.OUTPUT))

        outFeature = QgsFeature()
        outFeature.setFields(fields)

        fid = 0
        polyId = 0
        pointId = 0

        features = source.getFeatures(QgsFeatureRequest().setDestinationCrs(raster_layer.crs(), context.transformContext()))
        total = 100.0 / source.featureCount() if source.featureCount() else 0
        for current, f in enumerate(features):
            if feedback.isCanceled():
                break

            if not f.hasGeometry():
                continue

            geom = f.geometry()
            bbox = geom.boundingBox()

            xMin = bbox.xMinimum()
            xMax = bbox.xMaximum()
            yMin = bbox.yMinimum()
            yMax = bbox.yMaximum()

            (startRow, startColumn) = raster.mapToPixel(xMin, yMax, geoTransform)
            (endRow, endColumn) = raster.mapToPixel(xMax, yMin, geoTransform)

            # use prepared geometries for faster intersection tests
            engine = QgsGeometry.createGeometryEngine(geom.constGet())
            engine.prepareGeometry()

            for row in range(startRow, endRow + 1):
                for col in range(startColumn, endColumn + 1):
                    if feedback.isCanceled():
                        break

                    (x, y) = raster.pixelToMap(row, col, geoTransform)
                    point = QgsPoint(x, y)

                    if engine.contains(point):
                        outFeature.setGeometry(QgsGeometry(point))
                        outFeature['id'] = fid
                        outFeature['poly_id'] = polyId
                        outFeature['point_id'] = pointId

                        fid += 1
                        pointId += 1

                        sink.addFeature(outFeature, QgsFeatureSink.FastInsert)

            pointId = 0
            polyId += 1

            feedback.setProgress(int(current * total))

        return {self.OUTPUT: dest_id}
