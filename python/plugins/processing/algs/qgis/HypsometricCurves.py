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

import os
import numpy
import csv

from osgeo import gdal, ogr, osr

from qgis.core import (QgsRectangle,
                       QgsGeometry,
                       QgsFeatureRequest,
                       QgsProcessingException,
                       QgsProcessing,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFolderDestination)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.tools import raster


class HypsometricCurves(QgisAlgorithm):

    INPUT_DEM = 'INPUT_DEM'
    BOUNDARY_LAYER = 'BOUNDARY_LAYER'
    STEP = 'STEP'
    USE_PERCENTAGE = 'USE_PERCENTAGE'
    OUTPUT_DIRECTORY = 'OUTPUT_DIRECTORY'

    def group(self):
        return self.tr('Raster terrain analysis')

    def groupId(self):
        return 'rasterterrainanalysis'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterRasterLayer(self.INPUT_DEM,
                                                            self.tr('DEM to analyze')))
        self.addParameter(QgsProcessingParameterFeatureSource(self.BOUNDARY_LAYER,
                                                              self.tr('Boundary layer'), [QgsProcessing.TypeVectorPolygon]))
        self.addParameter(QgsProcessingParameterNumber(self.STEP,
                                                       self.tr('Step'), type=QgsProcessingParameterNumber.Double, minValue=0.0, defaultValue=100.0))
        self.addParameter(QgsProcessingParameterBoolean(self.USE_PERCENTAGE,
                                                        self.tr('Use % of area instead of absolute value'), defaultValue=False))

        self.addParameter(QgsProcessingParameterFolderDestination(self.OUTPUT_DIRECTORY,
                                                                  self.tr('Hypsometric curves')))

    def name(self):
        return 'hypsometriccurves'

    def displayName(self):
        return self.tr('Hypsometric curves')

    def processAlgorithm(self, parameters, context, feedback):
        raster_layer = self.parameterAsRasterLayer(parameters, self.INPUT_DEM, context)
        target_crs = raster_layer.crs()
        rasterPath = raster_layer.source()

        source = self.parameterAsSource(parameters, self.BOUNDARY_LAYER, context)
        if source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.BOUNDARY_LAYER))

        step = self.parameterAsDouble(parameters, self.STEP, context)
        percentage = self.parameterAsBoolean(parameters, self.USE_PERCENTAGE, context)

        outputPath = self.parameterAsString(parameters, self.OUTPUT_DIRECTORY, context)
        if not os.path.exists(outputPath):
            os.makedirs(outputPath)

        rasterDS = gdal.Open(rasterPath, gdal.GA_ReadOnly)
        geoTransform = rasterDS.GetGeoTransform()
        rasterBand = rasterDS.GetRasterBand(1)
        noData = rasterBand.GetNoDataValue()

        cellXSize = abs(geoTransform[1])
        cellYSize = abs(geoTransform[5])
        rasterXSize = rasterDS.RasterXSize
        rasterYSize = rasterDS.RasterYSize

        rasterBBox = QgsRectangle(geoTransform[0],
                                  geoTransform[3] - cellYSize * rasterYSize,
                                  geoTransform[0] + cellXSize * rasterXSize,
                                  geoTransform[3])
        rasterGeom = QgsGeometry.fromRect(rasterBBox)

        crs = osr.SpatialReference()
        crs.ImportFromProj4(str(target_crs.toProj4()))

        memVectorDriver = ogr.GetDriverByName('Memory')
        memRasterDriver = gdal.GetDriverByName('MEM')

        features = source.getFeatures(QgsFeatureRequest().setDestinationCrs(target_crs, context.transformContext()))
        total = 100.0 / source.featureCount() if source.featureCount() else 0

        for current, f in enumerate(features):
            if not f.hasGeometry():
                continue

            if feedback.isCanceled():
                break

            geom = f.geometry()
            intersectedGeom = rasterGeom.intersection(geom)

            if intersectedGeom.isEmpty():
                feedback.pushInfo(
                    self.tr('Feature {0} does not intersect raster or '
                            'entirely located in NODATA area').format(f.id()))
                continue

            fName = os.path.join(
                outputPath, 'histogram_{}_{}.csv'.format(source.sourceName(), f.id()))

            ogrGeom = ogr.CreateGeometryFromWkt(intersectedGeom.asWkt())
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
                feedback.pushInfo(
                    self.tr('Feature {0} is smaller than raster '
                            'cell size').format(f.id()))
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

            self.calculateHypsometry(f.id(), fName, feedback, masked,
                                     cellXSize, cellYSize, percentage, step)

            memVDS = None
            rasterizedDS = None
            feedback.setProgress(int(current * total))

        rasterDS = None

        return {self.OUTPUT_DIRECTORY: outputPath}

    def calculateHypsometry(self, fid, fName, feedback, data, pX, pY,
                            percentage, step):
        out = dict()
        d = data.compressed()
        if d.size == 0:
            feedback.pushInfo(
                self.tr('Feature {0} does not intersect raster or '
                        'entirely located in NODATA area').format(fid))
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
            multiplier = 100.0 / len(d.flat)
        else:
            multiplier = pX * pY

        for k, v in out.items():
            out[k] = v * multiplier

        prev = None
        for i in sorted(out.items()):
            if prev is None:
                out[i[0]] = i[1]
            else:
                out[i[0]] = i[1] + out[prev]
            prev = i[0]

        with open(fName, 'w', newline='', encoding='utf-8') as out_file:
            writer = csv.writer(out_file)
            writer.writerow([self.tr('Area'), self.tr('Elevation')])

            for i in sorted(out.items()):
                writer.writerow([i[1], i[0]])
