"""QGIS Unit tests for ESRI I3S tiled scene layer

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Martin Dobias"
__date__ = "03/09/2025"
__copyright__ = "Copyright 2025, The QGIS Project"

import gzip
import os
import tempfile
import zipfile

from qgis.core import (
    Qgis,
    QgsCoordinateReferenceSystem,
    QgsMatrix4x4,
    QgsOrientedBox3D,
    QgsTiledSceneLayer,
    QgsTiledSceneRequest,
)
from qgis.PyQt.QtCore import QUrl
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


def _make_tmp_eslpk_dataset(
    temp_dir, layer_json_str, nodepage_json_str, i3s_version="1.8"
):
    """Creates files needed for a basic "Extracted SLPK" dataset"""

    metadata_file = os.path.join(temp_dir, "metadata.json")
    with open(metadata_file, "w", encoding="utf-8") as f:
        f.write('{ "I3SVersion": "' + i3s_version + '" }')

    layer_file = os.path.join(temp_dir, "3dSceneLayer.json.gz")
    with gzip.open(layer_file, "wt", encoding="utf-8") as f:
        f.write(layer_json_str)

    nodepages_dir = os.path.join(temp_dir, "nodepages")
    os.mkdir(nodepages_dir)
    nodepage_0_file = os.path.join(nodepages_dir, "0.json.gz")
    with gzip.open(nodepage_0_file, "wt", encoding="utf-8") as f:
        f.write(nodepage_json_str)


class TestQgsEsriI3sLayer(unittest.TestCase):
    def test_invalid_source(self):
        layer = QgsTiledSceneLayer("file:///nope", "my layer", "esrii3s")
        self.assertFalse(layer.dataProvider().isValid())

    def test_invalid_json(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            layer_json = """
            {
              "featurecollection": {}
            }
            """
            _make_tmp_eslpk_dataset(temp_dir, layer_json, "")

            layer = QgsTiledSceneLayer(temp_dir, "my layer", "esrii3s")
            self.assertFalse(layer.dataProvider().isValid())
            self.assertEqual(
                layer.error().summary(),
                "Invalid I3S source: missing layer type.",
            )

    def test_old_i3s_version(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            _make_tmp_eslpk_dataset(temp_dir, "", "", "1.6")

            layer = QgsTiledSceneLayer(temp_dir, "my layer", "esrii3s")
            self.assertFalse(layer.dataProvider().isValid())
            self.assertEqual(
                layer.error().summary(),
                "Unsupported I3S version: 1.6",
            )

    def test_valid_global(self):
        """Test using a "global" dataset - i.e. using EPSG:4326"""

        with tempfile.TemporaryDirectory(delete=False) as temp_dir:
            layer_json = """
            {
              "id": 0,
              "layerType": "IntegratedMesh",
              "version": "1111-2222-3333",
              "capabilities": ["View", "Query"],
              "spatialReference": {
                "wkid": 4326,
                "latestWkid": 4326,
                "vcsWkid": 3855,
                "latestVcsWkid": 3855
              },
              "nodePages": {
                "nodesPerPage": 64,
                "lodSelectionMetricType": "maxScreenThresholdSQ"
              }
            }
            """
            nodepage_json = """
            {
              "nodes": [
                {
                  "index": 0,
                  "obb": {
                    "center": [-117.53759594675871, 34.12419117052764, 411.5930244093761],
                    "halfSize": [67.92003, 86.17007, 14.765519],
                    "quaternion": [0.206154981448711, 0.8528643536373323, 0.4652900171822784, 0.11673781661986028]
                  }
                }
              ]
            }
            """
            _make_tmp_eslpk_dataset(temp_dir, layer_json, nodepage_json)

            layer = QgsTiledSceneLayer(temp_dir, "my layer", "esrii3s")
            self.assertTrue(layer.dataProvider().isValid())

            self.assertEqual(layer.crs(), QgsCoordinateReferenceSystem("EPSG:4979"))
            self.assertEqual(layer.dataProvider().sceneCrs().authid(), "EPSG:4978")

            self.assertAlmostEqual(layer.extent().xMinimum(), -117.538339, 3)
            self.assertAlmostEqual(layer.extent().xMaximum(), -117.536852, 3)
            self.assertAlmostEqual(layer.extent().yMinimum(), 34.12340679, 3)
            self.assertAlmostEqual(layer.extent().yMaximum(), 34.12497554, 3)

            self.assertAlmostEqual(
                layer.dataProvider().boundingVolume().box().centerX(),
                -2443825.362862,
                3,
            )
            self.assertAlmostEqual(
                layer.dataProvider().boundingVolume().box().centerY(),
                -4687033.280016,
                3,
            )
            self.assertAlmostEqual(
                layer.dataProvider().boundingVolume().box().centerZ(), 3558089.694926, 3
            )
            self.assertAlmostEqual(layer.dataProvider().zRange().lower(), 394.495, 3)
            self.assertAlmostEqual(layer.dataProvider().zRange().upper(), 428.693, 3)

            # check that version, tileset version, and z range are in html metadata
            self.assertIn("1.8", layer.dataProvider().htmlMetadata())
            self.assertIn("1111-2222-3333", layer.dataProvider().htmlMetadata())
            self.assertIn("394.495 - 428.693", layer.dataProvider().htmlMetadata())

    def test_valid_local(self):
        """Test using a "local" dataset - with projected CRS"""

        with tempfile.TemporaryDirectory(delete=False) as temp_dir:
            layer_json = """
            {
              "id": 0,
              "layerType": "3DObject",
              "version": "9FC7A46A-C550-4E1D-9001-DDCF825B5501",
              "capabilities": ["View", "Query"],
              "spatialReference": {
                "wkid": 102067,
                "latestWkid": 5514
              },
              "nodePages": {
                "nodesPerPage": 64,
                "lodSelectionMetricType": "maxScreenThresholdSQ"
              },
              "fullExtent": {
                "xmin": -503064.4241950555,
                "xmax": -490815.7221285819,
                "ymin": -1209277.75240015844,
                "ymax": -1199847.185454726,
                "spatialReference": {
                  "wkid": 102067,
                  "latestWkid": 5514
                },
                "zmin": 197.438445862085445,
                "zmax": 475.336097820065049
              }
            }
            """
            nodepage_json = """
            {
              "nodes": [
                {
                  "index": 0,
                  "obb" : {
                    "center": [-497670.6852373213, -1204678.5884475496, 295.70377745447672],
                    "halfSize": [6618.03271484375, 227.39961242675781, 4249.75927734375],
                    "quaternion": [0.12047340803502149, 0.70138504165507976, 0.6935764321960135, 0.11178959701679769]
                  }
                }
              ]
            }
            """
            _make_tmp_eslpk_dataset(temp_dir, layer_json, nodepage_json)

            layer = QgsTiledSceneLayer(temp_dir, "my layer", "esrii3s")
            self.assertTrue(layer.dataProvider().isValid())

            self.assertEqual(layer.crs(), QgsCoordinateReferenceSystem("EPSG:5514"))
            self.assertEqual(layer.dataProvider().sceneCrs().authid(), "EPSG:5514")

            self.assertAlmostEqual(layer.extent().xMinimum(), -503064.424195, 3)
            self.assertAlmostEqual(layer.extent().xMaximum(), -490815.722128, 3)
            self.assertAlmostEqual(layer.extent().yMinimum(), -1209277.752400, 3)
            self.assertAlmostEqual(layer.extent().yMaximum(), -1199847.185454, 3)

            self.assertAlmostEqual(
                layer.dataProvider().boundingVolume().box().centerX(),
                -497670.685237,
                3,
            )
            self.assertAlmostEqual(
                layer.dataProvider().boundingVolume().box().centerY(),
                -1204678.588447,
                3,
            )
            self.assertAlmostEqual(
                layer.dataProvider().boundingVolume().box().centerZ(), 295.703777, 3
            )
            self.assertAlmostEqual(layer.dataProvider().zRange().lower(), 197.438445, 3)
            self.assertAlmostEqual(layer.dataProvider().zRange().upper(), 475.336097, 3)

            # check that version, tileset version, and z range are in html metadata
            self.assertIn("1.8", layer.dataProvider().htmlMetadata())
            self.assertIn(
                "9FC7A46A-C550-4E1D-9001-DDCF825B5501",
                layer.dataProvider().htmlMetadata(),
            )
            self.assertIn("197.438 - 475.336", layer.dataProvider().htmlMetadata())

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

    def compare_transforms(
        self, transform1: QgsMatrix4x4, transform2: QgsMatrix4x4
    ) -> bool:
        """
        Compares two QgsMatrix4x4 objects within 4 decimal places
        """
        data1 = transform1.data()
        data2 = transform2.data()
        fail_message = (
            f"QgsMatrix4x4({data1[0]:.4f}, {data1[4]:.4f}, {data1[8]:.4f}, {data1[12]:.4f}, "
            f"{data1[1]:.4f}, {data1[5]:.4f}, {data1[9]:.4f}, {data1[13]:.4f}, "
            f"{data1[2]:.4f}, {data1[6]:.4f}, {data1[10]:.4f}, {data1[14]:.4f}, "
            f"{data1[3]:.4f}, {data1[7]:.4f}, {data1[11]:.4f}, {data1[15]:.4f})"
            "!="
            f"QgsMatrix4x4({data2[0]:.4f}, {data2[4]:.4f}, {data2[8]:.4f}, {data2[12]:.4f}, "
            f"{data2[1]:.4f}, {data2[5]:.4f}, {data2[9]:.4f}, {data2[13]:.4f}, "
            f"{data2[2]:.4f}, {data2[6]:.4f}, {data2[10]:.4f}, {data2[14]:.4f}, "
            f"{data2[3]:.4f}, {data2[7]:.4f}, {data2[11]:.4f}, {data2[15]:.4f})"
        )
        for i in range(16):
            self.assertAlmostEqual(data1[i], data2[i], 4, fail_message)

    def test_index(self):
        # TODO: use Rancho

        with tempfile.TemporaryDirectory(delete=False) as temp_dir:
            layer_json = """
            {
              "id": 0,
              "layerType": "IntegratedMesh",
              "version": "1111-2222-3333",
              "capabilities": ["View", "Query"],
              "spatialReference": {
                "wkid": 4326,
                "latestWkid": 4326,
                "vcsWkid": 3855,
                "latestVcsWkid": 3855
              },
              "nodePages": {
                "nodesPerPage": 64,
                "lodSelectionMetricType": "maxScreenThresholdSQ"
              },

              "materialDefinitions" : [{
                "doubleSided" : true,
                "pbrMetallicRoughness" : {
                  "baseColorTexture" : {
                    "textureSetDefinitionId" : 0
                  },
                  "metallicFactor" : 0
                }
              }],

              "textureSetDefinitions" : [{
                "formats" : [{
                    "name" : "0",
                    "format" : "jpg"
                  },
                  {
                    "name" : "0_0_1",
                    "format" : "dds"
                }]
              }],

              "geometryDefinitions" : [{
                "geometryBuffers" : [{
                  "offset" : 8,
                  "position" : {
                    "type" : "Float32",
                    "component" : 3
                  },
                  "normal" : {
                    "type" : "Float32",
                    "component" : 3
                  },
                  "uv0" : {
                    "type" : "Float32",
                    "component" : 2
                  },
                  "color" : {
                    "type" : "UInt8",
                    "component" : 4
                  },
                  "featureId" : {
                    "type" : "UInt64",
                    "component" : 1,
                    "binding" : "per-feature"
                  },
                  "faceRange" : {
                    "type" : "UInt32",
                    "component" : 2,
                    "binding" : "per-feature"
                  }
                },
                {
                  "compressedAttributes" : {
                    "encoding" : "draco",
                    "attributes" : ["position", "uv0", "feature-index"]
                  }
                }]
              }]

            }
            """
            nodepage_json = """
            {
              "nodes" : [
              {
                "index" : 0,
                "lodThreshold" : 196349.54374999998,
                "obb" : {
                  "center" : [-117.53759594675871, 34.12419117052764, 411.5930244093761],
                  "halfSize" : [67.92003, 86.17007, 14.765519],
                  "quaternion" : [0.206154981448711, 0.8528643536373323, 0.4652900171822784, 0.11673781661986028]
                },
                "children" : [1, 2, 3, 4]
              },
              {
                "index" : 1,
                "parentIndex" : 0,
                "lodThreshold" : 785398.1749999999,
                "obb" : {
                  "center" : [-117.53793932189886, 34.123819641151094, 410.4816717179492],
                  "halfSize" : [34.66071, 43.807545, 8.943618],
                  "quaternion" : [-0.16656774214665032, -0.4782915919434134, 0.8335042596514344, 0.22082343511347818]
                },
                "mesh" : {
                  "material" : {
                    "definition" : 0,
                    "resource" : 16,
                    "texelCountHint" : 16777216
                  },
                  "geometry" : {
                    "definition" : 0,
                    "resource" : 16,
                    "vertexCount" : 60000,
                    "featureCount" : 1
                  }
                },
                "children" : [5, 6, 7, 8]
              },
              {
                "index" : 2,
                "parentIndex" : 0,
                "lodThreshold" : 785398.1749999999,
                "obb" : {
                  "center" : [-117.53724992475546, 34.123821545486685, 410.2505478467792],
                  "halfSize" : [9.228004, 34.19266, 44.047585],
                  "quaternion" : [0.8140355254912096, 0.5257760547630878, -0.216103082756028, 0.11918540640260444]
                },
                "mesh" : {
                    "material" : {
                    "definition" : 0,
                    "resource" : 37,
                    "texelCountHint" : 16777216
                  },
                  "geometry" : {
                    "definition" : 0,
                    "resource" : 37,
                    "vertexCount" : 59997,
                    "featureCount" : 1
                  }
                }
              },
              {
                "index" : 3,
                "parentIndex" : 0,
                "lodThreshold" : 785398.1749999999,
                "obb" : {
                  "center" : [-117.53795948214312, 34.124566053867625, 412.3858846835792],
                  "halfSize" : [43.66336, 34.043926, 10.792544],
                  "quaternion" : [-0.397611945430886, -0.21999843379435893, 0.7659425991790682, -0.4547937606668636]
                },
                "mesh" : {
                  "material" : {
                    "definition" : 0,
                    "resource" : 58,
                    "texelCountHint" : 16777216
                  },
                  "geometry" : {
                    "definition" : 0,
                    "resource" : 58,
                    "vertexCount" : 59997,
                    "featureCount" : 1
                  }
                }
              },
              {
                "index" : 4,
                "parentIndex" : 0,
                "lodThreshold" : 785398.1749999999,
                "obb" : {
                  "center" : [-117.53724435770368, 34.124570367218126, 414.2313824603334],
                  "halfSize" : [34.85811, 43.642643, 10.348813],
                  "quaternion" : [0.44530053235754224, -0.11174393593578129, -0.20527452317737305, 0.8643396894728205]
                },
                "mesh" : {
                  "material" : {
                    "definition" : 0,
                    "resource" : 79,
                    "texelCountHint" : 16777216
                  },
                  "geometry" : {
                    "definition" : 0,
                    "resource" : 79,
                    "vertexCount" : 60000,
                    "featureCount" : 1
                  }
                }
              },
              {
                "index" : 5,
                "parentIndex" : 1,
                "lodThreshold" : 1767145.8937499998,
                "obb" : {
                  "center" : [-117.53812892736529, 34.12366471837075, 410.5171263786033],
                  "halfSize" : [16.485355, 16.677244, 1.7911408],
                  "quaternion" : [0.19655535620659972, 0.8626227061906642, 0.45742106616689565, 0.08952109772300766]
                },
                "mesh" : {
                  "material" : {
                    "definition" : 0,
                    "resource" : 0,
                    "texelCountHint" : 16777216
                  },
                  "geometry" : {
                    "definition" : 0,
                    "resource" : 0,
                    "vertexCount" : 13791,
                    "featureCount" : 1
                  }
                }
              },
              {
                "index" : 6,
                "parentIndex" : 1,
                "lodThreshold" : 1767145.8937499998,
                "obb" : {
                  "center" : [-117.53777576262333, 34.12362478044946, 410.1134819108993],
                  "halfSize" : [21.521395, 3.2239213, 17.154518],
                  "quaternion" : [-0.33920143557919363, -0.6148235345765982, 0.04024529502917849, 0.7108549244816181]
                },
                "mesh" : {
                  "material" : {
                    "definition" : 0,
                    "resource" : 5,
                    "texelCountHint" : 16777216
                  },
                  "geometry" : {
                    "definition" : 0,
                    "resource" : 5,
                    "vertexCount" : 20673,
                    "featureCount" : 1
                  }
                }
              },
              {
                "index" : 7,
                "parentIndex" : 1,
                "lodThreshold" : 1767145.8937499998,
                "obb" : {
                  "center" : [-117.53813350608226, 34.1240024065191, 411.1757797691971],
                  "halfSize" : [21.572693, 2.083206, 16.931368],
                  "quaternion" : [0.7021424175592919, 0.03536027529687852, 0.6126886538878836, 0.36105164421724356]
                },
                "mesh" : {
                  "material" : {
                    "definition" : 0,
                    "resource" : 10,
                    "texelCountHint" : 16777216
                  },
                  "geometry" : {
                    "definition" : 0,
                    "resource" : 10,
                    "vertexCount" : 40731,
                    "featureCount" : 1
                  }
                }
              },
              {
                "index" : 8,
                "parentIndex" : 1,
                "lodThreshold" : 1767145.8937499998,
                "obb" : {
                  "center" : [-117.53777285367366, 34.12400549129491, 412.516752644442],
                  "halfSize" : [18.18257, 6.2166224, 22.5802],
                  "quaternion" : [0.9312857555826259, -0.2857478245548806, -0.04241912677533244, 0.22193611669728053]
                },
                "mesh" : {
                  "material" : {
                    "definition" : 0,
                    "resource" : 15,
                    "texelCountHint" : 16777216
                  },
                  "geometry" : {
                    "definition" : 0,
                    "resource" : 15,
                    "vertexCount" : 38001,
                    "featureCount" : 1
                  }
                }
              }
            ]}
            """
            _make_tmp_eslpk_dataset(temp_dir, layer_json, nodepage_json)

            layer = QgsTiledSceneLayer(temp_dir, "my layer", "esrii3s")
            self.assertTrue(layer.dataProvider().isValid())

            index = layer.dataProvider().index()
            self.assertTrue(index.isValid())

            root_tile = index.rootTile()
            self.assertEqual(root_tile.id(), 0)
            self.assertEqual(
                root_tile.refinementProcess(), Qgis.TileRefinementProcess.Replacement
            )
            self.assertAlmostEqual(root_tile.geometricError(), 5.51488, 3)
            self.assertEqual(root_tile.metadata(), {})
            self.assertEqual(root_tile.resources(), {})

            self.compare_boxes(
                root_tile.boundingVolume().box(),
                QgsOrientedBox3D(
                    [-2443825.3629, -4687033.2800, 3558089.6949],
                    [
                        -60.2956,
                        31.2621,
                        -0.4944,
                        20.9402,
                        41.5349,
                        72.5372,
                        5.7728,
                        11.0081,
                        -7.9698,
                    ],
                ),
            )

            self.compare_transforms(
                root_tile.transform(),
                QgsMatrix4x4(
                    1.0,
                    0.0,
                    0.0,
                    -2443825.3629,
                    0.0,
                    1.0,
                    0.0,
                    -4687033.2800,
                    0.0,
                    0.0,
                    1.0,
                    3558089.6949,
                    0.0,
                    0.0,
                    0.0,
                    1.0,
                ),
            )

            self.assertEqual(index.parentTileId(root_tile.id()), -1)

            self.assertEqual(index.childTileIds(root_tile.id()), [1, 2, 3, 4])
            self.assertEqual(index.parentTileId(1), 0)
            self.assertEqual(index.parentTileId(2), 0)
            self.assertEqual(index.parentTileId(3), 0)
            self.assertEqual(index.parentTileId(4), 0)

            child_tile0 = index.getTile(1)
            self.assertEqual(
                child_tile0.resources(),
                {"content": "file://" + temp_dir + "/nodes/16/geometries/1.bin.gz"},
            )
            self.assertEqual(
                child_tile0.metadata(),
                {
                    "contentFormat": "draco",
                    "gltfUpAxis": int(Qgis.Axis.Z),
                    "material": {
                        "doubleSided": True,
                        "pbrBaseColorFactor": [1.0, 1.0, 1.0, 1.0],
                        "pbrBaseColorTexture": "file://"
                        + temp_dir
                        + "/nodes/16/textures/0.jpg",
                    },
                },
            )
            self.assertAlmostEqual(child_tile0.geometricError(), 1.40184, 3)
            self.assertEqual(
                child_tile0.refinementProcess(), Qgis.TileRefinementProcess.Replacement
            )
            self.compare_boxes(
                child_tile0.boundingVolume().box(),
                QgsOrientedBox3D(
                    [-2443863.7165, -4687038.3195, 3558054.9531],
                    [
                        -29.3571,
                        18.2818,
                        -2.3026,
                        -9.1461,
                        -19.4921,
                        -38.1511,
                        -4.3726,
                        -6.4730,
                        4.3554,
                    ],
                ),
            )

            self.assertEqual(index.childTileIds(child_tile0.id()), [5, 6, 7, 8])
            self.assertEqual(index.parentTileId(5), 1)
            self.assertEqual(index.parentTileId(6), 1)
            self.assertEqual(index.parentTileId(7), 1)
            self.assertEqual(index.parentTileId(8), 1)

            child_tile00 = index.getTile(5)
            self.assertEqual(
                child_tile00.resources(),
                {"content": "file://" + temp_dir + "/nodes/0/geometries/1.bin.gz"},
            )
            self.assertAlmostEqual(child_tile00.geometricError(), 0.35578, 3)
            self.assertEqual(
                child_tile00.refinementProcess(), Qgis.TileRefinementProcess.Replacement
            )
            self.compare_boxes(
                child_tile00.boundingVolume().box(),
                QgsOrientedBox3D(
                    [-2443883.6980, -4687038.8068, 3558040.7461],
                    [
                        -14.9473,
                        6.9404,
                        0.4183,
                        4.2895,
                        8.4097,
                        13.7480,
                        0.5987,
                        1.3505,
                        -1.0129,
                    ],
                ),
            )
            self.assertEqual(index.childTileIds(5), [])

            child_tile1 = index.getTile(2)
            self.assertEqual(
                child_tile1.resources(),
                {"content": "file://" + temp_dir + "/nodes/37/geometries/1.bin.gz"},
            )
            self.assertAlmostEqual(child_tile1.geometricError(), 1.40952, 3)
            self.assertEqual(
                child_tile1.refinementProcess(), Qgis.TileRefinementProcess.Replacement
            )
            self.compare_boxes(
                child_tile1.boundingVolume().box(),
                QgsOrientedBox3D(
                    [-2443807.1775, -4687067.4496, 3558054.9984],
                    [
                        3.2641,
                        7.4238,
                        -4.4032,
                        31.0303,
                        -14.3168,
                        -1.1352,
                        -9.9768,
                        -18.5566,
                        -38.6821,
                    ],
                ),
            )

            self.assertEqual(index.childTileIds(2), [])

            # getTiles() tests

            # request to get tiles at max. resolution
            # (nodes 0 and 1 are not present as they are replaced by children)
            self.assertEqual(
                index.getTiles(QgsTiledSceneRequest()), [5, 6, 7, 8, 2, 3, 4]
            )

            # request with coarse geometric error set
            request = QgsTiledSceneRequest()
            request.setRequiredGeometricError(10)
            self.assertEqual(index.getTiles(request), [0])

            # request with more detailed geometric error set
            request = QgsTiledSceneRequest()
            request.setRequiredGeometricError(5)
            self.assertEqual(index.getTiles(request), [1, 2, 3, 4])

            # restrict request to one parent tile
            request = QgsTiledSceneRequest()
            request.setParentTileId(1)
            self.assertEqual(index.getTiles(request), [5, 6, 7, 8])

    def test_slpk_single_node(self):
        """Test loading from a single node SLPK file"""
        slpk_path = os.path.join(TEST_DATA_DIR, "i3s/single_node.slpk")
        layer = QgsTiledSceneLayer(slpk_path, "my layer", "esrii3s")
        self.assertTrue(layer.dataProvider().isValid())

        self.assertEqual(layer.crs(), QgsCoordinateReferenceSystem("EPSG:5514"))
        self.assertEqual(layer.dataProvider().sceneCrs().authid(), "EPSG:5514")

        self.assertAlmostEqual(layer.extent().xMinimum(), -503064.424195, 3)
        self.assertAlmostEqual(layer.extent().xMaximum(), -490815.722128, 3)
        self.assertAlmostEqual(layer.extent().yMinimum(), -1209277.752400, 3)
        self.assertAlmostEqual(layer.extent().yMaximum(), -1199847.185454, 3)

        self.assertAlmostEqual(
            layer.dataProvider().boundingVolume().box().centerX(),
            -497670.685237,
            3,
        )
        self.assertAlmostEqual(
            layer.dataProvider().boundingVolume().box().centerY(),
            -1204678.588447,
            3,
        )
        self.assertAlmostEqual(
            layer.dataProvider().boundingVolume().box().centerZ(), 295.703777, 3
        )
        self.assertAlmostEqual(layer.dataProvider().zRange().lower(), 197.438445, 3)
        self.assertAlmostEqual(layer.dataProvider().zRange().upper(), 475.336097, 3)

        # check that version, tileset version, and z range are in html metadata
        self.assertIn("1.8", layer.dataProvider().htmlMetadata())
        self.assertIn(
            "9FC7A46A-C550-4E1D-9001-DDCF825B5501",
            layer.dataProvider().htmlMetadata(),
        )
        self.assertIn("197.438 - 475.336", layer.dataProvider().htmlMetadata())


if __name__ == "__main__":
    unittest.main()
