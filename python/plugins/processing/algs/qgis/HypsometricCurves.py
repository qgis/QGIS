# -*- coding: utf-8 -*-

"""
***************************************************************************
    HypsometricCurves.py
    ---------------------
    Date                 : November 2014
    Copyright            : (C) 2014 by Alexander Bruy
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
__date__ = 'November 2014'
__copyright__ = '(C) 2014, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'


import os

import numpy
from osgeo import gdal, ogr, osr

from qgis.core import QgsRectangle, QgsGeometry

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterBoolean
from processing.core.outputs import OutputDirectory

from processing.tools import raster
from processing.tools import dataobjects
from processing.tools import vector


class HypsometricCurves(GeoAlgorithm):

    INPUT_DEM = 'INPUT_DEM'
    BOUNDARY_LAYER = 'BOUNDARY_LAYER'
    STEP = 'STEP'
    USE_PERCENTAGE = 'USE_PERCENTAGE'
    OUTPUT_DIRECTORY = 'OUTPUT_DIRECTORY'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Hypsometric curves')
        self.group, self.i18n_group = self.trAlgorithm('Raster tools')

        self.addParameter(ParameterRaster(self.INPUT_DEM,
            self.tr('DEM to analyze')))
        self.addParameter(ParameterVector(self.BOUNDARY_LAYER,
            self.tr('Boundary layer'), ParameterVector.VECTOR_TYPE_POLYGON))
        self.addParameter(ParameterNumber(self.STEP,
            self.tr('Step'), 0.0, 999999999.999999, 100.0))
        self.addParameter(ParameterBoolean(self.USE_PERCENTAGE,
            self.tr('Use % of area instead of absolute value'), False))

        self.addOutput(OutputDirectory(self.OUTPUT_DIRECTORY,
            self.tr('Hypsometric curves')))

    def processAlgorithm(self, progress):
        rasterPath = self.getParameterValue(self.INPUT_DEM)
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.BOUNDARY_LAYER))
        step = self.getParameterValue(self.STEP)
        percentage = self.getParameterValue(self.USE_PERCENTAGE)

        outputPath = self.getOutputValue(self.OUTPUT_DIRECTORY)

        rasterDS = gdal.Open(rasterPath, gdal.GA_ReadOnly)
        geoTransform = rasterDS.GetGeoTransform()
        rasterBand = rasterDS.GetRasterBand(1)
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

        memVectorDriver = ogr.GetDriverByName('Memory')
        memRasterDriver = gdal.GetDriverByName('MEM')

        features = vector.features(layer)
        count = len(features)
        total = 100.0 / float(count)

        for count, f in enumerate(features):
            geom = f.geometry()
            intersectedGeom = rasterGeom.intersection(geom)

            if intersectedGeom.isGeosEmpty():
                progress.setInfo(
                    self.tr('Feature %d does not intersect raster or '
                            'entirely located in NODATA area' % f.id()))
                continue

            fName = os.path.join(
                outputPath, 'hystogram_%s_%s.csv' % (layer.name(), f.id()))

            ogrGeom = ogr.CreateGeometryFromWkt(intersectedGeom.exportToWkt())
            bbox = intersectedGeom.boundingBox()
            xMin = bbox.xMinimum()
            xMax = bbox.xMaximum()
            yMin = bbox.yMinimum()
            yMax = bbox.yMaximum()

            (startColumn, startRow) = raster.mapToPixel(xMin, yMax, geoTransform)
            (endColumn, endRow) = raster.mapToPixel(xMax, yMin, geoTransform)

            width = endColumn - startColumn
            height = endRow - startRow

            srcOffset = (startColumn, startRow, width, height)
            srcArray = rasterBand.ReadAsArray(*srcOffset)

            if srcOffset[2] == 0 or srcOffset[3] == 0:
                progress.setInfo(
                    self.tr('Feature %d is smaller than raster '
                            'cell size' % f.id()))
                continue

            newGeoTransform = (
                geoTransform[0] + srcOffset[0] * geoTransform[1],
                geoTransform[1],
                0.0,
                geoTransform[3] + srcOffset[1] * geoTransform[5],
                0.0,
                geoTransform[5]
            )

            memVDS = memVectorDriver.CreateDataSource('out')
            memLayer = memVDS.CreateLayer('poly', crs, ogr.wkbPolygon)

            ft = ogr.Feature(memLayer.GetLayerDefn())
            ft.SetGeometry(ogrGeom)
            memLayer.CreateFeature(ft)
            ft.Destroy()

            rasterizedDS = memRasterDriver.Create('', srcOffset[2],
                srcOffset[3], 1, gdal.GDT_Byte)
            rasterizedDS.SetGeoTransform(newGeoTransform)
            gdal.RasterizeLayer(rasterizedDS, [1], memLayer, burn_values=[1])
            rasterizedArray = rasterizedDS.ReadAsArray()

            srcArray = numpy.nan_to_num(srcArray)
            masked = numpy.ma.MaskedArray(srcArray,
                mask=numpy.logical_or(srcArray == noData,
                    numpy.logical_not(rasterizedArray)))

            self.calculateHypsometry(f.id(), fName, progress, masked,
                cellXSize, cellYSize, percentage, step)

            memVDS = None
            rasterizedDS = None
            progress.setPercentage(int(count * total))

        rasterDS = None

    def calculateHypsometry(self, fid, fName, progress, data, pX, pY,
                            percentage, step):
        out = dict()
        d = data.compressed()
        if d.size == 0:
            progress.setInfo(
                self.tr('Feature %d does not intersect raster or '
                        'entirely located in NODATA area' % fid))
            return

        minValue = d.min()
        maxValue = d.max()
        startValue = minValue
        tmpValue = minValue + step
        while startValue < maxValue:
            out[tmpValue] = ((startValue <= d) & (d < tmpValue)).sum()
            startValue = tmpValue
            tmpValue += step

        if percentage:
            multiplier = 100.0 / float(len(d.flat))
        else:
            multiplier = pX * pY

        for k, v in out.iteritems():
            out[k] = v * multiplier

        prev = None
        for i in sorted(out.items()):
            if prev is None:
                out[i[0]] = i[1]
            else:
                out[i[0]] = i[1] + out[prev]
            prev = i[0]

        writer = vector.TableWriter(fName, 'utf-8', [self.tr('Area'), self.tr('Elevation')])
        for i in sorted(out.items()):
            writer.addRecord([i[1], i[0]])
        del writer
