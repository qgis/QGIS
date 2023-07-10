"""QGIS Unit tests for QgsCesiumTiledMeshLayer

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '27/06/2023'
__copyright__ = 'Copyright 2023, The QGIS Project'

import os
import tempfile

import qgis  # NOQA
from qgis.core import (
    QgsTiledMeshLayer,
    QgsCoordinateReferenceSystem
)
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

start_app()


class TestQgsCesiumTiledMeshLayer(unittest.TestCase):

    def test_invalid_source(self):
        layer = QgsTiledMeshLayer('/nope/tileset.json', 'my layer',
                                  'cesiumtiles')
        self.assertFalse(layer.dataProvider().isValid())

    def test_valid_source(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            tmp_file = os.path.join(temp_dir, 'tileset.json')
            with open(tmp_file, 'wt', encoding='utf-8') as f:
                f.write("""
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
}""")

            layer = QgsTiledMeshLayer(tmp_file, 'my layer',
                                      'cesiumtiles')
            self.assertTrue(layer.dataProvider().isValid())

            self.assertEqual(layer.dataProvider().crs(),
                             QgsCoordinateReferenceSystem('EPSG:4979'))

            self.assertAlmostEqual(layer.dataProvider().extent().xMinimum(),
                                   -75.61444109, 3)
            self.assertAlmostEqual(layer.dataProvider().extent().xMaximum(),
                                   -75.60974751, 3)
            self.assertAlmostEqual(layer.dataProvider().extent().yMinimum(),
                                   40.04072131, 3)
            self.assertAlmostEqual(layer.dataProvider().extent().yMaximum(),
                                   40.044339909, 3)

            # check that version, tileset version, and z range are in html metadata
            self.assertIn('1.1', layer.dataProvider().htmlMetadata())
            self.assertIn('e575c6f1', layer.dataProvider().htmlMetadata())
            self.assertIn('1.2 - 67.01', layer.dataProvider().htmlMetadata())


if __name__ == '__main__':
    unittest.main()
