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
    QgsReadWriteContext
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

        clone = provider.clone()
        self.assertIsInstance(clone, QgsFlatTerrainProvider)

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

        clone = provider.clone()
        self.assertIsInstance(clone, QgsRasterDemTerrainProvider)
        self.assertEqual(clone.layer(), rl)

        # via xml
        doc = QDomDocument("testdoc")

        context = QgsReadWriteContext()
        parent_elem = doc.createElement('test')
        element = provider.writeXml(doc, context)
        parent_elem.appendChild(element)

        from_xml = QgsRasterDemTerrainProvider()
        self.assertTrue(from_xml.readXml(parent_elem, context))

        # layer won't be resolved till we resolve references from project
        self.assertFalse(from_xml.layer())
        from_xml.resolveReferences(p)
        self.assertEqual(from_xml.layer(), rl)

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
        mesh_layer = QgsMeshLayer(os.path.join(unitTestDataPath(), 'mesh', 'quad_flower.2dm'), 'mesh', 'mdal')
        self.assertTrue(mesh_layer.isValid())
        p.addMapLayer(mesh_layer)

        provider.setLayer(mesh_layer)
        self.assertEqual(provider.layer(), mesh_layer)

        # not implemented yet
        # self.assertEqual(provider.heightAt(1,2), 0)
        # self.assertEqual(provider.heightAt(106.4105,-6.6341), 11.0)

        clone = provider.clone()
        self.assertIsInstance(clone, QgsMeshTerrainProvider)
        self.assertEqual(clone.layer(), mesh_layer)

        # via xml
        doc = QDomDocument("testdoc")

        context = QgsReadWriteContext()
        parent_elem = doc.createElement('test')
        element = provider.writeXml(doc, context)
        parent_elem.appendChild(element)

        from_xml = QgsMeshTerrainProvider()
        self.assertTrue(from_xml.readXml(parent_elem, context))

        # layer won't be resolved till we resolve references from project
        self.assertFalse(from_xml.layer())
        from_xml.resolveReferences(p)
        self.assertEqual(from_xml.layer(), mesh_layer)


if __name__ == '__main__':
    unittest.main()
