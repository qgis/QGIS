# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsVirtualLayerProvider

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Hugo Mercier'
__date__ = '26/11/2015'
__copyright__ = 'Copyright 2015, The QGIS Project'

import qgis  # NOQA
import os

from qgis.core import (QgsVectorLayer,
                       QgsField,
                       QgsFeature,
                       QgsFeatureRequest,
                       QgsGeometry,
                       QgsRectangle,
                       QgsVirtualLayerDefinition,
                       QgsVirtualLayerDefinitionUtils,
                       QgsWkbTypes,
                       QgsProject,
                       QgsVectorLayerJoinInfo,
                       QgsVectorFileWriter,
                       QgsVirtualLayerDefinitionUtils
                       )

from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

from providertestbase import ProviderTestCase
from qgis.PyQt.QtCore import QUrl, QVariant, QTemporaryDir

from qgis.utils import spatialite_connect

import tempfile

# Convenience instances in case you may need them
start_app()
TEST_DATA_DIR = unitTestDataPath()


def toPercent(s):
    return bytes(QUrl.toPercentEncoding(s)).decode()


class TestQgsVirtualLayerProvider(unittest.TestCase, ProviderTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        # Create the layer for the common provider tests
        shp = os.path.join(TEST_DATA_DIR, 'provider/shapefile.shp')
        d = QgsVirtualLayerDefinition()
        d.addSource("vtab1", shp, "ogr")
        d.setUid("pk")
        cls.vl = QgsVectorLayer(d.toString(), 'test', 'virtual')
        assert (cls.vl.isValid())
        cls.source = cls.vl.dataProvider()

        shp_poly = os.path.join(TEST_DATA_DIR, 'provider/shapefile_poly.shp')
        d = QgsVirtualLayerDefinition()
        d.addSource("vtab2", shp_poly, "ogr")
        d.setUid("pk")
        cls.poly_vl = QgsVectorLayer(d.toString(), 'test_poly', 'virtual')
        assert (cls.poly_vl.isValid())
        cls.poly_provider = cls.poly_vl.dataProvider()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        del(cls.vl)
        del(cls.poly_vl)

    def treat_datetime_as_string(self):
        return True

    def treat_date_as_string(self):
        return True

    def treat_time_as_string(self):
        return True

    def setUp(self):
        """Run before each test."""
        self.testDataDir = unitTestDataPath()
        print("****************************************************")
        print(("In method", self._testMethodName))
        print("****************************************************")
        pass

    def testGetFeaturesThreadSafety(self):
        # provider does not work with this test - sqlite mutex prevents
        # execution
        pass

    def tearDown(self):
        """Run after each test."""
        pass

    def test_filterfid_crossjoin(self):
        l0 = QgsVectorLayer(os.path.join(self.testDataDir, "france_parts.shp"), "france_parts", "ogr")
        self.assertTrue(l0.isValid())
        QgsProject.instance().addMapLayer(l0)

        l1 = QgsVectorLayer(os.path.join(self.testDataDir, "points.shp"), "points", "ogr")
        self.assertTrue(l1.isValid())
        QgsProject.instance().addMapLayer(l1)

        # cross join
        query = toPercent("SELECT * FROM france_parts,points")
        vl = QgsVectorLayer("?query=%s" % query, "tt", "virtual")

        self.assertEqual(vl.featureCount(), l0.featureCount() * l1.featureCount())

        # test with FilterFid requests
        f = next(vl.getFeatures(QgsFeatureRequest().setFilterFid(0)))
        idx = f.fields().indexOf('Class')
        self.assertEqual(f.id(), 0)
        self.assertEqual(f.attributes()[idx], 'Jet')

        f = next(vl.getFeatures(QgsFeatureRequest().setFilterFid(5)))
        self.assertEqual(f.id(), 5)
        self.assertEqual(f.attributes()[idx], 'Biplane')

        # test with FilterFid requests
        fit = vl.getFeatures(QgsFeatureRequest().setFilterFids([0, 3, 5]))

        f = next(fit)
        self.assertEqual(f.id(), 0)
        self.assertEqual(f.attributes()[idx], 'Jet')

        f = next(fit)
        self.assertEqual(f.id(), 3)
        self.assertEqual(f.attributes()[idx], 'Jet')

        f = next(fit)
        self.assertEqual(f.id(), 5)
        self.assertEqual(f.attributes()[idx], 'Biplane')

    def test_CsvNoGeometry(self):
        l1 = QgsVectorLayer(QUrl.fromLocalFile(os.path.join(self.testDataDir,
                                                            "delimitedtext/test.csv")).toString() + "?type=csv&geomType=none&subsetIndex=no&watchFile=no",
                            "test", "delimitedtext", QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l1.isValid(), True)
        QgsProject.instance().addMapLayer(l1)

        l2 = QgsVectorLayer("?layer_ref=" + l1.id(), "vtab", "virtual", QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l2.isValid(), True)

        QgsProject.instance().removeMapLayer(l1.id())

    def test_source_escaping(self):
        # the source contains ':'
        source = QUrl.fromLocalFile(os.path.join(self.testDataDir,
                                                 "delimitedtext/test.csv")).toString() + "?type=csv&geomType=none&subsetIndex=no&watchFile=no"
        d = QgsVirtualLayerDefinition()
        d.addSource("t", source, "delimitedtext")
        l = QgsVectorLayer(d.toString(), "vtab", "virtual", QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l.isValid(), True)

    def test_source_escaping2(self):
        def create_test_db(dbfile):
            if os.path.exists(dbfile):
                os.remove(dbfile)
            con = spatialite_connect(dbfile)
            cur = con.cursor()
            cur.execute("SELECT InitSpatialMetadata(1)")
            cur.execute("CREATE TABLE test (id INTEGER, name TEXT)")
            cur.execute("SELECT AddGeometryColumn('test', 'geometry', 4326, 'POINT', 'XY')")
            sql = "INSERT INTO test (id, name, geometry) "
            sql += "VALUES (1, 'toto',GeomFromText('POINT(0 0)',4326))"
            cur.execute(sql)
            con.close()

        # the source contains ',' and single quotes
        fn = os.path.join(tempfile.gettempdir(), "test,.db")
        create_test_db(fn)
        source = "dbname='%s' table=\"test\" (geometry) sql=" % fn
        d = QgsVirtualLayerDefinition()
        d.addSource("t", source, "spatialite")
        l = QgsVectorLayer(d.toString(), "vtab", "virtual", QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l.isValid(), True)

        # the source contains ':' and single quotes
        fn = os.path.join(tempfile.gettempdir(), "test:.db")
        create_test_db(fn)
        source = "dbname='%s' table=\"test\" (geometry) sql=" % fn
        d = QgsVirtualLayerDefinition()
        d.addSource("t", source, "spatialite")
        l = QgsVectorLayer(d.toString(), "vtab", "virtual", QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l.isValid(), True)

    def test_DynamicGeometry(self):
        l1 = QgsVectorLayer(QUrl.fromLocalFile(os.path.join(self.testDataDir,
                                                            "delimitedtext/testextpt.txt")).toString() + "?type=csv&delimiter=%7C&geomType=none&subsetIndex=no&watchFile=no",
                            "test", "delimitedtext", QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l1.isValid(), True)
        QgsProject.instance().addMapLayer(l1)

        query = toPercent("select *,makepoint(x,y) as geom from vtab1")
        l2 = QgsVectorLayer("?layer_ref=%s&query=%s&geometry=geom:point:0&uid=id" % (l1.id(), query), "vtab", "virtual",
                            QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l2.isValid(), True)

        QgsProject.instance().removeMapLayer(l1)

    def test_ShapefileWithGeometry(self):
        l1 = QgsVectorLayer(os.path.join(self.testDataDir, "france_parts.shp"), "france_parts", "ogr",
                            QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l1.isValid(), True)
        QgsProject.instance().addMapLayer(l1)

        # use a temporary file
        l2 = QgsVectorLayer("?layer_ref=" + l1.id(), "vtab", "virtual", QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l2.isValid(), True)

        l2 = QgsVectorLayer("?layer_ref=%s:nn" % l1.id(), "vtab", "virtual", QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l2.isValid(), True)

        QgsProject.instance().removeMapLayer(l1.id())

    def test_Query(self):
        l1 = QgsVectorLayer(os.path.join(self.testDataDir, "france_parts.shp"), "france_parts", "ogr",
                            QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l1.isValid(), True)
        QgsProject.instance().addMapLayer(l1)
        ref_sum = sum(f.attributes()[0] for f in l1.getFeatures())

        query = toPercent("SELECT * FROM vtab1")
        l2 = QgsVectorLayer("?layer_ref=%s&geometry=geometry:3:4326&query=%s&uid=OBJECTID" % (l1.id(), query), "vtab",
                            "virtual", QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l2.isValid(), True)
        self.assertEqual(l2.dataProvider().wkbType(), 3)

        ref_sum2 = sum(f.attributes()[0] for f in l2.getFeatures())
        ref_sum3 = sum(f.id() for f in l2.getFeatures())
        # check we have the same rows
        self.assertEqual(ref_sum, ref_sum2)
        # check the id is OK
        self.assertEqual(ref_sum, ref_sum3)

        # the same, without specifying the geometry column name
        l2 = QgsVectorLayer("?layer_ref=%s&query=%s&uid=OBJECTID" % (l1.id(), query), "vtab", "virtual",
                            QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l2.isValid(), True)
        self.assertEqual(l2.dataProvider().wkbType(), 6)
        ref_sum2 = sum(f.attributes()[0] for f in l2.getFeatures())
        ref_sum3 = sum(f.id() for f in l2.getFeatures())
        # check we have the same rows
        self.assertEqual(ref_sum, ref_sum2)
        # check the id is OK
        self.assertEqual(ref_sum, ref_sum3)

        # with two geometry columns
        query = toPercent("SELECT *,geometry as geom FROM vtab1")
        l2 = QgsVectorLayer("?layer_ref=%s&query=%s&uid=OBJECTID&geometry=geom:3:4326" % (l1.id(), query), "vtab",
                            "virtual", QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l2.isValid(), True)
        self.assertEqual(l2.dataProvider().wkbType(), 3)
        ref_sum2 = sum(f.attributes()[0] for f in l2.getFeatures())
        ref_sum3 = sum(f.id() for f in l2.getFeatures())
        # check we have the same rows
        self.assertEqual(ref_sum, ref_sum2)
        # check the id is OK
        self.assertEqual(ref_sum, ref_sum3)

        # with two geometry columns, but no geometry column specified (will take the first)
        l2 = QgsVectorLayer("?layer_ref=%s&query=%s&uid=OBJECTID" % (l1.id(), query), "vtab", "virtual",
                            QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l2.isValid(), True)
        self.assertEqual(l2.dataProvider().wkbType(), 6)
        ref_sum2 = sum(f.attributes()[0] for f in l2.getFeatures())
        ref_sum3 = sum(f.id() for f in l2.getFeatures())
        # check we have the same rows
        self.assertEqual(ref_sum, ref_sum2)
        # check the id is OK
        self.assertEqual(ref_sum, ref_sum3)

        # the same, without geometry
        query = toPercent("SELECT * FROM ww")
        l2 = QgsVectorLayer("?layer_ref=%s:ww&query=%s&uid=ObJeCtId&nogeometry" % (l1.id(), query), "vtab", "virtual",
                            QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l2.isValid(), True)
        self.assertEqual(l2.dataProvider().wkbType(), 100)  # NoGeometry
        ref_sum2 = sum(f.attributes()[0] for f in l2.getFeatures())
        ref_sum3 = sum(f.id() for f in l2.getFeatures())
        self.assertEqual(ref_sum, ref_sum2)
        self.assertEqual(ref_sum, ref_sum3)

        # check that it fails when a query has a wrong geometry column
        l2 = QgsVectorLayer("?layer_ref=%s&query=%s&geometry=geo" % (l1.id(), query), "vtab", "virtual",
                            QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l2.isValid(), False)

        QgsProject.instance().removeMapLayer(l1.id())

    def test_QueryUrlEncoding(self):
        l1 = QgsVectorLayer(os.path.join(self.testDataDir, "france_parts.shp"), "france_parts", "ogr",
                            QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l1.isValid(), True)
        QgsProject.instance().addMapLayer(l1)

        query = toPercent("SELECT * FROM vtab1")
        l2 = QgsVectorLayer("?layer_ref=%s&query=%s&uid=ObjectId&nogeometry" % (l1.id(), query), "vtab", "virtual",
                            QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l2.isValid(), True)

        QgsProject.instance().removeMapLayer(l1.id())

    def test_QueryTableName(self):
        l1 = QgsVectorLayer(os.path.join(self.testDataDir, "france_parts.shp"), "france_parts", "ogr",
                            QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l1.isValid(), True)
        QgsProject.instance().addMapLayer(l1)

        query = toPercent("SELECT * FROM vt")
        l2 = QgsVectorLayer("?layer_ref=%s:vt&query=%s&uid=ObJeCtId&nogeometry" % (l1.id(), query), "vtab", "virtual",
                            QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l2.isValid(), True)
        self.assertEqual(l2.dataProvider().wkbType(), 100)  # NoGeometry

        QgsProject.instance().removeMapLayer(l1.id())

    def test_Join(self):
        l1 = QgsVectorLayer(os.path.join(self.testDataDir, "points.shp"), "points", "ogr",
                            QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l1.isValid(), True)
        QgsProject.instance().addMapLayer(l1)
        l2 = QgsVectorLayer(os.path.join(self.testDataDir, "points_relations.shp"), "points_relations", "ogr",
                            QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l2.isValid(), True)
        QgsProject.instance().addMapLayer(l2)
        ref_sum = sum(f.attributes()[1] for f in l2.getFeatures())

        # use a temporary file
        query = toPercent(
            "select id,Pilots,vtab1.geometry from vtab1,vtab2 where intersects(vtab1.geometry,vtab2.geometry)")
        l3 = QgsVectorLayer(
            "?layer_ref=%s&layer_ref=%s&uid=id&query=%s&geometry=geometry:1:4326" % (l1.id(), l2.id(), query), "vtab",
            "virtual", QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l3.isValid(), True)
        self.assertEqual(l3.dataProvider().wkbType(), 1)
        self.assertEqual(l3.dataProvider().fields().count(), 2)
        ref_sum2 = sum(f.id() for f in l3.getFeatures())
        self.assertEqual(ref_sum, ref_sum2)

        QgsProject.instance().removeMapLayer(l1)
        QgsProject.instance().removeMapLayer(l2)

    def test_geometryTypes(self):

        geo = [(1, "POINT", "(0 0)"),
               (2, "LINESTRING", "(0 0,1 0)"),
               (3, "POLYGON", "((0 0,1 0,1 1,0 0))"),
               (4, "MULTIPOINT", "((1 1))"),
               (5, "MULTILINESTRING", "((0 0,1 0),(0 1,1 1))"),
               (6, "MULTIPOLYGON", "(((0 0,1 0,1 1,0 0)),((2 2,3 0,3 3,2 2)))"),
               (9, "COMPOUNDCURVE", "(CIRCULARSTRING(0 0, 1 0, 1 1))"),
               (10, "CURVEPOLYGON", "(COMPOUNDCURVE(CIRCULARSTRING(0 0, 1 0, 1 1)))"),
               (11, "MULTICURVE", "(COMPOUNDCURVE(CIRCULARSTRING(0 0, 1 0, 1 1)),COMPOUNDCURVE(CIRCULARSTRING(2 2, 3 2, 3 3)))"),
               (12, "MULTISURFACE", "(CURVEPOLYGON(COMPOUNDCURVE(CIRCULARSTRING(0 0, 1 0, 1 1))),CURVEPOLYGON(COMPOUNDCURVE(CIRCULARSTRING(2 2, 3 2, 3 3))))")]
        for wkb_type, wkt_type, wkt in geo:
            l = QgsVectorLayer("%s?crs=epsg:4326" % wkt_type, "m1", "memory", QgsVectorLayer.LayerOptions(False))
            self.assertEqual(l.isValid(), True)
            QgsProject.instance().addMapLayer(l)

            f1 = QgsFeature(1)
            g = QgsGeometry.fromWkt(wkt_type + wkt)
            self.assertEqual(g is None, False)
            f1.setGeometry(g)
            l.dataProvider().addFeatures([f1])

            l2 = QgsVectorLayer("?layer_ref=%s" % l.id(), "vtab", "virtual", QgsVectorLayer.LayerOptions(False))
            self.assertEqual(l2.isValid(), True)
            self.assertEqual(l2.dataProvider().featureCount(), 1)
            # we ensure the geom type matches the original (segmentized) type
            self.assertEqual(l2.dataProvider().wkbType(), QgsWkbTypes.linearType(wkb_type))

            # we ensure the geometry still matches the original (segmentized) geometry
            f2 = QgsFeature()
            l2.getFeatures().nextFeature(f2)
            self.assertEqual(f2.geometry().asWkt(), f1.geometry().constGet().segmentize().asWkt())

            QgsProject.instance().removeMapLayer(l.id())

    def test_embeddedLayer(self):
        source = toPercent(os.path.join(self.testDataDir, "france_parts.shp"))
        l = QgsVectorLayer("?layer=ogr:%s" % source, "vtab", "virtual", QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l.isValid(), True)

        l = QgsVectorLayer("?layer=ogr:%s:nn" % source, "vtab", "virtual", QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l.isValid(), True)

    def test_filter_rect(self):
        source = toPercent(os.path.join(self.testDataDir, "france_parts.shp"))

        query = toPercent("select * from vtab where _search_frame_=BuildMbr(-2.10,49.38,-1.3,49.99,4326)")
        l2 = QgsVectorLayer("?layer=ogr:%s:vtab&query=%s&uid=objectid" % (source, query), "vtab2", "virtual",
                            QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l2.isValid(), True)
        self.assertEqual(l2.dataProvider().featureCount(), 1)
        a = [fit.attributes()[4] for fit in l2.getFeatures()]
        self.assertEqual(a, ["Basse-Normandie"])

    def test_recursiveLayer(self):
        source = toPercent(os.path.join(self.testDataDir, "france_parts.shp"))
        l = QgsVectorLayer("?layer=ogr:%s" % source, "vtab", "virtual", QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l.isValid(), True)
        QgsProject.instance().addMapLayer(l)

        l2 = QgsVectorLayer("?layer_ref=" + l.id(), "vtab2", "virtual", QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l2.isValid(), True)

        QgsProject.instance().removeMapLayer(l.id())

    def test_no_geometry(self):
        df = QgsVirtualLayerDefinition()
        df.addSource("vtab", os.path.join(self.testDataDir, "france_parts.shp"), "ogr")
        df.setGeometryWkbType(QgsWkbTypes.NoGeometry)
        l2 = QgsVectorLayer(df.toString(), "vtab2", "virtual", QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l2.isValid(), True)
        self.assertEqual(l2.dataProvider().wkbType(), 100)  # NoGeometry

    def test_reopen(self):
        source = toPercent(os.path.join(self.testDataDir, "france_parts.shp"))
        tmp = QUrl.fromLocalFile(os.path.join(tempfile.gettempdir(), "t.sqlite")).toString()
        l = QgsVectorLayer("%s?layer=ogr:%s:vtab" % (tmp, source), "vtab2", "virtual",
                           QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l.isValid(), True)

        l2 = QgsVectorLayer(tmp, "tt", "virtual", QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l2.isValid(), True)
        self.assertEqual(l2.dataProvider().wkbType(), 6)
        self.assertEqual(l2.dataProvider().featureCount(), 4)

    def test_reopen2(self):
        source = toPercent(os.path.join(self.testDataDir, "france_parts.shp"))
        tmp = QUrl.fromLocalFile(os.path.join(tempfile.gettempdir(), "t.sqlite")).toString()
        l = QgsVectorLayer("%s?layer=ogr:%s:vtab&nogeometry" % (tmp, source), "vtab2", "virtual",
                           QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l.isValid(), True)

        l2 = QgsVectorLayer(tmp, "tt", "virtual", QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l2.isValid(), True)
        self.assertEqual(l2.dataProvider().wkbType(), 100)
        self.assertEqual(l2.dataProvider().featureCount(), 4)

    def test_reopen3(self):
        source = toPercent(os.path.join(self.testDataDir, "france_parts.shp"))
        tmp = QUrl.fromLocalFile(os.path.join(tempfile.gettempdir(), "t.sqlite")).toString()
        query = toPercent("SELECT * FROM vtab")
        l = QgsVectorLayer("%s?layer=ogr:%s:vtab&query=%s&uid=objectid&geometry=geometry:3:4326" % (tmp, source, query),
                           "vtab2", "virtual", QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l.isValid(), True)

        l2 = QgsVectorLayer(tmp, "tt", "virtual", QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l2.isValid(), True)
        self.assertEqual(l2.dataProvider().wkbType(), 3)
        self.assertEqual(l2.dataProvider().featureCount(), 4)
        sumid = sum([f.id() for f in l2.getFeatures()])
        self.assertEqual(sumid, 10659)
        suma = sum([f.attributes()[1] for f in l2.getFeatures()])
        self.assertEqual(suma, 3064.0)

    def test_reopen4(self):
        source = toPercent(os.path.join(self.testDataDir, "france_parts.shp"))
        tmp = QUrl.fromLocalFile(os.path.join(tempfile.gettempdir(), "t.sqlite")).toString()
        query = toPercent("SELECT * FROM vtab")
        l = QgsVectorLayer("%s?layer=ogr:%s:vtab&query=%s&uid=objectid&nogeometry" % (tmp, source, query), "vtab2",
                           "virtual", QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l.isValid(), True)

        l2 = QgsVectorLayer(tmp, "tt", "virtual", QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l2.isValid(), True)
        self.assertEqual(l2.dataProvider().wkbType(), 100)
        self.assertEqual(l2.dataProvider().featureCount(), 4)
        sumid = sum([f.id() for f in l2.getFeatures()])
        self.assertEqual(sumid, 10659)
        suma = sum([f.attributes()[1] for f in l2.getFeatures()])
        self.assertEqual(suma, 3064.0)

    def test_refLayer(self):
        l1 = QgsVectorLayer(QUrl.fromLocalFile(os.path.join(self.testDataDir,
                                                            "delimitedtext/test.csv")).toString() + "?type=csv&geomType=none&subsetIndex=no&watchFile=no",
                            "test", "delimitedtext", QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l1.isValid(), True)
        QgsProject.instance().addMapLayer(l1)

        l2 = QgsVectorLayer("?layer_ref=" + l1.id(), "vtab", "virtual", QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l2.isValid(), True)

        # now delete the layer
        QgsProject.instance().removeMapLayer(l1.id())
        # check that it does not crash
        print((sum([f.id() for f in l2.getFeatures()])))

    def test_refLayers(self):
        l1 = QgsVectorLayer(QUrl.fromLocalFile(os.path.join(self.testDataDir,
                                                            "delimitedtext/test.csv")).toString() + "?type=csv&geomType=none&subsetIndex=no&watchFile=no",
                            "test", "delimitedtext", QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l1.isValid(), True)
        QgsProject.instance().addMapLayer(l1)

        # cf qgis bug #12266
        for i in range(10):
            q = toPercent("select * from t" + str(i))
            l2 = QgsVectorLayer("?layer_ref=%s:t%d&query=%s&uid=id" % (l1.id(), i, q), "vtab", "virtual",
                                QgsVectorLayer.LayerOptions(False))
            QgsProject.instance().addMapLayer(l2)
            self.assertEqual(l2.isValid(), True)
            s = sum([f.id() for f in l2.dataProvider().getFeatures()])  # NOQA
            self.assertEqual(sum([f.id() for f in l2.getFeatures()]), 21)
            QgsProject.instance().removeMapLayer(l2.id())

    def test_refLayers2(self):
        l1 = QgsVectorLayer(QUrl.fromLocalFile(os.path.join(self.testDataDir,
                                                            "delimitedtext/test.csv")).toString() + "?type=csv&geomType=none&subsetIndex=no&watchFile=no",
                            "test", "delimitedtext", QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l1.isValid(), True)
        QgsProject.instance().addMapLayer(l1)

        # referenced layers cannot be stored !
        tmp = QUrl.fromLocalFile(os.path.join(tempfile.gettempdir(), "t.sqlite")).toString()
        l2 = QgsVectorLayer("%s?layer_ref=%s" % (tmp, l1.id()), "tt", "virtual", QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l2.isValid(), False)
        self.assertEqual("Cannot store referenced layers" in l2.dataProvider().error().message(), True)

    def test_sql(self):
        l1 = QgsVectorLayer(QUrl.fromLocalFile(os.path.join(self.testDataDir,
                                                            "delimitedtext/test.csv")).toString() + "?type=csv&geomType=none&subsetIndex=no&watchFile=no",
                            "test", "delimitedtext", QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l1.isValid(), True)
        QgsProject.instance().addMapLayer(l1)

        l3 = QgsVectorLayer("?query=SELECT * FROM test", "tt", "virtual")

        self.assertEqual(l3.isValid(), True)
        s = sum(f.id() for f in l3.getFeatures())
        self.assertEqual(s, 15)

    def test_sql2(self):
        l2 = QgsVectorLayer(os.path.join(self.testDataDir, "france_parts.shp"), "france_parts", "ogr",
                            QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l2.isValid(), True)
        QgsProject.instance().addMapLayer(l2)

        query = toPercent("SELECT * FROM france_parts")
        l4 = QgsVectorLayer("?query=%s&uid=ObjectId" % query, "tt", "virtual")
        self.assertEqual(l4.isValid(), True)

        self.assertEqual(l4.dataProvider().wkbType(), 6)
        self.assertEqual(l4.dataProvider().crs().postgisSrid(), 4326)

        n = 0
        r = QgsFeatureRequest(QgsRectangle(-1.677, 49.624, -0.816, 49.086))
        for f in l4.getFeatures(r):
            self.assertEqual(f.geometry() is not None, True)
            self.assertEqual(f.attributes()[0], 2661)
            n += 1
        self.assertEqual(n, 1)

        # use uid
        query = toPercent("SELECT * FROM france_parts")
        l5 = QgsVectorLayer("?query=%s&geometry=geometry:polygon:4326&uid=ObjectId" % query, "tt", "virtual")
        self.assertEqual(l5.isValid(), True)

        idSum = sum(f.id() for f in l5.getFeatures())
        self.assertEqual(idSum, 10659)

        r = QgsFeatureRequest(2661)
        idSum2 = sum(f.id() for f in l5.getFeatures(r))
        self.assertEqual(idSum2, 2661)

        r = QgsFeatureRequest()
        r.setFilterFids([2661, 2664])
        self.assertEqual(sum(f.id() for f in l5.getFeatures(r)), 2661 + 2664)

        # test attribute subset
        r = QgsFeatureRequest()
        r.setFlags(QgsFeatureRequest.SubsetOfAttributes)
        r.setSubsetOfAttributes([1])
        s = [(f.id(), f.attributes()[1]) for f in l5.getFeatures(r)]
        self.assertEqual(sum([x[0] for x in s]), 10659)
        self.assertEqual(sum([x[1] for x in s]), 3064.0)

        # test NoGeometry
        # by request flag
        r = QgsFeatureRequest()
        r.setFlags(QgsFeatureRequest.NoGeometry)
        self.assertEqual(all([not f.hasGeometry() for f in l5.getFeatures(r)]), True)

        # test subset
        self.assertEqual(l5.dataProvider().featureCount(), 4)
        l5.setSubsetString("ObjectId = 2661")
        idSum2 = sum(f.id() for f in l5.getFeatures(r))
        self.assertEqual(idSum2, 2661)
        self.assertEqual(l5.dataProvider().featureCount(), 1)

    def test_sql3(self):
        l2 = QgsVectorLayer(os.path.join(self.testDataDir, "france_parts.shp"), "france_parts", "ogr",
                            QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l2.isValid(), True)
        QgsProject.instance().addMapLayer(l2)

        # unnamed column
        query = toPercent("SELECT count(*)")
        l4 = QgsVectorLayer("?query=%s" % query, "tt", "virtual", QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l4.isValid(), True)
        self.assertEqual(l4.dataProvider().fields().at(0).name(), "count(*)")
        self.assertEqual(l4.dataProvider().fields().at(0).type(), QVariant.LongLong)

    def test_sql_field_types(self):
        query = toPercent("SELECT 42 as t, 'ok'||'ok' as t2, GeomFromText('') as t3, 3.14*2 as t4")
        l4 = QgsVectorLayer("?query=%s" % query, "tt", "virtual", QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l4.isValid(), True)
        self.assertEqual(l4.dataProvider().fields().at(0).name(), "t")
        self.assertEqual(l4.dataProvider().fields().at(0).type(), QVariant.LongLong)
        self.assertEqual(l4.dataProvider().fields().at(1).name(), "t2")
        self.assertEqual(l4.dataProvider().fields().at(1).type(), QVariant.String)
        self.assertEqual(l4.dataProvider().fields().at(2).name(), "t3")
        self.assertEqual(l4.dataProvider().fields().at(2).type(), QVariant.String)
        self.assertEqual(l4.dataProvider().fields().at(3).name(), "t4")
        self.assertEqual(l4.dataProvider().fields().at(3).type(), QVariant.Double)

        # with type annotations
        query = toPercent(
            "SELECT '42.0' as t /*:real*/, 3 as t2/*:text  */, GeomFromText('') as t3 /*:multiPoInT:4326 */, 3.14*2 as t4/*:int*/")
        l4 = QgsVectorLayer("?query=%s" % query, "tt", "virtual", QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l4.isValid(), True)
        self.assertEqual(l4.dataProvider().fields().at(0).name(), "t")
        self.assertEqual(l4.dataProvider().fields().at(0).type(), QVariant.Double)
        self.assertEqual(l4.dataProvider().fields().at(1).name(), "t2")
        self.assertEqual(l4.dataProvider().fields().at(1).type(), QVariant.String)
        self.assertEqual(l4.dataProvider().fields().at(2).name(), "t4")
        self.assertEqual(l4.dataProvider().fields().at(2).type(), QVariant.LongLong)
        self.assertEqual(l4.dataProvider().wkbType(), 4)  # multipoint

        # test value types (!= from declared column types)
        for f in l4.getFeatures():
            self.assertEqual(f.attributes()[0], "42.0")
            self.assertEqual(f.attributes()[1], 3)
            self.assertEqual(f.attributes()[2], 6.28)

        # with type annotations and url options
        query = toPercent("SELECT 1 as id /*:int*/, geomfromtext('point(0 0)',4326) as geometry/*:point:4326*/")
        l4 = QgsVectorLayer("?query=%s&geometry=geometry" % query, "tt", "virtual", QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l4.isValid(), True)
        self.assertEqual(l4.dataProvider().wkbType(), 1)  # point

        # with type annotations and url options (2)
        query = toPercent(
            "SELECT 1 as id /*:int*/, 3.14 as f, geomfromtext('point(0 0)',4326) as geometry/*:point:4326*/")
        l4 = QgsVectorLayer("?query=%s&geometry=geometry&field=id:text" % query, "tt", "virtual",
                            QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l4.isValid(), True)
        self.assertEqual(l4.dataProvider().fields().at(0).name(), "id")
        self.assertEqual(l4.dataProvider().fields().at(0).type(), QVariant.String)
        self.assertEqual(l4.dataProvider().fields().at(1).name(), "f")
        self.assertEqual(l4.dataProvider().fields().at(1).type(), QVariant.Double)
        self.assertEqual(l4.dataProvider().wkbType(), 1)  # point

    def test_sql3b(self):
        query = toPercent("SELECT GeomFromText('POINT(0 0)') as geom")
        l4 = QgsVectorLayer("?query=%s&geometry=geom" % query, "tt", "virtual", QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l4.isValid(), True)
        self.assertEqual(l4.dataProvider().wkbType(), 1)

        # forced geometry type
        query = toPercent("SELECT GeomFromText('POINT(0 0)') as geom")
        l4 = QgsVectorLayer("?query=%s&geometry=geom:point:0" % query, "tt", "virtual",
                            QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l4.isValid(), True)
        self.assertEqual(l4.dataProvider().wkbType(), 1)

        query = toPercent("SELECT CastToPoint(GeomFromText('POINT(0 0)')) as geom")
        l4 = QgsVectorLayer("?query=%s" % query, "tt", "virtual", QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l4.isValid(), True)
        self.assertEqual(l4.dataProvider().wkbType(), 1)

    def test_sql4(self):
        l2 = QgsVectorLayer(os.path.join(self.testDataDir, "france_parts.shp"), "france_parts", "ogr",
                            QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l2.isValid(), True)
        QgsProject.instance().addMapLayer(l2)

        query = toPercent("SELECT OBJECTId from france_parts")
        l4 = QgsVectorLayer("?query=%s" % query, "tt", "virtual", QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l4.isValid(), True)
        s = sum(f.attributes()[0] for f in l4.getFeatures())
        self.assertEqual(s, 10659)

    def test_layer_name(self):
        # test space and upper case
        l2 = QgsVectorLayer(os.path.join(self.testDataDir, "france_parts.shp"), "FranCe parts", "ogr",
                            QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l2.isValid(), True)
        QgsProject.instance().addMapLayer(l2)

        query = toPercent('SELECT OBJECTId from "FranCe parts"')
        l4 = QgsVectorLayer("?query=%s" % query, "tt", "virtual", QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l4.isValid(), True)
        s = sum(f.attributes()[0] for f in l4.getFeatures())
        self.assertEqual(s, 10659)

    def test_encoding(self):
        # changes encoding on a shapefile (the only provider supporting setEncoding)
        source = toPercent(os.path.join(self.testDataDir, "shp_latin1.dbf"))
        l = QgsVectorLayer("?layer=ogr:%s:fp:latin1" % source, "vtab", "virtual", QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l.isValid(), True)

        for f in l.getFeatures():
            self.assertEqual(f.attributes()[1], "accents éàè")

        # use UTF-8 now
        l = QgsVectorLayer("?layer=ogr:%s:fp:UTF-8" % source, "vtab", "virtual", QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l.isValid(), True)
        for f in l.getFeatures():
            self.assertEqual(f.attributes()[1], "accents \ufffd\ufffd\ufffd")  # invalid unicode characters

    def test_rowid(self):
        source = toPercent(os.path.join(self.testDataDir, "france_parts.shp"))
        query = toPercent("select rowid as uid, * from vtab limit 1 offset 3")
        l = QgsVectorLayer("?layer=ogr:%s:vtab&query=%s" % (source, query), "vtab2", "virtual",
                           QgsVectorLayer.LayerOptions(False))
        # the last line must have a fixed rowid (not an autoincrement)
        for f in l.getFeatures():
            lid = f.attributes()[0]
        self.assertEqual(lid, 3)

    def test_geometry_conversion(self):
        query = toPercent("select geomfromtext('multipoint((0 0),(1 1))') as geom")
        l = QgsVectorLayer("?query=%s&geometry=geom:multipoint:0" % query, "tt", "virtual",
                           QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l.isValid(), True)
        for f in l.getFeatures():
            self.assertEqual(f.geometry().asWkt().lower().startswith("multipoint"), True)
            self.assertEqual("),(" in f.geometry().asWkt(), True)  # has two points

        query = toPercent(
            "select geomfromtext('multipolygon(((0 0,1 0,1 1,0 1,0 0)),((0 1,1 1,1 2,0 2,0 1)))') as geom")
        l = QgsVectorLayer("?query=%s&geometry=geom:multipolygon:0" % query, "tt", "virtual",
                           QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l.isValid(), True)
        for f in l.getFeatures():
            self.assertEqual(f.geometry().asWkt().lower().startswith("multipolygon"), True)
            self.assertEqual(")),((" in f.geometry().asWkt(), True)  # has two polygons

        query = toPercent("select geomfromtext('multilinestring((0 0,1 0,1 1,0 1,0 0),(0 1,1 1,1 2,0 2,0 1))') as geom")
        l = QgsVectorLayer("?query=%s&geometry=geom:multilinestring:0" % query, "tt", "virtual",
                           QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l.isValid(), True)
        for f in l.getFeatures():
            self.assertEqual(f.geometry().asWkt().lower().startswith("multilinestring"), True)
            self.assertEqual("),(" in f.geometry().asWkt(), True)  # has two linestrings

    def test_queryOnMemoryLayer(self):
        ml = QgsVectorLayer("Point?srid=EPSG:4326&field=a:int", "mem", "memory")
        self.assertEqual(ml.isValid(), True)
        QgsProject.instance().addMapLayer(ml)

        ml.startEditing()
        f1 = QgsFeature(ml.fields())
        f1.setGeometry(QgsGeometry.fromWkt('POINT(0 0)'))
        f2 = QgsFeature(ml.fields())
        f2.setGeometry(QgsGeometry.fromWkt('POINT(1 1)'))
        ml.addFeatures([f1, f2])
        ml.commitChanges()

        vl = QgsVectorLayer("?query=select * from mem", "vl", "virtual")
        self.assertEqual(vl.isValid(), True)

        self.assertEqual(ml.featureCount(), vl.featureCount())

        # test access to pending features as well
        ml.startEditing()
        f3 = QgsFeature(ml.fields())
        ml.addFeatures([f3])
        self.assertEqual(ml.featureCount(), vl.featureCount())

    def test_ProjectDependencies(self):
        # make a virtual layer with living references and save it to a project
        l1 = QgsVectorLayer(os.path.join(self.testDataDir, "france_parts.shp"), "france_parts", "ogr",
                            QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l1.isValid(), True)
        QgsProject.instance().addMapLayer(l1)

        query = toPercent("SELECT * FROM france_parts")
        l2 = QgsVectorLayer("?query=%s" % query, "aa", "virtual", QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l2.isValid(), True)
        QgsProject.instance().addMapLayer(l2)

        self.assertEqual(len(l2.dependencies()), 1)
        ll0 = l2.dependencies().pop()
        self.assertEqual(ll0.layerId().startswith('france_parts'), True)

        query = toPercent("SELECT t1.objectid, t2.name_0 FROM france_parts as t1, aa as t2")
        l3 = QgsVectorLayer("?query=%s" % query, "bb", "virtual", QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l3.isValid(), True)
        QgsProject.instance().addMapLayer(l3)

        self.assertEqual(len(l2.dependencies()), 1)
        ll0 = l2.dependencies().pop()
        self.assertEqual(ll0.layerId().startswith('france_parts'), True)

        self.assertEqual(len(l3.dependencies()), 2)

        temp = os.path.join(tempfile.gettempdir(), "qgstestproject.qgs")

        QgsProject.instance().setFileName(temp)
        QgsProject.instance().write()

        QgsProject.instance().removeMapLayers([l1.id(), l2.id()])
        QgsProject.instance().clear()

        QgsProject.instance().setFileName(temp)
        QgsProject.instance().read()

        # make sure the 3 layers are loaded back
        self.assertEqual(len(QgsProject.instance().mapLayers()), 3)

    def test_relative_paths(self):
        """
        Test whether paths to layer sources are stored as relative to the project path
        """

        # make a virtual layer with living references and save it to a project
        QgsProject.instance().clear()
        l0 = QgsVectorLayer(os.path.join(self.testDataDir, "france_parts.shp"), "france_parts", "ogr",
                            QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l0.isValid(), True)
        QgsProject.instance().addMapLayer(l0)

        df = QgsVirtualLayerDefinition()
        df.addSource("vtab", os.path.join(self.testDataDir, "france_parts.shp"), "ogr")
        l1 = QgsVectorLayer(df.toString(), "vtab", "virtual", QgsVectorLayer.LayerOptions(False))

        self.assertEqual(l1.isValid(), True)
        QgsProject.instance().addMapLayer(l1)

        temp = os.path.join(self.testDataDir, "qgstestproject_relative_path_test.qgs")

        QgsProject.instance().setFileName(temp)
        QgsProject.instance().write()

        QgsProject.instance().clear()
        self.assertEqual(len(QgsProject.instance().mapLayers()), 0)

        # Check that virtual layer source is stored with relative path
        percent_path_relative = toPercent("./france_parts.shp")
        with open(temp, 'r') as f:
            content = ''.join(f.readlines())
            self.assertTrue('<datasource>?layer=ogr:{}'.format(percent_path_relative) in content)

        # Check that project is correctly re-read with all layers
        QgsProject.instance().setFileName(temp)
        QgsProject.instance().read()
        print(QgsProject.instance().mapLayers())
        self.assertEqual(len(QgsProject.instance().mapLayers()), 2)

        # Store absolute
        QgsProject.instance().writeEntryBool('Paths', '/Absolute', True)
        QgsProject.instance().write()

        QgsProject.instance().clear()
        self.assertEqual(len(QgsProject.instance().mapLayers()), 0)

        # Check that virtual layer source is stored with absolute path
        percent_path_absolute = toPercent(os.path.join(self.testDataDir, "france_parts.shp"))
        with open(temp, 'r') as f:
            content = ''.join(f.readlines())
            self.assertTrue('<datasource>?layer=ogr:{}'.format(percent_path_absolute) in content)

        # Check that project is correctly re-read with all layers
        QgsProject.instance().setFileName(temp)
        QgsProject.instance().read()
        self.assertEqual(len(QgsProject.instance().mapLayers()), 2)

    def test_qgisExpressionFunctions(self):
        QgsProject.instance().setTitle('project')
        self.assertEqual(QgsProject.instance().title(), 'project')
        df = QgsVirtualLayerDefinition()
        df.setQuery(
            "SELECT format('hello %1', 'world') as a, year(todate('2016-01-02')) as b, title('This') as t, var('project_title') as c")
        l = QgsVectorLayer(df.toString(), "testq", "virtual")
        self.assertEqual(l.isValid(), True)

        for f in l.getFeatures():
            self.assertEqual(f.attributes(), ['hello world', 2016, 'This', 'project'])

    def test_query_with_accents(self):
        # shapefile with accents and latin1 encoding
        df = QgsVirtualLayerDefinition()
        df.addSource("vtab", os.path.join(self.testDataDir, "france_parts.shp"), "ogr", "ISO-8859-1")
        df.setQuery("SELECT * FROM vtab WHERE TYPE_1 = 'Région'")
        vl = QgsVectorLayer(df.toString(), "testq", "virtual")
        self.assertEqual(vl.isValid(), True)
        ids = [f.id() for f in vl.getFeatures()]
        self.assertEqual(len(ids), 4)

        # the same shapefile with a wrong encoding
        df.addSource("vtab", os.path.join(self.testDataDir, "france_parts.shp"), "ogr", "UTF-8")
        df.setQuery("SELECT * FROM vtab WHERE TYPE_1 = 'Région'")
        vl2 = QgsVectorLayer(df.toString(), "testq", "virtual")
        self.assertEqual(vl2.isValid(), True)
        ids = [f.id() for f in vl2.getFeatures()]
        self.assertEqual(ids, [])

    def test_layer_with_accents(self):
        l1 = QgsVectorLayer(os.path.join(self.testDataDir, "france_parts.shp"), "françéà", "ogr",
                            QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l1.isValid(), True)
        QgsProject.instance().addMapLayer(l1)

        df = QgsVirtualLayerDefinition()
        df.setQuery('select * from "françéà"')

        vl = QgsVectorLayer(df.toString(), "testq", "virtual")
        self.assertEqual(vl.isValid(), True)
        ids = [f.id() for f in vl.getFeatures()]
        self.assertEqual(len(ids), 4)

        QgsProject.instance().removeMapLayer(l1.id())

    def test_lazy(self):
        l1 = QgsVectorLayer(os.path.join(self.testDataDir, "france_parts.shp"), "françéà", "ogr",
                            QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l1.isValid(), True)
        QgsProject.instance().addMapLayer(l1)

        df = QgsVirtualLayerDefinition()
        df.setQuery('select * from "françéà"')
        df.setLazy(True)

        vl = QgsVectorLayer(df.toString(), "testq", "virtual")
        self.assertEqual(vl.isValid(), True)
        ids = [f.id() for f in vl.getFeatures()]
        self.assertEqual(len(ids), 0)

        vl.reload()
        ids = [f.id() for f in vl.getFeatures()]
        self.assertEqual(len(ids), 4)

        QgsProject.instance().removeMapLayer(l1.id())

    def test_joined_layers_conversion(self):
        v1 = QgsVectorLayer("Point?field=id:integer&field=b_id:integer&field=c_id:integer&field=name:string", "A",
                            "memory")
        self.assertEqual(v1.isValid(), True)
        v2 = QgsVectorLayer("Point?field=id:integer&field=bname:string&field=bfield:integer", "B", "memory")
        self.assertEqual(v2.isValid(), True)
        v3 = QgsVectorLayer("Point?field=id:integer&field=cname:string", "C", "memory")
        self.assertEqual(v3.isValid(), True)
        tl1 = QgsVectorLayer("NoGeometry?field=id:integer&field=e_id:integer&field=0name:string", "D", "memory")
        self.assertEqual(tl1.isValid(), True)
        tl2 = QgsVectorLayer("NoGeometry?field=id:integer&field=ena me:string", "E", "memory")
        self.assertEqual(tl2.isValid(), True)
        QgsProject.instance().addMapLayers([v1, v2, v3, tl1, tl2])
        joinInfo = QgsVectorLayerJoinInfo()
        joinInfo.setTargetFieldName("b_id")
        joinInfo.setJoinLayer(v2)
        joinInfo.setJoinFieldName("id")
        # joinInfo.setPrefix("B_")
        v1.addJoin(joinInfo)
        self.assertEqual(len(v1.fields()), 6)

        df = QgsVirtualLayerDefinitionUtils.fromJoinedLayer(v1)
        self.assertEqual(df.query(),
                         'SELECT t.geometry, t.rowid AS uid, t."id", t."b_id", t."c_id", t."name", j1."bname" AS "B_bname", j1."bfield" AS "B_bfield" FROM "{}" AS t LEFT JOIN "{}" AS j1 ON t."b_id"=j1."id"'.format(
                             v1.id(), v2.id()))

        # with a field subset
        v1.removeJoin(v2.id())
        joinInfo.setJoinFieldNamesSubset(["bname"])
        v1.addJoin(joinInfo)
        self.assertEqual(len(v1.fields()), 5)
        df = QgsVirtualLayerDefinitionUtils.fromJoinedLayer(v1)
        self.assertEqual(df.query(),
                         'SELECT t.geometry, t.rowid AS uid, t."id", t."b_id", t."c_id", t."name", j1."bname" AS "B_bname" FROM "{}" AS t LEFT JOIN "{}" AS j1 ON t."b_id"=j1."id"'.format(
                             v1.id(), v2.id()))
        joinInfo.setJoinFieldNamesSubset(None)

        # add a table prefix to the join
        v1.removeJoin(v2.id())
        joinInfo.setPrefix("BB_")
        v1.addJoin(joinInfo)
        self.assertEqual(len(v1.fields()), 6)
        df = QgsVirtualLayerDefinitionUtils.fromJoinedLayer(v1)
        self.assertEqual(df.query(),
                         'SELECT t.geometry, t.rowid AS uid, t."id", t."b_id", t."c_id", t."name", j1."bname" AS "BB_bname", j1."bfield" AS "BB_bfield" FROM "{}" AS t LEFT JOIN "{}" AS j1 ON t."b_id"=j1."id"'.format(
                             v1.id(), v2.id()))
        joinInfo.setPrefix("")
        v1.removeJoin(v2.id())
        v1.addJoin(joinInfo)

        # add another join
        joinInfo2 = QgsVectorLayerJoinInfo()
        joinInfo2.setTargetFieldName("c_id")
        joinInfo2.setJoinLayer(v3)
        joinInfo2.setJoinFieldName("id")
        v1.addJoin(joinInfo2)
        self.assertEqual(len(v1.fields()), 7)
        df = QgsVirtualLayerDefinitionUtils.fromJoinedLayer(v1)
        self.assertEqual(df.query(), (
            'SELECT t.geometry, t.rowid AS uid, t."id", t."b_id", t."c_id", t."name", j1."bname" AS "B_bname", j1."bfield" AS "B_bfield", j2."cname" AS "C_cname" FROM "{}" AS t ' +
            'LEFT JOIN "{}" AS j1 ON t."b_id"=j1."id" ' +
            'LEFT JOIN "{}" AS j2 ON t."c_id"=j2."id"').format(v1.id(), v2.id(), v3.id()))

        # test NoGeometry joined layers with field names starting with a digit or containing white spaces
        joinInfo3 = QgsVectorLayerJoinInfo()
        joinInfo3.setTargetFieldName("e_id")
        joinInfo3.setJoinLayer(tl2)
        joinInfo3.setJoinFieldName("id")
        tl1.addJoin(joinInfo3)
        self.assertEqual(len(tl1.fields()), 4)
        df = QgsVirtualLayerDefinitionUtils.fromJoinedLayer(tl1)
        self.assertEqual(df.query(),
                         'SELECT t.rowid AS uid, t."id", t."e_id", t."0name", j1."ena me" AS "E_ena me" FROM "{}" AS t LEFT JOIN "{}" AS j1 ON t."e_id"=j1."id"'.format(
                             tl1.id(), tl2.id()))

        QgsProject.instance().removeMapLayers([v1.id(), v2.id(), v3.id(), tl1.id(), tl2.id()])

    def testFieldsWithSpecialCharacters(self):
        ml = QgsVectorLayer("Point?srid=EPSG:4326&field=123:int", "mem_with_nontext_fieldnames", "memory")
        self.assertEqual(ml.isValid(), True)
        QgsProject.instance().addMapLayer(ml)

        ml.startEditing()
        self.assertTrue(ml.addAttribute(QgsField('abc:123', QVariant.String)))
        self.assertTrue(ml.addAttribute(QgsField('map', QVariant.String)))  # matches QGIS expression function name
        f1 = QgsFeature(ml.fields())
        f1.setGeometry(QgsGeometry.fromWkt('POINT(0 0)'))
        f1.setAttributes([1, 'a', 'b'])
        f2 = QgsFeature(ml.fields())
        f2.setGeometry(QgsGeometry.fromWkt('POINT(1 1)'))
        f2.setAttributes([2, 'c', 'd'])
        ml.addFeatures([f1, f2])
        ml.commitChanges()

        vl = QgsVectorLayer("?query=select * from mem_with_nontext_fieldnames", "vl", "virtual")
        self.assertEqual(vl.isValid(), True)
        self.assertEqual(vl.fields().at(0).name(), '123')
        self.assertEqual(vl.fields().at(1).name(), 'abc:123')

        self.assertEqual(vl.featureCount(), 2)

        features = [f for f in vl.getFeatures(QgsFeatureRequest().setFilterExpression('"abc:123"=\'c\''))]
        self.assertEqual(len(features), 1)
        self.assertEqual(features[0].attributes(), [2, 'c', 'd'])

        features = [f for f in vl.getFeatures(QgsFeatureRequest().setFilterExpression('"map"=\'b\''))]
        self.assertEqual(len(features), 1)
        self.assertEqual(features[0].attributes(), [1, 'a', 'b'])

        vl2 = QgsVectorLayer("?query=select * from mem_with_nontext_fieldnames where \"abc:123\"='c'", "vl", "virtual")
        self.assertEqual(vl2.isValid(), True)
        self.assertEqual(vl2.fields().at(0).name(), '123')
        self.assertEqual(vl2.fields().at(1).name(), 'abc:123')

        self.assertEqual(vl2.featureCount(), 1)

        features = [f for f in vl2.getFeatures()]
        self.assertEqual(len(features), 1)
        self.assertEqual(features[0].attributes(), [2, 'c', 'd'])

        vl3 = QgsVectorLayer("?query=select * from mem_with_nontext_fieldnames where \"map\"='b'", "vl", "virtual")
        self.assertEqual(vl3.isValid(), True)
        self.assertEqual(vl3.fields().at(0).name(), '123')
        self.assertEqual(vl3.fields().at(1).name(), 'abc:123')

        self.assertEqual(vl3.featureCount(), 1)

        features = [f for f in vl3.getFeatures()]
        self.assertEqual(len(features), 1)
        self.assertEqual(features[0].attributes(), [1, 'a', 'b'])

        QgsProject.instance().removeMapLayer(ml)

    def testFiltersWithoutUid(self):
        ml = QgsVectorLayer("Point?srid=EPSG:4326&field=a:int", "mem_no_uid", "memory")
        self.assertEqual(ml.isValid(), True)
        QgsProject.instance().addMapLayer(ml)

        # a memory layer with 10 features
        ml.startEditing()
        for i in range(10):
            f = QgsFeature(ml.fields())
            f.setGeometry(QgsGeometry.fromWkt('POINT({} 0)'.format(i)))
            f.setAttributes([i])
            ml.addFeatures([f])
        ml.commitChanges()

        df = QgsVirtualLayerDefinition()
        df.setQuery('select * from mem_no_uid')
        vl = QgsVectorLayer(df.toString(), "vl", "virtual")
        self.assertEqual(vl.isValid(), True)

        # make sure the returned id with a filter is the same as
        # if there is no filter
        req = QgsFeatureRequest().setFilterRect(QgsRectangle(4.5, -1, 5.5, 1))
        fids = [f.id() for f in vl.getFeatures(req)]
        self.assertEqual(fids, [5])

        req = QgsFeatureRequest().setFilterExpression("a = 5")
        fids = [f.id() for f in vl.getFeatures(req)]
        self.assertEqual(fids, [5])

        req = QgsFeatureRequest().setFilterFid(5)
        a = [(f.id(), f['a']) for f in vl.getFeatures(req)]
        self.assertEqual(a, [(5, 5)])

        req = QgsFeatureRequest().setFilterFids([5, 6, 8])
        a = [(f.id(), f['a']) for f in vl.getFeatures(req)]
        self.assertEqual(a, [(5, 5), (6, 6), (8, 8)])

        QgsProject.instance().removeMapLayer(ml)

    def testUpdatedFields(self):
        """Test when referenced layer update its fields
        https://github.com/qgis/QGIS/issues/28712
        """

        ml = QgsVectorLayer("Point?srid=EPSG:4326&field=a:int", "mem", "memory")
        self.assertEqual(ml.isValid(), True)
        QgsProject.instance().addMapLayer(ml)

        ml.startEditing()
        f1 = QgsFeature(ml.fields())
        f1.setGeometry(QgsGeometry.fromWkt('POINT(2 3)'))
        ml.addFeatures([f1])
        ml.commitChanges()

        vl = QgsVectorLayer("?query=select a, geometry from mem", "vl", "virtual")
        self.assertEqual(vl.isValid(), True)

        # add one more field
        ml.dataProvider().addAttributes([QgsField('newfield', QVariant.Int)])
        ml.updateFields()

        self.assertEqual(ml.featureCount(), vl.featureCount())
        self.assertEqual(vl.fields().count(), 1)

        geometry = next(vl.getFeatures()).geometry()
        self.assertTrue(geometry)

        point = geometry.asPoint()
        self.assertEqual(point.x(), 2)
        self.assertEqual(point.y(), 3)

        QgsProject.instance().removeMapLayer(ml)

    def test_filter_rect_precise(self):

        # Check if don't lost precision when filtering on rect (see https://github.com/qgis/QGIS/issues/36054)

        pl = QgsVectorLayer(os.path.join(self.testDataDir, "points.shp"), "points", "ogr")
        self.assertTrue(pl.isValid())
        QgsProject.instance().addMapLayer(pl)

        query = toPercent("SELECT * FROM points")
        vl = QgsVectorLayer("?query=%s&uid=OBJECTID" % (query), "vl", "virtual")
        self.assertEqual(vl.isValid(), True)
        self.assertEqual(vl.dataProvider().featureCount(), 17)

        # Take an extent where east farthest point is excluded and second farthest east is on the edge
        # and should be returned
        extent = QgsRectangle(-117.23257418909581418, 22.80020703933767834, -85.6521739130433276, 46.87198067632875365)
        r = QgsFeatureRequest(extent)
        features = [feature for feature in vl.getFeatures(r)]
        self.assertEqual(len(features), 16)

        QgsProject.instance().removeMapLayer(pl)

    def test_subset_string(self):
        """Test that subset strings are stored and restored correctly from the project
        See: GH #26189
        """

        project = QgsProject.instance()
        project.clear()
        data_layer = QgsVectorLayer('Point?crs=epsg:4326&field=fid:integer&field=value:integer&field=join_pk:integer', 'data', 'memory')
        join_layer = QgsVectorLayer('NoGeometry?field=fid:integer&field=value:string', 'join', 'memory')
        tempdir = QTemporaryDir()
        gpkg_path = os.path.join(tempdir.path(), 'test_subset.gpkg')
        project_path = os.path.join(tempdir.path(), 'test_subset.qgs')
        self.assertTrue(data_layer.isValid())
        self.assertTrue(join_layer.isValid())
        self.assertFalse(join_layer.isSpatial())

        f = QgsFeature(data_layer.fields())
        f.setAttributes([1, 20, 2])
        f.setGeometry(QgsGeometry.fromWkt('point(9 45'))
        self.assertTrue(data_layer.dataProvider().addFeature(f))

        f = QgsFeature(data_layer.fields())
        f.setAttributes([2, 10, 1])
        f.setGeometry(QgsGeometry.fromWkt('point(9 45'))
        self.assertTrue(data_layer.dataProvider().addFeature(f))

        options = QgsVectorFileWriter.SaveVectorOptions()
        options.driverName = 'GPKG'
        options.actionOnExistingFile = QgsVectorFileWriter.CreateOrOverwriteFile
        options.layerName = 'data'

        _, _ = QgsVectorFileWriter.writeAsVectorFormatV2(
            data_layer,
            gpkg_path,
            data_layer.transformContext(),
            options
        )

        f = QgsFeature(join_layer.fields())
        f.setAttributes([1, "ten"])
        self.assertTrue(join_layer.dataProvider().addFeature(f))
        f.setAttributes([2, "twenty"])
        self.assertTrue(join_layer.dataProvider().addFeature(f))

        options.layerName = 'join'
        options.actionOnExistingFile = QgsVectorFileWriter.CreateOrOverwriteLayer

        _, _ = QgsVectorFileWriter.writeAsVectorFormatV2(
            join_layer,
            gpkg_path,
            join_layer.transformContext(),
            options
        )

        gpkg_join_layer = QgsVectorLayer(gpkg_path + '|layername=join', 'join', 'ogr')
        gpkg_data_layer = QgsVectorLayer(gpkg_path + '|layername=data', 'data', 'ogr')

        self.assertTrue(gpkg_join_layer.isValid())
        self.assertTrue(gpkg_data_layer.isValid())
        self.assertEqual(gpkg_data_layer.featureCount(), 2)
        self.assertEqual(gpkg_join_layer.featureCount(), 2)

        self.assertTrue(project.addMapLayers([gpkg_data_layer, gpkg_join_layer]))

        joinInfo = QgsVectorLayerJoinInfo()
        joinInfo.setTargetFieldName("join_pk")
        joinInfo.setJoinLayer(gpkg_join_layer)
        joinInfo.setJoinFieldName("fid")
        gpkg_data_layer.addJoin(joinInfo)
        self.assertEqual(len(gpkg_data_layer.fields()), 4)

        self.assertTrue(project.write(project_path))

        # Reload project
        self.assertTrue(project.read(project_path))
        gpkg_data_layer = project.mapLayersByName('data')[0]
        gpkg_join_layer = project.mapLayersByName('join')[0]

        self.assertEqual(gpkg_data_layer.vectorJoins()[0], joinInfo)

        # Now set a subset filter -> virtual layer
        virtual_def = QgsVirtualLayerDefinitionUtils.fromJoinedLayer(gpkg_data_layer)
        virtual = QgsVectorLayer(virtual_def.toString(), "virtual_data", "virtual")
        self.assertTrue(virtual.isValid())
        project.addMapLayers([virtual])

        self.assertEqual(virtual.featureCount(), 2)
        self.assertTrue(virtual.setSubsetString('"join_value" = \'twenty\''))
        self.assertEqual(virtual.featureCount(), 1)
        self.assertEqual([f.attributes() for f in virtual.getFeatures()], [[1, 20, 2, 'twenty']])

        # Store and reload the project
        self.assertTrue(project.write(project_path))
        self.assertTrue(project.read(project_path))
        gpkg_virtual_layer = project.mapLayersByName('virtual_data')[0]
        self.assertEqual(gpkg_virtual_layer.featureCount(), 1)
        self.assertEqual(gpkg_virtual_layer.subsetString(), '"join_value" = \'twenty\'')

    def test_feature_count_on_error(self):
        """Test that triggered exception while getting feature count on a badly defined
        virtual layer is correctly caught (see https://github.com/qgis/QGIS/issues/34378)"""

        l1 = QgsVectorLayer(os.path.join(self.testDataDir, "france_parts.shp"), "france", "ogr",
                            QgsVectorLayer.LayerOptions(False))
        self.assertEqual(l1.isValid(), True)
        QgsProject.instance().addMapLayer(l1)

        df = QgsVirtualLayerDefinition()
        df.setQuery('select error')

        vl = QgsVectorLayer(df.toString(), "testq", "virtual")
        self.assertEqual(vl.isValid(), False)
        self.assertEqual(vl.featureCount(), 0)
        ids = [f.id() for f in vl.getFeatures()]
        self.assertEqual(len(ids), 0)

        QgsProject.instance().removeMapLayer(l1.id())

    def test_bool_fields(self):

        ml = QgsVectorLayer("NoGeometry?field=a:int&field=b:boolean", "mem", "memory")
        self.assertEqual(ml.isValid(), True)
        QgsProject.instance().addMapLayer(ml)

        ml.startEditing()
        f1 = QgsFeature(ml.fields())
        f1.setAttribute('a', 1)
        f1.setAttribute('b', True)
        f2 = QgsFeature(ml.fields())
        f2.setAttribute('a', 2)
        f2.setAttribute('b', False)
        ml.addFeatures([f1, f2])
        ml.commitChanges()

        self.assertEqual([(f['a'], f['b']) for f in ml.getFeatures()], [(1, True), (2, False)])

        df = QgsVirtualLayerDefinition()
        df.setQuery('select * from mem')
        vl = QgsVectorLayer(df.toString(), "testq", "virtual")
        self.assertEqual([(f['a'], f['b']) for f in vl.getFeatures()], [(1, True), (2, False)])

        df = QgsVirtualLayerDefinition()
        df.setQuery('select * from mem where b')
        vl = QgsVectorLayer(df.toString(), "testq", "virtual")
        self.assertEqual([(f['a'], f['b']) for f in vl.getFeatures()], [(1, True)])

        df = QgsVirtualLayerDefinition()
        df.setQuery('select * from mem where not b')
        vl = QgsVectorLayer(df.toString(), "testq", "virtual")
        self.assertEqual([(f['a'], f['b']) for f in vl.getFeatures()], [(2, False)])

        QgsProject.instance().removeMapLayer(ml.id())

    def test_int64(self):
        """
        Test that 64 bits integer doesn't generate an integer overflow
        """
        bigint = 2262000000

        ml = QgsVectorLayer('NoGeometry?crs=epsg:4326&field=fldlonglong:long',
                            'test_bigint', 'memory')
        provider = ml.dataProvider()
        feat = QgsFeature(ml.fields())
        feat.setAttribute('fldlonglong', bigint)
        provider.addFeatures([feat])

        self.assertEqual(ml.isValid(), True)
        QgsProject.instance().addMapLayer(ml)

        df = QgsVirtualLayerDefinition()
        df.setQuery('select * from test_bigint')
        vl = QgsVectorLayer(df.toString(), "testq", "virtual")
        self.assertEqual(len(vl.fields()), 1)
        field = vl.fields()[0]
        self.assertEqual(field.type(), QVariant.LongLong)
        self.assertTrue(vl.isValid())
        feat = next(vl.getFeatures())
        self.assertEqual(feat.attribute('fldlonglong'), bigint)

    def test_layer_starting_with_digit(self):
        """Test issue GH #45347"""

        project = QgsProject.instance()
        project.clear()
        layer = QgsVectorLayer('Point?crs=epsg:4326&field=fid:integer', '1_layer', 'memory')
        project.addMapLayers([layer])

        df = QgsVirtualLayerDefinition()
        df.setQuery('select * from "1_layer"')
        vl = QgsVectorLayer(df.toString(), "1_layer_virtual", "virtual")
        self.assertTrue(vl.isValid())

    def test_layer_using_joined_fields_from_another_layer(self):
        """Test issue GH #46834"""

        project = QgsProject.instance()
        project.clear()
        layer_1 = QgsVectorLayer('Point?crs=epsg:4326&field=fid:integer', 'layer_1', 'memory')
        layer_2 = QgsVectorLayer('Point?crs=epsg:4326&field=fid:integer', 'layer_2', 'memory')

        project.addMapLayers([layer_1])

        # Add a join from 2 to 1
        join_info = QgsVectorLayerJoinInfo()
        join_info.setJoinLayer(layer_1)
        join_info.setJoinFieldName('layer_1_fid')
        join_info.setTargetFieldName('fid')
        self.assertTrue(layer_2.addJoin(join_info))
        self.assertIn('layer_1_fid', layer_2.fields().names())

        project.addMapLayers([layer_2])

        df = QgsVirtualLayerDefinition()
        df.setQuery('select fid, layer_1_fid from "layer_2"')
        vl = QgsVectorLayer(df.toString(), "virtual", "virtual")
        self.assertTrue(vl.isValid())

        project.addMapLayers([vl])

        tmp = QTemporaryDir()
        path = tmp.path()
        project.write(os.path.join(path, 'test_4683.qgs'))

        project.clear()
        project.read(os.path.join(path, 'test_4683.qgs'))

        layer_2 = project.mapLayersByName('layer_2')[0]
        vl = project.mapLayersByName('virtual')[0]

        self.assertTrue(vl.isValid())


if __name__ == '__main__':
    unittest.main()
