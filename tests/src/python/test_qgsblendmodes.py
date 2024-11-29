"""
***************************************************************************
    test_qgsblendmodes.py
    ---------------------
    Date                 : May 2013
    Copyright            : (C) 2013 by Nyall Dawson, Massimo Endrighi
    Email                : nyall dot dawson at gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Nyall Dawson"
__date__ = "May 2013"
__copyright__ = "(C) 2013, Nyall Dawson, Massimo Endrighi"

import os
import unittest

from qgis.PyQt.QtCore import QSize
from qgis.PyQt.QtGui import QColor, QPainter
from qgis.core import (
    Qgis,
    QgsMapSettings,
    QgsMultiBandColorRenderer,
    QgsPainting,
    QgsRasterLayer,
    QgsRectangle,
    QgsVectorLayer,
    QgsVectorSimplifyMethod,
)
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsBlendModes(QgisTestCase):

    def __init__(self, methodName):
        """Run once on class initialization."""
        QgisTestCase.__init__(self, methodName)

        # create point layer
        shp_file = os.path.join(TEST_DATA_DIR, "points.shp")
        self.point_layer = QgsVectorLayer(shp_file, "Points", "ogr")

        simplify_method = QgsVectorSimplifyMethod()
        simplify_method.setSimplifyHints(
            QgsVectorSimplifyMethod.SimplifyHint.NoSimplification
        )

        # create polygon layer
        shp_file = os.path.join(TEST_DATA_DIR, "polys.shp")
        self.polygon_layer = QgsVectorLayer(shp_file, "Polygons", "ogr")
        self.polygon_layer.setSimplifyMethod(simplify_method)

        # create line layer
        shp_file = os.path.join(TEST_DATA_DIR, "lines.shp")
        self.line_layer = QgsVectorLayer(shp_file, "Lines", "ogr")
        self.line_layer.setSimplifyMethod(simplify_method)

        # create two raster layers
        raster_file = os.path.join(TEST_DATA_DIR, "rgb256x256.png")
        self.raster_layer1 = QgsRasterLayer(raster_file, "raster1")
        self.raster_layer2 = QgsRasterLayer(raster_file, "raster2")
        multi_band_renderer1 = QgsMultiBandColorRenderer(
            self.raster_layer1.dataProvider(), 1, 2, 3
        )
        self.raster_layer1.setRenderer(multi_band_renderer1)
        multi_band_renderer2 = QgsMultiBandColorRenderer(
            self.raster_layer2.dataProvider(), 1, 2, 3
        )
        self.raster_layer2.setRenderer(multi_band_renderer2)

        # to match blend modes test comparisons background
        self.map_settings = QgsMapSettings()
        self.map_settings.setLayers([self.raster_layer1, self.raster_layer2])
        self.map_settings.setBackgroundColor(QColor(152, 219, 249))
        self.map_settings.setOutputSize(QSize(400, 400))
        self.map_settings.setOutputDpi(96)

        self.extent = QgsRectangle(
            -118.8888888888887720,
            22.8002070393376783,
            -83.3333333333331581,
            46.8719806763287536,
        )

    def testVectorBlending(self):
        """Test that blend modes work for vector layers."""

        # Add vector layers to map
        self.map_settings.setLayers([self.line_layer, self.polygon_layer])
        self.map_settings.setExtent(self.extent)

        # Set blending modes for both layers
        self.line_layer.setBlendMode(
            QPainter.CompositionMode.CompositionMode_Difference
        )
        self.polygon_layer.setBlendMode(
            QPainter.CompositionMode.CompositionMode_Difference
        )

        result = self.render_map_settings_check(
            "vector_blendmodes",
            "vector_blendmodes",
            self.map_settings,
            allowed_mismatch=20,
            color_tolerance=1,
        )

        # Reset layers
        self.line_layer.setBlendMode(
            QPainter.CompositionMode.CompositionMode_SourceOver
        )
        self.polygon_layer.setBlendMode(
            QPainter.CompositionMode.CompositionMode_SourceOver
        )

        self.assertTrue(result)

    def testVectorFeatureBlending(self):
        """Test that feature blend modes work for vector layers."""

        # Add vector layers to map
        self.map_settings.setLayers([self.line_layer, self.polygon_layer])
        self.map_settings.setExtent(self.extent)

        # Set feature blending for line layer
        self.line_layer.setFeatureBlendMode(
            QPainter.CompositionMode.CompositionMode_Plus
        )

        result = self.render_map_settings_check(
            "vector_featureblendmodes",
            "vector_featureblendmodes",
            self.map_settings,
            allowed_mismatch=20,
            color_tolerance=1,
        )

        # Reset layers
        self.line_layer.setFeatureBlendMode(
            QPainter.CompositionMode.CompositionMode_SourceOver
        )

        self.assertTrue(result)

    def testVectorLayerOpacity(self):
        """Test that layer opacity works for vector layers."""

        # Add vector layers to map
        self.map_settings.setLayers([self.line_layer, self.polygon_layer])
        self.map_settings.setExtent(self.extent)

        # Set feature blending for line layer
        self.line_layer.setOpacity(0.5)

        result = self.render_map_settings_check(
            "vector_layertransparency",
            "vector_layertransparency",
            self.map_settings,
            allowed_mismatch=20,
            color_tolerance=1,
        )

        self.line_layer.setOpacity(1)
        self.assertTrue(result)

    def testRasterBlending(self):
        """Test that blend modes work for raster layers."""
        # Add raster layers to map
        self.map_settings.setLayers([self.raster_layer1, self.raster_layer2])
        self.map_settings.setExtent(self.raster_layer1.extent())

        # Set blending mode for top layer
        self.raster_layer1.setBlendMode(
            QPainter.CompositionMode.CompositionMode_Difference
        )

        result = self.render_map_settings_check(
            "raster_blendmodes",
            "raster_blendmodes",
            self.map_settings,
            allowed_mismatch=20,
            color_tolerance=1,
        )

        self.raster_layer1.setBlendMode(
            QPainter.CompositionMode.CompositionMode_SourceOver
        )

        self.assertTrue(result)

    def test_is_clipping_mode(self):
        self.assertFalse(QgsPainting.isClippingMode(Qgis.BlendMode.Normal))
        self.assertFalse(QgsPainting.isClippingMode(Qgis.BlendMode.Lighten))
        self.assertTrue(QgsPainting.isClippingMode(Qgis.BlendMode.SourceIn))
        self.assertTrue(QgsPainting.isClippingMode(Qgis.BlendMode.DestinationIn))
        self.assertTrue(QgsPainting.isClippingMode(Qgis.BlendMode.SourceOut))
        self.assertTrue(QgsPainting.isClippingMode(Qgis.BlendMode.DestinationOut))
        self.assertTrue(QgsPainting.isClippingMode(Qgis.BlendMode.SourceAtop))
        self.assertTrue(QgsPainting.isClippingMode(Qgis.BlendMode.DestinationAtop))


if __name__ == "__main__":
    unittest.main()
