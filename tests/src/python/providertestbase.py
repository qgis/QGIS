# -*- coding: utf-8 -*-
"""QGIS Unit test utils for provider tests.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Matthias Kuhn'
__date__ = '2015-04-27'
__copyright__ = 'Copyright 2015, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from qgis.core import QgsRectangle, QgsFeatureRequest, QgsGeometry, NULL

class ProviderTestCase(object):
    def assert_query(self, provider, expression, expected):
        result = set([f['pk'] for f in provider.getFeatures(QgsFeatureRequest().setFilterExpression(expression))])
        assert set(expected) == result, 'Expected {} and got {} when testing expression "{}"'.format(set(expected), result, expression)

    '''
        This is a collection of tests for vector data providers and kept generic.
        To make use of it, subclass it and set self.provider to a provider you want to test.
        Make sure that your provider uses the default dataset by converting one of the provided datasets from the folder
        tests/testdata/provider to a dataset your provider is able to handle.

        To test expression compilation, add the methods `enableCompiler()` and `disableCompiler()` to your subclass.
        If these methods are present, the tests will ensure that the result of server side and client side expression
        evaluation are equal.
    '''
    def runGetFeatureTests(self, provider):
        assert len([f for f in provider.getFeatures()]) == 5
        self.assert_query(provider, 'name ILIKE \'QGIS\'', [])
        self.assert_query(provider, '"name" IS NULL', [5])
        self.assert_query(provider, '"name" IS NOT NULL', [1, 2, 3, 4])
        self.assert_query(provider, '"name" NOT LIKE \'Ap%\'', [1, 3, 4])
        self.assert_query(provider, '"name" NOT ILIKE \'QGIS\'', [1, 2, 3, 4])
        self.assert_query(provider, '"name" NOT ILIKE \'pEAR\'', [1, 2, 4])
        self.assert_query(provider, 'name LIKE \'Apple\'', [2])
        self.assert_query(provider, 'name ILIKE \'aPple\'', [2])
        self.assert_query(provider, 'name ILIKE \'%pp%\'', [2])
        self.assert_query(provider, 'cnt > 0', [1, 2, 3, 4])
        self.assert_query(provider, 'cnt < 0', [5])
        self.assert_query(provider, 'cnt >= 100', [1, 2, 3, 4])
        self.assert_query(provider, 'cnt <= 100', [1, 5])
        self.assert_query(provider, 'pk IN (1, 2, 4, 8)', [1, 2, 4])
        self.assert_query(provider, 'cnt = 50 * 2', [1])
        self.assert_query(provider, 'cnt = 99 + 1', [1])
        self.assert_query(provider, 'cnt = 1100 % 1000', [1])
        self.assert_query(provider, '"name" || \' \' || "cnt" = \'Orange 100\'', [1])
        self.assert_query(provider, 'cnt = 10 ^ 2', [1])
        self.assert_query(provider, '"name" ~ \'[OP]ra[gne]+\'', [1])
        self.assert_query(provider, 'true', [1, 2, 3, 4, 5])


    def testGetFeaturesUncompiled(self):
        try:
            self.disableCompiler()
        except AttributeError:
            pass
        self.runGetFeatureTests(self.provider)

    def testGetFeaturesCompiled(self):
        try:
            self.enableCompiler()
            self.runGetFeatureTests(self.provider)
        except AttributeError:
            print 'Provider does not support compiling'

    def testGetFeaturesFilterRectTests(self):
        extent = QgsRectangle(-70, 67, -60, 80)
        features = [f['pk'] for f in self.provider.getFeatures(QgsFeatureRequest().setFilterRect(extent))]
        assert set(features) == set([2L, 4L]), 'Got {} instead'.format(features)

    def testRectAndExpression(self):
        extent = QgsRectangle(-70, 67, -60, 80)
        result = set([f['pk'] for f in self.provider.getFeatures(
            QgsFeatureRequest()
                .setFilterExpression('"cnt">200')
                .setFilterRect(extent))])
        expected=[4L]
        assert set(expected) == result, 'Expected {} and got {} when testing for combination of filterRect and expression'.format(set(expected), result)

    def testMinValue(self):
        assert self.provider.minimumValue(1) == -200
        assert self.provider.minimumValue(2) == 'Apple'

    def testMaxValue(self):
        assert self.provider.maximumValue(1) == 400
        assert self.provider.maximumValue(2) == 'Pear'

    def testExtent(self):
        reference = QgsGeometry.fromRect(
            QgsRectangle(-71.123, 66.33, -65.32, 78.3))
        provider_extent = QgsGeometry.fromRect(self.provider.extent())

        assert QgsGeometry.compare(provider_extent.asPolygon(), reference.asPolygon(), 0.000001)

    def testUnique(self):
        assert set(self.provider.uniqueValues(1)) == set([-200, 100, 200, 300, 400])
        assert set([u'Apple', u'Honey', u'Orange', u'Pear', NULL]) == set(self.provider.uniqueValues(2)), 'Got {}'.format(set(self.provider.uniqueValues(2)))

    def testFeatureCount(self):
        assert self.provider.featureCount() == 5, 'Got {}'.format( self.provider.featureCount() )
