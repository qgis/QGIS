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
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'


from qgis.core import (
    QGis,
    QgsField,
    QgsPoint,
    QgsMapLayer,
    QgsVectorLayer,
    QgsFeatureRequest,
    QgsFeature,
    QgsGeometry,
    NULL
)

from qgis.testing import (
    start_app,
    unittest
)

from utilities import (
    unitTestDataPath,
    compareWkt
)

from providertestbase import ProviderTestCase
from qgis.PyQt.QtCore import QVariant

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestPyQgsMemoryProvider(unittest.TestCase, ProviderTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        # Create test layer
        cls.vl = QgsVectorLayer(u'Point?crs=epsg:4326&field=pk:integer&field=cnt:integer&field=name:string(0)&field=name2:string(0)&field=num_char:string&key=pk',
                                u'test', u'memory')
        assert (cls.vl.isValid())
        cls.provider = cls.vl.dataProvider()

        f1 = QgsFeature()
        f1.setAttributes([5, -200, NULL, 'NuLl', '5'])
        f1.setGeometry(QgsGeometry.fromWkt('Point (-71.123 78.23)'))

        f2 = QgsFeature()
        f2.setAttributes([3, 300, 'Pear', 'PEaR', '3'])

        f3 = QgsFeature()
        f3.setAttributes([1, 100, 'Orange', 'oranGe', '1'])
        f3.setGeometry(QgsGeometry.fromWkt('Point (-70.332 66.33)'))

        f4 = QgsFeature()
        f4.setAttributes([2, 200, 'Apple', 'Apple', '2'])
        f4.setGeometry(QgsGeometry.fromWkt('Point (-68.2 70.8)'))

        f5 = QgsFeature()
        f5.setAttributes([4, 400, 'Honey', 'Honey', '4'])
        f5.setGeometry(QgsGeometry.fromWkt('Point (-65.32 78.3)'))

        cls.provider.addFeatures([f1, f2, f3, f4, f5])

        # poly layer
        cls.poly_vl = QgsVectorLayer(u'Polygon?crs=epsg:4326&field=pk:integer&key=pk',
                                     u'test', u'memory')
        assert (cls.poly_vl.isValid())
        cls.poly_provider = cls.poly_vl.dataProvider()

        f1 = QgsFeature()
        f1.setAttributes([1])
        f1.setGeometry(QgsGeometry.fromWkt('Polygon ((-69.03664108 81.35818902, -69.09237722 80.24346619, -73.718477 80.1319939, -73.718477 76.28620011, -74.88893598 76.34193625, -74.83319983 81.35818902, -69.03664108 81.35818902))'))

        f2 = QgsFeature()
        f2.setAttributes([2])
        f2.setGeometry(QgsGeometry.fromWkt('Polygon ((-67.58750139 81.1909806, -66.30557012 81.24671674, -66.30557012 76.89929767, -67.58750139 76.89929767, -67.58750139 81.1909806))'))

        f3 = QgsFeature()
        f3.setAttributes([3])
        f3.setGeometry(QgsGeometry.fromWkt('Polygon ((-68.36780737 75.78457483, -67.53176524 72.60761475, -68.64648808 73.66660144, -70.20710006 72.9420316, -68.36780737 75.78457483))'))

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

    def testCtors(self):
        testVectors = ["Point", "LineString", "Polygon", "MultiPoint", "MultiLineString", "MultiPolygon", "None"]
        for v in testVectors:
            layer = QgsVectorLayer(v, "test", "memory")
            assert layer.isValid(), "Failed to create valid %s memory layer" % (v)

    def testLayerGeometry(self):
        testVectors = [("Point", QGis.Point, QGis.WKBPoint),
                       ("LineString", QGis.Line, QGis.WKBLineString),
                       ("Polygon", QGis.Polygon, QGis.WKBPolygon),
                       ("MultiPoint", QGis.Point, QGis.WKBMultiPoint),
                       ("MultiLineString", QGis.Line, QGis.WKBMultiLineString),
                       ("MultiPolygon", QGis.Polygon, QGis.WKBMultiPolygon),
                       ("None", QGis.NoGeometry, QGis.WKBNoGeometry)]
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

        res = provider.addAttributes([QgsField("name", QVariant.String, ),
                                      QgsField("age", QVariant.Int),
                                      QgsField("size", QVariant.Double)])
        assert res, "Failed to add attributes"

        myMessage = ('Expected: %s\nGot: %s\n' %
                     (3, len(provider.fields())))

        assert len(provider.fields()) == 3, myMessage

        ft = QgsFeature()
        ft.setGeometry(QgsGeometry.fromPoint(QgsPoint(10, 10)))
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
                         ("Point (10 10)", str(geom.exportToWkt())))

            assert compareWkt(str(geom.exportToWkt()), "Point (10 10)"), myMessage

    def testGetFields(self):
        layer = QgsVectorLayer("Point", "test", "memory")
        provider = layer.dataProvider()

        provider.addAttributes([QgsField("name", QVariant.String, ),
                                QgsField("age", QVariant.Int),
                                QgsField("size", QVariant.Double)])
        myMessage = ('Expected: %s\nGot: %s\n' %
                     (3, len(provider.fields())))

        assert len(provider.fields()) == 3, myMessage

        ft = QgsFeature()
        ft.setGeometry(QgsGeometry.fromPoint(QgsPoint(10, 10)))
        ft.setAttributes(["Johny",
                          20,
                          0.3])
        provider.addFeatures([ft])

        for f in provider.getFeatures(QgsFeatureRequest()):
            myMessage = ('Expected: %s\nGot: %s\n' %
                         ("Johny", f['name']))

            self.assertEqual(f["name"], "Johny", myMessage)

    def testFromUri(self):
        """Test we can construct the mem provider from a uri"""
        myMemoryLayer = QgsVectorLayer(
            ('Point?crs=epsg:4326&field=name:string(20)&'
             'field=age:integer&field=size:double&index=yes'),
            'test',
            'memory')

        assert myMemoryLayer is not None, 'Provider not initialized'
        myProvider = myMemoryLayer.dataProvider()
        assert myProvider is not None

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
                    QgsField('TestDateTime', QVariant.DateTime, 'datetime')]
        assert myMemoryLayer.startEditing()
        for f in myFields:
            assert myMemoryLayer.addAttribute(f)
        assert myMemoryLayer.commitChanges()
        myMemoryLayer.updateFields()

        # Export the layer to a layer-definition-XML
        qlr = QgsMapLayer.asLayerDefinition([myMemoryLayer])
        assert qlr is not None

        # Import the layer from the layer-definition-XML
        layers = QgsMapLayer.fromLayerDefinition(qlr)
        assert layers is not None
        myImportedLayer = layers[0]
        assert myImportedLayer is not None

        # Check for the presence of the fields
        importedFields = myImportedLayer.fields()
        assert importedFields is not None
        for f in myFields:
            assert f == importedFields.field(f.name())

    def testRenameAttributes(self):
        layer = QgsVectorLayer("Point", "test", "memory")
        provider = layer.dataProvider()

        res = provider.addAttributes([QgsField("name", QVariant.String, ),
                                      QgsField("age", QVariant.Int),
                                      QgsField("size", QVariant.Double)])
        layer.updateFields()
        assert res, "Failed to add attributes"
        ft = QgsFeature()
        ft.setGeometry(QgsGeometry.fromPoint(QgsPoint(10, 10)))
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


class TestPyQgsMemoryProviderIndexed(unittest.TestCase, ProviderTestCase):

    """Runs the provider test suite against an indexed memory layer"""

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        # Create test layer
        cls.vl = QgsVectorLayer(u'Point?crs=epsg:4326&index=yes&field=pk:integer&field=cnt:int8&field=name:string(0)&field=name2:string(0)&field=num_char:string&key=pk',
                                u'test', u'memory')
        assert (cls.vl.isValid())
        cls.provider = cls.vl.dataProvider()

        f1 = QgsFeature()
        f1.setAttributes([5, -200, NULL, 'NuLl', '5'])
        f1.setGeometry(QgsGeometry.fromWkt('Point (-71.123 78.23)'))

        f2 = QgsFeature()
        f2.setAttributes([3, 300, 'Pear', 'PEaR', '3'])

        f3 = QgsFeature()
        f3.setAttributes([1, 100, 'Orange', 'oranGe', '1'])
        f3.setGeometry(QgsGeometry.fromWkt('Point (-70.332 66.33)'))

        f4 = QgsFeature()
        f4.setAttributes([2, 200, 'Apple', 'Apple', '2'])
        f4.setGeometry(QgsGeometry.fromWkt('Point (-68.2 70.8)'))

        f5 = QgsFeature()
        f5.setAttributes([4, 400, 'Honey', 'Honey', '4'])
        f5.setGeometry(QgsGeometry.fromWkt('Point (-65.32 78.3)'))

        cls.provider.addFeatures([f1, f2, f3, f4, f5])

        # poly layer
        cls.poly_vl = QgsVectorLayer(u'Polygon?crs=epsg:4326&index=yes&field=pk:integer&key=pk',
                                     u'test', u'memory')
        assert (cls.poly_vl.isValid())
        cls.poly_provider = cls.poly_vl.dataProvider()

        f1 = QgsFeature()
        f1.setAttributes([1])
        f1.setGeometry(QgsGeometry.fromWkt('Polygon ((-69.0 81.4, -69.0 80.2, -73.7 80.2, -73.7 76.3, -74.9 76.3, -74.9 81.4, -69.0 81.4))'))

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
