# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsAggregateCalculator.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '16/05/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.core import (QgsAggregateCalculator,
                       QgsVectorLayer,
                       QgsFeature,
                       QgsVectorDataProvider,
                       QgsInterval,
                       QgsExpressionContext,
                       QgsExpressionContextScope,
                       NULL
                       )
from qgis.PyQt.QtCore import QDateTime, QDate, QTime
from qgis.testing import unittest, start_app

start_app()


class TestQgsAggregateCalculator(unittest.TestCase):

    def testLayer(self):
        """ Test setting/retrieving layer """
        a = QgsAggregateCalculator(None)
        self.assertEqual(a.layer(), None)

        # should not crash
        val, ok = a.calculate(QgsAggregateCalculator.Sum, 'field')
        self.assertFalse(ok)

        layer = QgsVectorLayer("Point?field=fldint:integer&field=flddbl:double",
                               "layer", "memory")
        a = QgsAggregateCalculator(layer)
        self.assertEqual(a.layer(), layer)

    def testParameters(self):
        """ Test setting parameters"""
        a = QgsAggregateCalculator(None)
        params = QgsAggregateCalculator.AggregateParameters()
        params.filter = 'string filter'
        params.delimiter = 'delim'
        a.setParameters(params)
        self.assertEqual(a.filter(), 'string filter')
        self.assertEqual(a.delimiter(), 'delim')

    def testNumeric(self):
        """ Test calculation of aggregates on numeric fields"""

        layer = QgsVectorLayer("Point?field=fldint:integer&field=flddbl:double",
                               "layer", "memory")
        pr = layer.dataProvider()

        # must be same length:
        int_values = [4, 2, 3, 2, 5, None, 8]
        dbl_values = [5.5, 3.5, 7.5, 5, 9, None, 7]
        self.assertEqual(len(int_values), len(dbl_values))

        features = []
        for i in range(len(int_values)):
            f = QgsFeature()
            f.setFields(layer.fields())
            f.setAttributes([int_values[i], dbl_values[i]])
            features.append(f)
        assert pr.addFeatures(features)

        tests = [[QgsAggregateCalculator.Count, 'fldint', 6],
                 [QgsAggregateCalculator.Count, 'flddbl', 6],
                 [QgsAggregateCalculator.Sum, 'fldint', 24],
                 [QgsAggregateCalculator.Sum, 'flddbl', 37.5],
                 [QgsAggregateCalculator.Mean, 'fldint', 4],
                 [QgsAggregateCalculator.Mean, 'flddbl', 6.25],
                 [QgsAggregateCalculator.StDev, 'fldint', 2.0816],
                 [QgsAggregateCalculator.StDev, 'flddbl', 1.7969],
                 [QgsAggregateCalculator.StDevSample, 'fldint', 2.2803],
                 [QgsAggregateCalculator.StDevSample, 'flddbl', 1.9685],
                 [QgsAggregateCalculator.Min, 'fldint', 2],
                 [QgsAggregateCalculator.Min, 'flddbl', 3.5],
                 [QgsAggregateCalculator.Max, 'fldint', 8],
                 [QgsAggregateCalculator.Max, 'flddbl', 9],
                 [QgsAggregateCalculator.Range, 'fldint', 6],
                 [QgsAggregateCalculator.Range, 'flddbl', 5.5],
                 [QgsAggregateCalculator.Median, 'fldint', 3.5],
                 [QgsAggregateCalculator.Median, 'flddbl', 6.25],
                 [QgsAggregateCalculator.CountDistinct, 'fldint', 5],
                 [QgsAggregateCalculator.CountDistinct, 'flddbl', 6],
                 [QgsAggregateCalculator.CountMissing, 'fldint', 1],
                 [QgsAggregateCalculator.CountMissing, 'flddbl', 1],
                 [QgsAggregateCalculator.FirstQuartile, 'fldint', 2],
                 [QgsAggregateCalculator.FirstQuartile, 'flddbl', 5.0],
                 [QgsAggregateCalculator.ThirdQuartile, 'fldint', 5.0],
                 [QgsAggregateCalculator.ThirdQuartile, 'flddbl', 7.5],
                 [QgsAggregateCalculator.InterQuartileRange, 'fldint', 3.0],
                 [QgsAggregateCalculator.InterQuartileRange, 'flddbl', 2.5],
                 ]

        agg = QgsAggregateCalculator(layer)
        for t in tests:
            val, ok = agg.calculate(t[0], t[1])
            self.assertTrue(ok)
            if isinstance(t[2], int):
                self.assertEqual(val, t[2])
            else:
                self.assertAlmostEqual(val, t[2], 3)

        # bad tests - the following stats should not be calculatable for numeric fields
        for t in [QgsAggregateCalculator.StringMinimumLength,
                  QgsAggregateCalculator.StringMaximumLength]:
            val, ok = agg.calculate(t, 'fldint')
            self.assertFalse(ok)
            val, ok = agg.calculate(t, 'flddbl')
            self.assertFalse(ok)

    def testString(self):
        """ Test calculation of aggregates on string fields"""

        layer = QgsVectorLayer("Point?field=fldstring:string", "layer", "memory")
        pr = layer.dataProvider()

        values = ['cc', 'aaaa', 'bbbbbbbb', 'aaaa', 'eeee', '', 'eeee', '', 'dddd']
        features = []
        for v in values:
            f = QgsFeature()
            f.setFields(layer.fields())
            f.setAttributes([v])
            features.append(f)
        assert pr.addFeatures(features)

        tests = [[QgsAggregateCalculator.Count, 'fldstring', 9],
                 [QgsAggregateCalculator.CountDistinct, 'fldstring', 6],
                 [QgsAggregateCalculator.CountMissing, 'fldstring', 2],
                 [QgsAggregateCalculator.Min, 'fldstring', 'aaaa'],
                 [QgsAggregateCalculator.Max, 'fldstring', 'eeee'],
                 [QgsAggregateCalculator.StringMinimumLength, 'fldstring', 0],
                 [QgsAggregateCalculator.StringMaximumLength, 'fldstring', 8],
                 ]

        agg = QgsAggregateCalculator(layer)
        for t in tests:
            val, ok = agg.calculate(t[0], t[1])
            self.assertTrue(ok)
            self.assertEqual(val, t[2])

        # test string concatenation
        agg.setDelimiter(',')
        self.assertEqual(agg.delimiter(), ',')
        val, ok = agg.calculate(QgsAggregateCalculator.StringConcatenate, 'fldstring')
        self.assertTrue(ok)
        self.assertEqual(val, 'cc,aaaa,bbbbbbbb,aaaa,eeee,,eeee,,dddd')

        # bad tests - the following stats should not be calculatable for string fields
        for t in [QgsAggregateCalculator.Sum,
                  QgsAggregateCalculator.Mean,
                  QgsAggregateCalculator.Median,
                  QgsAggregateCalculator.StDev,
                  QgsAggregateCalculator.StDevSample,
                  QgsAggregateCalculator.Range,
                  QgsAggregateCalculator.Minority,
                  QgsAggregateCalculator.Majority,
                  QgsAggregateCalculator.FirstQuartile,
                  QgsAggregateCalculator.ThirdQuartile,
                  QgsAggregateCalculator.InterQuartileRange
                  ]:
            val, ok = agg.calculate(t, 'fldstring')
            self.assertFalse(ok)

    def testDateTime(self):
        """ Test calculation of aggregates on date/datetime fields"""

        layer = QgsVectorLayer("Point?field=flddate:date&field=flddatetime:datetime", "layer", "memory")
        pr = layer.dataProvider()

        # must be same length:
        datetime_values = [QDateTime(QDate(2015, 3, 4), QTime(11, 10, 54)),
                           QDateTime(QDate(2011, 1, 5), QTime(15, 3, 1)),
                           QDateTime(QDate(2015, 3, 4), QTime(11, 10, 54)),
                           QDateTime(QDate(2015, 3, 4), QTime(11, 10, 54)),
                           QDateTime(QDate(2019, 12, 28), QTime(23, 10, 1)),
                           QDateTime(),
                           QDateTime(QDate(1998, 1, 2), QTime(1, 10, 54)),
                           QDateTime(),
                           QDateTime(QDate(2011, 1, 5), QTime(11, 10, 54))]
        date_values = [QDate(2015, 3, 4),
                       QDate(2015, 3, 4),
                       QDate(2019, 12, 28),
                       QDate(),
                       QDate(1998, 1, 2),
                       QDate(),
                       QDate(2011, 1, 5),
                       QDate(2011, 1, 5),
                       QDate(2011, 1, 5)]
        self.assertEqual(len(datetime_values), len(date_values))

        features = []
        for i in range(len(datetime_values)):
            f = QgsFeature()
            f.setFields(layer.fields())
            f.setAttributes([date_values[i], datetime_values[i]])
            features.append(f)
        assert pr.addFeatures(features)

        tests = [[QgsAggregateCalculator.Count, 'flddatetime', 9],
                 [QgsAggregateCalculator.Count, 'flddate', 9],
                 [QgsAggregateCalculator.CountDistinct, 'flddatetime', 6],
                 [QgsAggregateCalculator.CountDistinct, 'flddate', 5],
                 [QgsAggregateCalculator.CountMissing, 'flddatetime', 2],
                 [QgsAggregateCalculator.CountMissing, 'flddate', 2],
                 [QgsAggregateCalculator.Min, 'flddatetime', QDateTime(QDate(1998, 1, 2), QTime(1, 10, 54))],
                 [QgsAggregateCalculator.Min, 'flddate', QDateTime(QDate(1998, 1, 2), QTime(0, 0, 0))],
                 [QgsAggregateCalculator.Max, 'flddatetime', QDateTime(QDate(2019, 12, 28), QTime(23, 10, 1))],
                 [QgsAggregateCalculator.Max, 'flddate', QDateTime(QDate(2019, 12, 28), QTime(0, 0, 0))],

                 [QgsAggregateCalculator.Range, 'flddatetime', QgsInterval(693871147)],
                 [QgsAggregateCalculator.Range, 'flddate', QgsInterval(693792000)],

                 ]

        agg = QgsAggregateCalculator(layer)
        for t in tests:
            val, ok = agg.calculate(t[0], t[1])
            self.assertTrue(ok)
            self.assertEqual(val, t[2])

        # bad tests - the following stats should not be calculatable for string fields
        for t in [QgsAggregateCalculator.Sum,
                  QgsAggregateCalculator.Mean,
                  QgsAggregateCalculator.Median,
                  QgsAggregateCalculator.StDev,
                  QgsAggregateCalculator.StDevSample,
                  QgsAggregateCalculator.Minority,
                  QgsAggregateCalculator.Majority,
                  QgsAggregateCalculator.FirstQuartile,
                  QgsAggregateCalculator.ThirdQuartile,
                  QgsAggregateCalculator.InterQuartileRange,
                  QgsAggregateCalculator.StringMinimumLength,
                  QgsAggregateCalculator.StringMaximumLength,
                  ]:
            val, ok = agg.calculate(t, 'flddatetime')
            self.assertFalse(ok)

    def testFilter(self):
        """ test calculating aggregate with filter """

        layer = QgsVectorLayer("Point?field=fldint:integer", "layer", "memory")
        pr = layer.dataProvider()

        int_values = [4, 2, 3, 2, 5, None, 8]

        features = []
        for v in int_values:
            f = QgsFeature()
            f.setFields(layer.fields())
            f.setAttributes([v])
            features.append(f)
        assert pr.addFeatures(features)

        agg = QgsAggregateCalculator(layer)

        filter_string = "fldint > 2"
        agg.setFilter(filter_string)
        self.assertEqual(agg.filter(), filter_string)

        val, ok = agg.calculate(QgsAggregateCalculator.Sum, 'fldint')
        self.assertTrue(ok)
        self.assertEqual(val, 20)

        # remove filter and retest
        agg.setFilter(None)
        val, ok = agg.calculate(QgsAggregateCalculator.Sum, 'fldint')
        self.assertTrue(ok)
        self.assertEqual(val, 24)

    def testExpression(self):
        """ test aggregate calculation using an expression """

        # numeric
        layer = QgsVectorLayer("Point?field=fldint:integer", "layer", "memory")
        pr = layer.dataProvider()

        int_values = [4, 2, 3, 2, 5, None, 8]

        features = []
        for v in int_values:
            f = QgsFeature()
            f.setFields(layer.fields())
            f.setAttributes([v])
            features.append(f)
        assert pr.addFeatures(features)

        #int
        agg = QgsAggregateCalculator(layer)
        val, ok = agg.calculate(QgsAggregateCalculator.Sum, 'fldint * 2')
        self.assertTrue(ok)
        self.assertEqual(val, 48)

        # double
        val, ok = agg.calculate(QgsAggregateCalculator.Sum, 'fldint * 1.5')
        self.assertTrue(ok)
        self.assertEqual(val, 36)

        # datetime
        val, ok = agg.calculate(QgsAggregateCalculator.Max, "to_date('2012-05-04') + to_interval( fldint || ' day' )")
        self.assertTrue(ok)
        self.assertEqual(val, QDateTime(QDate(2012, 5, 12), QTime(0, 0, 0)))

        # date
        val, ok = agg.calculate(QgsAggregateCalculator.Min, "to_date(to_date('2012-05-04') + to_interval( fldint || ' day' ))")
        self.assertTrue(ok)
        self.assertEqual(val, QDateTime(QDate(2012, 5, 6), QTime(0, 0, 0)))

        # string
        val, ok = agg.calculate(QgsAggregateCalculator.Max, "fldint || ' oranges'")
        self.assertTrue(ok)
        self.assertEqual(val, '8 oranges')

        # try a bad expression
        val, ok = agg.calculate(QgsAggregateCalculator.Max, "not_a_field || ' oranges'")
        self.assertFalse(ok)
        val, ok = agg.calculate(QgsAggregateCalculator.Max, "5+")
        self.assertFalse(ok)

        # test expression context

        # check default context first
        # should have layer variables:
        val, ok = agg.calculate(QgsAggregateCalculator.Min, "@layer_name")
        self.assertTrue(ok)
        self.assertEqual(val, 'layer')
        # but not custom variables:
        val, ok = agg.calculate(QgsAggregateCalculator.Min, "@my_var")
        self.assertTrue(ok)
        self.assertEqual(val, NULL)

        # test with manual expression context
        scope = QgsExpressionContextScope()
        scope.setVariable('my_var', 5)
        context = QgsExpressionContext()
        context.appendScope(scope)
        val, ok = agg.calculate(QgsAggregateCalculator.Min, "@my_var", context)
        self.assertTrue(ok)
        self.assertEqual(val, 5)

    def testStringToAggregate(self):
        """ test converting strings to aggregate types """

        tests = [[QgsAggregateCalculator.Count, ' cOUnT '],
                 [QgsAggregateCalculator.CountDistinct, ' count_distinct   '],
                 [QgsAggregateCalculator.CountMissing, 'COUNT_MISSING'],
                 [QgsAggregateCalculator.Min, ' MiN'],
                 [QgsAggregateCalculator.Max, 'mAX'],
                 [QgsAggregateCalculator.Sum, 'sum'],
                 [QgsAggregateCalculator.Mean, 'MEAn  '],
                 [QgsAggregateCalculator.Median, 'median'],
                 [QgsAggregateCalculator.StDev, 'stdev'],
                 [QgsAggregateCalculator.StDevSample, 'stdevsample'],
                 [QgsAggregateCalculator.Range, 'range'],
                 [QgsAggregateCalculator.Minority, 'minority'],
                 [QgsAggregateCalculator.Majority, 'majority'],
                 [QgsAggregateCalculator.FirstQuartile, 'q1'],
                 [QgsAggregateCalculator.ThirdQuartile, 'q3'],
                 [QgsAggregateCalculator.InterQuartileRange, 'iqr'],
                 [QgsAggregateCalculator.StringMinimumLength, 'min_length'],
                 [QgsAggregateCalculator.StringMaximumLength, 'max_length'],
                 [QgsAggregateCalculator.StringConcatenate, 'concatenate']]

        for t in tests:
            agg, ok = QgsAggregateCalculator.stringToAggregate(t[1])
            self.assertTrue(ok)
            self.assertEqual(agg, t[0])

        # test some bad values
        agg, ok = QgsAggregateCalculator.stringToAggregate('')
        self.assertFalse(ok)
        agg, ok = QgsAggregateCalculator.stringToAggregate('bad')
        self.assertFalse(ok)


if __name__ == "__main__":
    unittest.main()
