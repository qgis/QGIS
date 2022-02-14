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

import os
import math
import re
import urllib.parse
from uuid import uuid4

import sqlite3
from osgeo import gdal
from qgis.PyQt.QtCore import QSize, Qt, QByteArray, QBuffer
from qgis.PyQt.QtGui import QColor, QImage, QPainter
from qgis.core import (QgsProcessingException,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterString,
                       QgsProcessingParameterExtent,
                       QgsProcessingParameterColor,
                       QgsProcessingOutputFile,
                       QgsProcessingParameterFileDestination,
                       QgsProcessingParameterFolderDestination,
                       QgsGeometry,
                       QgsRectangle,
                       QgsMapSettings,
                       QgsCoordinateTransform,
                       QgsCoordinateReferenceSystem,
                       QgsMapRendererCustomPainterJob,
                       QgsLabelingEngineSettings,
                       QgsApplication,
                       QgsExpressionContextUtils,
                       QgsProcessingAlgorithm)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
import threading
from concurrent.futures import ThreadPoolExecutor
from processing.core.ProcessingConfig import ProcessingConfig


# TMS functions taken from https://alastaira.wordpress.com/2011/07/06/converting-tms-tile-coordinates-to-googlebingosm-tile-coordinates/ #spellok
def tms(ytile, zoom):
    n = 2.0 ** zoom
    ytile = n - ytile - 1
    return int(ytile)


# Math functions taken from https://wiki.openstreetmap.org/wiki/Slippy_map_tilenames #spellok
def deg2num(lat_deg, lon_deg, zoom):
    lat_rad = math.radians(lat_deg)
    n = 2.0 ** zoom
    xtile = int((lon_deg + 180.0) / 360.0 * n)
    ytile = int((1.0 - math.log(math.tan(lat_rad) + (1 / math.cos(lat_rad))) / math.pi) / 2.0 * n)
    return (xtile, ytile)


# Math functions taken from https://wiki.openstreetmap.org/wiki/Slippy_map_tilenames #spellok
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


class TilesXYZAlgorithmBase(QgisAlgorithm):
    EXTENT = 'EXTENT'
    ZOOM_MIN = 'ZOOM_MIN'
    ZOOM_MAX = 'ZOOM_MAX'
    DPI = 'DPI'
    BACKGROUND_COLOR = 'BACKGROUND_COLOR'
    TILE_FORMAT = 'TILE_FORMAT'
    QUALITY = 'QUALITY'
    METATILESIZE = 'METATILESIZE'

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
        self.addParameter(QgsProcessingParameterColor(self.BACKGROUND_COLOR,
                                                      self.tr('Background color'),
                                                      defaultValue=QColor(Qt.transparent),
                                                      optional=True))
        self.formats = ['PNG', 'JPG']
        self.addParameter(QgsProcessingParameterEnum(self.TILE_FORMAT,
                                                     self.tr('Tile format'),
                                                     self.formats,
                                                     defaultValue=0))
        self.addParameter(QgsProcessingParameterNumber(self.QUALITY,
                                                       self.tr('Quality (JPG only)'),
                                                       minValue=1,
                                                       maxValue=100,
                                                       defaultValue=75))
        self.addParameter(QgsProcessingParameterNumber(self.METATILESIZE,
                                                       self.tr('Metatile size'),
                                                       minValue=1,
                                                       maxValue=20,
                                                       defaultValue=4))
        self.thread_nr_re = re.compile('[0-9]+$')  # thread number regex

    def prepareAlgorithm(self, parameters, context, feedback):
        project = context.project()
        visible_layers = [item.layer() for item in project.layerTreeRoot().findLayers() if item.isVisible()]
        self.layers = [l for l in project.layerTreeRoot().layerOrder() if l in visible_layers]
        return True

    def renderSingleMetatile(self, metatile):
        if self.feedback.isCanceled():
            return
            # Haven't found a better way to break than to make all the new threads return instantly

        if "Dummy" in threading.current_thread().name or len(self.settingsDictionary) == 1:  # single thread testing
            threadSpecificSettings = list(self.settingsDictionary.values())[0]
        else:
            thread_nr = self.thread_nr_re.search(threading.current_thread().name)[0]  # terminating number only
            threadSpecificSettings = self.settingsDictionary[thread_nr]

        size = QSize(self.tile_width * metatile.rows(), self.tile_height * metatile.columns())
        extent = QgsRectangle(*metatile.extent())
        threadSpecificSettings.setExtent(self.wgs_to_dest.transformBoundingBox(extent))
        threadSpecificSettings.setOutputSize(size)

        # Append MapSettings scope in order to update map variables (e.g @map_scale) with new extent data
        exp_context = threadSpecificSettings.expressionContext()
        exp_context.appendScope(QgsExpressionContextUtils.mapSettingsScope(threadSpecificSettings))
        threadSpecificSettings.setExpressionContext(exp_context)

        image = QImage(size, QImage.Format_ARGB32_Premultiplied)
        image.fill(self.color)
        dpm = round(threadSpecificSettings.outputDpi() / 25.4 * 1000)
        image.setDotsPerMeterX(dpm)
        image.setDotsPerMeterY(dpm)
        painter = QPainter(image)
        job = QgsMapRendererCustomPainterJob(threadSpecificSettings, painter)
        job.renderSynchronously()
        painter.end()

        # For analysing metatiles (labels, etc.)
        # metatile_dir = os.path.join(output_dir, str(zoom))
        # os.makedirs(metatile_dir, exist_ok=True)
        # image.save(os.path.join(metatile_dir, 'metatile_%s.png' % i))

        for r, c, tile in metatile.tiles:
            tileImage = image.copy(self.tile_width * r, self.tile_height * c, self.tile_width, self.tile_height)
            self.writer.write_tile(tile, tileImage)

        # to stop thread sync issues
        with self.progressThreadLock:
            self.progress += 1
            self.feedback.setProgress(100 * (self.progress / self.totalMetatiles))

    def generate(self, writer, parameters, context, feedback):
        self.feedback = feedback
        feedback.setProgress(1)

        extent = self.parameterAsExtent(parameters, self.EXTENT, context)
        self.min_zoom = self.parameterAsInt(parameters, self.ZOOM_MIN, context)
        self.max_zoom = self.parameterAsInt(parameters, self.ZOOM_MAX, context)
        dpi = self.parameterAsInt(parameters, self.DPI, context)
        self.color = self.parameterAsColor(parameters, self.BACKGROUND_COLOR, context)
        self.tile_format = self.formats[self.parameterAsEnum(parameters, self.TILE_FORMAT, context)]
        self.quality = self.parameterAsInt(parameters, self.QUALITY, context)
        self.metatilesize = self.parameterAsInt(parameters, self.METATILESIZE, context)
        self.maxThreads = int(ProcessingConfig.getSetting(ProcessingConfig.MAX_THREADS))
        try:
            self.tile_width = self.parameterAsInt(parameters, self.TILE_WIDTH, context)
            self.tile_height = self.parameterAsInt(parameters, self.TILE_HEIGHT, context)
        except AttributeError:
            self.tile_width = 256
            self.tile_height = 256

        wgs_crs = QgsCoordinateReferenceSystem('EPSG:4326')
        dest_crs = QgsCoordinateReferenceSystem('EPSG:3857')

        project = context.project()
        self.src_to_wgs = QgsCoordinateTransform(project.crs(), wgs_crs, context.transformContext())
        self.wgs_to_dest = QgsCoordinateTransform(wgs_crs, dest_crs, context.transformContext())
        # without re-writing, we need a different settings for each thread to stop async errors
        # naming doesn't always line up, but the last number does
        self.settingsDictionary = {str(i): QgsMapSettings() for i in range(self.maxThreads)}
        for thread in self.settingsDictionary:
            self.settingsDictionary[thread].setOutputImageFormat(QImage.Format_ARGB32_Premultiplied)
            self.settingsDictionary[thread].setDestinationCrs(dest_crs)
            self.settingsDictionary[thread].setLayers(self.layers)
            self.settingsDictionary[thread].setOutputDpi(dpi)
            if self.tile_format == 'PNG':
                self.settingsDictionary[thread].setBackgroundColor(self.color)

            # disable partial labels (they would be cut at the edge of tiles)
            labeling_engine_settings = self.settingsDictionary[thread].labelingEngineSettings()
            labeling_engine_settings.setFlag(QgsLabelingEngineSettings.UsePartialCandidates, False)
            self.settingsDictionary[thread].setLabelingEngineSettings(labeling_engine_settings)

            # Transfer context scopes to MapSettings
            self.settingsDictionary[thread].setExpressionContext(context.expressionContext())

        self.wgs_extent = self.src_to_wgs.transformBoundingBox(extent)
        self.wgs_extent = [self.wgs_extent.xMinimum(), self.wgs_extent.yMinimum(), self.wgs_extent.xMaximum(),
                           self.wgs_extent.yMaximum()]

        metatiles_by_zoom = {}
        self.totalMetatiles = 0
        allMetatiles = []
        for zoom in range(self.min_zoom, self.max_zoom + 1):
            metatiles = get_metatiles(self.wgs_extent, zoom, self.metatilesize)
            metatiles_by_zoom[zoom] = metatiles
            allMetatiles += metatiles
            self.totalMetatiles += len(metatiles)

        lab_buffer_px = 100
        self.progress = 0

        tile_params = {
            'format': self.tile_format,
            'quality': self.quality,
            'width': self.tile_width,
            'height': self.tile_height,
            'min_zoom': self.min_zoom,
            'max_zoom': self.max_zoom,
            'extent': self.wgs_extent,
        }
        writer.set_parameters(tile_params)
        self.writer = writer

        self.progressThreadLock = threading.Lock()
        if self.maxThreads > 1:
            feedback.pushConsoleInfo(self.tr('Using {max_threads} CPU Threads:').format(max_threads=self.maxThreads))
            for zoom in range(self.min_zoom, self.max_zoom + 1):
                feedback.pushConsoleInfo(self.tr('Generating tiles for zoom level: {zoom}').format(zoom=zoom))
                with ThreadPoolExecutor(max_workers=self.maxThreads) as threadPool:
                    for result in threadPool.map(self.renderSingleMetatile, metatiles_by_zoom[zoom]):
                        # re-raise exceptions from threads
                        _ = result
        else:
            feedback.pushConsoleInfo(self.tr('Using 1 CPU Thread:'))
            for zoom in range(self.min_zoom, self.max_zoom + 1):
                feedback.pushConsoleInfo(self.tr('Generating tiles for zoom level: {zoom}').format(zoom=zoom))
                for i, metatile in enumerate(metatiles_by_zoom[zoom]):
                    self.renderSingleMetatile(metatile)

        writer.close()

    def checkParameterValues(self, parameters, context):
        min_zoom = self.parameterAsInt(parameters, self.ZOOM_MIN, context)
        max_zoom = self.parameterAsInt(parameters, self.ZOOM_MAX, context)
        if max_zoom < min_zoom:
            return False, self.tr('Invalid zoom levels range.')

        return super().checkParameterValues(parameters, context)


########################################################################
# MBTiles
########################################################################
class MBTilesWriter:

    def __init__(self, filename):
        base_dir = os.path.dirname(filename)
        os.makedirs(base_dir, exist_ok=True)
        self.filename = filename

    def set_parameters(self, tile_params):
        self.extent = tile_params.get('extent')
        self.tile_width = tile_params.get('width', 256)
        self.tile_height = tile_params.get('height', 256)
        self.min_zoom = tile_params.get('min_zoom')
        self.max_zoom = tile_params.get('max_zoom')
        tile_format = tile_params['format']
        options = []
        if tile_format == 'JPG':
            tile_format = 'JPEG'
            options = ['QUALITY=%s' % tile_params.get('quality', 75)]
        driver = gdal.GetDriverByName('MBTiles')
        ds = driver.Create(self.filename, 1, 1, 1, options=['TILE_FORMAT=%s' % tile_format] + options)
        ds = None

        self._execute_sqlite(
            "INSERT INTO metadata(name, value) VALUES ('{}', '{}');".format('minzoom', self.min_zoom),
            "INSERT INTO metadata(name, value) VALUES ('{}', '{}');".format('maxzoom', self.max_zoom),
            # will be set properly after writing all tiles
            "INSERT INTO metadata(name, value) VALUES ('{}', '');".format('bounds')
        )
        self._zoom = None

    def _execute_sqlite(self, *commands):
        conn = sqlite3.connect(self.filename)
        for cmd in commands:
            conn.execute(cmd)
        conn.commit()
        conn.close()

    def _init_zoom_layer(self, zoom):
        self._zoom_ds = None
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

        bounds = ','.join(map(str, zoom_extent))
        self._execute_sqlite("UPDATE metadata SET value='{}' WHERE name='bounds'".format(bounds))

        self._zoom_ds = gdal.OpenEx(self.filename, 1, open_options=['ZOOM_LEVEL=%s' % first_tile.z])
        self._first_tile = first_tile
        self._zoom = zoom

    def write_tile(self, tile, image):
        if tile.z != self._zoom:
            self._init_zoom_layer(tile.z)

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
        self._zoom_ds.WriteRaster(xoff, yoff, self.tile_width, self.tile_height, data)

    def close(self):
        self._zoom_ds = None
        bounds = ','.join(map(str, self.extent))
        self._execute_sqlite("UPDATE metadata SET value='{}' WHERE name='bounds'".format(bounds))


class TilesXYZAlgorithmMBTiles(TilesXYZAlgorithmBase):
    OUTPUT_FILE = 'OUTPUT_FILE'

    def initAlgorithm(self, config=None):
        super(TilesXYZAlgorithmMBTiles, self).initAlgorithm()
        self.addParameter(QgsProcessingParameterFileDestination(self.OUTPUT_FILE,
                                                                self.tr('Output file (for MBTiles)'),
                                                                self.tr('MBTiles files (*.mbtiles)'),
                                                                optional=True))

    def name(self):
        return 'tilesxyzmbtiles'

    def displayName(self):
        return self.tr('Generate XYZ tiles (MBTiles)')

    def group(self):
        return self.tr('Raster tools')

    def groupId(self):
        return 'rastertools'

    def flags(self):
        return super().flags() | QgsProcessingAlgorithm.FlagRequiresProject

    def processAlgorithm(self, parameters, context, feedback):
        output_file = self.parameterAsString(parameters, self.OUTPUT_FILE, context)
        if not output_file:
            raise QgsProcessingException(self.tr('You need to specify output filename.'))

        writer = MBTilesWriter(output_file)
        self.generate(writer, parameters, context, feedback)

        results = {'OUTPUT_FILE': output_file}
        return results


########################################################################
# Directory
########################################################################
LEAFLET_TEMPLATE = '''
<!DOCTYPE html>
<html>
<head>
  <title>{tilesetname}</title>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0">

  <link rel="stylesheet" href="https://unpkg.com/leaflet@1.5.1/dist/leaflet.css"
   integrity="sha512-xwE/Az9zrjBIphAcBb3F6JVqxf46+CDLwfLMHloNu6KEQCAWi6HcDUbeOfBIptF7tcCzusKFjFw2yuvEpDL9wQ=="
   crossorigin=""/>
  <script src="https://unpkg.com/leaflet@1.5.1/dist/leaflet.js"
   integrity="sha512-GffPMF3RvMeYyc1LWMHtK8EbPv0iNZ8/oTtHPx9/cc2ILxQ+u905qIwdpULaqDkyBKgOaB57QTMg7ztg8Jm2Og=="
   crossorigin=""></script>
  <style type="text/css">
    body {{
       margin: 0;
       padding: 0;
    }}
    html, body, #map{{
       width: 100%;
       height: 100%;
    }}
  </style>
</head>
<body>
  <div id="map"></div>
  <script>
      var map = L.map('map').setView([{centery}, {centerx}], {avgzoom});
      L.tileLayer({tilesource}, {{
        minZoom: {minzoom},
        maxZoom: {maxzoom},
        tms: {tms},
        attribution: 'Generated by TilesXYZ'
      }}).addTo(map);
  </script>
</body>
</html>
'''


class DirectoryWriter:

    def __init__(self, folder, is_tms):
        self.folder = folder
        self.is_tms = is_tms

    def set_parameters(self, tile_params):
        self.format = tile_params.get('format', 'PNG')
        self.quality = tile_params.get('quality', -1)

    def write_tile(self, tile, image):
        directory = os.path.join(self.folder, str(tile.z), str(tile.x))
        os.makedirs(directory, exist_ok=True)
        ytile = tile.y
        if self.is_tms:
            ytile = tms(ytile, tile.z)
        path = os.path.join(directory, '{}.{}'.format(ytile, self.format.lower()))
        image.save(path, self.format, self.quality)
        return path

    def close(self):
        pass


class TilesXYZAlgorithmDirectory(TilesXYZAlgorithmBase):
    TMS_CONVENTION = 'TMS_CONVENTION'
    OUTPUT_DIRECTORY = 'OUTPUT_DIRECTORY'
    OUTPUT_HTML = 'OUTPUT_HTML'
    TILE_WIDTH = 'TILE_WIDTH'
    TILE_HEIGHT = 'TILE_HEIGHT'

    def initAlgorithm(self, config=None):
        super(TilesXYZAlgorithmDirectory, self).initAlgorithm()
        self.addParameter(QgsProcessingParameterNumber(self.TILE_WIDTH,
                                                       self.tr('Tile width'),
                                                       minValue=1,
                                                       maxValue=4096,
                                                       defaultValue=256))
        self.addParameter(QgsProcessingParameterNumber(self.TILE_HEIGHT,
                                                       self.tr('Tile height'),
                                                       minValue=1,
                                                       maxValue=4096,
                                                       defaultValue=256))
        self.addParameter(QgsProcessingParameterBoolean(self.TMS_CONVENTION,
                                                        self.tr('Use inverted tile Y axis (TMS convention)'),
                                                        defaultValue=False,
                                                        optional=True))
        self.addParameter(QgsProcessingParameterFolderDestination(self.OUTPUT_DIRECTORY,
                                                                  self.tr('Output directory'),
                                                                  optional=True))
        self.addParameter(QgsProcessingParameterFileDestination(self.OUTPUT_HTML,
                                                                self.tr('Output html (Leaflet)'),
                                                                self.tr('HTML files (*.html)'),
                                                                optional=True))

    def name(self):
        return 'tilesxyzdirectory'

    def displayName(self):
        return self.tr('Generate XYZ tiles (Directory)')

    def group(self):
        return self.tr('Raster tools')

    def groupId(self):
        return 'rastertools'

    def processAlgorithm(self, parameters, context, feedback):
        is_tms = self.parameterAsBoolean(parameters, self.TMS_CONVENTION, context)
        output_html = self.parameterAsString(parameters, self.OUTPUT_HTML, context)
        output_dir = self.parameterAsString(parameters, self.OUTPUT_DIRECTORY, context)
        if not output_dir:
            raise QgsProcessingException(self.tr('You need to specify output directory.'))

        writer = DirectoryWriter(output_dir, is_tms)
        self.generate(writer, parameters, context, feedback)

        results = {'OUTPUT_DIRECTORY': output_dir}

        if output_html:
            output_dir_safe = urllib.parse.quote(output_dir.replace('\\', '/'))
            html_code = LEAFLET_TEMPLATE.format(
                tilesetname="Leaflet Preview",
                centerx=self.wgs_extent[0] + (self.wgs_extent[2] - self.wgs_extent[0]) / 2,
                centery=self.wgs_extent[1] + (self.wgs_extent[3] - self.wgs_extent[1]) / 2,
                avgzoom=(self.max_zoom + self.min_zoom) / 2,
                tilesource="'file:///{}/{{z}}/{{x}}/{{y}}.{}'".format(output_dir_safe, self.tile_format.lower()),
                minzoom=self.min_zoom,
                maxzoom=self.max_zoom,
                tms='true' if is_tms else 'false'
            )
            with open(output_html, "w") as fh:
                fh.write(html_code)
            results['OUTPUT_HTML'] = output_html

        return results
