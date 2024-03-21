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

from qgis.PyQt.QtCore import (
    QSize,
    QDate,
    QTime,
    QDateTime
)
from qgis.core import (
    Qgis,
    QgsCoordinateReferenceSystem,
    QgsGeometry,
    QgsMapClippingRegion,
    QgsMapSettings,
    QgsRasterLayer,
    QgsRectangle,
    QgsDoubleRange,
    QgsSingleBandGrayRenderer,
    QgsContrastEnhancement,
    QgsRasterLayerElevationProperties,
    QgsProperty,
    QgsDateTimeRange
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
        map settings has a z range filter
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

    def test_render_fixed_range_per_band_with_z_range_filter(self):
        """
        Test rendering a raster with a fixed range per band when
        map settings has a z range filter
        """
        raster_layer = QgsRasterLayer(os.path.join(TEST_DATA_DIR, 'landsat_4326.tif'))
        self.assertTrue(raster_layer.isValid())

        renderer = QgsSingleBandGrayRenderer(raster_layer.dataProvider(), 3)
        contrast = QgsContrastEnhancement()
        contrast.setMinimumValue(70)
        contrast.setMaximumValue(125)
        renderer.setContrastEnhancement(contrast)
        raster_layer.setRenderer(renderer)

        # set layer as elevation enabled
        raster_layer.elevationProperties().setEnabled(True)
        raster_layer.elevationProperties().setMode(
            Qgis.RasterElevationMode.FixedRangePerBand
        )
        raster_layer.elevationProperties().setFixedRangePerBand(
            {3: QgsDoubleRange(33, 38),
             4: QgsDoubleRange(35, 40),
             5: QgsDoubleRange(40, 48)}
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
                'No Z range filter on map settings, elevation range per band',
                'elevation_range_per_band_no_filter',
                map_settings)
        )

        # map settings range matches band 3 only
        map_settings.setZRange(QgsDoubleRange(30, 34))
        self.assertTrue(
            self.render_map_settings_check(
                'Z range filter on map settings matches band 3 only',
                'elevation_range_per_band_match_3',
                map_settings)
        )

        # map settings range matches band 3 and 4, should pick the highest (4)
        map_settings.setZRange(QgsDoubleRange(36, 38.5))
        self.assertTrue(
            self.render_map_settings_check(
                'Z range filter on map settings matches band 3 and 4',
                'elevation_range_per_band_match_4',
                map_settings)
        )

        # map settings range matches band 5
        map_settings.setZRange(QgsDoubleRange(46, 58.5))
        self.assertTrue(
            self.render_map_settings_check(
                'Z range filter on map settings matches band 5',
                'elevation_range_per_band_match_5',
                map_settings)
        )

        # map settings range excludes layer's range
        map_settings.setZRange(QgsDoubleRange(130, 135))
        self.assertTrue(
            self.render_map_settings_check(
                'Z range filter on map settings outside of layer band ranges',
                'fixed_elevation_range_excluded',
                map_settings)
        )

    def test_render_dynamic_range_per_band_with_z_range_filter(self):
        """
        Test rendering a raster with a dynamic range per band when
        map settings has a z range filter
        """
        raster_layer = QgsRasterLayer(os.path.join(TEST_DATA_DIR, 'landsat_4326.tif'))
        self.assertTrue(raster_layer.isValid())

        renderer = QgsSingleBandGrayRenderer(raster_layer.dataProvider(), 3)
        contrast = QgsContrastEnhancement()
        contrast.setMinimumValue(70)
        contrast.setMaximumValue(125)
        renderer.setContrastEnhancement(contrast)
        raster_layer.setRenderer(renderer)

        # set layer as elevation enabled
        raster_layer.elevationProperties().setEnabled(True)
        raster_layer.elevationProperties().setMode(
            Qgis.RasterElevationMode.DynamicRangePerBand
        )

        raster_layer.elevationProperties().dataDefinedProperties().setProperty(
            QgsRasterLayerElevationProperties.Property.RasterPerBandLowerElevation,
            QgsProperty.fromExpression('case when @band=3 then 33 when @band=4 then 35 when @band=5 then 40 else null end')
        )
        raster_layer.elevationProperties().dataDefinedProperties().setProperty(
            QgsRasterLayerElevationProperties.Property.RasterPerBandUpperElevation,
            QgsProperty.fromExpression('case when @band=3 then 38 when @band=4 then 40 when @band=5 then 48 else null end')
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
                'No Z range filter on map settings, elevation range per band',
                'elevation_range_per_band_no_filter',
                map_settings)
        )

        # map settings range matches band 3 only
        map_settings.setZRange(QgsDoubleRange(30, 34))
        self.assertTrue(
            self.render_map_settings_check(
                'Z range filter on map settings matches band 3 only',
                'elevation_range_per_band_match_3',
                map_settings)
        )

        # map settings range matches band 3 and 4, should pick the highest (4)
        map_settings.setZRange(QgsDoubleRange(36, 38.5))
        self.assertTrue(
            self.render_map_settings_check(
                'Z range filter on map settings matches band 3 and 4',
                'elevation_range_per_band_match_4',
                map_settings)
        )

        # map settings range matches band 5
        map_settings.setZRange(QgsDoubleRange(46, 58.5))
        self.assertTrue(
            self.render_map_settings_check(
                'Z range filter on map settings matches band 5',
                'elevation_range_per_band_match_5',
                map_settings)
        )

        # map settings range excludes layer's range
        map_settings.setZRange(QgsDoubleRange(130, 135))
        self.assertTrue(
            self.render_map_settings_check(
                'Z range filter on map settings outside of layer band ranges',
                'fixed_elevation_range_excluded',
                map_settings)
        )

    def test_render_fixed_temporal_range_with_temporal_range_filter(self):
        """
        Test rendering a raster with a fixed temporal range when
        map settings has a temporal range filter
        """
        raster_layer = QgsRasterLayer(os.path.join(TEST_DATA_DIR, '3d', 'dtm.tif'))
        self.assertTrue(raster_layer.isValid())

        # set layer as elevation enabled
        raster_layer.temporalProperties().setIsActive(True)
        raster_layer.temporalProperties().setMode(
            Qgis.RasterTemporalMode.FixedTemporalRange
        )
        raster_layer.temporalProperties().setFixedTemporalRange(
            QgsDateTimeRange(
                QDateTime(QDate(2023, 1, 1),
                          QTime(0, 0, 0)),
                QDateTime(QDate(2023, 12, 31),
                          QTime(23, 59, 59))
            )
        )

        map_settings = QgsMapSettings()
        map_settings.setOutputSize(QSize(400, 400))
        map_settings.setOutputDpi(96)
        map_settings.setDestinationCrs(raster_layer.crs())
        map_settings.setExtent(raster_layer.extent())
        map_settings.setLayers([raster_layer])

        # no filter on map settings
        map_settings.setIsTemporal(False)
        self.assertTrue(
            self.render_map_settings_check(
                'No temporal filter on map settings, fixed temporal range layer',
                'dem_no_filter',
                map_settings)
        )

        # map settings range includes layer's range
        map_settings.setIsTemporal(True)
        map_settings.setTemporalRange(QgsDateTimeRange(
            QDateTime(QDate(2022, 1, 1),
                      QTime(0, 0, 0)),
            QDateTime(QDate(2023, 6, 30),
                      QTime(23, 59, 59))
        ))
        self.assertTrue(
            self.render_map_settings_check(
                'Temporal range filter on map settings includes layers fixed range',
                'fixed_elevation_range_included',
                map_settings)
        )

        # map settings range excludes layer's range
        map_settings.setTemporalRange(QgsDateTimeRange(
            QDateTime(QDate(2024, 1, 1),
                      QTime(0, 0, 0)),
            QDateTime(QDate(2024, 6, 30),
                      QTime(23, 59, 59))
        ))
        self.assertTrue(
            self.render_map_settings_check(
                'Temporal range filter on map settings outside of layers fixed range',
                'fixed_elevation_range_excluded',
                map_settings)
        )

    def test_render_fixed_range_per_band_with_temporal_range_filter(self):
        """
        Test rendering a raster with a fixed temporal range per band when
        map settings has a temporal range filter
        """
        raster_layer = QgsRasterLayer(os.path.join(TEST_DATA_DIR, 'landsat_4326.tif'))
        self.assertTrue(raster_layer.isValid())

        renderer = QgsSingleBandGrayRenderer(raster_layer.dataProvider(), 3)
        contrast = QgsContrastEnhancement()
        contrast.setMinimumValue(70)
        contrast.setMaximumValue(125)
        renderer.setContrastEnhancement(contrast)
        raster_layer.setRenderer(renderer)

        # set layer as temporal enabled
        raster_layer.temporalProperties().setIsActive(True)
        raster_layer.temporalProperties().setMode(
            Qgis.RasterTemporalMode.FixedRangePerBand
        )
        raster_layer.temporalProperties().setFixedRangePerBand(
            {
                3: QgsDateTimeRange(
                    QDateTime(QDate(2023, 5, 6), QTime(12, 13, 14)),
                    QDateTime(QDate(2023, 5, 8), QTime(12, 13, 14))
                ),
                4: QgsDateTimeRange(
                    QDateTime(QDate(2023, 5, 7),
                              QTime(12, 13, 14)),
                    QDateTime(QDate(2023, 5, 9),
                              QTime(12, 13, 14))
                ),
                5: QgsDateTimeRange(
                    QDateTime(QDate(2023, 5, 9),
                              QTime(12, 13, 14)),
                    QDateTime(QDate(2023, 5, 11),
                              QTime(12, 13, 14))
                )}
        )

        map_settings = QgsMapSettings()
        map_settings.setOutputSize(QSize(400, 400))
        map_settings.setOutputDpi(96)
        map_settings.setDestinationCrs(raster_layer.crs())
        map_settings.setExtent(raster_layer.extent())
        map_settings.setLayers([raster_layer])

        # no filter on map settings
        map_settings.setIsTemporal(False)
        self.assertTrue(
            self.render_map_settings_check(
                'No temporal range filter on map settings, temporal range per band',
                'elevation_range_per_band_no_filter',
                map_settings)
        )

        # map settings range matches band 3 only
        map_settings.setIsTemporal(True)
        map_settings.setTemporalRange(QgsDateTimeRange(
            QDateTime(QDate(2023, 5, 3),
                      QTime(12, 13, 14)),
            QDateTime(QDate(2023, 5, 6),
                      QTime(13, 13, 14))
        ))
        self.assertTrue(
            self.render_map_settings_check(
                'Temporal range filter on map settings matches band 3 only',
                'elevation_range_per_band_match_3',
                map_settings)
        )

        # map settings range matches band 3 and 4, should pick the latest (4)
        map_settings.setTemporalRange(QgsDateTimeRange(
            QDateTime(QDate(2023, 5, 5),
                      QTime(12, 13, 14)),
            QDateTime(QDate(2023, 5, 8),
                      QTime(13, 13, 14))
        ))
        self.assertTrue(
            self.render_map_settings_check(
                'Temporal range filter on map settings matches band 3 and 4',
                'elevation_range_per_band_match_4',
                map_settings)
        )

        # map settings range matches band 5
        map_settings.setTemporalRange(QgsDateTimeRange(
            QDateTime(QDate(2023, 5, 10),
                      QTime(12, 13, 14)),
            QDateTime(QDate(2023, 5, 15),
                      QTime(13, 13, 14))
        ))
        self.assertTrue(
            self.render_map_settings_check(
                'Temporal range filter on map settings matches band 5',
                'elevation_range_per_band_match_5',
                map_settings)
        )

        # map settings range excludes layer's range
        map_settings.setTemporalRange(QgsDateTimeRange(
            QDateTime(QDate(2024, 5, 10),
                      QTime(12, 13, 14)),
            QDateTime(QDate(2024, 5, 15),
                      QTime(13, 13, 14))
        ))
        self.assertTrue(
            self.render_map_settings_check(
                'Temporal range filter on map settings outside of layer band ranges',
                'fixed_elevation_range_excluded',
                map_settings)
        )


if __name__ == '__main__':
    unittest.main()
