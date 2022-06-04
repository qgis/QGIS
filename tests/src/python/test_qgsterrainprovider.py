# -*- coding: utf-8 -*-
"""QGIS Unit tests for terrain providers

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '17/03/2022'
__copyright__ = 'Copyright 2022, The QGIS Project'

import os
import math

import qgis  # NOQA

from qgis.PyQt.QtXml import QDomDocument

from qgis.core import (
    QgsFlatTerrainProvider,
    QgsRasterDemTerrainProvider,
    QgsMeshTerrainProvider,
    QgsProject,
    QgsRasterLayer,
    QgsMeshLayer,
    QgsReadWriteContext,
    QgsCoordinateReferenceSystem
)
from qgis.testing import start_app, unittest

from utilities import unitTestDataPath

start_app()


class TestQgsTerrainProviders(unittest.TestCase):

    def testFlatProvider(self):
        """
        Test QgsFlatTerrainProvider
        """
        provider = QgsFlatTerrainProvider()
        self.assertEqual(provider.type(), 'flat')
        self.assertFalse(provider.crs().isValid())

        self.assertEqual(provider.heightAt(1, 2), 0)

        provider.setOffset(5)
        self.assertEqual(provider.heightAt(1, 2), 5)

        clone = provider.clone()
        self.assertIsInstance(clone, QgsFlatTerrainProvider)
        self.assertEqual(clone.offset(), 5)

        # via xml
        doc = QDomDocument("testdoc")

        context = QgsReadWriteContext()
        parent_elem = doc.createElement('test')
        element = provider.writeXml(doc, context)
        parent_elem.appendChild(element)

        from_xml = QgsRasterDemTerrainProvider()
        self.assertTrue(from_xml.readXml(parent_elem, context))

        self.assertEqual(from_xml.offset(), 5)

        # test equals
        provider1 = QgsFlatTerrainProvider()
        provider2 = QgsFlatTerrainProvider()
        self.assertTrue(provider1.equals(provider2))
        self.assertFalse(provider1.equals(QgsRasterDemTerrainProvider()))
        provider1.setOffset(1)
        self.assertFalse(provider1.equals(provider2))
        provider2.setOffset(1)
        self.assertTrue(provider1.equals(provider2))

    def testRasterDemProvider(self):
        """
        Test QgsRasterDemTerrainProvider
        """
        provider = QgsRasterDemTerrainProvider()
        self.assertEqual(provider.type(), 'raster')

        # without layer assigned
        self.assertFalse(provider.crs().isValid())
        self.assertTrue(math.isnan(provider.heightAt(1, 2)))

        # add raster layer to project
        p = QgsProject()
        rl = QgsRasterLayer(os.path.join(unitTestDataPath(), 'float1-16.tif'), 'rl')
        self.assertTrue(rl.isValid())
        p.addMapLayer(rl)

        provider.setLayer(rl)
        self.assertEqual(provider.layer(), rl)

        self.assertEqual(provider.crs().authid(), 'EPSG:4326')
        self.assertEqual(provider.heightAt(106.4105, -6.6341), 11.0)
        # outside of raster extent
        self.assertTrue(math.isnan(provider.heightAt(1, 2)))

        provider.setOffset(5)
        self.assertEqual(provider.offset(), 5)
        provider.setScale(3)
        self.assertEqual(provider.scale(), 3)
        self.assertEqual(provider.heightAt(106.4105, -6.6341), 11 * 3 + 5)

        clone = provider.clone()
        self.assertIsInstance(clone, QgsRasterDemTerrainProvider)
        self.assertEqual(clone.layer(), rl)
        self.assertEqual(clone.offset(), 5)
        self.assertEqual(clone.scale(), 3)

        # via xml
        doc = QDomDocument("testdoc")

        context = QgsReadWriteContext()
        parent_elem = doc.createElement('test')
        element = provider.writeXml(doc, context)
        parent_elem.appendChild(element)

        from_xml = QgsRasterDemTerrainProvider()
        self.assertTrue(from_xml.readXml(parent_elem, context))

        self.assertEqual(from_xml.offset(), 5)
        self.assertEqual(from_xml.scale(), 3)

        # layer won't be resolved till we resolve references from project
        self.assertFalse(from_xml.layer())
        from_xml.resolveReferences(p)
        self.assertEqual(from_xml.layer(), rl)

        # test equals
        provider1 = QgsRasterDemTerrainProvider()
        provider2 = QgsRasterDemTerrainProvider()
        self.assertTrue(provider1.equals(provider2))
        self.assertFalse(provider1.equals(QgsFlatTerrainProvider()))
        provider1.setOffset(1)
        self.assertFalse(provider1.equals(provider2))
        provider2.setOffset(1)
        self.assertTrue(provider1.equals(provider2))
        provider1.setScale(11)
        self.assertFalse(provider1.equals(provider2))
        provider2.setScale(11)
        self.assertTrue(provider1.equals(provider2))
        provider1.setLayer(rl)
        self.assertFalse(provider1.equals(provider2))
        provider2.setLayer(rl)
        self.assertTrue(provider1.equals(provider2))

    def testMeshProvider(self):
        """
        Test QgsMeshTerrainProvider
        """
        provider = QgsMeshTerrainProvider()
        self.assertEqual(provider.type(), 'mesh')

        # without layer assigned
        self.assertFalse(provider.crs().isValid())
        self.assertTrue(math.isnan(provider.heightAt(1, 2)))

        # add mesh layer to project
        p = QgsProject()
        mesh_layer = QgsMeshLayer(os.path.join(unitTestDataPath(), '3d', 'elev_mesh.2dm'), 'mdal', 'mdal')
        mesh_layer.setCrs(QgsCoordinateReferenceSystem('EPSG:27700'))
        self.assertTrue(mesh_layer.isValid())
        p.addMapLayer(mesh_layer)

        provider.setLayer(mesh_layer)
        self.assertEqual(provider.layer(), mesh_layer)
        self.assertEqual(provider.crs().authid(), 'EPSG:27700')

        self.assertTrue(math.isnan(provider.heightAt(1, 2)))
        self.assertAlmostEqual(provider.heightAt(321695.2, 129990.5), 89.49743150684921, 5)

        provider.setOffset(5)
        self.assertEqual(provider.offset(), 5)
        provider.setScale(3)
        self.assertEqual(provider.scale(), 3)

        clone = provider.clone()
        self.assertIsInstance(clone, QgsMeshTerrainProvider)
        self.assertEqual(clone.layer(), mesh_layer)
        self.assertEqual(clone.offset(), 5)
        self.assertEqual(clone.scale(), 3)

        # via xml
        doc = QDomDocument("testdoc")

        context = QgsReadWriteContext()
        parent_elem = doc.createElement('test')
        element = provider.writeXml(doc, context)
        parent_elem.appendChild(element)

        from_xml = QgsMeshTerrainProvider()
        self.assertTrue(from_xml.readXml(parent_elem, context))

        self.assertEqual(from_xml.offset(), 5)
        self.assertEqual(from_xml.scale(), 3)

        # layer won't be resolved till we resolve references from project
        self.assertFalse(from_xml.layer())
        from_xml.resolveReferences(p)
        self.assertEqual(from_xml.layer(), mesh_layer)

        # test equals
        provider1 = QgsMeshTerrainProvider()
        provider2 = QgsMeshTerrainProvider()
        self.assertTrue(provider1.equals(provider2))
        self.assertFalse(provider1.equals(QgsFlatTerrainProvider()))
        provider1.setOffset(1)
        self.assertFalse(provider1.equals(provider2))
        provider2.setOffset(1)
        self.assertTrue(provider1.equals(provider2))
        provider1.setScale(11)
        self.assertFalse(provider1.equals(provider2))
        provider2.setScale(11)
        self.assertTrue(provider1.equals(provider2))
        provider1.setLayer(mesh_layer)
        self.assertFalse(provider1.equals(provider2))
        provider2.setLayer(mesh_layer)
        self.assertTrue(provider1.equals(provider2))


if __name__ == '__main__':
    unittest.main()
