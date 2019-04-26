# -*- coding: utf-8 -*-

"""
***************************************************************************
    TilesXYZ.py
    ---------------------
    Date                 : April 2019
    Copyright            : (C) 2019 by Lutra Consulting Limited
    Email                : marcel.dancak@lutraconsulting.co.uk
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Marcel Dancak'
__date__ = 'April 2019'
__copyright__ = '(C) 2019 by Lutra Consulting Limited'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import math
from uuid import uuid4

import ogr
import gdal
from qgis.PyQt.QtCore import QSize, Qt, QByteArray, QBuffer
from qgis.PyQt.QtGui import QColor, QImage, QPainter
from qgis.core import (QgsProcessingException,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterString,
                       QgsProcessingParameterExtent,
                       QgsProcessingOutputFile,
                       QgsProcessingParameterFileDestination,
                       QgsProcessingParameterFolderDestination,
                       QgsGeometry,
                       QgsRectangle,
                       QgsMapSettings,
                       QgsCoordinateTransform,
                       QgsCoordinateReferenceSystem,
                       QgsMapRendererCustomPainterJob)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


# Math functions taken from https://wiki.openstreetmap.org/wiki/Slippy_map_tilenames #spellok
def deg2num(lat_deg, lon_deg, zoom):
    lat_rad = math.radians(lat_deg)
    n = 2.0 ** zoom
    xtile = int((lon_deg + 180.0) / 360.0 * n)
    ytile = int((1.0 - math.log(math.tan(lat_rad) + (1 / math.cos(lat_rad))) / math.pi) / 2.0 * n)
    return (xtile, ytile)


def num2deg(xtile, ytile, zoom):
    n = 2.0 ** zoom
    lon_deg = xtile / n * 360.0 - 180.0
    lat_rad = math.atan(math.sinh(math.pi * (1 - 2 * ytile / n)))
    lat_deg = math.degrees(lat_rad)
    return (lat_deg, lon_deg)


class Tile:
    def __init__(self, x, y, z):
        self.x = x
        self.y = y
        self.z = z

    def extent(self):
        lat1, lon1 = num2deg(self.x, self.y, self.z)
        lat2, lon2 = num2deg(self.x + 1, self.y + 1, self.z)
        return [lon1, lat2, lon2, lat1]


class MetaTile:
    def __init__(self):
        # list of tuple(row index, column index, Tile)
        self.tiles = []

    def add_tile(self, row, column, tile):
        self.tiles.append((row, column, tile))

    def rows(self):
        return max([r for r, _, _ in self.tiles]) + 1

    def columns(self):
        return max([c for _, c, _ in self.tiles]) + 1

    def extent(self):
        _, _, first = self.tiles[0]
        _, _, last = self.tiles[-1]
        lat1, lon1 = num2deg(first.x, first.y, first.z)
        lat2, lon2 = num2deg(last.x + 1, last.y + 1, first.z)
        return [lon1, lat2, lon2, lat1]


def get_metatiles(extent, zoom, size=4):
    west_edge, south_edge, east_edge, north_edge = extent
    left_tile, top_tile = deg2num(north_edge, west_edge, zoom)
    right_tile, bottom_tile = deg2num(south_edge, east_edge, zoom)

    metatiles = {}
    for i, x in enumerate(range(left_tile, right_tile + 1)):
        for j, y in enumerate(range(top_tile, bottom_tile + 1)):
            meta_key = '{}:{}'.format(int(i / size), int(j / size))
            if meta_key not in metatiles:
                metatiles[meta_key] = MetaTile()
            metatile = metatiles[meta_key]
            metatile.add_tile(i % size, j % size, Tile(x, y, zoom))

    return list(metatiles.values())


class DirectoryWriter:
    def __init__(self, folder, tile_params):
        self.folder = folder
        self.format = tile_params.get('format', 'PNG')
        self.quality = tile_params.get('quality', -1)

    def writeTile(self, tile, image):
        directory = os.path.join(self.folder, str(tile.z), str(tile.x))
        os.makedirs(directory, exist_ok=True)
        path = os.path.join(directory, '{}.{}'.format(tile.y, self.format.lower()))
        image.save(path, self.format, self.quality)
        return path

    def close(self):
        pass


class MBTilesWriter:
    def __init__(self, filename, tile_params, extent, min_zoom, max_zoom):
        base_dir = os.path.dirname(filename)
        os.makedirs(base_dir, exist_ok=True)
        self.filename = filename
        self.extent = extent
        self.tile_width = tile_params.get('width', 256)
        self.tile_height = tile_params.get('height', 256)
        tile_format = tile_params['format']
        if tile_format == 'JPG':
            tile_format = 'JPEG'
            options = ['QUALITY=%s' % tile_params.get('quality', 75)]
        else:
            options = ['ZLEVEL=8']
        driver = gdal.GetDriverByName('MBTiles')
        ds = driver.Create(filename, 1, 1, 1, options=['TILE_FORMAT=%s' % tile_format] + options)
        ds = None
        sqlite_driver = ogr.GetDriverByName('SQLite')
        ds = sqlite_driver.Open(filename, 1)
        ds.ExecuteSQL("INSERT INTO metadata(name, value) VALUES ('{}', '{}');".format('minzoom', min_zoom))
        ds.ExecuteSQL("INSERT INTO metadata(name, value) VALUES ('{}', '{}');".format('maxzoom', max_zoom))
        # will be set properly after writing all tiles
        ds.ExecuteSQL("INSERT INTO metadata(name, value) VALUES ('{}', '');".format('bounds'))
        ds = None
        self._zoom = None

    def _initZoomLayer(self, zoom):
        west_edge, south_edge, east_edge, north_edge = self.extent
        first_tile = Tile(*deg2num(north_edge, west_edge, zoom), zoom)
        last_tile = Tile(*deg2num(south_edge, east_edge, zoom), zoom)

        first_tile_extent = first_tile.extent()
        last_tile_extent = last_tile.extent()
        zoom_extent = [
            first_tile_extent[0],
            last_tile_extent[1],
            last_tile_extent[2],
            first_tile_extent[3]
        ]

        sqlite_driver = ogr.GetDriverByName('SQLite')
        ds = sqlite_driver.Open(self.filename, 1)
        bounds = ','.join(map(str, zoom_extent))
        ds.ExecuteSQL("UPDATE metadata SET value='{}' WHERE name='bounds'".format(bounds))
        ds = None

        self._zoomDs = gdal.OpenEx(self.filename, 1, open_options=['ZOOM_LEVEL=%s' % first_tile.z])
        self._first_tile = first_tile
        self._zoom = zoom

    def writeTile(self, tile, image):
        if tile.z != self._zoom:
            self._initZoomLayer(tile.z)

        data = QByteArray()
        buff = QBuffer(data)
        image.save(buff, 'PNG')

        mmap_name = '/vsimem/' + uuid4().hex
        gdal.FileFromMemBuffer(mmap_name, data.data())
        gdal_dataset = gdal.Open(mmap_name)
        data = gdal_dataset.ReadRaster(0, 0, self.tile_width, self.tile_height)
        gdal_dataset = None
        gdal.Unlink(mmap_name)

        xoff = (tile.x - self._first_tile.x) * self.tile_width
        yoff = (tile.y - self._first_tile.y) * self.tile_height
        self._zoomDs.WriteRaster(xoff, yoff, self.tile_width, self.tile_height, data)

    def close(self):
        self._zoomDs = None
        sqlite_driver = ogr.GetDriverByName('SQLite')
        ds = sqlite_driver.Open(self.filename, 1)
        bounds = ','.join(map(str, self.extent))
        ds.ExecuteSQL("UPDATE metadata SET value='{}' WHERE name='bounds'".format(bounds))
        ds = None


class TilesXYZ(QgisAlgorithm):
    EXTENT = 'EXTENT'
    ZOOM_MIN = 'ZOOM_MIN'
    ZOOM_MAX = 'ZOOM_MAX'
    TILE_FORMAT = 'TILE_FORMAT'
    DPI = 'DPI'
    OUTPUT_FORMAT = 'OUTPUT_FORMAT'
    OUTPUT_DIRECTORY = 'OUTPUT_DIRECTORY'
    OUTPUT_FILE = 'OUTPUT_FILE'

    def group(self):
        return self.tr('Raster tools')

    def groupId(self):
        return 'rastertools'

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterExtent(self.EXTENT, self.tr('Extent')))
        self.addParameter(QgsProcessingParameterNumber(self.ZOOM_MIN,
                                                       self.tr('Minimum zoom'),
                                                       minValue=0,
                                                       maxValue=25,
                                                       defaultValue=12))
        self.addParameter(QgsProcessingParameterNumber(self.ZOOM_MAX,
                                                       self.tr('Maximum zoom'),
                                                       minValue=0,
                                                       maxValue=25,
                                                       defaultValue=12))
        self.addParameter(QgsProcessingParameterNumber(self.DPI,
                                                       self.tr('DPI'),
                                                       minValue=48,
                                                       maxValue=600,
                                                       defaultValue=96))
        self.formats = ['PNG', 'JPG']
        self.addParameter(QgsProcessingParameterEnum(self.TILE_FORMAT,
                                                     self.tr('Tile format'),
                                                     self.formats,
                                                     defaultValue=0))
        self.outputs = ['Directory', 'MBTiles']
        self.addParameter(QgsProcessingParameterEnum(self.OUTPUT_FORMAT,
                                                     self.tr('Output format'),
                                                     self.outputs,
                                                     defaultValue=0))
        self.addParameter(QgsProcessingParameterFolderDestination(self.OUTPUT_DIRECTORY,
                                                                  self.tr('Output directory'),
                                                                  optional=True))
        self.addParameter(QgsProcessingParameterFileDestination(self.OUTPUT_FILE,
                                                                self.tr('Output file (for MBTiles)'),
                                                                self.tr('MBTiles files (*.mbtiles)'),
                                                                optional=True))

    def name(self):
        return 'tilesxyz'

    def displayName(self):
        return self.tr('Generate XYZ tiles')

    def prepareAlgorithm(self, parameters, context, feedback):
        project = context.project()
        visible_layers = [item.layer() for item in project.layerTreeRoot().findLayers() if item.isVisible()]
        self.layers = [l for l in project.layerTreeRoot().layerOrder() if l in visible_layers]
        return True

    def processAlgorithm(self, parameters, context, feedback):
        feedback.setProgress(1)

        extent = self.parameterAsExtent(parameters, self.EXTENT, context)
        min_zoom = self.parameterAsInt(parameters, self.ZOOM_MIN, context)
        max_zoom = self.parameterAsInt(parameters, self.ZOOM_MAX, context)
        dpi = self.parameterAsInt(parameters, self.DPI, context)
        tile_format = self.formats[self.parameterAsEnum(parameters, self.TILE_FORMAT, context)]
        output_format = self.outputs[self.parameterAsEnum(parameters, self.OUTPUT_FORMAT, context)]
        if output_format == 'Directory':
            output_dir = self.parameterAsString(parameters, self.OUTPUT_DIRECTORY, context)
            if not output_dir:
                raise QgsProcessingException(self.tr('You need to specify output directory.'))
        else:  # MBTiles
            output_file = self.parameterAsString(parameters, self.OUTPUT_FILE, context)
            if not output_file:
                raise QgsProcessingException(self.tr('You need to specify output filename.'))
        tile_width = 256
        tile_height = 256

        wgs_crs = QgsCoordinateReferenceSystem('EPSG:4326')
        dest_crs = QgsCoordinateReferenceSystem('EPSG:3857')

        project = context.project()
        src_to_wgs = QgsCoordinateTransform(project.crs(), wgs_crs, context.transformContext())
        wgs_to_dest = QgsCoordinateTransform(wgs_crs, dest_crs, context.transformContext())

        settings = QgsMapSettings()
        settings.setOutputImageFormat(QImage.Format_ARGB32_Premultiplied)
        settings.setDestinationCrs(dest_crs)
        settings.setLayers(self.layers)
        settings.setOutputDpi(dpi)

        wgs_extent = src_to_wgs.transformBoundingBox(extent)
        wgs_extent = [wgs_extent.xMinimum(), wgs_extent.yMinimum(), wgs_extent.xMaximum(), wgs_extent.yMaximum()]

        metatiles_by_zoom = {}
        metatiles_count = 0
        for zoom in range(min_zoom, max_zoom + 1):
            metatiles = get_metatiles(wgs_extent, zoom, 4)
            metatiles_by_zoom[zoom] = metatiles
            metatiles_count += len(metatiles)

        lab_buffer_px = 100
        progress = 0

        tile_params = {
            'format': tile_format,
            'quality': 75,
            'width': tile_width,
            'height': tile_height
        }
        if output_format == 'Directory':
            writer = DirectoryWriter(output_dir, tile_params)
        else:
            writer = MBTilesWriter(output_file, tile_params, wgs_extent, min_zoom, max_zoom)

        for zoom in range(min_zoom, max_zoom + 1):
            feedback.pushConsoleInfo('Generating tiles for zoom level: %s' % zoom)

            for i, metatile in enumerate(metatiles_by_zoom[zoom]):
                size = QSize(tile_width * metatile.rows(), tile_height * metatile.columns())
                extent = QgsRectangle(*metatile.extent())
                settings.setExtent(wgs_to_dest.transformBoundingBox(extent))
                settings.setOutputSize(size)

                label_area = QgsRectangle(settings.extent())
                lab_buffer = label_area.width() * (lab_buffer_px / size.width())
                label_area.set(
                    label_area.xMinimum() + lab_buffer,
                    label_area.yMinimum() + lab_buffer,
                    label_area.xMaximum() - lab_buffer,
                    label_area.yMaximum() - lab_buffer
                )
                settings.setLabelBoundaryGeometry(QgsGeometry.fromRect(label_area))

                image = QImage(size, QImage.Format_ARGB32_Premultiplied)
                image.fill(Qt.transparent)
                dpm = settings.outputDpi() / 25.4 * 1000
                image.setDotsPerMeterX(dpm)
                image.setDotsPerMeterY(dpm)
                painter = QPainter(image)
                job = QgsMapRendererCustomPainterJob(settings, painter)
                job.renderSynchronously()
                painter.end()

                # For analysing metatiles (labels, etc.)
                # metatile_dir = os.path.join(output_dir, str(zoom))
                # os.makedirs(metatile_dir, exist_ok=True)
                # image.save(os.path.join(metatile_dir, 'metatile_%s.png' % i))

                for r, c, tile in metatile.tiles:
                    tile_img = image.copy(tile_width * r, tile_height * c, tile_width, tile_height)
                    writer.writeTile(tile, tile_img)

                progress += 1
                feedback.setProgress(100 * (progress / metatiles_count))

        writer.close()

        results = {}
        if output_format == 'Directory':
            results['OUTPUT_DIRECTORY'] = output_dir
        else:  # MBTiles
            results['OUTPUT_FILE'] = output_file
        return results

    def checkParameterValues(self, parameters, context):
        min_zoom = self.parameterAsInt(parameters, self.ZOOM_MIN, context)
        max_zoom = self.parameterAsInt(parameters, self.ZOOM_MAX, context)
        if max_zoom < min_zoom:
            return False, self.tr('Invalid zoom levels range.')

        return super().checkParameterValues(parameters, context)
