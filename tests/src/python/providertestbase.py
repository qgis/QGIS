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

from qgis.core import QgsRectangle, QgsFeatureRequest, QgsFeature, QgsGeometry, NULL


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
        self.assert_query(provider, 'name = \'Apple\'', [2])
        self.assert_query(provider, 'name <> \'Apple\'', [1, 3, 4])
        self.assert_query(provider, 'name = \'apple\'', [])
        self.assert_query(provider, '"name" <> \'apple\'', [1, 2, 3, 4])
        self.assert_query(provider, '(name = \'Apple\') is not null', [1, 2, 3, 4])
        self.assert_query(provider, 'name LIKE \'Apple\'', [2])
        self.assert_query(provider, 'name LIKE \'aPple\'', [])
        self.assert_query(provider, 'name ILIKE \'aPple\'', [2])
        self.assert_query(provider, 'name ILIKE \'%pp%\'', [2])
        self.assert_query(provider, 'cnt > 0', [1, 2, 3, 4])
        self.assert_query(provider, 'cnt < 0', [5])
        self.assert_query(provider, 'cnt >= 100', [1, 2, 3, 4])
        self.assert_query(provider, 'cnt <= 100', [1, 5])
        self.assert_query(provider, 'pk IN (1, 2, 4, 8)', [1, 2, 4])
        self.assert_query(provider, 'cnt = 50 * 2', [1])
        self.assert_query(provider, 'cnt = 99 + 1', [1])
        self.assert_query(provider, 'cnt = 101 - 1', [1])
        self.assert_query(provider, 'cnt - 1 = 99', [1])
        self.assert_query(provider, 'cnt + 1 = 101', [1])
        self.assert_query(provider, 'cnt = 1100 % 1000', [1])
        self.assert_query(provider, '"name" || \' \' || "name" = \'Orange Orange\'', [1])
        self.assert_query(provider, '"name" || \' \' || "cnt" = \'Orange 100\'', [1])
        self.assert_query(provider, '\'x\' || "name" IS NOT NULL', [1, 2, 3, 4])
        self.assert_query(provider, '\'x\' || "name" IS NULL', [5])
        self.assert_query(provider, 'cnt = 10 ^ 2', [1])
        self.assert_query(provider, '"name" ~ \'[OP]ra[gne]+\'', [1])
        self.assert_query(provider, '"name"="name2"', [2, 4])  # mix of matched and non-matched case sensitive names
        self.assert_query(provider, 'true', [1, 2, 3, 4, 5])
        self.assert_query(provider, 'false', [])

        # Three value logic
        self.assert_query(provider, 'false and false', [])
        self.assert_query(provider, 'false and true', [])
        self.assert_query(provider, 'false and NULL', [])
        self.assert_query(provider, 'true and false', [])
        self.assert_query(provider, 'true and true', [1, 2, 3, 4, 5])
        self.assert_query(provider, 'true and NULL', [])
        self.assert_query(provider, 'NULL and false', [])
        self.assert_query(provider, 'NULL and true', [])
        self.assert_query(provider, 'NULL and NULL', [])
        self.assert_query(provider, 'false or false', [])
        self.assert_query(provider, 'false or true', [1, 2, 3, 4, 5])
        self.assert_query(provider, 'false or NULL', [])
        self.assert_query(provider, 'true or false', [1, 2, 3, 4, 5])
        self.assert_query(provider, 'true or true', [1, 2, 3, 4, 5])
        self.assert_query(provider, 'true or NULL', [1, 2, 3, 4, 5])
        self.assert_query(provider, 'NULL or false', [])
        self.assert_query(provider, 'NULL or true', [1, 2, 3, 4, 5])
        self.assert_query(provider, 'NULL or NULL', [])
        self.assert_query(provider, 'not true', [])
        self.assert_query(provider, 'not false', [1, 2, 3, 4, 5])
        self.assert_query(provider, 'not null', [])

        # not
        self.assert_query(provider, 'not name = \'Apple\'', [1, 3, 4])
        self.assert_query(provider, 'not name IS NULL', [1, 2, 3, 4])
        self.assert_query(provider, 'not name = \'Apple\' or name = \'Apple\'', [1, 2, 3, 4])
        self.assert_query(provider, 'not name = \'Apple\' or not name = \'Apple\'', [1, 3, 4])
        self.assert_query(provider, 'not name = \'Apple\' and pk = 4', [4])
        self.assert_query(provider, 'not name = \'Apple\' and not pk = 4', [1, 3])
        self.assert_query(provider, 'not pk IN (1, 2, 4, 8)', [3, 5])

        # type conversion - QGIS expressions do not mind that we are comparing a string
        # against numeric literals
        self.assert_query(provider, 'num_char IN (2, 4, 5)', [2, 4, 5])

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

    def testOrderBy(self):
        request = QgsFeatureRequest().addOrderBy('cnt')
        values = [f['cnt'] for f in self.provider.getFeatures(request)]
        self.assertEquals(values, [-200, 100, 200, 300, 400])

        request = QgsFeatureRequest().addOrderBy('cnt', False)
        values = [f['cnt'] for f in self.provider.getFeatures(request)]
        self.assertEquals(values, [400, 300, 200, 100, -200])

        request = QgsFeatureRequest().addOrderBy('name')
        values = [f['name'] for f in self.provider.getFeatures(request)]
        self.assertEquals(values, ['Apple', 'Honey', 'Orange', 'Pear', NULL])

        request = QgsFeatureRequest().addOrderBy('name', True, True)
        values = [f['name'] for f in self.provider.getFeatures(request)]
        self.assertEquals(values, [NULL, 'Apple', 'Honey', 'Orange', 'Pear'])

        request = QgsFeatureRequest().addOrderBy('name', False)
        values = [f['name'] for f in self.provider.getFeatures(request)]
        self.assertEquals(values, [NULL, 'Pear', 'Orange', 'Honey', 'Apple'])

        request = QgsFeatureRequest().addOrderBy('name', False, False)
        values = [f['name'] for f in self.provider.getFeatures(request)]
        self.assertEquals(values, ['Pear', 'Orange', 'Honey', 'Apple', NULL])

        # Case sensitivity
        request = QgsFeatureRequest().addOrderBy('name2')
        values = [f['name2'] for f in self.provider.getFeatures(request)]
        self.assertEquals(values, ['Apple', 'Honey', 'NuLl', 'oranGe', 'PEaR'])

        # Combination with LIMIT
        request = QgsFeatureRequest().addOrderBy('pk', False).setLimit(2)
        values = [f['pk'] for f in self.provider.getFeatures(request)]
        self.assertEquals(values, [5, 4])

        # A slightly more complex expression
        request = QgsFeatureRequest().addOrderBy('pk*2', False)
        values = [f['pk'] for f in self.provider.getFeatures(request)]
        self.assertEquals(values, [5, 4, 3, 2, 1])

        # Type dependant expression
        request = QgsFeatureRequest().addOrderBy('num_char*2', False)
        values = [f['pk'] for f in self.provider.getFeatures(request)]
        self.assertEquals(values, [5, 4, 3, 2, 1])

        # Order by guaranteed to fail
        request = QgsFeatureRequest().addOrderBy('not a valid expression*', False)
        values = [f['pk'] for f in self.provider.getFeatures(request)]
        self.assertEquals(set(values), set([5, 4, 3, 2, 1]))

        # Multiple order bys and boolean
        request = QgsFeatureRequest().addOrderBy('pk > 2').addOrderBy('pk', False)
        values = [f['pk'] for f in self.provider.getFeatures(request)]
        self.assertEquals(values, [2, 1, 5, 4, 3])

        # Multiple order bys, one bad, and a limit
        request = QgsFeatureRequest().addOrderBy('pk', False).addOrderBy('not a valid expression*', False).setLimit(2)
        values = [f['pk'] for f in self.provider.getFeatures(request)]
        self.assertEquals(values, [5, 4])

        # Bad expression first
        request = QgsFeatureRequest().addOrderBy('not a valid expression*', False).addOrderBy('pk', False).setLimit(2)
        values = [f['pk'] for f in self.provider.getFeatures(request)]
        self.assertEquals(values, [5, 4])

        # Combination with subset of attributes
        request = QgsFeatureRequest().addOrderBy('num_char', False).setSubsetOfAttributes(['pk'], self.vl.fields())
        values = [f['pk'] for f in self.vl.getFeatures(request)]
        self.assertEquals(values, [5, 4, 3, 2, 1])

    def testGetFeaturesFidTests(self):
        fids = [f.id() for f in self.provider.getFeatures()]
        assert len(fids) == 5, 'Expected 5 features, got {} instead'.format(len(fids))
        for id in fids:
            result = [f.id() for f in self.provider.getFeatures(QgsFeatureRequest().setFilterFid(id))]
            expected = [id]
            assert result == expected, 'Expected {} and got {} when testing for feature ID filter'.format(expected, result)

    def testGetFeaturesFidsTests(self):
        fids = [f.id() for f in self.provider.getFeatures()]

        result = set([f.id() for f in self.provider.getFeatures(QgsFeatureRequest().setFilterFids([fids[0], fids[2]]))])
        expected = set([fids[0], fids[2]])
        assert result == expected, 'Expected {} and got {} when testing for feature IDs filter'.format(expected, result)

        result = set([f.id() for f in self.provider.getFeatures(QgsFeatureRequest().setFilterFids([fids[1], fids[3], fids[4]]))])
        expected = set([fids[1], fids[3], fids[4]])
        assert result == expected, 'Expected {} and got {} when testing for feature IDs filter'.format(expected, result)

        result = set([f.id() for f in self.provider.getFeatures(QgsFeatureRequest().setFilterFids([]))])
        expected = set([])
        assert result == expected, 'Expected {} and got {} when testing for feature IDs filter'.format(expected, result)

    def testGetFeaturesFilterRectTests(self):
        extent = QgsRectangle(-70, 67, -60, 80)
        features = [f['pk'] for f in self.provider.getFeatures(QgsFeatureRequest().setFilterRect(extent))]
        assert set(features) == set([2, 4]), 'Got {} instead'.format(features)

    def testRectAndExpression(self):
        extent = QgsRectangle(-70, 67, -60, 80)
        result = set([f['pk'] for f in self.provider.getFeatures(
            QgsFeatureRequest().setFilterExpression('"cnt">200').setFilterRect(extent))])
        expected = [4]
        assert set(expected) == result, 'Expected {} and got {} when testing for combination of filterRect and expression'.format(set(expected), result)

    def testGetFeaturesLimit(self):
        it = self.provider.getFeatures(QgsFeatureRequest().setLimit(2))
        features = [f['pk'] for f in it]
        assert len(features) == 2, 'Expected two features, got {} instead'.format(len(features))
        # fetch one feature
        feature = QgsFeature()
        assert not it.nextFeature(feature), 'Expected no feature after limit, got one'
        it.rewind()
        features = [f['pk'] for f in it]
        assert len(features) == 2, 'Expected two features after rewind, got {} instead'.format(len(features))
        it.rewind()
        assert it.nextFeature(feature), 'Expected feature after rewind, got none'
        it.rewind()
        features = [f['pk'] for f in it]
        assert len(features) == 2, 'Expected two features after rewind, got {} instead'.format(len(features))
        # test with expression, both with and without compilation
        try:
            self.disableCompiler()
        except AttributeError:
            pass
        it = self.provider.getFeatures(QgsFeatureRequest().setLimit(2).setFilterExpression('cnt <= 100'))
        features = [f['pk'] for f in it]
        assert set(features) == set([1, 5]), 'Expected [1,5] for expression and feature limit, Got {} instead'.format(features)
        try:
            self.enableCompiler()
        except AttributeError:
            pass
        it = self.provider.getFeatures(QgsFeatureRequest().setLimit(2).setFilterExpression('cnt <= 100'))
        features = [f['pk'] for f in it]
        assert set(features) == set([1, 5]), 'Expected [1,5] for expression and feature limit, Got {} instead'.format(features)
        # limit to more features than exist
        it = self.provider.getFeatures(QgsFeatureRequest().setLimit(3).setFilterExpression('cnt <= 100'))
        features = [f['pk'] for f in it]
        assert set(features) == set([1, 5]), 'Expected [1,5] for expression and feature limit, Got {} instead'.format(features)
        # limit to less features than possible
        it = self.provider.getFeatures(QgsFeatureRequest().setLimit(1).setFilterExpression('cnt <= 100'))
        features = [f['pk'] for f in it]
        assert 1 in features or 5 in features, 'Expected either 1 or 5 for expression and feature limit, Got {} instead'.format(features)

    def testMinValue(self):
        self.assertEqual(self.provider.minimumValue(1), -200)
        self.assertEqual(self.provider.minimumValue(2), 'Apple')

    def testMaxValue(self):
        self.assertEqual(self.provider.maximumValue(1), 400)
        self.assertEqual(self.provider.maximumValue(2), 'Pear')

    def testExtent(self):
        reference = QgsGeometry.fromRect(
            QgsRectangle(-71.123, 66.33, -65.32, 78.3))
        provider_extent = QgsGeometry.fromRect(self.provider.extent())

        assert QgsGeometry.compare(provider_extent.asPolygon(), reference.asPolygon(), 0.00001), 'Expected {}, got {}'.format(reference.exportToWkt(), provider_extent.exportToWkt())

    def testUnique(self):
        self.assertEqual(set(self.provider.uniqueValues(1)), set([-200, 100, 200, 300, 400]))
        assert set([u'Apple', u'Honey', u'Orange', u'Pear', NULL]) == set(self.provider.uniqueValues(2)), 'Got {}'.format(set(self.provider.uniqueValues(2)))

    def testFeatureCount(self):
        assert self.provider.featureCount() == 5, 'Got {}'.format(self.provider.featureCount())
