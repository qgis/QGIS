# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsVectorLayerTemporalProperties

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '73/05/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import qgis  # NOQA

from qgis.core import (QgsDateTimeRange,
                       QgsVectorLayerTemporalProperties,
                       QgsReadWriteContext,
                       QgsVectorLayer,
                       QgsVectorDataProviderTemporalCapabilities,
                       QgsUnitTypes,
                       QgsVectorLayerTemporalContext,
                       QgsFeature,
                       QgsGeometry,
                       QgsPointXY,
                       QgsFeatureRequest)
from qgis.PyQt.QtCore import (QDateTime,
                              QDate,
                              QTime,
                              QVariant)
from qgis.testing import start_app, unittest
from qgis.PyQt.QtXml import QDomDocument

app = start_app()


class TestQgsVectorLayerTemporalProperties(unittest.TestCase):

    def testReadWrite(self):
        props = QgsVectorLayerTemporalProperties()
        props.setMode(QgsVectorLayerTemporalProperties.ModeFeatureDateTimeInstantFromField)
        props.setFixedTemporalRange(QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10))))
        props.setStartField('start')
        props.setEndField('end')
        props.setDurationField('duration')
        props.setDurationUnits(QgsUnitTypes.TemporalWeeks)
        props.setFixedDuration(5.6)
        props.setAccumulateFeatures(True)
        props.setStartExpression('start exp')
        props.setEndExpression('end exp')

        # save to xml
        doc = QDomDocument("testdoc")
        elem = doc.createElement('test')
        elem = props.writeXml(elem, doc, QgsReadWriteContext())

        # restore from xml
        props2 = QgsVectorLayerTemporalProperties()
        self.assertTrue(props2.readXml(elem, QgsReadWriteContext()))

        self.assertEqual(props2.mode(), props.mode())
        self.assertEqual(props2.fixedTemporalRange(), props.fixedTemporalRange())
        self.assertEqual(props2.startField(), props.startField())
        self.assertEqual(props2.endField(), props.endField())
        self.assertEqual(props2.durationField(), props.durationField())
        self.assertEqual(props2.durationUnits(), props.durationUnits())
        self.assertEqual(props2.fixedDuration(), props.fixedDuration())
        self.assertEqual(props2.accumulateFeatures(), props.accumulateFeatures())
        self.assertEqual(props2.startExpression(), props.startExpression())
        self.assertEqual(props2.endExpression(), props.endExpression())

    def testModeFromProvider(self):
        caps = QgsVectorDataProviderTemporalCapabilities()
        props = QgsVectorLayerTemporalProperties()
        props.setDefaultsFromDataProviderTemporalCapabilities(caps)
        self.assertFalse(props.isActive())

        caps.setHasTemporalCapabilities(True)
        caps.setAvailableTemporalRange(QgsDateTimeRange(QDateTime(2006, 3, 11, 0, 13, 20), QDateTime(2017, 2, 14, 1, 33, 20)))
        props.setDefaultsFromDataProviderTemporalCapabilities(caps)
        self.assertTrue(props.isActive())
        self.assertFalse(props.startField())
        self.assertFalse(props.endField())
        self.assertEqual(props.mode(), QgsVectorLayerTemporalProperties.ModeFixedTemporalRange)
        self.assertEqual(props.fixedTemporalRange().begin(), QDateTime(2006, 3, 11, 0, 13, 20))
        self.assertEqual(props.fixedTemporalRange().end(), QDateTime(2017, 2, 14, 1, 33, 20))

        caps.setStartField('start_field')
        caps.setMode(QgsVectorDataProviderTemporalCapabilities.ProviderStoresFeatureDateTimeInstantInField)
        props.setDefaultsFromDataProviderTemporalCapabilities(caps)
        self.assertTrue(props.isActive())
        self.assertEqual(props.startField(), 'start_field')
        self.assertFalse(props.endField())
        self.assertEqual(props.mode(), QgsVectorLayerTemporalProperties.ModeFeatureDateTimeInstantFromField)

        caps.setEndField('end_field')
        caps.setMode(QgsVectorDataProviderTemporalCapabilities.ProviderStoresFeatureDateTimeStartAndEndInSeparateFields)
        props.setDefaultsFromDataProviderTemporalCapabilities(caps)
        self.assertTrue(props.isActive())
        self.assertEqual(props.startField(), 'start_field')
        self.assertEqual(props.endField(), 'end_field')
        self.assertEqual(props.mode(), QgsVectorLayerTemporalProperties.ModeFeatureDateTimeStartAndEndFromFields)

    def testGuessDefaultsFromFields(self):
        layer = QgsVectorLayer("Point?field=start_date:string&field=end_date:integer", "test", "memory")
        self.assertTrue(layer.isValid())
        # no date/datetime fields, should not guess anything
        props = layer.temporalProperties()
        self.assertFalse(props.isActive())
        self.assertFalse(props.startField())
        self.assertFalse(props.endField())

        # datetime fields, but not expected names
        layer = QgsVectorLayer("Point?field=date_created:date&field=date_modified:date", "test", "memory")
        self.assertTrue(layer.isValid())
        props = layer.temporalProperties()
        self.assertFalse(props.isActive())
        self.assertFalse(props.startField())
        self.assertFalse(props.endField())

        # sample table with likely single field
        layer = QgsVectorLayer("Point?field=event_id:integer&field=event_date:date", "test", "memory")
        self.assertTrue(layer.isValid())
        props = layer.temporalProperties()
        self.assertFalse(props.isActive())
        self.assertEqual(props.startField(), 'event_date')
        self.assertFalse(props.endField())
        self.assertEqual(props.mode(), QgsVectorLayerTemporalProperties.ModeFeatureDateTimeInstantFromField)

        # sample table with likely dual fields
        layer = QgsVectorLayer("Point?field=event_id:integer&field=start_date:datetime&field=end_date:datetime", "test", "memory")
        self.assertTrue(layer.isValid())
        props = layer.temporalProperties()
        self.assertFalse(props.isActive())
        self.assertEqual(props.startField(), 'start_date')
        self.assertEqual(props.endField(), 'end_date')
        self.assertEqual(props.mode(), QgsVectorLayerTemporalProperties.ModeFeatureDateTimeStartAndEndFromFields)

    def testFixedRangeMode(self):
        props = QgsVectorLayerTemporalProperties(enabled=True)
        props.setMode(QgsVectorLayerTemporalProperties.ModeFixedTemporalRange)
        props.setFixedTemporalRange(QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10))))

        self.assertTrue(props.isVisibleInTemporalRange(QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)))))
        self.assertTrue(props.isVisibleInTemporalRange(QgsDateTimeRange(QDateTime(QDate(2019, 1, 4), QTime(11, 12, 13)), QDateTime(QDate(2019, 5, 6), QTime(8, 9, 10)))))
        self.assertTrue(props.isVisibleInTemporalRange(QgsDateTimeRange(QDateTime(QDate(2020, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2019, 9, 6), QTime(8, 9, 10)))))
        self.assertFalse(props.isVisibleInTemporalRange(QgsDateTimeRange(QDateTime(QDate(2120, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2121, 9, 6), QTime(8, 9, 10)))))
        self.assertFalse(props.isVisibleInTemporalRange(QgsDateTimeRange(QDateTime(QDate(1920, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(1921, 9, 6), QTime(8, 9, 10)))))

        layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer&field=start_field:datetime", "test", "memory")
        context = QgsVectorLayerTemporalContext()
        context.setLayer(layer)

        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)))
        # ALWAYS must be empty for ModeFixedTemporalRange
        self.assertFalse(props.createFilterString(context, range))

    def testSingleFieldMode(self):
        layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer&field=start_field:datetime", "test", "memory")
        self.assertTrue(layer.isValid())
        self.assertEqual(layer.fields()[2].type(), QVariant.DateTime)
        context = QgsVectorLayerTemporalContext()
        context.setLayer(layer)

        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)))

        props = QgsVectorLayerTemporalProperties(enabled=False)
        props.setMode(QgsVectorLayerTemporalProperties.ModeFeatureDateTimeInstantFromField)
        props.setStartField('start_field')
        self.assertFalse(props.createFilterString(context, range))

        props.setIsActive(True)
        self.assertEqual(props.createFilterString(context, range), '("start_field" >= make_datetime(2019,3,4,11,12,13) AND "start_field" < make_datetime(2020,5,6,8,9,10)) OR "start_field" IS NULL')

        # RD: strange tests below? if you go over data with a only one datetime field, you should always include beginning (==default in code)
        # if you do NOT do this, you will miss data which are ON the beginning
        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeBeginning=False)
        self.assertEqual(props.createFilterString(context, range), '("start_field" >= make_datetime(2019,3,4,11,12,13) AND "start_field" < make_datetime(2020,5,6,8,9,10)) OR "start_field" IS NULL')

        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeEnd=False)
        self.assertEqual(props.createFilterString(context, range), '("start_field" >= make_datetime(2019,3,4,11,12,13) AND "start_field" < make_datetime(2020,5,6,8,9,10)) OR "start_field" IS NULL')

        # with fixed duration
        props.setFixedDuration(3)
        props.setDurationUnits(QgsUnitTypes.TemporalDays)
        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeBeginning=False, includeEnd=False)
        self.assertEqual(props.createFilterString(context, range), '("start_field" > make_datetime(2019,3,1,11,12,13) AND "start_field" < make_datetime(2020,5,6,8,9,10)) OR "start_field" IS NULL')

        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeBeginning=False)
        self.assertEqual(props.createFilterString(context, range), '("start_field" > make_datetime(2019,3,1,11,12,13) AND "start_field" < make_datetime(2020,5,6,8,9,10)) OR "start_field" IS NULL')

        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeEnd=False)
        self.assertEqual(props.createFilterString(context, range), '("start_field" > make_datetime(2019,3,1,11,12,13) AND "start_field" < make_datetime(2020,5,6,8,9,10)) OR "start_field" IS NULL')

        props.setDurationUnits(QgsUnitTypes.TemporalMinutes)
        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)))
        self.assertEqual(props.createFilterString(context, range), '("start_field" > make_datetime(2019,3,4,11,9,13) AND "start_field" < make_datetime(2020,5,6,8,9,10)) OR "start_field" IS NULL')

        # accumulate mode
        props.setAccumulateFeatures(True)
        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)))
        self.assertEqual(props.createFilterString(context, range), '("start_field" < make_datetime(2020,5,6,8,9,10)) OR "start_field" IS NULL')

        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeBeginning=False)
        self.assertEqual(props.createFilterString(context, range), '("start_field" < make_datetime(2020,5,6,8,9,10)) OR "start_field" IS NULL')

        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeEnd=False)
        self.assertEqual(props.createFilterString(context, range), '("start_field" < make_datetime(2020,5,6,8,9,10)) OR "start_field" IS NULL')

    def testDualFieldMode(self):
        layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer&field=start_field:datetime&field=end_field:datetime", "test", "memory")
        self.assertTrue(layer.isValid())
        self.assertEqual(layer.fields()[2].type(), QVariant.DateTime)
        self.assertEqual(layer.fields()[3].type(), QVariant.DateTime)
        context = QgsVectorLayerTemporalContext()
        context.setLayer(layer)

        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeBeginning=False, includeEnd=False)

        props = QgsVectorLayerTemporalProperties(enabled=False)
        props.setMode(QgsVectorLayerTemporalProperties.ModeFeatureDateTimeStartAndEndFromFields)
        props.setStartField('start_field')
        props.setEndField('end_field')
        self.assertFalse(props.createFilterString(context, range))

        props.setIsActive(True)
        self.assertEqual(props.createFilterString(context, range), '("start_field" < make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL) AND ("end_field" > make_datetime(2019,3,4,11,12,13) OR "end_field" IS NULL)')

        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeBeginning=False)
        self.assertEqual(props.createFilterString(context, range), '("start_field" < make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL) AND ("end_field" > make_datetime(2019,3,4,11,12,13) OR "end_field" IS NULL)')

        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeEnd=False)
        self.assertEqual(props.createFilterString(context, range), '("start_field" < make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL) AND ("end_field" > make_datetime(2019,3,4,11,12,13) OR "end_field" IS NULL)')

        props.setEndField('')
        self.assertEqual(props.createFilterString(context, range), '"start_field" < make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL')

        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeBeginning=False)
        self.assertEqual(props.createFilterString(context, range), '"start_field" < make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL')

        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeEnd=False)
        self.assertEqual(props.createFilterString(context, range), '"start_field" < make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL')

        props.setStartField('')
        props.setEndField('end_field')
        self.assertEqual(props.createFilterString(context, range), '"end_field" > make_datetime(2019,3,4,11,12,13) OR "end_field" IS NULL')

        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeBeginning=False)
        self.assertEqual(props.createFilterString(context, range), '"end_field" > make_datetime(2019,3,4,11,12,13) OR "end_field" IS NULL')

        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeEnd=False)
        self.assertEqual(props.createFilterString(context, range), '"end_field" > make_datetime(2019,3,4,11,12,13) OR "end_field" IS NULL')

    def testStartAndDurationMode(self):
        layer = QgsVectorLayer(
            "Point?field=fldtxt:string&field=fldint:integer&field=start_field:datetime&field=duration:double",
            "test", "memory")
        self.assertTrue(layer.isValid())
        self.assertEqual(layer.fields()[2].type(), QVariant.DateTime)
        self.assertEqual(layer.fields()[3].type(), QVariant.Double)
        context = QgsVectorLayerTemporalContext()
        context.setLayer(layer)

        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)),
                                 QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)))

        props = QgsVectorLayerTemporalProperties(enabled=False)
        props.setMode(QgsVectorLayerTemporalProperties.ModeFeatureDateTimeStartAndDurationFromFields)
        props.setStartField('start_field')
        props.setDurationField('duration')
        props.setDurationUnits(QgsUnitTypes.TemporalMilliseconds)
        self.assertFalse(props.createFilterString(context, range))

        props.setIsActive(True)
        self.assertEqual(props.createFilterString(context, range),
                         '("start_field" < make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL) AND (("start_field" + make_interval(0,0,0,0,0,0,"duration"/1000) > make_datetime(2019,3,4,11,12,13)) OR "duration" IS NULL)')

        props.setDurationUnits(QgsUnitTypes.TemporalSeconds)
        self.assertEqual(props.createFilterString(context, range),
                         '("start_field" < make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL) AND (("start_field" + make_interval(0,0,0,0,0,0,"duration") > make_datetime(2019,3,4,11,12,13)) OR "duration" IS NULL)')

        props.setDurationUnits(QgsUnitTypes.TemporalMinutes)
        self.assertEqual(props.createFilterString(context, range),
                         '("start_field" < make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL) AND (("start_field" + make_interval(0,0,0,0,0,"duration",0) > make_datetime(2019,3,4,11,12,13)) OR "duration" IS NULL)')

        props.setDurationUnits(QgsUnitTypes.TemporalHours)
        self.assertEqual(props.createFilterString(context, range),
                         '("start_field" < make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL) AND (("start_field" + make_interval(0,0,0,0,"duration",0,0) > make_datetime(2019,3,4,11,12,13)) OR "duration" IS NULL)')

        props.setDurationUnits(QgsUnitTypes.TemporalDays)
        self.assertEqual(props.createFilterString(context, range),
                         '("start_field" < make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL) AND (("start_field" + make_interval(0,0,0,"duration",0,0,0) > make_datetime(2019,3,4,11,12,13)) OR "duration" IS NULL)')

        props.setDurationUnits(QgsUnitTypes.TemporalWeeks)
        self.assertEqual(props.createFilterString(context, range),
                         '("start_field" < make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL) AND (("start_field" + make_interval(0,0,"duration",0,0,0,0) > make_datetime(2019,3,4,11,12,13)) OR "duration" IS NULL)')

        props.setDurationUnits(QgsUnitTypes.TemporalMonths)
        self.assertEqual(props.createFilterString(context, range),
                         '("start_field" < make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL) AND (("start_field" + make_interval(0,"duration",0,0,0,0,0) > make_datetime(2019,3,4,11,12,13)) OR "duration" IS NULL)')

        props.setDurationUnits(QgsUnitTypes.TemporalYears)
        self.assertEqual(props.createFilterString(context, range),
                         '("start_field" < make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL) AND (("start_field" + make_interval("duration",0,0,0,0,0,0) > make_datetime(2019,3,4,11,12,13)) OR "duration" IS NULL)')

        props.setDurationUnits(QgsUnitTypes.TemporalDecades)
        self.assertEqual(props.createFilterString(context, range),
                         '("start_field" < make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL) AND (("start_field" + make_interval(10 * "duration",0,0,0,0,0,0) > make_datetime(2019,3,4,11,12,13)) OR "duration" IS NULL)')

        props.setDurationUnits(QgsUnitTypes.TemporalCenturies)
        self.assertEqual(props.createFilterString(context, range),
                         '("start_field" < make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL) AND (("start_field" + make_interval(100 * "duration",0,0,0,0,0,0) > make_datetime(2019,3,4,11,12,13)) OR "duration" IS NULL)')

    def testExpressionMode(self):
        layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer&field=start_field:datetime&field=end_field:datetime", "test", "memory")
        self.assertTrue(layer.isValid())
        self.assertEqual(layer.fields()[2].type(), QVariant.DateTime)
        self.assertEqual(layer.fields()[3].type(), QVariant.DateTime)
        context = QgsVectorLayerTemporalContext()
        context.setLayer(layer)

        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)))

        props = QgsVectorLayerTemporalProperties(enabled=False)
        props.setMode(QgsVectorLayerTemporalProperties.ModeFeatureDateTimeStartAndEndFromExpressions)
        props.setStartExpression('to_datetime("my string field",  \'yyyy MM dd hh::mm:ss\')"')
        props.setEndExpression('to_datetime("my end field",  \'yyyy MM dd hh::mm:ss\')"')
        self.assertFalse(props.createFilterString(context, range))

        props.setIsActive(True)
        self.assertEqual(props.createFilterString(context, range), '((to_datetime("my string field",  \'yyyy MM dd hh::mm:ss\')") < make_datetime(2020,5,6,8,9,10)) AND ((to_datetime("my end field",  \'yyyy MM dd hh::mm:ss\')") > make_datetime(2019,3,4,11,12,13))')

        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeBeginning=False)
        self.assertEqual(props.createFilterString(context, range), '((to_datetime("my string field",  \'yyyy MM dd hh::mm:ss\')") < make_datetime(2020,5,6,8,9,10)) AND ((to_datetime("my end field",  \'yyyy MM dd hh::mm:ss\')") > make_datetime(2019,3,4,11,12,13))')

        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeEnd=False)
        self.assertEqual(props.createFilterString(context, range), '((to_datetime("my string field",  \'yyyy MM dd hh::mm:ss\')") < make_datetime(2020,5,6,8,9,10)) AND ((to_datetime("my end field",  \'yyyy MM dd hh::mm:ss\')") > make_datetime(2019,3,4,11,12,13))')

        props.setEndExpression('')
        self.assertEqual(props.createFilterString(context, range), '(to_datetime("my string field",  \'yyyy MM dd hh::mm:ss\')") < make_datetime(2020,5,6,8,9,10)')

        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeBeginning=False)
        self.assertEqual(props.createFilterString(context, range), '(to_datetime("my string field",  \'yyyy MM dd hh::mm:ss\')") < make_datetime(2020,5,6,8,9,10)')

        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeEnd=False)
        self.assertEqual(props.createFilterString(context, range), '(to_datetime("my string field",  \'yyyy MM dd hh::mm:ss\')") < make_datetime(2020,5,6,8,9,10)')

        props.setStartExpression('')
        props.setEndExpression('to_datetime("my end field",  \'yyyy MM dd hh::mm:ss\')"')
        self.assertEqual(props.createFilterString(context, range), '(to_datetime("my end field",  \'yyyy MM dd hh::mm:ss\')") > make_datetime(2019,3,4,11,12,13)')

        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeBeginning=False)
        self.assertEqual(props.createFilterString(context, range), '(to_datetime("my end field",  \'yyyy MM dd hh::mm:ss\')") > make_datetime(2019,3,4,11,12,13)')

        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeEnd=False)
        self.assertEqual(props.createFilterString(context, range), '(to_datetime("my end field",  \'yyyy MM dd hh::mm:ss\')") > make_datetime(2019,3,4,11,12,13)')

    def testFeatureTemporalFilterOutput(self):
        # some background on the 'overlapping' code: https://wiki.c2.com/?TestIfDateRangesOverlap
        # idea is to test:
        #    - a phenemenon with a time-range (so having  a start time and an end time (OR duration))
        #    - a phenemenon with only one attribute/timestamp (in time)
        # both when the overlap with the timestep-frame (is ALWAYS a timeframe, though sometimes very short: 1 millisecond)
        # for both sets test:
        #    - when phenemenon timeframe is exactly on timestep frame (should be always be just 1 result)
        #    - when phenomenon timeframe and timestep frame are overlapping (should have 2 results)
        #    - when both do NOT overlap
        layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer&field=start_field:datetime&field=end_field:datetime&field=text:string", "test", "memory")
        self.assertTrue(layer.isValid())
        self.assertEqual(layer.fields()[2].type(), QVariant.DateTime)
        self.assertEqual(layer.fields()[3].type(), QVariant.DateTime)
        # creating (point) features on a row with a one-hour duration phenomenon
        # in a csv use:
        #
        #    X,Y,fid,id,t_start,t_end,text
        #    5.02435455597371,52.6540035154413,"1","1",2020/08/26 01:00:00,2020/08/26 02:00:00,"1-2"
        #    5.0646388842879,52.6544602897524,"2","2",2020/08/26 02:00:00,2020/08/26 03:00:00,"2-3"
        #    5.11922979648937,52.6542319031934,"3","3",2020/08/26 03:00:00,2020/08/26 04:00:00,"3-4"
        #    5.16930284009487,52.6546886751184,"4","4",,2020/08/26 05:00:00,"NULL-5"
        #    5.22314077419701,52.6551454422712,"5","5",2020/08/26 05:00:00,,"5-NULL"
        #    5.27584924115016,52.6558305840525,"6","6",2020/08/26 06:00:00,2020/08/26 07:00:00,"6-7"

        # fldint and fldtxt hold the START of the temporal props (not used)
        # fv=feature values
        # X,Y,fldint,fldtxt,start_field,end_field,text
        fv1 = [5.02435455597371, 52.6540035154413, 1, "1", QDateTime(QDate(2020, 8, 26), QTime(1, 0, 0)), QDateTime(QDate(2020, 8, 26), QTime(2, 0, 0)), "1-2"]
        fv2 = [5.06463888428790, 52.6544602897524, 2, "2", QDateTime(QDate(2020, 8, 26), QTime(2, 0, 0)), QDateTime(QDate(2020, 8, 26), QTime(3, 0, 0)), "2-3"]
        fv3 = [5.11922979648937, 52.6542319031934, 3, "3", QDateTime(QDate(2020, 8, 26), QTime(3, 0, 0)), QDateTime(QDate(2020, 8, 26), QTime(4, 0, 0)), "3-4"]
        layer.startEditing()
        for fv in [fv1, fv2, fv3]:
            f = QgsFeature()
            f.setAttributes(fv[2:])
            f.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(fv[0], fv[1])))
            self.assertTrue(f.isValid())
            layer.addFeature(f)
        layer.commitChanges()
        self.assertEqual(len(list(layer.dataProvider().getFeatures())), 3)

        context = QgsVectorLayerTemporalContext()
        context.setLayer(layer)
        props = QgsVectorLayerTemporalProperties(enabled=True)
        props.setMode(QgsVectorLayerTemporalProperties.ModeFeatureDateTimeStartAndEndFromFields)
        props.setStartField('start_field')
        props.setEndField('end_field')

        # frame OUTSIDE data range, should return zero features
        frame = QgsDateTimeRange(QDateTime(QDate(2020, 8, 25), QTime(0, 0, 0)), QDateTime(QDate(2020, 8, 25), QTime(1, 0, 0)), False, False)
        featureRequest = QgsFeatureRequest().combineFilterExpression(props.createFilterString(context, frame))
        result = list(layer.dataProvider().getFeatures(featureRequest))
        self.assertEqual(len(result), 0)

        frame = QgsDateTimeRange(QDateTime(QDate(2020, 8, 26), QTime(2, 0, 0)), QDateTime(QDate(2020, 8, 26), QTime(3, 0, 0)), False, False)
        # timestep frame on exact one data range frame: 2:00-3:00
        # should return only the feature with text attribute '2-3'
        featureRequest = QgsFeatureRequest().combineFilterExpression(props.createFilterString(context, frame))
        result = list(layer.dataProvider().getFeatures(featureRequest))
        self.assertEqual(len(result), 1)
        self.assertEqual(result[0]['text'], '2-3')

        # layer definition end_field value is NULL
        # should return all features which were started before (?not including?) the endfield, that is why 2 features (start of third one does not count)
        props.setEndField(None)
        featureRequest = QgsFeatureRequest().combineFilterExpression(props.createFilterString(context, frame))
        result = list(layer.dataProvider().getFeatures(featureRequest))
        self.assertEqual(len(result), 2)

        # layer definition start_field value is NULL
        # should return all features which
        props.setEndField('end_field')  # RESET end_field
        props.setStartField(None)
        featureRequest = QgsFeatureRequest().combineFilterExpression(props.createFilterString(context, frame))
        result = list(layer.dataProvider().getFeatures(featureRequest))
        self.assertEqual(len(result), 2)

        # now setting to one field an fixed duration
        props.setMode(QgsVectorLayerTemporalProperties.ModeFeatureDateTimeInstantFromField)
        props.setStartField('start_field')
        props.setFixedDuration(1)
        props.setDurationUnits(QgsUnitTypes.TemporalHours)
        # frame is still 2-3
        featureRequest = QgsFeatureRequest().combineFilterExpression(props.createFilterString(context, frame))
        result = list(layer.dataProvider().getFeatures(featureRequest))
        self.assertEqual(len(result), 1)
        self.assertEqual(result[0]['text'], '2-3')

        # set duration to zero: a momentarily event
        props.setFixedDuration(0)
        self.assertEqual(len(result), 1)  # NO duration, should show in 2-3 but NOT on 3-4
        self.assertEqual(result[0]['text'], '2-3')
        frame = QgsDateTimeRange(QDateTime(QDate(2020, 8, 26), QTime(3, 0, 0)), QDateTime(QDate(2020, 8, 26), QTime(4, 0, 0)), False, False)
        featureRequest = QgsFeatureRequest().combineFilterExpression(props.createFilterString(context, frame))
        result = list(layer.dataProvider().getFeatures(featureRequest))
        self.assertEqual(len(result), 1)
        self.assertEqual(result[0]['text'], '3-4')

        # now add features which MISS either the start or then end VALUE
        fv4 = [5.16930284009487, 52.6546886751184, 4, "4", None, QDateTime(QDate(2020, 8, 26), QTime(5, 0, 0)), "NULL-5"]
        fv5 = [5.22314077419701, 52.6551454422712, 5, "5", QDateTime(QDate(2020, 8, 26), QTime(5, 0, 0)), None, "5-NULL"]
        fv6 = [5.27584924115016, 52.6558305840525, 6, "6", QDateTime(QDate(2020, 8, 26), QTime(6, 0, 0)), QDateTime(QDate(2020, 8, 26), QTime(7, 0, 0)), "6-7"]
        layer.startEditing()
        layer.startEditing()
        for fv in [fv4, fv5, fv6]:
            f = QgsFeature()
            f.setAttributes(fv[2:])
            f.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(fv[0], fv[1])))
            self.assertTrue(f.isValid())
            layer.addFeature(f)
        layer.commitChanges()
        self.assertEqual(len(list(layer.dataProvider().getFeatures())), 6)
        # REsetting the props to having both a start- and end-field
        props.setMode(QgsVectorLayerTemporalProperties.ModeFeatureDateTimeStartAndEndFromFields)
        props.setStartField('start_field')
        props.setEndField('end_field')
        frame = QgsDateTimeRange(QDateTime(QDate(2020, 8, 26), QTime(2, 0, 0)), QDateTime(QDate(2020, 8, 26), QTime(3, 0, 0)), False, False)

        # looking at timestep 2-3: should now show '2-3' AND the feature which only has an end value of 5 'NULL-5'
        featureRequest = QgsFeatureRequest().combineFilterExpression(props.createFilterString(context, frame))
        result = list(layer.dataProvider().getFeatures(featureRequest))
        self.assertEqual(len(result), 2)
        self.assertEqual(list(result)[0]['text'], '2-3')
        self.assertEqual(list(result)[1]['text'], 'NULL-5')

        # changing timestep to 6-7: should show both '6-7' AND feature starting at 5 till infinity '5-NULL'
        frame = QgsDateTimeRange(QDateTime(QDate(2020, 8, 26), QTime(6, 0, 0)), QDateTime(QDate(2020, 8, 26), QTime(7, 0, 0)), False, False)
        featureRequest = QgsFeatureRequest().combineFilterExpression(props.createFilterString(context, frame))
        result = list(layer.dataProvider().getFeatures(featureRequest))
        self.assertEqual(len(result), 2)
        self.assertEqual(list(result)[0]['text'], '5-NULL')
        self.assertEqual(list(result)[1]['text'], '6-7')


if __name__ == '__main__':
    unittest.main()
