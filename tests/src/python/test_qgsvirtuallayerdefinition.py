# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsVirtualLayerDefinition

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Hugo Mercier'
__date__ = '10/12/2015'
__copyright__ = 'Copyright 2015, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.core import (QgsField,
                       QgsWKBTypes,
                       QgsFields,
                       QgsVirtualLayerDefinition
                       )

from qgis.testing import unittest
from qgis.PyQt.QtCore import QVariant, QUrl
import os


class TestQgsVirtualLayerDefinition(unittest.TestCase):

    def test1(self):
        d = QgsVirtualLayerDefinition()
        self.assertEqual(d.toString(), "")
        d.setFilePath("/file")
        self.assertEqual(d.toString(), "file:///file")
        self.assertEqual(QgsVirtualLayerDefinition.fromUrl(d.toUrl()).filePath(), "/file")
        self.assertEqual(QgsVirtualLayerDefinition.fromUrl(QUrl.fromEncoded(d.toString())).filePath(), "/file")
        d.setFilePath(os.path.join('C:/', 'file'))
        self.assertEqual(d.toString(), "file:///C:/file")
        self.assertEqual(QgsVirtualLayerDefinition.fromUrl(d.toUrl()).filePath(), os.path.join('C:/', 'file'))
        d.setQuery("SELECT * FROM mytable")
        self.assertEqual(QgsVirtualLayerDefinition.fromUrl(d.toUrl()).query(), "SELECT * FROM mytable")
        self.assertEqual(QgsVirtualLayerDefinition.fromUrl(QUrl.fromEncoded(d.toString())).query(), "SELECT * FROM mytable")

        q = u"SELECT * FROM tableéé /*:int*/"
        d.setQuery(q)
        self.assertEqual(QgsVirtualLayerDefinition.fromUrl(d.toUrl()).query(), q)
        self.assertEqual(QgsVirtualLayerDefinition.fromUrl(QUrl.fromEncoded(d.toString())).query(), q)

        s1 = u"file://foo&bar=okié"
        d.addSource("name", s1, "provider", "utf8")
        self.assertEqual(QgsVirtualLayerDefinition.fromUrl(d.toUrl()).sourceLayers()[0].source(), s1)
        self.assertEqual(QgsVirtualLayerDefinition.fromUrl(QUrl.fromEncoded(d.toString())).sourceLayers()[0].source(), s1)

        n1 = u"éé ok"
        d.addSource(n1, s1, "provider")
        self.assertEqual(QgsVirtualLayerDefinition.fromUrl(d.toUrl()).sourceLayers()[1].name(), n1)
        self.assertEqual(QgsVirtualLayerDefinition.fromUrl(QUrl.fromEncoded(d.toString())).sourceLayers()[1].name(), n1)

        d.addSource("ref1", "id0001")
        self.assertEqual(QgsVirtualLayerDefinition.fromUrl(d.toUrl()).sourceLayers()[2].reference(), "id0001")
        self.assertEqual(QgsVirtualLayerDefinition.fromUrl(QUrl.fromEncoded(d.toString())).sourceLayers()[2].reference(), "id0001")

        s = "dbname='C:\\tt' table=\"test\" (geometry) sql="
        d.addSource("nn", s, "spatialite")
        self.assertEqual(QgsVirtualLayerDefinition.fromUrl(d.toUrl()).sourceLayers()[3].source(), s)
        self.assertEqual(QgsVirtualLayerDefinition.fromUrl(QUrl.fromEncoded(d.toString())).sourceLayers()[3].source(), s)

        d.setGeometryField("geom")
        self.assertEqual(QgsVirtualLayerDefinition.fromUrl(d.toUrl()).geometryField(), "geom")
        self.assertEqual(QgsVirtualLayerDefinition.fromUrl(QUrl.fromEncoded(d.toString())).geometryField(), "geom")

        d.setGeometryWkbType(QgsWKBTypes.Point)
        self.assertEqual(QgsVirtualLayerDefinition.fromUrl(d.toUrl()).geometryWkbType(), QgsWKBTypes.Point)
        self.assertEqual(QgsVirtualLayerDefinition.fromUrl(QUrl.fromEncoded(d.toString())).geometryWkbType(), QgsWKBTypes.Point)

        f = QgsFields()
        f.append(QgsField("a", QVariant.Int))
        f.append(QgsField("f", QVariant.Double))
        f.append(QgsField("s", QVariant.String))
        d.setFields(f)
        f2 = QgsVirtualLayerDefinition.fromUrl(d.toUrl()).fields()
        self.assertEqual(f[0].name(), f2[0].name())
        self.assertEqual(f[0].type(), f2[0].type())
        self.assertEqual(f[1].name(), f2[1].name())
        self.assertEqual(f[1].type(), f2[1].type())
        self.assertEqual(f[2].name(), f2[2].name())
        self.assertEqual(f[2].type(), f2[2].type())

if __name__ == '__main__':
    unittest.main()
