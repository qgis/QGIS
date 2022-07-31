# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsVectorLayer load/write named style.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
__author__ = 'Alessandro Pasotti'
__date__ = '22/01/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import tempfile
from qgis.core import (
    QgsVectorLayer,
    QgsMapLayer,
    QgsReadWriteContext,
)
from qgis.PyQt.QtXml import QDomDocument, QDomNode

from qgis.testing import unittest


class TestPyQgsVectorLayerNamedStyle(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        pass

    def testLoadWriteRenderingScaleVisibility(self):
        """Test write and load scale visibility, see GH #33840"""

        vl = QgsVectorLayer("LineString?crs=epsg:4326", "result", "memory")
        vl.setScaleBasedVisibility(True)
        vl.setMinimumScale(125.0)
        vl.setMaximumScale(1.25)
        style = QDomDocument()
        style.setContent("<!DOCTYPE qgis PUBLIC 'http://mrcc.com/qgis.dtd' 'SYSTEM'><qgis></qgis>")
        node = style.firstChild()
        self.assertTrue(vl.writeStyle(node, style, "Error writing style", QgsReadWriteContext(), QgsMapLayer.Rendering))

        style_content = style.toString()
        del vl

        # Read
        vl2 = QgsVectorLayer("LineString?crs=epsg:4326", "result", "memory")
        self.assertFalse(vl2.hasScaleBasedVisibility())
        style2 = QDomDocument()
        style2.setContent(style_content)
        self.assertTrue(vl2.readStyle(style.namedItem('qgis'), "Error reading style", QgsReadWriteContext(), QgsMapLayer.Rendering))
        self.assertTrue(vl2.hasScaleBasedVisibility())
        self.assertEqual(vl2.minimumScale(), 125.0)
        self.assertEqual(vl2.maximumScale(), 1.25)


if __name__ == '__main__':
    unittest.main()
