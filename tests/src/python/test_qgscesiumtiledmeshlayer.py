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
    Qgis,
    QgsTiledMeshLayer,
    QgsCoordinateReferenceSystem,
    QgsMatrix4x4,
    QgsOrientedBox3D,
    QgsTiledMeshRequest
)
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

start_app()


class TestQgsCesiumTiledMeshLayer(unittest.TestCase):

    def test_invalid_source(self):
        layer = QgsTiledMeshLayer('/nope/tileset.json', 'my layer',
                                  'cesiumtiles')
        self.assertFalse(layer.dataProvider().isValid())

    def test_source_bounding_volume_region(self):
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

            self.assertEqual(layer.crs(),
                             QgsCoordinateReferenceSystem('EPSG:4979'))
            self.assertEqual(layer.dataProvider().meshCrs().authid(),
                             'EPSG:4978')

            self.assertAlmostEqual(layer.extent().xMinimum(),
                                   -75.61444109, 3)
            self.assertAlmostEqual(layer.extent().xMaximum(),
                                   -75.60974751, 3)
            self.assertAlmostEqual(layer.extent().yMinimum(),
                                   40.04072131, 3)
            self.assertAlmostEqual(layer.extent().yMaximum(),
                                   40.044339909, 3)

            self.assertAlmostEqual(layer.dataProvider().boundingVolume().region().xMinimum(), -75.6144410, 3)
            self.assertAlmostEqual(layer.dataProvider().boundingVolume().region().xMaximum(), -75.6097475, 3)
            self.assertAlmostEqual(
                layer.dataProvider().boundingVolume().region().yMinimum(),
                40.0407213, 3)
            self.assertAlmostEqual(
                layer.dataProvider().boundingVolume().region().yMaximum(),
                40.044339909, 3)
            self.assertAlmostEqual(
                layer.dataProvider().boundingVolume().region().zMinimum(),
                1.2, 3)
            self.assertAlmostEqual(
                layer.dataProvider().boundingVolume().region().zMaximum(),
                67.00999, 3)

            # check that version, tileset version, and z range are in html metadata
            self.assertIn('1.1', layer.dataProvider().htmlMetadata())
            self.assertIn('e575c6f1', layer.dataProvider().htmlMetadata())
            self.assertIn('1.2 - 67.01', layer.dataProvider().htmlMetadata())

    def test_source_bounding_volume_region_with_transform(self):
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
    "children": [],
    "transform":[1, 0, 0, 200, 0, 1, 0, 500, 0, 0, 1, 1000, 0, 0, 0, 1]
  }
}""")

            layer = QgsTiledMeshLayer(tmp_file, 'my layer',
                                      'cesiumtiles')
            self.assertTrue(layer.dataProvider().isValid())

            self.assertEqual(
                layer.dataProvider().boundingVolume().transform(),
                QgsMatrix4x4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 200, 500, 1000, 1))

    def test_source_bounding_volume_box(self):
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
    "boundingVolume": {"box":[-4595750,2698725,-3493318,182,0,0,0,86,0,0,0,20]},
    "geometricError": 100,
    "refine": "ADD",
    "children": []
  }
}""")

            layer = QgsTiledMeshLayer(tmp_file, 'my layer',
                                      'cesiumtiles')
            self.assertTrue(layer.dataProvider().isValid())

            self.assertEqual(layer.dataProvider().meshCrs().authid(), 'EPSG:4978')
            # layer must advertise as EPSG:4979, as the various QgsMapLayer
            # methods which utilize crs (such as layer extent transformation)
            # are all purely 2D and can't handle the cesium data source z value
            # range in EPSG:4978
            self.assertEqual(layer.dataProvider().crs().authid(), 'EPSG:4979')

            self.assertAlmostEqual(layer.dataProvider().extent().xMinimum(),
                                   149.575823489, 3)
            self.assertAlmostEqual(layer.dataProvider().extent().xMaximum(),
                                   149.57939956, 3)
            self.assertAlmostEqual(layer.dataProvider().extent().yMinimum(),
                                   -33.42101266, 3)
            self.assertAlmostEqual(layer.dataProvider().extent().yMaximum(),
                                   -33.41902168, 3)

            self.assertAlmostEqual(layer.dataProvider().boundingVolume().box().centerX(), -4595750, 1)
            self.assertAlmostEqual(layer.dataProvider().boundingVolume().box().centerY(), 2698725, 1)
            self.assertEqual(
                layer.dataProvider().boundingVolume().box().centerZ(),
                -3493318.0)
            self.assertEqual(
                layer.dataProvider().boundingVolume().box().halfAxes(),
                [182.0, 0.0, 0.0, 0.0, 86.0, 0.0, 0.0, 0.0, 20.0])

            # check that version, tileset version, and z range are in html metadata
            self.assertIn('1.1', layer.dataProvider().htmlMetadata())
            self.assertIn('e575c6f1', layer.dataProvider().htmlMetadata())
            self.assertIn('519.977 - 876.687', layer.dataProvider().htmlMetadata())

    def test_source_bounding_sphere(self):
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
    "boundingVolume": {"sphere":[-4595750.5786738498136401,2698725.1282528499141335,-3493318,1983]},
    "geometricError": 100,
    "refine": "ADD",
    "children": []
  }
}""")

            layer = QgsTiledMeshLayer(tmp_file, 'my layer',
                                      'cesiumtiles')
            self.assertTrue(layer.dataProvider().isValid())

            self.assertEqual(layer.dataProvider().meshCrs().authid(), 'EPSG:4978')
            # layer must advertise as EPSG:4979, as the various QgsMapLayer
            # methods which utilize crs (such as layer extent transformation)
            # are all purely 2D and can't handle the cesium data source z value
            # range in EPSG:4978
            self.assertEqual(layer.dataProvider().crs().authid(), 'EPSG:4979')

            # extent must be in EPSG:4979 to match the layer crs()
            self.assertAlmostEqual(layer.extent().xMinimum(),
                                   149.5562895, 3)
            self.assertAlmostEqual(layer.extent().xMaximum(),
                                   149.5989376, 3)
            self.assertAlmostEqual(layer.extent().yMinimum(),
                                   -33.4378807, 3)
            self.assertAlmostEqual(layer.extent().yMaximum(),
                                   -33.402147, 3)

            self.assertAlmostEqual(layer.dataProvider().boundingVolume().sphere().centerX(), -4595750.5786, 1)
            self.assertAlmostEqual(layer.dataProvider().boundingVolume().sphere().centerY(), 2698725.128252, 1)
            self.assertEqual(
                layer.dataProvider().boundingVolume().sphere().centerZ(),
                -3493318.0)
            self.assertEqual(
                layer.dataProvider().boundingVolume().sphere().radius(),
                1983.0)

            # check that version, tileset version, and z range are in html metadata
            self.assertIn('1.1', layer.dataProvider().htmlMetadata())
            self.assertIn('e575c6f1', layer.dataProvider().htmlMetadata())
            self.assertIn('-2,658.68 - 4,056.37', layer.dataProvider().htmlMetadata())

    def test_index(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            tmp_file = os.path.join(temp_dir, 'tileset.json')
            with open(tmp_file, 'wt', encoding='utf-8') as f:
                f.write("""
        {
  "asset": {
    "version": "1.0"
  },
  "geometricError": 100.0,
  "root": {
    "transform": [
      -0.45434427515502945,
      -0.8908261781255931,
      0.0,
      0.0,
      0.7960963970658078,
      -0.4060296490606728,
      0.4487431901015397,
      0.0,
      -0.39975218099804105,
      0.20388389943743965,
      0.8936607574116106,
      0.0,
      -5061037.787957486,
      2571460.026087591,
      -2824903.437545935,
      1.0
    ],
    "boundingVolume": {
      "box": [
        -1.45782,
        0.265355,
        7.44958,
        94.1946,
        0.0,
        0.0,
        0.0,
        -14.9309,
        0.0,
        0.0,
        0.0,
        75.0565
      ]
    },
    "geometricError": 100.0,
    "refine": "ADD",
    "content": null,
    "children": [
      {
        "transform": null,
        "boundingVolume": {
          "box": [
            44.4926,
            -2.87012,
            39.2136,
            45.9504,
            0.0,
            0.0,
            0.0,
            -8.28534,
            0.0,
            0.0,
            0.0,
            31.8188
          ]
        },
        "geometricError": 9.1,
        "refine": "ADD",
        "content": {
          "uri": "LOD-2/Mesh-XR-YR.b3dm"
        },
        "children": [
          {
            "transform": null,
            "boundingVolume": {
              "box": [
                44.4926,
                -2.89708,
                39.2136,
                45.9504,
                0.0,
                0.0,
                0.0,
                -8.31229,
                0.0,
                0.0,
                0.0,
                31.8188
              ]
            },
            "geometricError": 3.0,
            "refine": "ADD",
            "content": {
              "uri": "LOD-1/Mesh-XR-YR.b3dm"
            },
            "children": [
              {
                "transform": null,
                "boundingVolume": {
                  "box": [
                    44.4926,
                    -2.89708,
                    39.2136,
                    45.9504,
                    0.0,
                    0.0,
                    0.0,
                    -8.31229,
                    0.0,
                    0.0,
                    0.0,
                    31.8188
                  ]
                },
                "geometricError": 0.0,
                "refine": "ADD",
                "content": {
                  "uri": "LOD-0/Mesh-XR-YR.b3dm"
                },
                "children": []
              }
            ]
          }
        ]
      },
      {
        "transform": null,
        "boundingVolume": {
          "box": [
            -48.5551,
            5.67839,
            44.9504,
            47.0973,
            0.0,
            0.0,
            0.0,
            -9.5179,
            0.0,
            0.0,
            0.0,
            37.5556
          ]
        },
        "geometricError": 9.1,
        "refine": "ADD",
        "content": {
          "uri": "LOD-2/Mesh-XL-YR.b3dm"
        },
        "children": [
          {
            "transform": null,
            "boundingVolume": {
              "box": [
                -48.5551,
                5.7113,
                44.9504,
                47.0973,
                0.0,
                0.0,
                0.0,
                -9.48498,
                0.0,
                0.0,
                0.0,
                37.5556
              ]
            },
            "geometricError": 3.0,
            "refine": "ADD",
            "content": {
              "uri": "LOD-1/Mesh-XL-YR.b3dm"
            },
            "children": [
              {
                "transform": null,
                "boundingVolume": {
                  "box": [
                    -48.5551,
                    5.7113,
                    44.9504,
                    47.0973,
                    0.0,
                    0.0,
                    0.0,
                    -9.48498,
                    0.0,
                    0.0,
                    0.0,
                    37.5556
                  ]
                },
                "geometricError": 0.0,
                "refine": "ADD",
                "content": {
                  "uri": "LOD-0/Mesh-XL-YR.b3dm"
                },
                "children": []
              }
            ]
          }
        ]
      },
      {
        "transform": null,
        "boundingVolume": {
          "box": [
            45.6395,
            -2.2089,
            -30.1061,
            47.0973,
            0.0,
            0.0,
            0.0,
            -12.4567,
            0.0,
            0.0,
            0.0,
            37.5008
          ]
        },
        "geometricError": 9.0,
        "refine": "ADD",
        "content": {
          "uri": "LOD-2/Mesh-XR-YL.b3dm"
        },
        "children": [
          {
            "transform": null,
            "boundingVolume": {
              "box": [
                45.6395,
                -2.2089,
                -30.1061,
                47.0973,
                0.0,
                0.0,
                0.0,
                -12.4567,
                0.0,
                0.0,
                0.0,
                37.5008
              ]
            },
            "geometricError": 3.0,
            "refine": "ADD",
            "content": {
              "uri": "LOD-1/Mesh-XR-YL.b3dm"
            },
            "children": [
              {
                "transform": null,
                "boundingVolume": {
                  "box": [
                    45.6395,
                    -2.2089,
                    -30.1061,
                    47.0973,
                    0.0,
                    0.0,
                    0.0,
                    -12.4567,
                    0.0,
                    0.0,
                    0.0,
                    37.5008
                  ]
                },
                "geometricError": 0.0,
                "refine": "ADD",
                "content": {
                  "uri": "LOD-0/Mesh-XR-YL.b3dm"
                },
                "children": []
              }
            ]
          }
        ]
      },
      {
        "transform": null,
        "boundingVolume": {
          "box": [
            -47.7663,
            0.128709,
            -29.9984,
            46.3085,
            0.0,
            0.0,
            0.0,
            -9.23776,
            0.0,
            0.0,
            0.0,
            37.3932
          ]
        },
        "geometricError": 9.1,
        "refine": "ADD",
        "content": {
          "uri": "LOD-2/Mesh-XL-YL.b3dm"
        },
        "children": [
          {
            "transform": null,
            "boundingVolume": {
              "box": [
                -47.7663,
                0.067987,
                -29.9984,
                46.3085,
                0.0,
                0.0,
                0.0,
                -9.29848,
                0.0,
                0.0,
                0.0,
                37.3932
              ]
            },
            "geometricError": 3.0,
            "refine": "REPLACE",
            "content": {
              "uri": "LOD-1/Mesh-XL-YL.b3dm"
            },
            "children": [
              {
                "transform": null,
                "boundingVolume": {
                  "box": [
                    -47.7663,
                    0.067987,
                    -29.9984,
                    46.3085,
                    0.0,
                    0.0,
                    0.0,
                    -9.29848,
                    0.0,
                    0.0,
                    0.0,
                    37.3932
                  ]
                },
                "geometricError": 0.0,
                "refine": "ADD",
                "content": {
                  "uri": "LOD-0/Mesh-XL-YL.b3dm"
                },
                "children": []
              }
            ]
          }
        ]
      }
    ]
  }
}""")

            layer = QgsTiledMeshLayer(tmp_file, 'my layer',
                                      'cesiumtiles')
            self.assertTrue(layer.dataProvider().isValid())

            index = layer.dataProvider().index()
            self.assertTrue(index.isValid())

            self.assertEqual(index.rootNode().boundingVolume().box(),
                             QgsOrientedBox3D([-1.45782, 0.265355, 7.44958], [94.1946, 0, 0, 0, -14.9309, 0, 0, 0, 75.0565]))

            # children should not be populated in advance
            self.assertEqual(len(index.rootNode().children()), 0)

            # get all nodes
            root_node = index.getNodes(QgsTiledMeshRequest())
            self.assertFalse(root_node.parentNode())
            self.assertFalse(root_node.contentUri())
            self.assertEqual(root_node.geometricError(), 100.0)
            self.assertEqual(root_node.refinementProcess(), Qgis.TileRefinementProcess.Additive)
            self.assertEqual(root_node.boundingVolume().box(),
                             QgsOrientedBox3D([-1.45782, 0.265355, 7.44958], [94.1946, 0, 0, 0, -14.9309, 0, 0, 0, 75.0565]))

            self.assertEqual(len(root_node.children()), 4)
            self.assertEqual(root_node.children()[0].parentNode(), root_node)
            self.assertEqual(root_node.children()[0].contentUri(), temp_dir + '/LOD-2/Mesh-XR-YR.b3dm')
            self.assertEqual(root_node.children()[0].geometricError(), 9.1)
            self.assertEqual(root_node.children()[0].refinementProcess(), Qgis.TileRefinementProcess.Additive)
            self.assertEqual(root_node.children()[0].boundingVolume().box(),
                             QgsOrientedBox3D([44.4926, -2.87012, 39.2136], [45.9504, 0, 0, 0, -8.28534, 0, 0, 0, 31.8188]))

            self.assertEqual(len(root_node.children()[0].children()), 1)
            self.assertEqual(root_node.children()[0].children()[0].parentNode(), root_node.children()[0])
            self.assertEqual(root_node.children()[0].children()[0].contentUri(), temp_dir + '/LOD-1/Mesh-XR-YR.b3dm')
            self.assertEqual(root_node.children()[0].children()[0].geometricError(), 3)
            self.assertEqual(root_node.children()[0].children()[0].refinementProcess(), Qgis.TileRefinementProcess.Additive)
            self.assertEqual(root_node.children()[0].children()[0].boundingVolume().box(),
                             QgsOrientedBox3D([44.4926, -2.89708, 39.2136], [45.9504, 0, 0, 0, -8.31229, 0, 0, 0, 31.8188]))
            self.assertEqual(len(root_node.children()[0].children()[0].children()), 1)
            self.assertEqual(root_node.children()[0].children()[0].children()[0].parentNode(), root_node.children()[0].children()[0])
            self.assertEqual(root_node.children()[0].children()[0].children()[0].contentUri(), temp_dir + '/LOD-0/Mesh-XR-YR.b3dm')
            self.assertEqual(root_node.children()[0].children()[0].children()[0].geometricError(), 0)
            self.assertEqual(root_node.children()[0].children()[0].children()[0].refinementProcess(), Qgis.TileRefinementProcess.Additive)
            self.assertEqual(root_node.children()[0].children()[0].children()[0].boundingVolume().box(),
                             QgsOrientedBox3D([44.4926, -2.89708, 39.2136], [45.9504, 0, 0, 0, -8.31229, 0, 0, 0, 31.8188]))
            self.assertFalse(
                len(root_node.children()[0].children()[0].children()[0].children()))

            self.assertEqual(root_node.children()[1].parentNode(), root_node)
            self.assertEqual(root_node.children()[1].contentUri(),
                             temp_dir + '/LOD-2/Mesh-XL-YR.b3dm')
            self.assertEqual(root_node.children()[1].geometricError(), 9.1)
            self.assertEqual(root_node.children()[1].refinementProcess(), Qgis.TileRefinementProcess.Additive)
            self.assertEqual(root_node.children()[1].boundingVolume().box(),
                             QgsOrientedBox3D([-48.5551, 5.67839, 44.9504], [47.0973, 0, 0, 0, -9.5179, 0, 0, 0, 37.5556]))

            self.assertEqual(len(root_node.children()[1].children()), 1)
            self.assertEqual(root_node.children()[1].children()[0].parentNode(), root_node.children()[1])
            self.assertEqual(root_node.children()[1].children()[0].contentUri(), temp_dir + '/LOD-1/Mesh-XL-YR.b3dm')
            self.assertEqual(root_node.children()[1].children()[0].geometricError(), 3)
            self.assertEqual(root_node.children()[1].children()[0].refinementProcess(), Qgis.TileRefinementProcess.Additive)
            self.assertEqual(root_node.children()[1].children()[0].boundingVolume().box(),
                             QgsOrientedBox3D([-48.5551, 5.7113, 44.9504], [47.0973, 0, 0, 0, -9.48498, 0, 0, 0, 37.5556]))
            self.assertEqual(len(root_node.children()[1].children()[0].children()), 1)
            self.assertEqual(root_node.children()[1].children()[0].children()[0].parentNode(), root_node.children()[1].children()[0])
            self.assertEqual(root_node.children()[1].children()[0].children()[0].contentUri(), temp_dir + '/LOD-0/Mesh-XL-YR.b3dm')
            self.assertEqual(root_node.children()[1].children()[0].children()[0].geometricError(), 0)
            self.assertEqual(root_node.children()[1].children()[0].children()[0].refinementProcess(), Qgis.TileRefinementProcess.Additive)
            self.assertEqual(root_node.children()[1].children()[0].children()[0].boundingVolume().box(),
                             QgsOrientedBox3D([-48.5551, 5.7113, 44.9504], [47.0973, 0, 0, 0, -9.48498, 0, 0, 0, 37.5556]))
            self.assertFalse(
                len(root_node.children()[1].children()[0].children()[0].children()))

            self.assertEqual(root_node.children()[2].parentNode(), root_node)
            self.assertEqual(root_node.children()[2].contentUri(),
                             temp_dir + '/LOD-2/Mesh-XR-YL.b3dm')
            self.assertEqual(root_node.children()[2].geometricError(), 9.0)
            self.assertEqual(root_node.children()[2].refinementProcess(), Qgis.TileRefinementProcess.Additive)
            self.assertEqual(root_node.children()[2].boundingVolume().box(),
                             QgsOrientedBox3D([45.6395, -2.2089, -30.1061], [47.0973, 0, 0, 0, -12.4567, 0, 0, 0, 37.5008]))
            self.assertEqual(len(root_node.children()[2].children()), 1)
            self.assertEqual(root_node.children()[2].children()[0].parentNode(), root_node.children()[2])
            self.assertEqual(root_node.children()[2].children()[0].contentUri(), temp_dir + '/LOD-1/Mesh-XR-YL.b3dm')
            self.assertEqual(root_node.children()[2].children()[0].geometricError(), 3)
            self.assertEqual(root_node.children()[2].children()[0].refinementProcess(), Qgis.TileRefinementProcess.Additive)
            self.assertEqual(root_node.children()[2].children()[0].boundingVolume().box(),
                             QgsOrientedBox3D([45.6395, -2.2089, -30.1061], [47.0973, 0, 0, 0, -12.4567, 0, 0, 0, 37.5008]))
            self.assertEqual(len(root_node.children()[2].children()[0].children()), 1)
            self.assertEqual(root_node.children()[2].children()[0].children()[0].parentNode(), root_node.children()[2].children()[0])
            self.assertEqual(root_node.children()[2].children()[0].children()[0].contentUri(), temp_dir + '/LOD-0/Mesh-XR-YL.b3dm')
            self.assertEqual(root_node.children()[2].children()[0].children()[0].geometricError(), 0)
            self.assertEqual(root_node.children()[2].children()[0].children()[0].refinementProcess(), Qgis.TileRefinementProcess.Additive)
            self.assertEqual(root_node.children()[2].children()[0].children()[0].boundingVolume().box(),
                             QgsOrientedBox3D([45.6395, -2.2089, -30.1061], [47.0973, 0, 0, 0, -12.4567, 0, 0, 0, 37.5008]))
            self.assertFalse(
                len(root_node.children()[2].children()[0].children()[0].children()))

            self.assertEqual(root_node.children()[3].parentNode(), root_node)
            self.assertEqual(root_node.children()[3].contentUri(),
                             temp_dir + '/LOD-2/Mesh-XL-YL.b3dm')
            self.assertEqual(root_node.children()[3].geometricError(), 9.1)
            self.assertEqual(root_node.children()[3].refinementProcess(), Qgis.TileRefinementProcess.Additive)
            self.assertEqual(root_node.children()[3].boundingVolume().box(),
                             QgsOrientedBox3D([-47.7663, 0.128709, -29.9984], [46.3085, 0, 0, 0, -9.23776, 0, 0, 0, 37.3932]))
            self.assertEqual(len(root_node.children()[3].children()), 1)
            self.assertEqual(root_node.children()[3].children()[0].parentNode(), root_node.children()[3])
            self.assertEqual(root_node.children()[3].children()[0].contentUri(), temp_dir + '/LOD-1/Mesh-XL-YL.b3dm')
            self.assertEqual(root_node.children()[3].children()[0].geometricError(), 3)
            self.assertEqual(root_node.children()[3].children()[0].refinementProcess(), Qgis.TileRefinementProcess.Replacement)
            self.assertEqual(root_node.children()[3].children()[0].boundingVolume().box(),
                             QgsOrientedBox3D([-47.7663, 0.067987, -29.9984], [46.3085, 0, 0, 0, -9.29848, 0, 0, 0, 37.3932]))
            self.assertEqual(len(root_node.children()[3].children()[0].children()), 1)
            self.assertEqual(root_node.children()[3].children()[0].children()[0].parentNode(), root_node.children()[3].children()[0])
            self.assertEqual(root_node.children()[3].children()[0].children()[0].contentUri(), temp_dir + '/LOD-0/Mesh-XL-YL.b3dm')
            self.assertEqual(root_node.children()[3].children()[0].children()[0].geometricError(), 0)
            self.assertEqual(root_node.children()[3].children()[0].children()[0].refinementProcess(), Qgis.TileRefinementProcess.Additive)
            self.assertEqual(root_node.children()[3].children()[0].children()[0].boundingVolume().box(),
                             QgsOrientedBox3D([-47.7663, 0.067987, -29.9984], [46.3085, 0, 0, 0, -9.29848, 0, 0, 0, 37.3932]))
            self.assertFalse(
                len(root_node.children()[3].children()[0].children()[0].children()))
            root_node = index.getNodes(QgsTiledMeshRequest())
            self.assertFalse(root_node.parentNode())
            self.assertFalse(root_node.contentUri())
            self.assertEqual(root_node.geometricError(), 100.0)
            self.assertEqual(root_node.refinementProcess(),
                             Qgis.TileRefinementProcess.Additive)
            self.assertEqual(root_node.boundingVolume().box(),
                             QgsOrientedBox3D([-1.45782, 0.265355, 7.44958],
                                              [94.1946, 0, 0, 0, -14.9309, 0,
                                               0, 0, 75.0565]))

            self.assertEqual(len(root_node.children()), 4)
            self.assertEqual(root_node.children()[0].parentNode(), root_node)
            self.assertEqual(root_node.children()[0].contentUri(),
                             temp_dir + '/LOD-2/Mesh-XR-YR.b3dm')
            self.assertEqual(root_node.children()[0].geometricError(), 9.1)
            self.assertEqual(root_node.children()[0].refinementProcess(),
                             Qgis.TileRefinementProcess.Additive)
            self.assertEqual(root_node.children()[0].boundingVolume().box(),
                             QgsOrientedBox3D([44.4926, -2.87012, 39.2136],
                                              [45.9504, 0, 0, 0, -8.28534, 0,
                                               0, 0, 31.8188]))

            self.assertEqual(len(root_node.children()[0].children()), 1)
            self.assertEqual(
                root_node.children()[0].children()[0].parentNode(),
                root_node.children()[0])
            self.assertEqual(
                root_node.children()[0].children()[0].contentUri(),
                temp_dir + '/LOD-1/Mesh-XR-YR.b3dm')
            self.assertEqual(
                root_node.children()[0].children()[0].geometricError(), 3)
            self.assertEqual(
                root_node.children()[0].children()[0].refinementProcess(),
                Qgis.TileRefinementProcess.Additive)
            self.assertEqual(
                root_node.children()[0].children()[0].boundingVolume().box(),
                QgsOrientedBox3D([44.4926, -2.89708, 39.2136],
                                 [45.9504, 0, 0, 0, -8.31229, 0, 0, 0,
                                  31.8188]))
            self.assertEqual(
                len(root_node.children()[0].children()[0].children()), 1)
            self.assertEqual(root_node.children()[0].children()[0].children()[
                0].parentNode(),
                root_node.children()[0].children()[0])
            self.assertEqual(root_node.children()[0].children()[0].children()[
                0].contentUri(),
                temp_dir + '/LOD-0/Mesh-XR-YR.b3dm')
            self.assertEqual(root_node.children()[0].children()[0].children()[
                0].geometricError(), 0)
            self.assertEqual(root_node.children()[0].children()[0].children()[
                0].refinementProcess(),
                Qgis.TileRefinementProcess.Additive)
            self.assertEqual(root_node.children()[0].children()[0].children()[
                0].boundingVolume().box(),
                QgsOrientedBox3D([44.4926, -2.89708, 39.2136],
                                 [45.9504, 0, 0, 0, -8.31229, 0,
                                  0, 0, 31.8188]))
            self.assertFalse(
                len(root_node.children()[0].children()[0].children()[
                    0].children()))

            self.assertEqual(root_node.children()[1].parentNode(), root_node)
            self.assertEqual(root_node.children()[1].contentUri(),
                             temp_dir + '/LOD-2/Mesh-XL-YR.b3dm')
            self.assertEqual(root_node.children()[1].geometricError(), 9.1)
            self.assertEqual(root_node.children()[1].refinementProcess(),
                             Qgis.TileRefinementProcess.Additive)
            self.assertEqual(root_node.children()[1].boundingVolume().box(),
                             QgsOrientedBox3D([-48.5551, 5.67839, 44.9504],
                                              [47.0973, 0, 0, 0, -9.5179, 0, 0,
                                               0, 37.5556]))

            self.assertEqual(len(root_node.children()[1].children()), 1)
            self.assertEqual(
                root_node.children()[1].children()[0].parentNode(),
                root_node.children()[1])
            self.assertEqual(
                root_node.children()[1].children()[0].contentUri(),
                temp_dir + '/LOD-1/Mesh-XL-YR.b3dm')
            self.assertEqual(
                root_node.children()[1].children()[0].geometricError(), 3)
            self.assertEqual(
                root_node.children()[1].children()[0].refinementProcess(),
                Qgis.TileRefinementProcess.Additive)
            self.assertEqual(
                root_node.children()[1].children()[0].boundingVolume().box(),
                QgsOrientedBox3D([-48.5551, 5.7113, 44.9504],
                                 [47.0973, 0, 0, 0, -9.48498, 0, 0, 0,
                                  37.5556]))
            self.assertEqual(
                len(root_node.children()[1].children()[0].children()), 1)
            self.assertEqual(root_node.children()[1].children()[0].children()[
                0].parentNode(),
                root_node.children()[1].children()[0])
            self.assertEqual(root_node.children()[1].children()[0].children()[
                0].contentUri(),
                temp_dir + '/LOD-0/Mesh-XL-YR.b3dm')
            self.assertEqual(root_node.children()[1].children()[0].children()[
                0].geometricError(), 0)
            self.assertEqual(root_node.children()[1].children()[0].children()[
                0].refinementProcess(),
                Qgis.TileRefinementProcess.Additive)
            self.assertEqual(root_node.children()[1].children()[0].children()[
                0].boundingVolume().box(),
                QgsOrientedBox3D([-48.5551, 5.7113, 44.9504],
                                 [47.0973, 0, 0, 0, -9.48498, 0,
                                  0, 0, 37.5556]))
            self.assertFalse(
                len(root_node.children()[1].children()[0].children()[
                    0].children()))

            self.assertEqual(root_node.children()[2].parentNode(), root_node)
            self.assertEqual(root_node.children()[2].contentUri(),
                             temp_dir + '/LOD-2/Mesh-XR-YL.b3dm')
            self.assertEqual(root_node.children()[2].geometricError(), 9.0)
            self.assertEqual(root_node.children()[2].refinementProcess(),
                             Qgis.TileRefinementProcess.Additive)
            self.assertEqual(root_node.children()[2].boundingVolume().box(),
                             QgsOrientedBox3D([45.6395, -2.2089, -30.1061],
                                              [47.0973, 0, 0, 0, -12.4567, 0,
                                               0, 0, 37.5008]))
            self.assertEqual(len(root_node.children()[2].children()), 1)
            self.assertEqual(
                root_node.children()[2].children()[0].parentNode(),
                root_node.children()[2])
            self.assertEqual(
                root_node.children()[2].children()[0].contentUri(),
                temp_dir + '/LOD-1/Mesh-XR-YL.b3dm')
            self.assertEqual(
                root_node.children()[2].children()[0].geometricError(), 3)
            self.assertEqual(
                root_node.children()[2].children()[0].refinementProcess(),
                Qgis.TileRefinementProcess.Additive)
            self.assertEqual(
                root_node.children()[2].children()[0].boundingVolume().box(),
                QgsOrientedBox3D([45.6395, -2.2089, -30.1061],
                                 [47.0973, 0, 0, 0, -12.4567, 0, 0, 0,
                                  37.5008]))
            self.assertEqual(
                len(root_node.children()[2].children()[0].children()), 1)
            self.assertEqual(root_node.children()[2].children()[0].children()[
                0].parentNode(),
                root_node.children()[2].children()[0])
            self.assertEqual(root_node.children()[2].children()[0].children()[
                0].contentUri(),
                temp_dir + '/LOD-0/Mesh-XR-YL.b3dm')
            self.assertEqual(root_node.children()[2].children()[0].children()[
                0].geometricError(), 0)
            self.assertEqual(root_node.children()[2].children()[0].children()[
                0].refinementProcess(),
                Qgis.TileRefinementProcess.Additive)
            self.assertEqual(root_node.children()[2].children()[0].children()[
                0].boundingVolume().box(),
                QgsOrientedBox3D([45.6395, -2.2089, -30.1061],
                                 [47.0973, 0, 0, 0, -12.4567, 0,
                                  0, 0, 37.5008]))
            self.assertFalse(
                len(root_node.children()[2].children()[0].children()[
                    0].children()))

            self.assertEqual(root_node.children()[3].parentNode(), root_node)
            self.assertEqual(root_node.children()[3].contentUri(),
                             temp_dir + '/LOD-2/Mesh-XL-YL.b3dm')
            self.assertEqual(root_node.children()[3].geometricError(), 9.1)
            self.assertEqual(root_node.children()[3].refinementProcess(),
                             Qgis.TileRefinementProcess.Additive)
            self.assertEqual(root_node.children()[3].boundingVolume().box(),
                             QgsOrientedBox3D([-47.7663, 0.128709, -29.9984],
                                              [46.3085, 0, 0, 0, -9.23776, 0,
                                               0, 0, 37.3932]))
            self.assertEqual(len(root_node.children()[3].children()), 1)
            self.assertEqual(
                root_node.children()[3].children()[0].parentNode(),
                root_node.children()[3])
            self.assertEqual(
                root_node.children()[3].children()[0].contentUri(),
                temp_dir + '/LOD-1/Mesh-XL-YL.b3dm')
            self.assertEqual(
                root_node.children()[3].children()[0].geometricError(), 3)
            self.assertEqual(
                root_node.children()[3].children()[0].refinementProcess(),
                Qgis.TileRefinementProcess.Replacement)
            self.assertEqual(
                root_node.children()[3].children()[0].boundingVolume().box(),
                QgsOrientedBox3D([-47.7663, 0.067987, -29.9984],
                                 [46.3085, 0, 0, 0, -9.29848, 0, 0, 0,
                                  37.3932]))
            self.assertEqual(
                len(root_node.children()[3].children()[0].children()), 1)
            self.assertEqual(root_node.children()[3].children()[0].children()[
                0].parentNode(),
                root_node.children()[3].children()[0])
            self.assertEqual(root_node.children()[3].children()[0].children()[
                0].contentUri(),
                temp_dir + '/LOD-0/Mesh-XL-YL.b3dm')
            self.assertEqual(root_node.children()[3].children()[0].children()[
                0].geometricError(), 0)
            self.assertEqual(root_node.children()[3].children()[0].children()[
                0].refinementProcess(),
                Qgis.TileRefinementProcess.Additive)
            self.assertEqual(root_node.children()[3].children()[0].children()[
                0].boundingVolume().box(),
                QgsOrientedBox3D([-47.7663, 0.067987, -29.9984],
                                 [46.3085, 0, 0, 0, -9.29848, 0,
                                  0, 0, 37.3932]))
            self.assertFalse(
                len(root_node.children()[3].children()[0].children()[
                    0].children()))

            # request with geometric error set
            request = QgsTiledMeshRequest()
            request.setRequiredGeometricError(11)

            root_node = index.getNodes(request)
            self.assertFalse(root_node.parentNode())
            self.assertFalse(root_node.contentUri())
            self.assertEqual(root_node.geometricError(), 100.0)
            self.assertEqual(root_node.refinementProcess(),
                             Qgis.TileRefinementProcess.Additive)
            self.assertEqual(root_node.boundingVolume().box(),
                             QgsOrientedBox3D([-1.45782, 0.265355, 7.44958],
                                              [94.1946, 0, 0, 0, -14.9309, 0,
                                               0, 0, 75.0565]))

            self.assertEqual(len(root_node.children()), 4)
            self.assertEqual(root_node.children()[0].parentNode(), root_node)
            self.assertEqual(root_node.children()[0].contentUri(),
                             temp_dir + '/LOD-2/Mesh-XR-YR.b3dm')
            self.assertEqual(root_node.children()[0].geometricError(), 9.1)
            self.assertEqual(root_node.children()[0].refinementProcess(),
                             Qgis.TileRefinementProcess.Additive)
            self.assertEqual(root_node.children()[0].boundingVolume().box(),
                             QgsOrientedBox3D([44.4926, -2.87012, 39.2136],
                                              [45.9504, 0, 0, 0, -8.28534, 0,
                                               0, 0, 31.8188]))

            self.assertFalse(root_node.children()[0].children())

            self.assertEqual(root_node.children()[1].parentNode(), root_node)
            self.assertEqual(root_node.children()[1].contentUri(),
                             temp_dir + '/LOD-2/Mesh-XL-YR.b3dm')
            self.assertEqual(root_node.children()[1].geometricError(), 9.1)
            self.assertEqual(root_node.children()[1].refinementProcess(),
                             Qgis.TileRefinementProcess.Additive)
            self.assertEqual(root_node.children()[1].boundingVolume().box(),
                             QgsOrientedBox3D([-48.5551, 5.67839, 44.9504],
                                              [47.0973, 0, 0, 0, -9.5179, 0, 0,
                                               0, 37.5556]))

            self.assertFalse(root_node.children()[1].children())

            self.assertEqual(root_node.children()[2].parentNode(), root_node)
            self.assertEqual(root_node.children()[2].contentUri(),
                             temp_dir + '/LOD-2/Mesh-XR-YL.b3dm')
            self.assertEqual(root_node.children()[2].geometricError(), 9.0)
            self.assertEqual(root_node.children()[2].refinementProcess(),
                             Qgis.TileRefinementProcess.Additive)
            self.assertEqual(root_node.children()[2].boundingVolume().box(),
                             QgsOrientedBox3D([45.6395, -2.2089, -30.1061],
                                              [47.0973, 0, 0, 0, -12.4567, 0,
                                               0, 0, 37.5008]))
            self.assertFalse(root_node.children()[2].children())

            self.assertEqual(root_node.children()[3].parentNode(), root_node)
            self.assertEqual(root_node.children()[3].contentUri(),
                             temp_dir + '/LOD-2/Mesh-XL-YL.b3dm')
            self.assertEqual(root_node.children()[3].geometricError(), 9.1)
            self.assertEqual(root_node.children()[3].refinementProcess(),
                             Qgis.TileRefinementProcess.Additive)
            self.assertEqual(root_node.children()[3].boundingVolume().box(),
                             QgsOrientedBox3D([-47.7663, 0.128709, -29.9984],
                                              [46.3085, 0, 0, 0, -9.23776, 0,
                                               0, 0, 37.3932]))
            self.assertFalse(root_node.children()[3].children())


if __name__ == '__main__':
    unittest.main()
