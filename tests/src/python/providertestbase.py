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
        assert len([f for f in provider.getFeatures(QgsFeatureRequest().setFilterExpression('name IS NOT NULL'))]) == 4
        assert len(
            [f for f in provider.getFeatures(QgsFeatureRequest().setFilterExpression('name LIKE \'Apple\''))]) == 1
        assert len(
            [f for f in provider.getFeatures(QgsFeatureRequest().setFilterExpression('name ILIKE \'aPple\''))]) == 1
        assert len(
            [f for f in provider.getFeatures(QgsFeatureRequest().setFilterExpression('name ILIKE \'%pp%\''))]) == 1
        assert len([f for f in provider.getFeatures(QgsFeatureRequest().setFilterExpression('cnt > 0'))]) == 4
        assert len([f for f in provider.getFeatures(QgsFeatureRequest().setFilterExpression('cnt < 0'))]) == 1
        assert len([f for f in provider.getFeatures(QgsFeatureRequest().setFilterExpression('cnt >= 100'))]) == 4
        assert len([f for f in provider.getFeatures(QgsFeatureRequest().setFilterExpression('cnt <= 100'))]) == 2
        assert len(
            [f for f in provider.getFeatures(QgsFeatureRequest().setFilterExpression('pk IN (1, 2, 4, 8)'))]) == 3

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
        assert set([u'Apple', u'Honey', u'Orange', u'Pear', NULL]) == set(
            self.provider.uniqueValues(2)), 'Got {}'.format(set(self.provider.uniqueValues(2)))

    def testFeatureCount(self):
        assert self.provider.featureCount() == 5
