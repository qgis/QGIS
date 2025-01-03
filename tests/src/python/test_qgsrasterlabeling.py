"""QGIS Unit tests for raster labeling

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import unittest

from qgis.PyQt.QtCore import QSize
from qgis.PyQt.QtGui import QColor
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    Qgis,
    QgsRasterLayerSimpleLabeling,
    QgsCoordinateReferenceSystem,
    QgsTextFormat,
    QgsMapSettings,
    QgsRasterLayer,
    QgsRectangle,
    QgsReadWriteContext,
    QgsAbstractRasterLayerLabeling,
    QgsBasicNumericFormat,
    QgsCurrencyNumericFormat,
    QgsPercentageNumericFormat,
)
from qgis.testing import start_app, QgisTestCase

from utilities import getTestFont

# Convenience instances in case you may need them
# not used in this test
start_app()


class TestQgsRasterLabeling(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "raster_labeling"

    def test_raster_labeling(self):
        raster_layer = QgsRasterLayer(
            self.get_test_data_path("rgb256x256.png").as_posix()
        )
        self.assertTrue(raster_layer.isValid())
        self.assertFalse(raster_layer.labelsEnabled())
        self.assertIsNone(raster_layer.labeling())
        raster_layer.setLabelsEnabled(True)
        # no labeling, so not enabled
        self.assertFalse(raster_layer.labelsEnabled())
        raster_layer.setLabelsEnabled(False)

        labeling = QgsRasterLayerSimpleLabeling()
        raster_layer.setLabeling(labeling)
        self.assertEqual(raster_layer.labeling(), labeling)
        self.assertFalse(raster_layer.labelsEnabled())

        raster_layer.setLabelsEnabled(True)
        self.assertTrue(raster_layer.labelsEnabled())

        doc = QDomDocument()
        context = QgsReadWriteContext()
        element = doc.createElement("maplayer")
        raster_layer.writeLayerXml(element, doc, context)

        raster_layer2 = QgsRasterLayer(
            self.get_test_data_path("rgb256x256.png").as_posix()
        )
        raster_layer2.readLayerXml(element, context)

        self.assertIsInstance(raster_layer2.labeling(), QgsRasterLayerSimpleLabeling)
        self.assertTrue(raster_layer2.labelsEnabled())

    def test_simple_labeling(self):
        labeling = QgsRasterLayerSimpleLabeling()
        labeling.setBand(3)
        text_format = QgsTextFormat()
        text_format.setSize(33)
        labeling.setTextFormat(text_format)
        labeling.setNumericFormat(QgsCurrencyNumericFormat())
        labeling.setPriority(0.2)
        labeling.placementSettings().setOverlapHandling(
            Qgis.LabelOverlapHandling.AllowOverlapIfRequired
        )
        labeling.thinningSettings().setLimitNumberLabelsEnabled(True)
        labeling.thinningSettings().setMaximumNumberLabels(123)
        labeling.thinningSettings().setMinimumFeatureSize(16)
        labeling.setZIndex(22)
        labeling.setMaximumScale(12345)
        labeling.setMinimumScale(123456)
        labeling.setScaleBasedVisibility(True)
        labeling.setResampleOver(3)
        labeling.setResampleMethod(Qgis.RasterResamplingMethod.CubicSpline)

        self.assertEqual(labeling.band(), 3)
        self.assertEqual(labeling.textFormat().size(), 33)
        self.assertIsInstance(labeling.numericFormat(), QgsCurrencyNumericFormat)
        self.assertEqual(labeling.priority(), 0.2)
        self.assertEqual(
            labeling.placementSettings().overlapHandling(),
            Qgis.LabelOverlapHandling.AllowOverlapIfRequired,
        )
        self.assertEqual(labeling.thinningSettings().limitNumberOfLabelsEnabled(), True)
        self.assertEqual(labeling.thinningSettings().maximumNumberLabels(), 123)
        self.assertEqual(labeling.thinningSettings().minimumFeatureSize(), 16)
        self.assertEqual(labeling.zIndex(), 22)
        self.assertEqual(labeling.maximumScale(), 12345)
        self.assertEqual(labeling.minimumScale(), 123456)
        self.assertEqual(labeling.hasScaleBasedVisibility(), True)
        self.assertEqual(labeling.resampleOver(), 3)
        self.assertEqual(
            labeling.resampleMethod(), Qgis.RasterResamplingMethod.CubicSpline
        )

        labeling_clone = labeling.clone()
        self.assertIsInstance(labeling_clone, QgsRasterLayerSimpleLabeling)
        self.assertEqual(labeling_clone.band(), 3)
        self.assertEqual(labeling_clone.textFormat().size(), 33)
        self.assertIsInstance(labeling_clone.numericFormat(), QgsCurrencyNumericFormat)
        self.assertEqual(labeling_clone.priority(), 0.2)
        self.assertEqual(
            labeling_clone.placementSettings().overlapHandling(),
            Qgis.LabelOverlapHandling.AllowOverlapIfRequired,
        )
        self.assertEqual(
            labeling_clone.thinningSettings().limitNumberOfLabelsEnabled(), True
        )
        self.assertEqual(labeling_clone.thinningSettings().maximumNumberLabels(), 123)
        self.assertEqual(labeling_clone.thinningSettings().minimumFeatureSize(), 16)
        self.assertEqual(labeling_clone.zIndex(), 22)
        self.assertEqual(labeling_clone.maximumScale(), 12345)
        self.assertEqual(labeling_clone.minimumScale(), 123456)
        self.assertEqual(labeling_clone.hasScaleBasedVisibility(), True)
        self.assertEqual(labeling_clone.resampleOver(), 3)
        self.assertEqual(
            labeling_clone.resampleMethod(), Qgis.RasterResamplingMethod.CubicSpline
        )

        doc = QDomDocument()
        context = QgsReadWriteContext()
        element = labeling.save(doc, context)

        labeling_from_xml = QgsAbstractRasterLayerLabeling.createFromElement(
            element, context
        )
        self.assertIsInstance(labeling_from_xml, QgsRasterLayerSimpleLabeling)
        self.assertEqual(labeling_from_xml.band(), 3)
        self.assertEqual(labeling_from_xml.textFormat().size(), 33)
        self.assertIsInstance(
            labeling_from_xml.numericFormat(), QgsCurrencyNumericFormat
        )
        self.assertEqual(labeling_from_xml.priority(), 0.2)
        self.assertEqual(
            labeling_from_xml.placementSettings().overlapHandling(),
            Qgis.LabelOverlapHandling.AllowOverlapIfRequired,
        )
        self.assertEqual(
            labeling_from_xml.thinningSettings().limitNumberOfLabelsEnabled(), True
        )
        self.assertEqual(
            labeling_from_xml.thinningSettings().maximumNumberLabels(), 123
        )
        self.assertEqual(labeling_from_xml.thinningSettings().minimumFeatureSize(), 16)
        self.assertEqual(labeling_from_xml.zIndex(), 22)
        self.assertEqual(labeling_from_xml.maximumScale(), 12345)
        self.assertEqual(labeling_from_xml.minimumScale(), 123456)
        self.assertEqual(labeling_from_xml.hasScaleBasedVisibility(), True)
        self.assertEqual(labeling_from_xml.resampleOver(), 3)
        self.assertEqual(
            labeling_from_xml.resampleMethod(), Qgis.RasterResamplingMethod.CubicSpline
        )

    def test_render_int(self):
        raster_layer = QgsRasterLayer(
            self.get_test_data_path("raster/byte.tif").as_posix()
        )
        self.assertTrue(raster_layer.isValid())

        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setColor(QColor(255, 0, 0))
        format.setSize(30)
        labeling = QgsRasterLayerSimpleLabeling()
        labeling.setBand(1)
        labeling.setTextFormat(format)

        raster_layer.setLabeling(labeling)
        raster_layer.setLabelsEnabled(True)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
        transform_context = mapsettings.transformContext()
        transform_context.addCoordinateOperation(
            raster_layer.crs(),
            QgsCoordinateReferenceSystem("EPSG:3857"),
            "+proj=pipeline +step +inv +proj=utm +zone=11 +ellps=clrk66 +step +proj=push +v_3 +step +proj=cart +ellps=clrk66 +step +proj=helmert +x=-10 +y=158 +z=187 +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=webmerc +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84",
        )
        mapsettings.setTransformContext(transform_context)
        mapsettings.setExtent(
            QgsRectangle(-13095009.8, 4014898.9, -13094808.4, 4015061.7)
        )
        mapsettings.setLayers([raster_layer])

        self.assertTrue(
            self.render_map_settings_check("labeling_int", "labeling_int", mapsettings)
        )

    def test_render_rotated_map(self):
        raster_layer = QgsRasterLayer(
            self.get_test_data_path("raster/byte.tif").as_posix()
        )
        self.assertTrue(raster_layer.isValid())

        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setColor(QColor(255, 0, 0))
        format.setSize(30)
        labeling = QgsRasterLayerSimpleLabeling()
        labeling.setBand(1)
        labeling.setTextFormat(format)

        raster_layer.setLabeling(labeling)
        raster_layer.setLabelsEnabled(True)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setRotation(45)
        mapsettings.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
        transform_context = mapsettings.transformContext()
        transform_context.addCoordinateOperation(
            raster_layer.crs(),
            QgsCoordinateReferenceSystem("EPSG:3857"),
            "+proj=pipeline +step +inv +proj=utm +zone=11 +ellps=clrk66 +step +proj=push +v_3 +step +proj=cart +ellps=clrk66 +step +proj=helmert +x=-10 +y=158 +z=187 +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=webmerc +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84",
        )
        mapsettings.setTransformContext(transform_context)
        mapsettings.setExtent(
            QgsRectangle(-13095009.8, 4014898.9, -13094808.4, 4015061.7)
        )
        mapsettings.setLayers([raster_layer])

        self.assertTrue(
            self.render_map_settings_check(
                "labeling_rotated", "labeling_rotated", mapsettings
            )
        )

    def test_render_numeric_format(self):
        raster_layer = QgsRasterLayer(
            self.get_test_data_path("raster/byte.tif").as_posix()
        )
        self.assertTrue(raster_layer.isValid())

        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setColor(QColor(255, 0, 0))
        format.setSize(24)
        labeling = QgsRasterLayerSimpleLabeling()
        labeling.setBand(1)
        labeling.setTextFormat(format)

        labeling.setNumericFormat(QgsPercentageNumericFormat())

        raster_layer.setLabeling(labeling)
        raster_layer.setLabelsEnabled(True)

        mapsettings = QgsMapSettings()
        transform_context = mapsettings.transformContext()
        transform_context.addCoordinateOperation(
            raster_layer.crs(),
            QgsCoordinateReferenceSystem("EPSG:3857"),
            "+proj=pipeline +step +inv +proj=utm +zone=11 +ellps=clrk66 +step +proj=push +v_3 +step +proj=cart +ellps=clrk66 +step +proj=helmert +x=-10 +y=158 +z=187 +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=webmerc +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84",
        )
        mapsettings.setTransformContext(transform_context)
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
        mapsettings.setExtent(
            QgsRectangle(-13095009.8, 4014898.9, -13094808.4, 4015061.7)
        )
        mapsettings.setLayers([raster_layer])

        self.assertTrue(
            self.render_map_settings_check(
                "numeric_format", "numeric_format", mapsettings
            )
        )

    def test_render_resampled(self):
        raster_layer = QgsRasterLayer(
            self.get_test_data_path("raster/dem.tif").as_posix()
        )
        self.assertTrue(raster_layer.isValid())

        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setColor(QColor(255, 0, 0))
        format.setSize(30)
        labeling = QgsRasterLayerSimpleLabeling()
        labeling.setBand(1)
        labeling.setTextFormat(format)
        labeling.setResampleOver(4)
        labeling.setResampleMethod(Qgis.RasterResamplingMethod.Average)

        numeric_format = QgsBasicNumericFormat()
        numeric_format.setNumberDecimalPlaces(1)
        labeling.setNumericFormat(numeric_format)

        raster_layer.setLabeling(labeling)
        raster_layer.setLabelsEnabled(True)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(800, 800))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
        mapsettings.setExtent(QgsRectangle(2080356.7, 5746858.1, 2080585.6, 5747055.3))
        mapsettings.setLayers([raster_layer])

        self.assertTrue(
            self.render_map_settings_check("resampling", "resampling", mapsettings)
        )


if __name__ == "__main__":
    unittest.main()
