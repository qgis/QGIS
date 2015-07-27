# -*- coding: utf-8 -*-

"""
***************************************************************************
    ZonalStatistics.py
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

import numpy

try:
    from scipy.stats.mstats import mode
    hasSciPy = True
except:
    hasSciPy = False

from osgeo import gdal, ogr, osr
from qgis.core import QgsRectangle, QgsGeometry, QgsFeature

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterString
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterBoolean
from processing.core.outputs import OutputVector
from processing.tools.raster import mapToPixel
from processing.tools import dataobjects, vector


class ZonalStatistics(GeoAlgorithm):

    INPUT_RASTER = 'INPUT_RASTER'
    RASTER_BAND = 'RASTER_BAND'
    INPUT_VECTOR = 'INPUT_VECTOR'
    COLUMN_PREFIX = 'COLUMN_PREFIX'
    GLOBAL_EXTENT = 'GLOBAL_EXTENT'
    OUTPUT_LAYER = 'OUTPUT_LAYER'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Zonal Statistics')
        self.group, self.i18n_group = self.trAlgorithm('Raster tools')

        self.addParameter(ParameterRaster(self.INPUT_RASTER,
            self.tr('Raster layer')))
        self.addParameter(ParameterNumber(self.RASTER_BAND,
            self.tr('Raster band'), 1, 999, 1))
        self.addParameter(ParameterVector(self.INPUT_VECTOR,
            self.tr('Vector layer containing zones'),
            [ParameterVector.VECTOR_TYPE_POLYGON]))
        self.addParameter(ParameterString(self.COLUMN_PREFIX,
            self.tr('Output column prefix'), '_'))
        self.addParameter(ParameterBoolean(self.GLOBAL_EXTENT,
            self.tr('Load whole raster in memory')))
        self.addOutput(OutputVector(self.OUTPUT_LAYER, self.tr('Zonal statistics')))

    def processAlgorithm(self, progress):
        """ Based on code by Matthew Perry
            https://gist.github.com/perrygeo/5667173
        """

        layer = dataobjects.getObjectFromUri(self.getParameterValue(self.INPUT_VECTOR))

        rasterPath = unicode(self.getParameterValue(self.INPUT_RASTER))
        bandNumber = self.getParameterValue(self.RASTER_BAND)
        columnPrefix = self.getParameterValue(self.COLUMN_PREFIX)
        useGlobalExtent = self.getParameterValue(self.GLOBAL_EXTENT)

        rasterDS = gdal.Open(rasterPath, gdal.GA_ReadOnly)
        geoTransform = rasterDS.GetGeoTransform()
        rasterBand = rasterDS.GetRasterBand(bandNumber)
        noData = rasterBand.GetNoDataValue()

        cellXSize = abs(geoTransform[1])
        cellYSize = abs(geoTransform[5])
        rasterXSize = rasterDS.RasterXSize
        rasterYSize = rasterDS.RasterYSize

        rasterBBox = QgsRectangle(geoTransform[0], geoTransform[3] - cellYSize
                                  * rasterYSize, geoTransform[0] + cellXSize
                                  * rasterXSize, geoTransform[3])

        rasterGeom = QgsGeometry.fromRect(rasterBBox)

        crs = osr.SpatialReference()
        crs.ImportFromProj4(str(layer.crs().toProj4()))

        if useGlobalExtent:
            xMin = rasterBBox.xMinimum()
            xMax = rasterBBox.xMaximum()
            yMin = rasterBBox.yMinimum()
            yMax = rasterBBox.yMaximum()

            (startColumn, startRow) = mapToPixel(xMin, yMax, geoTransform)
            (endColumn, endRow) = mapToPixel(xMax, yMin, geoTransform)

            width = endColumn - startColumn
            height = endRow - startRow

            srcOffset = (startColumn, startRow, width, height)
            srcArray = rasterBand.ReadAsArray(*srcOffset)

            newGeoTransform = (
                geoTransform[0] + srcOffset[0] * geoTransform[1],
                geoTransform[1],
                0.0,
                geoTransform[3] + srcOffset[1] * geoTransform[5],
                0.0,
                geoTransform[5],
            )

        memVectorDriver = ogr.GetDriverByName('Memory')
        memRasterDriver = gdal.GetDriverByName('MEM')

        fields = layer.pendingFields()
        (idxMin, fields) = vector.findOrCreateField(layer, fields,
                columnPrefix + 'min', 21, 6)
        (idxMax, fields) = vector.findOrCreateField(layer, fields,
                columnPrefix + 'max', 21, 6)
        (idxSum, fields) = vector.findOrCreateField(layer, fields,
                columnPrefix + 'sum', 21, 6)
        (idxCount, fields) = vector.findOrCreateField(layer, fields,
                columnPrefix + 'count', 21, 6)
        (idxMean, fields) = vector.findOrCreateField(layer, fields,
                columnPrefix + 'mean', 21, 6)
        (idxStd, fields) = vector.findOrCreateField(layer, fields,
                columnPrefix + 'std', 21, 6)
        (idxUnique, fields) = vector.findOrCreateField(layer, fields,
                columnPrefix + 'unique', 21, 6)
        (idxRange, fields) = vector.findOrCreateField(layer, fields,
                columnPrefix + 'range', 21, 6)
        (idxVar, fields) = vector.findOrCreateField(layer, fields,
                columnPrefix + 'var', 21, 6)
        (idxMedian, fields) = vector.findOrCreateField(layer, fields,
                columnPrefix + 'median', 21, 6)
        if hasSciPy:
            (idxMode, fields) = vector.findOrCreateField(layer, fields,
                    columnPrefix + 'mode', 21, 6)

        writer = self.getOutputFromName(self.OUTPUT_LAYER).getVectorWriter(
            fields.toList(), layer.dataProvider().geometryType(), layer.crs())

        outFeat = QgsFeature()

        outFeat.initAttributes(len(fields))
        outFeat.setFields(fields)

        current = 0
        features = vector.features(layer)
        total = 100.0 / len(features)
        for f in features:
            geom = f.geometry()

            intersectedGeom = rasterGeom.intersection(geom)
            ogrGeom = ogr.CreateGeometryFromWkt(intersectedGeom.exportToWkt())

            if not useGlobalExtent:
                bbox = intersectedGeom.boundingBox()

                xMin = bbox.xMinimum()
                xMax = bbox.xMaximum()
                yMin = bbox.yMinimum()
                yMax = bbox.yMaximum()

                (startColumn, startRow) = mapToPixel(xMin, yMax, geoTransform)
                (endColumn, endRow) = mapToPixel(xMax, yMin, geoTransform)

                width = endColumn - startColumn
                height = endRow - startRow

                if width == 0 or height == 0:
                    continue

                srcOffset = (startColumn, startRow, width, height)
                srcArray = rasterBand.ReadAsArray(*srcOffset)

                newGeoTransform = (
                    geoTransform[0] + srcOffset[0] * geoTransform[1],
                    geoTransform[1],
                    0.0,
                    geoTransform[3] + srcOffset[1] * geoTransform[5],
                    0.0,
                    geoTransform[5],
                )

            # Create a temporary vector layer in memory
            memVDS = memVectorDriver.CreateDataSource('out')
            memLayer = memVDS.CreateLayer('poly', crs, ogr.wkbPolygon)

            ft = ogr.Feature(memLayer.GetLayerDefn())
            ft.SetGeometry(ogrGeom)
            memLayer.CreateFeature(ft)
            ft.Destroy()

            # Rasterize it
            rasterizedDS = memRasterDriver.Create('', srcOffset[2],
                    srcOffset[3], 1, gdal.GDT_Byte)
            rasterizedDS.SetGeoTransform(newGeoTransform)
            gdal.RasterizeLayer(rasterizedDS, [1], memLayer, burn_values=[1])
            rasterizedArray = rasterizedDS.ReadAsArray()

            srcArray = numpy.nan_to_num(srcArray)
            masked = numpy.ma.MaskedArray(srcArray,
                    mask=numpy.logical_or(srcArray == noData,
                    numpy.logical_not(rasterizedArray)))

            outFeat.setGeometry(geom)

            attrs = f.attributes()
            attrs.insert(idxMin, float(masked.min()))
            attrs.insert(idxMax, float(masked.max()))
            attrs.insert(idxSum, float(masked.sum()))
            attrs.insert(idxCount, int(masked.count()))
            attrs.insert(idxMean, float(masked.mean()))
            attrs.insert(idxStd, float(masked.std()))
            attrs.insert(idxUnique, numpy.unique(masked.compressed()).size)
            attrs.insert(idxRange, float(masked.max()) - float(masked.min()))
            attrs.insert(idxVar, float(masked.var()))
            attrs.insert(idxMedian, float(numpy.ma.median(masked)))
            if hasSciPy:
                attrs.insert(idxMode, float(mode(masked, axis=None)[0][0]))

            outFeat.setAttributes(attrs)
            writer.addFeature(outFeat)

            memVDS = None
            rasterizedDS = None

            current += 1
            progress.setPercentage(int(current * total))

        rasterDS = None

        del writer
