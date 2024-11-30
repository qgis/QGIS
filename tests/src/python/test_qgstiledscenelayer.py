"""QGIS Unit tests for QgsTiledSceneLayer

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "27/06/2023"
__copyright__ = "Copyright 2023, The QGIS Project"

from qgis.PyQt.QtGui import QPainter
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    Qgis,
    QgsTiledSceneLayer,
    QgsReadWriteContext,
    QgsLayerNotesUtils,
    QgsMapLayer,
    QgsTiledSceneDataProvider,
    QgsProviderRegistry,
)
from qgis.testing import start_app, unittest

start_app()


class TestQgsTiledSceneLayer(unittest.TestCase):

    def test_data_provider(self):
        """
        Test data provider creation
        """
        layer = QgsTiledSceneLayer(
            "/home/me/test/tileset.json", "my layer", "cesiumtiles"
        )
        self.assertEqual(layer.providerType(), "cesiumtiles")
        self.assertIsInstance(layer.dataProvider(), QgsTiledSceneDataProvider)
        self.assertEqual(layer.dataProvider().name(), "cesiumtiles")
        self.assertEqual(
            layer.dataProvider().dataSourceUri(), "/home/me/test/tileset.json"
        )

    def test_read_write_xml(self):
        """
        Test saving and restoring layer from xml
        """
        layer = QgsTiledSceneLayer("uri", "my layer", "cesiumtiles")
        self.assertEqual(layer.providerType(), "cesiumtiles")
        layer.setOpacity(0.25)
        layer.setBlendMode(QPainter.CompositionMode.CompositionMode_Darken)

        doc = QDomDocument("testdoc")
        elem = doc.createElement("maplayer")
        self.assertTrue(layer.writeXml(elem, doc, QgsReadWriteContext()))

        layer2 = QgsTiledSceneLayer("uri2", "my layer 2", "xtiled_meshx")
        layer2.readXml(elem, QgsReadWriteContext())
        self.assertEqual(layer2.providerType(), "cesiumtiles")
        self.assertEqual(layer2.opacity(), 0.25)
        self.assertEqual(
            layer2.blendMode(), QPainter.CompositionMode.CompositionMode_Darken
        )

    def test_clone(self):
        """
        Test cloning layers
        """
        layer = QgsTiledSceneLayer("uri", "my layer", "cesiumtiles")
        self.assertEqual(layer.providerType(), "cesiumtiles")
        layer.setOpacity(0.25)
        layer.setBlendMode(QPainter.CompositionMode.CompositionMode_Darken)

        layer2 = layer.clone()
        self.assertEqual(layer2.source(), "uri")
        self.assertEqual(layer2.providerType(), "cesiumtiles")
        self.assertEqual(layer2.opacity(), 0.25)
        self.assertEqual(
            layer2.blendMode(), QPainter.CompositionMode.CompositionMode_Darken
        )

    def test_read_write_symbology(self):
        """
        Test reading/writing symbology
        """
        layer = QgsTiledSceneLayer("uri", "my layer", "tiled_mesh")
        self.assertEqual(layer.providerType(), "tiled_mesh")
        layer.setOpacity(0.25)
        layer.setBlendMode(QPainter.CompositionMode.CompositionMode_Darken)

        doc = QDomDocument("testdoc")
        elem = doc.createElement("symbology")

        context = QgsReadWriteContext()
        error = ""
        self.assertTrue(layer.writeSymbology(elem, doc, error, context))

        layer2 = QgsTiledSceneLayer("uri2", "my layer 2", "tiled_mesh")
        layer2.readSymbology(elem, error, context)
        self.assertEqual(layer2.opacity(), 0.25)
        self.assertEqual(
            layer2.blendMode(), QPainter.CompositionMode.CompositionMode_Darken
        )

        layer = QgsTiledSceneLayer("uri", "my layer", "tiled_mesh")
        self.assertEqual(layer.providerType(), "tiled_mesh")
        layer.setOpacity(0.25)
        layer.setBlendMode(QPainter.CompositionMode.CompositionMode_Darken)
        layer.setMinimumScale(1000)
        layer.setMaximumScale(2000)
        layer.setScaleBasedVisibility(True)
        layer.setCustomProperty("prop", "value")
        QgsLayerNotesUtils.setLayerNotes(layer, "my notes")

        doc = QDomDocument("testdoc")
        elem = doc.createElement("symbology")

        context = QgsReadWriteContext()
        error = ""
        self.assertTrue(
            layer.writeSymbology(
                elem, doc, error, context, QgsMapLayer.StyleCategory.Rendering
            )
        )

        layer2 = QgsTiledSceneLayer("uri2", "my layer 2", "tiled_mesh")
        layer2.readSymbology(elem, error, context)
        # only rendering properties should be applied
        self.assertEqual(layer2.opacity(), 0.25)
        self.assertEqual(layer2.minimumScale(), 1000)
        self.assertEqual(layer2.maximumScale(), 2000)
        self.assertTrue(layer2.hasScaleBasedVisibility())

        # these should be unchanged
        self.assertEqual(
            layer2.blendMode(), QPainter.CompositionMode.CompositionMode_SourceOver
        )
        self.assertFalse(layer2.customProperty("prop"))
        self.assertFalse(QgsLayerNotesUtils.layerNotes(layer2))

        # try restoring layer notes when these were not writing originally
        layer2.readSymbology(elem, error, context, QgsMapLayer.StyleCategory.Notes)
        self.assertFalse(QgsLayerNotesUtils.layerNotes(layer2))

        # write layer notes and restore
        self.assertTrue(
            layer.writeSymbology(
                elem, doc, error, context, QgsMapLayer.StyleCategory.Notes
            )
        )
        layer2.readSymbology(elem, error, context, QgsMapLayer.StyleCategory.Notes)
        self.assertEqual(QgsLayerNotesUtils.layerNotes(layer2), "my notes")

    def test_cesium_provider_metadata(self):
        """
        Test cesium provider metadata methods
        """
        self.assertIn(
            "cesiumtiles",
            QgsProviderRegistry.instance().providersForLayerType(
                Qgis.LayerType.TiledScene
            ),
        )

        metadata = QgsProviderRegistry.instance().providerMetadata("cesiumtiles")
        self.assertIsNotNone(metadata)

        self.assertEqual(
            metadata.decodeUri("/home/me/test/tileset.json"),
            {"file-name": "tileset.json", "path": "/home/me/test/tileset.json"},
        )
        self.assertEqual(
            metadata.encodeUri(
                {"file-name": "tileset.json", "path": "/home/me/test/tileset.json"}
            ),
            "/home/me/test/tileset.json",
        )


if __name__ == "__main__":
    unittest.main()
