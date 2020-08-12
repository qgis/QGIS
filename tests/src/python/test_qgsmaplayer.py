# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsMapLayer

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '1/02/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'

import os
import qgis  # NOQA
import tempfile
import glob
import shutil

from qgis.core import (QgsReadWriteContext,
                       QgsVectorLayer,
                       QgsRasterLayer,
                       QgsProject)
from qgis.testing import start_app, unittest
from qgis.PyQt.QtXml import QDomDocument
from qgis.PyQt.QtCore import QTemporaryDir
from utilities import unitTestDataPath

TEST_DATA_DIR = unitTestDataPath()

start_app()


class TestQgsMapLayer(unittest.TestCase):

    def testUniqueId(self):
        """
        Test that layers created quickly with same name get a unique ID
        """

        # make 1000 layers quickly
        layers = []
        for i in range(1000):
            layer = QgsVectorLayer(
                'Point?crs=epsg:4326&field=name:string(20)',
                'test',
                'memory')
            layers.append(layer)

        # make sure all ids are unique
        ids = set()
        for l in layers:
            self.assertFalse(l.id() in ids)
            ids.add(l.id())

    def copyLayerViaXmlReadWrite(self, source, dest):
        # write to xml
        doc = QDomDocument("testdoc")
        elem = doc.createElement("maplayer")
        self.assertTrue(source.writeLayerXml(elem, doc, QgsReadWriteContext()))
        self.assertTrue(dest.readLayerXml(elem, QgsReadWriteContext()), QgsProject.instance())

    def testGettersSetters(self):
        # test auto refresh getters/setters
        layer = QgsVectorLayer("Point?field=fldtxt:string",
                               "layer", "memory")
        self.assertFalse(layer.hasAutoRefreshEnabled())
        self.assertEqual(layer.autoRefreshInterval(), 0)
        layer.setAutoRefreshInterval(5)
        self.assertFalse(layer.hasAutoRefreshEnabled())
        self.assertEqual(layer.autoRefreshInterval(), 5)
        layer.setAutoRefreshEnabled(True)
        self.assertTrue(layer.hasAutoRefreshEnabled())
        self.assertEqual(layer.autoRefreshInterval(), 5)
        layer.setAutoRefreshInterval(0)  # should disable auto refresh
        self.assertFalse(layer.hasAutoRefreshEnabled())
        self.assertEqual(layer.autoRefreshInterval(), 0)

    def testSaveRestoreAutoRefresh(self):
        """ test saving/restoring auto refresh to xml """
        layer = QgsVectorLayer("Point?field=fldtxt:string",
                               "layer", "memory")
        layer2 = QgsVectorLayer("Point?field=fldtxt:string",
                                "layer", "memory")
        self.copyLayerViaXmlReadWrite(layer, layer2)
        self.assertFalse(layer2.hasAutoRefreshEnabled())
        self.assertEqual(layer2.autoRefreshInterval(), 0)

        layer.setAutoRefreshInterval(56)
        self.copyLayerViaXmlReadWrite(layer, layer2)
        self.assertFalse(layer2.hasAutoRefreshEnabled())
        self.assertEqual(layer2.autoRefreshInterval(), 56)

        layer.setAutoRefreshEnabled(True)
        self.copyLayerViaXmlReadWrite(layer, layer2)
        self.assertTrue(layer2.hasAutoRefreshEnabled())
        self.assertEqual(layer2.autoRefreshInterval(), 56)

    def testReadWriteMetadata(self):
        layer = QgsVectorLayer("Point?field=fldtxt:string", "layer", "memory")
        m = layer.metadata()
        # Only abstract, more tests are done in test_qgslayermetadata.py
        m.setAbstract('My abstract')
        layer.setMetadata(m)
        self.assertTrue(layer.metadata().abstract(), 'My abstract')
        destination = tempfile.NamedTemporaryFile(suffix='.qmd').name
        message, status = layer.saveNamedMetadata(destination)
        self.assertTrue(status, message)

        layer2 = QgsVectorLayer("Point?field=fldtxt:string", "layer", "memory")
        message, status = layer2.loadNamedMetadata(destination)
        self.assertTrue(status)
        self.assertTrue(layer2.metadata().abstract(), 'My abstract')

    def testSaveNamedStyle(self):
        layer = QgsVectorLayer("Point?field=fldtxt:string", "layer", "memory")
        dir = QTemporaryDir()
        dir_path = dir.path()
        style_path = os.path.join(dir_path, 'my.qml')
        _, result = layer.saveNamedStyle(style_path)
        self.assertTrue(result)
        self.assertTrue(os.path.exists(style_path))

    def testStyleUri(self):
        # shapefile
        layer = QgsVectorLayer(os.path.join(TEST_DATA_DIR, 'points.shp'), "layer", "ogr")
        uri = layer.styleURI()
        self.assertEqual(uri, os.path.join(TEST_DATA_DIR, 'points.qml'))

        # geopackage without and with layername
        layer = QgsVectorLayer(os.path.join(TEST_DATA_DIR, 'provider', 'bug_17795.gpkg'), "layer", "ogr")
        uri = layer.styleURI()
        self.assertEqual(uri, os.path.join(TEST_DATA_DIR, 'provider', 'bug_17795.qml'))

        layer = QgsVectorLayer("{}|layername=bug_17795".format(os.path.join(TEST_DATA_DIR, 'provider', 'bug_17795.gpkg')), "layer", "ogr")
        uri = layer.styleURI()
        self.assertEqual(uri, os.path.join(TEST_DATA_DIR, 'provider', 'bug_17795.qml'))

        # delimited text
        uri = 'file://{}?type=csv&detectTypes=yes&geomType=none'.format(os.path.join(TEST_DATA_DIR, 'delimitedtext', 'test.csv'))
        layer = QgsVectorLayer(uri, "layer", "delimitedtext")
        uri = layer.styleURI()
        self.assertEqual(uri, os.path.join(TEST_DATA_DIR, 'delimitedtext', 'test.qml'))

    def testIsTemporary(self):
        # test if a layer is correctly marked as temporary
        dir = QTemporaryDir()
        dir_path = dir.path()
        for file in glob.glob(os.path.join(TEST_DATA_DIR, 'france_parts.*')):
            shutil.copy(os.path.join(TEST_DATA_DIR, file), dir_path)

        not_temp_source = os.path.join(TEST_DATA_DIR, 'france_parts.*')
        temp_source = os.path.join(dir_path, 'france_parts.shp')

        vl = QgsVectorLayer('invalid', 'test')
        self.assertFalse(vl.isValid())
        self.assertFalse(vl.isTemporary())

        vl = QgsVectorLayer(os.path.join(TEST_DATA_DIR, 'france_parts.shp'), 'test')
        self.assertTrue(vl.isValid())
        self.assertFalse(vl.isTemporary())

        vl = QgsVectorLayer(temp_source, 'test')
        self.assertTrue(vl.isValid())
        self.assertTrue(vl.isTemporary())

        # memory layers are temp
        layer = QgsVectorLayer("Point?field=fldtxt:string",
                               "layer", "memory")
        self.assertTrue(vl.isValid())
        self.assertTrue(vl.isTemporary())

        rl = QgsRasterLayer('invalid', 'test')
        self.assertFalse(rl.isValid())
        self.assertFalse(rl.isTemporary())

        not_temp_source = os.path.join(TEST_DATA_DIR, 'float1-16.tif')
        shutil.copy(not_temp_source, dir_path)
        temp_source = os.path.join(dir_path, 'float1-16.tif')

        rl = QgsRasterLayer(not_temp_source, 'test')
        self.assertTrue(rl.isValid())
        self.assertFalse(rl.isTemporary())

        rl = QgsRasterLayer(temp_source, 'test')
        self.assertTrue(rl.isValid())
        self.assertTrue(rl.isTemporary())


if __name__ == '__main__':
    unittest.main()
