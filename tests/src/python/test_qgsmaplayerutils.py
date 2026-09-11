"""QGIS Unit tests for QgsMapLayerUtils.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "2021-05"
__copyright__ = "Copyright 2021, The QGIS Project"


import unittest

from qgis.core import (
    Qgis,
    QgsAnnotationLayer,
    QgsCoordinateReferenceSystem,
    QgsCoordinateTransformContext,
    QgsGroupLayer,
    QgsMapLayerType,
    QgsMapLayerUtils,
    QgsNotSupportedException,
    QgsProject,
    QgsRasterLayer,
    QgsVectorLayer,
)
from qgis.testing import QgisTestCase, start_app
from utilities import unitTestDataPath

start_app()


class TestQgsMapLayerUtils(QgisTestCase):
    def testCombinedExtent(self):
        extent = QgsMapLayerUtils.combinedExtent(
            [], QgsCoordinateReferenceSystem(), QgsCoordinateTransformContext()
        )
        self.assertTrue(extent.isEmpty())

        layer1 = QgsVectorLayer(unitTestDataPath() + "/points.shp", "l1")
        self.assertTrue(layer1.isValid())

        # one layer
        extent = QgsMapLayerUtils.combinedExtent(
            [layer1], QgsCoordinateReferenceSystem(), QgsCoordinateTransformContext()
        )
        self.assertEqual(extent.toString(3), "-118.889,22.800 : -83.333,46.872")

        extent = QgsMapLayerUtils.combinedExtent(
            [layer1],
            QgsCoordinateReferenceSystem("EPSG:4326"),
            QgsCoordinateTransformContext(),
        )
        self.assertEqual(extent.toString(3), "-118.889,22.800 : -83.333,46.872")
        extent = QgsMapLayerUtils.combinedExtent(
            [layer1],
            QgsCoordinateReferenceSystem("EPSG:3857"),
            QgsCoordinateTransformContext(),
        )
        self.assertEqual(extent.toString(0), "-13234651,2607875 : -9276624,5921203")

        # two layers
        layer2 = QgsRasterLayer(unitTestDataPath() + "/landsat-f32-b1.tif", "l2")
        self.assertTrue(layer2.isValid())
        extent = QgsMapLayerUtils.combinedExtent(
            [layer1, layer2],
            QgsCoordinateReferenceSystem("EPSG:4326"),
            QgsCoordinateTransformContext(),
        )
        self.assertEqual(extent.toString(3), "-118.889,22.800 : 18.046,46.872")
        extent = QgsMapLayerUtils.combinedExtent(
            [layer2, layer1],
            QgsCoordinateReferenceSystem("EPSG:4326"),
            QgsCoordinateTransformContext(),
        )
        self.assertEqual(extent.toString(3), "-118.889,22.800 : 18.046,46.872")
        extent = QgsMapLayerUtils.combinedExtent(
            [layer1, layer2],
            QgsCoordinateReferenceSystem("EPSG:3857"),
            QgsCoordinateTransformContext(),
        )
        self.assertEqual(extent.toString(0), "-13234651,2607875 : 2008833,5921203")
        extent = QgsMapLayerUtils.combinedExtent(
            [layer2, layer1],
            QgsCoordinateReferenceSystem("EPSG:3857"),
            QgsCoordinateTransformContext(),
        )
        self.assertEqual(extent.toString(0), "-13234651,2607875 : 2008833,5921203")

    def test_layerSourceMatchesPath(self):
        """
        Test QgsMapLayerUtils.layerSourceMatchesPath()
        """
        self.assertFalse(QgsMapLayerUtils.layerSourceMatchesPath(None, ""))
        self.assertFalse(QgsMapLayerUtils.layerSourceMatchesPath(None, "aaaaa"))

        # shapefile
        layer1 = QgsVectorLayer(unitTestDataPath() + "/points.shp", "l1")
        self.assertFalse(QgsMapLayerUtils.layerSourceMatchesPath(layer1, ""))
        self.assertFalse(QgsMapLayerUtils.layerSourceMatchesPath(layer1, "aaaaa"))
        self.assertTrue(
            QgsMapLayerUtils.layerSourceMatchesPath(
                layer1, unitTestDataPath() + "/points.shp"
            )
        )

        # geopackage with layers
        layer1 = QgsVectorLayer(
            unitTestDataPath() + "/mixed_layers.gpkg|layername=lines", "l1"
        )
        self.assertFalse(QgsMapLayerUtils.layerSourceMatchesPath(layer1, ""))
        self.assertFalse(QgsMapLayerUtils.layerSourceMatchesPath(layer1, "aaaaa"))
        self.assertTrue(
            QgsMapLayerUtils.layerSourceMatchesPath(
                layer1, unitTestDataPath() + "/mixed_layers.gpkg"
            )
        )
        layer2 = QgsVectorLayer(
            unitTestDataPath() + "/mixed_layers.gpkg|layername=points", "l1"
        )
        self.assertTrue(
            QgsMapLayerUtils.layerSourceMatchesPath(
                layer2, unitTestDataPath() + "/mixed_layers.gpkg"
            )
        )

        # raster layer from gpkg
        rl = QgsRasterLayer(f"GPKG:{unitTestDataPath()}/mixed_layers.gpkg:band1")
        self.assertFalse(QgsMapLayerUtils.layerSourceMatchesPath(rl, ""))
        self.assertFalse(QgsMapLayerUtils.layerSourceMatchesPath(rl, "aaaaa"))
        self.assertTrue(
            QgsMapLayerUtils.layerSourceMatchesPath(
                rl, unitTestDataPath() + "/mixed_layers.gpkg"
            )
        )

    def test_layerRefersToUri(self):
        """
        Test QgsMapLayerUtils.layerRefersToUri()
        """
        self.assertFalse(QgsMapLayerUtils.layerRefersToUri(None, ""))
        self.assertFalse(QgsMapLayerUtils.layerRefersToUri(None, "aaaaa"))

        # not supported
        layer2 = QgsRasterLayer(unitTestDataPath() + "/landsat-f32-b1.tif", "l2")
        with self.assertRaises(QgsNotSupportedException):
            QgsMapLayerUtils.layerRefersToUri(layer2, "")

        # shapefile
        layer1 = QgsVectorLayer(unitTestDataPath() + "/points.shp", "l1")
        self.assertFalse(QgsMapLayerUtils.layerRefersToUri(layer1, ""))
        self.assertFalse(QgsMapLayerUtils.layerRefersToUri(layer1, "aaaaa"))
        self.assertTrue(
            QgsMapLayerUtils.layerRefersToUri(
                layer1, unitTestDataPath() + "/points.shp"
            )
        )

        # geopackage with layers
        layer1 = QgsVectorLayer(
            unitTestDataPath() + "/mixed_layers.gpkg|layername=lines", "l1"
        )
        self.assertFalse(QgsMapLayerUtils.layerRefersToUri(layer1, ""))
        self.assertFalse(QgsMapLayerUtils.layerRefersToUri(layer1, "aaaaa"))
        self.assertTrue(
            QgsMapLayerUtils.layerRefersToUri(
                layer1,
                unitTestDataPath() + "/mixed_layers.gpkg|layername=points",
                Qgis.SourceHierarchyLevel.Connection,
            )
        )
        self.assertFalse(
            QgsMapLayerUtils.layerRefersToUri(
                layer1,
                unitTestDataPath() + "/mixed_layers.gpkg|layername=points",
                Qgis.SourceHierarchyLevel.Object,
            )
        )
        self.assertTrue(
            QgsMapLayerUtils.layerRefersToUri(
                layer1,
                unitTestDataPath() + "/mixed_layers.gpkg|layername=lines",
                Qgis.SourceHierarchyLevel.Object,
            )
        )

    def test_updateLayerSourcePath(self):
        """
        Test QgsMapLayerUtils.updateLayerSourcePath()
        """
        self.assertFalse(QgsMapLayerUtils.updateLayerSourcePath(None, ""))
        self.assertFalse(QgsMapLayerUtils.updateLayerSourcePath(None, "aaaaa"))

        # shapefile
        layer1 = QgsVectorLayer(unitTestDataPath() + "/points.shp", "l1")
        self.assertTrue(
            QgsMapLayerUtils.updateLayerSourcePath(
                layer1, unitTestDataPath() + "/points22.shp"
            )
        )
        self.assertEqual(layer1.source(), unitTestDataPath() + "/points22.shp")

        # geopackage with layers
        layer1 = QgsVectorLayer(
            unitTestDataPath() + "/mixed_layers.gpkg|layername=lines", "l1"
        )
        self.assertTrue(
            QgsMapLayerUtils.updateLayerSourcePath(
                layer1, unitTestDataPath() + "/mixed_layers22.gpkg"
            )
        )
        self.assertEqual(
            layer1.source(), unitTestDataPath() + "/mixed_layers22.gpkg|layername=lines"
        )
        layer2 = QgsVectorLayer(
            unitTestDataPath() + "/mixed_layers.gpkg|layername=points", "l1"
        )
        self.assertTrue(
            QgsMapLayerUtils.updateLayerSourcePath(
                layer2, unitTestDataPath() + "/mixed_layers22.gpkg"
            )
        )
        self.assertEqual(
            layer2.source(),
            unitTestDataPath() + "/mixed_layers22.gpkg|layername=points",
        )

        # raster layer from gpkg
        rl = QgsRasterLayer(f"GPKG:{unitTestDataPath()}/mixed_layers.gpkg:band1")
        self.assertTrue(
            QgsMapLayerUtils.updateLayerSourcePath(
                rl, unitTestDataPath() + "/mixed_layers22.gpkg"
            )
        )
        self.assertEqual(
            rl.source(), f"GPKG:{unitTestDataPath()}/mixed_layers22.gpkg:band1"
        )

        # a layer from a provider which doesn't use file based paths
        layer = QgsVectorLayer("Point?field=x:string", "my layer", "memory")
        old_source = layer.source()
        self.assertTrue(layer.isValid())
        self.assertFalse(
            QgsMapLayerUtils.updateLayerSourcePath(
                layer, unitTestDataPath() + "/mixed_layers22.gpkg"
            )
        )
        self.assertEqual(layer.source(), old_source)

    def test_sort_layers_by_type(self):
        vl1 = QgsVectorLayer("Point?field=x:string", "vector 1", "memory")
        vl2 = QgsVectorLayer("Point?field=x:string", "vector 2", "memory")
        options = QgsAnnotationLayer.LayerOptions(
            QgsProject.instance().transformContext()
        )
        al1 = QgsAnnotationLayer("annotations 1", options)
        al2 = QgsAnnotationLayer("annotations 2", options)
        rl1 = QgsRasterLayer(
            f"GPKG:{unitTestDataPath()}/mixed_layers.gpkg:band1", "raster 1"
        )
        options = QgsGroupLayer.LayerOptions(QgsProject.instance().transformContext())
        gp1 = QgsGroupLayer("group 1", options)

        self.assertEqual(
            QgsMapLayerUtils.sortLayersByType([vl1, rl1, gp1, vl2, al2, al1], []),
            [vl1, rl1, gp1, vl2, al2, al1],
        )
        self.assertEqual(
            QgsMapLayerUtils.sortLayersByType(
                [vl1, rl1, gp1, vl2, al2, al1], [QgsMapLayerType.VectorLayer]
            ),
            [vl1, vl2, rl1, gp1, al2, al1],
        )
        self.assertEqual(
            QgsMapLayerUtils.sortLayersByType(
                [vl1, rl1, gp1, vl2, al2, al1],
                [QgsMapLayerType.RasterLayer, QgsMapLayerType.VectorLayer],
            ),
            [rl1, vl1, vl2, gp1, al2, al1],
        )
        self.assertEqual(
            QgsMapLayerUtils.sortLayersByType(
                [vl1, rl1, gp1, vl2, al2, al1],
                [QgsMapLayerType.GroupLayer, QgsMapLayerType.VectorLayer],
            ),
            [gp1, vl1, vl2, rl1, al2, al1],
        )
        self.assertEqual(
            QgsMapLayerUtils.sortLayersByType(
                [vl1, rl1, gp1, vl2, al2, al1],
                [
                    QgsMapLayerType.GroupLayer,
                    QgsMapLayerType.VectorLayer,
                    QgsMapLayerType.AnnotationLayer,
                ],
            ),
            [gp1, vl1, vl2, al2, al1, rl1],
        )
        self.assertEqual(
            QgsMapLayerUtils.sortLayersByType(
                [vl1, rl1, gp1, vl2, al2, al1],
                [
                    QgsMapLayerType.GroupLayer,
                    QgsMapLayerType.VectorLayer,
                    QgsMapLayerType.RasterLayer,
                    QgsMapLayerType.AnnotationLayer,
                ],
            ),
            [gp1, vl1, vl2, rl1, al2, al1],
        )
        self.assertEqual(
            QgsMapLayerUtils.sortLayersByType(
                [vl1, rl1, gp1, vl2],
                [
                    QgsMapLayerType.GroupLayer,
                    QgsMapLayerType.VectorLayer,
                    QgsMapLayerType.RasterLayer,
                ],
            ),
            [gp1, vl1, vl2, rl1],
        )
        self.assertEqual(
            QgsMapLayerUtils.sortLayersByType(
                [vl1, rl1, gp1, vl2], [QgsMapLayerType.AnnotationLayer]
            ),
            [vl1, rl1, gp1, vl2],
        )

    def test_launder_layer_name_ascii(self):
        launder = QgsMapLayerUtils.launderLayerName
        ascii_mode = Qgis.LayerNameLaunderingMode.Ascii

        self.assertEqual(launder("abc Def4_a.h%", ascii_mode), "abc_def4_a_h")

        # non-conforming characters are replaced, never dropped: dropping them
        # silently collided distinct names (GH #67248)
        self.assertEqual(launder("Stra\u00dfe", ascii_mode), "stra_e")
        self.assertEqual(launder("Strae", ascii_mode), "strae")
        self.assertNotEqual(
            launder("Stra\u00dfe", ascii_mode), launder("Strae", ascii_mode)
        )

        self.assertEqual(launder("Gro\u00df-Umstadt", ascii_mode), "gro_umstadt")
        self.assertEqual(
            launder("Bezirk K\u00f6nigsberg (Pr.)", ascii_mode), "bezirk_k_nigsberg_pr"
        )

        # runs of underscores are collapsed and trimmed from the ends
        self.assertEqual(launder("  a %% b  ", ascii_mode), "a_b")

    def test_launder_layer_name_unicode(self):
        launder = QgsMapLayerUtils.launderLayerName

        # the default mode preserves letters and digits of any script
        self.assertEqual(launder("Zusammengef\u00fchrt"), "Zusammengef\u00fchrt")
        self.assertEqual(
            launder("Haltestellen Ro\u00dfdorf"), "Haltestellen_Ro\u00dfdorf"
        )
        self.assertEqual(
            launder("\u0414\u043e\u0440\u043e\u0433\u0438"),
            "\u0414\u043e\u0440\u043e\u0433\u0438",
        )
        self.assertEqual(
            launder("Bezirk K\u00f6nigsberg (Pr.)"), "Bezirk_K\u00f6nigsberg_(Pr.)"
        )

        # ... but replaces characters hostile to data source URIs, paths and SQL quoting
        self.assertEqual(launder('a/b\\c|d"e'), "a_b_c_d_e")
        self.assertEqual(launder("a\tb\nc"), "a_b_c")

        # non-ASCII whitespace counts as whitespace and is not left in place
        self.assertEqual(launder("Nicht\u00a0umbrechend"), "Nicht_umbrechend")

        # names are normalized to NFC, so a decomposed source string launders
        # identically to its composed equivalent
        self.assertEqual(launder("O\u0308tzi"), launder("\u00d6tzi"))
        self.assertEqual(launder("O\u0308tzi"), "\u00d6tzi")

    def test_launder_layer_name_default_mode(self):
        # the default argument must match the default of the corresponding setting
        self.assertEqual(
            QgsMapLayerUtils.launderLayerName("Ro\u00dfdorf"),
            QgsMapLayerUtils.launderLayerName(
                "Ro\u00dfdorf", Qgis.LayerNameLaunderingMode.PreserveUnicode
            ),
        )


if __name__ == "__main__":
    unittest.main()
