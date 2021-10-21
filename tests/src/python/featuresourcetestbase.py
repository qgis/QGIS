# -*- coding: utf-8 -*-
"""QGIS Unit test utils for QgsFeatureSource subclasses.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from builtins import str
from builtins import object

__author__ = 'Nyall Dawson'
__date__ = '2017-05-25'
__copyright__ = 'Copyright 2017, The QGIS Project'

from qgis.core import (
    QgsRectangle,
    QgsFeatureRequest,
    QgsFeature,
    QgsWkbTypes,
    QgsProject,
    QgsGeometry,
    QgsAbstractFeatureIterator,
    QgsExpressionContextScope,
    QgsExpressionContext,
    QgsVectorLayerFeatureSource,
    QgsCoordinateReferenceSystem,
    NULL
)
from qgis.PyQt.QtCore import QDate, QTime, QDateTime

from utilities import compareWkt


class FeatureSourceTestCase(object):
    """
    This is a collection of tests for QgsFeatureSources subclasses and kept generic.
    To make use of it, subclass it and set self.source to a QgsFeatureSource you want to test.
    Make sure that your source uses the default dataset by converting one of the provided datasets from the folder
    tests/testdata/source to a dataset your source is able to handle.
    """

    def treat_date_as_datetime(self):
        return False

    def treat_datetime_as_string(self):
        return False

    def treat_date_as_string(self):
        return False

    def treat_time_as_string(self):
        return False

    def testCrs(self):
        self.assertEqual(self.source.sourceCrs().authid(), 'EPSG:4326')

    def testWkbType(self):
        self.assertEqual(self.source.wkbType(), QgsWkbTypes.Point)

    def testFeatureCount(self):
        self.assertEqual(self.source.featureCount(), 5)
        self.assertEqual(len(self.source), 5)

    def testFields(self):
        fields = self.source.fields()
        for f in ('pk', 'cnt', 'name', 'name2', 'num_char'):
            self.assertTrue(fields.lookupField(f) >= 0)

    def testGetFeatures(self, source=None, extra_features=[], skip_features=[], changed_attributes={},
                        changed_geometries={}):
        """ Test that expected results are returned when fetching all features """

        # IMPORTANT - we do not use `for f in source.getFeatures()` as we are also
        # testing that existing attributes & geometry in f are overwritten correctly
        # (for f in ... uses a new QgsFeature for every iteration)

        if not source:
            source = self.source

        it = source.getFeatures()
        f = QgsFeature()
        attributes = {}
        geometries = {}
        while it.nextFeature(f):
            # expect feature to be valid
            self.assertTrue(f.isValid())
            # some source test datasets will include additional attributes which we ignore,
            # so cherry pick desired attributes
            attrs = [f['pk'], f['cnt'], f['name'], f['name2'], f['num_char'], f['dt'], f['date'], f['time']]
            # force the num_char attribute to be text - some sources (e.g., delimited text) will
            # automatically detect that this attribute contains numbers and set it as a numeric
            # field
            attrs[4] = str(attrs[4])
            attributes[f['pk']] = attrs
            geometries[f['pk']] = f.hasGeometry() and f.geometry().asWkt()

        expected_attributes = {5: [5, -200, NULL, 'NuLl', '5', QDateTime(QDate(2020, 5, 4), QTime(12, 13, 14)) if not self.treat_datetime_as_string() else '2020-05-04 12:13:14', QDate(2020, 5, 2) if not self.treat_date_as_datetime() and not self.treat_date_as_string() else QDateTime(2020, 5, 2, 0, 0, 0) if not self.treat_date_as_string() else '2020-05-02', QTime(12, 13, 1) if not self.treat_time_as_string() else '12:13:01'],
                               3: [3, 300, 'Pear', 'PEaR', '3', NULL, NULL, NULL],
                               1: [1, 100, 'Orange', 'oranGe', '1', QDateTime(QDate(2020, 5, 3), QTime(12, 13, 14)) if not self.treat_datetime_as_string() else '2020-05-03 12:13:14', QDate(2020, 5, 3) if not self.treat_date_as_datetime() and not self.treat_date_as_string() else QDateTime(2020, 5, 3, 0, 0, 0) if not self.treat_date_as_string() else '2020-05-03', QTime(12, 13, 14) if not self.treat_time_as_string() else '12:13:14'],
                               2: [2, 200, 'Apple', 'Apple', '2', QDateTime(QDate(2020, 5, 4), QTime(12, 14, 14)) if not self.treat_datetime_as_string() else '2020-05-04 12:14:14', QDate(2020, 5, 4) if not self.treat_date_as_datetime() and not self.treat_date_as_string() else QDateTime(2020, 5, 4, 0, 0, 0) if not self.treat_date_as_string() else '2020-05-04', QTime(12, 14, 14) if not self.treat_time_as_string() else '12:14:14'],
                               4: [4, 400, 'Honey', 'Honey', '4', QDateTime(QDate(2021, 5, 4), QTime(13, 13, 14)) if not self.treat_datetime_as_string() else '2021-05-04 13:13:14', QDate(2021, 5, 4) if not self.treat_date_as_datetime() and not self.treat_date_as_string() else QDateTime(2021, 5, 4, 0, 0, 0) if not self.treat_date_as_string() else '2021-05-04', QTime(13, 13, 14) if not self.treat_time_as_string() else '13:13:14']}

        expected_geometries = {1: 'Point (-70.332 66.33)',
                               2: 'Point (-68.2 70.8)',
                               3: None,
                               4: 'Point(-65.32 78.3)',
                               5: 'Point(-71.123 78.23)'}
        for f in extra_features:
            expected_attributes[f[0]] = f.attributes()
            if f.hasGeometry():
                expected_geometries[f[0]] = f.geometry().asWkt()
            else:
                expected_geometries[f[0]] = None

        for i in skip_features:
            del expected_attributes[i]
            del expected_geometries[i]
        for i, a in changed_attributes.items():
            for attr_idx, v in a.items():
                expected_attributes[i][attr_idx] = v
        for i, g, in changed_geometries.items():
            if g:
                expected_geometries[i] = g.asWkt()
            else:
                expected_geometries[i] = None

        self.assertEqual(attributes, expected_attributes, 'Expected {}, got {}'.format(expected_attributes, attributes))

        self.assertEqual(len(expected_geometries), len(geometries))

        for pk, geom in list(expected_geometries.items()):
            if geom:
                assert compareWkt(geom, geometries[pk]), "Geometry {} mismatch Expected:\n{}\nGot:\n{}\n".format(pk,
                                                                                                                 geom,
                                                                                                                 geometries[
                                                                                                                     pk])
            else:
                self.assertFalse(geometries[pk], 'Expected null geometry for {}'.format(pk))

    def assert_query(self, source, expression, expected):
        request = QgsFeatureRequest().setFilterExpression(expression).setFlags(QgsFeatureRequest.NoGeometry | QgsFeatureRequest.IgnoreStaticNodesDuringExpressionCompilation)
        result = set([f['pk'] for f in source.getFeatures(request)])
        assert set(expected) == result, 'Expected {} and got {} when testing expression "{}"'.format(set(expected),
                                                                                                     result, expression)
        self.assertTrue(all(f.isValid() for f in source.getFeatures(request)))

        # Also check that filter works when referenced fields are not being retrieved by request
        result = set([f['pk'] for f in source.getFeatures(
            QgsFeatureRequest().setFilterExpression(expression).setSubsetOfAttributes(['pk'], self.source.fields()).setFlags(QgsFeatureRequest.IgnoreStaticNodesDuringExpressionCompilation))])
        assert set(
            expected) == result, 'Expected {} and got {} when testing expression "{}" using empty attribute subset'.format(
            set(expected), result, expression)

        # test that results match QgsFeatureRequest.acceptFeature
        request = QgsFeatureRequest().setFilterExpression(expression).setFlags(QgsFeatureRequest.IgnoreStaticNodesDuringExpressionCompilation)
        for f in source.getFeatures():
            self.assertEqual(request.acceptFeature(f), f['pk'] in expected)

    def runGetFeatureTests(self, source):
        self.assertEqual(len([f for f in source.getFeatures()]), 5)
        self.assert_query(source, 'name ILIKE \'QGIS\'', [])
        self.assert_query(source, '"name" IS NULL', [5])
        self.assert_query(source, '"name" IS NOT NULL', [1, 2, 3, 4])
        self.assert_query(source, '"name" NOT LIKE \'Ap%\'', [1, 3, 4])
        self.assert_query(source, '"name" NOT ILIKE \'QGIS\'', [1, 2, 3, 4])
        self.assert_query(source, '"name" NOT ILIKE \'pEAR\'', [1, 2, 4])
        self.assert_query(source, 'name = \'Apple\'', [2])
        # field names themselves are NOT case sensitive -- QGIS expressions don't care about this
        self.assert_query(source, '\"NaMe\" = \'Apple\'', [2])
        self.assert_query(source, 'name <> \'Apple\'', [1, 3, 4])
        self.assert_query(source, 'name = \'apple\'', [])
        self.assert_query(source, '"name" <> \'apple\'', [1, 2, 3, 4])
        self.assert_query(source, '(name = \'Apple\') is not null', [1, 2, 3, 4])
        self.assert_query(source, 'name LIKE \'Apple\'', [2])
        self.assert_query(source, 'name LIKE \'aPple\'', [])
        self.assert_query(source, 'name LIKE \'Ap_le\'', [2])
        self.assert_query(source, 'name LIKE \'Ap\\_le\'', [])
        self.assert_query(source, 'name ILIKE \'aPple\'', [2])
        self.assert_query(source, 'name ILIKE \'%pp%\'', [2])
        self.assert_query(source, 'cnt > 0', [1, 2, 3, 4])
        self.assert_query(source, '-cnt > 0', [5])
        self.assert_query(source, 'cnt < 0', [5])
        self.assert_query(source, '-cnt < 0', [1, 2, 3, 4])
        self.assert_query(source, 'cnt >= 100', [1, 2, 3, 4])
        self.assert_query(source, 'cnt <= 100', [1, 5])
        self.assert_query(source, 'pk IN (1, 2, 4, 8)', [1, 2, 4])
        self.assert_query(source, 'cnt = 50 * 2', [1])
        self.assert_query(source, 'cnt = 150 / 1.5', [1])
        self.assert_query(source, 'cnt = 1000 / 10', [1])
        self.assert_query(source, 'cnt = 1000/11+10', [])  # checks that source isn't rounding int/int
        self.assert_query(source, 'pk = 9 // 4', [2])  # int division
        self.assert_query(source, 'cnt = 99 + 1', [1])
        self.assert_query(source, 'cnt = 101 - 1', [1])
        self.assert_query(source, 'cnt - 1 = 99', [1])
        self.assert_query(source, '-cnt - 1 = -101', [1])
        self.assert_query(source, '-(-cnt) = 100', [1])
        self.assert_query(source, '-(cnt) = -(100)', [1])
        self.assert_query(source, 'cnt + 1 = 101', [1])
        self.assert_query(source, 'cnt = 1100 % 1000', [1])
        self.assert_query(source, '"name" || \' \' || "name" = \'Orange Orange\'', [1])
        self.assert_query(source, '"name" || \' \' || "cnt" = \'Orange 100\'', [1])
        self.assert_query(source, '\'x\' || "name" IS NOT NULL', [1, 2, 3, 4])
        self.assert_query(source, '\'x\' || "name" IS NULL', [5])
        self.assert_query(source, 'cnt = 10 ^ 2', [1])
        self.assert_query(source, '"name" ~ \'[OP]ra[gne]+\'', [1])
        self.assert_query(source, '"name"="name2"', [2, 4])  # mix of matched and non-matched case sensitive names
        self.assert_query(source, 'true', [1, 2, 3, 4, 5])
        self.assert_query(source, 'false', [])

        # Three value logic
        self.assert_query(source, 'false and false', [])
        self.assert_query(source, 'false and true', [])
        self.assert_query(source, 'false and NULL', [])
        self.assert_query(source, 'true and false', [])
        self.assert_query(source, 'true and true', [1, 2, 3, 4, 5])
        self.assert_query(source, 'true and NULL', [])
        self.assert_query(source, 'NULL and false', [])
        self.assert_query(source, 'NULL and true', [])
        self.assert_query(source, 'NULL and NULL', [])
        self.assert_query(source, 'false or false', [])
        self.assert_query(source, 'false or true', [1, 2, 3, 4, 5])
        self.assert_query(source, 'false or NULL', [])
        self.assert_query(source, 'true or false', [1, 2, 3, 4, 5])
        self.assert_query(source, 'true or true', [1, 2, 3, 4, 5])
        self.assert_query(source, 'true or NULL', [1, 2, 3, 4, 5])
        self.assert_query(source, 'NULL or false', [])
        self.assert_query(source, 'NULL or true', [1, 2, 3, 4, 5])
        self.assert_query(source, 'NULL or NULL', [])
        self.assert_query(source, 'not true', [])
        self.assert_query(source, 'not false', [1, 2, 3, 4, 5])
        self.assert_query(source, 'not null', [])

        # not
        self.assert_query(source, 'not name = \'Apple\'', [1, 3, 4])
        self.assert_query(source, 'not name IS NULL', [1, 2, 3, 4])
        self.assert_query(source, 'not name = \'Apple\' or name = \'Apple\'', [1, 2, 3, 4])
        self.assert_query(source, 'not name = \'Apple\' or not name = \'Apple\'', [1, 3, 4])
        self.assert_query(source, 'not name = \'Apple\' and pk = 4', [4])
        self.assert_query(source, 'not name = \'Apple\' and not pk = 4', [1, 3])
        self.assert_query(source, 'not pk IN (1, 2, 4, 8)', [3, 5])

        # type conversion - QGIS expressions do not mind that we are comparing a string
        # against numeric literals
        self.assert_query(source, 'num_char IN (2, 4, 5)', [2, 4, 5])

        # function
        self.assert_query(source, 'sqrt(pk) >= 2', [4, 5])
        self.assert_query(source, 'radians(cnt) < 2', [1, 5])
        self.assert_query(source, 'degrees(pk) <= 200', [1, 2, 3])
        self.assert_query(source, 'abs(cnt) <= 200', [1, 2, 5])
        self.assert_query(source, 'cos(pk) < 0', [2, 3, 4])
        self.assert_query(source, 'sin(pk) < 0', [4, 5])
        self.assert_query(source, 'tan(pk) < 0', [2, 3, 5])
        self.assert_query(source, 'acos(-1) < pk', [4, 5])
        self.assert_query(source, 'asin(1) < pk', [2, 3, 4, 5])
        self.assert_query(source, 'atan(3.14) < pk', [2, 3, 4, 5])
        self.assert_query(source, 'atan2(3.14, pk) < 1', [3, 4, 5])
        self.assert_query(source, 'exp(pk) < 10', [1, 2])
        self.assert_query(source, 'ln(pk) <= 1', [1, 2])
        self.assert_query(source, 'log(3, pk) <= 1', [1, 2, 3])
        self.assert_query(source, 'log10(pk) < 0.5', [1, 2, 3])
        self.assert_query(source, 'round(3.14) <= pk', [3, 4, 5])
        self.assert_query(source, 'round(0.314,1) * 10 = pk', [3])
        self.assert_query(source, 'floor(3.14) <= pk', [3, 4, 5])
        self.assert_query(source, 'ceil(3.14) <= pk', [4, 5])
        self.assert_query(source, 'pk < pi()', [1, 2, 3])

        self.assert_query(source, 'round(cnt / 66.67) <= 2', [1, 5])
        self.assert_query(source, 'floor(cnt / 66.67) <= 2', [1, 2, 5])
        self.assert_query(source, 'ceil(cnt / 66.67) <= 2', [1, 5])
        self.assert_query(source, 'pk < pi() / 2', [1])
        self.assert_query(source, 'pk = char(51)', [3])
        self.assert_query(source, 'pk = coalesce(NULL,3,4)', [3])
        self.assert_query(source, 'lower(name) = \'apple\'', [2])
        self.assert_query(source, 'upper(name) = \'APPLE\'', [2])
        self.assert_query(source, 'name = trim(\'   Apple   \')', [2])

        # geometry
        # azimuth and touches tests are deactivated because they do not pass for WFS source
        # self.assert_query(source, 'azimuth($geometry,geom_from_wkt( \'Point (-70 70)\')) < pi()', [1, 5])
        self.assert_query(source, 'x($geometry) < -70', [1, 5])
        self.assert_query(source, 'y($geometry) > 70', [2, 4, 5])
        self.assert_query(source, 'xmin($geometry) < -70', [1, 5])
        self.assert_query(source, 'ymin($geometry) > 70', [2, 4, 5])
        self.assert_query(source, 'xmax($geometry) < -70', [1, 5])
        self.assert_query(source, 'ymax($geometry) > 70', [2, 4, 5])
        self.assert_query(source,
                          'disjoint($geometry,geom_from_wkt( \'Polygon ((-72.2 66.1, -65.2 66.1, -65.2 72.0, -72.2 72.0, -72.2 66.1))\'))',
                          [4, 5])
        self.assert_query(source,
                          'intersects($geometry,geom_from_wkt( \'Polygon ((-72.2 66.1, -65.2 66.1, -65.2 72.0, -72.2 72.0, -72.2 66.1))\'))',
                          [1, 2])
        # self.assert_query(source, 'touches($geometry,geom_from_wkt( \'Polygon ((-70.332 66.33, -65.32 66.33, -65.32 78.3, -70.332 78.3, -70.332 66.33))\'))', [1, 4])
        self.assert_query(source,
                          'contains(geom_from_wkt( \'Polygon ((-72.2 66.1, -65.2 66.1, -65.2 72.0, -72.2 72.0, -72.2 66.1))\'),$geometry)',
                          [1, 2])
        self.assert_query(source, 'distance($geometry,geom_from_wkt( \'Point (-70 70)\')) > 7', [4, 5])
        self.assert_query(source,
                          'intersects($geometry,geom_from_gml( \'<gml:Polygon srsName="EPSG:4326"><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>-72.2,66.1 -65.2,66.1 -65.2,72.0 -72.2,72.0 -72.2,66.1</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon>\'))',
                          [1, 2])

        # datetime
        if self.treat_datetime_as_string():
            self.assert_query(source, '"dt" <= format_date(make_datetime(2020, 5, 4, 12, 13, 14), \'yyyy-MM-dd hh:mm:ss\')', [1, 5])
            self.assert_query(source, '"dt" < format_date(make_date(2020, 5, 4), \'yyyy-MM-dd hh:mm:ss\')', [1])
            self.assert_query(source, '"dt" = format_date(to_datetime(\'000www14ww13ww12www4ww5ww2020\',\'zzzwwwsswwmmwwhhwwwdwwMwwyyyy\'),\'yyyy-MM-dd hh:mm:ss\')', [5])
        else:
            self.assert_query(source, '"dt" <= make_datetime(2020, 5, 4, 12, 13, 14)', [1, 5])
            self.assert_query(source, '"dt" < make_date(2020, 5, 4)', [1])
            self.assert_query(source, '"dt" = to_datetime(\'000www14ww13ww12www4ww5ww2020\',\'zzzwwwsswwmmwwhhwwwdwwMwwyyyy\')', [5])

        self.assert_query(source, '"date" <= make_datetime(2020, 5, 4, 12, 13, 14)', [1, 2, 5])
        self.assert_query(source, '"date" >= make_date(2020, 5, 4)', [2, 4])

        if not self.treat_date_as_datetime():
            self.assert_query(source,
                              '"date" = to_date(\'www4ww5ww2020\',\'wwwdwwMwwyyyy\')',
                              [2])
        else:
            # TODO - we don't have any expression functions which can upgrade a date value to a datetime value!
            pass

        if not self.treat_time_as_string():
            self.assert_query(source, '"time" >= make_time(12, 14, 14)', [2, 4])
            self.assert_query(source, '"time" = to_time(\'000www14ww13ww12www\',\'zzzwwwsswwmmwwhhwww\')', [1])
        else:
            self.assert_query(source, 'to_time("time") >= make_time(12, 14, 14)', [2, 4])
            self.assert_query(source, 'to_time("time") = to_time(\'000www14ww13ww12www\',\'zzzwwwsswwmmwwhhwww\')', [1])

        # TODO - enable, but needs fixing on Travis due to timezone handling issues
        # if self.treat_datetime_as_string():
        #     self.assert_query(source, 'to_datetime("dt", \'yyyy-MM-dd hh:mm:ss\') + make_interval(days:=1) <= make_datetime(2020, 5, 4, 12, 13, 14)', [1])
        #     self.assert_query(source, 'to_datetime("dt", \'yyyy-MM-dd hh:mm:ss\') + make_interval(days:=0.01) <= make_datetime(2020, 5, 4, 12, 13, 14)', [1, 5])
        # else:
        #     self.assert_query(source, '"dt" + make_interval(days:=1) <= make_datetime(2020, 5, 4, 12, 13, 14)', [1])
        #     self.assert_query(source, '"dt" + make_interval(days:=0.01) <= make_datetime(2020, 5, 4, 12, 13, 14)', [1, 5])

        # combination of an uncompilable expression and limit

        # TODO - move this test to FeatureSourceTestCase
        # it's currently added in ProviderTestCase, but tests only using a QgsVectorLayer getting features,
        # i.e. not directly requesting features from the provider. Turns out the WFS provider fails this
        # and should be fixed - then we can enable this test at the FeatureSourceTestCase level

        # feature = next(self.source.getFeatures(QgsFeatureRequest().setFilterExpression('pk=4')))
        # context = QgsExpressionContext()
        # scope = QgsExpressionContextScope()
        # scope.setVariable('parent', feature)
        # context.appendScope(scope)

        # request = QgsFeatureRequest()
        # request.setExpressionContext(context)
        # request.setFilterExpression('"pk" = attribute(@parent, \'pk\')')
        # request.setLimit(1)

        # values = [f['pk'] for f in self.source.getFeatures(request)]
        # self.assertEqual(values, [4])

    def testGetFeaturesExp(self):
        self.runGetFeatureTests(self.source)

    def runOrderByTests(self):
        request = QgsFeatureRequest().addOrderBy('cnt')
        values = [f['cnt'] for f in self.source.getFeatures(request)]
        self.assertEqual(values, [-200, 100, 200, 300, 400])

        request = QgsFeatureRequest().addOrderBy('cnt', False)
        values = [f['cnt'] for f in self.source.getFeatures(request)]
        self.assertEqual(values, [400, 300, 200, 100, -200])

        request = QgsFeatureRequest().addOrderBy('name')
        values = [f['name'] for f in self.source.getFeatures(request)]
        self.assertEqual(values, ['Apple', 'Honey', 'Orange', 'Pear', NULL])

        request = QgsFeatureRequest().addOrderBy('name', True, True)
        values = [f['name'] for f in self.source.getFeatures(request)]
        self.assertEqual(values, [NULL, 'Apple', 'Honey', 'Orange', 'Pear'])

        request = QgsFeatureRequest().addOrderBy('name', False)
        values = [f['name'] for f in self.source.getFeatures(request)]
        self.assertEqual(values, [NULL, 'Pear', 'Orange', 'Honey', 'Apple'])

        request = QgsFeatureRequest().addOrderBy('name', False, False)
        values = [f['name'] for f in self.source.getFeatures(request)]
        self.assertEqual(values, ['Pear', 'Orange', 'Honey', 'Apple', NULL])

        request = QgsFeatureRequest().addOrderBy('num_char', False)
        values = [f['pk'] for f in self.source.getFeatures(request)]
        self.assertEqual(values, [5, 4, 3, 2, 1])

        request = QgsFeatureRequest().addOrderBy('dt', False)
        values = [f['pk'] for f in self.source.getFeatures(request)]
        self.assertEqual(values, [3, 4, 2, 5, 1])

        request = QgsFeatureRequest().addOrderBy('date', False)
        values = [f['pk'] for f in self.source.getFeatures(request)]
        self.assertEqual(values, [3, 4, 2, 1, 5])

        request = QgsFeatureRequest().addOrderBy('time', False)
        values = [f['pk'] for f in self.source.getFeatures(request)]
        self.assertEqual(values, [3, 4, 2, 1, 5])

        # Case sensitivity
        request = QgsFeatureRequest().addOrderBy('name2')
        values = [f['name2'] for f in self.source.getFeatures(request)]
        self.assertEqual(values, ['Apple', 'Honey', 'NuLl', 'oranGe', 'PEaR'])

        # Combination with LIMIT
        request = QgsFeatureRequest().addOrderBy('pk', False).setLimit(2)
        values = [f['pk'] for f in self.source.getFeatures(request)]
        self.assertEqual(values, [5, 4])

        # A slightly more complex expression
        request = QgsFeatureRequest().addOrderBy('pk*2', False)
        values = [f['pk'] for f in self.source.getFeatures(request)]
        self.assertEqual(values, [5, 4, 3, 2, 1])

        # Order reversing expression
        request = QgsFeatureRequest().addOrderBy('pk*-1', False)
        values = [f['pk'] for f in self.source.getFeatures(request)]
        self.assertEqual(values, [1, 2, 3, 4, 5])

        # Type dependent expression
        request = QgsFeatureRequest().addOrderBy('num_char*2', False)
        values = [f['pk'] for f in self.source.getFeatures(request)]
        self.assertEqual(values, [5, 4, 3, 2, 1])

        # Order by guaranteed to fail
        request = QgsFeatureRequest().addOrderBy('not a valid expression*', False)
        values = [f['pk'] for f in self.source.getFeatures(request)]
        self.assertEqual(set(values), set([5, 4, 3, 2, 1]))

        # Multiple order bys and boolean
        request = QgsFeatureRequest().addOrderBy('pk > 2').addOrderBy('pk', False)
        values = [f['pk'] for f in self.source.getFeatures(request)]
        self.assertEqual(values, [2, 1, 5, 4, 3])

        # Multiple order bys, one bad, and a limit
        request = QgsFeatureRequest().addOrderBy('pk', False).addOrderBy('not a valid expression*', False).setLimit(2)
        values = [f['pk'] for f in self.source.getFeatures(request)]
        self.assertEqual(values, [5, 4])

        # Bad expression first
        request = QgsFeatureRequest().addOrderBy('not a valid expression*', False).addOrderBy('pk', False).setLimit(2)
        values = [f['pk'] for f in self.source.getFeatures(request)]
        self.assertEqual(values, [5, 4])

        # Combination with subset of attributes
        request = QgsFeatureRequest().addOrderBy('num_char', False).setSubsetOfAttributes(['pk'], self.source.fields())
        values = [f['pk'] for f in self.source.getFeatures(request)]
        self.assertEqual(values, [5, 4, 3, 2, 1])

    def testOrderBy(self):
        self.runOrderByTests()

    def testOpenIteratorAfterSourceRemoval(self):
        """
        Test that removing source after opening an iterator does not crash. All required
        information should be captured in the iterator's source and there MUST be no
        links between the iterators and the sources's data source
        """
        if not getattr(self, 'getSource', None):
            return

        source = self.getSource()
        it = source.getFeatures()
        del source

        # get the features
        pks = []
        for f in it:
            pks.append(f['pk'])
        self.assertEqual(set(pks), {1, 2, 3, 4, 5})

    def testGetFeaturesFidTests(self):
        fids = [f.id() for f in self.source.getFeatures()]
        assert len(fids) == 5, 'Expected 5 features, got {} instead'.format(len(fids))
        for id in fids:
            features = [f for f in self.source.getFeatures(QgsFeatureRequest().setFilterFid(id))]
            self.assertEqual(len(features), 1)
            feature = features[0]
            self.assertTrue(feature.isValid())

            result = [feature.id()]
            expected = [id]
            assert result == expected, 'Expected {} and got {} when testing for feature ID filter'.format(expected,
                                                                                                          result)

            # test that results match QgsFeatureRequest.acceptFeature
            request = QgsFeatureRequest().setFilterFid(id)
            for f in self.source.getFeatures():
                self.assertEqual(request.acceptFeature(f), f.id() == id)

        # bad features
        it = self.source.getFeatures(QgsFeatureRequest().setFilterFid(-99999999))
        feature = QgsFeature(5)
        feature.setValid(False)
        self.assertFalse(it.nextFeature(feature))
        self.assertFalse(feature.isValid())

    def testGetFeaturesFidsTests(self):
        fids = [f.id() for f in self.source.getFeatures()]
        self.assertEqual(len(fids), 5)

        # empty list = no features
        request = QgsFeatureRequest().setFilterFids([])
        result = set([f.id() for f in self.source.getFeatures(request)])
        self.assertFalse(result)

        request = QgsFeatureRequest().setFilterFids([fids[0], fids[2]])
        result = set([f.id() for f in self.source.getFeatures(request)])
        all_valid = (all(f.isValid() for f in self.source.getFeatures(request)))
        expected = set([fids[0], fids[2]])
        assert result == expected, 'Expected {} and got {} when testing for feature IDs filter'.format(expected, result)
        self.assertTrue(all_valid)

        # test that results match QgsFeatureRequest.acceptFeature
        for f in self.source.getFeatures():
            self.assertEqual(request.acceptFeature(f), f.id() in expected)

        result = set(
            [f.id() for f in self.source.getFeatures(QgsFeatureRequest().setFilterFids([fids[1], fids[3], fids[4]]))])
        expected = set([fids[1], fids[3], fids[4]])
        assert result == expected, 'Expected {} and got {} when testing for feature IDs filter'.format(expected, result)

        # sources should ignore non-existent fids
        result = set([f.id() for f in self.source.getFeatures(
            QgsFeatureRequest().setFilterFids([-101, fids[1], -102, fids[3], -103, fids[4], -104]))])
        expected = set([fids[1], fids[3], fids[4]])
        assert result == expected, 'Expected {} and got {} when testing for feature IDs filter'.format(expected, result)

        result = set([f.id() for f in self.source.getFeatures(QgsFeatureRequest().setFilterFids([]))])
        expected = set([])
        assert result == expected, 'Expected {} and got {} when testing for feature IDs filter'.format(expected, result)

        # Rewind mid-way
        request = QgsFeatureRequest().setFilterFids([fids[1], fids[3], fids[4]])
        feature_it = self.source.getFeatures(request)
        feature = QgsFeature()
        feature.setValid(True)
        self.assertTrue(feature_it.nextFeature(feature))
        self.assertIn(feature.id(), [fids[1], fids[3], fids[4]])
        first_feature = feature
        self.assertTrue(feature.isValid())
        # rewind
        self.assertTrue(feature_it.rewind())
        self.assertTrue(feature_it.nextFeature(feature))
        self.assertEqual(feature.id(), first_feature.id())
        self.assertTrue(feature.isValid())
        # grab all features
        self.assertTrue(feature_it.nextFeature(feature))
        self.assertTrue(feature_it.nextFeature(feature))
        # none left
        self.assertFalse(feature_it.nextFeature(feature))
        self.assertFalse(feature.isValid())

    def testGetFeaturesFilterRectTests(self):
        extent = QgsRectangle(-70, 67, -60, 80)
        request = QgsFeatureRequest().setFilterRect(extent)
        features = [f['pk'] for f in self.source.getFeatures(request)]
        all_valid = (all(f.isValid() for f in self.source.getFeatures(request)))
        assert set(features) == set([2, 4]), 'Got {} instead'.format(features)
        self.assertTrue(all_valid)

        # test that results match QgsFeatureRequest.acceptFeature
        for f in self.source.getFeatures():
            self.assertEqual(request.acceptFeature(f), f['pk'] in set([2, 4]))

        # test with an empty rectangle
        extent = QgsRectangle()
        request = QgsFeatureRequest().setFilterRect(extent)
        features = [f['pk'] for f in self.source.getFeatures(request)]
        all_valid = (all(f.isValid() for f in self.source.getFeatures(request)))
        assert set(features) == set([1, 2, 3, 4, 5]), 'Got {} instead'.format(features)
        self.assertTrue(all_valid)

        # ExactIntersection flag set, but no filter rect set. Should be ignored.
        request = QgsFeatureRequest()
        request.setFlags(QgsFeatureRequest.ExactIntersect)
        features = [f['pk'] for f in self.source.getFeatures(request)]
        all_valid = (all(f.isValid() for f in self.source.getFeatures(request)))
        assert set(features) == set([1, 2, 3, 4, 5]), 'Got {} instead'.format(features)
        self.assertTrue(all_valid)

    def testRectAndExpression(self):
        extent = QgsRectangle(-70, 67, -60, 80)
        request = QgsFeatureRequest().setFilterExpression('"cnt">200').setFilterRect(extent)
        result = set([f['pk'] for f in self.source.getFeatures(request)])
        all_valid = (all(f.isValid() for f in self.source.getFeatures(request)))
        expected = [4]
        assert set(
            expected) == result, 'Expected {} and got {} when testing for combination of filterRect and expression'.format(
            set(expected), result)
        self.assertTrue(all_valid)

        # shouldn't matter what order this is done in
        request = QgsFeatureRequest().setFilterRect(extent).setFilterExpression('"cnt">200')
        result = set([f['pk'] for f in self.source.getFeatures(request)])
        all_valid = (all(f.isValid() for f in self.source.getFeatures(request)))
        expected = [4]
        assert set(
            expected) == result, 'Expected {} and got {} when testing for combination of filterRect and expression'.format(
            set(expected), result)
        self.assertTrue(all_valid)

        # test that results match QgsFeatureRequest.acceptFeature
        for f in self.source.getFeatures():
            self.assertEqual(request.acceptFeature(f), f['pk'] in expected)

    def testGetFeaturesDistanceWithinTests(self):
        request = QgsFeatureRequest().setDistanceWithin(QgsGeometry.fromWkt('LineString (-63.2 69.9, -68.47 69.86, -69.74 79.28)'), 1.7)
        features = [f['pk'] for f in self.source.getFeatures(request)]
        all_valid = (all(f.isValid() for f in self.source.getFeatures(request)))
        assert set(features) == set([2, 5]), 'Got {} instead'.format(features)
        self.assertTrue(all_valid)

        # test that results match QgsFeatureRequest.acceptFeature
        for f in self.source.getFeatures():
            self.assertEqual(request.acceptFeature(f), f['pk'] in set([2, 5]))

        request = QgsFeatureRequest().setDistanceWithin(QgsGeometry.fromWkt('LineString (-63.2 69.9, -68.47 69.86, -69.74 79.28)'), 0.6)
        features = [f['pk'] for f in self.source.getFeatures(request)]
        all_valid = (all(f.isValid() for f in self.source.getFeatures(request)))
        assert set(features) == set([2]), 'Got {} instead'.format(features)
        self.assertTrue(all_valid)

        # test that results match QgsFeatureRequest.acceptFeature
        for f in self.source.getFeatures():
            self.assertEqual(request.acceptFeature(f), f['pk'] in set([2]))

        # in different crs
        request = QgsFeatureRequest().setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:3857'), QgsProject.instance().transformContext()).setDistanceWithin(QgsGeometry.fromWkt('LineString (-7035391 11036245, -7622045 11023301, -7763421 15092839)'), 250000)
        features = [f['pk'] for f in self.source.getFeatures(request)]
        all_valid = (all(f.isValid() for f in self.source.getFeatures(request)))
        self.assertEqual(set(features), {2, 5})
        self.assertTrue(all_valid)

        # point geometry
        request = QgsFeatureRequest().setDistanceWithin(
            QgsGeometry.fromWkt('Point (-68.1 78.1)'), 3.6)
        features = [f['pk'] for f in self.source.getFeatures(request)]
        all_valid = (all(f.isValid() for f in self.source.getFeatures(request)))
        assert set(features) == set([4, 5]), 'Got {} instead'.format(features)
        self.assertTrue(all_valid)

        # test that results match QgsFeatureRequest.acceptFeature
        for f in self.source.getFeatures():
            self.assertEqual(request.acceptFeature(f), f['pk'] in set([4, 5]))

        request = QgsFeatureRequest().setDistanceWithin(
            QgsGeometry.fromWkt('Polygon ((-64.47 79.59, -64.37 73.59, -72.69 73.61, -72.73 68.07, -62.51 68.01, -62.71 79.55, -64.47 79.59))'), 0)
        features = [f['pk'] for f in self.source.getFeatures(request)]
        all_valid = (all(f.isValid() for f in self.source.getFeatures(request)))
        assert set(features) == set([2]), 'Got {} instead'.format(features)
        self.assertTrue(all_valid)

        # test that results match QgsFeatureRequest.acceptFeature
        for f in self.source.getFeatures():
            self.assertEqual(request.acceptFeature(f), f['pk'] in set([2]))

        request = QgsFeatureRequest().setDistanceWithin(
            QgsGeometry.fromWkt('Polygon ((-64.47 79.59, -64.37 73.59, -72.69 73.61, -72.73 68.07, -62.51 68.01, -62.71 79.55, -64.47 79.59))'), 1.3)
        features = [f['pk'] for f in self.source.getFeatures(request)]
        all_valid = (all(f.isValid() for f in self.source.getFeatures(request)))
        assert set(features) == set([2, 4]), 'Got {} instead'.format(features)
        self.assertTrue(all_valid)

        # test that results match QgsFeatureRequest.acceptFeature
        for f in self.source.getFeatures():
            self.assertEqual(request.acceptFeature(f), f['pk'] in set([2, 4]))

        request = QgsFeatureRequest().setDistanceWithin(
            QgsGeometry.fromWkt('Polygon ((-64.47 79.59, -64.37 73.59, -72.69 73.61, -72.73 68.07, -62.51 68.01, -62.71 79.55, -64.47 79.59))'), 2.3)
        features = [f['pk'] for f in self.source.getFeatures(request)]
        all_valid = (all(f.isValid() for f in self.source.getFeatures(request)))
        assert set(features) == set([1, 2, 4]), 'Got {} instead'.format(features)
        self.assertTrue(all_valid)

        # test that results match QgsFeatureRequest.acceptFeature
        for f in self.source.getFeatures():
            self.assertEqual(request.acceptFeature(f), f['pk'] in set([1, 2, 4]))

        # test with linestring whose bounding box overlaps all query
        # points but being only within one of them, which we hope will
        # be returned NOT as the first one.
        # This is a test for https://github.com/qgis/QGIS/issues/45352
        request = QgsFeatureRequest().setDistanceWithin(
            QgsGeometry.fromWkt('LINESTRING(-100 80, -100 66, -30 66, -30 80)'), 0.5)
        features = {f['pk'] for f in self.source.getFeatures(request)}
        self.assertEqual(features, {1}, "Unexpected return from QgsFeatureRequest with DistanceWithin filter")

    def testGeomAndAllAttributes(self):
        """
        Test combination of a filter which requires geometry and all attributes
        """
        request = QgsFeatureRequest().setFilterExpression(
            'attribute($currentfeature,\'cnt\')>200 and $x>=-70 and $x<=-60').setSubsetOfAttributes([]).setFlags(
            QgsFeatureRequest.NoGeometry | QgsFeatureRequest.IgnoreStaticNodesDuringExpressionCompilation)
        result = set([f['pk'] for f in self.source.getFeatures(request)])
        all_valid = (all(f.isValid() for f in self.source.getFeatures(request)))
        self.assertEqual(result, {4})
        self.assertTrue(all_valid)

        request = QgsFeatureRequest().setFilterExpression(
            'attribute($currentfeature,\'cnt\')>200 and $x>=-70 and $x<=-60').setFlags(QgsFeatureRequest.IgnoreStaticNodesDuringExpressionCompilation)
        result = set([f['pk'] for f in self.source.getFeatures(request)])
        all_valid = (all(f.isValid() for f in self.source.getFeatures(request)))
        self.assertEqual(result, {4})
        self.assertTrue(all_valid)

    def testRectAndFids(self):
        """
        Test the combination of a filter rect along with filterfids
        """

        # first get feature ids
        ids = {f['pk']: f.id() for f in self.source.getFeatures()}

        extent = QgsRectangle(-70, 67, -60, 80)
        request = QgsFeatureRequest().setFilterFids([ids[3], ids[4]]).setFilterRect(extent)
        result = set([f['pk'] for f in self.source.getFeatures(request)])
        all_valid = (all(f.isValid() for f in self.source.getFeatures(request)))
        expected = [4]
        assert set(
            expected) == result, 'Expected {} and got {} when testing for combination of filterRect and expression'.format(
            set(expected), result)
        self.assertTrue(all_valid)

        # shouldn't matter what order this is done in
        request = QgsFeatureRequest().setFilterRect(extent).setFilterFids([ids[3], ids[4]])
        result = set([f['pk'] for f in self.source.getFeatures(request)])
        all_valid = (all(f.isValid() for f in self.source.getFeatures(request)))
        expected = [4]
        assert set(
            expected) == result, 'Expected {} and got {} when testing for combination of filterRect and expression'.format(
            set(expected), result)
        self.assertTrue(all_valid)

        # test that results match QgsFeatureRequest.acceptFeature
        for f in self.source.getFeatures():
            self.assertEqual(request.acceptFeature(f), f['pk'] in expected)

    def testGetFeaturesDestinationCrs(self):
        request = QgsFeatureRequest().setDestinationCrs(QgsCoordinateReferenceSystem('epsg:3785'),
                                                        QgsProject.instance().transformContext())
        features = {f['pk']: f for f in self.source.getFeatures(request)}
        # test that features have been reprojected
        self.assertAlmostEqual(features[1].geometry().constGet().x(), -7829322, -5)
        self.assertAlmostEqual(features[1].geometry().constGet().y(), 9967753, -5)
        self.assertAlmostEqual(features[2].geometry().constGet().x(), -7591989, -5)
        self.assertAlmostEqual(features[2].geometry().constGet().y(), 11334232, -5)
        self.assertFalse(features[3].hasGeometry())
        self.assertAlmostEqual(features[4].geometry().constGet().x(), -7271389, -5)
        self.assertAlmostEqual(features[4].geometry().constGet().y(), 14531322, -5)
        self.assertAlmostEqual(features[5].geometry().constGet().x(), -7917376, -5)
        self.assertAlmostEqual(features[5].geometry().constGet().y(), 14493008, -5)

        # when destination crs is set, filter rect should be in destination crs
        rect = QgsRectangle(-7650000, 10500000, -7200000, 15000000)
        request = QgsFeatureRequest().setDestinationCrs(QgsCoordinateReferenceSystem('epsg:3785'),
                                                        QgsProject.instance().transformContext()).setFilterRect(rect)
        features = {f['pk']: f for f in self.source.getFeatures(request)}
        self.assertEqual(set(features.keys()), {2, 4})
        # test that features have been reprojected
        self.assertAlmostEqual(features[2].geometry().constGet().x(), -7591989, -5)
        self.assertAlmostEqual(features[2].geometry().constGet().y(), 11334232, -5)
        self.assertAlmostEqual(features[4].geometry().constGet().x(), -7271389, -5)
        self.assertAlmostEqual(features[4].geometry().constGet().y(), 14531322, -5)

        # bad rect for transform
        rect = QgsRectangle(-99999999999, 99999999999, -99999999998, 99999999998)
        request = QgsFeatureRequest().setDestinationCrs(QgsCoordinateReferenceSystem('epsg:28356'),
                                                        QgsProject.instance().transformContext()).setFilterRect(rect)
        features = [f for f in self.source.getFeatures(request)]
        self.assertFalse(features)

    def testGetFeaturesLimit(self):
        it = self.source.getFeatures(QgsFeatureRequest().setLimit(2))
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
        it = self.source.getFeatures(QgsFeatureRequest().setLimit(2).setFilterExpression('cnt <= 100'))
        features = [f['pk'] for f in it]
        assert set(features) == set([1, 5]), 'Expected [1,5] for expression and feature limit, Got {} instead'.format(
            features)
        try:
            self.enableCompiler()
        except AttributeError:
            pass
        it = self.source.getFeatures(QgsFeatureRequest().setLimit(2).setFilterExpression('cnt <= 100'))
        features = [f['pk'] for f in it]
        assert set(features) == set([1, 5]), 'Expected [1,5] for expression and feature limit, Got {} instead'.format(
            features)
        # limit to more features than exist
        it = self.source.getFeatures(QgsFeatureRequest().setLimit(3).setFilterExpression('cnt <= 100'))
        features = [f['pk'] for f in it]
        assert set(features) == set([1, 5]), 'Expected [1,5] for expression and feature limit, Got {} instead'.format(
            features)
        # limit to less features than possible
        it = self.source.getFeatures(QgsFeatureRequest().setLimit(1).setFilterExpression('cnt <= 100'))
        features = [f['pk'] for f in it]
        assert 1 in features or 5 in features, 'Expected either 1 or 5 for expression and feature limit, Got {} instead'.format(
            features)

    def testClosedIterators(self):
        """ Test behavior of closed iterators """

        # Test retrieving feature after closing iterator
        f_it = self.source.getFeatures(QgsFeatureRequest())
        fet = QgsFeature()
        assert f_it.nextFeature(fet), 'Could not fetch feature'
        assert fet.isValid(), 'Feature is not valid'
        assert f_it.close(), 'Could not close iterator'
        self.assertFalse(f_it.nextFeature(fet),
                         'Fetched feature after iterator closed, expected nextFeature() to return False')
        self.assertFalse(fet.isValid(), 'Valid feature fetched from closed iterator, should be invalid')

        # Test rewinding closed iterator
        self.assertFalse(f_it.rewind(), 'Rewinding closed iterator successful, should not be allowed')

    def testGetFeaturesSubsetAttributes(self):
        """ Test that expected results are returned when using subsets of attributes """

        tests = {'pk': set([1, 2, 3, 4, 5]),
                 'cnt': set([-200, 300, 100, 200, 400]),
                 'name': set(['Pear', 'Orange', 'Apple', 'Honey', NULL]),
                 'name2': set(['NuLl', 'PEaR', 'oranGe', 'Apple', 'Honey']),
                 'dt': set([NULL, '2021-05-04 13:13:14' if self.treat_datetime_as_string() else QDateTime(2021, 5, 4, 13, 13, 14) if not self.treat_datetime_as_string() else '2021-05-04 13:13:14',
                            '2020-05-04 12:14:14' if self.treat_datetime_as_string() else QDateTime(2020, 5, 4, 12, 14, 14) if not self.treat_datetime_as_string() else '2020-05-04 12:14:14',
                            '2020-05-04 12:13:14' if self.treat_datetime_as_string() else QDateTime(2020, 5, 4, 12, 13, 14) if not self.treat_datetime_as_string() else '2020-05-04 12:13:14',
                            '2020-05-03 12:13:14' if self.treat_datetime_as_string() else QDateTime(2020, 5, 3, 12, 13, 14) if not self.treat_datetime_as_string() else '2020-05-03 12:13:14']),
                 'date': set([NULL,
                              '2020-05-02' if self.treat_date_as_string() else QDate(2020, 5, 2) if not self.treat_date_as_datetime() else QDateTime(2020, 5, 2, 0, 0, 0),
                              '2020-05-03' if self.treat_date_as_string() else QDate(2020, 5, 3) if not self.treat_date_as_datetime() else QDateTime(2020, 5, 3, 0, 0, 0),
                              '2020-05-04' if self.treat_date_as_string() else QDate(2020, 5, 4) if not self.treat_date_as_datetime() else QDateTime(2020, 5, 4, 0, 0, 0),
                              '2021-05-04' if self.treat_date_as_string() else QDate(2021, 5, 4) if not self.treat_date_as_datetime() else QDateTime(2021, 5, 4, 0, 0, 0)]),
                 'time': set([QTime(12, 13, 1) if not self.treat_time_as_string() else '12:13:01',
                              QTime(12, 14, 14) if not self.treat_time_as_string() else '12:14:14',
                              QTime(12, 13, 14) if not self.treat_time_as_string() else '12:13:14',
                              QTime(13, 13, 14) if not self.treat_time_as_string() else '13:13:14', NULL])}
        for field, expected in list(tests.items()):
            request = QgsFeatureRequest().setSubsetOfAttributes([field], self.source.fields())
            result = set([f[field] for f in self.source.getFeatures(request)])
            all_valid = (all(f.isValid() for f in self.source.getFeatures(request)))
            self.assertEqual(result, expected, 'Expected {}, got {}'.format(expected, result))
            self.assertTrue(all_valid)

    def testGetFeaturesSubsetAttributes2(self):
        """ Test that other fields are NULL when fetching subsets of attributes """

        for field_to_fetch in ['pk', 'cnt', 'name', 'name2', 'dt', 'date', 'time']:
            for f in self.source.getFeatures(
                    QgsFeatureRequest().setSubsetOfAttributes([field_to_fetch], self.source.fields())):
                # Check that all other fields are NULL and force name to lower-case
                for other_field in [field.name() for field in self.source.fields() if
                                    field.name().lower() != field_to_fetch]:
                    if other_field == 'pk' or other_field == 'PK':
                        # skip checking the primary key field, as it may be validly fetched by providers to use as feature id
                        continue
                    self.assertEqual(f[other_field], NULL,
                                     'Value for field "{}" was present when it should not have been fetched by request'.format(
                                         other_field))

    def testGetFeaturesNoGeometry(self):
        """ Test that no geometry is present when fetching features without geometry"""

        for f in self.source.getFeatures(QgsFeatureRequest().setFlags(QgsFeatureRequest.NoGeometry)):
            self.assertFalse(f.hasGeometry(), 'Expected no geometry, got one')
            self.assertTrue(f.isValid())

    def testGetFeaturesWithGeometry(self):
        """ Test that geometry is present when fetching features without setting NoGeometry flag"""
        for f in self.source.getFeatures(QgsFeatureRequest()):
            if f['pk'] == 3:
                # no geometry for this feature
                continue

            assert f.hasGeometry(), 'Expected geometry, got none'
            self.assertTrue(f.isValid())

    def testUniqueValues(self):
        self.assertEqual(set(self.source.uniqueValues(self.source.fields().lookupField('cnt'))),
                         set([-200, 100, 200, 300, 400]))
        assert set(['Apple', 'Honey', 'Orange', 'Pear', NULL]) == set(
            self.source.uniqueValues(self.source.fields().lookupField('name'))), 'Got {}'.format(
            set(self.source.uniqueValues(self.source.fields().lookupField('name'))))

        if self.treat_datetime_as_string():
            self.assertEqual(set(self.source.uniqueValues(self.source.fields().lookupField('dt'))),
                             set(['2021-05-04 13:13:14', '2020-05-04 12:14:14', '2020-05-04 12:13:14', '2020-05-03 12:13:14', NULL]))
        else:
            self.assertEqual(set(self.source.uniqueValues(self.source.fields().lookupField('dt'))),
                             set([QDateTime(2021, 5, 4, 13, 13, 14), QDateTime(2020, 5, 4, 12, 14, 14), QDateTime(2020, 5, 4, 12, 13, 14), QDateTime(2020, 5, 3, 12, 13, 14), NULL]))

        if self.treat_date_as_string():
            self.assertEqual(set(self.source.uniqueValues(self.source.fields().lookupField('date'))),
                             set(['2020-05-03', '2020-05-04', '2021-05-04', '2020-05-02', NULL]))
        elif self.treat_date_as_datetime():
            self.assertEqual(set(self.source.uniqueValues(self.source.fields().lookupField('date'))),
                             set([QDateTime(2020, 5, 3, 0, 0, 0), QDateTime(2020, 5, 4, 0, 0, 0), QDateTime(2021, 5, 4, 0, 0, 0), QDateTime(2020, 5, 2, 0, 0, 0), NULL]))
        else:
            self.assertEqual(set(self.source.uniqueValues(self.source.fields().lookupField('date'))),
                             set([QDate(2020, 5, 3), QDate(2020, 5, 4), QDate(2021, 5, 4), QDate(2020, 5, 2), NULL]))
        if self.treat_time_as_string():
            self.assertEqual(set(self.source.uniqueValues(self.source.fields().lookupField('time'))),
                             set(['12:14:14', '13:13:14', '12:13:14', '12:13:01', NULL]))
        else:
            self.assertEqual(set(self.source.uniqueValues(self.source.fields().lookupField('time'))),
                             set([QTime(12, 14, 14), QTime(13, 13, 14), QTime(12, 13, 14), QTime(12, 13, 1), NULL]))

    def testMinimumValue(self):
        self.assertEqual(self.source.minimumValue(self.source.fields().lookupField('cnt')), -200)
        self.assertEqual(self.source.minimumValue(self.source.fields().lookupField('name')), 'Apple')

        if self.treat_datetime_as_string():
            self.assertEqual(self.source.minimumValue(self.source.fields().lookupField('dt')), '2020-05-03 12:13:14')
        else:
            self.assertEqual(self.source.minimumValue(self.source.fields().lookupField('dt')), QDateTime(QDate(2020, 5, 3), QTime(12, 13, 14)))

        if self.treat_date_as_string():
            self.assertEqual(self.source.minimumValue(self.source.fields().lookupField('date')), '2020-05-02')
        elif not self.treat_date_as_datetime():
            self.assertEqual(self.source.minimumValue(self.source.fields().lookupField('date')), QDate(2020, 5, 2))
        else:
            self.assertEqual(self.source.minimumValue(self.source.fields().lookupField('date')), QDateTime(2020, 5, 2, 0, 0, 0))

        if not self.treat_time_as_string():
            self.assertEqual(self.source.minimumValue(self.source.fields().lookupField('time')), QTime(12, 13, 1))
        else:
            self.assertEqual(self.source.minimumValue(self.source.fields().lookupField('time')), '12:13:01')

    def testMaximumValue(self):
        self.assertEqual(self.source.maximumValue(self.source.fields().lookupField('cnt')), 400)
        self.assertEqual(self.source.maximumValue(self.source.fields().lookupField('name')), 'Pear')

        if not self.treat_datetime_as_string():
            self.assertEqual(self.source.maximumValue(self.source.fields().lookupField('dt')), QDateTime(QDate(2021, 5, 4), QTime(13, 13, 14)))
        else:
            self.assertEqual(self.source.maximumValue(self.source.fields().lookupField('dt')), '2021-05-04 13:13:14')

        if self.treat_date_as_string():
            self.assertEqual(self.source.maximumValue(self.source.fields().lookupField('date')), '2021-05-04')
        elif not self.treat_date_as_datetime():
            self.assertEqual(self.source.maximumValue(self.source.fields().lookupField('date')), QDate(2021, 5, 4))
        else:
            self.assertEqual(self.source.maximumValue(self.source.fields().lookupField('date')), QDateTime(2021, 5, 4, 0, 0, 0))

        if not self.treat_time_as_string():
            self.assertEqual(self.source.maximumValue(self.source.fields().lookupField('time')), QTime(13, 13, 14))
        else:
            self.assertEqual(self.source.maximumValue(self.source.fields().lookupField('time')), '13:13:14')

    def testAllFeatureIds(self):
        ids = set([f.id() for f in self.source.getFeatures()])
        self.assertEqual(set(self.source.allFeatureIds()), ids)

    def testSubsetOfAttributesWithFilterExprWithNonExistingColumn(self):
        """ Test fix for https://github.com/qgis/QGIS/issues/33878 """
        request = QgsFeatureRequest().setSubsetOfAttributes([0])
        request.setFilterExpression("non_existing = 1")
        features = [f for f in self.source.getFeatures(request)]
        self.assertEqual(len(features), 0)
