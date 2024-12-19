"""QGIS Unit tests for QgsPointCloudRgbRenderer

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "09/11/2020"
__copyright__ = "Copyright 2020, The QGIS Project"

from qgis.PyQt.QtCore import QDir, QSize
from qgis.PyQt.QtGui import QPainter
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    Qgis,
    QgsContrastEnhancement,
    QgsCoordinateReferenceSystem,
    QgsDoubleRange,
    QgsGeometry,
    QgsMapClippingRegion,
    QgsMapSettings,
    QgsMapUnitScale,
    QgsPointCloudLayer,
    QgsPointCloudRenderContext,
    QgsPointCloudRenderer,
    QgsPointCloudRgbRenderer,
    QgsProviderRegistry,
    QgsReadWriteContext,
    QgsRectangle,
    QgsRenderContext,
    QgsUnitTypes,
    QgsVector3D,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()


class TestQgsPointCloudRgbRenderer(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "pointcloudrenderer"

    @unittest.skipIf(
        "ept" not in QgsProviderRegistry.instance().providerList(),
        "EPT provider not available",
    )
    def testSetLayer(self):
        layer = QgsPointCloudLayer(
            unitTestDataPath() + "/point_clouds/ept/rgb/ept.json", "test", "ept"
        )
        self.assertTrue(layer.isValid())

        # test that a point cloud with RGB attributes is automatically assigned the RGB renderer by default
        self.assertIsInstance(layer.renderer(), QgsPointCloudRgbRenderer)

        # for this point cloud, we should default to 0-255 ranges (ie. no contrast enhancement)
        self.assertIsNone(layer.renderer().redContrastEnhancement())
        self.assertIsNone(layer.renderer().greenContrastEnhancement())
        self.assertIsNone(layer.renderer().blueContrastEnhancement())

    @unittest.skipIf(
        "ept" not in QgsProviderRegistry.instance().providerList(),
        "EPT provider not available",
    )
    def testSetLayer16(self):
        layer = QgsPointCloudLayer(
            unitTestDataPath() + "/point_clouds/ept/rgb16/ept.json", "test", "ept"
        )
        self.assertTrue(layer.isValid())

        # test that a point cloud with RGB attributes is automatically assigned the RGB renderer by default
        self.assertIsInstance(layer.renderer(), QgsPointCloudRgbRenderer)

        # for this point cloud, we should default to 0-65024 ranges with contrast enhancement
        self.assertEqual(layer.renderer().redContrastEnhancement().minimumValue(), 0)
        self.assertEqual(
            layer.renderer().redContrastEnhancement().maximumValue(), 65535.0
        )
        self.assertEqual(
            layer.renderer().redContrastEnhancement().contrastEnhancementAlgorithm(),
            QgsContrastEnhancement.ContrastEnhancementAlgorithm.StretchToMinimumMaximum,
        )
        self.assertEqual(layer.renderer().greenContrastEnhancement().minimumValue(), 0)
        self.assertEqual(
            layer.renderer().greenContrastEnhancement().maximumValue(), 65535.0
        )
        self.assertEqual(
            layer.renderer().greenContrastEnhancement().contrastEnhancementAlgorithm(),
            QgsContrastEnhancement.ContrastEnhancementAlgorithm.StretchToMinimumMaximum,
        )
        self.assertEqual(layer.renderer().blueContrastEnhancement().minimumValue(), 0)
        self.assertEqual(
            layer.renderer().blueContrastEnhancement().maximumValue(), 65535.0
        )
        self.assertEqual(
            layer.renderer().blueContrastEnhancement().contrastEnhancementAlgorithm(),
            QgsContrastEnhancement.ContrastEnhancementAlgorithm.StretchToMinimumMaximum,
        )

    def testBasic(self):
        renderer = QgsPointCloudRgbRenderer()
        renderer.setBlueAttribute("b")
        self.assertEqual(renderer.blueAttribute(), "b")
        renderer.setGreenAttribute("g")
        self.assertEqual(renderer.greenAttribute(), "g")
        renderer.setRedAttribute("r")
        self.assertEqual(renderer.redAttribute(), "r")

        redce = QgsContrastEnhancement()
        redce.setMinimumValue(100)
        redce.setMaximumValue(120)
        redce.setContrastEnhancementAlgorithm(
            QgsContrastEnhancement.ContrastEnhancementAlgorithm.StretchAndClipToMinimumMaximum
        )
        renderer.setRedContrastEnhancement(redce)

        greence = QgsContrastEnhancement()
        greence.setMinimumValue(130)
        greence.setMaximumValue(150)
        greence.setContrastEnhancementAlgorithm(
            QgsContrastEnhancement.ContrastEnhancementAlgorithm.StretchToMinimumMaximum
        )
        renderer.setGreenContrastEnhancement(greence)

        bluece = QgsContrastEnhancement()
        bluece.setMinimumValue(170)
        bluece.setMaximumValue(190)
        bluece.setContrastEnhancementAlgorithm(
            QgsContrastEnhancement.ContrastEnhancementAlgorithm.ClipToMinimumMaximum
        )
        renderer.setBlueContrastEnhancement(bluece)

        renderer.setMaximumScreenError(18)
        renderer.setMaximumScreenErrorUnit(QgsUnitTypes.RenderUnit.RenderInches)
        renderer.setPointSize(13)
        renderer.setPointSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        renderer.setPointSizeMapUnitScale(QgsMapUnitScale(1000, 2000))

        rr = renderer.clone()
        self.assertEqual(rr.maximumScreenError(), 18)
        self.assertEqual(
            rr.maximumScreenErrorUnit(), QgsUnitTypes.RenderUnit.RenderInches
        )
        self.assertEqual(rr.pointSize(), 13)
        self.assertEqual(rr.pointSizeUnit(), QgsUnitTypes.RenderUnit.RenderPoints)
        self.assertEqual(rr.pointSizeMapUnitScale().minScale, 1000)
        self.assertEqual(rr.pointSizeMapUnitScale().maxScale, 2000)

        self.assertEqual(rr.blueAttribute(), "b")
        self.assertEqual(rr.greenAttribute(), "g")
        self.assertEqual(rr.redAttribute(), "r")
        self.assertEqual(rr.redContrastEnhancement().minimumValue(), 100)
        self.assertEqual(rr.redContrastEnhancement().maximumValue(), 120)
        self.assertEqual(
            rr.redContrastEnhancement().contrastEnhancementAlgorithm(),
            QgsContrastEnhancement.ContrastEnhancementAlgorithm.StretchAndClipToMinimumMaximum,
        )
        self.assertEqual(rr.greenContrastEnhancement().minimumValue(), 130)
        self.assertEqual(rr.greenContrastEnhancement().maximumValue(), 150)
        self.assertEqual(
            rr.greenContrastEnhancement().contrastEnhancementAlgorithm(),
            QgsContrastEnhancement.ContrastEnhancementAlgorithm.StretchToMinimumMaximum,
        )
        self.assertEqual(rr.blueContrastEnhancement().minimumValue(), 170)
        self.assertEqual(rr.blueContrastEnhancement().maximumValue(), 190)
        self.assertEqual(
            rr.blueContrastEnhancement().contrastEnhancementAlgorithm(),
            QgsContrastEnhancement.ContrastEnhancementAlgorithm.ClipToMinimumMaximum,
        )

        doc = QDomDocument("testdoc")
        elem = renderer.save(doc, QgsReadWriteContext())

        r2 = QgsPointCloudRgbRenderer.create(elem, QgsReadWriteContext())
        self.assertEqual(r2.maximumScreenError(), 18)
        self.assertEqual(
            r2.maximumScreenErrorUnit(), QgsUnitTypes.RenderUnit.RenderInches
        )
        self.assertEqual(r2.pointSize(), 13)
        self.assertEqual(r2.pointSizeUnit(), QgsUnitTypes.RenderUnit.RenderPoints)
        self.assertEqual(r2.pointSizeMapUnitScale().minScale, 1000)
        self.assertEqual(r2.pointSizeMapUnitScale().maxScale, 2000)

        self.assertEqual(r2.blueAttribute(), "b")
        self.assertEqual(r2.greenAttribute(), "g")
        self.assertEqual(r2.redAttribute(), "r")
        self.assertEqual(r2.redContrastEnhancement().minimumValue(), 100)
        self.assertEqual(r2.redContrastEnhancement().maximumValue(), 120)
        self.assertEqual(
            r2.redContrastEnhancement().contrastEnhancementAlgorithm(),
            QgsContrastEnhancement.ContrastEnhancementAlgorithm.StretchAndClipToMinimumMaximum,
        )
        self.assertEqual(r2.greenContrastEnhancement().minimumValue(), 130)
        self.assertEqual(r2.greenContrastEnhancement().maximumValue(), 150)
        self.assertEqual(
            r2.greenContrastEnhancement().contrastEnhancementAlgorithm(),
            QgsContrastEnhancement.ContrastEnhancementAlgorithm.StretchToMinimumMaximum,
        )
        self.assertEqual(r2.blueContrastEnhancement().minimumValue(), 170)
        self.assertEqual(r2.blueContrastEnhancement().maximumValue(), 190)
        self.assertEqual(
            r2.blueContrastEnhancement().contrastEnhancementAlgorithm(),
            QgsContrastEnhancement.ContrastEnhancementAlgorithm.ClipToMinimumMaximum,
        )

    def testUsedAttributes(self):
        renderer = QgsPointCloudRgbRenderer()
        renderer.setBlueAttribute("b")
        renderer.setGreenAttribute("g")
        renderer.setRedAttribute("r")

        rc = QgsRenderContext()
        prc = QgsPointCloudRenderContext(rc, QgsVector3D(), QgsVector3D(), 1, 0)

        self.assertEqual(renderer.usedAttributes(prc), {"r", "g", "b"})

    @unittest.skipIf(
        "ept" not in QgsProviderRegistry.instance().providerList(),
        "EPT provider not available",
    )
    def testRender(self):
        layer = QgsPointCloudLayer(
            unitTestDataPath() + "/point_clouds/ept/rgb/ept.json", "test", "ept"
        )
        self.assertTrue(layer.isValid())

        layer.renderer().setPointSize(2)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(layer.crs())
        mapsettings.setExtent(QgsRectangle(497753.5, 7050887.5, 497754.6, 7050888.6))
        mapsettings.setLayers([layer])

        self.assertTrue(
            self.render_map_settings_check("rgb_render", "rgb_render", mapsettings)
        )

    @unittest.skipIf(
        "ept" not in QgsProviderRegistry.instance().providerList(),
        "EPT provider not available",
    )
    def testRenderCircles(self):
        layer = QgsPointCloudLayer(
            unitTestDataPath() + "/point_clouds/ept/rgb/ept.json", "test", "ept"
        )
        self.assertTrue(layer.isValid())

        layer.renderer().setPointSize(3)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        layer.renderer().setPointSymbol(QgsPointCloudRenderer.PointSymbol.Circle)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(layer.crs())
        mapsettings.setExtent(QgsRectangle(497753.5, 7050887.5, 497754.6, 7050888.6))
        mapsettings.setLayers([layer])

        self.assertTrue(
            self.render_map_settings_check(
                "rgb_circle_render", "rgb_circle_render", mapsettings
            )
        )

    @unittest.skipIf(
        "ept" not in QgsProviderRegistry.instance().providerList(),
        "EPT provider not available",
    )
    def testRenderCrsTransform(self):
        layer = QgsPointCloudLayer(
            unitTestDataPath() + "/point_clouds/ept/rgb/ept.json", "test", "ept"
        )
        self.assertTrue(layer.isValid())

        layer.renderer().setPointSize(2)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        mapsettings.setExtent(
            QgsRectangle(152.977434544, -26.663017454, 152.977424882, -26.663009624)
        )
        mapsettings.setLayers([layer])

        self.assertTrue(
            self.render_map_settings_check(
                "rgb_render_crs_transform", "rgb_render_crs_transform", mapsettings
            )
        )

    @unittest.skipIf(
        "ept" not in QgsProviderRegistry.instance().providerList(),
        "EPT provider not available",
    )
    def testRenderWithContrast(self):
        layer = QgsPointCloudLayer(
            unitTestDataPath() + "/point_clouds/ept/rgb/ept.json", "test", "ept"
        )
        self.assertTrue(layer.isValid())

        layer.renderer().setPointSize(2)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)

        redce = QgsContrastEnhancement()
        redce.setMinimumValue(100)
        redce.setMaximumValue(120)
        redce.setContrastEnhancementAlgorithm(
            QgsContrastEnhancement.ContrastEnhancementAlgorithm.StretchToMinimumMaximum
        )
        layer.renderer().setRedContrastEnhancement(redce)

        greence = QgsContrastEnhancement()
        greence.setMinimumValue(130)
        greence.setMaximumValue(150)
        greence.setContrastEnhancementAlgorithm(
            QgsContrastEnhancement.ContrastEnhancementAlgorithm.StretchToMinimumMaximum
        )
        layer.renderer().setGreenContrastEnhancement(greence)

        bluece = QgsContrastEnhancement()
        bluece.setMinimumValue(170)
        bluece.setMaximumValue(190)
        bluece.setContrastEnhancementAlgorithm(
            QgsContrastEnhancement.ContrastEnhancementAlgorithm.StretchToMinimumMaximum
        )
        layer.renderer().setBlueContrastEnhancement(bluece)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(layer.crs())
        mapsettings.setExtent(QgsRectangle(497753.5, 7050887.5, 497754.6, 7050888.6))
        mapsettings.setLayers([layer])

        self.assertTrue(
            self.render_map_settings_check("rgb_contrast", "rgb_contrast", mapsettings)
        )

    @unittest.skipIf(
        "ept" not in QgsProviderRegistry.instance().providerList(),
        "EPT provider not available",
    )
    def testRenderOpacity(self):
        layer = QgsPointCloudLayer(
            unitTestDataPath() + "/point_clouds/ept/rgb/ept.json", "test", "ept"
        )
        self.assertTrue(layer.isValid())

        layer.renderer().setPointSize(2)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)

        layer.setOpacity(0.5)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(layer.crs())
        mapsettings.setExtent(QgsRectangle(497753.5, 7050887.5, 497754.6, 7050888.6))
        mapsettings.setLayers([layer])

        self.assertTrue(
            self.render_map_settings_check("opacity", "opacity", mapsettings)
        )

    @unittest.skipIf(
        "ept" not in QgsProviderRegistry.instance().providerList(),
        "EPT provider not available",
    )
    def testRenderBlendMode(self):
        layer = QgsPointCloudLayer(
            unitTestDataPath() + "/point_clouds/ept/rgb/ept.json", "test", "ept"
        )
        self.assertTrue(layer.isValid())

        layer.renderer().setPointSize(2)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)

        layer.setBlendMode(QPainter.CompositionMode.CompositionMode_ColorBurn)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(layer.crs())
        mapsettings.setExtent(QgsRectangle(497753.5, 7050887.5, 497754.6, 7050888.6))
        mapsettings.setLayers([layer])

        self.assertTrue(
            self.render_map_settings_check("blendmode", "blendmode", mapsettings)
        )

    @unittest.skipIf(
        "ept" not in QgsProviderRegistry.instance().providerList(),
        "EPT provider not available",
    )
    def testRenderPointSize(self):
        layer = QgsPointCloudLayer(
            unitTestDataPath() + "/point_clouds/ept/rgb/ept.json", "test", "ept"
        )
        self.assertTrue(layer.isValid())

        layer.renderer().setPointSize(0.05)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(layer.crs())
        mapsettings.setExtent(QgsRectangle(497753.5, 7050887.5, 497754.6, 7050888.6))
        mapsettings.setLayers([layer])

        self.assertTrue(
            self.render_map_settings_check("pointsize", "pointsize", mapsettings)
        )

    @unittest.skipIf(
        "ept" not in QgsProviderRegistry.instance().providerList(),
        "EPT provider not available",
    )
    def testRenderZRange(self):
        layer = QgsPointCloudLayer(
            unitTestDataPath() + "/point_clouds/ept/rgb/ept.json", "test", "ept"
        )
        self.assertTrue(layer.isValid())

        layer.renderer().setPointSize(2)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(layer.crs())
        mapsettings.setExtent(QgsRectangle(497753.5, 7050887.5, 497754.6, 7050888.6))
        mapsettings.setLayers([layer])
        mapsettings.setZRange(QgsDoubleRange(1.1, 1.2))

        self.assertTrue(
            self.render_map_settings_check("zfilter", "zfilter", mapsettings)
        )

    @unittest.skipIf(
        "ept" not in QgsProviderRegistry.instance().providerList(),
        "EPT provider not available",
    )
    def testRenderClipRegion(self):
        layer = QgsPointCloudLayer(
            unitTestDataPath() + "/point_clouds/ept/rgb/ept.json", "test", "ept"
        )
        self.assertTrue(layer.isValid())

        layer.renderer().setPointSize(2)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        mapsettings.setExtent(
            QgsRectangle(152.977434544, -26.663017454, 152.977424882, -26.663009624)
        )
        mapsettings.setLayers([layer])

        region = QgsMapClippingRegion(
            QgsGeometry.fromWkt(
                "Polygon ((152.97742833685992991 -26.66301088198133584, 152.97742694456141521 -26.66301085776744983, 152.97742676295726483 -26.66301358182974468, 152.97742895431403554 -26.66301349708113833, 152.97742833685992991 -26.66301088198133584))"
            )
        )
        region.setFeatureClip(QgsMapClippingRegion.FeatureClippingType.ClipPainterOnly)
        region2 = QgsMapClippingRegion(
            QgsGeometry.fromWkt(
                "Polygon ((152.97743215054714483 -26.66301111201326535, 152.97742715037946937 -26.66301116044103736, 152.97742754990858316 -26.66301436878107367, 152.97743264693181686 -26.66301491359353193, 152.97743215054714483 -26.66301111201326535))"
            )
        )
        region2.setFeatureClip(
            QgsMapClippingRegion.FeatureClippingType.ClipToIntersection
        )
        mapsettings.addClippingRegion(region)
        mapsettings.addClippingRegion(region2)

        self.assertTrue(
            self.render_map_settings_check("clip_region", "clip_region", mapsettings)
        )

    @unittest.skipIf(
        "ept" not in QgsProviderRegistry.instance().providerList(),
        "EPT provider not available",
    )
    def testRenderOrderedTopToBottom(self):
        layer = QgsPointCloudLayer(
            unitTestDataPath() + "/point_clouds/ept/rgb/ept.json", "test", "ept"
        )
        self.assertTrue(layer.isValid())

        layer.renderer().setPointSize(6)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        layer.renderer().setDrawOrder2d(QgsPointCloudRenderer.DrawOrder.TopToBottom)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(layer.crs())
        mapsettings.setExtent(QgsRectangle(497753.5, 7050887.5, 497754.6, 7050888.6))
        mapsettings.setLayers([layer])

        self.assertTrue(
            self.render_map_settings_check(
                "rgb_top_to_bottom", "rgb_top_to_bottom", mapsettings
            )
        )

    @unittest.skipIf(
        "ept" not in QgsProviderRegistry.instance().providerList(),
        "EPT provider not available",
    )
    def testRenderOrderedBottomToTop(self):
        layer = QgsPointCloudLayer(
            unitTestDataPath() + "/point_clouds/ept/rgb/ept.json", "test", "ept"
        )
        self.assertTrue(layer.isValid())

        layer.renderer().setPointSize(6)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        layer.renderer().setDrawOrder2d(QgsPointCloudRenderer.DrawOrder.BottomToTop)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(layer.crs())
        mapsettings.setExtent(QgsRectangle(497753.5, 7050887.5, 497754.6, 7050888.6))
        mapsettings.setLayers([layer])

        self.assertTrue(
            self.render_map_settings_check(
                "rgb_bottom_to_top", "rgb_bottom_to_top", mapsettings
            )
        )

    @unittest.skipIf(
        "ept" not in QgsProviderRegistry.instance().providerList(),
        "EPT provider not available",
    )
    def testRenderTriangles(self):
        layer = QgsPointCloudLayer(
            unitTestDataPath() + "/point_clouds/ept/rgb/ept.json", "test", "ept"
        )
        self.assertTrue(layer.isValid())

        layer.renderer().setPointSize(6)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        layer.renderer().setRenderAsTriangles(True)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(layer.crs())
        mapsettings.setExtent(QgsRectangle(497753.5, 7050887.5, 497754.6, 7050888.6))
        mapsettings.setLayers([layer])

        self.assertTrue(
            self.render_map_settings_check(
                "rgb_triangles", "rgb_triangles", mapsettings
            )
        )

    @unittest.skipIf(
        "ept" not in QgsProviderRegistry.instance().providerList(),
        "EPT provider not available",
    )
    def testRenderTrianglesHorizontalFilter(self):
        layer = QgsPointCloudLayer(
            unitTestDataPath() + "/point_clouds/ept/rgb/ept.json", "test", "ept"
        )
        self.assertTrue(layer.isValid())

        layer.renderer().setPointSize(6)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        layer.renderer().setRenderAsTriangles(True)
        layer.renderer().setHorizontalTriangleFilter(True)
        layer.renderer().setHorizontalTriangleFilterThreshold(10.0)
        layer.renderer().setHorizontalTriangleFilterUnit(Qgis.RenderUnit.Millimeters)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(layer.crs())
        mapsettings.setExtent(QgsRectangle(497753.5, 7050887.5, 497754.6, 7050888.6))
        mapsettings.setLayers([layer])

        self.assertTrue(
            self.render_map_settings_check(
                "rgb_triangles_filter", "rgb_triangles_filter", mapsettings
            )
        )


if __name__ == "__main__":
    unittest.main()
