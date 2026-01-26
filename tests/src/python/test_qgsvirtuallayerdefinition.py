"""QGIS Unit tests for QgsVirtualLayerDefinition

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Hugo Mercier"
__date__ = "10/12/2015"
__copyright__ = "Copyright 2015, The QGIS Project"

import os

from qgis.PyQt.QtCore import QUrl, QVariant
from qgis.core import (
    QgsField,
    QgsFields,
    QgsVirtualLayerDefinition,
    QgsWkbTypes,
)
from qgis.testing import unittest


def strToUrl(s):
    return QUrl.fromEncoded(bytes(s, "utf8"))


class TestQgsVirtualLayerDefinition(unittest.TestCase):

    def test1(self):
        d = QgsVirtualLayerDefinition()
        self.assertEqual(d.toString(), "")
        d.setFilePath("/file")
        self.assertEqual(d.toString(), "file:///file")
        self.assertEqual(
            QgsVirtualLayerDefinition.fromUrl(d.toUrl()).filePath(), "/file"
        )
        self.assertEqual(
            QgsVirtualLayerDefinition.fromUrl(strToUrl(d.toString())).filePath(),
            "/file",
        )
        d.setFilePath(os.path.join("C:/", "file"))
        self.assertEqual(d.toString(), "file:///C:/file")
        # self.assertEqual(QgsVirtualLayerDefinition.fromUrl(d.toUrl()).filePath(), os.path.join('C:/', 'file'))
        d.setQuery("SELECT * FROM mytable")
        self.assertEqual(
            QgsVirtualLayerDefinition.fromUrl(d.toUrl()).query(),
            "SELECT * FROM mytable",
        )
        self.assertEqual(
            QgsVirtualLayerDefinition.fromUrl(strToUrl(d.toString())).query(),
            "SELECT * FROM mytable",
        )

        q = "SELECT * FROM tableéé /*:int*/"
        d.setQuery(q)
        self.assertEqual(QgsVirtualLayerDefinition.fromUrl(d.toUrl()).query(), q)
        self.assertEqual(
            QgsVirtualLayerDefinition.fromUrl(strToUrl(d.toString())).query(), q
        )

        s1 = "file://foo&bar=okié"
        d.addSource("name", s1, "provider", "utf8")
        self.assertEqual(
            QgsVirtualLayerDefinition.fromUrl(d.toUrl()).sourceLayers()[0].source(), s1
        )
        self.assertEqual(
            QgsVirtualLayerDefinition.fromUrl(strToUrl(d.toString()))
            .sourceLayers()[0]
            .source(),
            s1,
        )

        n1 = "éé ok"
        d.addSource(n1, s1, "provider")
        self.assertEqual(
            QgsVirtualLayerDefinition.fromUrl(d.toUrl()).sourceLayers()[1].name(), n1
        )
        self.assertEqual(
            QgsVirtualLayerDefinition.fromUrl(strToUrl(d.toString()))
            .sourceLayers()[1]
            .name(),
            n1,
        )

        d.addSource("ref1", "id0001")
        self.assertEqual(
            QgsVirtualLayerDefinition.fromUrl(d.toUrl()).sourceLayers()[2].reference(),
            "id0001",
        )
        self.assertEqual(
            QgsVirtualLayerDefinition.fromUrl(strToUrl(d.toString()))
            .sourceLayers()[2]
            .reference(),
            "id0001",
        )

        s = "dbname='C:\\tt' table=\"test\" (geometry) sql="
        d.addSource("nn", s, "spatialite")
        self.assertEqual(
            QgsVirtualLayerDefinition.fromUrl(d.toUrl()).sourceLayers()[3].source(), s
        )
        self.assertEqual(
            QgsVirtualLayerDefinition.fromUrl(strToUrl(d.toString()))
            .sourceLayers()[3]
            .source(),
            s,
        )

        d.setGeometryField("geom")
        self.assertEqual(
            QgsVirtualLayerDefinition.fromUrl(d.toUrl()).geometryField(), "geom"
        )
        self.assertEqual(
            QgsVirtualLayerDefinition.fromUrl(strToUrl(d.toString())).geometryField(),
            "geom",
        )

        d.setGeometryWkbType(QgsWkbTypes.Type.Point)
        self.assertEqual(
            QgsVirtualLayerDefinition.fromUrl(d.toUrl()).geometryWkbType(),
            QgsWkbTypes.Type.Point,
        )
        self.assertEqual(
            QgsVirtualLayerDefinition.fromUrl(strToUrl(d.toString())).geometryWkbType(),
            QgsWkbTypes.Type.Point,
        )

        f = QgsFields()
        f.append(QgsField("a", QVariant.Int))
        f.append(QgsField("f", QVariant.Double))
        f.append(QgsField("s", QVariant.String))
        d.setFields(f)

        f2 = QgsVirtualLayerDefinition.fromUrl(d.toUrl()).fields()
        self.assertEqual(f[0].name(), f2[0].name())
        self.assertEqual(f2[0].type(), QVariant.LongLong)
        self.assertEqual(f[1].name(), f2[1].name())
        self.assertEqual(f[1].type(), f2[1].type())
        self.assertEqual(f[2].name(), f2[2].name())
        self.assertEqual(f[2].type(), f2[2].type())

        # Issue https://github.com/qgis/QGIS/issues/44130
        url = QUrl(
            r"?layer_ref=Reprojet%C3%A9_e888ce1e_17a9_46f4_b8c3_254eef3f2931:input1&query=SELECT%20*%20FROM%20input1"
        )
        f3 = QgsVirtualLayerDefinition.fromUrl(url)
        self.assertEqual(f3.query(), "SELECT * FROM input1")
        source_layer = f3.sourceLayers()[0]
        self.assertEqual(
            source_layer.reference(), "Reprojeté_e888ce1e_17a9_46f4_b8c3_254eef3f2931"
        )


if __name__ == "__main__":
    unittest.main()
