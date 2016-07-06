# -*- coding: utf-8 -*-
"""QGIS Unit test utils for provider tests.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
from __future__ import print_function
from builtins import str
from builtins import object
__author__ = 'Matthias Kuhn'
__date__ = '2015-04-27'
__copyright__ = 'Copyright 2015, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from qgis.core import QgsRectangle, QgsFeatureRequest, QgsFeature, QgsGeometry, QgsAbstractFeatureIterator, NULL

from utilities import(
    compareWkt
)


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

    def testGetFeatures(self):
        """ Test that expected results are returned when fetching all features """

        # IMPORTANT - we do not use `for f in provider.getFeatures()` as we are also
        # testing that existing attributes & geometry in f are overwritten correctly
        # (for f in ... uses a new QgsFeature for every iteration)

        it = self.provider.getFeatures()
        f = QgsFeature()
        attributes = {}
        geometries = {}
        while it.nextFeature(f):
            # expect feature to be valid
            self.assertTrue(f.isValid())
            # split off the first 5 attributes only - some provider test datasets will include
            # additional attributes which we ignore
            attrs = f.attributes()[0:5]
            # force the num_char attribute to be text - some providers (eg delimited text) will
            # automatically detect that this attribute contains numbers and set it as a numeric
            # field
            attrs[4] = str(attrs[4])
            attributes[f['pk']] = attrs
            geometries[f['pk']] = f.constGeometry() and f.constGeometry().exportToWkt()

        expected_attributes = {5: [5, -200, NULL, 'NuLl', '5'],
                               3: [3, 300, 'Pear', 'PEaR', '3'],
                               1: [1, 100, 'Orange', 'oranGe', '1'],
                               2: [2, 200, 'Apple', 'Apple', '2'],
                               4: [4, 400, 'Honey', 'Honey', '4']}
        self.assertEqual(attributes, expected_attributes, 'Expected {}, got {}'.format(expected_attributes, attributes))

        expected_geometries = {1: 'Point (-70.332 66.33)',
                               2: 'Point (-68.2 70.8)',
                               3: None,
                               4: 'Point(-65.32 78.3)',
                               5: 'Point(-71.123 78.23)'}
        for pk, geom in expected_geometries.items():
            if geom:
                assert compareWkt(geom, geometries[pk]), "Geometry {} mismatch Expected:\n{}\nGot:\n{}\n".format(pk, geom, geometries[pk].exportToWkt())
            else:
                self.assertFalse(geometries[pk], 'Expected null geometry for {}'.format(pk))

    def uncompiledFilters(self):
        """ Individual derived provider tests should override this to return a list of expressions which
        cannot be compiled """
        return set()

    def partiallyCompiledFilters(self):
        """ Individual derived provider tests should override this to return a list of expressions which
        should be partially compiled """
        return set()

    def assert_query(self, provider, expression, expected):
        request = QgsFeatureRequest().setFilterExpression(expression).setFlags(QgsFeatureRequest.NoGeometry)
        result = set([f['pk'] for f in provider.getFeatures(request)])
        assert set(expected) == result, 'Expected {} and got {} when testing expression "{}"'.format(set(expected), result, expression)
        self.assertTrue(all(f.isValid() for f in provider.getFeatures(request)))

        if self.compiled:
            # Check compilation status
            it = provider.getFeatures(QgsFeatureRequest().setFilterExpression(expression))

            if expression in self.uncompiledFilters():
                self.assertEqual(it.compileStatus(), QgsAbstractFeatureIterator.NoCompilation)
            elif expression in self.partiallyCompiledFilters():
                self.assertEqual(it.compileStatus(), QgsAbstractFeatureIterator.PartiallyCompiled)
            else:
                self.assertEqual(it.compileStatus(), QgsAbstractFeatureIterator.Compiled)

        # Also check that filter works when referenced fields are not being retrieved by request
        result = set([f['pk'] for f in provider.getFeatures(QgsFeatureRequest().setFilterExpression(expression).setSubsetOfAttributes([0]))])
        assert set(expected) == result, 'Expected {} and got {} when testing expression "{}" using empty attribute subset'.format(set(expected), result, expression)

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
        self.assert_query(provider, '-cnt > 0', [5])
        self.assert_query(provider, 'cnt < 0', [5])
        self.assert_query(provider, '-cnt < 0', [1, 2, 3, 4])
        self.assert_query(provider, 'cnt >= 100', [1, 2, 3, 4])
        self.assert_query(provider, 'cnt <= 100', [1, 5])
        self.assert_query(provider, 'pk IN (1, 2, 4, 8)', [1, 2, 4])
        self.assert_query(provider, 'cnt = 50 * 2', [1])
        self.assert_query(provider, 'cnt = 99 + 1', [1])
        self.assert_query(provider, 'cnt = 101 - 1', [1])
        self.assert_query(provider, 'cnt - 1 = 99', [1])
        self.assert_query(provider, '-cnt - 1 = -101', [1])
        self.assert_query(provider, '-(-cnt) = 100', [1])
        self.assert_query(provider, '-(cnt) = -(100)', [1])
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

        # geometry
        self.assert_query(provider, 'intersects($geometry,geom_from_wkt( \'Polygon ((-72.2 66.1, -65.2 66.1, -65.2 72.0, -72.2 72.0, -72.2 66.1))\'))', [1, 2])

    def testGetFeaturesUncompiled(self):
        self.compiled = False
        try:
            self.disableCompiler()
        except AttributeError:
            pass
        self.runGetFeatureTests(self.provider)

    def testGetFeaturesCompiled(self):
        try:
            self.enableCompiler()
            self.compiled = True
            self.runGetFeatureTests(self.provider)
        except AttributeError:
            print('Provider does not support compiling')

    def testSubsetString(self):
        if not self.provider.supportsSubsetString():
            print('Provider does not support subset strings')
            return

        subset = self.getSubsetString()
        self.provider.setSubsetString(subset)
        self.assertEqual(self.provider.subsetString(), subset)
        result = set([f['pk'] for f in self.provider.getFeatures()])
        all_valid = (all(f.isValid() for f in self.provider.getFeatures()))
        self.provider.setSubsetString(None)

        expected = set([2, 3, 4])
        assert set(expected) == result, 'Expected {} and got {} when testing subset string {}'.format(set(expected), result, subset)
        self.assertTrue(all_valid)

        # Subset string AND filter rect
        self.provider.setSubsetString(subset)
        extent = QgsRectangle(-70, 70, -60, 75)
        request = QgsFeatureRequest().setFilterRect(extent)
        result = set([f['pk'] for f in self.provider.getFeatures(request)])
        all_valid = (all(f.isValid() for f in self.provider.getFeatures(request)))
        self.provider.setSubsetString(None)
        expected = set([2])
        assert set(expected) == result, 'Expected {} and got {} when testing subset string {}'.format(set(expected), result, subset)
        self.assertTrue(all_valid)

        # Subset string AND filter rect, version 2
        self.provider.setSubsetString(subset)
        extent = QgsRectangle(-71, 65, -60, 80)
        result = set([f['pk'] for f in self.provider.getFeatures(QgsFeatureRequest().setFilterRect(extent))])
        self.provider.setSubsetString(None)
        expected = set([2, 4])
        assert set(expected) == result, 'Expected {} and got {} when testing subset string {}'.format(set(expected), result, subset)

        # Subset string AND expression
        self.provider.setSubsetString(subset)
        request = QgsFeatureRequest().setFilterExpression('length("name")=5')
        result = set([f['pk'] for f in self.provider.getFeatures(request)])
        all_valid = (all(f.isValid() for f in self.provider.getFeatures(request)))
        self.provider.setSubsetString(None)
        expected = set([2, 4])
        assert set(expected) == result, 'Expected {} and got {} when testing subset string {}'.format(set(expected), result, subset)
        self.assertTrue(all_valid)

    def getSubsetString(self):
        """Individual providers may need to override this depending on their subset string formats"""
        return '"cnt" > 100 and "cnt" < 410'

    def getSubsetString2(self):
        """Individual providers may need to override this depending on their subset string formats"""
        return '"cnt" > 100 and "cnt" < 400'

    def testOrderByUncompiled(self):
        try:
            self.disableCompiler()
        except AttributeError:
            pass
        self.runOrderByTests()

    def testOrderByCompiled(self):
        try:
            self.enableCompiler()
            self.runOrderByTests()
        except AttributeError:
            print('Provider does not support compiling')

    def runOrderByTests(self):
        request = QgsFeatureRequest().addOrderBy('cnt')
        values = [f['cnt'] for f in self.provider.getFeatures(request)]
        self.assertEqual(values, [-200, 100, 200, 300, 400])

        request = QgsFeatureRequest().addOrderBy('cnt', False)
        values = [f['cnt'] for f in self.provider.getFeatures(request)]
        self.assertEqual(values, [400, 300, 200, 100, -200])

        request = QgsFeatureRequest().addOrderBy('name')
        values = [f['name'] for f in self.provider.getFeatures(request)]
        self.assertEqual(values, ['Apple', 'Honey', 'Orange', 'Pear', NULL])

        request = QgsFeatureRequest().addOrderBy('name', True, True)
        values = [f['name'] for f in self.provider.getFeatures(request)]
        self.assertEqual(values, [NULL, 'Apple', 'Honey', 'Orange', 'Pear'])

        request = QgsFeatureRequest().addOrderBy('name', False)
        values = [f['name'] for f in self.provider.getFeatures(request)]
        self.assertEqual(values, [NULL, 'Pear', 'Orange', 'Honey', 'Apple'])

        request = QgsFeatureRequest().addOrderBy('name', False, False)
        values = [f['name'] for f in self.provider.getFeatures(request)]
        self.assertEqual(values, ['Pear', 'Orange', 'Honey', 'Apple', NULL])

        # Case sensitivity
        request = QgsFeatureRequest().addOrderBy('name2')
        values = [f['name2'] for f in self.provider.getFeatures(request)]
        self.assertEqual(values, ['Apple', 'Honey', 'NuLl', 'oranGe', 'PEaR'])

        # Combination with LIMIT
        request = QgsFeatureRequest().addOrderBy('pk', False).setLimit(2)
        values = [f['pk'] for f in self.provider.getFeatures(request)]
        self.assertEqual(values, [5, 4])

        # A slightly more complex expression
        request = QgsFeatureRequest().addOrderBy('pk*2', False)
        values = [f['pk'] for f in self.provider.getFeatures(request)]
        self.assertEqual(values, [5, 4, 3, 2, 1])

        # Order reversing expression
        request = QgsFeatureRequest().addOrderBy('pk*-1', False)
        values = [f['pk'] for f in self.provider.getFeatures(request)]
        self.assertEqual(values, [1, 2, 3, 4, 5])

        # Type dependent expression
        request = QgsFeatureRequest().addOrderBy('num_char*2', False)
        values = [f['pk'] for f in self.provider.getFeatures(request)]
        self.assertEqual(values, [5, 4, 3, 2, 1])

        # Order by guaranteed to fail
        request = QgsFeatureRequest().addOrderBy('not a valid expression*', False)
        values = [f['pk'] for f in self.provider.getFeatures(request)]
        self.assertEqual(set(values), set([5, 4, 3, 2, 1]))

        # Multiple order bys and boolean
        request = QgsFeatureRequest().addOrderBy('pk > 2').addOrderBy('pk', False)
        values = [f['pk'] for f in self.provider.getFeatures(request)]
        self.assertEqual(values, [2, 1, 5, 4, 3])

        # Multiple order bys, one bad, and a limit
        request = QgsFeatureRequest().addOrderBy('pk', False).addOrderBy('not a valid expression*', False).setLimit(2)
        values = [f['pk'] for f in self.provider.getFeatures(request)]
        self.assertEqual(values, [5, 4])

        # Bad expression first
        request = QgsFeatureRequest().addOrderBy('not a valid expression*', False).addOrderBy('pk', False).setLimit(2)
        values = [f['pk'] for f in self.provider.getFeatures(request)]
        self.assertEqual(values, [5, 4])

        # Combination with subset of attributes
        request = QgsFeatureRequest().addOrderBy('num_char', False).setSubsetOfAttributes(['pk'], self.vl.fields())
        values = [f['pk'] for f in self.vl.getFeatures(request)]
        self.assertEqual(values, [5, 4, 3, 2, 1])

    def testGetFeaturesFidTests(self):
        fids = [f.id() for f in self.provider.getFeatures()]
        assert len(fids) == 5, 'Expected 5 features, got {} instead'.format(len(fids))
        for id in fids:
            features = [f for f in self.provider.getFeatures(QgsFeatureRequest().setFilterFid(id))]
            self.assertEqual(len(features), 1)
            feature = features[0]
            self.assertTrue(feature.isValid())

            result = [feature.id()]
            expected = [id]
            assert result == expected, 'Expected {} and got {} when testing for feature ID filter'.format(expected, result)

        # bad features
        it = self.provider.getFeatures(QgsFeatureRequest().setFilterFid(-99999999))
        feature = QgsFeature(5)
        feature.setValid(False)
        self.assertFalse(it.nextFeature(feature))
        self.assertFalse(feature.isValid())

    def testGetFeaturesFidsTests(self):
        fids = [f.id() for f in self.provider.getFeatures()]
        self.assertEqual(len(fids), 5)

        request = QgsFeatureRequest().setFilterFids([fids[0], fids[2]])
        result = set([f.id() for f in self.provider.getFeatures(request)])
        all_valid = (all(f.isValid() for f in self.provider.getFeatures(request)))
        expected = set([fids[0], fids[2]])
        assert result == expected, 'Expected {} and got {} when testing for feature IDs filter'.format(expected, result)
        self.assertTrue(all_valid)

        result = set([f.id() for f in self.provider.getFeatures(QgsFeatureRequest().setFilterFids([fids[1], fids[3], fids[4]]))])
        expected = set([fids[1], fids[3], fids[4]])
        assert result == expected, 'Expected {} and got {} when testing for feature IDs filter'.format(expected, result)

        result = set([f.id() for f in self.provider.getFeatures(QgsFeatureRequest().setFilterFids([]))])
        expected = set([])
        assert result == expected, 'Expected {} and got {} when testing for feature IDs filter'.format(expected, result)

    def testGetFeaturesFilterRectTests(self):
        extent = QgsRectangle(-70, 67, -60, 80)
        request = QgsFeatureRequest().setFilterRect(extent)
        features = [f['pk'] for f in self.provider.getFeatures(request)]
        all_valid = (all(f.isValid() for f in self.provider.getFeatures(request)))
        assert set(features) == set([2, 4]), 'Got {} instead'.format(features)
        self.assertTrue(all_valid)

        # test with an empty rectangle
        extent = QgsRectangle()
        request = QgsFeatureRequest().setFilterRect(extent)
        features = [f['pk'] for f in self.provider.getFeatures(request)]
        all_valid = (all(f.isValid() for f in self.provider.getFeatures(request)))
        assert set(features) == set([1, 2, 3, 4, 5]), 'Got {} instead'.format(features)
        self.assertTrue(all_valid)

    def testGetFeaturesPolyFilterRectTests(self):
        """ Test fetching features from a polygon layer with filter rect"""
        try:
            if not self.poly_provider:
                return
        except:
            return

        extent = QgsRectangle(-73, 70, -63, 80)
        request = QgsFeatureRequest().setFilterRect(extent)
        features = [f['pk'] for f in self.poly_provider.getFeatures(request)]
        all_valid = (all(f.isValid() for f in self.provider.getFeatures(request)))
        # Some providers may return the exact intersection matches (2, 3) even without the ExactIntersect flag, so we accept that too
        assert set(features) == set([2, 3]) or set(features) == set([1, 2, 3]), 'Got {} instead'.format(features)
        self.assertTrue(all_valid)

        # Test with exact intersection
        request = QgsFeatureRequest().setFilterRect(extent).setFlags(QgsFeatureRequest.ExactIntersect)
        features = [f['pk'] for f in self.poly_provider.getFeatures(request)]
        all_valid = (all(f.isValid() for f in self.provider.getFeatures(request)))
        assert set(features) == set([2, 3]), 'Got {} instead'.format(features)
        self.assertTrue(all_valid)

        # test with an empty rectangle
        extent = QgsRectangle()
        features = [f['pk'] for f in self.provider.getFeatures(QgsFeatureRequest().setFilterRect(extent))]
        assert set(features) == set([1, 2, 3, 4, 5]), 'Got {} instead'.format(features)

    def testRectAndExpression(self):
        extent = QgsRectangle(-70, 67, -60, 80)
        request = QgsFeatureRequest().setFilterExpression('"cnt">200').setFilterRect(extent)
        result = set([f['pk'] for f in self.provider.getFeatures(request)])
        all_valid = (all(f.isValid() for f in self.provider.getFeatures(request)))
        expected = [4]
        assert set(expected) == result, 'Expected {} and got {} when testing for combination of filterRect and expression'.format(set(expected), result)
        self.assertTrue(all_valid)

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

        subset = self.getSubsetString()
        self.provider.setSubsetString(subset)
        min_value = self.provider.minimumValue(1)
        self.provider.setSubsetString(None)
        self.assertEqual(min_value, 200)

    def testMaxValue(self):
        self.assertEqual(self.provider.maximumValue(1), 400)
        self.assertEqual(self.provider.maximumValue(2), 'Pear')

        subset = self.getSubsetString2()
        self.provider.setSubsetString(subset)
        max_value = self.provider.maximumValue(1)
        self.provider.setSubsetString(None)
        self.assertEqual(max_value, 300)

    def testExtent(self):
        reference = QgsGeometry.fromRect(
            QgsRectangle(-71.123, 66.33, -65.32, 78.3))
        provider_extent = QgsGeometry.fromRect(self.provider.extent())

        self.assertTrue(QgsGeometry.compare(provider_extent.asPolygon()[0], reference.asPolygon()[0], 0.00001))

    def testUnique(self):
        self.assertEqual(set(self.provider.uniqueValues(1)), set([-200, 100, 200, 300, 400]))
        assert set([u'Apple', u'Honey', u'Orange', u'Pear', NULL]) == set(self.provider.uniqueValues(2)), 'Got {}'.format(set(self.provider.uniqueValues(2)))

        subset = self.getSubsetString2()
        self.provider.setSubsetString(subset)
        values = self.provider.uniqueValues(1)
        self.provider.setSubsetString(None)
        self.assertEqual(set(values), set([200, 300]))

    def testFeatureCount(self):
        assert self.provider.featureCount() == 5, 'Got {}'.format(self.provider.featureCount())

        #Add a subset string and test feature count
        subset = self.getSubsetString()
        self.provider.setSubsetString(subset)
        count = self.provider.featureCount()
        self.provider.setSubsetString(None)
        assert count == 3, 'Got {}'.format(count)

    def testClosedIterators(self):
        """ Test behaviour of closed iterators """

        # Test retrieving feature after closing iterator
        f_it = self.provider.getFeatures(QgsFeatureRequest())
        fet = QgsFeature()
        assert f_it.nextFeature(fet), 'Could not fetch feature'
        assert fet.isValid(), 'Feature is not valid'
        assert f_it.close(), 'Could not close iterator'
        self.assertFalse(f_it.nextFeature(fet), 'Fetched feature after iterator closed, expected nextFeature() to return False')
        self.assertFalse(fet.isValid(), 'Valid feature fetched from closed iterator, should be invalid')

        # Test rewinding closed iterator
        self.assertFalse(f_it.rewind(), 'Rewinding closed iterator successful, should not be allowed')

    def testGetFeaturesSubsetAttributes(self):
        """ Test that expected results are returned when using subsets of attributes """

        tests = {'pk': set([1, 2, 3, 4, 5]),
                 'cnt': set([-200, 300, 100, 200, 400]),
                 'name': set(['Pear', 'Orange', 'Apple', 'Honey', NULL]),
                 'name2': set(['NuLl', 'PEaR', 'oranGe', 'Apple', 'Honey'])}
        for field, expected in tests.items():
            request = QgsFeatureRequest().setSubsetOfAttributes([field], self.provider.fields())
            result = set([f[field] for f in self.provider.getFeatures(request)])
            all_valid = (all(f.isValid() for f in self.provider.getFeatures(request)))
            self.assertEqual(result, expected, 'Expected {}, got {}'.format(expected, result))
            self.assertTrue(all_valid)

    def testGetFeaturesSubsetAttributes2(self):
        """ Test that other fields are NULL when fetching subsets of attributes """

        for field_to_fetch in ['pk', 'cnt', 'name', 'name2']:
            for f in self.provider.getFeatures(QgsFeatureRequest().setSubsetOfAttributes([field_to_fetch], self.provider.fields())):
                # Check that all other fields are NULL and force name to lower-case
                for other_field in [field.name() for field in self.provider.fields() if field.name().lower() != field_to_fetch]:
                    if other_field == 'pk' or other_field == 'PK':
                        # skip checking the primary key field, as it may be validly fetched by providers to use as feature id
                        continue
                    self.assertEqual(f[other_field], NULL, 'Value for field "{}" was present when it should not have been fetched by request'.format(other_field))

    def testGetFeaturesNoGeometry(self):
        """ Test that no geometry is present when fetching features without geometry"""

        for f in self.provider.getFeatures(QgsFeatureRequest().setFlags(QgsFeatureRequest.NoGeometry)):
            self.assertFalse(f.constGeometry(), 'Expected no geometry, got one')
            self.assertTrue(f.isValid())

    def testGetFeaturesWithGeometry(self):
        """ Test that geometry is present when fetching features without setting NoGeometry flag"""
        for f in self.provider.getFeatures(QgsFeatureRequest()):
            if f['pk'] == 3:
                # no geometry for this feature
                continue

            assert f.constGeometry(), 'Expected geometry, got none'
            self.assertTrue(f.isValid())
