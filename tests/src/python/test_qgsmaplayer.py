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

import qgis  # NOQA

from qgis.core import (QgsMapRendererCache,
                       QgsRectangle,
                       QgsVectorLayer,
                       QgsProject)
from qgis.testing import start_app, unittest
from qgis.PyQt.QtXml import (QDomDocument, QDomElement)

start_app()


class TestQgsMapLayer(unittest.TestCase):

    def copyLayerViaXmlReadWrite(self, source, dest):
        # write to xml
        doc = QDomDocument("testdoc")
        elem = doc.createElement("maplayer")
        self.assertTrue(source.writeLayerXml(elem, doc))
        self.assertTrue(dest.readLayerXml(elem), QgsProject.instance())

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
        layer.setAutoRefreshInterval(0) # should disable auto refresh
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


if __name__ == '__main__':
    unittest.main()
