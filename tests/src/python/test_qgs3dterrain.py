"""QGIS Unit tests for 3D terrrain objects.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from qgis.PyQt.QtXml import (
    QDomDocument,
)
from qgis.core import (
    QgsProject,
    QgsRasterLayer,
    QgsMeshLayer,
    QgsTiledSceneLayer,
    QgsReadWriteContext,
)
from qgis._3d import (
    QgsFlatTerrainSettings,
    QgsDemTerrainSettings,
    QgsOnlineDemTerrainSettings,
    QgsMeshTerrainSettings,
    Qgs3DTerrainRegistry,
    QgsQuantizedMeshTerrainSettings,
    QgsMesh3DSymbol,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgs3DTerrain(QgisTestCase):

    def test_flat_terrain(self):
        settings = QgsFlatTerrainSettings.create()
        self.assertIsInstance(settings, QgsFlatTerrainSettings)

        self.assertEqual(settings.verticalScale(), 1)
        settings.setVerticalScale(3)
        self.assertEqual(settings.verticalScale(), 3)

        self.assertEqual(settings.mapTileResolution(), 512)
        settings.setMapTileResolution(36)
        self.assertEqual(settings.mapTileResolution(), 36)

        self.assertEqual(settings.maximumScreenError(), 3.0)
        settings.setMaximumScreenError(4.5)
        self.assertEqual(settings.maximumScreenError(), 4.5)

        self.assertEqual(settings.maximumGroundError(), 1)
        settings.setMaximumGroundError(14.5)
        self.assertEqual(settings.maximumGroundError(), 14.5)

        self.assertEqual(settings.elevationOffset(), 0)
        settings.setElevationOffset(24.5)
        self.assertEqual(settings.elevationOffset(), 24.5)

        # clone
        settings2 = settings.clone()
        self.assertEqual(settings2.verticalScale(), 3)
        self.assertEqual(settings2.mapTileResolution(), 36)
        self.assertEqual(settings2.maximumScreenError(), 4.5)
        self.assertEqual(settings2.maximumGroundError(), 14.5)
        self.assertEqual(settings2.elevationOffset(), 24.5)
        self.assertTrue(settings2.equals(settings))

        # equals
        settings2.setVerticalScale(4)
        self.assertFalse(settings2.equals(settings))
        settings2 = settings.clone()
        settings2.setMapTileResolution(136)
        self.assertFalse(settings2.equals(settings))
        settings2 = settings.clone()
        settings2.setMaximumScreenError(136)
        self.assertFalse(settings2.equals(settings))
        settings2 = settings.clone()
        settings2.setMaximumGroundError(136)
        self.assertFalse(settings2.equals(settings))
        settings2 = settings.clone()
        settings2.setElevationOffset(136)
        self.assertFalse(settings2.equals(settings))

        # read/write xml
        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")
        settings.writeXml(elem, QgsReadWriteContext())

        settings3 = QgsFlatTerrainSettings()
        settings3.readXml(elem, QgsReadWriteContext())
        self.assertEqual(settings3.verticalScale(), 3)
        self.assertEqual(settings3.mapTileResolution(), 36)
        self.assertEqual(settings3.maximumScreenError(), 4.5)
        self.assertEqual(settings3.maximumGroundError(), 14.5)
        self.assertEqual(settings3.elevationOffset(), 24.5)

    def test_dem_terrain(self):
        p = QgsProject()
        rl = QgsRasterLayer(
            self.get_test_data_path("raster/dem.tif").as_posix(), "layer1"
        )
        self.assertTrue(rl.isValid())
        rl2 = QgsRasterLayer(
            self.get_test_data_path("raster/dem.tif").as_posix(), "layer2"
        )
        self.assertTrue(rl2.isValid())
        p.addMapLayer(rl)
        p.addMapLayer(rl2)

        settings = QgsDemTerrainSettings.create()
        self.assertIsInstance(settings, QgsDemTerrainSettings)

        settings.setLayer(rl)
        self.assertEqual(settings.layer(), rl)

        self.assertEqual(settings.resolution(), 16)
        settings.setResolution(66)
        self.assertEqual(settings.resolution(), 66)

        self.assertEqual(settings.skirtHeight(), 10)
        settings.setSkirtHeight(366)
        self.assertEqual(settings.skirtHeight(), 366)

        self.assertEqual(settings.verticalScale(), 1)
        settings.setVerticalScale(3)
        self.assertEqual(settings.verticalScale(), 3)

        self.assertEqual(settings.mapTileResolution(), 512)
        settings.setMapTileResolution(36)
        self.assertEqual(settings.mapTileResolution(), 36)

        self.assertEqual(settings.maximumScreenError(), 3.0)
        settings.setMaximumScreenError(4.5)
        self.assertEqual(settings.maximumScreenError(), 4.5)

        self.assertEqual(settings.maximumGroundError(), 1)
        settings.setMaximumGroundError(14.5)
        self.assertEqual(settings.maximumGroundError(), 14.5)

        self.assertEqual(settings.elevationOffset(), 0)
        settings.setElevationOffset(24.5)
        self.assertEqual(settings.elevationOffset(), 24.5)

        # clone
        settings2 = settings.clone()
        self.assertEqual(settings2.layer(), rl)
        self.assertEqual(settings2.resolution(), 66)
        self.assertEqual(settings2.skirtHeight(), 366)
        self.assertEqual(settings2.verticalScale(), 3)
        self.assertEqual(settings2.mapTileResolution(), 36)
        self.assertEqual(settings2.maximumScreenError(), 4.5)
        self.assertEqual(settings2.maximumGroundError(), 14.5)
        self.assertEqual(settings2.elevationOffset(), 24.5)
        self.assertTrue(settings2.equals(settings))

        # equals
        settings2.setLayer(rl2)
        self.assertFalse(settings2.equals(settings))
        settings2 = settings.clone()
        settings2.setResolution(555)
        self.assertFalse(settings2.equals(settings))
        settings2 = settings.clone()
        settings2.setSkirtHeight(1555)
        self.assertFalse(settings2.equals(settings))
        settings2 = settings.clone()
        settings2.setVerticalScale(4)
        self.assertFalse(settings2.equals(settings))
        settings2 = settings.clone()
        settings2.setMapTileResolution(136)
        self.assertFalse(settings2.equals(settings))
        settings2 = settings.clone()
        settings2.setMaximumScreenError(136)
        self.assertFalse(settings2.equals(settings))
        settings2 = settings.clone()
        settings2.setMaximumGroundError(136)
        self.assertFalse(settings2.equals(settings))
        settings2 = settings.clone()
        settings2.setElevationOffset(136)
        self.assertFalse(settings2.equals(settings))

        # read/write xml
        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")
        settings.writeXml(elem, QgsReadWriteContext())

        settings3 = QgsDemTerrainSettings()
        settings3.readXml(elem, QgsReadWriteContext())
        self.assertIsNone(settings3.layer())
        settings3.resolveReferences(p)
        self.assertEqual(settings3.layer(), rl)
        self.assertEqual(settings3.resolution(), 66)
        self.assertEqual(settings3.skirtHeight(), 366)
        self.assertEqual(settings3.verticalScale(), 3)
        self.assertEqual(settings3.mapTileResolution(), 36)
        self.assertEqual(settings3.maximumScreenError(), 4.5)
        self.assertEqual(settings3.maximumGroundError(), 14.5)
        self.assertEqual(settings3.elevationOffset(), 24.5)

    def test_online_dem_terrain(self):
        settings = QgsOnlineDemTerrainSettings.create()
        self.assertIsInstance(settings, QgsOnlineDemTerrainSettings)

        self.assertEqual(settings.resolution(), 16)
        settings.setResolution(66)
        self.assertEqual(settings.resolution(), 66)

        self.assertEqual(settings.skirtHeight(), 10)
        settings.setSkirtHeight(366)
        self.assertEqual(settings.skirtHeight(), 366)

        self.assertEqual(settings.verticalScale(), 1)
        settings.setVerticalScale(3)
        self.assertEqual(settings.verticalScale(), 3)

        self.assertEqual(settings.mapTileResolution(), 512)
        settings.setMapTileResolution(36)
        self.assertEqual(settings.mapTileResolution(), 36)

        self.assertEqual(settings.maximumScreenError(), 3.0)
        settings.setMaximumScreenError(4.5)
        self.assertEqual(settings.maximumScreenError(), 4.5)

        self.assertEqual(settings.maximumGroundError(), 1)
        settings.setMaximumGroundError(14.5)
        self.assertEqual(settings.maximumGroundError(), 14.5)

        self.assertEqual(settings.elevationOffset(), 0)
        settings.setElevationOffset(24.5)
        self.assertEqual(settings.elevationOffset(), 24.5)

        # clone
        settings2 = settings.clone()
        self.assertEqual(settings2.resolution(), 66)
        self.assertEqual(settings2.skirtHeight(), 366)
        self.assertEqual(settings2.verticalScale(), 3)
        self.assertEqual(settings2.mapTileResolution(), 36)
        self.assertEqual(settings2.maximumScreenError(), 4.5)
        self.assertEqual(settings2.maximumGroundError(), 14.5)
        self.assertEqual(settings2.elevationOffset(), 24.5)
        self.assertTrue(settings2.equals(settings))

        # equals
        settings2 = settings.clone()
        settings2.setResolution(555)
        self.assertFalse(settings2.equals(settings))
        settings2 = settings.clone()
        settings2.setSkirtHeight(1555)
        self.assertFalse(settings2.equals(settings))
        settings2 = settings.clone()
        settings2.setVerticalScale(4)
        self.assertFalse(settings2.equals(settings))
        settings2 = settings.clone()
        settings2.setMapTileResolution(136)
        self.assertFalse(settings2.equals(settings))
        settings2 = settings.clone()
        settings2.setMaximumScreenError(136)
        self.assertFalse(settings2.equals(settings))
        settings2 = settings.clone()
        settings2.setMaximumGroundError(136)
        self.assertFalse(settings2.equals(settings))
        settings2 = settings.clone()
        settings2.setElevationOffset(136)
        self.assertFalse(settings2.equals(settings))

        # read/write xml
        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")
        settings.writeXml(elem, QgsReadWriteContext())

        settings3 = QgsOnlineDemTerrainSettings()
        settings3.readXml(elem, QgsReadWriteContext())
        self.assertEqual(settings3.resolution(), 66)
        self.assertEqual(settings3.skirtHeight(), 366)
        self.assertEqual(settings3.verticalScale(), 3)
        self.assertEqual(settings3.mapTileResolution(), 36)
        self.assertEqual(settings3.maximumScreenError(), 4.5)
        self.assertEqual(settings3.maximumGroundError(), 14.5)
        self.assertEqual(settings3.elevationOffset(), 24.5)

    def test_mesh_terrain(self):
        p = QgsProject()
        ml = QgsMeshLayer(
            self.get_test_data_path("mesh/quad_and_triangle.2dm").as_posix(), "layer1"
        )
        self.assertTrue(ml.isValid())
        ml2 = QgsMeshLayer(
            self.get_test_data_path("mesh/quad_and_triangle.2dm").as_posix(), "layer2"
        )
        self.assertTrue(ml2.isValid())
        p.addMapLayer(ml)
        p.addMapLayer(ml2)

        settings = QgsMeshTerrainSettings.create()
        self.assertIsInstance(settings, QgsMeshTerrainSettings)

        settings.setLayer(ml)
        self.assertEqual(settings.layer(), ml)

        symbol = QgsMesh3DSymbol()
        symbol.setWireframeLineWidth(11.2)
        settings.setSymbol(symbol.clone())
        self.assertEqual(settings.symbol().wireframeLineWidth(), 11.2)

        self.assertEqual(settings.verticalScale(), 1)
        settings.setVerticalScale(3)
        self.assertEqual(settings.verticalScale(), 3)

        self.assertEqual(settings.mapTileResolution(), 512)
        settings.setMapTileResolution(36)
        self.assertEqual(settings.mapTileResolution(), 36)

        self.assertEqual(settings.maximumScreenError(), 3.0)
        settings.setMaximumScreenError(4.5)
        self.assertEqual(settings.maximumScreenError(), 4.5)

        self.assertEqual(settings.maximumGroundError(), 1)
        settings.setMaximumGroundError(14.5)
        self.assertEqual(settings.maximumGroundError(), 14.5)

        self.assertEqual(settings.elevationOffset(), 0)
        settings.setElevationOffset(24.5)
        self.assertEqual(settings.elevationOffset(), 24.5)

        # clone
        settings2 = settings.clone()
        self.assertEqual(settings2.layer(), ml)
        self.assertEqual(settings2.verticalScale(), 3)
        self.assertEqual(settings2.mapTileResolution(), 36)
        self.assertEqual(settings2.maximumScreenError(), 4.5)
        self.assertEqual(settings2.maximumGroundError(), 14.5)
        self.assertEqual(settings2.elevationOffset(), 24.5)
        self.assertEqual(settings2.symbol().wireframeLineWidth(), 11.2)
        self.assertTrue(settings2.equals(settings))

        # equals
        settings2.setLayer(ml2)
        self.assertFalse(settings2.equals(settings))
        settings2 = settings.clone()
        settings2.setVerticalScale(4)
        self.assertFalse(settings2.equals(settings))
        settings2 = settings.clone()
        settings2.setMapTileResolution(136)
        self.assertFalse(settings2.equals(settings))
        settings2 = settings.clone()
        settings2.setMaximumScreenError(136)
        self.assertFalse(settings2.equals(settings))
        settings2 = settings.clone()
        settings2.setMaximumGroundError(136)
        self.assertFalse(settings2.equals(settings))
        settings2 = settings.clone()
        settings2.setElevationOffset(136)
        self.assertFalse(settings2.equals(settings))

        settings2 = settings.clone()
        symbol = QgsMesh3DSymbol()
        symbol.setWireframeLineWidth(111.2)
        settings2.setSymbol(symbol.clone())
        self.assertFalse(settings2.equals(settings))

        # read/write xml
        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")
        settings.writeXml(elem, QgsReadWriteContext())

        settings3 = QgsMeshTerrainSettings()
        settings3.readXml(elem, QgsReadWriteContext())
        self.assertIsNone(settings3.layer())
        settings3.resolveReferences(p)
        self.assertEqual(settings3.layer(), ml)
        self.assertEqual(settings3.verticalScale(), 3)
        self.assertEqual(settings3.mapTileResolution(), 36)
        self.assertEqual(settings3.maximumScreenError(), 4.5)
        self.assertEqual(settings3.maximumGroundError(), 14.5)
        self.assertEqual(settings3.elevationOffset(), 24.5)
        self.assertEqual(settings3.symbol().wireframeLineWidth(), 11.2)

    def test_quantized_mesh_terrain(self):
        p = QgsProject()
        ml = QgsTiledSceneLayer("not valid", "layer1")
        ml2 = QgsTiledSceneLayer("not valid", "layer2")
        p.addMapLayer(ml)
        p.addMapLayer(ml2)

        settings = QgsQuantizedMeshTerrainSettings.create()
        self.assertIsInstance(settings, QgsQuantizedMeshTerrainSettings)

        settings.setLayer(ml)
        self.assertEqual(settings.layer(), ml)

        self.assertEqual(settings.verticalScale(), 1)
        settings.setVerticalScale(3)
        self.assertEqual(settings.verticalScale(), 3)

        self.assertEqual(settings.mapTileResolution(), 512)
        settings.setMapTileResolution(36)
        self.assertEqual(settings.mapTileResolution(), 36)

        self.assertEqual(settings.maximumScreenError(), 3.0)
        settings.setMaximumScreenError(4.5)
        self.assertEqual(settings.maximumScreenError(), 4.5)

        self.assertEqual(settings.maximumGroundError(), 1)
        settings.setMaximumGroundError(14.5)
        self.assertEqual(settings.maximumGroundError(), 14.5)

        self.assertEqual(settings.elevationOffset(), 0)
        settings.setElevationOffset(24.5)
        self.assertEqual(settings.elevationOffset(), 24.5)

        # clone
        settings2 = settings.clone()
        self.assertEqual(settings2.layer(), ml)
        self.assertEqual(settings2.verticalScale(), 3)
        self.assertEqual(settings2.mapTileResolution(), 36)
        self.assertEqual(settings2.maximumScreenError(), 4.5)
        self.assertEqual(settings2.maximumGroundError(), 14.5)
        self.assertEqual(settings2.elevationOffset(), 24.5)
        self.assertTrue(settings2.equals(settings))

        # equals
        settings2.setLayer(ml2)
        self.assertFalse(settings2.equals(settings))
        settings2 = settings.clone()
        settings2.setVerticalScale(4)
        self.assertFalse(settings2.equals(settings))
        settings2 = settings.clone()
        settings2.setMapTileResolution(136)
        self.assertFalse(settings2.equals(settings))
        settings2 = settings.clone()
        settings2.setMaximumScreenError(136)
        self.assertFalse(settings2.equals(settings))
        settings2 = settings.clone()
        settings2.setMaximumGroundError(136)
        self.assertFalse(settings2.equals(settings))
        settings2 = settings.clone()
        settings2.setElevationOffset(136)
        self.assertFalse(settings2.equals(settings))

        # read/write xml
        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")
        settings.writeXml(elem, QgsReadWriteContext())

        settings3 = QgsQuantizedMeshTerrainSettings()
        settings3.readXml(elem, QgsReadWriteContext())
        self.assertIsNone(settings3.layer())
        settings3.resolveReferences(p)
        self.assertEqual(settings3.layer(), ml)
        self.assertEqual(settings3.verticalScale(), 3)
        self.assertEqual(settings3.mapTileResolution(), 36)
        self.assertEqual(settings3.maximumScreenError(), 4.5)
        self.assertEqual(settings3.maximumGroundError(), 14.5)
        self.assertEqual(settings3.elevationOffset(), 24.5)

    def test_registry(self):
        registry = Qgs3DTerrainRegistry()
        # registry should be populated with known types
        self.assertIn("flat", registry.types())
        self.assertIn("dem", registry.types())
        self.assertIn("mesh", registry.types())
        self.assertIn("quantizedmesh", registry.types())
        self.assertIn("online", registry.types())

        # check settings
        settings = registry.createTerrainSettings("flat")
        self.assertIsInstance(settings, QgsFlatTerrainSettings)
        settings = registry.createTerrainSettings("dem")
        self.assertIsInstance(settings, QgsDemTerrainSettings)
        settings = registry.createTerrainSettings("mesh")
        self.assertIsInstance(settings, QgsMeshTerrainSettings)
        settings = registry.createTerrainSettings("quantizedmesh")
        self.assertIsInstance(settings, QgsQuantizedMeshTerrainSettings)
        settings = registry.createTerrainSettings("online")
        self.assertIsInstance(settings, QgsOnlineDemTerrainSettings)


if __name__ == "__main__":
    unittest.main()
