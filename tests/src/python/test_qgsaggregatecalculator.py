"""QGIS Unit tests for QgsAggregateCalculator.

From build dir, run: ctest -R PyQgsAggregateCalculator -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "16/05/2016"
__copyright__ = "Copyright 2016, The QGIS Project"

from qgis.PyQt.QtCore import QDate, QDateTime, QTime
from qgis.core import (
    NULL,
    QgsAggregateCalculator,
    QgsExpressionContext,
    QgsExpressionContextScope,
    QgsFeature,
    QgsFeatureRequest,
    QgsGeometry,
    QgsInterval,
    QgsVectorLayer,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import compareWkt

start_app()


class TestQgsAggregateCalculator(QgisTestCase):

    def testLayer(self):
        """Test setting/retrieving layer"""
        a = QgsAggregateCalculator(None)
        self.assertEqual(a.layer(), None)

        # should not crash
        val, ok = a.calculate(QgsAggregateCalculator.Aggregate.Sum, "field")
        self.assertFalse(ok)

        layer = QgsVectorLayer(
            "Point?field=fldint:integer&field=flddbl:double", "layer", "memory"
        )
        a = QgsAggregateCalculator(layer)
        self.assertEqual(a.layer(), layer)

    def testParameters(self):
        """Test setting parameters"""
        a = QgsAggregateCalculator(None)
        params = QgsAggregateCalculator.AggregateParameters()
        params.filter = "string filter"
        params.delimiter = "delim"
        a.setParameters(params)
        self.assertEqual(a.filter(), "string filter")
        self.assertEqual(a.delimiter(), "delim")

    def testGeometry(self):
        """Test calculation of aggregates on geometry expressions"""

        layer = QgsVectorLayer("Point?", "layer", "memory")
        pr = layer.dataProvider()

        # must be same length:
        geometry_values = [
            QgsGeometry.fromWkt("Point ( 0 0 )"),
            QgsGeometry.fromWkt("Point ( 1 1 )"),
            QgsGeometry.fromWkt("Point ( 2 2 )"),
        ]

        features = []
        for i in range(len(geometry_values)):
            f = QgsFeature()
            f.setGeometry(geometry_values[i])
            features.append(f)
        self.assertTrue(pr.addFeatures(features))

        agg = QgsAggregateCalculator(layer)

        val, ok = agg.calculate(
            QgsAggregateCalculator.Aggregate.GeometryCollect, "$geometry"
        )
        self.assertTrue(ok)
        expwkt = "MultiPoint ((0 0), (1 1), (2 2))"
        wkt = val.asWkt()
        self.assertTrue(compareWkt(expwkt, wkt), f"Expected:\n{expwkt}\nGot:\n{wkt}\n")

    def testNumeric(self):
        """Test calculation of aggregates on numeric fields"""

        layer = QgsVectorLayer(
            "Point?field=fldint:integer&field=flddbl:double", "layer", "memory"
        )
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

        tests = [
            [QgsAggregateCalculator.Aggregate.Count, "fldint", 6],
            [QgsAggregateCalculator.Aggregate.Count, "flddbl", 6],
            [QgsAggregateCalculator.Aggregate.Sum, "fldint", 24],
            [QgsAggregateCalculator.Aggregate.Sum, "flddbl", 37.5],
            [QgsAggregateCalculator.Aggregate.Mean, "fldint", 4],
            [QgsAggregateCalculator.Aggregate.Mean, "flddbl", 6.25],
            [QgsAggregateCalculator.Aggregate.StDev, "fldint", 2.0816],
            [QgsAggregateCalculator.Aggregate.StDev, "flddbl", 1.7969],
            [QgsAggregateCalculator.Aggregate.StDevSample, "fldint", 2.2803],
            [QgsAggregateCalculator.Aggregate.StDevSample, "flddbl", 1.9685],
            [QgsAggregateCalculator.Aggregate.Min, "fldint", 2],
            [QgsAggregateCalculator.Aggregate.Min, "flddbl", 3.5],
            [QgsAggregateCalculator.Aggregate.Max, "fldint", 8],
            [QgsAggregateCalculator.Aggregate.Max, "flddbl", 9],
            [QgsAggregateCalculator.Aggregate.Range, "fldint", 6],
            [QgsAggregateCalculator.Aggregate.Range, "flddbl", 5.5],
            [QgsAggregateCalculator.Aggregate.Median, "fldint", 3.5],
            [QgsAggregateCalculator.Aggregate.Median, "flddbl", 6.25],
            [QgsAggregateCalculator.Aggregate.CountDistinct, "fldint", 5],
            [QgsAggregateCalculator.Aggregate.CountDistinct, "flddbl", 6],
            [QgsAggregateCalculator.Aggregate.CountMissing, "fldint", 1],
            [QgsAggregateCalculator.Aggregate.CountMissing, "flddbl", 1],
            [QgsAggregateCalculator.Aggregate.FirstQuartile, "fldint", 2],
            [QgsAggregateCalculator.Aggregate.FirstQuartile, "flddbl", 5.0],
            [QgsAggregateCalculator.Aggregate.ThirdQuartile, "fldint", 5.0],
            [QgsAggregateCalculator.Aggregate.ThirdQuartile, "flddbl", 7.5],
            [QgsAggregateCalculator.Aggregate.InterQuartileRange, "fldint", 3.0],
            [QgsAggregateCalculator.Aggregate.InterQuartileRange, "flddbl", 2.5],
            [QgsAggregateCalculator.Aggregate.ArrayAggregate, "fldint", int_values],
            [QgsAggregateCalculator.Aggregate.ArrayAggregate, "flddbl", dbl_values],
        ]

        agg = QgsAggregateCalculator(layer)
        for t in tests:
            val, ok = agg.calculate(t[0], t[1])
            self.assertTrue(ok)
            if isinstance(t[2], (int, list)):
                self.assertEqual(val, t[2])
            else:
                self.assertAlmostEqual(val, t[2], 3)

        # bad tests - the following stats should not be calculatable for numeric fields
        for t in [
            QgsAggregateCalculator.Aggregate.StringMinimumLength,
            QgsAggregateCalculator.Aggregate.StringMaximumLength,
        ]:
            val, ok = agg.calculate(t, "fldint")
            self.assertFalse(ok)
            val, ok = agg.calculate(t, "flddbl")
            self.assertFalse(ok)

        # with order by
        agg = QgsAggregateCalculator(layer)
        val, ok = agg.calculate(
            QgsAggregateCalculator.Aggregate.ArrayAggregate, "fldint"
        )
        self.assertEqual(val, [4, 2, 3, 2, 5, NULL, 8])
        params = QgsAggregateCalculator.AggregateParameters()
        params.orderBy = QgsFeatureRequest.OrderBy(
            [QgsFeatureRequest.OrderByClause("fldint")]
        )
        agg.setParameters(params)
        val, ok = agg.calculate(
            QgsAggregateCalculator.Aggregate.ArrayAggregate, "fldint"
        )
        self.assertEqual(val, [2, 2, 3, 4, 5, 8, NULL])
        params.orderBy = QgsFeatureRequest.OrderBy(
            [QgsFeatureRequest.OrderByClause("flddbl")]
        )
        agg.setParameters(params)
        val, ok = agg.calculate(
            QgsAggregateCalculator.Aggregate.ArrayAggregate, "fldint"
        )
        self.assertEqual(val, [2, 2, 4, 8, 3, 5, NULL])

    def testString(self):
        """Test calculation of aggregates on string fields"""

        layer = QgsVectorLayer("Point?field=fldstring:string", "layer", "memory")
        pr = layer.dataProvider()

        values = ["cc", "aaaa", "bbbbbbbb", "aaaa", "eeee", "", "eeee", "", "dddd"]
        features = []
        for v in values:
            f = QgsFeature()
            f.setFields(layer.fields())
            f.setAttributes([v])
            features.append(f)
        assert pr.addFeatures(features)

        tests = [
            [QgsAggregateCalculator.Aggregate.Count, "fldstring", 9],
            [QgsAggregateCalculator.Aggregate.CountDistinct, "fldstring", 6],
            [QgsAggregateCalculator.Aggregate.CountMissing, "fldstring", 2],
            [QgsAggregateCalculator.Aggregate.Min, "fldstring", "aaaa"],
            [QgsAggregateCalculator.Aggregate.Max, "fldstring", "eeee"],
            [QgsAggregateCalculator.Aggregate.StringMinimumLength, "fldstring", 0],
            [QgsAggregateCalculator.Aggregate.StringMaximumLength, "fldstring", 8],
            [QgsAggregateCalculator.Aggregate.ArrayAggregate, "fldstring", values],
        ]

        agg = QgsAggregateCalculator(layer)
        for t in tests:
            val, ok = agg.calculate(t[0], t[1])
            self.assertTrue(ok)
            self.assertEqual(val, t[2])

        # test string concatenation
        agg.setDelimiter(",")
        self.assertEqual(agg.delimiter(), ",")
        val, ok = agg.calculate(
            QgsAggregateCalculator.Aggregate.StringConcatenate, "fldstring"
        )
        self.assertTrue(ok)
        self.assertEqual(val, "cc,aaaa,bbbbbbbb,aaaa,eeee,,eeee,,dddd")
        val, ok = agg.calculate(
            QgsAggregateCalculator.Aggregate.StringConcatenateUnique, "fldstring"
        )
        self.assertTrue(ok)
        self.assertEqual(val, "cc,aaaa,bbbbbbbb,eeee,,dddd")

        # bad tests - the following stats should not be calculatable for string fields
        for t in [
            QgsAggregateCalculator.Aggregate.Sum,
            QgsAggregateCalculator.Aggregate.Mean,
            QgsAggregateCalculator.Aggregate.Median,
            QgsAggregateCalculator.Aggregate.StDev,
            QgsAggregateCalculator.Aggregate.StDevSample,
            QgsAggregateCalculator.Aggregate.Range,
            QgsAggregateCalculator.Aggregate.FirstQuartile,
            QgsAggregateCalculator.Aggregate.ThirdQuartile,
            QgsAggregateCalculator.Aggregate.InterQuartileRange,
        ]:
            val, ok = agg.calculate(t, "fldstring")
            self.assertFalse(ok)

        # with order by
        agg = QgsAggregateCalculator(layer)
        val, ok = agg.calculate(
            QgsAggregateCalculator.Aggregate.ArrayAggregate, "fldstring"
        )
        self.assertEqual(
            val, ["cc", "aaaa", "bbbbbbbb", "aaaa", "eeee", "", "eeee", "", "dddd"]
        )
        params = QgsAggregateCalculator.AggregateParameters()
        params.orderBy = QgsFeatureRequest.OrderBy(
            [QgsFeatureRequest.OrderByClause("fldstring")]
        )
        agg.setParameters(params)
        val, ok = agg.calculate(
            QgsAggregateCalculator.Aggregate.ArrayAggregate, "fldstring"
        )
        self.assertEqual(
            val, ["", "", "aaaa", "aaaa", "bbbbbbbb", "cc", "dddd", "eeee", "eeee"]
        )
        val, ok = agg.calculate(
            QgsAggregateCalculator.Aggregate.StringConcatenate, "fldstring"
        )
        self.assertEqual(val, "aaaaaaaabbbbbbbbccddddeeeeeeee")
        val, ok = agg.calculate(QgsAggregateCalculator.Aggregate.Minority, "fldstring")
        self.assertEqual(val, "bbbbbbbb")
        val, ok = agg.calculate(QgsAggregateCalculator.Aggregate.Majority, "fldstring")
        self.assertEqual(val, "")

    def testDateTime(self):
        """Test calculation of aggregates on date/datetime fields"""

        layer = QgsVectorLayer(
            "Point?field=flddate:date&field=flddatetime:datetime", "layer", "memory"
        )
        pr = layer.dataProvider()

        # must be same length:
        datetime_values = [
            QDateTime(QDate(2015, 3, 4), QTime(11, 10, 54)),
            QDateTime(QDate(2011, 1, 5), QTime(15, 3, 1)),
            QDateTime(QDate(2015, 3, 4), QTime(11, 10, 54)),
            QDateTime(QDate(2015, 3, 4), QTime(11, 10, 54)),
            QDateTime(QDate(2019, 12, 28), QTime(23, 10, 1)),
            QDateTime(),
            QDateTime(QDate(1998, 1, 2), QTime(1, 10, 54)),
            QDateTime(),
            QDateTime(QDate(2011, 1, 5), QTime(11, 10, 54)),
        ]
        date_values = [
            QDate(2015, 3, 4),
            QDate(2015, 3, 4),
            QDate(2019, 12, 28),
            QDate(),
            QDate(1998, 1, 2),
            QDate(),
            QDate(2011, 1, 5),
            QDate(2011, 1, 5),
            QDate(2011, 1, 5),
        ]
        self.assertEqual(len(datetime_values), len(date_values))

        features = []
        for i in range(len(datetime_values)):
            f = QgsFeature()
            f.setFields(layer.fields())
            f.setAttributes([date_values[i], datetime_values[i]])
            features.append(f)
        assert pr.addFeatures(features)

        tests = [
            [QgsAggregateCalculator.Aggregate.Count, "flddatetime", 9],
            [QgsAggregateCalculator.Aggregate.Count, "flddate", 9],
            [QgsAggregateCalculator.Aggregate.CountDistinct, "flddatetime", 6],
            [QgsAggregateCalculator.Aggregate.CountDistinct, "flddate", 5],
            [QgsAggregateCalculator.Aggregate.CountMissing, "flddatetime", 2],
            [QgsAggregateCalculator.Aggregate.CountMissing, "flddate", 2],
            [
                QgsAggregateCalculator.Aggregate.Min,
                "flddatetime",
                QDateTime(QDate(1998, 1, 2), QTime(1, 10, 54)),
            ],
            [
                QgsAggregateCalculator.Aggregate.Min,
                "flddate",
                QDateTime(QDate(1998, 1, 2), QTime(0, 0, 0)),
            ],
            [
                QgsAggregateCalculator.Aggregate.Max,
                "flddatetime",
                QDateTime(QDate(2019, 12, 28), QTime(23, 10, 1)),
            ],
            [
                QgsAggregateCalculator.Aggregate.Max,
                "flddate",
                QDateTime(QDate(2019, 12, 28), QTime(0, 0, 0)),
            ],
            [
                QgsAggregateCalculator.Aggregate.Range,
                "flddatetime",
                QgsInterval(693871147),
            ],
            [QgsAggregateCalculator.Aggregate.Range, "flddate", QgsInterval(693792000)],
            [
                QgsAggregateCalculator.Aggregate.ArrayAggregate,
                "flddatetime",
                [None if v.isNull() else v for v in datetime_values],
            ],
            [
                QgsAggregateCalculator.Aggregate.ArrayAggregate,
                "flddate",
                [None if v.isNull() else v for v in date_values],
            ],
        ]

        agg = QgsAggregateCalculator(layer)
        for t in tests:
            val, ok = agg.calculate(t[0], t[1])
            self.assertTrue(ok)
            self.assertEqual(val, t[2])

        # bad tests - the following stats should not be calculatable for string fields
        for t in [
            QgsAggregateCalculator.Aggregate.Sum,
            QgsAggregateCalculator.Aggregate.Mean,
            QgsAggregateCalculator.Aggregate.Median,
            QgsAggregateCalculator.Aggregate.StDev,
            QgsAggregateCalculator.Aggregate.StDevSample,
            QgsAggregateCalculator.Aggregate.Minority,
            QgsAggregateCalculator.Aggregate.Majority,
            QgsAggregateCalculator.Aggregate.FirstQuartile,
            QgsAggregateCalculator.Aggregate.ThirdQuartile,
            QgsAggregateCalculator.Aggregate.InterQuartileRange,
            QgsAggregateCalculator.Aggregate.StringMinimumLength,
            QgsAggregateCalculator.Aggregate.StringMaximumLength,
        ]:
            val, ok = agg.calculate(t, "flddatetime")
            self.assertFalse(ok)

    def testFilter(self):
        """test calculating aggregate with filter"""

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

        val, ok = agg.calculate(QgsAggregateCalculator.Aggregate.Sum, "fldint")
        self.assertTrue(ok)
        self.assertEqual(val, 20)

        # remove filter and retest
        agg.setFilter(None)
        val, ok = agg.calculate(QgsAggregateCalculator.Aggregate.Sum, "fldint")
        self.assertTrue(ok)
        self.assertEqual(val, 24)

    def testExpression(self):
        """test aggregate calculation using an expression"""

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

        # int
        agg = QgsAggregateCalculator(layer)
        val, ok = agg.calculate(QgsAggregateCalculator.Aggregate.Sum, "fldint * 2")
        self.assertTrue(ok)
        self.assertEqual(val, 48)

        # double
        val, ok = agg.calculate(QgsAggregateCalculator.Aggregate.Sum, "fldint * 1.5")
        self.assertTrue(ok)
        self.assertEqual(val, 36)

        # datetime
        val, ok = agg.calculate(
            QgsAggregateCalculator.Aggregate.Max,
            "to_date('2012-05-04') + to_interval( fldint || ' day' )",
        )
        self.assertTrue(ok)
        self.assertEqual(val, QDateTime(QDate(2012, 5, 12), QTime(0, 0, 0)))

        # date
        val, ok = agg.calculate(
            QgsAggregateCalculator.Aggregate.Min,
            "to_date(to_date('2012-05-04') + to_interval( fldint || ' day' ))",
        )
        self.assertTrue(ok)
        self.assertEqual(val, QDateTime(QDate(2012, 5, 6), QTime(0, 0, 0)))

        # string
        val, ok = agg.calculate(
            QgsAggregateCalculator.Aggregate.Max, "fldint || ' oranges'"
        )
        self.assertTrue(ok)
        self.assertEqual(val, "8 oranges")

        # geometry
        val, ok = agg.calculate(
            QgsAggregateCalculator.Aggregate.GeometryCollect,
            "make_point( coalesce(fldint,0), 2 )",
        )
        self.assertTrue(ok)
        self.assertTrue(val.asWkt(), "MultiPoint((4 2, 2 2, 3 2, 2 2,5 2, 0 2,8 2))")

        # try a bad expression
        val, ok = agg.calculate(
            QgsAggregateCalculator.Aggregate.Max, "not_a_field || ' oranges'"
        )
        self.assertFalse(ok)
        val, ok = agg.calculate(QgsAggregateCalculator.Aggregate.Max, "5+")
        self.assertFalse(ok)

        # test expression context

        # check default context first
        # should have layer variables:
        val, ok = agg.calculate(QgsAggregateCalculator.Aggregate.Min, "@layer_name")
        self.assertTrue(ok)
        self.assertEqual(val, "layer")
        # but not custom variables:
        val, ok = agg.calculate(QgsAggregateCalculator.Aggregate.Min, "@my_var")
        self.assertTrue(ok)
        self.assertEqual(val, NULL)

        # test with manual expression context
        scope = QgsExpressionContextScope()
        scope.setVariable("my_var", 5)
        context = QgsExpressionContext()
        context.appendScope(scope)
        val, ok = agg.calculate(
            QgsAggregateCalculator.Aggregate.Min, "@my_var", context
        )
        self.assertTrue(ok)
        self.assertEqual(val, 5)

        # test with subset
        agg = QgsAggregateCalculator(layer)  # reset to remove expression filter
        agg.setFidsFilter([1, 2])
        val, ok = agg.calculate(QgsAggregateCalculator.Aggregate.Sum, "fldint")
        self.assertTrue(ok)
        self.assertEqual(val, 6.0)

        # test with empty subset
        agg.setFidsFilter(list())
        val, ok = agg.calculate(QgsAggregateCalculator.Aggregate.Sum, "fldint")
        self.assertTrue(ok)
        self.assertEqual(val, 0.0)

    def testExpressionNullValuesAtStart(self):
        """test aggregate calculation using an expression which returns null values at first"""

        # numeric
        layer = QgsVectorLayer("Point?field=fldstr:string", "layer", "memory")
        pr = layer.dataProvider()

        values = [
            None,
            None,
            None,
            None,
            None,
            None,
            None,
            None,
            None,
            None,
            "2",
            "3",
            "5",
        ]

        features = []
        for v in values:
            f = QgsFeature()
            f.setFields(layer.fields())
            f.setAttributes([v])
            features.append(f)
        assert pr.addFeatures(features)

        # number aggregation
        agg = QgsAggregateCalculator(layer)
        val, ok = agg.calculate(QgsAggregateCalculator.Aggregate.Sum, "to_int(fldstr)")
        self.assertTrue(ok)
        self.assertEqual(val, 10)

        # string aggregation
        agg.setDelimiter(",")
        val, ok = agg.calculate(
            QgsAggregateCalculator.Aggregate.StringConcatenate, "fldstr || 'suffix'"
        )
        self.assertTrue(ok)
        self.assertEqual(val, ",,,,,,,,,,2suffix,3suffix,5suffix")

    def testExpressionNoMatch(self):
        """test aggregate calculation using an expression with no features"""

        # no features
        layer = QgsVectorLayer("Point?field=fldint:integer", "layer", "memory")

        # sum
        agg = QgsAggregateCalculator(layer)
        val, ok = agg.calculate(QgsAggregateCalculator.Aggregate.Sum, "fldint * 2")
        self.assertTrue(ok)
        self.assertEqual(val, None)

        # count
        agg = QgsAggregateCalculator(layer)
        val, ok = agg.calculate(QgsAggregateCalculator.Aggregate.Count, "fldint * 2")
        self.assertTrue(ok)
        self.assertEqual(val, 0)

        # count distinct
        agg = QgsAggregateCalculator(layer)
        val, ok = agg.calculate(
            QgsAggregateCalculator.Aggregate.CountDistinct, "fldint * 2"
        )
        self.assertTrue(ok)
        self.assertEqual(val, 0)

        # count missing
        agg = QgsAggregateCalculator(layer)
        val, ok = agg.calculate(
            QgsAggregateCalculator.Aggregate.CountMissing, "fldint * 2"
        )
        self.assertTrue(ok)
        self.assertEqual(val, 0)

        # min
        agg = QgsAggregateCalculator(layer)
        val, ok = agg.calculate(QgsAggregateCalculator.Aggregate.Min, "fldint * 2")
        self.assertTrue(ok)
        self.assertEqual(val, None)

        # max
        agg = QgsAggregateCalculator(layer)
        val, ok = agg.calculate(QgsAggregateCalculator.Aggregate.Max, "fldint * 2")
        self.assertTrue(ok)
        self.assertEqual(val, None)

        # array_agg
        agg = QgsAggregateCalculator(layer)
        val, ok = agg.calculate(
            QgsAggregateCalculator.Aggregate.ArrayAggregate, "fldint * 2"
        )
        self.assertTrue(ok)
        self.assertEqual(val, [])

    def testStringToAggregate(self):
        """test converting strings to aggregate types"""

        tests = [
            [QgsAggregateCalculator.Aggregate.Count, " cOUnT "],
            [QgsAggregateCalculator.Aggregate.CountDistinct, " count_distinct   "],
            [QgsAggregateCalculator.Aggregate.CountMissing, "COUNT_MISSING"],
            [QgsAggregateCalculator.Aggregate.Min, " MiN"],
            [QgsAggregateCalculator.Aggregate.Max, "mAX"],
            [QgsAggregateCalculator.Aggregate.Sum, "sum"],
            [QgsAggregateCalculator.Aggregate.Mean, "MEAn  "],
            [QgsAggregateCalculator.Aggregate.Median, "median"],
            [QgsAggregateCalculator.Aggregate.StDev, "stdev"],
            [QgsAggregateCalculator.Aggregate.StDevSample, "stdevsample"],
            [QgsAggregateCalculator.Aggregate.Range, "range"],
            [QgsAggregateCalculator.Aggregate.Minority, "minority"],
            [QgsAggregateCalculator.Aggregate.Majority, "majority"],
            [QgsAggregateCalculator.Aggregate.FirstQuartile, "q1"],
            [QgsAggregateCalculator.Aggregate.ThirdQuartile, "q3"],
            [QgsAggregateCalculator.Aggregate.InterQuartileRange, "iqr"],
            [QgsAggregateCalculator.Aggregate.StringMinimumLength, "min_length"],
            [QgsAggregateCalculator.Aggregate.StringMaximumLength, "max_length"],
            [QgsAggregateCalculator.Aggregate.StringConcatenate, "concatenate"],
            [
                QgsAggregateCalculator.Aggregate.StringConcatenateUnique,
                "concatenate_unique",
            ],
            [QgsAggregateCalculator.Aggregate.GeometryCollect, "collect"],
        ]

        for t in tests:
            agg, ok = QgsAggregateCalculator.stringToAggregate(t[1])
            self.assertTrue(ok)
            self.assertEqual(agg, t[0])

        # test some bad values
        agg, ok = QgsAggregateCalculator.stringToAggregate("")
        self.assertFalse(ok)
        agg, ok = QgsAggregateCalculator.stringToAggregate("bad")
        self.assertFalse(ok)


if __name__ == "__main__":
    unittest.main()
