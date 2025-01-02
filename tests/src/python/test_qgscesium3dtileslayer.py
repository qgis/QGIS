"""QGIS Unit tests for QgsCesium3dTilesLayer

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "27/06/2023"
__copyright__ = "Copyright 2023, The QGIS Project"

import os
import tempfile

from qgis.PyQt.QtCore import QUrl
from qgis.core import (
    Qgis,
    QgsTiledSceneLayer,
    QgsCoordinateReferenceSystem,
    QgsOrientedBox3D,
    QgsTiledSceneRequest,
)
from qgis.testing import start_app, unittest

start_app()


class TestQgsCesium3dTilesLayer(unittest.TestCase):
    def test_invalid_source(self):
        layer = QgsTiledSceneLayer("/nope/tileset.json", "my layer", "cesiumtiles")
        self.assertFalse(layer.dataProvider().isValid())

    def test_invalid_json(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            tmp_file = os.path.join(temp_dir, "tileset.json")
            with open(tmp_file, "w", encoding="utf-8") as f:
                f.write(
                    """
{
  "featurecollection": {}
}
"""
                )
            layer = QgsTiledSceneLayer(tmp_file, "my layer", "cesiumtiles")
            self.assertFalse(layer.dataProvider().isValid())
            self.assertEqual(
                layer.error().summary(),
                'JSON is not a valid Cesium 3D Tiles source (does not contain "root" value)',
            )

    def test_source_bounding_volume_region(self):
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

            self.assertEqual(layer.crs(), QgsCoordinateReferenceSystem("EPSG:4979"))
            self.assertEqual(layer.dataProvider().sceneCrs().authid(), "EPSG:4978")

            self.assertAlmostEqual(layer.extent().xMinimum(), -75.61444109, 3)
            self.assertAlmostEqual(layer.extent().xMaximum(), -75.60974751, 3)
            self.assertAlmostEqual(layer.extent().yMinimum(), 40.04072131, 3)
            self.assertAlmostEqual(layer.extent().yMaximum(), 40.044339909, 3)

            self.assertAlmostEqual(
                layer.dataProvider().boundingVolume().box().centerX(),
                -75.612094,
                3,
            )
            self.assertAlmostEqual(
                layer.dataProvider().boundingVolume().box().centerY(),
                40.0425306,
                3,
            )
            self.assertAlmostEqual(
                layer.dataProvider().boundingVolume().box().centerZ(), 34.105, 3
            )
            self.assertAlmostEqual(layer.dataProvider().zRange().lower(), 1.2, 3)
            self.assertAlmostEqual(layer.dataProvider().zRange().upper(), 67.0099, 3)

            # check that version, tileset version, and z range are in html metadata
            self.assertIn("1.1", layer.dataProvider().htmlMetadata())
            self.assertIn("e575c6f1", layer.dataProvider().htmlMetadata())
            self.assertIn("1.2 - 67.01", layer.dataProvider().htmlMetadata())

            # check metadata
            layer.loadDefaultMetadata()
            self.assertEqual(layer.metadata().type(), "dataset")
            self.assertEqual(layer.metadata().identifier(), "e575c6f1")
            self.assertEqual(layer.metadata().crs().authid(), "EPSG:4978")
            self.assertEqual(
                layer.metadata().extent().spatialExtents()[0].extentCrs.authid(),
                "EPSG:4979",
            )
            self.assertAlmostEqual(
                layer.metadata().extent().spatialExtents()[0].bounds.xMinimum(),
                -75.61444,
                3,
            )
            self.assertAlmostEqual(
                layer.metadata().extent().spatialExtents()[0].bounds.xMaximum(),
                -75.609747,
                3,
            )
            self.assertAlmostEqual(
                layer.metadata().extent().spatialExtents()[0].bounds.yMinimum(),
                40.040721,
                3,
            )
            self.assertAlmostEqual(
                layer.metadata().extent().spatialExtents()[0].bounds.yMaximum(),
                40.0443399,
                3,
            )
            self.assertAlmostEqual(
                layer.metadata().extent().spatialExtents()[0].bounds.zMinimum(), 1.2, 3
            )
            self.assertAlmostEqual(
                layer.metadata().extent().spatialExtents()[0].bounds.zMaximum(),
                67.0099999,
                3,
            )

    def test_source_bounding_volume_region_with_transform(self):
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
        -1.3197,
        0.6988,
        -1.3196,
        0.6989,
        1.2,
        67.01
      ]
    },
    "geometricError": 100,
    "refine": "ADD",
    "children": [],
    "transform":[100, 0, 0, 200, 0, 1, 0, 500, 0, 0, 1, 1000, 0, 0, 0, 1]
  }
}"""
                )

            layer = QgsTiledSceneLayer(tmp_file, "my layer", "cesiumtiles")
            self.assertTrue(layer.dataProvider().isValid())

            layer_bounds = layer.dataProvider().boundingVolume().box()
            self.assertAlmostEqual(layer_bounds.centerX(), -75.61037543, 4)
            self.assertAlmostEqual(layer_bounds.centerY(), 40.0411555, 4)
            self.assertAlmostEqual(layer_bounds.centerZ(), 34.1050000, 4)

    def test_source_bounding_volume_box(self):
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
    "boundingVolume": {"box":[-4595750,2698725,-3493318,182,0,0,0,86,0,0,0,20]},
    "geometricError": 100,
    "refine": "ADD",
    "children": []
  }
}"""
                )

            layer = QgsTiledSceneLayer(tmp_file, "my layer", "cesiumtiles")
            self.assertTrue(layer.dataProvider().isValid())

            self.assertEqual(layer.dataProvider().sceneCrs().authid(), "EPSG:4978")
            # layer must advertise as EPSG:4979, as the various QgsMapLayer
            # methods which utilize crs (such as layer extent transformation)
            # are all purely 2D and can't handle the cesium data source z value
            # range in EPSG:4978
            self.assertEqual(layer.dataProvider().crs().authid(), "EPSG:4979")

            self.assertAlmostEqual(
                layer.dataProvider().extent().xMinimum(), 149.575823489, 3
            )
            self.assertAlmostEqual(
                layer.dataProvider().extent().xMaximum(), 149.57939956, 3
            )
            self.assertAlmostEqual(
                layer.dataProvider().extent().yMinimum(), -33.42101266, 3
            )
            self.assertAlmostEqual(
                layer.dataProvider().extent().yMaximum(), -33.41902168, 3
            )

            self.assertAlmostEqual(
                layer.dataProvider().boundingVolume().box().centerX(), -4595750, 1
            )
            self.assertAlmostEqual(
                layer.dataProvider().boundingVolume().box().centerY(), 2698725, 1
            )
            self.assertEqual(
                layer.dataProvider().boundingVolume().box().centerZ(), -3493318.0
            )
            self.assertEqual(
                layer.dataProvider().boundingVolume().box().halfAxes(),
                [182.0, 0.0, 0.0, 0.0, 86.0, 0.0, 0.0, 0.0, 20.0],
            )

            # check that version, tileset version, and z range are in html metadata
            self.assertIn("1.1", layer.dataProvider().htmlMetadata())
            self.assertIn("e575c6f1", layer.dataProvider().htmlMetadata())
            self.assertIn("519.977 - 876.687", layer.dataProvider().htmlMetadata())

    def test_source_bounding_sphere(self):
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
    "boundingVolume": {"sphere":[-4595750.5786738498136401,2698725.1282528499141335,-3493318,1983]},
    "geometricError": 100,
    "refine": "ADD",
    "children": []
  }
}"""
                )

            layer = QgsTiledSceneLayer(tmp_file, "my layer", "cesiumtiles")
            self.assertTrue(layer.dataProvider().isValid())

            self.assertEqual(layer.dataProvider().sceneCrs().authid(), "EPSG:4978")
            # layer must advertise as EPSG:4979, as the various QgsMapLayer
            # methods which utilize crs (such as layer extent transformation)
            # are all purely 2D and can't handle the cesium data source z value
            # range in EPSG:4978
            self.assertEqual(layer.dataProvider().crs().authid(), "EPSG:4979")

            # extent must be in EPSG:4979 to match the layer crs()
            self.assertAlmostEqual(layer.extent().xMinimum(), 149.5484313, 3)
            self.assertAlmostEqual(layer.extent().xMaximum(), 149.60678790, 3)
            self.assertAlmostEqual(layer.extent().yMinimum(), -33.4484168, 3)
            self.assertAlmostEqual(layer.extent().yMaximum(), -33.391621, 3)

            self.assertAlmostEqual(
                layer.dataProvider().boundingVolume().box().centerX(),
                -4595750.5786,
                1,
            )
            self.assertAlmostEqual(
                layer.dataProvider().boundingVolume().box().centerY(),
                2698725.128252,
                1,
            )
            self.assertEqual(
                layer.dataProvider().boundingVolume().box().centerZ(), -3493318.0
            )

            # check that version, tileset version, and z range are in html metadata
            self.assertIn("1.1", layer.dataProvider().htmlMetadata())
            self.assertIn("e575c6f1", layer.dataProvider().htmlMetadata())
            self.assertIn("-2,658.68 - 4,056.37", layer.dataProvider().htmlMetadata())

    def compare_boxes(self, box1: QgsOrientedBox3D, box2: QgsOrientedBox3D) -> bool:
        """
        Compares two QgsOrientedBox3D objects within 4 decimal places
        """
        fail_message = (
            f"QgsOrientedBox3D([{box1.centerX():.4f}, {box1.centerY():.4f}, {box1.centerZ():.4f}], [{box1.halfAxes()[0]:.4f}, {box1.halfAxes()[1]:.4f},{box1.halfAxes()[2]:.4f},{box1.halfAxes()[3]:.4f},{box1.halfAxes()[4]:.4f},{box1.halfAxes()[5]:.4f},{box1.halfAxes()[6]:.4f},{box1.halfAxes()[7]:.4f},{box1.halfAxes()[8]:.4f}])"
            "!="
            f"QgsOrientedBox3D([{box2.centerX():.4f}, {box2.centerY():.4f}, {box2.centerZ():.4f}], [{box2.halfAxes()[0]:.4f}, {box2.halfAxes()[1]:.4f},{box2.halfAxes()[2]:.4f},{box2.halfAxes()[3]:.4f},{box2.halfAxes()[4]:.4f},{box2.halfAxes()[5]:.4f},{box2.halfAxes()[6]:.4f},{box2.halfAxes()[7]:.4f},{box2.halfAxes()[8]:.4f}])"
        )
        self.assertAlmostEqual(box1.centerX(), box2.centerX(), 4, fail_message)
        self.assertAlmostEqual(box1.centerY(), box2.centerY(), 4, fail_message)
        self.assertAlmostEqual(box1.centerZ(), box2.centerZ(), 4, fail_message)
        self.assertAlmostEqual(box1.halfAxes()[0], box2.halfAxes()[0], 4, fail_message)
        self.assertAlmostEqual(box1.halfAxes()[1], box2.halfAxes()[1], 4, fail_message)
        self.assertAlmostEqual(box1.halfAxes()[2], box2.halfAxes()[2], 4, fail_message)
        self.assertAlmostEqual(box1.halfAxes()[3], box2.halfAxes()[3], 4, fail_message)
        self.assertAlmostEqual(box1.halfAxes()[4], box2.halfAxes()[4], 4, fail_message)
        self.assertAlmostEqual(box1.halfAxes()[5], box2.halfAxes()[5], 4, fail_message)
        self.assertAlmostEqual(box1.halfAxes()[6], box2.halfAxes()[6], 4, fail_message)
        self.assertAlmostEqual(box1.halfAxes()[7], box2.halfAxes()[7], 4, fail_message)
        self.assertAlmostEqual(box1.halfAxes()[8], box2.halfAxes()[8], 4, fail_message)

    def test_index(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            tmp_file = os.path.join(temp_dir, "tileset.json")
            with open(tmp_file, "w", encoding="utf-8") as f:
                f.write(
                    """
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
}"""
                )
            layer = QgsTiledSceneLayer(tmp_file, "my layer", "cesiumtiles")
            self.assertTrue(layer.dataProvider().isValid())

            index = layer.dataProvider().index()
            self.assertTrue(index.isValid())

            root_tile = index.rootTile()
            self.assertEqual(
                root_tile.metadata(),
                {"gltfUpAxis": int(Qgis.Axis.Y), "contentFormat": "cesiumtiles"},
            )

            root_node_bounds = root_tile.boundingVolume()
            self.compare_boxes(
                root_node_bounds.box(),
                QgsOrientedBox3D(
                    [-5061039.8923, 2571462.7359, -2824896.6611],
                    [
                        -42.7968,
                        -83.9110,
                        0.0000,
                        -11.8864,
                        6.0624,
                        -6.7001,
                        -30.0040,
                        15.3028,
                        67.0750,
                    ],
                ),
            )

            # children should be populated in advance
            self.assertEqual(index.parentTileId(root_tile.id()), -1)
            self.assertFalse(root_tile.resources())
            self.assertEqual(root_tile.geometricError(), 100.0)
            self.assertEqual(root_tile.baseUrl(), QUrl("file://" + tmp_file))
            self.assertEqual(
                root_tile.refinementProcess(), Qgis.TileRefinementProcess.Additive
            )
            self.compare_boxes(
                root_tile.boundingVolume().box(),
                QgsOrientedBox3D(
                    [-5061039.8923, 2571462.7359, -2824896.6611],
                    [
                        -42.7968,
                        -83.9110,
                        0.0000,
                        -11.8864,
                        6.0624,
                        -6.7001,
                        -30.0040,
                        15.3028,
                        67.0750,
                    ],
                ),
            )

            children = index.childTileIds(root_tile.id())
            self.assertEqual(len(children), 4)
            self.assertEqual(index.parentTileId(children[0]), root_tile.id())
            child_tile0 = index.getTile(children[0])
            self.assertEqual(
                child_tile0.resources(),
                {"content": "file://" + temp_dir + "/LOD-2/Mesh-XR-YR.b3dm"},
            )
            self.assertEqual(child_tile0.geometricError(), 9.1)
            self.assertEqual(child_tile0.baseUrl(), QUrl("file://" + tmp_file))
            self.assertEqual(
                child_tile0.refinementProcess(), Qgis.TileRefinementProcess.Additive
            )
            self.compare_boxes(
                child_tile0.boundingVolume().box(),
                QgsOrientedBox3D(
                    [-5061075.9635, 2571429.5513, -2824869.6818],
                    [
                        -20.8773,
                        -40.9338,
                        0.0000,
                        -6.5959,
                        3.3641,
                        -3.7180,
                        -12.7196,
                        6.4873,
                        28.4352,
                    ],
                ),
            )

            children0 = index.childTileIds(child_tile0.id())
            self.assertEqual(len(children0), 1)
            child_tile00 = index.getTile(children0[0])
            self.assertEqual(index.parentTileId(child_tile00.id()), child_tile0.id())
            self.assertEqual(
                child_tile00.resources(),
                {"content": "file://" + temp_dir + "/LOD-1/Mesh-XR-YR.b3dm"},
            )
            self.assertEqual(child_tile00.geometricError(), 3)
            self.assertEqual(child_tile00.baseUrl(), QUrl("file://" + tmp_file))
            self.assertEqual(
                child_tile00.refinementProcess(), Qgis.TileRefinementProcess.Additive
            )
            self.compare_boxes(
                child_tile00.boundingVolume().box(),
                QgsOrientedBox3D(
                    [-5061075.9850, 2571429.5622, -2824869.6939],
                    [
                        -20.8773,
                        -40.9338,
                        0.0000,
                        -6.6174,
                        3.3750,
                        -3.7301,
                        -12.7196,
                        6.4873,
                        28.4352,
                    ],
                ),
            )
            children00 = index.childTileIds(child_tile00.id())
            self.assertEqual(len(children00), 1)
            child_tile000 = index.getTile(children00[0])
            self.assertEqual(index.parentTileId(child_tile000.id()), child_tile00.id())
            self.assertEqual(
                child_tile000.resources(),
                {"content": "file://" + temp_dir + "/LOD-0/Mesh-XR-YR.b3dm"},
            )
            self.assertEqual(child_tile000.geometricError(), 0)
            self.assertEqual(child_tile000.baseUrl(), QUrl("file://" + tmp_file))
            self.assertEqual(
                child_tile000.refinementProcess(), Qgis.TileRefinementProcess.Additive
            )
            self.compare_boxes(
                child_tile000.boundingVolume().box(),
                QgsOrientedBox3D(
                    [-5061075.9850, 2571429.5622, -2824869.6939],
                    [
                        -20.8773,
                        -40.9338,
                        0.0000,
                        -6.6174,
                        3.3750,
                        -3.7301,
                        -12.7196,
                        6.4873,
                        28.4352,
                    ],
                ),
            )
            self.assertFalse(index.childTileIds(child_tile000.id()))

            child_tile1 = index.getTile(children[1])
            self.assertEqual(index.parentTileId(child_tile1.id()), root_tile.id())
            self.assertEqual(
                child_tile1.resources(),
                {"content": "file://" + temp_dir + "/LOD-2/Mesh-XL-YR.b3dm"},
            )
            self.assertEqual(child_tile1.geometricError(), 9.1)
            self.assertEqual(child_tile1.baseUrl(), QUrl("file://" + tmp_file))
            self.assertEqual(
                child_tile1.refinementProcess(), Qgis.TileRefinementProcess.Additive
            )
            self.compare_boxes(
                child_tile1.boundingVolume().box(),
                QgsOrientedBox3D(
                    [-5061029.1757, 2571510.1393, -2824860.7190],
                    [
                        -21.3984,
                        -41.9555,
                        0.0000,
                        -7.5772,
                        3.8645,
                        -4.2711,
                        -15.0129,
                        7.6570,
                        33.5620,
                    ],
                ),
            )

            children1 = index.childTileIds(child_tile1.id())
            self.assertEqual(len(children1), 1)
            child_tile10 = index.getTile(children1[0])
            self.assertEqual(index.parentTileId(child_tile10.id()), child_tile1.id())
            self.assertEqual(
                child_tile10.resources(),
                {"content": "file://" + temp_dir + "/LOD-1/Mesh-XL-YR.b3dm"},
            )
            self.assertEqual(child_tile10.geometricError(), 3)
            self.assertEqual(child_tile10.baseUrl(), QUrl("file://" + tmp_file))
            self.assertEqual(
                child_tile10.refinementProcess(), Qgis.TileRefinementProcess.Additive
            )
            self.compare_boxes(
                child_tile10.boundingVolume().box(),
                QgsOrientedBox3D(
                    [-5061029.1495, 2571510.1259, -2824860.7042],
                    [
                        -21.3984,
                        -41.9555,
                        0.0000,
                        -7.5510,
                        3.8512,
                        -4.2563,
                        -15.0129,
                        7.6570,
                        33.5620,
                    ],
                ),
            )
            children10 = index.childTileIds(child_tile10.id())
            self.assertEqual(len(children10), 1)
            child_tile100 = index.getTile(children10[0])
            self.assertEqual(index.parentTileId(child_tile100.id()), child_tile10.id())
            self.assertEqual(
                child_tile100.resources(),
                {"content": "file://" + temp_dir + "/LOD-0/Mesh-XL-YR.b3dm"},
            )
            self.assertEqual(child_tile100.geometricError(), 0)
            self.assertEqual(child_tile100.baseUrl(), QUrl("file://" + tmp_file))
            self.assertEqual(
                child_tile100.refinementProcess(), Qgis.TileRefinementProcess.Additive
            )
            self.compare_boxes(
                child_tile100.boundingVolume().box(),
                QgsOrientedBox3D(
                    [-5061029.1495, 2571510.1259, -2824860.7042],
                    [
                        -21.3984,
                        -41.9555,
                        0.0000,
                        -7.5510,
                        3.8512,
                        -4.2563,
                        -15.0129,
                        7.6570,
                        33.5620,
                    ],
                ),
            )
            self.assertFalse(index.childTileIds(child_tile100.id()))

            child_tile2 = index.getTile(children[2])
            self.assertEqual(index.parentTileId(child_tile2.id()), root_tile.id())
            self.assertEqual(
                child_tile2.resources(),
                {"content": "file://" + temp_dir + "/LOD-2/Mesh-XR-YL.b3dm"},
            )
            self.assertEqual(child_tile2.geometricError(), 9.0)
            self.assertEqual(child_tile2.baseUrl(), QUrl("file://" + tmp_file))
            self.assertEqual(
                child_tile2.refinementProcess(), Qgis.TileRefinementProcess.Additive
            )
            self.compare_boxes(
                child_tile2.boundingVolume().box(),
                QgsOrientedBox3D(
                    [-5061048.2475, 2571414.1280, -2824931.3334],
                    [
                        -21.3984,
                        -41.9555,
                        0.0000,
                        -9.9167,
                        5.0578,
                        -5.5899,
                        -14.9910,
                        7.6458,
                        33.5130,
                    ],
                ),
            )
            children2 = index.childTileIds(child_tile2.id())
            self.assertEqual(len(children2), 1)
            child_tile20 = index.getTile(children2[0])
            self.assertEqual(index.parentTileId(child_tile20.id()), child_tile2.id())
            self.assertEqual(
                child_tile20.resources(),
                {"content": "file://" + temp_dir + "/LOD-1/Mesh-XR-YL.b3dm"},
            )
            self.assertEqual(child_tile20.geometricError(), 3)
            self.assertEqual(child_tile20.baseUrl(), QUrl("file://" + tmp_file))
            self.assertEqual(
                child_tile20.refinementProcess(), Qgis.TileRefinementProcess.Additive
            )
            self.compare_boxes(
                child_tile20.boundingVolume().box(),
                QgsOrientedBox3D(
                    [-5061048.2475, 2571414.1280, -2824931.3334],
                    [
                        -21.3984,
                        -41.9555,
                        0.0000,
                        -9.9167,
                        5.0578,
                        -5.5899,
                        -14.9910,
                        7.6458,
                        33.5130,
                    ],
                ),
            )
            children20 = index.childTileIds(child_tile20.id())
            self.assertEqual(len(children20), 1)
            child_tile200 = index.getTile(children20[0])
            self.assertEqual(index.parentTileId(child_tile200.id()), child_tile20.id())
            self.assertEqual(
                child_tile200.resources(),
                {"content": "file://" + temp_dir + "/LOD-0/Mesh-XR-YL.b3dm"},
            )
            self.assertEqual(child_tile200.geometricError(), 0)
            self.assertEqual(child_tile200.baseUrl(), QUrl("file://" + tmp_file))
            self.assertEqual(
                child_tile200.refinementProcess(), Qgis.TileRefinementProcess.Additive
            )
            self.compare_boxes(
                child_tile200.boundingVolume().box(),
                QgsOrientedBox3D(
                    [-5061048.2475, 2571414.1280, -2824931.3334],
                    [
                        -21.3984,
                        -41.9555,
                        0.0000,
                        -9.9167,
                        5.0578,
                        -5.5899,
                        -14.9910,
                        7.6458,
                        33.5130,
                    ],
                ),
            )
            self.assertFalse(index.childTileIds(child_tile200.id()))

            child_tile3 = index.getTile(children[3])
            self.assertEqual(index.parentTileId(child_tile3.id()), root_tile.id())
            self.assertEqual(
                child_tile3.resources(),
                {"content": "file://" + temp_dir + "/LOD-2/Mesh-XL-YL.b3dm"},
            )
            self.assertEqual(child_tile3.geometricError(), 9.1)
            self.assertEqual(child_tile3.baseUrl(), QUrl("file://" + tmp_file))
            self.assertEqual(
                child_tile3.refinementProcess(), Qgis.TileRefinementProcess.Additive
            )
            self.compare_boxes(
                child_tile3.boundingVolume().box(),
                QgsOrientedBox3D(
                    [-5061003.9912, 2571496.4091, -2824930.1882],
                    [
                        -21.0400,
                        -41.2528,
                        0.0000,
                        -7.3541,
                        3.7508,
                        -4.1454,
                        -14.9480,
                        7.6239,
                        33.4168,
                    ],
                ),
            )
            children3 = index.childTileIds(child_tile3.id())
            self.assertEqual(len(children3), 1)
            child_tile30 = index.getTile(children3[0])
            self.assertEqual(index.parentTileId(child_tile30.id()), child_tile3.id())
            self.assertEqual(
                child_tile30.resources(),
                {"content": "file://" + temp_dir + "/LOD-1/Mesh-XL-YL.b3dm"},
            )
            self.assertEqual(child_tile30.geometricError(), 3)
            self.assertEqual(child_tile30.baseUrl(), QUrl("file://" + tmp_file))
            self.assertEqual(
                child_tile30.refinementProcess(), Qgis.TileRefinementProcess.Replacement
            )
            self.compare_boxes(
                child_tile30.boundingVolume().box(),
                QgsOrientedBox3D(
                    [-5061004.0396, 2571496.4338, -2824930.2154],
                    [
                        -21.0400,
                        -41.2528,
                        0.0000,
                        -7.4025,
                        3.7755,
                        -4.1726,
                        -14.9480,
                        7.6239,
                        33.4168,
                    ],
                ),
            )
            children30 = index.childTileIds(child_tile30.id())
            self.assertEqual(len(children30), 1)
            child_tile300 = index.getTile(children30[0])
            self.assertEqual(index.parentTileId(child_tile300.id()), child_tile30.id())
            self.assertEqual(
                child_tile300.resources(),
                {"content": "file://" + temp_dir + "/LOD-0/Mesh-XL-YL.b3dm"},
            )
            self.assertEqual(child_tile300.geometricError(), 0)
            self.assertEqual(child_tile300.baseUrl(), QUrl("file://" + tmp_file))
            self.assertEqual(
                child_tile300.refinementProcess(), Qgis.TileRefinementProcess.Additive
            )
            self.compare_boxes(
                child_tile300.boundingVolume().box(),
                QgsOrientedBox3D(
                    [-5061004.0396, 2571496.4338, -2824930.2154],
                    [
                        -21.0400,
                        -41.2528,
                        0.0000,
                        -7.4025,
                        3.7755,
                        -4.1726,
                        -14.9480,
                        7.6239,
                        33.4168,
                    ],
                ),
            )
            self.assertFalse(index.childTileIds(child_tile300.id()))

            # get nodes
            tile_ids = index.getTiles(QgsTiledSceneRequest())
            self.assertEqual(len(tile_ids), 12)
            tile = index.getTile(tile_ids[11])
            self.assertFalse(tile.resources())
            self.assertEqual(tile.geometricError(), 100.0)
            self.assertEqual(tile.baseUrl(), QUrl("file://" + tmp_file))
            self.assertEqual(
                tile.refinementProcess(), Qgis.TileRefinementProcess.Additive
            )
            self.compare_boxes(
                tile.boundingVolume().box(),
                QgsOrientedBox3D(
                    [-5061039.8923, 2571462.7359, -2824896.6611],
                    [
                        -42.7968,
                        -83.9110,
                        0.0000,
                        -11.8864,
                        6.0624,
                        -6.7001,
                        -30.0040,
                        15.3028,
                        67.0750,
                    ],
                ),
            )

            tile = index.getTile(tile_ids[2])
            self.assertEqual(
                tile.resources(),
                {"content": "file://" + temp_dir + "/LOD-2/Mesh-XR-YR.b3dm"},
            )
            self.assertEqual(tile.geometricError(), 9.1)
            self.assertEqual(tile.baseUrl(), QUrl("file://" + tmp_file))
            self.assertEqual(
                tile.refinementProcess(), Qgis.TileRefinementProcess.Additive
            )
            self.compare_boxes(
                tile.boundingVolume().box(),
                QgsOrientedBox3D(
                    [-5061075.9635, 2571429.5513, -2824869.6818],
                    [
                        -20.8773,
                        -40.9338,
                        0.0000,
                        -6.5959,
                        3.3641,
                        -3.7180,
                        -12.7196,
                        6.4873,
                        28.4352,
                    ],
                ),
            )

            tile = index.getTile(tile_ids[1])
            self.assertEqual(
                tile.resources(),
                {"content": "file://" + temp_dir + "/LOD-1/Mesh-XR-YR.b3dm"},
            )
            self.assertEqual(tile.geometricError(), 3)
            self.assertEqual(tile.baseUrl(), QUrl("file://" + tmp_file))
            self.assertEqual(
                tile.refinementProcess(), Qgis.TileRefinementProcess.Additive
            )
            self.compare_boxes(
                tile.boundingVolume().box(),
                QgsOrientedBox3D(
                    [-5061075.9850, 2571429.5622, -2824869.6939],
                    [
                        -20.8773,
                        -40.9338,
                        0.0000,
                        -6.6174,
                        3.3750,
                        -3.7301,
                        -12.7196,
                        6.4873,
                        28.4352,
                    ],
                ),
            )

            tile = index.getTile(tile_ids[0])
            self.assertEqual(
                tile.resources(),
                {"content": "file://" + temp_dir + "/LOD-0/Mesh-XR-YR.b3dm"},
            )
            self.assertEqual(tile.geometricError(), 0)
            self.assertEqual(tile.baseUrl(), QUrl("file://" + tmp_file))
            self.assertEqual(
                tile.refinementProcess(), Qgis.TileRefinementProcess.Additive
            )
            self.compare_boxes(
                tile.boundingVolume().box(),
                QgsOrientedBox3D(
                    [-5061075.9850, 2571429.5622, -2824869.6939],
                    [
                        -20.8773,
                        -40.9338,
                        0.0000,
                        -6.6174,
                        3.3750,
                        -3.7301,
                        -12.7196,
                        6.4873,
                        28.4352,
                    ],
                ),
            )

            tile = index.getTile(tile_ids[5])
            self.assertEqual(
                tile.resources(),
                {"content": "file://" + temp_dir + "/LOD-2/Mesh-XL-YR.b3dm"},
            )
            self.assertEqual(tile.geometricError(), 9.1)
            self.assertEqual(tile.baseUrl(), QUrl("file://" + tmp_file))
            self.assertEqual(
                tile.refinementProcess(), Qgis.TileRefinementProcess.Additive
            )
            self.compare_boxes(
                tile.boundingVolume().box(),
                QgsOrientedBox3D(
                    [-5061029.1757, 2571510.1393, -2824860.7190],
                    [
                        -21.3984,
                        -41.9555,
                        0.0000,
                        -7.5772,
                        3.8645,
                        -4.2711,
                        -15.0129,
                        7.6570,
                        33.5620,
                    ],
                ),
            )

            tile = index.getTile(tile_ids[4])
            self.assertEqual(
                tile.resources(),
                {"content": "file://" + temp_dir + "/LOD-1/Mesh-XL-YR.b3dm"},
            )
            self.assertEqual(tile.geometricError(), 3)
            self.assertEqual(tile.baseUrl(), QUrl("file://" + tmp_file))
            self.assertEqual(
                tile.refinementProcess(), Qgis.TileRefinementProcess.Additive
            )
            self.compare_boxes(
                tile.boundingVolume().box(),
                QgsOrientedBox3D(
                    [-5061029.1495, 2571510.1259, -2824860.7042],
                    [
                        -21.3984,
                        -41.9555,
                        0.0000,
                        -7.5510,
                        3.8512,
                        -4.2563,
                        -15.0129,
                        7.6570,
                        33.5620,
                    ],
                ),
            )

            tile = index.getTile(tile_ids[3])
            self.assertEqual(
                tile.resources(),
                {"content": "file://" + temp_dir + "/LOD-0/Mesh-XL-YR.b3dm"},
            )
            self.assertEqual(tile.geometricError(), 0)
            self.assertEqual(tile.baseUrl(), QUrl("file://" + tmp_file))
            self.assertEqual(
                tile.refinementProcess(), Qgis.TileRefinementProcess.Additive
            )
            self.compare_boxes(
                tile.boundingVolume().box(),
                QgsOrientedBox3D(
                    [-5061029.1495, 2571510.1259, -2824860.7042],
                    [
                        -21.3984,
                        -41.9555,
                        0.0000,
                        -7.5510,
                        3.8512,
                        -4.2563,
                        -15.0129,
                        7.6570,
                        33.5620,
                    ],
                ),
            )

            tile = index.getTile(tile_ids[8])
            self.assertEqual(
                tile.resources(),
                {"content": "file://" + temp_dir + "/LOD-2/Mesh-XR-YL.b3dm"},
            )
            self.assertEqual(tile.geometricError(), 9.0)
            self.assertEqual(tile.baseUrl(), QUrl("file://" + tmp_file))
            self.assertEqual(
                tile.refinementProcess(), Qgis.TileRefinementProcess.Additive
            )
            self.compare_boxes(
                tile.boundingVolume().box(),
                QgsOrientedBox3D(
                    [-5061048.2475, 2571414.1280, -2824931.3334],
                    [
                        -21.3984,
                        -41.9555,
                        0.0000,
                        -9.9167,
                        5.0578,
                        -5.5899,
                        -14.9910,
                        7.6458,
                        33.5130,
                    ],
                ),
            )

            tile = index.getTile(tile_ids[7])
            self.assertEqual(
                tile.resources(),
                {"content": "file://" + temp_dir + "/LOD-1/Mesh-XR-YL.b3dm"},
            )
            self.assertEqual(tile.geometricError(), 3)
            self.assertEqual(tile.baseUrl(), QUrl("file://" + tmp_file))
            self.assertEqual(
                tile.refinementProcess(), Qgis.TileRefinementProcess.Additive
            )
            self.compare_boxes(
                tile.boundingVolume().box(),
                QgsOrientedBox3D(
                    [-5061048.2475, 2571414.1280, -2824931.3334],
                    [
                        -21.3984,
                        -41.9555,
                        0.0000,
                        -9.9167,
                        5.0578,
                        -5.5899,
                        -14.9910,
                        7.6458,
                        33.5130,
                    ],
                ),
            )

            tile = index.getTile(tile_ids[6])
            self.assertEqual(
                tile.resources(),
                {"content": "file://" + temp_dir + "/LOD-0/Mesh-XR-YL.b3dm"},
            )
            self.assertEqual(tile.geometricError(), 0)
            self.assertEqual(tile.baseUrl(), QUrl("file://" + tmp_file))
            self.assertEqual(
                tile.refinementProcess(), Qgis.TileRefinementProcess.Additive
            )
            self.compare_boxes(
                tile.boundingVolume().box(),
                QgsOrientedBox3D(
                    [-5061048.2475, 2571414.1280, -2824931.3334],
                    [
                        -21.3984,
                        -41.9555,
                        0.0000,
                        -9.9167,
                        5.0578,
                        -5.5899,
                        -14.9910,
                        7.6458,
                        33.5130,
                    ],
                ),
            )

            tile = index.getTile(tile_ids[10])
            self.assertEqual(
                tile.resources(),
                {"content": "file://" + temp_dir + "/LOD-2/Mesh-XL-YL.b3dm"},
            )
            self.assertEqual(tile.geometricError(), 9.1)
            self.assertEqual(tile.baseUrl(), QUrl("file://" + tmp_file))
            self.assertEqual(
                tile.refinementProcess(), Qgis.TileRefinementProcess.Additive
            )
            self.compare_boxes(
                tile.boundingVolume().box(),
                QgsOrientedBox3D(
                    [-5061003.9912, 2571496.4091, -2824930.1882],
                    [
                        -21.0400,
                        -41.2528,
                        0.0000,
                        -7.3541,
                        3.7508,
                        -4.1454,
                        -14.9480,
                        7.6239,
                        33.4168,
                    ],
                ),
            )

            # '/LOD-1/Mesh-XL-YL.b3dm' should not be present -- it has been replaced by children

            tile = index.getTile(tile_ids[9])
            self.assertEqual(
                tile.resources(),
                {"content": "file://" + temp_dir + "/LOD-0/Mesh-XL-YL.b3dm"},
            )
            self.assertEqual(tile.geometricError(), 0)
            self.assertEqual(tile.baseUrl(), QUrl("file://" + tmp_file))
            self.assertEqual(
                tile.refinementProcess(), Qgis.TileRefinementProcess.Additive
            )
            self.compare_boxes(
                tile.boundingVolume().box(),
                QgsOrientedBox3D(
                    [-5061004.0396, 2571496.4338, -2824930.2154],
                    [
                        -21.0400,
                        -41.2528,
                        0.0000,
                        -7.4025,
                        3.7755,
                        -4.1726,
                        -14.9480,
                        7.6239,
                        33.4168,
                    ],
                ),
            )

            # request with geometric error set
            request = QgsTiledSceneRequest()
            request.setRequiredGeometricError(11)

            tile_ids = index.getTiles(request)
            self.assertEqual(len(tile_ids), 5)
            tile = index.getTile(tile_ids[4])
            self.assertFalse(tile.resources())
            self.assertEqual(tile.geometricError(), 100.0)
            self.assertEqual(
                tile.refinementProcess(), Qgis.TileRefinementProcess.Additive
            )
            self.compare_boxes(
                tile.boundingVolume().box(),
                QgsOrientedBox3D(
                    [-5061039.8923, 2571462.7359, -2824896.6611],
                    [
                        -42.7968,
                        -83.9110,
                        0.0000,
                        -11.8864,
                        6.0624,
                        -6.7001,
                        -30.0040,
                        15.3028,
                        67.0750,
                    ],
                ),
            )

            tile = index.getTile(tile_ids[0])
            parent_id = tile_ids[0]
            self.assertEqual(
                tile.resources(),
                {"content": "file://" + temp_dir + "/LOD-2/Mesh-XR-YR.b3dm"},
            )
            self.assertEqual(tile.geometricError(), 9.1)
            self.assertEqual(
                tile.refinementProcess(), Qgis.TileRefinementProcess.Additive
            )
            self.compare_boxes(
                tile.boundingVolume().box(),
                QgsOrientedBox3D(
                    [-5061075.9635, 2571429.5513, -2824869.6818],
                    [
                        -20.8773,
                        -40.9338,
                        0.0000,
                        -6.5959,
                        3.3641,
                        -3.7180,
                        -12.7196,
                        6.4873,
                        28.4352,
                    ],
                ),
            )

            tile = index.getTile(tile_ids[1])
            self.assertEqual(
                tile.resources(),
                {"content": "file://" + temp_dir + "/LOD-2/Mesh-XL-YR.b3dm"},
            )
            self.assertEqual(tile.geometricError(), 9.1)
            self.assertEqual(
                tile.refinementProcess(), Qgis.TileRefinementProcess.Additive
            )
            self.compare_boxes(
                tile.boundingVolume().box(),
                QgsOrientedBox3D(
                    [-5061029.1757, 2571510.1393, -2824860.7190],
                    [
                        -21.3984,
                        -41.9555,
                        0.0000,
                        -7.5772,
                        3.8645,
                        -4.2711,
                        -15.0129,
                        7.6570,
                        33.5620,
                    ],
                ),
            )

            tile = index.getTile(tile_ids[2])
            self.assertEqual(
                tile.resources(),
                {"content": "file://" + temp_dir + "/LOD-2/Mesh-XR-YL.b3dm"},
            )
            self.assertEqual(tile.geometricError(), 9.0)
            self.assertEqual(
                tile.refinementProcess(), Qgis.TileRefinementProcess.Additive
            )
            self.compare_boxes(
                tile.boundingVolume().box(),
                QgsOrientedBox3D(
                    [-5061048.2475, 2571414.1280, -2824931.3334],
                    [
                        -21.3984,
                        -41.9555,
                        0.0000,
                        -9.9167,
                        5.0578,
                        -5.5899,
                        -14.9910,
                        7.6458,
                        33.5130,
                    ],
                ),
            )

            tile = index.getTile(tile_ids[3])
            self.assertEqual(
                tile.resources(),
                {"content": "file://" + temp_dir + "/LOD-2/Mesh-XL-YL.b3dm"},
            )
            self.assertEqual(tile.geometricError(), 9.1)
            self.assertEqual(
                tile.refinementProcess(), Qgis.TileRefinementProcess.Additive
            )
            self.compare_boxes(
                tile.boundingVolume().box(),
                QgsOrientedBox3D(
                    [-5061003.9912, 2571496.4091, -2824930.1882],
                    [
                        -21.0400,
                        -41.2528,
                        0.0000,
                        -7.3541,
                        3.7508,
                        -4.1454,
                        -14.9480,
                        7.6239,
                        33.4168,
                    ],
                ),
            )

            # restrict request to one parent tile
            request.setParentTileId(parent_id)
            tile_ids = index.getTiles(request)
            self.assertEqual(len(tile_ids), 1)
            tile = index.getTile(tile_ids[0])
            self.assertEqual(
                tile.resources(),
                {"content": "file://" + temp_dir + "/LOD-2/Mesh-XR-YR.b3dm"},
            )

    def test_gltf_up_axis(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            tmp_file = os.path.join(temp_dir, "tileset.json")
            with open(tmp_file, "w", encoding="utf-8") as f:
                f.write(
                    """
        {
  "asset": {
    "version": "1.0",
    "gltfUpAxis":"Z"
  },
  "geometricError": 100.0,
  "root": {
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
    ]
  }
}"""
                )
            layer = QgsTiledSceneLayer(tmp_file, "my layer", "cesiumtiles")
            self.assertTrue(layer.dataProvider().isValid())

            index = layer.dataProvider().index()
            self.assertTrue(index.isValid())

            root_tile = index.rootTile()
            self.assertEqual(
                root_tile.metadata(),
                {"gltfUpAxis": int(Qgis.Axis.Z), "contentFormat": "cesiumtiles"},
            )

    def test_large_dataset(self):
        """
        Test a near-global dataset
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            tmp_file = os.path.join(temp_dir, "tileset.json")
            with open(tmp_file, "w", encoding="utf-8") as f:
                f.write(
                    """{
  "asset": {
    "version": "1.0",
    "tilesetVersion": "0"
  },
  "geometricError": 154134.67955991725,
  "root": {
    "geometricError": 77067.33977995862,
    "boundingVolume": {
      "region": [
        -3.1415925942485985,
        -1.4599681618940228,
        3.141545370875028,
        1.4403465997204372,
        -385.0565011513918,
        5967.300616082603
      ]
    },
    "content": null,
    "children": [],
    "refine": "ADD"
  }
}"""
                )
            layer = QgsTiledSceneLayer(tmp_file, "my layer", "cesiumtiles")
            self.assertTrue(layer.dataProvider().isValid())

            index = layer.dataProvider().index()
            self.assertTrue(index.isValid())

            root_tile = index.rootTile()
            # large (near global) datasets should have no bounding volume
            self.assertTrue(root_tile.boundingVolume().box().isNull())

            self.assertTrue(layer.dataProvider().zRange().isInfinite())


if __name__ == "__main__":
    unittest.main()
