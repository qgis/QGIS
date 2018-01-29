# -*- coding: utf-8 -*-

"""
/***************************************************************************
 Rasterize.py
                              -------------------
        begin                : 2016-10-05
        copyright            : (C) 2016 by OPENGIS.ch
        email                : matthias@opengis.ch
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
"""

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm

from qgis.PyQt.QtGui import QImage, QPainter, QColor
from qgis.PyQt.QtCore import QSize
from qgis.core import (
    QgsMapSettings,
    QgsMapRendererCustomPainterJob,
    QgsRectangle,
    QgsProject,
    QgsProcessingException,
    QgsProcessingParameterExtent,
    QgsProcessingParameterString,
    QgsProcessingParameterNumber,
    QgsProcessingParameterBoolean,
    QgsProcessingParameterMapLayer,
    QgsProcessingParameterRasterDestination,
    QgsRasterFileWriter
)

import qgis
import osgeo.gdal
import os
import tempfile
import math

__author__ = 'Matthias Kuhn'
__date__ = '2016-10-05'
__copyright__ = '(C) 2016 by OPENGIS.ch'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'


class RasterizeAlgorithm(QgisAlgorithm):

    """Processing algorithm renders map canvas to a raster file.
    It's possible to choose the following parameters:
        - Map theme to render
        - Layer to render
        - The minimum extent to render
        - The tile size
        - Map unit per pixel
        - The output (can be saved to a file or to a temporary file and
          automatically opened as layer in qgis)
    """

    # Constants used to refer to parameters and outputs. They will be
    # used when calling the algorithm from another algorithm, or when
    # calling from the QGIS console.

    OUTPUT = 'OUTPUT'
    MAP_THEME = 'MAP_THEME'
    LAYER = 'LAYER'
    EXTENT = 'EXTENT'
    TILE_SIZE = 'TILE_SIZE'
    MAP_UNITS_PER_PIXEL = 'MAP_UNITS_PER_PIXEL'
    MAKE_BACKGROUND_TRANSPARENT = 'MAKE_BACKGROUND_TRANSPARENT'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        """Here we define the inputs and output of the algorithm, along
        with some other properties.
        """
        # The parameters
        self.addParameter(
            QgsProcessingParameterExtent(self.EXTENT, description=self.tr(
                'Minimum extent to render')))

        self.addParameter(
            QgsProcessingParameterNumber(
                self.TILE_SIZE,
                self.tr('Tile size'),
                defaultValue=1024, minValue=64))

        self.addParameter(QgsProcessingParameterNumber(
            self.MAP_UNITS_PER_PIXEL,
            self.tr(
                'Map units per '
                'pixel'),
            defaultValue=100,
            minValue=0,
            type=QgsProcessingParameterNumber.Double
        ))

        self.addParameter(
            QgsProcessingParameterBoolean(
                self.MAKE_BACKGROUND_TRANSPARENT,
                self.tr('Make background transparent'),
                defaultValue=False))

        map_theme_param = QgsProcessingParameterString(
            self.MAP_THEME,
            description=self.tr(
                'Map theme to render'),
            defaultValue=None, optional=True)

        map_theme_param.setMetadata(
            {'widget_wrapper': {
                'class':
                    'processing.gui.wrappers_map_theme.MapThemeWrapper'}})
        self.addParameter(map_theme_param)

        self.addParameter(
            QgsProcessingParameterMapLayer(
                self.LAYER,
                description=self.tr(
                    'Single layer to render'),
                optional=True))

        # We add a raster layer as output
        self.addParameter(QgsProcessingParameterRasterDestination(
            self.OUTPUT,
            self.tr(
                'Output layer')))

    def name(self):
        # Unique (non-user visible) name of algorithm
        return 'rasterize'

    def displayName(self):
        # The name that the user will see in the toolbox
        return self.tr('Convert map to raster')

    def group(self):
        return self.tr('Raster tools')

    def groupId(self):
        return 'rastertools'

    def tags(self):
        return self.tr('layer,raster,convert,file,map themes,tiles,render').split(',')

    # def processAlgorithm(self, progress):
    def processAlgorithm(self, parameters, context, feedback):
        """Here is where the processing itself takes place."""

        # The first thing to do is retrieve the values of the parameters
        # entered by the user
        map_theme = self.parameterAsString(
            parameters,
            self.MAP_THEME,
            context)

        layer = self.parameterAsLayer(
            parameters,
            self.LAYER,
            context)

        extent = self.parameterAsExtent(
            parameters,
            self.EXTENT,
            context)

        tile_size = self.parameterAsInt(
            parameters,
            self.TILE_SIZE,
            context)

        make_trans = self.parameterAsBool(
            parameters,
            self.MAKE_BACKGROUND_TRANSPARENT,
            context)

        mupp = self.parameterAsDouble(
            parameters,
            self.MAP_UNITS_PER_PIXEL,
            context)

        output_layer = self.parameterAsOutputLayer(
            parameters,
            self.OUTPUT,
            context)

        tile_set = TileSet(map_theme, layer, extent, tile_size, mupp,
                           output_layer, make_trans,
                           qgis.utils.iface.mapCanvas().mapSettings())
        tile_set.render(feedback, make_trans)

        return {self.OUTPUT: output_layer}


class TileSet():

    """
    A set of tiles
    """

    def __init__(self, map_theme, layer, extent, tile_size, mupp, output,
                 make_trans, map_settings):
        """
        :param map_theme:
        :param extent:
        :param layer:
        :param tile_size:
        :param mupp:
        :param output:
        :param map_settings: Map canvas map settings used for some fallback
        values and CRS
        """

        self.extent = extent
        self.mupp = mupp
        self.tile_size = tile_size

        driver = self.getDriverForFile(output)

        if not driver:
            raise QgsProcessingException(
                u'Could not load GDAL driver for file {}'.format(output))

        crs = map_settings.destinationCrs()

        self.x_tile_count = math.ceil(extent.width() / mupp / tile_size)
        self.y_tile_count = math.ceil(extent.height() / mupp / tile_size)

        xsize = self.x_tile_count * tile_size
        ysize = self.y_tile_count * tile_size

        if make_trans:
            no_bands = 4
        else:
            no_bands = 3

        self.dataset = driver.Create(output, xsize, ysize, no_bands)
        self.dataset.SetProjection(str(crs.toWkt()))
        self.dataset.SetGeoTransform(
            [extent.xMinimum(), mupp, 0, extent.yMaximum(), 0, -mupp])

        self.image = QImage(QSize(tile_size, tile_size), QImage.Format_ARGB32)

        self.settings = QgsMapSettings()
        self.settings.setOutputDpi(self.image.logicalDpiX())
        self.settings.setOutputImageFormat(QImage.Format_ARGB32)
        self.settings.setDestinationCrs(crs)
        self.settings.setOutputSize(self.image.size())
        self.settings.setFlag(QgsMapSettings.Antialiasing, True)
        self.settings.setFlag(QgsMapSettings.RenderMapTile, True)
        self.settings.setFlag(QgsMapSettings.UseAdvancedEffects, True)

        if make_trans:
            self.settings.setBackgroundColor(QColor(255, 255, 255, 0))
        else:
            self.settings.setBackgroundColor(QColor(255, 255, 255))

        if QgsProject.instance().mapThemeCollection().hasMapTheme(map_theme):
            self.settings.setLayers(
                QgsProject.instance().mapThemeCollection(

                ).mapThemeVisibleLayers(
                    map_theme))
            self.settings.setLayerStyleOverrides(
                QgsProject.instance().mapThemeCollection(

                ).mapThemeStyleOverrides(
                    map_theme))
        elif layer:
            self.settings.setLayers([layer])
        else:
            self.settings.setLayers(map_settings.layers())

    def render(self, feedback, make_trans):
        for x in range(self.x_tile_count):
            for y in range(self.y_tile_count):
                if feedback.isCanceled():
                    return
                cur_tile = x * self.y_tile_count + y
                num_tiles = self.x_tile_count * self.y_tile_count
                self.renderTile(x, y, feedback, make_trans)

                feedback.setProgress(int((cur_tile / num_tiles) * 100))

    def renderTile(self, x, y, feedback, make_trans):
        """
        Render one tile
        :param x: The x index of the current tile
        :param y: The y index of the current tile
        """

        if make_trans:
            background_color = QColor(255, 255, 255, 0)
            self.image.fill(background_color.rgba())
        else:
            background_color = QColor(255, 255, 255)
            self.image.fill(background_color.rgb())

        painter = QPainter(self.image)

        self.settings.setExtent(QgsRectangle(
            self.extent.xMinimum() + x * self.mupp * self.tile_size,
            self.extent.yMaximum() - (y + 1) * self.mupp * self.tile_size,
            self.extent.xMinimum() + (x + 1) * self.mupp * self.tile_size,
            self.extent.yMaximum() - y * self.mupp * self.tile_size))

        job = QgsMapRendererCustomPainterJob(self.settings, painter)
        job.renderSynchronously()
        painter.end()

        # Needs not to be deleted or Windows will kill it too early...
        tmpfile = tempfile.NamedTemporaryFile(suffix='.png', delete=False)
        try:
            self.image.save(tmpfile.name)

            src_ds = osgeo.gdal.Open(tmpfile.name)

            self.dataset.WriteRaster(x * self.tile_size, y * self.tile_size,
                                     self.tile_size, self.tile_size,
                                     src_ds.ReadRaster(0, 0, self.tile_size,
                                                       self.tile_size))
        except Exception as e:
            feedback.reportError(str(e))
        finally:
            del src_ds
            tmpfile.close()
            os.unlink(tmpfile.name)

    def getDriverForFile(self, filename):
        """
        Get the GDAL driver for a filename, based on its extension. (.gpkg,
        .mbtiles...)
        """
        _, extension = os.path.splitext(filename)

        # If no extension is set, use .tif as default
        if extension == '':
            extension = '.tif'

        driver_name = QgsRasterFileWriter.driverForExtension(extension[1:])
        return osgeo.gdal.GetDriverByName(driver_name)
