"""QGIS Unit tests for QgsPointCloudExtentRenderer

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "04/12/2020"
__copyright__ = "Copyright 2020, The QGIS Project"

from qgis.PyQt.QtCore import QDir, QSize, Qt
from qgis.PyQt.QtGui import QColor
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    QgsCoordinateReferenceSystem,
    QgsFillSymbol,
    QgsLayerTreeLayer,
    QgsMapSettings,
    QgsPointCloudExtentRenderer,
    QgsPointCloudLayer,
    QgsProviderRegistry,
    QgsReadWriteContext,
    QgsRectangle,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()


class TestQgsPointCloudExtentRenderer(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "pointcloudrenderer"

    def testBasic(self):
        renderer = QgsPointCloudExtentRenderer(
            QgsFillSymbol.createSimple({"color": "#ff00ff", "outline_color": "black"})
        )
        self.assertEqual(renderer.fillSymbol()[0].color().name(), "#ff00ff")

        renderer.setFillSymbol(
            QgsFillSymbol.createSimple({"color": "#00ffff", "outline_color": "black"})
        )
        self.assertEqual(renderer.fillSymbol()[0].color().name(), "#00ffff")

        rr = renderer.clone()
        self.assertEqual(rr.fillSymbol()[0].color().name(), "#00ffff")

        doc = QDomDocument("testdoc")
        elem = renderer.save(doc, QgsReadWriteContext())

        r2 = QgsPointCloudExtentRenderer.create(elem, QgsReadWriteContext())
        self.assertEqual(r2.fillSymbol()[0].color().name(), "#00ffff")

    def testLegend(self):
        renderer = QgsPointCloudExtentRenderer()
        renderer.setFillSymbol(
            QgsFillSymbol.createSimple({"color": "#00ffff", "outline_color": "black"})
        )

        layer = QgsPointCloudLayer(
            unitTestDataPath() + "/point_clouds/ept/sunshine-coast/ept.json",
            "test",
            "ept",
        )
        layer_tree_layer = QgsLayerTreeLayer(layer)
        nodes = renderer.createLegendNodes(layer_tree_layer)
        self.assertEqual(len(nodes), 1)
        self.assertEqual(nodes[0].data(Qt.ItemDataRole.DisplayRole), "test")
        self.assertTrue(nodes[0].isEmbeddedInParent())

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

        renderer = QgsPointCloudExtentRenderer()
        layer.setRenderer(renderer)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(layer.crs())
        mapsettings.setExtent(QgsRectangle(498061, 7050991, 498069, 7050999))
        mapsettings.setLayers([layer])

        self.assertTrue(
            self.render_map_settings_check(
                "extent_render", "extent_render", mapsettings
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

        renderer = QgsPointCloudExtentRenderer()
        renderer.fillSymbol().setColor(QColor(33, 140, 20))
        layer.setRenderer(renderer)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        mapsettings.setExtent(
            QgsRectangle(
                152.980508492, -26.662023491, 152.980586020, -26.662071137
            ).buffered(0.00001)
        )
        mapsettings.setLayers([layer])

        self.assertTrue(
            self.render_map_settings_check(
                "extent_render_crs_transform",
                "extent_render_crs_transform",
                mapsettings,
            )
        )


if __name__ == "__main__":
    unittest.main()
