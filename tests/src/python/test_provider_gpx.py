# -*- coding: utf-8 -*-
"""QGIS Unit tests for the GPX provider.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '2021-07-30'
__copyright__ = 'Copyright 2021, The QGIS Project'

from qgis.core import (
    QgsVectorLayer,
    QgsFeature,
    QgsPoint,
    QgsProviderRegistry
)
from qgis.testing import (
    start_app,
    unittest
)

from providertestbase import ProviderTestCase
from utilities import (
    unitTestDataPath
)

start_app()


class TestPyQgsGpxProvider(unittest.TestCase, ProviderTestCase):

    @classmethod
    def createLayer(cls):
        vl = QgsVectorLayer(
            f'{unitTestDataPath()}/gpx_test_suite.gpx?type=waypoint',
            'test', 'gpx')
        assert (vl.isValid())
        return vl

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        # Create test layer
        cls.vl = cls.createLayer()
        assert (cls.vl.isValid())
        cls.source = cls.vl.dataProvider()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""

    @property
    def pk_name(self):
        """Return the primary key name, override if different than the default 'pk'"""
        return 'comment'

    @unittest.skip('Base provider test is not suitable for GPX provider')
    def testGetFeatures(self):
        pass

    @unittest.skip('Base provider test is not suitable for GPX provider')
    def testGetFeaturesDestinationCrs(self):
        pass

    @unittest.skip('Base provider test is not suitable for GPX provider')
    def testGetFeaturesLimit(self):
        pass

    @unittest.skip('Base provider test is not suitable for GPX provider')
    def testGetFeaturesSubsetAttributes(self):
        pass

    @unittest.skip('Base provider test is not suitable for GPX provider')
    def testGetFeaturesWithGeometry(self):
        pass

    @unittest.skip('Base provider test is not suitable for GPX provider')
    def testOrderBy(self):
        pass

    @unittest.skip('Base provider test is not suitable for GPX provider')
    def testRectAndFids(self):
        pass

    @unittest.skip('Base provider test is not suitable for GPX provider')
    def testCloneLayer(self):
        pass

    @unittest.skip('Base provider test is not suitable for GPX provider')
    def testExtent(self):
        pass

    @unittest.skip('Base provider test is not suitable for GPX provider')
    def testFeatureCount(self):
        pass

    @unittest.skip('Base provider test is not suitable for GPX provider')
    def testGetFeaturesFilterRectTests(self):
        pass

    @unittest.skip('Base provider test is not suitable for GPX provider')
    def testGetFeaturesFilterRectTestsNoGeomFlag(self):
        pass

    @unittest.skip('Base provider test is not suitable for GPX provider')
    def testGetFeaturesDistanceWithinTests(self):
        pass

    @unittest.skip('Base provider test is not suitable for GPX provider')
    def testFields(self):
        pass

    @unittest.skip('Base provider test is not suitable for GPX provider')
    def testGeomAndAllAttributes(self):
        pass

    @unittest.skip('Base provider test is not suitable for GPX provider')
    def testGetFeaturesFidTests(self):
        pass

    @unittest.skip('Base provider test is not suitable for GPX provider')
    def testGetFeaturesFidsTests(self):
        pass

    @unittest.skip('Base provider test is not suitable for GPX provider')
    def testGetFeaturesSubsetAttributes2(self):
        pass

    @unittest.skip('Base provider test is not suitable for GPX provider')
    def testGetFeaturesUncompiled(self):
        pass

    @unittest.skip('Base provider test is not suitable for GPX provider')
    def testMaxValue(self):
        pass

    @unittest.skip('Base provider test is not suitable for GPX provider')
    def testMaximumValue(self):
        pass

    @unittest.skip('Base provider test is not suitable for GPX provider')
    def testMinValue(self):
        pass

    @unittest.skip('Base provider test is not suitable for GPX provider')
    def testMinimumValue(self):
        pass

    @unittest.skip('Base provider test is not suitable for GPX provider')
    def testRectAndExpression(self):
        pass

    @unittest.skip('Base provider test is not suitable for GPX provider')
    def testStringComparison(self):
        pass

    @unittest.skip('Base provider test is not suitable for GPX provider')
    def testUnique(self):
        pass

    @unittest.skip('Base provider test is not suitable for GPX provider')
    def testUniqueStringsMatching(self):
        pass

    @unittest.skip('Base provider test is not suitable for GPX provider')
    def testUniqueValues(self):
        pass

    def test_invalid_source(self):
        """
        Test various methods with an invalid source
        """
        vl = QgsVectorLayer('not a gpx?type=waypoint', 'test', 'gpx')
        self.assertFalse(vl.isValid())
        self.assertEqual(vl.featureCount(), -1)
        self.assertTrue(vl.extent().isNull())

        f = QgsFeature()
        f.setGeometry(QgsPoint(1, 2))
        self.assertFalse(vl.dataProvider().addFeature(f))
        self.assertFalse(vl.dataProvider().addFeatures([f])[0])

        self.assertFalse(vl.dataProvider().deleteFeatures([1, 2]))

        self.assertFalse(vl.dataProvider().changeAttributeValues({1: {1: 'a'}}))

    def test_encode_decode_uri(self):
        metadata = QgsProviderRegistry.instance().providerMetadata('gpx')
        self.assertIsNotNone(metadata)

        self.assertEqual(metadata.encodeUri({}), '')
        self.assertEqual(metadata.decodeUri(''), {})
        self.assertEqual(metadata.encodeUri({'path': '/home/me/test.gpx'}), '/home/me/test.gpx')
        self.assertEqual(metadata.decodeUri('/home/me/test.gpx'), {'path': '/home/me/test.gpx'})
        self.assertEqual(metadata.encodeUri({'path': '/home/me/test.gpx',
                                             'layerName': 'waypoints'}), '/home/me/test.gpx?type=waypoints')
        self.assertEqual(metadata.decodeUri('/home/me/test.gpx?type=waypoints'), {'path': '/home/me/test.gpx',
                                                                                  'layerName': 'waypoints'})
        self.assertEqual(metadata.encodeUri({'path': '/home/me/test.gpx',
                                             'layerName': 'tracks'}), '/home/me/test.gpx?type=tracks')
        self.assertEqual(metadata.decodeUri('/home/me/test.gpx?type=tracks'), {'path': '/home/me/test.gpx',
                                                                               'layerName': 'tracks'})
        self.assertEqual(metadata.encodeUri({'path': '/home/me/test.gpx',
                                             'layerName': 'routes'}), '/home/me/test.gpx?type=routes')
        self.assertEqual(metadata.decodeUri('/home/me/test.gpx?type=routes'), {'path': '/home/me/test.gpx',
                                                                               'layerName': 'routes'})


if __name__ == '__main__':
    unittest.main()
