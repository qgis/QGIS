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

    def testSubsetString(self):
        if not self.provider.supportsSubsetString():
            print 'Provider does not support subset strings'
            return

        subset = self.getSubsetString()
        self.provider.setSubsetString(subset)
        self.assertEqual(self.provider.subsetString(), subset)
        result = set([f['pk'] for f in self.provider.getFeatures()])
        self.provider.setSubsetString(None)

        expected = set([2, 3, 4])
        assert set(expected) == result, 'Expected {} and got {} when testing subset string {}'.format(set(expected), result, subset)

        # Subset string AND filter rect
        self.provider.setSubsetString(subset)
        extent = QgsRectangle(-70, 70, -60, 75)
        result = set([f['pk'] for f in self.provider.getFeatures(QgsFeatureRequest().setFilterRect(extent))])
        self.provider.setSubsetString(None)
        expected = set([2])
        assert set(expected) == result, 'Expected {} and got {} when testing subset string {}'.format(set(expected), result, subset)

        # Subset string AND filter rect, version 2
        self.provider.setSubsetString(subset)
        extent = QgsRectangle(-71, 65, -60, 80)
        result = set([f['pk'] for f in self.provider.getFeatures(QgsFeatureRequest().setFilterRect(extent))])
        self.provider.setSubsetString(None)
        expected = set([2, 4])
        assert set(expected) == result, 'Expected {} and got {} when testing subset string {}'.format(set(expected), result, subset)

        # Subset string AND expression
        self.provider.setSubsetString(subset)
        result = set([f['pk'] for f in self.provider.getFeatures(QgsFeatureRequest().setFilterExpression('length("name")=5'))])
        self.provider.setSubsetString(None)
        expected = set([2, 4])
        assert set(expected) == result, 'Expected {} and got {} when testing subset string {}'.format(set(expected), result, subset)

    def getSubsetString(self):
        """Individual providers may need to override this depending on their subset string formats"""
        return '"cnt" > 100 and "cnt" < 410'

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
            print 'Provider does not support compiling'

    def runOrderByTests(self):
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

        # Order reversing expression
        request = QgsFeatureRequest().addOrderBy('pk*-1', False)
        values = [f['pk'] for f in self.provider.getFeatures(request)]
        self.assertEquals(values, [1, 2, 3, 4, 5])

        # Type dependent expression
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

    def testGetFeaturesPolyFilterRectTests(self):
        """ Test fetching features from a polygon layer with filter rect"""
        try:
            if not self.poly_provider:
                return
        except:
            return

        extent = QgsRectangle(-73, 70, -63, 80)
        features = [f['pk'] for f in self.poly_provider.getFeatures(QgsFeatureRequest().setFilterRect(extent))]
        # Some providers may return the exact intersection matches (2, 3) even without the ExactIntersect flag, so we accept that too
        assert set(features) == set([2, 3]) or set(features) == set([1, 2, 3]), 'Got {} instead'.format(features)

        # Test with exact intersection
        features = [f['pk'] for f in self.poly_provider.getFeatures(QgsFeatureRequest().setFilterRect(extent).setFlags(QgsFeatureRequest.ExactIntersect))]
        assert set(features) == set([2, 3]), 'Got {} instead'.format(features)

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
        for field, expected in tests.iteritems():
            result = set([f[field] for f in self.provider.getFeatures(QgsFeatureRequest().setSubsetOfAttributes([field], self.provider.fields()))])
            self.assertEqual(result, expected, 'Expected {}, got {}'.format(expected, result))

    def testGetFeaturesSubsetAttributes2(self):
        """ Test that other fields are NULL wen fetching subsets of attributes """

        for field_to_fetch in ['pk', 'cnt', 'name', 'name2']:
            for f in self.provider.getFeatures(QgsFeatureRequest().setSubsetOfAttributes([field_to_fetch], self.provider.fields())):
                # Check that all other fields are NULL
                for other_field in [field.name() for field in self.provider.fields() if field.name() != field_to_fetch]:
                    self.assertEqual(f[other_field], NULL, 'Value for field "{}" was present when it should not have been fetched by request'.format(other_field))

    def testGetFeaturesNoGeometry(self):
        """ Test that no geometry is present when fetching features without geometry"""

        for f in self.provider.getFeatures(QgsFeatureRequest().setFlags(QgsFeatureRequest.NoGeometry)):
            self.assertFalse(f.constGeometry(), 'Expected no geometry, got one')

    def testGetFeaturesNoGeometry(self):
        """ Test that geometry is present when fetching features without setting NoGeometry flag"""
        for f in self.provider.getFeatures(QgsFeatureRequest()):
            if f['pk'] == 3:
                # no geometry for this feature
                continue

            assert f.constGeometry(), 'Expected geometry, got none'
