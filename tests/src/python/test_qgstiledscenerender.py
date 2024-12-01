"""QGIS Unit tests for QgsCesium3dTilesLayer

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "27/06/2023"
__copyright__ = "Copyright 2023, The QGIS Project"

from qgis.PyQt.QtCore import QSize
from qgis.core import (
    QgsRectangle,
    QgsTiledSceneLayer,
    QgsTiledSceneTextureRenderer,
    QgsTiledSceneWireframeRenderer,
    QgsCoordinateReferenceSystem,
    QgsMapSettings,
)
from qgis.testing import start_app, QgisTestCase, unittest

from utilities import unitTestDataPath

start_app()


class TestQgsTiledSceneRender(QgisTestCase):
    @classmethod
    def control_path_prefix(cls):
        return "tiled_scene"

    def test_render_texture(self):
        layer = QgsTiledSceneLayer(
            unitTestDataPath() + "/tiled_scene/tileset.json", "my layer", "cesiumtiles"
        )
        self.assertTrue(layer.dataProvider().isValid())

        renderer = QgsTiledSceneTextureRenderer()
        layer.setRenderer(renderer)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
        mapsettings.setExtent(
            QgsRectangle(1023034.13, 5694847.48, 1023071.46, 5694876.59)
        )
        mapsettings.setLayers([layer])

        self.assertTrue(
            self.render_map_settings_check(
                "texture_render", "texture_render", mapsettings
            )
        )

    def test_render_wireframe(self):
        layer = QgsTiledSceneLayer(
            unitTestDataPath() + "/tiled_scene/tileset.json", "my layer", "cesiumtiles"
        )
        self.assertTrue(layer.dataProvider().isValid())

        renderer = QgsTiledSceneWireframeRenderer()
        layer.setRenderer(renderer)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
        mapsettings.setExtent(
            QgsRectangle(1023050.604, 5694861.052, 1023055.354, 5694864.755)
        )
        mapsettings.setLayers([layer])

        self.assertTrue(
            self.render_map_settings_check(
                "wireframe_render", "wireframe_render", mapsettings
            )
        )

    def test_render_wireframe_color(self):
        layer = QgsTiledSceneLayer(
            unitTestDataPath() + "/tiled_scene/tileset.json", "my layer", "cesiumtiles"
        )
        self.assertTrue(layer.dataProvider().isValid())

        renderer = QgsTiledSceneWireframeRenderer()
        renderer.setUseTextureColors(True)
        layer.setRenderer(renderer)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
        mapsettings.setExtent(
            QgsRectangle(1023050.604, 5694861.052, 1023055.354, 5694864.755)
        )
        mapsettings.setLayers([layer])

        self.assertTrue(
            self.render_map_settings_check(
                "wireframe_render_color", "wireframe_render_color", mapsettings
            )
        )


if __name__ == "__main__":
    unittest.main()
