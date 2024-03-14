"""QGIS Unit tests for QgsRasterLayerRenderer

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '2020-06'
__copyright__ = 'Copyright 2020, The QGIS Project'

import os

from qgis.PyQt.QtCore import QSize
from qgis.core import (
    Qgis,
    QgsCoordinateReferenceSystem,
    QgsGeometry,
    QgsMapClippingRegion,
    QgsMapSettings,
    QgsRasterLayer,
    QgsRectangle,
    QgsDoubleRange
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

# Convenience instances in case you may need them
# not used in this test
start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsRasterLayerRenderer(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return 'rasterlayerrenderer'

    def testRenderWithPainterClipRegions(self):
        raster_layer = QgsRasterLayer(os.path.join(TEST_DATA_DIR, 'rgb256x256.png'))
        self.assertTrue(raster_layer.isValid())
        raster_layer.setCrs(QgsCoordinateReferenceSystem('EPSG:3857'))

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        mapsettings.setExtent(QgsRectangle(0.0001451, -0.0001291, 0.0021493, -0.0021306))
        mapsettings.setLayers([raster_layer])

        region = QgsMapClippingRegion(QgsGeometry.fromWkt('Polygon ((0.00131242078273144 -0.00059281669806561, 0.00066744230712249 -0.00110186995774045, 0.00065145110524788 -0.00152830200772984, 0.00141369839460392 -0.00189076925022083, 0.00210931567614912 -0.00094195793899443, 0.00169354442740946 -0.00067810310806349, 0.00131242078273144 -0.00059281669806561))'))
        region.setFeatureClip(QgsMapClippingRegion.FeatureClippingType.ClipPainterOnly)
        region2 = QgsMapClippingRegion(QgsGeometry.fromWkt('Polygon ((0.00067010750743492 -0.0007740503193111, 0.00064612070462302 -0.00151764120648011, 0.00153629760897587 -0.00158693641460339, 0.0014909892036645 -0.00063812510337699, 0.00106722235398754 -0.00055816909400397, 0.00067010750743492 -0.0007740503193111))'))
        region2.setFeatureClip(QgsMapClippingRegion.FeatureClippingType.ClipToIntersection)
        mapsettings.addClippingRegion(region)
        mapsettings.addClippingRegion(region2)

        self.assertTrue(
            self.render_map_settings_check(
                'painterclip_region',
                'painterclip_region',
                mapsettings)
        )

    def test_render_dem_with_z_range_filter(self):
        raster_layer = QgsRasterLayer(os.path.join(TEST_DATA_DIR, '3d', 'dtm.tif'))
        self.assertTrue(raster_layer.isValid())
        # start with no elevation settings on layer
        self.assertFalse(raster_layer.elevationProperties().hasElevation())

        map_settings = QgsMapSettings()
        map_settings.setOutputSize(QSize(400, 400))
        map_settings.setOutputDpi(96)
        map_settings.setDestinationCrs(raster_layer.crs())
        map_settings.setExtent(raster_layer.extent())
        map_settings.setLayers([raster_layer])
        map_settings.setZRange(QgsDoubleRange(100, 130))

        self.assertTrue(
            self.render_map_settings_check(
                'Z range filter on map settings, not elevation enabled layer',
                'dem_no_filter',
                map_settings)
        )

        # set layer as elevation enabled
        raster_layer.elevationProperties().setEnabled(True)
        # no filter on map settings
        map_settings.setZRange(QgsDoubleRange())
        self.assertTrue(
            self.render_map_settings_check(
                'No Z range filter on map settings, elevation enabled layer',
                'dem_no_filter',
                map_settings)
        )

        # filter on map settings, elevation enabled layer => should be filtered
        map_settings.setZRange(QgsDoubleRange(100, 130))
        self.assertTrue(
            self.render_map_settings_check(
                'Z range filter on map settings, elevation enabled layer',
                'dem_filter',
                map_settings)
        )

        # with offset and scaling
        raster_layer.elevationProperties().setZOffset(50)
        raster_layer.elevationProperties().setZScale(0.75)
        self.assertTrue(
            self.render_map_settings_check(
                'Z range filter on map settings, elevation enabled layer with offset and scale',
                'dem_filter_offset_and_scale',
                map_settings)
        )

    def test_render_fixed_elevation_range_with_z_range_filter(self):
        """
        Test rendering a raster with a fixed elevation range when
        map settings has a z range filtrer
        """
        raster_layer = QgsRasterLayer(os.path.join(TEST_DATA_DIR, '3d', 'dtm.tif'))
        self.assertTrue(raster_layer.isValid())

        # set layer as elevation enabled
        raster_layer.elevationProperties().setEnabled(True)
        raster_layer.elevationProperties().setMode(
            Qgis.RasterElevationMode.FixedElevationRange
        )
        raster_layer.elevationProperties().setFixedRange(
            QgsDoubleRange(33, 38)
        )

        map_settings = QgsMapSettings()
        map_settings.setOutputSize(QSize(400, 400))
        map_settings.setOutputDpi(96)
        map_settings.setDestinationCrs(raster_layer.crs())
        map_settings.setExtent(raster_layer.extent())
        map_settings.setLayers([raster_layer])

        # no filter on map settings
        map_settings.setZRange(QgsDoubleRange())
        self.assertTrue(
            self.render_map_settings_check(
                'No Z range filter on map settings, fixed elevation range layer',
                'dem_no_filter',
                map_settings)
        )

        # map settings range includes layer's range
        map_settings.setZRange(QgsDoubleRange(30, 35))
        self.assertTrue(
            self.render_map_settings_check(
                'Z range filter on map settings includes layers fixed range',
                'fixed_elevation_range_included',
                map_settings)
        )

        # map settings range excludes layer's range
        map_settings.setZRange(QgsDoubleRange(130, 135))
        self.assertTrue(
            self.render_map_settings_check(
                'Z range filter on map settings outside of layers fixed range',
                'fixed_elevation_range_excluded',
                map_settings)
        )


if __name__ == '__main__':
    unittest.main()
