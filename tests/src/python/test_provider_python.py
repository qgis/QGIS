# -*- coding: utf-8 -*-
"""QGIS Unit tests for the python dataprovider.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Alessandro Pasotti'
__date__ = '2018-03-18'
__copyright__ = 'Copyright 2018, The QGIS Project'

# -*- coding: utf-8 -*-
"""QGIS Unit tests for the py layerprovider.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Matthias Kuhn'
__date__ = '2015-04-23'
__copyright__ = 'Copyright 2015, The QGIS Project'


import os

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
    QgsProviderMetadata,
    QgsProviderRegistry,
)

from qgis.testing import (
    start_app,
    unittest
)

from utilities import (
    unitTestDataPath,
    compareWkt
)

from provider_python import PyProvider

from providertestbase import ProviderTestCase
from qgis.PyQt.QtCore import QVariant

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestPyQgsPythonProvider(unittest.TestCase, ProviderTestCase):

    @classmethod
    def createLayer(cls):
        vl = QgsVectorLayer(
            'Point?crs=epsg:4326&field=pk:integer&field=cnt:integer&field=name:string(0)&field=name2:string(0)&field=num_char:string&key=pk',
            'test', 'pythonprovider')
        assert (vl.isValid())

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

        vl.dataProvider().addFeatures([f1, f2, f3, f4, f5])
        return vl

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        # Register the provider
        r = QgsProviderRegistry.instance()
        metadata = QgsProviderMetadata(PyProvider.providerKey(), PyProvider.description(), PyProvider.createProvider)
        assert r.registerProvider(metadata)
        assert r.providerMetadata(PyProvider.providerKey()) == metadata

        # Create test layer
        cls.vl = cls.createLayer()
        assert (cls.vl.isValid())
        cls.source = cls.vl.dataProvider()

        # poly layer
        cls.poly_vl = QgsVectorLayer('Polygon?crs=epsg:4326&field=pk:integer&key=pk',
                                     'test', 'pythonprovider')
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

    def getEditableLayer(self):
        return self.createLayer()

    def testGetFeaturesSubsetAttributes2(self):
        """ Override and skip this test for pythonprovider provider, as it's actually more efficient for the pythonprovider provider to return
        its features as direct copies (due to implicit sharing of QgsFeature)
        """
        pass

    def testGetFeaturesNoGeometry(self):
        """ Override and skip this test for pythonprovider provider, as it's actually more efficient for the pythonprovider provider to return
        its features as direct copies (due to implicit sharing of QgsFeature)
        """
        pass

    def testGetFeaturesDestinationCrs(self):
        """Skip this if on travis, passes locally and fails with no reason on Travis"""
        super().testGetFeaturesDestinationCrs()

    def testCtors(self):
        testVectors = ["Point", "LineString", "Polygon", "MultiPoint", "MultiLineString", "MultiPolygon", "None"]
        for v in testVectors:
            layer = QgsVectorLayer(v, "test", "pythonprovider")
            assert layer.isValid(), "Failed to create valid %s pythonprovider layer" % (v)

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
            layer = QgsVectorLayer(v[0], "test", "pythonprovider")

            myMessage = ('Expected: %s\nGot: %s\n' %
                         (v[1], layer.geometryType()))
            assert layer.geometryType() == v[1], myMessage

            myMessage = ('Expected: %s\nGot: %s\n' %
                         (v[2], layer.wkbType()))
            assert layer.wkbType() == v[2], myMessage

    def testAddFeatures(self):
        layer = QgsVectorLayer("Point", "test", "pythonprovider")
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

    def testGetFields(self):
        layer = QgsVectorLayer("Point", "test", "pythonprovider")
        provider = layer.dataProvider()

        provider.addAttributes([QgsField("name", QVariant.String),
                                QgsField("age", QVariant.Int),
                                QgsField("size", QVariant.Double)])
        myMessage = ('Expected: %s\nGot: %s\n' %
                     (3, len(provider.fields())))

        assert len(provider.fields()) == 3, myMessage

        ft = QgsFeature()
        ft.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(10, 10)))
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
        myPyLayer = QgsVectorLayer(
            ('Point?crs=epsg:4326&field=name:string(20)&'
             'field=age:integer&field=size:double&index=yes'),
            'test',
            'pythonprovider')

        assert myPyLayer is not None, 'Provider not initialized'
        myProvider = myPyLayer.dataProvider()
        assert myProvider is not None

    def testLengthPrecisionFromUri(self):
        """Test we can assign length and precision from a uri"""
        myPyLayer = QgsVectorLayer(
            ('Point?crs=epsg:4326&field=size:double(12,9)&index=yes'),
            'test',
            'pythonprovider')

        self.assertEqual(myPyLayer.fields().field('size').length(), 12)
        self.assertEqual(myPyLayer.fields().field('size').precision(), 9)

    @unittest.expectedFailure("Handled layers are hardcoded")
    def testSaveFields(self):
        # Create a new py layerwith no fields
        myPyLayer = QgsVectorLayer(
            ('Point?crs=epsg:4326&index=yes'),
            'test',
            'pythonprovider')

        # Add some fields to the layer
        myFields = [QgsField('TestInt', QVariant.Int, 'integer', 2, 0),
                    QgsField('TestLong', QVariant.LongLong, 'long', -1, 0),
                    QgsField('TestDbl', QVariant.Double, 'double', 8, 6),
                    QgsField('TestString', QVariant.String, 'string', 50, 0),
                    QgsField('TestDate', QVariant.Date, 'date'),
                    QgsField('TestTime', QVariant.Time, 'time'),
                    QgsField('TestDateTime', QVariant.DateTime, 'datetime')]
        assert myPyLayer.startEditing()
        for f in myFields:
            assert myPyLayer.addAttribute(f)
        assert myPyLayer.commitChanges()
        myPyLayer.updateFields()

        # Export the layer to a layer-definition-XML
        qlr = QgsLayerDefinition.exportLayerDefinitionLayers([myPyLayer], QgsReadWriteContext())
        assert qlr is not None

        # Import the layer from the layer-definition-XML
        layers = QgsLayerDefinition.loadLayerDefinitionLayers(qlr, QgsReadWriteContext())
        assert layers is not None
        myImportedLayer = layers[0]
        assert myImportedLayer is not None

        # Check for the presence of the fields
        importedFields = myImportedLayer.fields()
        assert importedFields is not None
        for f in myFields:
            assert f == importedFields.field(f.name())

    def testRenameAttributes(self):
        layer = QgsVectorLayer("Point", "test", "pythonprovider")
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

    def testThreadSafetyWithIndex(self):
        layer = QgsVectorLayer('Point?crs=epsg:4326&index=yes&field=pk:integer&field=cnt:int8&field=name:string(0)&field=name2:string(0)&field=num_char:string&key=pk',
                               'test', 'pythonprovider')

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

    def tesRegisterSameProviderTwice(self):
        """Test that a provider cannot be registered twice"""
        r = QgsProviderRegistry.instance()
        metadata = QgsProviderMetadata(PyProvider.providerKey(), PyProvider.description(), PyProvider.createProvider)
        self.assertFalse(r.registerProvider(metadata))


if __name__ == '__main__':
    unittest.main()
