# -*- coding: utf-8 -*-
"""QGIS Unit test utils for provider tests.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from builtins import str
from builtins import object

__author__ = 'Matthias Kuhn'
__date__ = '2015-04-27'
__copyright__ = 'Copyright 2015, The QGIS Project'

from qgis.core import (
    QgsApplication,
    QgsRectangle,
    QgsFeatureRequest,
    QgsFeature,
    QgsGeometry,
    QgsAbstractFeatureIterator,
    QgsExpressionContextScope,
    QgsExpressionContext,
    QgsExpression,
    QgsVectorDataProvider,
    QgsVectorLayerFeatureSource,
    QgsFeatureSink,
    QgsTestUtils,
    QgsFeatureSource,
    QgsFieldConstraints,
    QgsDataProvider,
    QgsVectorLayerUtils,
    NULL
)
from qgis.PyQt.QtCore import QDate, QTime, QDateTime, QVariant
from qgis.PyQt.QtTest import QSignalSpy

from utilities import compareWkt
from featuresourcetestbase import FeatureSourceTestCase


class ProviderTestCase(FeatureSourceTestCase):
    '''
        This is a collection of tests for vector data providers and kept generic.
        To make use of it, subclass it and set self.source to a provider you want to test.
        Make sure that your provider uses the default dataset by converting one of the provided datasets from the folder
        tests/testdata/provider to a dataset your provider is able to handle.

        To test expression compilation, add the methods `enableCompiler()` and `disableCompiler()` to your subclass.
        If these methods are present, the tests will ensure that the result of server side and client side expression
        evaluation are equal.

        To enable constraints checks for a data provider, please see the comment to the specific tests:
        - testChangeAttributesConstraintViolation
        - testUniqueNotNullConstraints

    '''

    def uncompiledFilters(self):
        """ Individual derived provider tests should override this to return a list of expressions which
        cannot be compiled """
        return set()

    def enableCompiler(self):
        """By default there is no expression compiling available, needs to be overridden in subclass"""
        print('Provider does not support compiling')
        return False

    def partiallyCompiledFilters(self):
        """ Individual derived provider tests should override this to return a list of expressions which
        should be partially compiled """
        return set()

    @property
    def pk_name(self):
        """Return the primary key name, override if different than the default 'pk'"""
        return 'pk'

    def assert_query(self, source, expression, expected):
        FeatureSourceTestCase.assert_query(self, source, expression, expected)

        if self.compiled:
            # Check compilation status
            it = source.getFeatures(QgsFeatureRequest().setFilterExpression(expression).setFlags(QgsFeatureRequest.IgnoreStaticNodesDuringExpressionCompilation))

            if expression in self.uncompiledFilters():
                self.assertEqual(it.compileStatus(), QgsAbstractFeatureIterator.NoCompilation)
            elif expression in self.partiallyCompiledFilters():
                self.assertEqual(it.compileStatus(), QgsAbstractFeatureIterator.PartiallyCompiled)
            else:
                self.assertEqual(it.compileStatus(), QgsAbstractFeatureIterator.Compiled, expression)

    def runGetFeatureTests(self, source):
        FeatureSourceTestCase.runGetFeatureTests(self, source)

        # combination of an uncompilable expression and limit
        feature = next(self.vl.getFeatures('pk=4'))
        context = QgsExpressionContext()
        scope = QgsExpressionContextScope()
        scope.setVariable('parent', feature)
        context.appendScope(scope)

        request = QgsFeatureRequest()
        request.setExpressionContext(context)
        request.setFilterExpression('"pk" = attribute(@parent, \'pk\')').setFlags(QgsFeatureRequest.IgnoreStaticNodesDuringExpressionCompilation)
        request.setLimit(1)

        values = [f[self.pk_name] for f in self.vl.getFeatures(request)]
        self.assertEqual(values, [4])

    def runPolyGetFeatureTests(self, provider):
        assert len([f for f in provider.getFeatures()]) == 4

        # geometry
        self.assert_query(provider, 'x($geometry) < -70', [1])
        self.assert_query(provider, 'y($geometry) > 79', [1, 2])
        self.assert_query(provider, 'xmin($geometry) < -70', [1, 3])
        self.assert_query(provider, 'ymin($geometry) < 76', [3])
        self.assert_query(provider, 'xmax($geometry) > -68', [2, 3])
        self.assert_query(provider, 'ymax($geometry) > 80', [1, 2])
        self.assert_query(provider, 'area($geometry) > 10', [1])
        self.assert_query(provider, 'perimeter($geometry) < 12', [2, 3])
        self.assert_query(provider,
                          'relate($geometry,geom_from_wkt( \'Polygon ((-68.2 82.1, -66.95 82.1, -66.95 79.05, -68.2 79.05, -68.2 82.1))\')) = \'FF2FF1212\'',
                          [1, 3])
        self.assert_query(provider,
                          'relate($geometry,geom_from_wkt( \'Polygon ((-68.2 82.1, -66.95 82.1, -66.95 79.05, -68.2 79.05, -68.2 82.1))\'), \'****F****\')',
                          [1, 3])
        self.assert_query(provider,
                          'crosses($geometry,geom_from_wkt( \'Linestring (-68.2 82.1, -66.95 82.1, -66.95 79.05)\'))',
                          [2])
        self.assert_query(provider,
                          'overlaps($geometry,geom_from_wkt( \'Polygon ((-68.2 82.1, -66.95 82.1, -66.95 79.05, -68.2 79.05, -68.2 82.1))\'))',
                          [2])
        self.assert_query(provider,
                          'within($geometry,geom_from_wkt( \'Polygon ((-75.1 76.1, -75.1 81.6, -68.8 81.6, -68.8 76.1, -75.1 76.1))\'))',
                          [1])
        self.assert_query(provider,
                          'overlaps(translate($geometry,-1,-1),geom_from_wkt( \'Polygon ((-75.1 76.1, -75.1 81.6, -68.8 81.6, -68.8 76.1, -75.1 76.1))\'))',
                          [1])
        self.assert_query(provider,
                          'overlaps(buffer($geometry,1),geom_from_wkt( \'Polygon ((-75.1 76.1, -75.1 81.6, -68.8 81.6, -68.8 76.1, -75.1 76.1))\'))',
                          [1, 3])
        self.assert_query(provider,
                          'intersects(centroid($geometry),geom_from_wkt( \'Polygon ((-74.4 78.2, -74.4 79.1, -66.8 79.1, -66.8 78.2, -74.4 78.2))\'))',
                          [2])
        self.assert_query(provider,
                          'intersects(point_on_surface($geometry),geom_from_wkt( \'Polygon ((-74.4 78.2, -74.4 79.1, -66.8 79.1, -66.8 78.2, -74.4 78.2))\'))',
                          [1, 2])
        self.assert_query(provider, 'distance($geometry,geom_from_wkt( \'Point (-70 70)\')) > 7', [1, 2])

    def testGetFeaturesUncompiled(self):
        self.compiled = False
        try:
            self.disableCompiler()
        except AttributeError:
            pass
        self.runGetFeatureTests(self.source)
        if hasattr(self, 'poly_provider'):
            self.runPolyGetFeatureTests(self.poly_provider)

    def testGetFeaturesExp(self):
        if self.enableCompiler():
            self.compiled = True
            self.runGetFeatureTests(self.source)
            if hasattr(self, 'poly_provider'):
                self.runPolyGetFeatureTests(self.poly_provider)

    def testSubsetString(self):
        if not self.source.supportsSubsetString():
            print('Provider does not support subset strings')
            return

        changed_spy = QSignalSpy(self.source.dataChanged)
        subset = self.getSubsetString()
        self.source.setSubsetString(subset)
        self.assertEqual(self.source.subsetString(), subset)
        self.assertEqual(len(changed_spy), 1)

        # No signal should be emitted if the subset string is not modified
        self.source.setSubsetString(subset)
        self.assertEqual(len(changed_spy), 1)

        result = set([f[self.pk_name] for f in self.source.getFeatures()])
        all_valid = (all(f.isValid() for f in self.source.getFeatures()))
        self.source.setSubsetString(None)

        expected = set([2, 3, 4])
        assert set(expected) == result, 'Expected {} and got {} when testing subset string {}'.format(set(expected),
                                                                                                      result, subset)
        self.assertTrue(all_valid)

        # Subset string AND filter rect
        self.source.setSubsetString(subset)
        extent = QgsRectangle(-70, 70, -60, 75)
        request = QgsFeatureRequest().setFilterRect(extent)
        result = set([f[self.pk_name] for f in self.source.getFeatures(request)])
        all_valid = (all(f.isValid() for f in self.source.getFeatures(request)))
        self.source.setSubsetString(None)
        expected = set([2])
        assert set(expected) == result, 'Expected {} and got {} when testing subset string {}'.format(set(expected),
                                                                                                      result, subset)
        self.assertTrue(all_valid)

        # Subset string AND filter rect, version 2
        self.source.setSubsetString(subset)
        extent = QgsRectangle(-71, 65, -60, 80)
        result = set([f[self.pk_name] for f in self.source.getFeatures(QgsFeatureRequest().setFilterRect(extent))])
        self.source.setSubsetString(None)
        expected = set([2, 4])
        assert set(expected) == result, 'Expected {} and got {} when testing subset string {}'.format(set(expected),
                                                                                                      result, subset)

        # Subset string AND expression
        self.source.setSubsetString(subset)
        request = QgsFeatureRequest().setFilterExpression('length("name")=5')
        result = set([f[self.pk_name] for f in self.source.getFeatures(request)])
        all_valid = (all(f.isValid() for f in self.source.getFeatures(request)))
        self.source.setSubsetString(None)
        expected = set([2, 4])
        assert set(expected) == result, 'Expected {} and got {} when testing subset string {}'.format(set(expected),
                                                                                                      result, subset)
        self.assertTrue(all_valid)

        # Subset string AND filter fid
        ids = {f[self.pk_name]: f.id() for f in self.source.getFeatures()}
        self.source.setSubsetString(subset)
        request = QgsFeatureRequest().setFilterFid(4)
        result = set([f.id() for f in self.source.getFeatures(request)])
        all_valid = (all(f.isValid() for f in self.source.getFeatures(request)))
        self.source.setSubsetString(None)
        expected = set([4])
        assert set(expected) == result, 'Expected {} and got {} when testing subset string {}'.format(set(expected),
                                                                                                      result, subset)
        self.assertTrue(all_valid)

        # Subset string AND filter fids
        self.source.setSubsetString(subset)
        request = QgsFeatureRequest().setFilterFids([ids[2], ids[4]])
        result = set([f.id() for f in self.source.getFeatures(request)])
        all_valid = (all(f.isValid() for f in self.source.getFeatures(request)))
        self.source.setSubsetString(None)
        expected = set([ids[2], ids[4]])
        assert set(expected) == result, 'Expected {} and got {} when testing subset string {}'.format(set(expected),
                                                                                                      result, subset)
        self.assertTrue(all_valid)

    def getSubsetString(self):
        """Individual providers may need to override this depending on their subset string formats"""
        return '"cnt" > 100 and "cnt" < 410'

    def getSubsetString2(self):
        """Individual providers may need to override this depending on their subset string formats"""
        return '"cnt" > 100 and "cnt" < 400'

    def getSubsetString3(self):
        """Individual providers may need to override this depending on their subset string formats"""
        return '"name"=\'Apple\''

    def getSubsetStringNoMatching(self):
        """Individual providers may need to override this depending on their subset string formats"""
        return '"name"=\'AppleBearOrangePear\''

    def testGetFeaturesThreadSafety(self):
        # no request
        self.assertTrue(QgsTestUtils.testProviderIteratorThreadSafety(self.source))

        # filter rect request
        extent = QgsRectangle(-73, 70, -63, 80)
        request = QgsFeatureRequest().setFilterRect(extent)
        self.assertTrue(QgsTestUtils.testProviderIteratorThreadSafety(self.source, request))

    def testOrderBy(self):
        try:
            self.disableCompiler()
        except AttributeError:
            pass
        self.runOrderByTests()

    def testOrderByCompiled(self):
        if self.enableCompiler():
            self.runOrderByTests()

    def runOrderByTests(self):
        FeatureSourceTestCase.runOrderByTests(self)

        # Combination with subset of attributes
        request = QgsFeatureRequest().addOrderBy('num_char', False).setSubsetOfAttributes([self.pk_name], self.vl.fields())
        values = [f[self.pk_name] for f in self.vl.getFeatures(request)]
        self.assertEqual(values, [5, 4, 3, 2, 1])

    def testOpenIteratorAfterLayerRemoval(self):
        """
        Test that removing layer after opening an iterator does not crash. All required
        information should be captured in the iterator's source and there MUST be no
        links between the iterators and the layer's data provider
        """
        if not getattr(self, 'getEditableLayer', None):
            return

        l = self.getEditableLayer()
        self.assertTrue(l.isValid())

        # store the source
        source = QgsVectorLayerFeatureSource(l)

        # delete the layer
        del l

        # get the features
        pks = []
        for f in source.getFeatures():
            pks.append(f[self.pk_name])
        self.assertEqual(set(pks), {1, 2, 3, 4, 5})

    def testCloneLayer(self):
        """
        Test that cloning layer works and has all expected features
        """
        l = self.vl.clone()

        pks = []
        for f in l.getFeatures():
            pks.append(f[self.pk_name])
        self.assertEqual(set(pks), {1, 2, 3, 4, 5})

    def testGetFeaturesPolyFilterRectTests(self):
        """ Test fetching features from a polygon layer with filter rect"""
        try:
            if not self.poly_provider:
                return
        except:
            return

        extent = QgsRectangle(-73, 70, -63, 80)
        request = QgsFeatureRequest().setFilterRect(extent)
        features = [f[self.pk_name] for f in self.poly_provider.getFeatures(request)]
        all_valid = (all(f.isValid() for f in self.source.getFeatures(request)))
        # Some providers may return the exact intersection matches (2, 3) even without the ExactIntersect flag, so we accept that too
        assert set(features) == set([2, 3]) or set(features) == set([1, 2, 3]), 'Got {} instead'.format(features)
        self.assertTrue(all_valid)

        # Test with exact intersection
        request = QgsFeatureRequest().setFilterRect(extent).setFlags(QgsFeatureRequest.ExactIntersect)
        features = [f[self.pk_name] for f in self.poly_provider.getFeatures(request)]
        all_valid = (all(f.isValid() for f in self.source.getFeatures(request)))
        assert set(features) == set([2, 3]), 'Got {} instead'.format(features)
        self.assertTrue(all_valid)

        # test with an empty rectangle
        extent = QgsRectangle()
        features = [f[self.pk_name] for f in self.source.getFeatures(QgsFeatureRequest().setFilterRect(extent))]
        assert set(features) == set([1, 2, 3, 4, 5]), 'Got {} instead'.format(features)

    def testMinValue(self):
        self.assertFalse(self.source.minimumValue(-1))
        self.assertFalse(self.source.minimumValue(1000))

        self.assertEqual(self.source.minimumValue(self.source.fields().lookupField('cnt')), -200)
        self.assertEqual(self.source.minimumValue(self.source.fields().lookupField('name')), 'Apple')

        if self.treat_datetime_as_string():
            self.assertEqual(self.source.minimumValue(self.source.fields().lookupField('dt')), '2020-05-03 12:13:14')
        else:
            self.assertEqual(self.source.minimumValue(self.source.fields().lookupField('dt')),
                             QDateTime(QDate(2020, 5, 3), QTime(12, 13, 14)))

        if self.treat_date_as_string():
            self.assertEqual(self.source.minimumValue(self.source.fields().lookupField('date')), '2020-05-02')
        elif not self.treat_date_as_datetime():
            self.assertEqual(self.source.minimumValue(self.source.fields().lookupField('date')), QDate(2020, 5, 2))
        else:
            self.assertEqual(self.source.minimumValue(self.source.fields().lookupField('date')),
                             QDateTime(2020, 5, 2, 0, 0, 0))

        if not self.treat_time_as_string():
            self.assertEqual(self.source.minimumValue(self.source.fields().lookupField('time')), QTime(12, 13, 1))
        else:
            self.assertEqual(self.source.minimumValue(self.source.fields().lookupField('time')), '12:13:01')

        if self.source.supportsSubsetString():
            subset = self.getSubsetString()
            self.source.setSubsetString(subset)
            min_value = self.source.minimumValue(self.source.fields().lookupField('cnt'))
            self.source.setSubsetString(None)
            self.assertEqual(min_value, 200)

    def testMaxValue(self):
        self.assertFalse(self.source.maximumValue(-1))
        self.assertFalse(self.source.maximumValue(1000))
        self.assertEqual(self.source.maximumValue(self.source.fields().lookupField('cnt')), 400)
        self.assertEqual(self.source.maximumValue(self.source.fields().lookupField('name')), 'Pear')

        if not self.treat_datetime_as_string():
            self.assertEqual(self.source.maximumValue(self.source.fields().lookupField('dt')),
                             QDateTime(QDate(2021, 5, 4), QTime(13, 13, 14)))
        else:
            self.assertEqual(self.source.maximumValue(self.source.fields().lookupField('dt')), '2021-05-04 13:13:14')

        if self.treat_date_as_string():
            self.assertEqual(self.source.maximumValue(self.source.fields().lookupField('date')), '2021-05-04')
        elif not self.treat_date_as_datetime():
            self.assertEqual(self.source.maximumValue(self.source.fields().lookupField('date')), QDate(2021, 5, 4))
        else:
            self.assertEqual(self.source.maximumValue(self.source.fields().lookupField('date')),
                             QDateTime(2021, 5, 4, 0, 0, 0))

        if not self.treat_time_as_string():
            self.assertEqual(self.source.maximumValue(self.source.fields().lookupField('time')), QTime(13, 13, 14))
        else:
            self.assertEqual(self.source.maximumValue(self.source.fields().lookupField('time')), '13:13:14')

        if self.source.supportsSubsetString():
            subset = self.getSubsetString2()
            self.source.setSubsetString(subset)
            max_value = self.source.maximumValue(self.source.fields().lookupField('cnt'))
            self.source.setSubsetString(None)
            self.assertEqual(max_value, 300)

    def testExtent(self):
        reference = QgsGeometry.fromRect(
            QgsRectangle(-71.123, 66.33, -65.32, 78.3))
        provider_extent = self.source.extent()
        self.assertAlmostEqual(provider_extent.xMinimum(), -71.123, 3)
        self.assertAlmostEqual(provider_extent.xMaximum(), -65.32, 3)
        self.assertAlmostEqual(provider_extent.yMinimum(), 66.33, 3)
        self.assertAlmostEqual(provider_extent.yMaximum(), 78.3, 3)

    def testExtentSubsetString(self):
        if self.source.supportsSubsetString():
            # with only one point
            subset = self.getSubsetString3()
            self.source.setSubsetString(subset)
            count = self.source.featureCount()
            provider_extent = self.source.extent()
            self.source.setSubsetString(None)
            self.assertEqual(count, 1)
            self.assertAlmostEqual(provider_extent.xMinimum(), -68.2, 3)
            self.assertAlmostEqual(provider_extent.xMaximum(), -68.2, 3)
            self.assertAlmostEqual(provider_extent.yMinimum(), 70.8, 3)
            self.assertAlmostEqual(provider_extent.yMaximum(), 70.8, 3)

            # with no points
            subset = self.getSubsetStringNoMatching()
            self.source.setSubsetString(subset)
            count = self.source.featureCount()
            provider_extent = self.source.extent()
            self.source.setSubsetString(None)
            self.assertEqual(count, 0)
            self.assertTrue(provider_extent.isNull())
            self.assertEqual(self.source.featureCount(), 5)

    def testUnique(self):
        self.assertEqual(self.source.uniqueValues(-1), set())
        self.assertEqual(self.source.uniqueValues(1000), set())

        self.assertEqual(set(self.source.uniqueValues(self.source.fields().lookupField('cnt'))),
                         set([-200, 100, 200, 300, 400]))
        assert set(['Apple', 'Honey', 'Orange', 'Pear', NULL]) == set(
            self.source.uniqueValues(self.source.fields().lookupField('name'))), 'Got {}'.format(
            set(self.source.uniqueValues(self.source.fields().lookupField('name'))))

        if self.treat_datetime_as_string():
            self.assertEqual(set(self.source.uniqueValues(self.source.fields().lookupField('dt'))),
                             set(['2021-05-04 13:13:14', '2020-05-04 12:14:14', '2020-05-04 12:13:14',
                                  '2020-05-03 12:13:14', NULL]))
        else:
            self.assertEqual(set(self.source.uniqueValues(self.source.fields().lookupField('dt'))),
                             set([QDateTime(2021, 5, 4, 13, 13, 14), QDateTime(2020, 5, 4, 12, 14, 14),
                                  QDateTime(2020, 5, 4, 12, 13, 14), QDateTime(2020, 5, 3, 12, 13, 14), NULL]))

        if self.treat_date_as_string():
            self.assertEqual(set(self.source.uniqueValues(self.source.fields().lookupField('date'))),
                             set(['2020-05-03', '2020-05-04', '2021-05-04', '2020-05-02', NULL]))
        elif self.treat_date_as_datetime():
            self.assertEqual(set(self.source.uniqueValues(self.source.fields().lookupField('date'))),
                             set([QDateTime(2020, 5, 3, 0, 0, 0), QDateTime(2020, 5, 4, 0, 0, 0),
                                  QDateTime(2021, 5, 4, 0, 0, 0), QDateTime(2020, 5, 2, 0, 0, 0), NULL]))
        else:
            self.assertEqual(set(self.source.uniqueValues(self.source.fields().lookupField('date'))),
                             set([QDate(2020, 5, 3), QDate(2020, 5, 4), QDate(2021, 5, 4), QDate(2020, 5, 2), NULL]))
        if self.treat_time_as_string():
            self.assertEqual(set(self.source.uniqueValues(self.source.fields().lookupField('time'))),
                             set(['12:14:14', '13:13:14', '12:13:14', '12:13:01', NULL]))
        else:
            self.assertEqual(set(self.source.uniqueValues(self.source.fields().lookupField('time'))),
                             set([QTime(12, 14, 14), QTime(13, 13, 14), QTime(12, 13, 14), QTime(12, 13, 1), NULL]))

        if self.source.supportsSubsetString():
            subset = self.getSubsetString2()
            self.source.setSubsetString(subset)
            values = self.source.uniqueValues(self.source.fields().lookupField('cnt'))
            self.source.setSubsetString(None)
            self.assertEqual(set(values), set([200, 300]))

    def testUniqueStringsMatching(self):
        self.assertEqual(self.source.uniqueStringsMatching(-1, 'a'), [])
        self.assertEqual(self.source.uniqueStringsMatching(100001, 'a'), [])

        field_index = self.source.fields().lookupField('name')
        self.assertEqual(set(self.source.uniqueStringsMatching(field_index, 'a')), set(['Pear', 'Orange', 'Apple']))
        # test case insensitive
        self.assertEqual(set(self.source.uniqueStringsMatching(field_index, 'A')), set(['Pear', 'Orange', 'Apple']))
        # test string ending in substring
        self.assertEqual(set(self.source.uniqueStringsMatching(field_index, 'ney')), set(['Honey']))
        # test limit
        result = set(self.source.uniqueStringsMatching(field_index, 'a', 2))
        self.assertEqual(len(result), 2)
        self.assertTrue(result.issubset(set(['Pear', 'Orange', 'Apple'])))

        assert set([u'Apple', u'Honey', u'Orange', u'Pear', NULL]) == set(
            self.source.uniqueValues(field_index)), 'Got {}'.format(set(self.source.uniqueValues(field_index)))

        if self.source.supportsSubsetString():
            subset = self.getSubsetString2()
            self.source.setSubsetString(subset)
            values = self.source.uniqueStringsMatching(field_index, 'a')
            self.source.setSubsetString(None)
            self.assertEqual(set(values), set(['Pear', 'Apple']))

    def testFeatureCount(self):
        self.assertEqual(self.source.featureCount(), 5)

        if self.source.supportsSubsetString():
            # Add a subset string and test feature count
            subset = self.getSubsetString()
            self.source.setSubsetString(subset)
            count = self.source.featureCount()
            self.source.setSubsetString(None)
            self.assertEqual(count, 3)
            self.assertEqual(self.source.featureCount(), 5)

            # one matching records
            subset = self.getSubsetString3()
            self.source.setSubsetString(subset)
            count = self.source.featureCount()
            self.source.setSubsetString(None)
            self.assertEqual(count, 1)
            self.assertEqual(self.source.featureCount(), 5)

            # no matching records
            subset = self.getSubsetStringNoMatching()
            self.source.setSubsetString(subset)
            count = self.source.featureCount()
            self.source.setSubsetString(None)
            self.assertEqual(count, 0)
            self.assertEqual(self.source.featureCount(), 5)

    def testEmpty(self):
        self.assertFalse(self.source.empty())
        self.assertEqual(self.source.hasFeatures(), QgsFeatureSource.FeaturesAvailable)

        if self.source.supportsSubsetString():
            try:
                backup = self.source.subsetString()
                # Add a subset string and test feature count
                subset = self.getSubsetString()
                self.source.setSubsetString(subset)
                self.assertFalse(self.source.empty())
                self.assertEqual(self.source.hasFeatures(), QgsFeatureSource.FeaturesAvailable)
                subsetNoMatching = self.getSubsetStringNoMatching()
                self.source.setSubsetString(subsetNoMatching)
                self.assertTrue(self.source.empty())
                self.assertEqual(self.source.hasFeatures(), QgsFeatureSource.NoFeaturesAvailable)
            finally:
                self.source.setSubsetString(None)
            self.assertFalse(self.source.empty())

        # If the provider supports tests on editable layers
        if getattr(self, 'getEditableLayer', None):
            l = self.getEditableLayer()
            self.assertTrue(l.isValid())

            self.assertEqual(l.hasFeatures(), QgsFeatureSource.FeaturesAvailable)

            # Test that deleting some features in the edit buffer does not
            # return empty, we accept FeaturesAvailable as well as
            # MaybeAvailable
            l.startEditing()
            l.deleteFeature(next(l.getFeatures()).id())
            self.assertNotEqual(l.hasFeatures(), QgsFeatureSource.NoFeaturesAvailable)
            l.rollBack()

            # Call truncate(), we need an empty set now
            l.dataProvider().truncate()
            self.assertTrue(l.dataProvider().empty())
            self.assertEqual(l.dataProvider().hasFeatures(), QgsFeatureSource.NoFeaturesAvailable)

    def testGetFeaturesNoGeometry(self):
        """ Test that no geometry is present when fetching features without geometry"""

        for f in self.source.getFeatures(QgsFeatureRequest().setFlags(QgsFeatureRequest.NoGeometry)):
            self.assertFalse(f.hasGeometry(), 'Expected no geometry, got one')
            self.assertTrue(f.isValid())

    def testAddFeature(self):
        if not getattr(self, 'getEditableLayer', None):
            return

        l = self.getEditableLayer()
        self.assertTrue(l.isValid())

        f1 = QgsFeature()
        f1.setAttributes([6, -220, NULL, 'String', '15',
                          '2019-01-02 03:04:05' if self.treat_datetime_as_string() else QDateTime(2019, 1, 2, 3, 4, 5),
                          '2019-01-02' if self.treat_date_as_string() else QDateTime(2019, 1, 2, 0, 0,
                                                                                     0) if self.treat_date_as_datetime() else QDate(
                              2019, 1, 2),
                          '03:04:05' if self.treat_time_as_string() else QTime(3, 4, 5)])
        f1.setGeometry(QgsGeometry.fromWkt('Point (-72.345 71.987)'))

        f2 = QgsFeature()
        f2.setAttributes([7, 330, 'Coconut', 'CoCoNut', '13',
                          '2018-05-06 07:08:09' if self.treat_datetime_as_string() else QDateTime(2018, 5, 6, 7, 8, 9),
                          '2018-05-06' if self.treat_date_as_string() else QDateTime(2018, 5, 6, 0, 0,
                                                                                     0) if self.treat_date_as_datetime() else QDate(
                              2018, 5, 6),
                          '07:08:09' if self.treat_time_as_string() else QTime(7, 8, 9)])

        if l.dataProvider().capabilities() & QgsVectorDataProvider.AddFeatures:
            # expect success
            result, added = l.dataProvider().addFeatures([f1, f2])
            self.assertTrue(result, 'Provider reported AddFeatures capability, but returned False to addFeatures')
            f1.setId(added[0].id())
            f2.setId(added[1].id())

            # check result
            self.testGetFeatures(l.dataProvider(), [f1, f2])

            # add empty list, should return true for consistency
            self.assertTrue(l.dataProvider().addFeatures([]))

            # ensure that returned features have been given the correct id
            f = next(l.getFeatures(QgsFeatureRequest().setFilterFid(added[0].id())))
            self.assertTrue(f.isValid())
            self.assertEqual(f['cnt'], -220)

            f = next(l.getFeatures(QgsFeatureRequest().setFilterFid(added[1].id())))
            self.assertTrue(f.isValid())
            self.assertEqual(f['cnt'], 330)
        else:
            # expect fail
            self.assertFalse(l.dataProvider().addFeatures([f1, f2]),
                             'Provider reported no AddFeatures capability, but returned true to addFeatures')

    def testAddFeatureFastInsert(self):
        if not getattr(self, 'getEditableLayer', None):
            return

        l = self.getEditableLayer()
        self.assertTrue(l.isValid())

        f1 = QgsFeature()
        f1.setAttributes(
            [6, -220, NULL, 'String', '15',
             '2019-01-02 03:04:05' if self.treat_datetime_as_string() else QDateTime(2019, 1, 2, 3, 4, 5),
             '2019-01-02' if self.treat_date_as_string() else QDateTime(2019, 1, 2, 0, 0, 0) if self.treat_date_as_datetime() else QDate(2019, 1, 2),
             '03:04:05' if self.treat_time_as_string() else QTime(3, 4, 5)])
        f1.setGeometry(QgsGeometry.fromWkt('Point (-72.345 71.987)'))

        f2 = QgsFeature()
        f2.setAttributes([7, 330, 'Coconut', 'CoCoNut', '13',
                          '2019-01-02 03:04:05' if self.treat_datetime_as_string() else QDateTime(2019, 1, 2, 3, 4, 5),
                          '2019-01-02' if self.treat_date_as_string() else QDateTime(2019, 1, 2, 0, 0, 0) if self.treat_date_as_datetime() else QDate(2019, 1, 2),
                          '03:04:05' if self.treat_time_as_string() else QTime(3, 4, 5)])

        if l.dataProvider().capabilities() & QgsVectorDataProvider.AddFeatures:
            # expect success
            result, added = l.dataProvider().addFeatures([f1, f2], QgsFeatureSink.FastInsert)
            self.assertTrue(result, 'Provider reported AddFeatures capability, but returned False to addFeatures')
            self.assertEqual(l.dataProvider().featureCount(), 7)

    def testAddFeatureMissingAttributes(self):
        if not getattr(self, 'getEditableLayer', None):
            return

        l = self.getEditableLayer()
        self.assertTrue(l.isValid())

        if not l.dataProvider().capabilities() & QgsVectorDataProvider.AddFeatures:
            return

        # test that adding features with missing attributes pads out these
        # attributes with NULL values to the correct length
        f1 = QgsFeature()
        f1.setAttributes([6, -220, NULL, 'String'])
        f2 = QgsFeature()
        f2.setAttributes([7, 330])

        result, added = l.dataProvider().addFeatures([f1, f2])
        self.assertTrue(result,
                        'Provider returned False to addFeatures with missing attributes. Providers should accept these features but add NULL attributes to the end of the existing attributes to the required field length.')
        f1.setId(added[0].id())
        f2.setId(added[1].id())

        # check result - feature attributes MUST be padded out to required number of fields
        f1.setAttributes([6, -220, NULL, 'String', 'NULL', NULL, NULL, NULL])
        f2.setAttributes([7, 330, NULL, NULL, 'NULL', NULL, NULL, NULL])
        self.testGetFeatures(l.dataProvider(), [f1, f2])

    def testAddFeatureExtraAttributes(self):
        if not getattr(self, 'getEditableLayer', None):
            return

        l = self.getEditableLayer()
        self.assertTrue(l.isValid())

        if not l.dataProvider().capabilities() & QgsVectorDataProvider.AddFeatures:
            return

        # test that adding features with too many attributes drops these attributes
        # we be more tricky and also add a valid feature to stress test the provider
        f1 = QgsFeature()
        f1.setAttributes([6, -220, NULL, 'String', '15',
                          '2019-01-02 03:04:05' if self.treat_datetime_as_string() else QDateTime(2019, 1, 2, 3, 4, 5),
                          '2019-01-02' if self.treat_date_as_string() else QDateTime(2019, 1, 2, 0, 0, 0) if self.treat_date_as_datetime() else QDate(2019, 1, 2),
                          '03:04:05' if self.treat_time_as_string() else QTime(3, 4, 5)])
        f2 = QgsFeature()
        f2.setAttributes([7, -230, NULL, 'String', '15',
                          '2019-01-02 03:04:05' if self.treat_datetime_as_string() else QDateTime(2019, 1, 2, 3, 4, 5),
                          '2019-01-02' if self.treat_date_as_string() else QDateTime(2019, 1, 2, 0, 0, 0) if self.treat_date_as_datetime() else QDate(2019, 1, 2),
                          '03:04:05' if self.treat_time_as_string() else QTime(3, 4, 5), 15, 16, 17])

        result, added = l.dataProvider().addFeatures([f1, f2])
        self.assertTrue(result,
                        'Provider returned False to addFeatures with extra attributes. Providers should accept these features but truncate the extra attributes.')

        # make sure feature was added correctly
        added = [f for f in l.dataProvider().getFeatures() if f[self.pk_name] == 7][0]
        self.assertEqual(added.attributes(), [7, -230, NULL, 'String', '15',
                                              '2019-01-02 03:04:05' if self.treat_datetime_as_string() else QDateTime(
                                                  2019, 1, 2, 3, 4, 5),
                                              '2019-01-02' if self.treat_date_as_string() else QDateTime(2019, 1, 2, 0, 0, 0) if self.treat_date_as_datetime() else QDate(2019, 1, 2),
                                              '03:04:05' if self.treat_time_as_string() else QTime(3, 4, 5)])

    def testAddFeatureWrongGeomType(self):
        if not getattr(self, 'getEditableLayer', None):
            return

        l = self.getEditableLayer()
        self.assertTrue(l.isValid())

        if not l.dataProvider().capabilities() & QgsVectorDataProvider.AddFeatures:
            return

        # test that adding features with incorrect geometry type rejects the feature
        # we be more tricky and also add a valid feature to stress test the provider
        f1 = QgsFeature()
        f1.setGeometry(QgsGeometry.fromWkt('LineString (-72.345 71.987, -80 80)'))
        f1.setAttributes([7])
        f2 = QgsFeature()
        f2.setGeometry(QgsGeometry.fromWkt('Point (-72.345 71.987)'))
        f2.setAttributes([8])

        result, added = l.dataProvider().addFeatures([f1, f2])
        self.assertFalse(result,
                         'Provider returned True to addFeatures with incorrect geometry type. Providers should reject these features.')

        # make sure feature was not added
        added = [f for f in l.dataProvider().getFeatures() if f[self.pk_name] == 7]
        self.assertFalse(added)

        # yet providers MUST always accept null geometries
        f3 = QgsFeature()
        f3.setAttributes([9])
        result, added = l.dataProvider().addFeatures([f3])
        self.assertTrue(result,
                        'Provider returned False to addFeatures with null geometry. Providers should always accept these features.')

        # make sure feature was added correctly
        added = [f for f in l.dataProvider().getFeatures() if f[self.pk_name] == 9][0]
        self.assertFalse(added.hasGeometry())

    def testAddFeaturesUpdateExtent(self):
        if not getattr(self, 'getEditableLayer', None):
            return

        l = self.getEditableLayer()
        self.assertTrue(l.isValid())

        self.assertEqual(l.dataProvider().extent().toString(1), '-71.1,66.3 : -65.3,78.3')

        if l.dataProvider().capabilities() & QgsVectorDataProvider.AddFeatures:
            f1 = QgsFeature()
            f1.setAttributes([6, -220, NULL, 'String', '15'])
            f1.setGeometry(QgsGeometry.fromWkt('Point (-50 90)'))
            l.dataProvider().addFeatures([f1])

            l.dataProvider().updateExtents()
            self.assertEqual(l.dataProvider().extent().toString(1), '-71.1,66.3 : -50.0,90.0')

    def testDeleteFeatures(self):
        if not getattr(self, 'getEditableLayer', None):
            return

        l = self.getEditableLayer()
        self.assertTrue(l.isValid())

        # find 2 features to delete
        features = [f for f in l.dataProvider().getFeatures()]
        to_delete = [f.id() for f in features if f.attributes()[0] in [1, 3]]

        if l.dataProvider().capabilities() & QgsVectorDataProvider.DeleteFeatures:
            # expect success
            result = l.dataProvider().deleteFeatures(to_delete)
            self.assertTrue(result, 'Provider reported DeleteFeatures capability, but returned False to deleteFeatures')

            # check result
            self.testGetFeatures(l.dataProvider(), skip_features=[1, 3])

            # delete empty list, should return true for consistency
            self.assertTrue(l.dataProvider().deleteFeatures([]))

        else:
            # expect fail
            self.assertFalse(l.dataProvider().deleteFeatures(to_delete),
                             'Provider reported no DeleteFeatures capability, but returned true to deleteFeatures')

    def testDeleteFeaturesUpdateExtent(self):
        if not getattr(self, 'getEditableLayer', None):
            return

        l = self.getEditableLayer()
        self.assertTrue(l.isValid())

        self.assertEqual(l.dataProvider().extent().toString(1), '-71.1,66.3 : -65.3,78.3')

        to_delete = [f.id() for f in l.dataProvider().getFeatures() if f.attributes()[0] in [5, 4]]

        if l.dataProvider().capabilities() & QgsVectorDataProvider.DeleteFeatures:
            l.dataProvider().deleteFeatures(to_delete)

            l.dataProvider().updateExtents()
            self.assertEqual(l.dataProvider().extent().toString(1), '-70.3,66.3 : -68.2,70.8')

    def testTruncate(self):
        if not getattr(self, 'getEditableLayer', None):
            return

        l = self.getEditableLayer()
        self.assertTrue(l.isValid())

        features = [f[self.pk_name] for f in l.dataProvider().getFeatures()]

        if l.dataProvider().capabilities() & QgsVectorDataProvider.FastTruncate or l.dataProvider().capabilities() & QgsVectorDataProvider.DeleteFeatures:
            # expect success
            result = l.dataProvider().truncate()
            self.assertTrue(result,
                            'Provider reported FastTruncate or DeleteFeatures capability, but returned False to truncate()')

            # check result
            features = [f[self.pk_name] for f in l.dataProvider().getFeatures()]
            self.assertEqual(len(features), 0)
        else:
            # expect fail
            self.assertFalse(l.dataProvider().truncate(),
                             'Provider reported no FastTruncate or DeleteFeatures capability, but returned true to truncate()')

    def testChangeAttributes(self):
        if not getattr(self, 'getEditableLayer', None):
            return

        l = self.getEditableLayer()
        self.assertTrue(l.isValid())

        # find 2 features to change
        features = [f for f in l.dataProvider().getFeatures()]
        # need to keep order here
        to_change = [f for f in features if f.attributes()[0] == 1]
        to_change.extend([f for f in features if f.attributes()[0] == 3])
        # changes by feature id, for changeAttributeValues call
        changes = {to_change[0].id(): {1: 501, 3: 'new string'}, to_change[1].id(): {1: 502, 4: 'NEW'}}
        # changes by pk, for testing after retrieving changed features
        new_attr_map = {1: {1: 501, 3: 'new string'}, 3: {1: 502, 4: 'NEW'}}

        if l.dataProvider().capabilities() & QgsVectorDataProvider.ChangeAttributeValues:
            # expect success
            result = l.dataProvider().changeAttributeValues(changes)
            self.assertTrue(result,
                            'Provider reported ChangeAttributeValues capability, but returned False to changeAttributeValues')

            # check result
            self.testGetFeatures(l.dataProvider(), changed_attributes=new_attr_map)

            # change empty list, should return true for consistency
            self.assertTrue(l.dataProvider().changeAttributeValues({}))

        else:
            # expect fail
            self.assertFalse(l.dataProvider().changeAttributeValues(changes),
                             'Provider reported no ChangeAttributeValues capability, but returned true to changeAttributeValues')

    def testChangeAttributesConstraintViolation(self):
        """Checks that changing attributes violating a DB-level CHECK constraint returns false
        the provider test case must provide an editable layer with a text field
        "i_will_fail_on_no_name" having a CHECK constraint that will fail when value is "no name".
        The layer must contain at least 2 features, that will be used to test the attribute change.
        """

        if not getattr(self, 'getEditableLayerWithCheckConstraint', None):
            return

        l = self.getEditableLayerWithCheckConstraint()
        self.assertTrue(l.isValid())

        assert l.dataProvider().capabilities() & QgsVectorDataProvider.ChangeAttributeValues

        # find the featurea to change
        feature0 = [f for f in l.dataProvider().getFeatures()][0]
        feature1 = [f for f in l.dataProvider().getFeatures()][1]
        field_idx = l.fields().indexFromName('i_will_fail_on_no_name')
        self.assertTrue(field_idx >= 0)
        # changes by feature id, for changeAttributeValues call
        changes = {
            feature0.id(): {field_idx: 'no name'},
            feature1.id(): {field_idx: 'I have a valid name'}
        }
        # expect failure
        result = l.dataProvider().changeAttributeValues(changes)
        self.assertFalse(
            result, 'Provider reported success when changing an attribute value that violates a DB level CHECK constraint')

        if getattr(self, 'stopEditableLayerWithCheckConstraint', None):
            self.stopEditableLayerWithCheckConstraint()

    def testUniqueNotNullConstraints(self):
        """Test provider-level NOT NULL and UNIQUE constraints, to enable
        this test, implement getEditableLayerWithUniqueNotNullConstraints
        to return an editable POINT layer with the following fields:

            "unique" TEXT UNIQUE,
            "not_null" TEXT NOT NULL

        """

        if not getattr(self, 'getEditableLayerWithUniqueNotNullConstraints', None):
            return

        vl = self.getEditableLayerWithUniqueNotNullConstraints()

        self.assertTrue(vl.isValid())
        unique_field_idx = vl.fields().indexFromName('unique')
        not_null_field_idx = vl.fields().indexFromName('not_null')
        self.assertTrue(unique_field_idx > 0)
        self.assertTrue(not_null_field_idx > 0)
        # Not null
        self.assertFalse(bool(vl.fieldConstraints(unique_field_idx) & QgsFieldConstraints.ConstraintNotNull))
        self.assertTrue(bool(vl.fieldConstraints(not_null_field_idx) & QgsFieldConstraints.ConstraintNotNull))
        # Unique
        self.assertTrue(bool(vl.fieldConstraints(unique_field_idx) & QgsFieldConstraints.ConstraintUnique))
        self.assertFalse(bool(vl.fieldConstraints(not_null_field_idx) & QgsFieldConstraints.ConstraintUnique))

    def testChangeGeometries(self):
        if not getattr(self, 'getEditableLayer', None):
            return

        l = self.getEditableLayer()
        self.assertTrue(l.isValid())

        # find 2 features to change
        features = [f for f in l.dataProvider().getFeatures()]
        to_change = [f for f in features if f.attributes()[0] == 1]
        to_change.extend([f for f in features if f.attributes()[0] == 3])
        # changes by feature id, for changeGeometryValues call
        changes = {to_change[0].id(): QgsGeometry.fromWkt('Point (10 20)'), to_change[1].id(): QgsGeometry()}
        # changes by pk, for testing after retrieving changed features
        new_geom_map = {1: QgsGeometry.fromWkt('Point ( 10 20 )'), 3: QgsGeometry()}

        if l.dataProvider().capabilities() & QgsVectorDataProvider.ChangeGeometries:
            # expect success
            result = l.dataProvider().changeGeometryValues(changes)
            self.assertTrue(result,
                            'Provider reported ChangeGeometries capability, but returned False to changeGeometryValues')

            # check result
            self.testGetFeatures(l.dataProvider(), changed_geometries=new_geom_map)

            # change empty list, should return true for consistency
            self.assertTrue(l.dataProvider().changeGeometryValues({}))

        else:
            # expect fail
            self.assertFalse(l.dataProvider().changeGeometryValues(changes),
                             'Provider reported no ChangeGeometries capability, but returned true to changeGeometryValues')

    def testChangeFeatures(self):
        if not getattr(self, 'getEditableLayer', None):
            return

        l = self.getEditableLayer()
        self.assertTrue(l.isValid())

        features = [f for f in l.dataProvider().getFeatures()]

        # find 2 features to change attributes for
        features = [f for f in l.dataProvider().getFeatures()]
        # need to keep order here
        to_change = [f for f in features if f.attributes()[0] == 1]
        to_change.extend([f for f in features if f.attributes()[0] == 2])
        # changes by feature id, for changeAttributeValues call
        attribute_changes = {to_change[0].id(): {1: 501, 3: 'new string'}, to_change[1].id(): {1: 502, 4: 'NEW'}}
        # changes by pk, for testing after retrieving changed features
        new_attr_map = {1: {1: 501, 3: 'new string'}, 2: {1: 502, 4: 'NEW'}}

        # find 2 features to change geometries for
        to_change = [f for f in features if f.attributes()[0] == 1]
        to_change.extend([f for f in features if f.attributes()[0] == 3])
        # changes by feature id, for changeGeometryValues call
        geometry_changes = {to_change[0].id(): QgsGeometry.fromWkt('Point (10 20)'), to_change[1].id(): QgsGeometry()}
        # changes by pk, for testing after retrieving changed features
        new_geom_map = {1: QgsGeometry.fromWkt('Point ( 10 20 )'), 3: QgsGeometry()}

        if l.dataProvider().capabilities() & QgsVectorDataProvider.ChangeGeometries and l.dataProvider().capabilities() & QgsVectorDataProvider.ChangeAttributeValues:
            # expect success
            result = l.dataProvider().changeFeatures(attribute_changes, geometry_changes)
            self.assertTrue(result,
                            'Provider reported ChangeGeometries and ChangeAttributeValues capability, but returned False to changeFeatures')

            # check result
            self.testGetFeatures(l.dataProvider(), changed_attributes=new_attr_map, changed_geometries=new_geom_map)

            # change empty list, should return true for consistency
            self.assertTrue(l.dataProvider().changeFeatures({}, {}))

        elif not l.dataProvider().capabilities() & QgsVectorDataProvider.ChangeGeometries:
            # expect fail
            self.assertFalse(l.dataProvider().changeFeatures(attribute_changes, geometry_changes),
                             'Provider reported no ChangeGeometries capability, but returned true to changeFeatures')
        elif not l.dataProvider().capabilities() & QgsVectorDataProvider.ChangeAttributeValues:
            # expect fail
            self.assertFalse(l.dataProvider().changeFeatures(attribute_changes, geometry_changes),
                             'Provider reported no ChangeAttributeValues capability, but returned true to changeFeatures')

    def testMinMaxAfterChanges(self):
        """
        Tests retrieving field min and max value after making changes to the provider's features
        """
        if not getattr(self, 'getEditableLayer', None):
            return

        vl = self.getEditableLayer()
        self.assertTrue(vl.isValid())

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
        f7.setAttributes([0, -1400])
        res, [f7] = vl.dataProvider().addFeatures([f7])
        self.assertTrue(res)
        self.assertEqual(vl.dataProvider().minimumValue(0), 0)
        self.assertEqual(vl.dataProvider().minimumValue(1), -1400)
        self.assertEqual(vl.dataProvider().maximumValue(0), 15)
        self.assertEqual(vl.dataProvider().maximumValue(1), 1400)

        # change attribute values
        self.assertTrue(vl.dataProvider().changeAttributeValues({f6.id(): {1: 150}, f7.id(): {1: -100}}))
        self.assertEqual(vl.dataProvider().minimumValue(1), -200)
        self.assertEqual(vl.dataProvider().maximumValue(1), 400)

        # delete features
        f1 = [f for f in vl.getFeatures() if f[self.pk_name] == 5][0]
        f3 = [f for f in vl.getFeatures() if f[self.pk_name] == 3][0]
        self.assertTrue(vl.dataProvider().deleteFeatures([f6.id(), f7.id()]))
        self.assertEqual(vl.dataProvider().minimumValue(0), 1)
        self.assertEqual(vl.dataProvider().minimumValue(1), -200)
        self.assertEqual(vl.dataProvider().maximumValue(0), 5)
        self.assertEqual(vl.dataProvider().maximumValue(1), 400)

        if vl.dataProvider().capabilities() & QgsVectorDataProvider.DeleteAttributes:
            # delete attributes
            if vl.dataProvider().deleteAttributes([0]):
                # may not be possible, e.g. if it's a primary key
                self.assertEqual(vl.dataProvider().minimumValue(0), -200)
                self.assertEqual(vl.dataProvider().maximumValue(0), 400)

    def testStringComparison(self):
        """
        Test if string comparisons with numbers are cast by the expression
        compiler (or work fine without doing anything :P)
        """
        for expression in (
                '5 LIKE \'5\'',
                '5 ILIKE \'5\'',
                '15 NOT LIKE \'5\'',
                '15 NOT ILIKE \'5\'',
                '5 ~ \'5\''):
            iterator = self.source.getFeatures(QgsFeatureRequest().setFilterExpression('5 LIKE \'5\'').setFlags(QgsFeatureRequest.IgnoreStaticNodesDuringExpressionCompilation))
            count = len([f for f in iterator])
            self.assertEqual(count, 5)
            self.assertFalse(iterator.compileFailed())
            if self.enableCompiler():
                iterator = self.source.getFeatures(QgsFeatureRequest().setFilterExpression('5 LIKE \'5\'').setFlags(QgsFeatureRequest.IgnoreStaticNodesDuringExpressionCompilation))
                self.assertEqual(count, 5)
                self.assertFalse(iterator.compileFailed())
                self.disableCompiler()

    def testConcurrency(self):
        """
        The connection pool has a maximum of 4 connections defined (+2 spare connections)
        Make sure that if we exhaust those 4 connections and force another connection
        it is actually using the spare connections and does not freeze.
        This situation normally happens when (at least) 4 rendering threads are active
        in parallel and one requires an expression to be evaluated.
        """
        # Acquire the maximum amount of concurrent connections
        iterators = list()
        for i in range(QgsApplication.instance().maxConcurrentConnectionsPerPool()):
            iterators.append(self.vl.getFeatures())

        # Run an expression that will also do a request and should use a spare
        # connection. It just should not deadlock here.

        feat = next(iterators[0])
        context = QgsExpressionContext()
        context.setFeature(feat)
        exp = QgsExpression('get_feature(\'{layer}\', \'pk\', 5)'.format(layer=self.vl.id()))
        exp.evaluate(context)

    def testEmptySubsetOfAttributesWithSubsetString(self):

        if self.source.supportsSubsetString():
            try:
                # Add a subset string
                subset = self.getSubsetString()
                self.source.setSubsetString(subset)

                # First test, in a regular way
                features = [f for f in self.source.getFeatures()]
                count = len(features)
                self.assertEqual(count, 3)
                has_geometry = features[0].hasGeometry()

                # Ask for no attributes
                request = QgsFeatureRequest().setSubsetOfAttributes([])
                # Make sure we still retrieve features !
                features = [f for f in self.source.getFeatures(request)]
                count = len(features)
                self.assertEqual(count, 3)
                # Check that we still get a geometry if we add one before
                self.assertEqual(features[0].hasGeometry(), has_geometry)

            finally:
                self.source.setSubsetString(None)

    def testGeneratedColumns(self):

        if not getattr(self, 'getGeneratedColumnsData', None):
            return

        vl, generated_value = self.getGeneratedColumnsData()
        if vl is None:
            return

        self.assertTrue(vl.isValid())
        self.assertEqual(vl.fields().count(), 2)

        field = vl.fields().at(1)
        self.assertEqual(field.name(), "generated_field")
        self.assertEqual(field.type(), QVariant.String)
        self.assertEqual(vl.dataProvider().defaultValueClause(1), generated_value)

        vl.startEditing()

        feature = next(vl.getFeatures())
        self.assertFalse(QgsVectorLayerUtils.fieldIsEditable(vl, 1, feature))
        self.assertTrue(QgsVectorLayerUtils.fieldIsEditable(vl, 0, feature))

        # same test on a new inserted feature
        feature = QgsFeature(vl.fields())
        feature.setAttribute(0, 2)
        vl.addFeature(feature)
        self.assertTrue(feature.id() < 0)
        self.assertFalse(QgsVectorLayerUtils.fieldIsEditable(vl, 1, feature))
        self.assertTrue(QgsVectorLayerUtils.fieldIsEditable(vl, 0, feature))
        vl.commitChanges()

        feature = vl.getFeature(2)
        self.assertTrue(feature.isValid())
        self.assertEqual(feature.attribute(1), "test:2")
        self.assertFalse(QgsVectorLayerUtils.fieldIsEditable(vl, 1, feature))
        self.assertFalse(QgsVectorLayerUtils.fieldIsEditable(vl, 0, feature))

        # test update id and commit
        vl.startEditing()
        self.assertFalse(QgsVectorLayerUtils.fieldIsEditable(vl, 1, feature))
        self.assertTrue(QgsVectorLayerUtils.fieldIsEditable(vl, 0, feature))
        self.assertTrue(vl.changeAttributeValue(2, 0, 10))
        self.assertTrue(vl.commitChanges())
        feature = vl.getFeature(10)
        self.assertTrue(feature.isValid())
        self.assertEqual(feature.attribute(0), 10)
        self.assertEqual(feature.attribute(1), "test:10")
        self.assertFalse(QgsVectorLayerUtils.fieldIsEditable(vl, 1, feature))
        self.assertFalse(QgsVectorLayerUtils.fieldIsEditable(vl, 0, feature))

        # test update the_field and commit (the value is not changed because the field is generated)
        vl.startEditing()
        self.assertFalse(QgsVectorLayerUtils.fieldIsEditable(vl, 1, feature))
        self.assertTrue(QgsVectorLayerUtils.fieldIsEditable(vl, 0, feature))
        self.assertTrue(vl.changeAttributeValue(10, 1, "new value"))
        self.assertTrue(vl.commitChanges())
        feature = vl.getFeature(10)
        self.assertTrue(feature.isValid())
        self.assertEqual(feature.attribute(1), "test:10")
        self.assertFalse(QgsVectorLayerUtils.fieldIsEditable(vl, 1, feature))
        self.assertFalse(QgsVectorLayerUtils.fieldIsEditable(vl, 0, feature))

        # Test insertion with default value evaluation on provider side to be sure
        # it doesn't fail generated columns
        vl.dataProvider().setProviderProperty(QgsDataProvider.EvaluateDefaultValues, True)

        vl.startEditing()
        feature = QgsVectorLayerUtils.createFeature(vl, QgsGeometry(), {0: 8})
        vl.addFeature(feature)
        self.assertTrue(feature.id() < 0)
        self.assertFalse(QgsVectorLayerUtils.fieldIsEditable(vl, 1, feature))
        self.assertTrue(QgsVectorLayerUtils.fieldIsEditable(vl, 0, feature))
        self.assertTrue(vl.commitChanges())

        feature = vl.getFeature(8)
        self.assertTrue(feature.isValid())
        self.assertEqual(feature.attribute(1), "test:8")
        self.assertFalse(QgsVectorLayerUtils.fieldIsEditable(vl, 1, feature))
        self.assertFalse(QgsVectorLayerUtils.fieldIsEditable(vl, 0, feature))

        # CLEANUP: delete features added during test (cleanup)
        vl.startEditing()
        self.assertTrue(vl.deleteFeature(10))
        self.assertTrue(vl.commitChanges())
        # TODO: further cleanups in case attributes have been changed
