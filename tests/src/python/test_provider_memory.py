# -*- coding: utf-8 -*-
"""QGIS Unit tests for the memory layer provider.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Matthias Kuhn'
__date__ = '2015-04-23'
__copyright__ = 'Copyright 2015, The QGIS Project'

from urllib.parse import parse_qs

from qgis.PyQt.QtCore import QVariant, QByteArray, QDate, QDateTime, QTime
from qgis.core import (
    QgsField,
    QgsFields,
    QgsLayerDefinition,
    QgsPointXY,
    QgsReadWriteContext,
    QgsVectorLayer,
    QgsFeatureRequest,
    QgsFeature,
    QgsGeometry,
    QgsWkbTypes,
    NULL,
    QgsMemoryProviderUtils,
    QgsCoordinateReferenceSystem,
    QgsRectangle,
    QgsTestUtils,
    QgsFeatureSource,
    QgsFeatureSink,
)
from qgis.testing import (
    start_app,
    unittest
)

from providertestbase import ProviderTestCase
from utilities import (
    unitTestDataPath,
    compareWkt
)

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestPyQgsMemoryProvider(unittest.TestCase, ProviderTestCase):

    @classmethod
    def createLayer(cls):
        vl = QgsVectorLayer(
            'Point?crs=epsg:4326&field=pk:integer&field=cnt:integer&field=name:string(0)&field=name2:string(0)&field=num_char:string&field=dt:datetime&field=date:date&field=time:time&key=pk',
            'test', 'memory')
        assert (vl.isValid())

        f1 = QgsFeature()
        f1.setAttributes(
            [5, -200, NULL, 'NuLl', '5', QDateTime(QDate(2020, 5, 4), QTime(12, 13, 14)), QDate(2020, 5, 2),
             QTime(12, 13, 1)])
        f1.setGeometry(QgsGeometry.fromWkt('Point (-71.123 78.23)'))

        f2 = QgsFeature()
        f2.setAttributes([3, 300, 'Pear', 'PEaR', '3', NULL, NULL, NULL])

        f3 = QgsFeature()
        f3.setAttributes(
            [1, 100, 'Orange', 'oranGe', '1', QDateTime(QDate(2020, 5, 3), QTime(12, 13, 14)), QDate(2020, 5, 3),
             QTime(12, 13, 14)])
        f3.setGeometry(QgsGeometry.fromWkt('Point (-70.332 66.33)'))

        f4 = QgsFeature()
        f4.setAttributes(
            [2, 200, 'Apple', 'Apple', '2', QDateTime(QDate(2020, 5, 4), QTime(12, 14, 14)), QDate(2020, 5, 4),
             QTime(12, 14, 14)])
        f4.setGeometry(QgsGeometry.fromWkt('Point (-68.2 70.8)'))

        f5 = QgsFeature()
        f5.setAttributes(
            [4, 400, 'Honey', 'Honey', '4', QDateTime(QDate(2021, 5, 4), QTime(13, 13, 14)), QDate(2021, 5, 4),
             QTime(13, 13, 14)])
        f5.setGeometry(QgsGeometry.fromWkt('Point (-65.32 78.3)'))

        vl.dataProvider().addFeatures([f1, f2, f3, f4, f5])
        return vl

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        # Create test layer
        cls.vl = cls.createLayer()
        assert (cls.vl.isValid())
        cls.source = cls.vl.dataProvider()

        # poly layer
        cls.poly_vl = QgsVectorLayer('Polygon?crs=epsg:4326&field=pk:integer&key=pk',
                                     'test', 'memory')
        assert (cls.poly_vl.isValid())
        cls.poly_provider = cls.poly_vl.dataProvider()

        f1 = QgsFeature()
        f1.setAttributes([1])
        f1.setGeometry(QgsGeometry.fromWkt(
            'Polygon ((-69.03664108 81.35818902, -69.09237722 80.24346619, -73.718477 80.1319939, -73.718477 76.28620011, -74.88893598 76.34193625, -74.83319983 81.35818902, -69.03664108 81.35818902))'))

        f2 = QgsFeature()
        f2.setAttributes([2])
        f2.setGeometry(QgsGeometry.fromWkt(
            'Polygon ((-67.58750139 81.1909806, -66.30557012 81.24671674, -66.30557012 76.89929767, -67.58750139 76.89929767, -67.58750139 81.1909806))'))

        f3 = QgsFeature()
        f3.setAttributes([3])
        f3.setGeometry(QgsGeometry.fromWkt(
            'Polygon ((-68.36780737 75.78457483, -67.53176524 72.60761475, -68.64648808 73.66660144, -70.20710006 72.9420316, -68.36780737 75.78457483))'))

        f4 = QgsFeature()
        f4.setAttributes([4])

        cls.poly_provider.addFeatures([f1, f2, f3, f4])

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""

    def getEditableLayer(self):
        return self.createLayer()

    def testGetFeaturesSubsetAttributes2(self):
        """ Override and skip this test for memory provider, as it's actually more efficient for the memory provider to return
        its features as direct copies (due to implicit sharing of QgsFeature)
        """
        pass

    def testGetFeaturesNoGeometry(self):
        """ Override and skip this test for memory provider, as it's actually more efficient for the memory provider to return
        its features as direct copies (due to implicit sharing of QgsFeature)
        """
        pass

    def testCtors(self):
        testVectors = ["Point", "LineString", "Polygon", "MultiPoint", "MultiLineString", "MultiPolygon", "None"]
        for v in testVectors:
            layer = QgsVectorLayer(v, "test", "memory")
            assert layer.isValid(), "Failed to create valid %s memory layer" % (v)

    def testLayerGeometry(self):
        testVectors = [("Point", QgsWkbTypes.PointGeometry, QgsWkbTypes.Point),
                       ("LineString", QgsWkbTypes.LineGeometry, QgsWkbTypes.LineString),
                       ("Polygon", QgsWkbTypes.PolygonGeometry, QgsWkbTypes.Polygon),
                       ("MultiPoint", QgsWkbTypes.PointGeometry, QgsWkbTypes.MultiPoint),
                       ("MultiLineString", QgsWkbTypes.LineGeometry, QgsWkbTypes.MultiLineString),
                       ("MultiPolygon", QgsWkbTypes.PolygonGeometry, QgsWkbTypes.MultiPolygon),
                       ("PointZ", QgsWkbTypes.PointGeometry, QgsWkbTypes.PointZ),
                       ("LineStringZ", QgsWkbTypes.LineGeometry, QgsWkbTypes.LineStringZ),
                       ("PolygonZ", QgsWkbTypes.PolygonGeometry, QgsWkbTypes.PolygonZ),
                       ("MultiPointZ", QgsWkbTypes.PointGeometry, QgsWkbTypes.MultiPointZ),
                       ("MultiLineStringZ", QgsWkbTypes.LineGeometry, QgsWkbTypes.MultiLineStringZ),
                       ("MultiPolygonZ", QgsWkbTypes.PolygonGeometry, QgsWkbTypes.MultiPolygonZ),
                       ("PointM", QgsWkbTypes.PointGeometry, QgsWkbTypes.PointM),
                       ("LineStringM", QgsWkbTypes.LineGeometry, QgsWkbTypes.LineStringM),
                       ("PolygonM", QgsWkbTypes.PolygonGeometry, QgsWkbTypes.PolygonM),
                       ("MultiPointM", QgsWkbTypes.PointGeometry, QgsWkbTypes.MultiPointM),
                       ("MultiLineStringM", QgsWkbTypes.LineGeometry, QgsWkbTypes.MultiLineStringM),
                       ("MultiPolygonM", QgsWkbTypes.PolygonGeometry, QgsWkbTypes.MultiPolygonM),
                       ("PointZM", QgsWkbTypes.PointGeometry, QgsWkbTypes.PointZM),
                       ("LineStringZM", QgsWkbTypes.LineGeometry, QgsWkbTypes.LineStringZM),
                       ("PolygonZM", QgsWkbTypes.PolygonGeometry, QgsWkbTypes.PolygonZM),
                       ("MultiPointZM", QgsWkbTypes.PointGeometry, QgsWkbTypes.MultiPointZM),
                       ("MultiLineStringZM", QgsWkbTypes.LineGeometry, QgsWkbTypes.MultiLineStringZM),
                       ("MultiPolygonZM", QgsWkbTypes.PolygonGeometry, QgsWkbTypes.MultiPolygonZM),
                       ("Point25D", QgsWkbTypes.PointGeometry, QgsWkbTypes.Point25D),
                       ("LineString25D", QgsWkbTypes.LineGeometry, QgsWkbTypes.LineString25D),
                       ("Polygon25D", QgsWkbTypes.PolygonGeometry, QgsWkbTypes.Polygon25D),
                       ("MultiPoint25D", QgsWkbTypes.PointGeometry, QgsWkbTypes.MultiPoint25D),
                       ("MultiLineString25D", QgsWkbTypes.LineGeometry, QgsWkbTypes.MultiLineString25D),
                       ("MultiPolygon25D", QgsWkbTypes.PolygonGeometry, QgsWkbTypes.MultiPolygon25D),
                       ("None", QgsWkbTypes.NullGeometry, QgsWkbTypes.NoGeometry)]
        for v in testVectors:
            layer = QgsVectorLayer(v[0], "test", "memory")

            myMessage = ('Expected: %s\nGot: %s\n' %
                         (v[1], layer.geometryType()))
            assert layer.geometryType() == v[1], myMessage

            myMessage = ('Expected: %s\nGot: %s\n' %
                         (v[2], layer.wkbType()))
            assert layer.wkbType() == v[2], myMessage

    def testAddFeatures(self):
        layer = QgsVectorLayer("Point", "test", "memory")
        provider = layer.dataProvider()

        res = provider.addAttributes([QgsField("name", QVariant.String),
                                      QgsField("age", QVariant.Int),
                                      QgsField("size", QVariant.Double)])
        assert res, "Failed to add attributes"

        myMessage = ('Expected: %s\nGot: %s\n' %
                     (3, len(provider.fields())))

        assert len(provider.fields()) == 3, myMessage

        ft = QgsFeature()
        ft.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(10, 10)))
        ft.setAttributes(["Johny",
                          20,
                          0.3])
        res, t = provider.addFeatures([ft])

        assert res, "Failed to add feature"

        myMessage = ('Expected: %s\nGot: %s\n' %
                     (1, provider.featureCount()))
        assert provider.featureCount() == 1, myMessage

        for f in provider.getFeatures(QgsFeatureRequest()):
            myMessage = ('Expected: %s\nGot: %s\n' %
                         ("Johny", f[0]))

            assert f[0] == "Johny", myMessage

            myMessage = ('Expected: %s\nGot: %s\n' %
                         (20, f[1]))

            assert f[1] == 20, myMessage

            myMessage = ('Expected: %s\nGot: %s\n' %
                         (0.3, f[2]))

            assert (f[2] - 0.3) < 0.0000001, myMessage

            geom = f.geometry()

            myMessage = ('Expected: %s\nGot: %s\n' %
                         ("Point (10 10)", str(geom.asWkt())))

            assert compareWkt(str(geom.asWkt()), "Point (10 10)"), myMessage

    def testCloneFeatures(self):
        """
        Test that cloning a memory layer also clones features
        """
        vl = QgsVectorLayer(
            'Point?crs=epsg:4326&field=f1:integer&field=f2:integer',
            'test', 'memory')
        self.assertTrue(vl.isValid())

        f1 = QgsFeature()
        f1.setAttributes([5, -200])
        f2 = QgsFeature()
        f2.setAttributes([3, 300])
        f3 = QgsFeature()
        f3.setAttributes([1, 100])
        res, [f1, f2, f3] = vl.dataProvider().addFeatures([f1, f2, f3])
        self.assertEqual(vl.featureCount(), 3)

        vl2 = vl.clone()
        self.assertEqual(vl2.featureCount(), 3)
        features = [f for f in vl2.getFeatures()]
        self.assertTrue([f for f in features if f['f1'] == 5])
        self.assertTrue([f for f in features if f['f1'] == 3])
        self.assertTrue([f for f in features if f['f1'] == 1])

    def testCloneId(self):
        """Test that a cloned layer has a single new id and
        the same fields as the source layer"""

        vl = QgsVectorLayer(
            'Point?crs=epsg:4326',
            'test', 'memory')
        self.assertTrue(vl.isValid)
        dp = vl.dataProvider()
        self.assertTrue(dp.addAttributes([QgsField("name", QVariant.String),
                                          QgsField("age", QVariant.Int),
                                          QgsField("size", QVariant.Double)]))
        vl2 = vl.clone()
        self.assertTrue(
            'memory?geometry=Point&crs=EPSG:4326&field=name:string(0,0)&field=age:integer(0,0)&field=size:double(0,0)' in vl2.publicSource())
        self.assertEqual(len(parse_qs(vl.publicSource())['uid']), 1)
        self.assertEqual(len(parse_qs(vl2.publicSource())['uid']), 1)
        self.assertNotEqual(parse_qs(vl2.publicSource())['uid'][0], parse_qs(vl.publicSource())['uid'][0])

    def testGetFields(self):
        layer = QgsVectorLayer("Point", "test", "memory")
        provider = layer.dataProvider()

        provider.addAttributes([QgsField("name", QVariant.String),
                                QgsField("age", QVariant.Int),
                                QgsField("size", QVariant.Double),
                                QgsField("vallist", QVariant.List, subType=QVariant.Int),
                                QgsField("stringlist", QVariant.List, subType=QVariant.String),
                                QgsField("reallist", QVariant.List, subType=QVariant.Double),
                                QgsField("longlist", QVariant.List, subType=QVariant.LongLong),
                                QgsField("dict", QVariant.Map)])
        self.assertEqual(len(provider.fields()), 8)
        self.assertEqual(provider.fields()[0].name(), "name")
        self.assertEqual(provider.fields()[0].type(), QVariant.String)
        self.assertEqual(provider.fields()[0].subType(), QVariant.Invalid)
        self.assertEqual(provider.fields()[1].name(), "age")
        self.assertEqual(provider.fields()[1].type(), QVariant.Int)
        self.assertEqual(provider.fields()[1].subType(), QVariant.Invalid)
        self.assertEqual(provider.fields()[2].name(), "size")
        self.assertEqual(provider.fields()[2].type(), QVariant.Double)
        self.assertEqual(provider.fields()[2].subType(), QVariant.Invalid)
        self.assertEqual(provider.fields()[3].name(), "vallist")
        self.assertEqual(provider.fields()[3].type(), QVariant.List)
        self.assertEqual(provider.fields()[3].subType(), QVariant.Int)
        self.assertEqual(provider.fields()[4].name(), "stringlist")
        self.assertEqual(provider.fields()[4].type(), QVariant.List)
        self.assertEqual(provider.fields()[4].subType(), QVariant.String)
        self.assertEqual(provider.fields()[5].name(), "reallist")
        self.assertEqual(provider.fields()[5].type(), QVariant.List)
        self.assertEqual(provider.fields()[5].subType(), QVariant.Double)
        self.assertEqual(provider.fields()[6].name(), "longlist")
        self.assertEqual(provider.fields()[6].type(), QVariant.List)
        self.assertEqual(provider.fields()[6].subType(), QVariant.LongLong)
        self.assertEqual(provider.fields()[7].name(), "dict")
        self.assertEqual(provider.fields()[7].type(), QVariant.Map)

        ft = QgsFeature()
        ft.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(10, 10)))
        ft.setAttributes(["Johny",
                          20,
                          0.3,
                          [1, 2, 3],
                          ['a', 'b', 'c'],
                          [1.1, 2.2, 3.3],
                          [1, 2, 3],
                          {'a': 1, 'b': 2}])
        provider.addFeatures([ft])

        for f in provider.getFeatures(QgsFeatureRequest()):
            self.assertEqual(f.attributes(), ['Johny', 20, 0.3, [1, 2, 3], ['a', 'b', 'c'], [1.1, 2.2, 3.3], [1, 2, 3], {'a': 1, 'b': 2}])

    def testFromUri(self):
        """Test we can construct the mem provider from a uri"""
        myMemoryLayer = QgsVectorLayer(
            ('Point?crs=epsg:4326&field=name:string(20)&'
             'field=age:integer&field=size:double&index=yes'),
            'test',
            'memory')

        self.assertIsNotNone(myMemoryLayer)
        myProvider = myMemoryLayer.dataProvider()
        self.assertIsNotNone(myProvider)

    def testLengthPrecisionFromUri(self):
        """Test we can assign length and precision from a uri"""
        myMemoryLayer = QgsVectorLayer(
            ('Point?crs=epsg:4326&field=size:double(12,9)&index=yes'),
            'test',
            'memory')

        self.assertEqual(myMemoryLayer.fields().field('size').length(), 12)
        self.assertEqual(myMemoryLayer.fields().field('size').precision(), 9)

        myMemoryLayer = QgsVectorLayer(
            ('Point?crs=epsg:4326&field=size:double(-1,-1)&index=yes'),
            'test',
            'memory')

        self.assertEqual(myMemoryLayer.fields().field('size').length(), -1)
        self.assertEqual(myMemoryLayer.fields().field('size').precision(), -1)

        myMemoryLayer = QgsVectorLayer(
            ('Point?crs=epsg:4326&field=size:string(-1)&index=yes'),
            'test',
            'memory')

        self.assertEqual(myMemoryLayer.fields().field('size').length(), -1)

    def testListFromUri(self):
        """Test we can create list type fields from a uri"""
        myMemoryLayer = QgsVectorLayer(
            ('Point?crs=epsg:4326&field=a:string(-1)[]&index=yes'),
            'test',
            'memory')

        self.assertEqual(myMemoryLayer.fields().field('a').type(), QVariant.StringList)
        self.assertEqual(myMemoryLayer.fields().field('a').subType(), QVariant.String)

        myMemoryLayer = QgsVectorLayer(
            ('Point?crs=epsg:4326&field=a:double(-1,-1)[]&index=yes'),
            'test',
            'memory')

        self.assertEqual(myMemoryLayer.fields().field('a').type(), QVariant.List)
        self.assertEqual(myMemoryLayer.fields().field('a').subType(), QVariant.Double)

        myMemoryLayer = QgsVectorLayer(
            ('Point?crs=epsg:4326&field=a:long(-1,-1)[]&index=yes'),
            'test',
            'memory')

        self.assertEqual(myMemoryLayer.fields().field('a').type(), QVariant.List)
        self.assertEqual(myMemoryLayer.fields().field('a').subType(), QVariant.LongLong)

        myMemoryLayer = QgsVectorLayer(
            ('Point?crs=epsg:4326&field=a:int(-1,-1)[]&index=yes'),
            'test',
            'memory')

        self.assertEqual(myMemoryLayer.fields().field('a').type(), QVariant.List)
        self.assertEqual(myMemoryLayer.fields().field('a').subType(), QVariant.Int)

    def testMapFromUri(self):
        """Test we can create map type fields from a uri"""
        layer = QgsVectorLayer(
            ('Point?crs=epsg:4326&field=a:map(-1)&index=yes'),
            'test',
            'memory')
        self.assertEqual(layer.fields().field('a').type(), QVariant.Map)

    def testFromUriWithEncodedField(self):
        """Test we can construct the mem provider from a uri when a field name is encoded"""
        layer = QgsVectorLayer(
            ('Point?crs=epsg:4326&field=name:string(20)&'
             'field=test%2Ffield:integer'),
            'test',
            'memory')
        self.assertTrue(layer.isValid())
        self.assertEqual([f.name() for f in layer.fields()], ['name', 'test/field'])

    def testSaveFields(self):
        # Create a new memory layer with no fields
        myMemoryLayer = QgsVectorLayer(
            ('Point?crs=epsg:4326&index=yes'),
            'test',
            'memory')

        # Add some fields to the layer
        myFields = [QgsField('TestInt', QVariant.Int, 'integer', 2, 0),
                    QgsField('TestLong', QVariant.LongLong, 'long', -1, 0),
                    QgsField('TestDbl', QVariant.Double, 'double', 8, 6),
                    QgsField('TestString', QVariant.String, 'string', 50, 0),
                    QgsField('TestDate', QVariant.Date, 'date'),
                    QgsField('TestTime', QVariant.Time, 'time'),
                    QgsField('TestDateTime', QVariant.DateTime, 'datetime'),
                    QgsField("vallist", QVariant.List, subType=QVariant.Int),
                    QgsField("stringlist", QVariant.StringList, subType=QVariant.String),
                    QgsField("stringlist2", QVariant.List, subType=QVariant.String),
                    QgsField("reallist", QVariant.List, subType=QVariant.Double),
                    QgsField("longlist", QVariant.List, subType=QVariant.LongLong),
                    QgsField("dict", QVariant.Map)]
        self.assertTrue(myMemoryLayer.startEditing())
        for f in myFields:
            assert myMemoryLayer.addAttribute(f)
        self.assertTrue(myMemoryLayer.commitChanges())
        myMemoryLayer.updateFields()

        for f in myFields:
            self.assertEqual(f, myMemoryLayer.fields().field(f.name()))

        # Export the layer to a layer-definition-XML
        qlr = QgsLayerDefinition.exportLayerDefinitionLayers([myMemoryLayer], QgsReadWriteContext())
        self.assertIsNotNone(qlr)

        # Import the layer from the layer-definition-XML
        layers = QgsLayerDefinition.loadLayerDefinitionLayers(qlr, QgsReadWriteContext())
        self.assertTrue(layers)
        myImportedLayer = layers[0]
        self.assertIsNotNone(myImportedLayer)

        # Check for the presence of the fields
        importedFields = myImportedLayer.fields()
        for f in myFields:
            self.assertEqual(f.name(), importedFields.field(f.name()).name())
            if f.name() != 'stringlist2':
                self.assertEqual(f.type(), importedFields.field(f.name()).type())
            else:
                # we automatically convert List with String subtype to StringList, to match other data providers
                self.assertEqual(importedFields.field(f.name()).type(), QVariant.StringList)

            self.assertEqual(f.subType(), importedFields.field(f.name()).subType())
            self.assertEqual(f.precision(), importedFields.field(f.name()).precision())
            self.assertEqual(f.length(), importedFields.field(f.name()).length())

    def testRenameAttributes(self):
        layer = QgsVectorLayer("Point", "test", "memory")
        provider = layer.dataProvider()

        res = provider.addAttributes([QgsField("name", QVariant.String),
                                      QgsField("age", QVariant.Int),
                                      QgsField("size", QVariant.Double)])
        layer.updateFields()
        assert res, "Failed to add attributes"
        ft = QgsFeature()
        ft.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(10, 10)))
        ft.setAttributes(["Johny",
                          20,
                          0.3])
        res, t = provider.addFeatures([ft])

        # bad rename
        self.assertFalse(provider.renameAttributes({-1: 'not_a_field'}))
        self.assertFalse(provider.renameAttributes({100: 'not_a_field'}))
        # already exists
        self.assertFalse(provider.renameAttributes({1: 'name'}))

        # rename one field
        self.assertTrue(provider.renameAttributes({1: 'this_is_the_new_age'}))
        self.assertEqual(provider.fields().at(1).name(), 'this_is_the_new_age')
        layer.updateFields()
        fet = next(layer.getFeatures())
        self.assertEqual(fet.fields()[1].name(), 'this_is_the_new_age')

        # rename two fields
        self.assertTrue(provider.renameAttributes({1: 'mapinfo_is_the_stone_age', 2: 'super_size'}))
        self.assertEqual(provider.fields().at(1).name(), 'mapinfo_is_the_stone_age')
        self.assertEqual(provider.fields().at(2).name(), 'super_size')
        layer.updateFields()
        fet = next(layer.getFeatures())
        self.assertEqual(fet.fields()[1].name(), 'mapinfo_is_the_stone_age')
        self.assertEqual(fet.fields()[2].name(), 'super_size')

    def testUniqueSource(self):
        """
        Similar memory layers should have unique source - some code checks layer source to identify
        matching layers
        """
        layer = QgsVectorLayer("Point", "test", "memory")
        layer2 = QgsVectorLayer("Point", "test2", "memory")
        self.assertNotEqual(layer.source(), layer2.source())

    def testCreateMemoryLayer(self):
        """
        Test QgsMemoryProviderUtils.createMemoryLayer()
        """

        # no fields
        layer = QgsMemoryProviderUtils.createMemoryLayer('my name', QgsFields())
        self.assertTrue(layer.isValid())
        self.assertEqual(layer.name(), 'my name')
        self.assertTrue(layer.fields().isEmpty())

        # similar layers should have unique sources
        layer2 = QgsMemoryProviderUtils.createMemoryLayer('my name', QgsFields())
        self.assertNotEqual(layer.source(), layer2.source())

        # geometry type
        layer = QgsMemoryProviderUtils.createMemoryLayer('my name', QgsFields(), QgsWkbTypes.Point)
        self.assertTrue(layer.isValid())
        self.assertEqual(layer.wkbType(), QgsWkbTypes.Point)
        layer = QgsMemoryProviderUtils.createMemoryLayer('my name', QgsFields(), QgsWkbTypes.PolygonZM)
        self.assertTrue(layer.isValid())
        self.assertEqual(layer.wkbType(), QgsWkbTypes.PolygonZM)

        # crs
        layer = QgsMemoryProviderUtils.createMemoryLayer('my name', QgsFields(), QgsWkbTypes.PolygonZM,
                                                         QgsCoordinateReferenceSystem.fromEpsgId(3111))
        self.assertTrue(layer.isValid())
        self.assertEqual(layer.wkbType(), QgsWkbTypes.PolygonZM)
        self.assertTrue(layer.crs().isValid())
        self.assertEqual(layer.crs().authid(), 'EPSG:3111')

        # custom CRS
        crs = QgsCoordinateReferenceSystem.fromProj(
            '+proj=qsc +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84 +units=m +no_defs')
        layer = QgsMemoryProviderUtils.createMemoryLayer('my name', QgsFields(), QgsWkbTypes.PolygonZM, crs)
        self.assertTrue(layer.isValid())
        self.assertTrue(layer.crs().isValid())
        self.assertEqual(layer.crs().toProj(),
                         '+proj=qsc +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84 +units=m +no_defs +type=crs')

        # clone it, just to check
        layer2 = layer.clone()
        self.assertEqual(layer2.crs().toProj(),
                         '+proj=qsc +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84 +units=m +no_defs +type=crs')

        # fields
        fields = QgsFields()
        fields.append(QgsField("string", QVariant.String))
        fields.append(QgsField("long", QVariant.LongLong))
        fields.append(QgsField("double", QVariant.Double))
        fields.append(QgsField("integer", QVariant.Int))
        fields.append(QgsField("date", QVariant.Date))
        fields.append(QgsField("datetime", QVariant.DateTime))
        fields.append(QgsField("time", QVariant.Time))
        fields.append(QgsField("#complex_name", QVariant.String))
        fields.append(QgsField("complex/name", QVariant.String))
        fields.append(QgsField("binaryfield", QVariant.ByteArray))
        fields.append(QgsField("boolfield", QVariant.Bool))
        fields.append(QgsField("vallist", QVariant.List, subType=QVariant.Int))
        fields.append(QgsField("stringlist", QVariant.StringList, subType=QVariant.String))
        fields.append(QgsField("stringlist2", QVariant.List, subType=QVariant.String))
        fields.append(QgsField("reallist", QVariant.List, subType=QVariant.Double))
        fields.append(QgsField("longlist", QVariant.List, subType=QVariant.LongLong))
        fields.append(QgsField("dict", QVariant.Map))

        layer = QgsMemoryProviderUtils.createMemoryLayer('my name', fields)
        self.assertTrue(layer.isValid())
        self.assertFalse(layer.fields().isEmpty())
        self.assertEqual(len(layer.fields()), len(fields))
        for i in range(len(fields)):
            self.assertEqual(layer.fields()[i].name(), fields[i].name())
            if layer.fields()[i].name() != 'stringlist2':
                self.assertEqual(layer.fields()[i].type(), fields[i].type())
            else:
                # we automatically convert List with String subtype to StringList, to match other data providers
                self.assertEqual(layer.fields()[i].type(), QVariant.StringList)
            self.assertEqual(layer.fields()[i].length(), fields[i].length())
            self.assertEqual(layer.fields()[i].precision(), fields[i].precision(), fields[i].name())

        # unsupported field type
        fields = QgsFields()
        fields.append(QgsField("rect", QVariant.RectF))
        layer = QgsMemoryProviderUtils.createMemoryLayer('my name', fields)
        self.assertTrue(layer.isValid())
        self.assertFalse(layer.fields().isEmpty())
        self.assertEqual(layer.fields()[0].name(), 'rect')
        self.assertEqual(layer.fields()[0].type(), QVariant.String)  # should be mapped to string

        # field precision
        fields = QgsFields()
        fields.append(QgsField("string", QVariant.String, len=10))
        fields.append(QgsField("long", QVariant.LongLong, len=6))
        fields.append(QgsField("double", QVariant.Double, len=10, prec=7))
        fields.append(QgsField("double2", QVariant.Double, len=-1, prec=-1))
        layer = QgsMemoryProviderUtils.createMemoryLayer('my name', fields)
        self.assertTrue(layer.isValid())
        self.assertFalse(layer.fields().isEmpty())
        self.assertEqual(len(layer.fields()), len(fields))
        for i in range(len(fields)):
            self.assertEqual(layer.fields()[i].name(), fields[i].name())
            self.assertEqual(layer.fields()[i].type(), fields[i].type())
            self.assertEqual(layer.fields()[i].length(), fields[i].length())
            self.assertEqual(layer.fields()[i].precision(), fields[i].precision())

    def testAddChangeFeatureConvertAttribute(self):
        """
        Test add features with attribute values which require conversion
        """
        layer = QgsVectorLayer(
            'Point?crs=epsg:4326&index=yes&field=pk:integer&field=cnt:int8&field=dt:datetime', 'test', 'memory')
        provider = layer.dataProvider()
        f = QgsFeature()
        # string value specified for datetime field -- must be converted when adding the feature
        f.setAttributes([5, -200, '2021-02-10 00:00'])
        self.assertTrue(provider.addFeatures([f]))

        saved_feature = next(provider.getFeatures())
        # saved feature must have a QDateTime value for field, not string
        self.assertEqual(saved_feature.attributes(), [5, -200, QDateTime(2021, 2, 10, 0, 0)])

        self.assertTrue(provider.changeAttributeValues({saved_feature.id(): {2: '2021-02-12 00:00'}}))
        saved_feature = next(provider.getFeatures())
        # saved feature must have a QDateTime value for field, not string
        self.assertEqual(saved_feature.attributes(), [5, -200, QDateTime(2021, 2, 12, 0, 0)])

    def testThreadSafetyWithIndex(self):
        layer = QgsVectorLayer(
            'Point?crs=epsg:4326&index=yes&field=pk:integer&field=cnt:int8&field=name:string(0)&field=name2:string(0)&field=num_char:string&key=pk',
            'test', 'memory')

        provider = layer.dataProvider()
        f = QgsFeature()
        f.setAttributes([5, -200, NULL, 'NuLl', '5'])
        f.setGeometry(QgsGeometry.fromWkt('Point (-71.123 78.23)'))

        for i in range(100000):
            provider.addFeatures([f])

        # filter rect request
        extent = QgsRectangle(-73, 70, -63, 80)
        request = QgsFeatureRequest().setFilterRect(extent)
        self.assertTrue(QgsTestUtils.testProviderIteratorThreadSafety(self.source, request))

    def testMinMaxCache(self):
        """
        Test that min/max cache is appropriately cleared
        :return:
        """
        vl = QgsVectorLayer(
            'Point?crs=epsg:4326&field=f1:integer&field=f2:integer',
            'test', 'memory')
        self.assertTrue(vl.isValid())

        f1 = QgsFeature()
        f1.setAttributes([5, -200])
        f2 = QgsFeature()
        f2.setAttributes([3, 300])
        f3 = QgsFeature()
        f3.setAttributes([1, 100])
        f4 = QgsFeature()
        f4.setAttributes([2, 200])
        f5 = QgsFeature()
        f5.setAttributes([4, 400])
        res, [f1, f2, f3, f4, f5] = vl.dataProvider().addFeatures([f1, f2, f3, f4, f5])
        self.assertTrue(res)

        self.assertEqual(vl.dataProvider().minimumValue(0), 1)
        self.assertEqual(vl.dataProvider().minimumValue(1), -200)
        self.assertEqual(vl.dataProvider().maximumValue(0), 5)
        self.assertEqual(vl.dataProvider().maximumValue(1), 400)

        # add feature
        f6 = QgsFeature()
        f6.setAttributes([15, 1400])
        res, [f6] = vl.dataProvider().addFeatures([f6])
        self.assertTrue(res)
        self.assertEqual(vl.dataProvider().minimumValue(0), 1)
        self.assertEqual(vl.dataProvider().minimumValue(1), -200)
        self.assertEqual(vl.dataProvider().maximumValue(0), 15)
        self.assertEqual(vl.dataProvider().maximumValue(1), 1400)
        f7 = QgsFeature()
        f7.setAttributes([-1, -1400])
        res, [f7] = vl.dataProvider().addFeatures([f7])
        self.assertTrue(res)
        self.assertEqual(vl.dataProvider().minimumValue(0), -1)
        self.assertEqual(vl.dataProvider().minimumValue(1), -1400)
        self.assertEqual(vl.dataProvider().maximumValue(0), 15)
        self.assertEqual(vl.dataProvider().maximumValue(1), 1400)

        # change attribute values
        self.assertTrue(vl.dataProvider().changeAttributeValues({f6.id(): {0: 3, 1: 150}, f7.id(): {0: 4, 1: -100}}))
        self.assertEqual(vl.dataProvider().minimumValue(0), 1)
        self.assertEqual(vl.dataProvider().minimumValue(1), -200)
        self.assertEqual(vl.dataProvider().maximumValue(0), 5)
        self.assertEqual(vl.dataProvider().maximumValue(1), 400)

        # delete features
        self.assertTrue(vl.dataProvider().deleteFeatures([f4.id(), f1.id()]))
        self.assertEqual(vl.dataProvider().minimumValue(0), 1)
        self.assertEqual(vl.dataProvider().minimumValue(1), -100)
        self.assertEqual(vl.dataProvider().maximumValue(0), 4)
        self.assertEqual(vl.dataProvider().maximumValue(1), 400)

        # delete attributes
        self.assertTrue(vl.dataProvider().deleteAttributes([0]))
        self.assertEqual(vl.dataProvider().minimumValue(0), -100)
        self.assertEqual(vl.dataProvider().maximumValue(0), 400)

    def testBinary(self):
        vl = QgsVectorLayer(
            'Point?crs=epsg:4326&field=f1:integer&field=f2:binary',
            'test', 'memory')
        self.assertTrue(vl.isValid())

        dp = vl.dataProvider()
        fields = dp.fields()
        self.assertEqual([f.name() for f in fields], ['f1', 'f2'])
        self.assertEqual([f.type() for f in fields], [QVariant.Int, QVariant.ByteArray])
        self.assertEqual([f.typeName() for f in fields], ['integer', 'binary'])

        f = QgsFeature(dp.fields())
        bin_1 = b'xxx'
        bin_val1 = QByteArray(bin_1)
        f.setAttributes([1, bin_val1])
        self.assertTrue(dp.addFeature(f))

        f2 = [f for f in dp.getFeatures()][0]
        self.assertEqual(f2.attributes(), [1, bin_val1])

        # add binary field
        self.assertTrue(dp.addAttributes([QgsField('binfield2', QVariant.ByteArray, 'Binary')]))

        fields = dp.fields()
        bin2_field = fields[fields.lookupField('binfield2')]
        self.assertEqual(bin2_field.type(), QVariant.ByteArray)
        self.assertEqual(bin2_field.typeName(), 'Binary')

        f = QgsFeature(fields)
        bin_2 = b'yyy'
        bin_val2 = QByteArray(bin_2)
        f.setAttributes([2, NULL, bin_val2])
        self.assertTrue(dp.addFeature(f))

        f1 = [f for f in dp.getFeatures()][0]
        self.assertEqual(f1.attributes(), [1, bin_val1, NULL])
        f2 = [f for f in dp.getFeatures()][1]
        self.assertEqual(f2.attributes(), [2, NULL, bin_val2])

    def testBool(self):
        vl = QgsVectorLayer(
            'Point?crs=epsg:4326&field=f1:integer&field=f2:bool',
            'test', 'memory')
        self.assertTrue(vl.isValid())

        dp = vl.dataProvider()
        fields = dp.fields()
        self.assertEqual([f.name() for f in fields], ['f1', 'f2'])
        self.assertEqual([f.type() for f in fields], [QVariant.Int, QVariant.Bool])
        self.assertEqual([f.typeName() for f in fields], ['integer', 'boolean'])

        f = QgsFeature(dp.fields())
        f.setAttributes([1, True])
        f2 = QgsFeature(dp.fields())
        f2.setAttributes([2, False])
        f3 = QgsFeature(dp.fields())
        f3.setAttributes([3, NULL])
        self.assertTrue(dp.addFeatures([f, f2, f3]))

        self.assertEqual([f.attributes() for f in dp.getFeatures()], [[1, True], [2, False], [3, NULL]])

        # add boolean field
        self.assertTrue(dp.addAttributes([QgsField('boolfield2', QVariant.Bool, 'Boolean')]))

        fields = dp.fields()
        bool2_field = fields[fields.lookupField('boolfield2')]
        self.assertEqual(bool2_field.type(), QVariant.Bool)
        self.assertEqual(bool2_field.typeName(), 'Boolean')

        f = QgsFeature(fields)
        f.setAttributes([2, NULL, True])
        self.assertTrue(dp.addFeature(f))

        self.assertEqual([f.attributes() for f in dp.getFeatures()],
                         [[1, True, NULL], [2, False, NULL], [3, NULL, NULL], [2, NULL, True]])

    def testSpatialIndex(self):
        vl = QgsVectorLayer(
            'Point?crs=epsg:4326&field=f1:integer&field=f2:bool',
            'test', 'memory')
        self.assertEqual(vl.hasSpatialIndex(), QgsFeatureSource.SpatialIndexNotPresent)
        vl.dataProvider().createSpatialIndex()
        self.assertEqual(vl.hasSpatialIndex(), QgsFeatureSource.SpatialIndexPresent)

    def testTypeValidation(self):
        """Test that incompatible types in attributes raise errors"""

        vl = QgsVectorLayer(
            'Point?crs=epsg:4326&field=int:integer',
            'test', 'memory')
        self.assertTrue(vl.isValid())
        invalid = QgsFeature(vl.fields())
        invalid.setAttribute('int', 'A string')
        invalid.setGeometry(QgsGeometry.fromWkt('point(9 45)'))
        self.assertTrue(vl.startEditing())
        # Validation happens on commit
        self.assertTrue(vl.addFeatures([invalid]))
        self.assertFalse(vl.commitChanges())
        self.assertTrue(vl.rollBack())
        self.assertFalse(vl.hasFeatures())

        # Add a valid feature
        valid = QgsFeature(vl.fields())
        valid.setAttribute('int', 123)
        self.assertTrue(vl.startEditing())
        self.assertTrue(vl.addFeatures([valid]))
        self.assertTrue(vl.commitChanges())
        self.assertEqual(vl.featureCount(), 1)

        f = vl.getFeature(1)
        self.assertEqual(f.attribute('int'), 123)

        # Add both
        vl = QgsVectorLayer(
            'Point?crs=epsg:4326&field=int:integer',
            'test', 'memory')
        self.assertEqual(vl.featureCount(), 0)
        self.assertTrue(vl.startEditing())
        self.assertTrue(vl.addFeatures([valid, invalid]))
        self.assertFalse(vl.commitChanges())
        self.assertEqual(vl.featureCount(), 2)
        self.assertTrue(vl.rollBack())
        self.assertEqual(vl.featureCount(), 0)

        # Add both swapped
        vl = QgsVectorLayer(
            'Point?crs=epsg:4326&field=int:integer',
            'test', 'memory')
        self.assertTrue(vl.startEditing())
        self.assertTrue(vl.addFeatures([invalid, valid]))
        self.assertFalse(vl.commitChanges())
        self.assertEqual(vl.featureCount(), 2)
        self.assertTrue(vl.rollBack())
        self.assertEqual(vl.featureCount(), 0)

        # Change attribute value
        vl = QgsVectorLayer(
            'Point?crs=epsg:4326&field=int:integer',
            'test', 'memory')
        self.assertTrue(vl.startEditing())
        self.assertTrue(vl.addFeatures([valid]))
        self.assertTrue(vl.commitChanges())
        self.assertTrue(vl.startEditing())
        self.assertTrue(vl.changeAttributeValue(1, 0, 'A string'))
        self.assertFalse(vl.commitChanges())
        f = vl.getFeature(1)
        self.assertEqual(f.attribute('int'), 'A string')
        self.assertTrue(vl.rollBack())

        f = vl.getFeature(1)
        self.assertEqual(f.attribute('int'), 123)

        # Change attribute values
        vl = QgsVectorLayer(
            'Point?crs=epsg:4326&field=int:integer',
            'test', 'memory')
        self.assertTrue(vl.startEditing())
        self.assertTrue(vl.addFeatures([valid]))
        self.assertTrue(vl.commitChanges())
        self.assertTrue(vl.startEditing())
        self.assertTrue(vl.changeAttributeValues(1, {0: 'A string'}))
        self.assertFalse(vl.commitChanges())
        f = vl.getFeature(1)
        self.assertEqual(f.attribute('int'), 'A string')
        self.assertTrue(vl.rollBack())

        f = vl.getFeature(1)
        self.assertEqual(f.attribute('int'), 123)

        ##############################################
        # Test direct data provider calls

        # No rollback (old behavior)
        vl = QgsVectorLayer(
            'Point?crs=epsg:4326&field=int:integer',
            'test', 'memory')
        dp = vl.dataProvider()
        self.assertFalse(dp.addFeatures([valid, invalid])[0])
        self.assertEqual([f.attributes() for f in dp.getFeatures()], [[123]])
        f = vl.getFeature(1)
        self.assertEqual(f.attribute('int'), 123)

        # Roll back
        vl = QgsVectorLayer(
            'Point?crs=epsg:4326&field=int:integer',
            'test', 'memory')
        dp = vl.dataProvider()
        self.assertFalse(dp.addFeatures([valid, invalid], QgsFeatureSink.RollBackOnErrors)[0])
        self.assertFalse(dp.hasFeatures())

        # Expected behavior for changeAttributeValues is to always roll back
        self.assertTrue(dp.addFeatures([valid])[0])
        self.assertFalse(dp.changeAttributeValues({1: {0: 'A string'}}))
        f = vl.getFeature(1)
        self.assertEqual(f.attribute('int'), 123)

    def testAddAttributes(self):
        """Test that fields with empty/invalid typenames are updated to native type names"""

        vl = QgsVectorLayer("Point", "temporary_points", "memory")
        pr = vl.dataProvider()

        # add fields
        pr.addAttributes([QgsField("name", QVariant.String),
                          QgsField("age", QVariant.Int, "invalidInteger"),
                          QgsField("size", QVariant.Double),
                          QgsField("mytext", QVariant.String, "text"),
                          QgsField("size2", QVariant.Double, "double precision"),
                          QgsField("short", QVariant.Int, "int2"),
                          QgsField("lessshort", QVariant.Int, "int4"),
                          QgsField("numericfield", QVariant.Double, "numeric"),
                          QgsField("decimalfield", QVariant.Double, "decimal"),
                          QgsField("stringlistfield", QVariant.StringList, "stringlist"),
                          QgsField("integerlistfield", QVariant.List, "integerlist"),
                          QgsField("doublelistfield", QVariant.List, "doublelist"),
                          QgsField("dict", QVariant.Map)])

        self.assertEqual(pr.fields()[0].typeName(), "string")
        self.assertEqual(pr.fields()[1].typeName(), "integer")
        self.assertEqual(pr.fields()[2].typeName(), "double")
        self.assertEqual(pr.fields()[3].typeName(), "text")
        self.assertEqual(pr.fields()[4].typeName(), "double precision")
        self.assertEqual(pr.fields()[5].typeName(), "int2")
        self.assertEqual(pr.fields()[6].typeName(), "int4")
        self.assertEqual(pr.fields()[7].typeName(), "numeric")
        self.assertEqual(pr.fields()[8].typeName(), "decimal")
        self.assertEqual(pr.fields()[9].typeName(), "stringlist")
        self.assertEqual(pr.fields()[10].typeName(), "integerlist")
        self.assertEqual(pr.fields()[11].typeName(), "doublelist")
        self.assertEqual(pr.fields()[12].typeName(), "map")

        vl2 = vl.clone()

        self.assertEqual(pr.fields()[0].name(), vl2.fields()[0].name())
        self.assertEqual(pr.fields()[1].name(), vl2.fields()[1].name())
        self.assertEqual(pr.fields()[2].name(), vl2.fields()[2].name())
        self.assertEqual(pr.fields()[3].name(), vl2.fields()[3].name())
        self.assertEqual(pr.fields()[4].name(), vl2.fields()[4].name())
        self.assertEqual(pr.fields()[5].name(), vl2.fields()[5].name())
        self.assertEqual(pr.fields()[6].name(), vl2.fields()[6].name())
        self.assertEqual(pr.fields()[7].name(), vl2.fields()[7].name())
        self.assertEqual(pr.fields()[8].name(), vl2.fields()[8].name())
        self.assertEqual(pr.fields()[9].name(), vl2.fields()[9].name())
        self.assertEqual(pr.fields()[10].name(), vl2.fields()[10].name())
        self.assertEqual(pr.fields()[11].name(), vl2.fields()[11].name())
        self.assertEqual(pr.fields()[12].name(), vl2.fields()[12].name())

        self.assertEqual(pr.fields()[0].typeName(), vl2.fields()[0].typeName())
        self.assertEqual(pr.fields()[1].typeName(), vl2.fields()[1].typeName())
        self.assertEqual(pr.fields()[2].typeName(), vl2.fields()[2].typeName())
        self.assertEqual(pr.fields()[3].typeName(), vl2.fields()[3].typeName())
        self.assertEqual(pr.fields()[4].typeName(), vl2.fields()[4].typeName())
        self.assertEqual(pr.fields()[5].typeName(), vl2.fields()[5].typeName())
        self.assertEqual(pr.fields()[6].typeName(), vl2.fields()[6].typeName())
        self.assertEqual(pr.fields()[7].typeName(), vl2.fields()[7].typeName())
        self.assertEqual(pr.fields()[8].typeName(), vl2.fields()[8].typeName())
        self.assertEqual(pr.fields()[9].typeName(), vl2.fields()[9].typeName())
        self.assertEqual(pr.fields()[10].typeName(), vl2.fields()[10].typeName())
        self.assertEqual(pr.fields()[11].typeName(), vl2.fields()[11].typeName())
        self.assertEqual(pr.fields()[12].typeName(), vl2.fields()[12].typeName())


class TestPyQgsMemoryProviderIndexed(unittest.TestCase, ProviderTestCase):
    """Runs the provider test suite against an indexed memory layer"""

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        # Create test layer
        cls.vl = QgsVectorLayer(
            'Point?crs=epsg:4326&field=pk:integer&field=cnt:integer&field=name:string(0)&field=name2:string(0)&field=num_char:string&field=dt:datetime&field=date:date&field=time:time&key=pk',
            'test', 'memory')
        assert (cls.vl.isValid())
        cls.source = cls.vl.dataProvider()

        f1 = QgsFeature()
        f1.setAttributes(
            [5, -200, NULL, 'NuLl', '5', QDateTime(QDate(2020, 5, 4), QTime(12, 13, 14)), QDate(2020, 5, 2),
             QTime(12, 13, 1)])
        f1.setGeometry(QgsGeometry.fromWkt('Point (-71.123 78.23)'))

        f2 = QgsFeature()
        f2.setAttributes([3, 300, 'Pear', 'PEaR', '3', NULL, NULL, NULL])

        f3 = QgsFeature()
        f3.setAttributes(
            [1, 100, 'Orange', 'oranGe', '1', QDateTime(QDate(2020, 5, 3), QTime(12, 13, 14)), QDate(2020, 5, 3),
             QTime(12, 13, 14)])
        f3.setGeometry(QgsGeometry.fromWkt('Point (-70.332 66.33)'))

        f4 = QgsFeature()
        f4.setAttributes(
            [2, 200, 'Apple', 'Apple', '2', QDateTime(QDate(2020, 5, 4), QTime(12, 14, 14)), QDate(2020, 5, 4),
             QTime(12, 14, 14)])
        f4.setGeometry(QgsGeometry.fromWkt('Point (-68.2 70.8)'))

        f5 = QgsFeature()
        f5.setAttributes(
            [4, 400, 'Honey', 'Honey', '4', QDateTime(QDate(2021, 5, 4), QTime(13, 13, 14)), QDate(2021, 5, 4),
             QTime(13, 13, 14)])
        f5.setGeometry(QgsGeometry.fromWkt('Point (-65.32 78.3)'))

        cls.source.addFeatures([f1, f2, f3, f4, f5])

        # poly layer
        cls.poly_vl = QgsVectorLayer('Polygon?crs=epsg:4326&index=yes&field=pk:integer&key=pk',
                                     'test', 'memory')
        assert (cls.poly_vl.isValid())
        cls.poly_provider = cls.poly_vl.dataProvider()

        f1 = QgsFeature()
        f1.setAttributes([1])
        f1.setGeometry(QgsGeometry.fromWkt(
            'Polygon ((-69.0 81.4, -69.0 80.2, -73.7 80.2, -73.7 76.3, -74.9 76.3, -74.9 81.4, -69.0 81.4))'))

        f2 = QgsFeature()
        f2.setAttributes([2])
        f2.setGeometry(QgsGeometry.fromWkt('Polygon ((-67.6 81.2, -66.3 81.2, -66.3 76.9, -67.6 76.9, -67.6 81.2))'))

        f3 = QgsFeature()
        f3.setAttributes([3])
        f3.setGeometry(QgsGeometry.fromWkt('Polygon ((-68.4 75.8, -67.5 72.6, -68.6 73.7, -70.2 72.9, -68.4 75.8))'))

        f4 = QgsFeature()
        f4.setAttributes([4])

        cls.poly_provider.addFeatures([f1, f2, f3, f4])

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""

    def testGetFeaturesSubsetAttributes2(self):
        """ Override and skip this test for memory provider, as it's actually more efficient for the memory provider to return
        its features as direct copies (due to implicit sharing of QgsFeature)
        """
        pass

    def testGetFeaturesNoGeometry(self):
        """ Override and skip this test for memory provider, as it's actually more efficient for the memory provider to return
        its features as direct copies (due to implicit sharing of QgsFeature)
        """
        pass


if __name__ == '__main__':
    unittest.main()
