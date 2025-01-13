"""QGIS Unit tests for QgsPointCloudClassifiedRenderer

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "09/11/2020"
__copyright__ = "Copyright 2020, The QGIS Project"

from qgis.PyQt.QtCore import QSize, Qt
from qgis.PyQt.QtGui import QColor
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    QgsCoordinateReferenceSystem,
    QgsDoubleRange,
    QgsLayerTreeLayer,
    QgsLayerTreeModelLegendNode,
    QgsMapSettings,
    QgsMapUnitScale,
    QgsPointCloudCategory,
    QgsPointCloudClassifiedRenderer,
    QgsPointCloudLayer,
    QgsPointCloudRenderContext,
    QgsPointCloudRenderer,
    QgsPointCloudRendererRegistry,
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


class TestQgsPointCloudClassifiedRenderer(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "pointcloudrenderer"

    def testBasic(self):
        renderer = QgsPointCloudClassifiedRenderer()
        renderer.setAttribute("attr")
        self.assertEqual(renderer.attribute(), "attr")

        renderer.setCategories(
            [
                QgsPointCloudCategory(3, QColor(255, 0, 0), "cat 3"),
                QgsPointCloudCategory(7, QColor(0, 255, 0), "cat 7"),
            ]
        )

        self.assertEqual(len(renderer.categories()), 2)
        self.assertEqual(renderer.categories()[0].label(), "cat 3")
        self.assertEqual(renderer.categories()[1].label(), "cat 7")

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

        self.assertEqual(rr.attribute(), "attr")
        self.assertEqual(len(rr.categories()), 2)
        self.assertEqual(rr.categories()[0].label(), "cat 3")
        self.assertEqual(rr.categories()[1].label(), "cat 7")

        doc = QDomDocument("testdoc")
        elem = renderer.save(doc, QgsReadWriteContext())

        r2 = QgsPointCloudClassifiedRenderer.create(elem, QgsReadWriteContext())
        self.assertEqual(r2.maximumScreenError(), 18)
        self.assertEqual(
            r2.maximumScreenErrorUnit(), QgsUnitTypes.RenderUnit.RenderInches
        )
        self.assertEqual(r2.pointSize(), 13)
        self.assertEqual(r2.pointSizeUnit(), QgsUnitTypes.RenderUnit.RenderPoints)
        self.assertEqual(r2.pointSizeMapUnitScale().minScale, 1000)
        self.assertEqual(r2.pointSizeMapUnitScale().maxScale, 2000)
        self.assertEqual(r2.attribute(), "attr")
        self.assertEqual(len(r2.categories()), 2)
        self.assertEqual(r2.categories()[0].label(), "cat 3")
        self.assertEqual(r2.categories()[1].label(), "cat 7")

    def testUsedAttributes(self):
        renderer = QgsPointCloudClassifiedRenderer()
        renderer.setAttribute("attr")

        rc = QgsRenderContext()
        prc = QgsPointCloudRenderContext(rc, QgsVector3D(), QgsVector3D(), 1, 0)

        self.assertEqual(renderer.usedAttributes(prc), {"attr"})

    def testLegend(self):
        renderer = QgsPointCloudClassifiedRenderer()
        renderer.setAttribute("Classification")
        renderer.setCategories(
            [
                QgsPointCloudCategory(3, QColor(255, 0, 0), "cat 3"),
                QgsPointCloudCategory(7, QColor(0, 255, 0), "cat 7"),
            ]
        )

        layer = QgsPointCloudLayer(
            unitTestDataPath() + "/point_clouds/ept/sunshine-coast/ept.json",
            "test",
            "ept",
        )
        layer_tree_layer = QgsLayerTreeLayer(layer)
        nodes = renderer.createLegendNodes(layer_tree_layer)
        self.assertEqual(len(nodes), 2)
        self.assertEqual(nodes[0].data(Qt.ItemDataRole.DisplayRole), "cat 3")
        self.assertEqual(
            nodes[0].data(QgsLayerTreeModelLegendNode.LegendNodeRoles.RuleKeyRole), "3"
        )
        self.assertEqual(nodes[1].data(Qt.ItemDataRole.DisplayRole), "cat 7")
        self.assertEqual(
            nodes[1].data(QgsLayerTreeModelLegendNode.LegendNodeRoles.RuleKeyRole), "7"
        )

    @unittest.skipIf(
        "ept" not in QgsProviderRegistry.instance().providerList(),
        "EPT provider not available",
    )
    def testRender(self):
        layer = QgsPointCloudLayer(
            unitTestDataPath() + "/point_clouds/ept/sunshine-coast/ept.json",
            "test",
            "ept",
        )
        self.assertTrue(layer.isValid())

        categories = QgsPointCloudRendererRegistry.classificationAttributeCategories(
            layer
        )
        renderer = QgsPointCloudClassifiedRenderer("Classification", categories)
        layer.setRenderer(renderer)

        layer.renderer().setPointSize(2)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(layer.crs())
        mapsettings.setExtent(QgsRectangle(498061, 7050991, 498069, 7050999))
        mapsettings.setLayers([layer])

        self.assertTrue(
            self.render_map_settings_check(
                "classified_render", "classified_render", mapsettings
            )
        )

    @unittest.skipIf(
        "ept" not in QgsProviderRegistry.instance().providerList(),
        "EPT provider not available",
    )
    def testRenderCrsTransform(self):
        layer = QgsPointCloudLayer(
            unitTestDataPath() + "/point_clouds/ept/sunshine-coast/ept.json",
            "test",
            "ept",
        )
        self.assertTrue(layer.isValid())

        categories = QgsPointCloudRendererRegistry.classificationAttributeCategories(
            layer
        )
        renderer = QgsPointCloudClassifiedRenderer("Classification", categories)
        layer.setRenderer(renderer)

        layer.renderer().setPointSize(2)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        mapsettings.setExtent(
            QgsRectangle(152.980508492, -26.662023491, 152.980586020, -26.662071137)
        )
        mapsettings.setLayers([layer])

        self.assertTrue(
            self.render_map_settings_check(
                "classified_render_crs_transform",
                "classified_render_crs_transform",
                mapsettings,
            )
        )

    @unittest.skipIf(
        "ept" not in QgsProviderRegistry.instance().providerList(),
        "EPT provider not available",
    )
    def testRenderPointSize(self):
        layer = QgsPointCloudLayer(
            unitTestDataPath() + "/point_clouds/ept/sunshine-coast/ept.json",
            "test",
            "ept",
        )
        self.assertTrue(layer.isValid())

        categories = QgsPointCloudRendererRegistry.classificationAttributeCategories(
            layer
        )
        renderer = QgsPointCloudClassifiedRenderer("Classification", categories)
        layer.setRenderer(renderer)

        layer.renderer().setPointSize(0.15)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(layer.crs())
        mapsettings.setExtent(QgsRectangle(498061, 7050991, 498069, 7050999))
        mapsettings.setLayers([layer])

        self.assertTrue(
            self.render_map_settings_check(
                "classified_pointsize", "classified_pointsize", mapsettings
            )
        )

    @unittest.skipIf(
        "ept" not in QgsProviderRegistry.instance().providerList(),
        "EPT provider not available",
    )
    def testRenderClassificationOverridePointSizes(self):
        layer = QgsPointCloudLayer(
            unitTestDataPath() + "/point_clouds/ept/sunshine-coast/ept.json",
            "test",
            "ept",
        )
        self.assertTrue(layer.isValid())

        categories = QgsPointCloudRendererRegistry.classificationAttributeCategories(
            layer
        )
        categories[0].setPointSize(1)
        categories[2].setPointSize(0.3)
        categories[3].setPointSize(0.5)

        renderer = QgsPointCloudClassifiedRenderer("Classification", categories)
        layer.setRenderer(renderer)

        layer.renderer().setPointSize(0.15)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(layer.crs())
        mapsettings.setExtent(QgsRectangle(498061, 7050991, 498069, 7050999))
        mapsettings.setLayers([layer])

        self.assertTrue(
            self.render_map_settings_check(
                "classified_override_pointsize",
                "classified_override_pointsize",
                mapsettings,
            )
        )

    @unittest.skipIf(
        "ept" not in QgsProviderRegistry.instance().providerList(),
        "EPT provider not available",
    )
    def testRenderZRange(self):
        layer = QgsPointCloudLayer(
            unitTestDataPath() + "/point_clouds/ept/sunshine-coast/ept.json",
            "test",
            "ept",
        )
        self.assertTrue(layer.isValid())

        categories = QgsPointCloudRendererRegistry.classificationAttributeCategories(
            layer
        )
        renderer = QgsPointCloudClassifiedRenderer("Classification", categories)
        layer.setRenderer(renderer)

        layer.renderer().setPointSize(2)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(layer.crs())
        mapsettings.setExtent(QgsRectangle(498061, 7050991, 498069, 7050999))
        mapsettings.setLayers([layer])
        mapsettings.setZRange(QgsDoubleRange(74.7, 75))

        self.assertTrue(
            self.render_map_settings_check(
                "classified_zfilter", "classified_zfilter", mapsettings
            )
        )

    @unittest.skipIf(
        "ept" not in QgsProviderRegistry.instance().providerList(),
        "EPT provider not available",
    )
    def testRenderOrderedTopToBottom(self):
        layer = QgsPointCloudLayer(
            unitTestDataPath() + "/point_clouds/ept/sunshine-coast/ept.json",
            "test",
            "ept",
        )
        self.assertTrue(layer.isValid())

        categories = QgsPointCloudRendererRegistry.classificationAttributeCategories(
            layer
        )
        renderer = QgsPointCloudClassifiedRenderer("Classification", categories)
        layer.setRenderer(renderer)

        layer.renderer().setPointSize(6)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        layer.renderer().setDrawOrder2d(QgsPointCloudRenderer.DrawOrder.TopToBottom)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(layer.crs())
        mapsettings.setExtent(QgsRectangle(498061, 7050991, 498069, 7050999))
        mapsettings.setLayers([layer])

        self.assertTrue(
            self.render_map_settings_check(
                "classified_top_to_bottom", "classified_top_to_bottom", mapsettings
            )
        )

    @unittest.skipIf(
        "ept" not in QgsProviderRegistry.instance().providerList(),
        "EPT provider not available",
    )
    def testRenderOrderedBottomToTop(self):
        layer = QgsPointCloudLayer(
            unitTestDataPath() + "/point_clouds/ept/sunshine-coast/ept.json",
            "test",
            "ept",
        )
        self.assertTrue(layer.isValid())

        categories = QgsPointCloudRendererRegistry.classificationAttributeCategories(
            layer
        )
        renderer = QgsPointCloudClassifiedRenderer("Classification", categories)
        layer.setRenderer(renderer)

        layer.renderer().setPointSize(6)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        layer.renderer().setDrawOrder2d(QgsPointCloudRenderer.DrawOrder.BottomToTop)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(layer.crs())
        mapsettings.setExtent(QgsRectangle(498061, 7050991, 498069, 7050999))
        mapsettings.setLayers([layer])

        self.assertTrue(
            self.render_map_settings_check(
                "classified_bottom_to_top", "classified_bottom_to_top", mapsettings
            )
        )

    @unittest.skipIf(
        "ept" not in QgsProviderRegistry.instance().providerList(),
        "EPT provider not available",
    )
    def testRenderFiltered(self):
        layer = QgsPointCloudLayer(
            unitTestDataPath() + "/point_clouds/ept/sunshine-coast/ept.json",
            "test",
            "ept",
        )
        self.assertTrue(layer.isValid())

        categories = QgsPointCloudRendererRegistry.classificationAttributeCategories(
            layer
        )
        renderer = QgsPointCloudClassifiedRenderer("Classification", categories)
        layer.setRenderer(renderer)

        layer.renderer().setPointSize(2)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        layer.setSubsetString("NumberOfReturns > 1")

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(layer.crs())
        mapsettings.setExtent(QgsRectangle(498061, 7050991, 498069, 7050999))
        mapsettings.setLayers([layer])

        self.assertTrue(
            self.render_map_settings_check(
                "classified_render_filtered", "classified_render_filtered", mapsettings
            )
        )

        layer.setSubsetString("")

        self.assertTrue(
            self.render_map_settings_check(
                "classified_render_unfiltered",
                "classified_render_unfiltered",
                mapsettings,
            )
        )

    @unittest.skipIf(
        "ept" not in QgsProviderRegistry.instance().providerList(),
        "EPT provider not available",
    )
    def testRenderTriangles(self):
        layer = QgsPointCloudLayer(
            unitTestDataPath() + "/point_clouds/ept/sunshine-coast/ept.json",
            "test",
            "ept",
        )
        self.assertTrue(layer.isValid())

        categories = QgsPointCloudRendererRegistry.classificationAttributeCategories(
            layer
        )
        renderer = QgsPointCloudClassifiedRenderer("Classification", categories)
        renderer.setRenderAsTriangles(True)
        layer.setRenderer(renderer)

        layer.renderer().setPointSize(2)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(layer.crs())
        mapsettings.setExtent(QgsRectangle(498061, 7050991, 498069, 7050999))
        mapsettings.setLayers([layer])

        self.assertTrue(
            self.render_map_settings_check(
                "classified_triangles", "classified_triangles", mapsettings
            )
        )

    @unittest.skipIf(
        "vpc" not in QgsProviderRegistry.instance().providerList(),
        "VPC provider not available",
    )
    def testOverviewRender(self):
        layer = QgsPointCloudLayer(
            unitTestDataPath()
            + "/point_clouds/virtual/sunshine-coast/combined-with-overview.vpc",
            "test",
            "vpc",
        )
        self.assertTrue(layer.isValid())

        layer.setRenderer(layer.dataProvider().createRenderer())

        layer.renderer().setPointSize(2)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(layer.crs())
        mapsettings.setExtent(QgsRectangle(498061, 7050991, 498069, 7050999))
        mapsettings.setLayers([layer])

        self.assertTrue(
            self.render_map_settings_check(
                "classified_render_overview", "classified_render_overview", mapsettings
            )
        )

    @unittest.skipIf(
        "vpc" not in QgsProviderRegistry.instance().providerList(),
        "VPC provider not available",
    )
    def testExtentsRender(self):
        layer = QgsPointCloudLayer(
            unitTestDataPath() + "/point_clouds/virtual/sunshine-coast/combined.vpc",
            "test",
            "vpc",
        )
        self.assertTrue(layer.isValid())

        layer.setRenderer(layer.dataProvider().createRenderer())

        layer.renderer().setPointSize(2)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(layer.crs())
        mapsettings.setExtent(QgsRectangle(498061, 7050991, 498069, 7050999))
        mapsettings.setLayers([layer])

        self.assertTrue(
            self.render_map_settings_check(
                "classified_render_extents", "classified_render_extents", mapsettings
            )
        )


if __name__ == "__main__":
    unittest.main()
