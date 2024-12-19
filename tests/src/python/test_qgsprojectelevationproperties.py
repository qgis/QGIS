"""QGIS Unit tests for QgsProjectElevationProperties

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '09/11/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import os

from qgis.PyQt.QtCore import QTemporaryDir
from qgis.PyQt.QtXml import QDomDocument
from qgis.PyQt.QtTest import QSignalSpy
from qgis.core import (
    QgsFlatTerrainProvider,
    QgsMeshTerrainProvider,
    QgsProject,
    QgsProjectElevationProperties,
    QgsRasterDemTerrainProvider,
    QgsRasterLayer,
    QgsReadWriteContext,
    QgsDoubleRange,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()


class TestQgsProjectElevationProperties(QgisTestCase):

    def testBasic(self):
        props = QgsProjectElevationProperties(None)
        self.assertIsInstance(props.terrainProvider(), QgsFlatTerrainProvider)
        self.assertTrue(props.elevationRange().isInfinite())

        provider = QgsRasterDemTerrainProvider()
        provider.setOffset(5)
        provider.setScale(3)
        props.setTerrainProvider(provider)

        self.assertIsInstance(props.terrainProvider(), QgsRasterDemTerrainProvider)

        range_changed_spy = QSignalSpy(props.elevationRangeChanged)
        props.setElevationRange(QgsDoubleRange(34.2, 78.6))
        self.assertEqual(props.elevationRange(), QgsDoubleRange(34.2, 78.6))
        self.assertEqual(len(range_changed_spy), 1)
        self.assertEqual(range_changed_spy[-1][0], QgsDoubleRange(34.2, 78.6))

        # no signal if not changed
        props.setElevationRange(QgsDoubleRange(34.2, 78.6))
        self.assertEqual(len(range_changed_spy), 1)

        doc = QDomDocument("testdoc")
        elem = props.writeXml(doc, QgsReadWriteContext())

        props2 = QgsProjectElevationProperties(None)
        props2.readXml(elem, QgsReadWriteContext())
        self.assertIsInstance(props2.terrainProvider(), QgsRasterDemTerrainProvider)
        self.assertEqual(props2.terrainProvider().offset(), 5)
        self.assertEqual(props2.terrainProvider().scale(), 3)
        self.assertEqual(props2.elevationRange(), QgsDoubleRange(34.2, 78.6))

        mesh_provider = QgsMeshTerrainProvider()
        mesh_provider.setOffset(2)
        mesh_provider.setScale(4)
        props.setTerrainProvider(mesh_provider)
        self.assertIsInstance(props.terrainProvider(), QgsMeshTerrainProvider)

        elem = props.writeXml(doc, QgsReadWriteContext())

        props2 = QgsProjectElevationProperties(None)
        props2.readXml(elem, QgsReadWriteContext())
        self.assertIsInstance(props2.terrainProvider(), QgsMeshTerrainProvider)
        self.assertEqual(props2.terrainProvider().offset(), 2)
        self.assertEqual(props2.terrainProvider().scale(), 4)

        flat_provider = QgsFlatTerrainProvider()
        flat_provider.setOffset(12)
        props.setTerrainProvider(flat_provider)
        self.assertIsInstance(props.terrainProvider(), QgsFlatTerrainProvider)

        elem = props.writeXml(doc, QgsReadWriteContext())

        props2 = QgsProjectElevationProperties(None)
        props2.readXml(elem, QgsReadWriteContext())
        self.assertIsInstance(props2.terrainProvider(), QgsFlatTerrainProvider)
        self.assertEqual(props2.terrainProvider().offset(), 12)

    def test_layer_resolving(self):
        provider = QgsRasterDemTerrainProvider()

        # add raster layer to a project
        p = QgsProject()
        rl = QgsRasterLayer(os.path.join(unitTestDataPath(), 'float1-16.tif'), 'rl')
        self.assertTrue(rl.isValid())
        p.addMapLayer(rl)

        provider.setLayer(rl)
        p.elevationProperties().setTerrainProvider(provider)

        tmp_dir = QTemporaryDir()
        tmp_project_file = f"{tmp_dir.path()}/project.qgs"
        self.assertTrue(p.write(tmp_project_file))

        project2 = QgsProject()
        self.assertTrue(project2.read(tmp_project_file))

        self.assertIsInstance(project2.elevationProperties().terrainProvider(), QgsRasterDemTerrainProvider)
        # make sure layer is resolved
        self.assertEqual(project2.elevationProperties().terrainProvider().layer().id(), rl.id())


if __name__ == '__main__':
    unittest.main()
