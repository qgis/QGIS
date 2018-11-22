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
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
import qgis  # NOQA
import tempfile

from qgis.core import (QgsReadWriteContext,
                       QgsVectorLayer,
                       QgsProject)
from qgis.testing import start_app, unittest
from qgis.PyQt.QtXml import QDomDocument
from qgis.PyQt.QtCore import QTemporaryDir

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


if __name__ == '__main__':
    unittest.main()
