"""QGIS Unit tests for QgsTiledSceneLayerElevationProperties

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "23/08/2023"
__copyright__ = "Copyright 2023, The QGIS Project"

import tempfile
import os

from qgis.PyQt.QtGui import QColor
from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    Qgis,
    QgsTiledSceneLayer,
    QgsTiledSceneLayerElevationProperties,
    QgsProviderRegistry,
    QgsReadWriteContext,
    QgsUnitTypes,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()


class TestQgsTiledSceneElevationProperties(QgisTestCase):

    def testBasic(self):
        props = QgsTiledSceneLayerElevationProperties(None)
        self.assertEqual(props.zScale(), 1)
        self.assertEqual(props.zOffset(), 0)

        props.setZOffset(0.5)
        props.setZScale(2)

        self.assertEqual(props.zScale(), 2)
        self.assertEqual(props.zOffset(), 0.5)

        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")
        props.writeXml(elem, doc, QgsReadWriteContext())

        props2 = QgsTiledSceneLayerElevationProperties(None)
        props2.readXml(elem, QgsReadWriteContext())
        self.assertEqual(props2.zScale(), 2)
        self.assertEqual(props2.zOffset(), 0.5)

        props2 = props.clone()
        self.assertEqual(props2.zScale(), 2)
        self.assertEqual(props2.zOffset(), 0.5)

    def testCalculateZRange(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            tmp_file = os.path.join(temp_dir, "tileset.json")
            with open(tmp_file, "w", encoding="utf-8") as f:
                f.write(
                    """
{
  "asset": {
    "version": "1.1",
    "tilesetVersion": "e575c6f1"
  },
  "geometricError": 100,
  "root": {
    "boundingVolume": {
      "region": [
        -1.3197209591796106,
        0.6988424218,
        -1.3196390408203893,
        0.6989055782,
        1.2,
        67.00999999999999
      ]
    },
    "geometricError": 100,
    "refine": "ADD",
    "children": []
  }
}"""
                )

            layer = QgsTiledSceneLayer(tmp_file, "my layer", "cesiumtiles")
            self.assertTrue(layer.dataProvider().isValid())
            self.assertEqual(layer.dataProvider().zRange().lower(), 1.2)
            self.assertEqual(layer.dataProvider().zRange().upper(), 67.00999999999999)

            props = QgsTiledSceneLayerElevationProperties(layer)
            self.assertEqual(props.zScale(), 1.0)
            self.assertEqual(props.zOffset(), 0.0)

            z_range = props.calculateZRange(layer)
            self.assertEqual(z_range.lower(), 1.2)
            self.assertEqual(z_range.upper(), 67.00999999999999)
            self.assertEqual(props.significantZValues(layer), [1.2, 67.00999999999999])

            props.setZOffset(10)
            z_range = props.calculateZRange(layer)
            self.assertEqual(z_range.lower(), 11.2)
            self.assertEqual(z_range.upper(), 77.00999999999999)
            self.assertEqual(props.significantZValues(layer), [11.2, 77.00999999999999])

            props.setZScale(2)
            z_range = props.calculateZRange(layer)
            self.assertEqual(z_range.lower(), 12.4)
            self.assertEqual(z_range.upper(), 144.01999999999998)
            self.assertEqual(
                props.significantZValues(layer), [12.4, 144.01999999999998]
            )


if __name__ == "__main__":
    unittest.main()
