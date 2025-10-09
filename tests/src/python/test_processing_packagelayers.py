"""QGIS Unit tests for Processing Package Layers algorithm.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Alessandro Pasotti"
__date__ = "2022-07"
__copyright__ = "Copyright 2022, The QGIS Project"

import os

from osgeo import ogr, osr
from processing.core.Processing import Processing
from processing.gui.AlgorithmExecutor import execute
from qgis.PyQt.QtCore import QCoreApplication, QTemporaryDir
from qgis.analysis import QgsNativeAlgorithms
from qgis.core import (
    QgsApplication,
    QgsProcessingContext,
    QgsProcessingFeedback,
    QgsProject,
    QgsRelation,
    QgsSettings,
    QgsVectorLayer,
    QgsReferencedRectangle,
    QgsCoordinateReferenceSystem,
    QgsRectangle,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class ConsoleFeedBack(QgsProcessingFeedback):
    _errors = []

    def reportError(self, error, fatalError=False):
        print(error)
        self._errors.append(error)

    def pushInfo(self, info):
        print(info)

    def setProgressText(self, info):
        print(info)

    def pushDebugInfo(self, info):
        print(info)


class TestPackageLayers(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("QGIS_TestPyQgsPackageLayers.com")
        QCoreApplication.setApplicationName("QGIS_TestPyQgsPackageLayers")
        QgsSettings().clear()
        Processing.initialize()
        QgsApplication.processingRegistry().addProvider(QgsNativeAlgorithms())
        cls.registry = QgsApplication.instance().processingRegistry()
        cls.tmp_dir = QTemporaryDir()
        cls.temp_path = os.path.join(cls.tmp_dir.path(), "package_layers.gpkg")
        cls.temp_export_path = os.path.join(
            cls.tmp_dir.path(), "package_layers_export.gpkg"
        )

        # Create test DB

        """
        Test data:

        Region 1
            Province 1
                City 1
                City 2
            Province 2
                City 3
        Region 2
            Province 3
            Province 4
                City 4

        XXXXXXXXXXXXXXXXXXXX
        X R1     X         X
        X  O     X         X
        X        X         X
        X        X         X
        X        X         XXXXXXXXXXXXXXXXXXXXX
        X        X       O X R2                X
        X     O  X         X                   X
        X        X         X                   X
        X P1     X P2      X P3                X
        XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
                           X                   X
                           X   O               X
                           X                   X
                           X P4                X
                           XXXXXXXXXXXXXXXXXXXXX
        """
        srs = osr.SpatialReference()
        srs.ImportFromEPSG(2056)

        ds = ogr.GetDriverByName("GPKG").CreateDataSource(cls.temp_path)
        lyr = ds.CreateLayer("region", srs, geom_type=ogr.wkbPolygon)
        lyr.CreateField(ogr.FieldDefn("name", ogr.OFTString))
        f = ogr.Feature(lyr.GetLayerDefn())
        f["name"] = "region one"
        outring = ogr.Geometry(ogr.wkbLinearRing)
        outring.AddPoint_2D(2580000, 1220500)
        outring.AddPoint_2D(2581000, 1220500)
        outring.AddPoint_2D(2581000, 1221500)
        outring.AddPoint_2D(2580000, 1221500)
        outring.AddPoint_2D(2580000, 1220500)
        polygon = ogr.Geometry(ogr.wkbPolygon)
        polygon.AddGeometry(outring)
        f.SetGeometry(polygon)
        lyr.CreateFeature(f)
        f = ogr.Feature(lyr.GetLayerDefn())
        f["name"] = "region two"
        outring = ogr.Geometry(ogr.wkbLinearRing)
        outring.AddPoint_2D(2581000, 1220000)
        outring.AddPoint_2D(2582000, 1220000)
        outring.AddPoint_2D(2582000, 1221000)
        outring.AddPoint_2D(2581000, 1221000)
        outring.AddPoint_2D(2581000, 1220000)
        polygon = ogr.Geometry(ogr.wkbPolygon)
        polygon.AddGeometry(outring)
        f.SetGeometry(polygon)
        lyr.CreateFeature(f)

        lyr = ds.CreateLayer("province", srs, geom_type=ogr.wkbPolygon)
        lyr.CreateField(ogr.FieldDefn("name", ogr.OFTString))
        lyr.CreateField(ogr.FieldDefn("region", ogr.OFTInteger))
        f = ogr.Feature(lyr.GetLayerDefn())
        f["name"] = "province one"
        f["region"] = 1
        outring = ogr.Geometry(ogr.wkbLinearRing)
        outring.AddPoint_2D(2580000, 1220500)
        outring.AddPoint_2D(2580500, 1220500)
        outring.AddPoint_2D(2580500, 1221500)
        outring.AddPoint_2D(2580000, 1221500)
        outring.AddPoint_2D(2580000, 1220500)
        polygon = ogr.Geometry(ogr.wkbPolygon)
        polygon.AddGeometry(outring)
        f.SetGeometry(polygon)
        lyr.CreateFeature(f)
        f = ogr.Feature(lyr.GetLayerDefn())
        f["name"] = "province two"
        f["region"] = 1
        outring = ogr.Geometry(ogr.wkbLinearRing)
        outring.AddPoint_2D(2580500, 1220500)
        outring.AddPoint_2D(2581000, 1220500)
        outring.AddPoint_2D(2581000, 1221500)
        outring.AddPoint_2D(2580500, 1221500)
        outring.AddPoint_2D(2580500, 1220500)
        polygon = ogr.Geometry(ogr.wkbPolygon)
        polygon.AddGeometry(outring)
        f.SetGeometry(polygon)
        lyr.CreateFeature(f)
        f = ogr.Feature(lyr.GetLayerDefn())
        f["name"] = "province three"
        f["region"] = 2
        outring = ogr.Geometry(ogr.wkbLinearRing)
        outring.AddPoint_2D(2581000, 1220000)
        outring.AddPoint_2D(2582000, 1220000)
        outring.AddPoint_2D(2582000, 1220500)
        outring.AddPoint_2D(2581000, 1220500)
        outring.AddPoint_2D(2581000, 1220000)
        polygon = ogr.Geometry(ogr.wkbPolygon)
        polygon.AddGeometry(outring)
        f.SetGeometry(polygon)
        lyr.CreateFeature(f)
        f = ogr.Feature(lyr.GetLayerDefn())
        f["name"] = "province four"
        f["region"] = 2
        outring = ogr.Geometry(ogr.wkbLinearRing)
        outring.AddPoint_2D(2581000, 1220500)
        outring.AddPoint_2D(2582000, 1220500)
        outring.AddPoint_2D(2582000, 1221000)
        outring.AddPoint_2D(2581000, 1221000)
        outring.AddPoint_2D(2581000, 1220500)
        polygon = ogr.Geometry(ogr.wkbPolygon)
        polygon.AddGeometry(outring)
        f.SetGeometry(polygon)
        lyr.CreateFeature(f)

        lyr = ds.CreateLayer("city", srs, geom_type=ogr.wkbPoint)
        lyr.CreateField(ogr.FieldDefn("name", ogr.OFTString))
        lyr.CreateField(ogr.FieldDefn("province", ogr.OFTInteger))
        f = ogr.Feature(lyr.GetLayerDefn())
        f["name"] = "city one"
        f["province"] = 1
        point = ogr.Geometry(ogr.wkbPoint)
        point.AddPoint_2D(2580200, 1221200)
        f.SetGeometry(point)
        lyr.CreateFeature(f)
        f = ogr.Feature(lyr.GetLayerDefn())
        f["name"] = "city two"
        f["province"] = 1
        point = ogr.Geometry(ogr.wkbPoint)
        point.AddPoint_2D(2580400, 1220700)
        f.SetGeometry(point)
        lyr.CreateFeature(f)
        f = ogr.Feature(lyr.GetLayerDefn())
        f["name"] = "city three"
        f["province"] = 2
        point = ogr.Geometry(ogr.wkbPoint)
        point.AddPoint_2D(2580900, 1220900)
        f.SetGeometry(point)
        lyr.CreateFeature(f)
        f = ogr.Feature(lyr.GetLayerDefn())
        f["name"] = "city four"
        f["province"] = 4
        point = ogr.Geometry(ogr.wkbPoint)
        point.AddPoint_2D(2581200, 1220300)
        f.SetGeometry(point)
        lyr.CreateFeature(f)

        outring = None
        polygon = None
        point = None
        f = None
        ds = None

        region = QgsVectorLayer(cls.temp_path + "|layername=region", "region")
        province = QgsVectorLayer(cls.temp_path + "|layername=province", "province")
        city = QgsVectorLayer(cls.temp_path + "|layername=city", "city")

        QgsProject.instance().addMapLayers([region, province, city])

        relMgr = QgsProject.instance().relationManager()

        rel = QgsRelation()
        rel.setId("rel1")
        rel.setName("province -> region")
        rel.setReferencingLayer(province.id())
        rel.setReferencedLayer(region.id())
        rel.addFieldPair("region", "fid")
        assert rel.isValid()

        relMgr.addRelation(rel)

        rel = QgsRelation()
        rel.setId("rel2")
        rel.setName("city -> province")
        rel.setReferencingLayer(city.id())
        rel.setReferencedLayer(province.id())
        rel.addFieldPair("province", "fid")
        assert rel.isValid()

        relMgr.addRelation(rel)

    def tearDown(self):
        super().tearDown()
        os.unlink(self.temp_export_path)

    def test_simple_export(self):
        """Test export with no selected features"""

        alg = self.registry.createAlgorithmById("qgis:package")
        self.assertIsNotNone(alg)

        def _test(parameters):

            feedback = ConsoleFeedBack()
            context = QgsProcessingContext()
            context.setProject(QgsProject.instance())
            # Note: the following returns true also in case of errors ...
            self.assertTrue(execute(alg, parameters, context, feedback))
            # ... so we check the log
            self.assertEqual(feedback._errors, [])

            # Check export
            l = QgsVectorLayer(
                self.temp_export_path + "|layername=province", "province"
            )
            self.assertTrue(l.isValid())
            self.assertEqual(l.featureCount(), 4)

            l = QgsVectorLayer(self.temp_export_path + "|layername=region", "region")
            self.assertTrue(l.isValid())
            self.assertEqual(l.featureCount(), 2)

            l = QgsVectorLayer(self.temp_export_path + "|layername=city", "city")
            self.assertTrue(l.isValid())
            self.assertEqual(l.featureCount(), 4)

        parameters = {
            "EXPORT_RELATED_LAYERS": True,
            "LAYERS": [QgsProject.instance().mapLayersByName("province")[0]],
            "OUTPUT": self.temp_export_path,
            "OVERWRITE": True,
            "SELECTED_FEATURES_ONLY": False,
        }

        # Test province
        _test(parameters)

        # Test region
        parameters["LAYERS"] = [QgsProject.instance().mapLayersByName("region")[0]]
        _test(parameters)

        # Test city
        parameters["LAYERS"] = [QgsProject.instance().mapLayersByName("city")[0]]
        _test(parameters)

    def test_selected_features_export(self):
        """Test export with selected features"""

        alg = self.registry.createAlgorithmById("qgis:package")
        self.assertIsNotNone(alg)

        def _test(parameters, expected_ids):

            feedback = ConsoleFeedBack()
            context = QgsProcessingContext()
            context.setProject(QgsProject.instance())
            # Note: the following returns true also in case of errors ...
            self.assertTrue(execute(alg, parameters, context, feedback))
            # ... so we check the log
            self.assertEqual(feedback._errors, [])

            # Check export
            for layer_name in list(expected_ids.keys()):
                l = QgsVectorLayer(
                    self.temp_export_path + f"|layername={layer_name}", layer_name
                )
                self.assertTrue(l.isValid())
                ids = {l.id() for l in l.getFeatures()}
                self.assertEqual(ids, expected_ids[layer_name], layer_name + str(ids))

        region = QgsProject.instance().mapLayersByName("region")[0]
        province = QgsProject.instance().mapLayersByName("province")[0]
        city = QgsProject.instance().mapLayersByName("city")[0]

        parameters = {
            "EXPORT_RELATED_LAYERS": True,
            "LAYERS": [province],
            "OUTPUT": self.temp_export_path,
            "OVERWRITE": True,
            "SELECTED_FEATURES_ONLY": True,
        }

        # Test province
        province.selectByIds([1])
        _test(parameters, {"region": {1}, "province": {1}, "city": {1, 2}})
        province.selectByIds([])

        # Test region
        parameters["LAYERS"] = [region]
        region.selectByIds([1])
        _test(parameters, {"region": {1}, "province": {1, 2}, "city": {1, 2, 3}})
        region.selectByIds([])

        # Test city
        parameters["LAYERS"] = [city]
        city.selectByIds([3])
        _test(parameters, {"region": {1}, "province": {2}, "city": {3}})
        city.selectByIds([])

        # Test multiple selection
        parameters["LAYERS"] = [city, province]
        city.selectByIds([3])
        province.selectByIds([3])
        _test(parameters, {"region": {1, 2}, "province": {2, 3}, "city": {3}})
        city.selectByIds([])
        province.selectByIds([])

        # Test referencing with selection
        parameters["LAYERS"] = [region]
        region.selectByIds([2])
        _test(parameters, {"region": {2}, "province": {3, 4}, "city": {4}})
        region.selectByIds([])

        # Test referencing with selection, empty city expected not to be exported
        parameters["LAYERS"] = [province]
        province.selectByIds([3])
        _test(parameters, {"region": {2}, "province": {3}})
        province.selectByIds([])

    def test_extent_filter_export(self):
        """Test export with extent"""

        alg = self.registry.createAlgorithmById("qgis:package")
        self.assertIsNotNone(alg)

        def _test(parameters, expected_ids):

            feedback = ConsoleFeedBack()
            context = QgsProcessingContext()
            context.setProject(QgsProject.instance())
            # Note: the following returns true also in case of errors ...
            self.assertTrue(execute(alg, parameters, context, feedback))
            # ... so we check the log
            self.assertEqual(feedback._errors, [])

            # Check export
            for layer_name in list(expected_ids.keys()):
                l = QgsVectorLayer(
                    self.temp_export_path + f"|layername={layer_name}", layer_name
                )
                self.assertTrue(l.isValid())
                ids = {l.id() for l in l.getFeatures()}
                self.assertEqual(ids, expected_ids[layer_name], layer_name + str(ids))

        region = QgsProject.instance().mapLayersByName("region")[0]
        province = QgsProject.instance().mapLayersByName("province")[0]
        city = QgsProject.instance().mapLayersByName("city")[0]

        parameters = {
            "EXPORT_RELATED_LAYERS": False,
            "LAYERS": [province, region, city],
            "OUTPUT": self.temp_export_path,
            "OVERWRITE": True,
            "SELECTED_FEATURES_ONLY": False,
            "EXTENT": None,
        }

        # Test with no extent given
        _test(
            parameters,
            {"region": {1, 2}, "province": {1, 2, 3, 4}, "city": {1, 2, 3, 4}},
        )

        # Test no features intersecting extent
        parameters["EXTENT"] = "2580700,2581500,1221800,1222000 [EPSG:2056]"
        _test(
            parameters,
            {"region": set(), "province": set(), "city": set()},
        )

        # Test some layers with features not intersecting extent
        parameters["EXTENT"] = "2581500,2582000,1220000,1222000 [EPSG:2056]"
        _test(
            parameters,
            {"region": {2}, "province": {3, 4}, "city": set()},
        )

        # Test all features intersecting extent
        parameters["EXTENT"] = "2580000,2582000,1220000,1222000 [EPSG:2056]"
        _test(
            parameters,
            {"region": {1, 2}, "province": {1, 2, 3, 4}, "city": {1, 2, 3, 4}},
        )

        # Test more interesting extent
        parameters["EXTENT"] = "2580700,2581500,1220000,1222000 [EPSG:2056]"
        _test(parameters, {"region": {1, 2}, "province": {2, 3, 4}, "city": {3, 4}})

        # Test extent in another CRS
        parameters["EXTENT"] = "7.184251,7.194712,47.130699,47.148712 [EPSG:4326]"
        _test(parameters, {"region": {1, 2}, "province": {2, 3, 4}, "city": {3, 4}})

        # Test extent with selected features
        parameters["EXTENT"] = "2580700,2581500,1220000,1222000 [EPSG:2056]"
        parameters["SELECTED_FEATURES_ONLY"] = True
        region.selectByIds([2])
        province.selectByIds([1, 2, 4])
        city.selectByIds([2, 3])
        _test(parameters, {"region": {2}, "province": {2, 4}, "city": {3}})
        region.selectByIds([])
        province.selectByIds([])
        city.selectByIds([])

    def test_crs(self):
        """Test export with transformation"""

        alg = self.registry.createAlgorithmById("qgis:package")
        self.assertIsNotNone(alg)

        def _test(parameters, expected_wkts):

            feedback = ConsoleFeedBack()
            context = QgsProcessingContext()
            context.setProject(QgsProject.instance())
            # Note: the following returns true also in case of errors ...
            self.assertTrue(execute(alg, parameters, context, feedback))
            # ... so we check the log
            self.assertEqual(feedback._errors, [])

            # Check export
            for layer_name, wkts in expected_wkts.items():
                l = QgsVectorLayer(
                    self.temp_export_path + f"|layername={layer_name}", layer_name
                )
                self.assertTrue(l.isValid())
                features = {f.id(): f for f in l.getFeatures()}
                self.assertCountEqual(
                    list(features.keys()),
                    list(wkts.keys()),
                    layer_name + str(features.keys()),
                )
                for id, wkt in wkts.items():
                    self.assertEqual(
                        features[id].geometry().asWkt(3), wkt, f"{layer_name}: {id}"
                    )

        region = QgsProject.instance().mapLayersByName("region")[0]
        province = QgsProject.instance().mapLayersByName("province")[0]
        city = QgsProject.instance().mapLayersByName("city")[0]

        parameters = {
            "EXPORT_RELATED_LAYERS": False,
            "LAYERS": [province, region, city],
            "OUTPUT": self.temp_export_path,
            "OVERWRITE": True,
            "SELECTED_FEATURES_ONLY": False,
            "EXTENT": None,
            "CRS": "EPSG:4326",
        }

        # Test with no extent given
        _test(
            parameters,
            {
                "region": {
                    1: "Polygon ((7.175 47.135, 7.188 47.135, 7.188 47.144, 7.175 47.144, 7.175 47.135))",
                    2: "Polygon ((7.188 47.131, 7.201 47.131, 7.201 47.14, 7.188 47.14, 7.188 47.131))",
                },
                "province": {
                    1: "Polygon ((7.175 47.135, 7.182 47.135, 7.182 47.144, 7.175 47.144, 7.175 47.135))",
                    2: "Polygon ((7.182 47.135, 7.188 47.135, 7.188 47.144, 7.182 47.144, 7.182 47.135))",
                    3: "Polygon ((7.188 47.131, 7.201 47.131, 7.201 47.135, 7.188 47.135, 7.188 47.131))",
                    4: "Polygon ((7.188 47.135, 7.201 47.135, 7.201 47.14, 7.188 47.14, 7.188 47.135))",
                },
                "city": {
                    1: "Point (7.178 47.141)",
                    2: "Point (7.18 47.137)",
                    3: "Point (7.187 47.139)",
                    4: "Point (7.191 47.133)",
                },
            },
        )

        # Test with extent
        # Test more interesting extent
        parameters["EXTENT"] = QgsReferencedRectangle(
            QgsRectangle(2580700, 1220000, 2581500, 1222000),
            QgsCoordinateReferenceSystem("EPSG:2056"),
        )
        _test(
            parameters,
            {
                "region": {
                    1: "Polygon ((7.175 47.135, 7.188 47.135, 7.188 47.144, 7.175 47.144, 7.175 47.135))",
                    2: "Polygon ((7.188 47.131, 7.201 47.131, 7.201 47.14, 7.188 47.14, 7.188 47.131))",
                },
                "province": {
                    2: "Polygon ((7.182 47.135, 7.188 47.135, 7.188 47.144, 7.182 47.144, 7.182 47.135))",
                    3: "Polygon ((7.188 47.131, 7.201 47.131, 7.201 47.135, 7.188 47.135, 7.188 47.131))",
                    4: "Polygon ((7.188 47.135, 7.201 47.135, 7.201 47.14, 7.188 47.14, 7.188 47.135))",
                },
                "city": {3: "Point (7.187 47.139)", 4: "Point (7.191 47.133)"},
            },
        )


if __name__ == "__main__":
    unittest.main()
