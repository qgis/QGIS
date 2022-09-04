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
                       Qgis)
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
        # testing both a layer/context with Datetime field as one with Date (only) field, as the last one should be added a cast to datetime
        datetime_layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer&field=start_field:datetime", "test", "memory")
        self.assertTrue(datetime_layer.isValid())
        self.assertEqual(datetime_layer.fields()[2].type(), QVariant.DateTime)
        datetime_context = QgsVectorLayerTemporalContext()
        datetime_context.setLayer(datetime_layer)

        date_layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer&field=start_field:date", "test", "memory")
        self.assertTrue(date_layer.isValid())
        self.assertEqual(date_layer.fields()[2].type(), QVariant.Date)
        date_context = QgsVectorLayerTemporalContext()
        date_context.setLayer(date_layer)

        # default QgsDateTimeRange includes beginning AND end
        filter_range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)))

        props = QgsVectorLayerTemporalProperties(enabled=False)
        props.setMode(QgsVectorLayerTemporalProperties.ModeFeatureDateTimeInstantFromField)
        props.setStartField('start_field')
        # createFilterString should return QString() (== '' in Python world) in case the QgsVectorLayerTemporalProperties is not yet active
        self.assertEqual('', props.createFilterString(datetime_context, filter_range))

        props.setIsActive(True)

        # map range              [-------------------------]
        # feature ranges         .                         . |    (false)
        #                        .                         |      (true)
        #                        .             |           .      (true)
        #                        |                         .      (true)
        #                    |   .                         .      (false)
        #
        # => feature time <= end of range AND feature time >= start of range
        self.assertEqual(props.createFilterString(datetime_context, filter_range), '("start_field" >= make_datetime(2019,3,4,11,12,13) AND "start_field" <= make_datetime(2020,5,6,8,9,10)) OR "start_field" IS NULL')
        self.assertEqual(props.createFilterString(date_context, filter_range), '(to_datetime( "start_field" ) >= make_datetime(2019,3,4,11,12,13) AND to_datetime( "start_field" ) <= make_datetime(2020,5,6,8,9,10)) OR to_datetime( "start_field" ) IS NULL')

        filter_range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeBeginning=False, includeEnd=False)
        # map range              (-------------------------)
        # feature ranges         .                         . |    (false)
        #                        .                         |      (false)
        #                        .             |           .      (true)
        #                        |                         .      (false)
        #                    |   .                         .      (false)
        #
        # => feature time < end of range AND feature time > start of range
        self.assertEqual(props.createFilterString(datetime_context, filter_range), '("start_field" > make_datetime(2019,3,4,11,12,13) AND "start_field" < make_datetime(2020,5,6,8,9,10)) OR "start_field" IS NULL')
        self.assertEqual(props.createFilterString(date_context, filter_range), '(to_datetime( "start_field" ) > make_datetime(2019,3,4,11,12,13) AND to_datetime( "start_field" ) < make_datetime(2020,5,6,8,9,10)) OR to_datetime( "start_field" ) IS NULL')

        filter_range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeBeginning=False)
        # map range              (-------------------------]
        # feature ranges         .                         . |    (false)
        #                        .                         |      (true)
        #                        .             |           .      (true)
        #                        |                         .      (false)
        #                    |   .                         .      (false)
        #
        # => feature time <= end of range AND feature time > start of range
        self.assertEqual(props.createFilterString(datetime_context, filter_range), '("start_field" > make_datetime(2019,3,4,11,12,13) AND "start_field" <= make_datetime(2020,5,6,8,9,10)) OR "start_field" IS NULL')
        self.assertEqual(props.createFilterString(date_context, filter_range), '(to_datetime( "start_field" ) > make_datetime(2019,3,4,11,12,13) AND to_datetime( "start_field" ) <= make_datetime(2020,5,6,8,9,10)) OR to_datetime( "start_field" ) IS NULL')

        filter_range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeEnd=False)
        # map range              [-------------------------)
        # feature ranges         .                         . |    (false)
        #                        .                         |      (false)
        #                        .             |           .      (true)
        #                        |                         .      (true)
        #                    |   .                         .      (false)
        #
        # => feature time < end of range AND feature time >= start of range
        self.assertEqual(props.createFilterString(datetime_context, filter_range), '("start_field" >= make_datetime(2019,3,4,11,12,13) AND "start_field" < make_datetime(2020,5,6,8,9,10)) OR "start_field" IS NULL')
        self.assertEqual(props.createFilterString(date_context, filter_range), '(to_datetime( "start_field" ) >= make_datetime(2019,3,4,11,12,13) AND to_datetime( "start_field" ) < make_datetime(2020,5,6,8,9,10)) OR to_datetime( "start_field" ) IS NULL')

        # with fixed duration
        props.setFixedDuration(3)
        props.setDurationUnits(QgsUnitTypes.TemporalDays)
        filter_range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)))
        # map range              [-------------------------]
        # feature ranges         .                         . [-------)  (false)
        #                        .                         [-------)    (true)
        #                        .                     [---.---)        (true)
        #                        .            [-------)    .            (true)
        #                        [-------)                 .            (true)
        #                    [---.---)                     .            (true)
        #                [-------)                         .            (false)
        #          [-------)     .                         .            (false)
        #
        # => start of feature <= end of range AND start of feature + duration > start of range
        # OR start of feature <= end of range AND start of feature > start of range - duration
        self.assertEqual(props.createFilterString(datetime_context, filter_range), '("start_field" > make_datetime(2019,3,1,11,12,13) AND "start_field" <= make_datetime(2020,5,6,8,9,10)) OR "start_field" IS NULL')
        self.assertEqual(props.createFilterString(date_context, filter_range), '(to_datetime( "start_field" ) > make_datetime(2019,3,1,11,12,13) AND to_datetime( "start_field" ) <= make_datetime(2020,5,6,8,9,10)) OR to_datetime( "start_field" ) IS NULL')

        filter_range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeBeginning=False, includeEnd=False)
        # map range              (-------------------------)
        # feature ranges         .                         . [-------)  (false)
        #                        .                         [-------)    (false)
        #                        .                     [---.---)        (true)
        #                        .            [-------)    .            (true)
        #                        [-------)                 .            (true)
        #                    [---.---)                     .            (true)
        #                [-------)                         .            (false)
        #          [-------)     .                         .            (false)
        #
        # => start of feature < end of range AND start of feature + duration > start of range
        # OR start of feature < end of range AND start of feature > start of range - duration
        self.assertEqual(props.createFilterString(datetime_context, filter_range), '("start_field" > make_datetime(2019,3,1,11,12,13) AND "start_field" < make_datetime(2020,5,6,8,9,10)) OR "start_field" IS NULL')
        self.assertEqual(props.createFilterString(date_context, filter_range), '(to_datetime( "start_field" ) > make_datetime(2019,3,1,11,12,13) AND to_datetime( "start_field" ) < make_datetime(2020,5,6,8,9,10)) OR to_datetime( "start_field" ) IS NULL')

        filter_range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeBeginning=False)
        # map range              (-------------------------]
        # feature ranges         .                         . [-------)  (false)
        #                        .                         [-------)    (true)
        #                        .                     [---.---)        (true)
        #                        .            [-------)    .            (true)
        #                        [-------)                 .            (true)
        #                    [---.---)                     .            (true)
        #                [-------)                         .            (false)
        #          [-------)     .                         .            (false)
        #
        # => start of feature <= end of range AND start of feature + duration > start of range
        # OR start of feature <= end of range AND start of feature > start of range - duration
        self.assertEqual(props.createFilterString(datetime_context, filter_range), '("start_field" > make_datetime(2019,3,1,11,12,13) AND "start_field" <= make_datetime(2020,5,6,8,9,10)) OR "start_field" IS NULL')
        self.assertEqual(props.createFilterString(date_context, filter_range), '(to_datetime( "start_field" ) > make_datetime(2019,3,1,11,12,13) AND to_datetime( "start_field" ) <= make_datetime(2020,5,6,8,9,10)) OR to_datetime( "start_field" ) IS NULL')

        # THIS is the QGIS default: using a range with includeBeginning=true and includeEnd=false
        # and the temporal properties exactly the same
        filter_range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeEnd=False)
        # map range              [-------------------------)
        # feature ranges         .                         . [-------)  (false)
        #                        .                         [-------)    (false)
        #                        .                     [---.---)        (true)
        #                        .            [-------)    .            (true)
        #                        [-------)                 .            (true)
        #                    [---.---)                     .            (true)
        #                [-------)                         .            (false)
        #          [-------)     .                         .            (false)
        #
        # => start of feature < end of range AND start of feature + duration > start of range
        # OR start of feature < end of range AND start of feature > start of range - duration
        self.assertEqual(props.createFilterString(datetime_context, filter_range), '("start_field" > make_datetime(2019,3,1,11,12,13) AND "start_field" < make_datetime(2020,5,6,8,9,10)) OR "start_field" IS NULL')
        self.assertEqual(props.createFilterString(date_context, filter_range), '(to_datetime( "start_field" ) > make_datetime(2019,3,1,11,12,13) AND to_datetime( "start_field" ) < make_datetime(2020,5,6,8,9,10)) OR to_datetime( "start_field" ) IS NULL')

        # since 3.22 there is also the option to include the end of the feature event
        props.setLimitMode(Qgis.VectorTemporalLimitMode.IncludeBeginIncludeEnd)
        # map range              [-------------------------)
        # feature ranges         .                         . [-------]  (false)
        #                        .                         [-------]    (false)
        #                        .                     [---.---]        (true)
        #                        .            [-------]    .            (true)
        #                        [-------]                 .            (true)
        #                    [---.---]                     .            (true)
        #                [-------]                         .            (true)
        #          [-------]     .                         .            (false)
        # => start of feature < end of range AND start of feature + duration >= start of range
        # OR start of feature < end of range AND start of feature >= start of range - duration
        self.assertEqual(props.createFilterString(datetime_context, filter_range), '("start_field" >= make_datetime(2019,3,1,11,12,13) AND "start_field" < make_datetime(2020,5,6,8,9,10)) OR "start_field" IS NULL')
        self.assertEqual(props.createFilterString(date_context, filter_range), '(to_datetime( "start_field" ) >= make_datetime(2019,3,1,11,12,13) AND to_datetime( "start_field" ) < make_datetime(2020,5,6,8,9,10)) OR to_datetime( "start_field" ) IS NULL')
        props.setLimitMode(Qgis.VectorTemporalLimitMode.IncludeBeginExcludeEnd)  # back to default

        # different unit
        props.setDurationUnits(QgsUnitTypes.TemporalMinutes)
        filter_range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)))
        self.assertEqual(props.createFilterString(datetime_context, filter_range), '("start_field" > make_datetime(2019,3,4,11,9,13) AND "start_field" <= make_datetime(2020,5,6,8,9,10)) OR "start_field" IS NULL')
        self.assertEqual(props.createFilterString(date_context, filter_range), '(to_datetime( "start_field" ) > make_datetime(2019,3,4,11,9,13) AND to_datetime( "start_field" ) <= make_datetime(2020,5,6,8,9,10)) OR to_datetime( "start_field" ) IS NULL')

        # accumulate mode
        props.setFixedDuration(0)
        props.setAccumulateFeatures(True)
        filter_range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)))
        # with accumulate mode effectively map range starts at -eternity, regardless of what it actually is
        # map range              [-------------------------]
        # feature ranges         .                         . |    (false)
        #                        .                         |      (true)
        #                        .             |           .      (true)
        #                        |                         .      (true)
        #                    |   .                         .      (true)
        #
        # => feature time <= end of range
        self.assertEqual(props.createFilterString(datetime_context, filter_range), '("start_field" <= make_datetime(2020,5,6,8,9,10)) OR "start_field" IS NULL')
        self.assertEqual(props.createFilterString(date_context, filter_range), '(to_datetime( "start_field" ) <= make_datetime(2020,5,6,8,9,10)) OR to_datetime( "start_field" ) IS NULL')

        filter_range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeBeginning=False, includeEnd=False)
        # with accumulate mode effectively map range starts at -eternity, regardless of what it actually is
        # map range              (-------------------------)
        # feature ranges         .                         . |    (false)
        #                        .                         |      (false)
        #                        .             |           .      (true)
        #                        |                         .      (true)
        #                    |   .                         .      (true)
        #
        # => feature time < end of range
        self.assertEqual(props.createFilterString(datetime_context, filter_range), '("start_field" < make_datetime(2020,5,6,8,9,10)) OR "start_field" IS NULL')
        self.assertEqual(props.createFilterString(date_context, filter_range), '(to_datetime( "start_field" ) < make_datetime(2020,5,6,8,9,10)) OR to_datetime( "start_field" ) IS NULL')

        filter_range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeBeginning=False)
        # with accumulate mode effectively map range starts at -eternity, regardless of what it actually is
        # map range              (-------------------------]
        # feature ranges         .                         . |    (false)
        #                        .                         |      (true)
        #                        .             |           .      (true)
        #                        |                         .      (true)
        #                    |   .                         .      (true)
        #
        # => feature time <= end of range
        self.assertEqual(props.createFilterString(datetime_context, filter_range), '("start_field" <= make_datetime(2020,5,6,8,9,10)) OR "start_field" IS NULL')
        self.assertEqual(props.createFilterString(date_context, filter_range), '(to_datetime( "start_field" ) <= make_datetime(2020,5,6,8,9,10)) OR to_datetime( "start_field" ) IS NULL')

        filter_range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeEnd=False)
        # with accumulate mode effectively map range starts at -eternity, regardless of what it actually is
        # map range              [-------------------------)
        # feature ranges         .                         . |    (false)
        #                        .                         |      (false)
        #                        .             |           .      (true)
        #                        |                         .      (true)
        #                    |   .                         .      (true)
        #
        # => feature time < end of range
        self.assertEqual(props.createFilterString(datetime_context, filter_range), '("start_field" < make_datetime(2020,5,6,8,9,10)) OR "start_field" IS NULL')
        self.assertEqual(props.createFilterString(date_context, filter_range), '(to_datetime( "start_field" ) < make_datetime(2020,5,6,8,9,10)) OR to_datetime( "start_field" ) IS NULL')

        # accumulate mode, with duration
        props.setFixedDuration(3)
        props.setDurationUnits(QgsUnitTypes.TemporalDays)
        filter_range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)))
        # with accumulate mode effectively map range starts at -eternity, regardless of what it actually is
        # map range              [-------------------------]
        # feature ranges         .                         . [-------)  (false)
        #                        .                         [-------)    (true)
        #                        .                     [---.---)        (true)
        #                        .            [-------)    .            (true)
        #                        [-------)                 .            (true)
        #                    [---.---)                     .            (true)
        #                [-------)                         .            (true)
        #          [-------)     .                         .            (true)
        #
        # => start of feature <= end of range
        self.assertEqual(props.createFilterString(datetime_context, filter_range), '("start_field" <= make_datetime(2020,5,6,8,9,10)) OR "start_field" IS NULL')
        self.assertEqual(props.createFilterString(date_context, filter_range), '(to_datetime( "start_field" ) <= make_datetime(2020,5,6,8,9,10)) OR to_datetime( "start_field" ) IS NULL')

        filter_range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeBeginning=False, includeEnd=False)
        # with accumulate mode effectively map range starts at -eternity, regardless of what it actually is
        # map range              (-------------------------)
        # feature ranges         .                         . [-------)  (false)
        #                        .                         [-------)    (false)
        #                        .                     [---.---)        (true)
        #                        .            [-------)    .            (true)
        #                        [-------)                 .            (true)
        #                    [---.---)                     .            (true)
        #                [-------)                         .            (true)
        #          [-------)     .                         .            (true)
        #
        # => start of feature < end of range
        self.assertEqual(props.createFilterString(datetime_context, filter_range), '("start_field" < make_datetime(2020,5,6,8,9,10)) OR "start_field" IS NULL')
        self.assertEqual(props.createFilterString(date_context, filter_range), '(to_datetime( "start_field" ) < make_datetime(2020,5,6,8,9,10)) OR to_datetime( "start_field" ) IS NULL')

        filter_range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeBeginning=False)
        # with accumulate mode effectively map range starts at -eternity, regardless of what it actually is
        # map range              (-------------------------]
        # feature ranges         .                         . [-------)  (false)
        #                        .                         [-------)    (true)
        #                        .                     [---.---)        (true)
        #                        .            [-------)    .            (true)
        #                        [-------)                 .            (true)
        #                    [---.---)                     .            (true)
        #                [-------)                         .            (true)
        #          [-------)     .                         .            (true)
        #
        # => start of feature <= end of range
        self.assertEqual(props.createFilterString(datetime_context, filter_range), '("start_field" <= make_datetime(2020,5,6,8,9,10)) OR "start_field" IS NULL')
        self.assertEqual(props.createFilterString(date_context, filter_range), '(to_datetime( "start_field" ) <= make_datetime(2020,5,6,8,9,10)) OR to_datetime( "start_field" ) IS NULL')

        filter_range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeEnd=False)
        # with accumulate mode effectively map range starts at -eternity, regardless of what it actually is
        # map range              [-------------------------)
        # feature ranges         .                         . [-------)  (false)
        #                        .                         [-------)    (false)
        #                        .                     [---.---)        (true)
        #                        .            [-------)    .            (true)
        #                        [-------)                 .            (true)
        #                    [---.---)                     .            (true)
        #                [-------)                         .            (true)
        #          [-------)     .                         .            (true)
        #
        # => start of feature < end of range
        self.assertEqual(props.createFilterString(datetime_context, filter_range), '("start_field" < make_datetime(2020,5,6,8,9,10)) OR "start_field" IS NULL')
        self.assertEqual(props.createFilterString(date_context, filter_range), '(to_datetime( "start_field" ) < make_datetime(2020,5,6,8,9,10)) OR to_datetime( "start_field" ) IS NULL')

    def testDualFieldMode(self):
        # testing both a layer/context with Datetime fields as one with Date (only) fields, as the last one should be added a cast to datetime
        datetime_layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer&field=start_field:datetime&field=end_field:datetime", "test", "memory")
        self.assertTrue(datetime_layer.isValid())
        self.assertEqual(datetime_layer.fields()[2].type(), QVariant.DateTime)
        self.assertEqual(datetime_layer.fields()[3].type(), QVariant.DateTime)
        datetime_context = QgsVectorLayerTemporalContext()
        datetime_context.setLayer(datetime_layer)

        date_layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer&field=start_field:date&field=end_field:date", "test", "memory")
        self.assertTrue(date_layer.isValid())
        self.assertEqual(date_layer.fields()[2].type(), QVariant.Date)
        self.assertEqual(date_layer.fields()[3].type(), QVariant.Date)
        date_context = QgsVectorLayerTemporalContext()
        date_context.setLayer(date_layer)

        # default QgsDateTimeRange includes beginning AND end
        filter_range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)))

        props = QgsVectorLayerTemporalProperties(enabled=False)
        props.setMode(QgsVectorLayerTemporalProperties.ModeFeatureDateTimeStartAndEndFromFields)
        props.setStartField('start_field')
        props.setEndField('end_field')
        # createFilterString should return QString() (== '' in Python world) in case the QgsVectorLayerTemporalProperties is not yet active
        self.assertEqual('', props.createFilterString(datetime_context, filter_range))

        props.setIsActive(True)

        # map range              [-------------------------]
        # feature ranges         .                         . [-------)  (false)
        #                        .                         [-------)    (true)
        #                        .                     [---.---)        (true)
        #                        .            [-------)    .            (true)
        #                        [-------)                 .            (true)
        #                    [---.---)                     .            (true)
        #                [-------)                         .            (false)
        #          [-------)     .                         .            (false)
        #
        # => start of feature <= end of range AND end of feature > start of range
        self.assertEqual(props.createFilterString(datetime_context, filter_range), '("start_field" <= make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL) AND ("end_field" > make_datetime(2019,3,4,11,12,13) OR "end_field" IS NULL)')
        self.assertEqual(props.createFilterString(date_context, filter_range), '(to_datetime( "start_field" ) <= make_datetime(2020,5,6,8,9,10) OR to_datetime( "start_field" ) IS NULL) AND (to_datetime( "end_field" ) > make_datetime(2019,3,4,11,12,13) OR to_datetime( "end_field" ) IS NULL)')

        # map range              (-------------------------)
        # feature ranges         .                         . [-------)  (false)
        #                        .                         [-------)    (false)
        #                        .                     [---.---)        (true)
        #                        .            [-------)    .            (true)
        #                        [-------)                 .            (true)
        #                    [---.---)                     .            (true)
        #                [-------)                         .            (false)
        #          [-------)     .                         .            (false)
        #
        # => start of feature < end of range AND end of feature > start of range
        filter_range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeBeginning=False, includeEnd=False)
        self.assertEqual(props.createFilterString(datetime_context, filter_range), '("start_field" < make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL) AND ("end_field" > make_datetime(2019,3,4,11,12,13) OR "end_field" IS NULL)')
        self.assertEqual(props.createFilterString(date_context, filter_range), '(to_datetime( "start_field" ) < make_datetime(2020,5,6,8,9,10) OR to_datetime( "start_field" ) IS NULL) AND (to_datetime( "end_field" ) > make_datetime(2019,3,4,11,12,13) OR to_datetime( "end_field" ) IS NULL)')

        # map range              (-------------------------]
        # feature ranges         .                         . [-------)  (false)
        #                        .                         [-------)    (true)
        #                        .                     [---.---)        (true)
        #                        .            [-------)    .            (true)
        #                        [-------)                 .            (true)
        #                    [---.---)                     .            (true)
        #                [-------)                         .            (false)
        #          [-------)     .                         .            (false)
        #
        # => start of feature <= end of range AND end of feature > start of range
        filter_range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeBeginning=False)
        self.assertEqual(props.createFilterString(datetime_context, filter_range), '("start_field" <= make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL) AND ("end_field" > make_datetime(2019,3,4,11,12,13) OR "end_field" IS NULL)')
        self.assertEqual(props.createFilterString(date_context, filter_range), '(to_datetime( "start_field" ) <= make_datetime(2020,5,6,8,9,10) OR to_datetime( "start_field" ) IS NULL) AND (to_datetime( "end_field" ) > make_datetime(2019,3,4,11,12,13) OR to_datetime( "end_field" ) IS NULL)')

        # THIS is the QGIS default: using a range with includeBeginning=true and includeEnd=false
        # map range              [-------------------------)
        # feature ranges         .                         . [-------)  (false)
        #                        .                         [-------)    (false)
        #                        .                     [---.---)        (true)
        #                        .            [-------)    .            (true)
        #                        [-------)                 .            (true)
        #                    [---.---)                     .            (true)
        #                [-------)                         .            (false)
        #          [-------)     .                         .            (false)
        #
        # => start of feature < end of range AND end of feature > start of range
        filter_range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeEnd=False)
        self.assertEqual(props.createFilterString(datetime_context, filter_range), '("start_field" < make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL) AND ("end_field" > make_datetime(2019,3,4,11,12,13) OR "end_field" IS NULL)')
        self.assertEqual(props.createFilterString(date_context, filter_range), '(to_datetime( "start_field" ) < make_datetime(2020,5,6,8,9,10) OR to_datetime( "start_field" ) IS NULL) AND (to_datetime( "end_field" ) > make_datetime(2019,3,4,11,12,13) OR to_datetime( "end_field" ) IS NULL)')
        # since 3.22 there is also the option to include the end of the feature event
        props.setLimitMode(Qgis.VectorTemporalLimitMode.IncludeBeginIncludeEnd)
        # map range              [-------------------------)
        # feature ranges         .                         . [-------]  (false)
        #                        .                         [-------]    (false)
        #                        .                     [---.---]        (true)
        #                        .            [-------]    .            (true)
        #                        [-------]                 .            (true)
        #                    [---.---]                     .            (true)
        #                [-------]                         .            (true)
        #          [-------]     .                         .            (false)
        #
        # => start of feature < end of range AND end of feature >= start of range
        self.assertEqual(props.createFilterString(datetime_context, filter_range), '("start_field" < make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL) AND ("end_field" >= make_datetime(2019,3,4,11,12,13) OR "end_field" IS NULL)')
        self.assertEqual(props.createFilterString(date_context, filter_range), '(to_datetime( "start_field" ) < make_datetime(2020,5,6,8,9,10) OR to_datetime( "start_field" ) IS NULL) AND (to_datetime( "end_field" ) >= make_datetime(2019,3,4,11,12,13) OR to_datetime( "end_field" ) IS NULL)')
        props.setLimitMode(Qgis.VectorTemporalLimitMode.IncludeBeginExcludeEnd)  # back to default

        # features go to +eternity
        props.setEndField('')
        # includes beginning AND end
        filter_range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)))
        # map range              [-------------------------]
        # feature ranges         .                         . [-------->  (false)
        #                        .                         [---------->  (true)
        #                        .            [------------.---------->  (true)
        #                        [-------------------------.---------->  (true)
        #          [-------------.-------------------------.---------->  (true)
        #
        # => start of feature <= end of range
        self.assertEqual(props.createFilterString(datetime_context, filter_range), '"start_field" <= make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL')
        self.assertEqual(props.createFilterString(date_context, filter_range), 'to_datetime( "start_field" ) <= make_datetime(2020,5,6,8,9,10) OR to_datetime( "start_field" ) IS NULL')

        filter_range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeBeginning=False, includeEnd=False)
        # map range              (-------------------------)
        # feature ranges         .                         . [-------->  (false)
        #                        .                         [---------->  (false)
        #                        .            [------------.---------->  (true)
        #                        [-------------------------.---------->  (true)
        #          [-------------.-------------------------.---------->  (true)
        #
        # => start of feature < end of range
        self.assertEqual(props.createFilterString(datetime_context, filter_range), '"start_field" < make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL')
        self.assertEqual(props.createFilterString(date_context, filter_range), 'to_datetime( "start_field" ) < make_datetime(2020,5,6,8,9,10) OR to_datetime( "start_field" ) IS NULL')

        filter_range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeBeginning=False)
        # map range              (-------------------------]
        # feature ranges         .                         . [-------->  (false)
        #                        .                         [---------->  (true)
        #                        .            [------------.---------->  (true)
        #                        [-------------------------.---------->  (true)
        #          [-------------.-------------------------.---------->  (true)
        #
        # => start of feature <= end of range
        self.assertEqual(props.createFilterString(datetime_context, filter_range), '"start_field" <= make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL')
        self.assertEqual(props.createFilterString(date_context, filter_range), 'to_datetime( "start_field" ) <= make_datetime(2020,5,6,8,9,10) OR to_datetime( "start_field" ) IS NULL')

        # THIS is the QGIS default: using a range with includeBeginning=true and includeEnd=false
        filter_range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeEnd=False)
        # map range              [-------------------------)
        # feature ranges         .                         . [-------->  (false)
        #                        .                         [---------->  (false)
        #                        .            [------------.---------->  (true)
        #                        [-------------------------.---------->  (true)
        #          [-------------.-------------------------.---------->  (true)
        #
        # => start of feature < end of range
        self.assertEqual(props.createFilterString(datetime_context, filter_range), '"start_field" < make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL')
        self.assertEqual(props.createFilterString(date_context, filter_range), 'to_datetime( "start_field" ) < make_datetime(2020,5,6,8,9,10) OR to_datetime( "start_field" ) IS NULL')

        # features start at -eternity
        props.setStartField('')
        props.setEndField('end_field')
        # includes beginning AND end
        filter_range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)))
        # map range              [-------------------------]
        # feature ranges --------.-------------------------.---------)  (true)
        #                --------.-------------------------)            (true)
        #                --------.--------------------)    .            (true)
        #                --------)                         .            (false)
        #                -----)                            .            (false)
        #
        # => end of feature > start of range
        self.assertEqual(props.createFilterString(datetime_context, filter_range), '"end_field" > make_datetime(2019,3,4,11,12,13) OR "end_field" IS NULL')
        self.assertEqual(props.createFilterString(date_context, filter_range), 'to_datetime( "end_field" ) > make_datetime(2019,3,4,11,12,13) OR to_datetime( "end_field" ) IS NULL')

        filter_range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeBeginning=False, includeEnd=False)
        # map range              (-------------------------)
        # feature ranges --------.-------------------------.---------)  (true)
        #                --------.-------------------------)            (true)
        #                --------.--------------------)    .            (true)
        #                --------)                         .            (false)
        #                -----)                            .            (false)
        #
        # => end of feature > start of range
        self.assertEqual(props.createFilterString(datetime_context, filter_range), '"end_field" > make_datetime(2019,3,4,11,12,13) OR "end_field" IS NULL')
        self.assertEqual(props.createFilterString(date_context, filter_range), 'to_datetime( "end_field" ) > make_datetime(2019,3,4,11,12,13) OR to_datetime( "end_field" ) IS NULL')

        filter_range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeBeginning=False)
        # map range              (-------------------------]
        # feature ranges --------.-------------------------.---------)  (true)
        #                --------.-------------------------)            (true)
        #                --------.--------------------)    .            (true)
        #                --------)                         .            (false)
        #                -----)                            .            (false)
        #
        # => end of feature > start of range
        self.assertEqual(props.createFilterString(datetime_context, filter_range), '"end_field" > make_datetime(2019,3,4,11,12,13) OR "end_field" IS NULL')
        self.assertEqual(props.createFilterString(date_context, filter_range), 'to_datetime( "end_field" ) > make_datetime(2019,3,4,11,12,13) OR to_datetime( "end_field" ) IS NULL')

        # THIS is the QGIS default: using a range with includeBeginning=true and includeEnd=false
        filter_range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeEnd=False)
        # map range              [-------------------------)
        # feature ranges --------.-------------------------.---------)  (true)
        #                --------.-------------------------)            (true)
        #                --------.--------------------)    .            (true)
        #                --------)                         .            (false)
        #                -----)                            .            (false)
        #
        # => end of feature > start of range
        self.assertEqual(props.createFilterString(datetime_context, filter_range), '"end_field" > make_datetime(2019,3,4,11,12,13) OR "end_field" IS NULL')
        self.assertEqual(props.createFilterString(date_context, filter_range), 'to_datetime( "end_field" ) > make_datetime(2019,3,4,11,12,13) OR to_datetime( "end_field" ) IS NULL')
        # since 3.22 there is also the option to include the end of the feature event
        # => end of feature >= start of range
        props.setLimitMode(Qgis.VectorTemporalLimitMode.IncludeBeginIncludeEnd)
        # map range              [-------------------------)
        # feature ranges --------.-------------------------.---------]  (true)
        #                --------.-------------------------]            (true)
        #                --------.--------------------]    .            (true)
        #                --------]                         .            (true)
        #                -----]                            .            (false)
        #
        # => end of feature >= start of range
        self.assertEqual(props.createFilterString(datetime_context, filter_range), '"end_field" >= make_datetime(2019,3,4,11,12,13) OR "end_field" IS NULL')
        self.assertEqual(props.createFilterString(date_context, filter_range), 'to_datetime( "end_field" ) >= make_datetime(2019,3,4,11,12,13) OR to_datetime( "end_field" ) IS NULL')
        props.setLimitMode(Qgis.VectorTemporalLimitMode.IncludeBeginExcludeEnd)  # back to default

    def testStartAndDurationMode(self):
        # testing both a layer/context with Datetime field as one with Date (only) field, as the last one should be added a cast to datetime
        datetime_layer = QgsVectorLayer(
            "Point?field=fldtxt:string&field=fldint:integer&field=start_field:datetime&field=duration:double",
            "test", "memory")
        self.assertTrue(datetime_layer.isValid())
        self.assertEqual(datetime_layer.fields()[2].type(), QVariant.DateTime)
        self.assertEqual(datetime_layer.fields()[3].type(), QVariant.Double)
        datetime_context = QgsVectorLayerTemporalContext()
        datetime_context.setLayer(datetime_layer)

        date_layer = QgsVectorLayer(
            "Point?field=fldtxt:string&field=fldint:integer&field=start_field:date&field=duration:double",
            "test", "memory")
        self.assertTrue(date_layer.isValid())
        self.assertEqual(date_layer.fields()[2].type(), QVariant.Date)
        self.assertEqual(date_layer.fields()[3].type(), QVariant.Double)
        date_context = QgsVectorLayerTemporalContext()
        date_context.setLayer(date_layer)

        filter_range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)))

        props = QgsVectorLayerTemporalProperties(enabled=False)
        props.setMode(QgsVectorLayerTemporalProperties.ModeFeatureDateTimeStartAndDurationFromFields)
        props.setStartField('start_field')
        props.setDurationField('duration')
        props.setDurationUnits(QgsUnitTypes.TemporalMilliseconds)
        # createFilterString should return QString() (== '' in Python world) in case the QgsVectorLayerTemporalProperties is not yet active
        self.assertEqual('', props.createFilterString(datetime_context, filter_range))

        props.setIsActive(True)
        # map range              [-------------------------]
        # feature ranges         .                         . [-------)  (false)
        #                        .                         [-------)    (true)
        #                        .                     [---.---)        (true)
        #                        .            [-------)    .            (true)
        #                        [-------)                 .            (true)
        #                    [---.---)                     .            (true)
        #                [-------)                         .            (false)
        #          [-------)     .                         .            (false)
        #
        # => start of feature <= end of range AND start + duration > start of range
        self.assertEqual(props.createFilterString(datetime_context, filter_range),
                         '("start_field" <= make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL) AND (("start_field" + make_interval(0,0,0,0,0,0,"duration"/1000) > make_datetime(2019,3,4,11,12,13)) OR "duration" IS NULL)')
        self.assertEqual(props.createFilterString(date_context, filter_range),
                         '(to_datetime( "start_field" ) <= make_datetime(2020,5,6,8,9,10) OR to_datetime( "start_field" ) IS NULL) AND ((to_datetime( "start_field" ) + make_interval(0,0,0,0,0,0,"duration"/1000) > make_datetime(2019,3,4,11,12,13)) OR "duration" IS NULL)')

        filter_range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)),
                                        QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeBeginning=False, includeEnd=False)
        # map range              (-------------------------)
        # feature ranges         .                         . [-------)  (false)
        #                        .                         [-------)    (false)
        #                        .                     [---.---)        (true)
        #                        .            [-------)    .            (true)
        #                        [-------)                 .            (true)
        #                    [---.---)                     .            (true)
        #                [-------)                         .            (false)
        #          [-------)     .                         .            (false)
        #
        # => start of feature < end of range AND start + duration > start of range
        self.assertEqual(props.createFilterString(datetime_context, filter_range),
                         '("start_field" < make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL) AND (("start_field" + make_interval(0,0,0,0,0,0,"duration"/1000) > make_datetime(2019,3,4,11,12,13)) OR "duration" IS NULL)')
        self.assertEqual(props.createFilterString(date_context, filter_range),
                         '(to_datetime( "start_field" ) < make_datetime(2020,5,6,8,9,10) OR to_datetime( "start_field" ) IS NULL) AND ((to_datetime( "start_field" ) + make_interval(0,0,0,0,0,0,"duration"/1000) > make_datetime(2019,3,4,11,12,13)) OR "duration" IS NULL)')

        filter_range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)),
                                        QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeBeginning=False)
        # map range              (-------------------------]
        # feature ranges         .                         . [-------)  (false)
        #                        .                         [-------)    (true)
        #                        .                     [---.---)        (true)
        #                        .            [-------)    .            (true)
        #                        [-------)                 .            (true)
        #                    [---.---)                     .            (true)
        #                [-------)                         .            (false)
        #          [-------)     .                         .            (false)
        #
        # => start of feature <= end of range AND start + duration > start of range
        self.assertEqual(props.createFilterString(datetime_context, filter_range),
                         '("start_field" <= make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL) AND (("start_field" + make_interval(0,0,0,0,0,0,"duration"/1000) > make_datetime(2019,3,4,11,12,13)) OR "duration" IS NULL)')
        self.assertEqual(props.createFilterString(date_context, filter_range),
                         '(to_datetime( "start_field" ) <= make_datetime(2020,5,6,8,9,10) OR to_datetime( "start_field" ) IS NULL) AND ((to_datetime( "start_field" ) + make_interval(0,0,0,0,0,0,"duration"/1000) > make_datetime(2019,3,4,11,12,13)) OR "duration" IS NULL)')

        # THIS is the QGIS default: using a range with includeBeginning=true and includeEnd=false
        filter_range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)),
                                        QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeEnd=False)
        # map range              [-------------------------)
        # feature ranges         .                         . [-------)  (false)
        #                        .                         [-------)    (true)
        #                        .                     [---.---)        (true)
        #                        .            [-------)    .            (true)
        #                        [-------)                 .            (true)
        #                    [---.---)                     .            (true)
        #                [-------)                         .            (false)
        #          [-------)     .                         .            (false)
        #
        # => start of feature < end of range AND start + duration > start of range
        self.assertEqual(props.createFilterString(datetime_context, filter_range),
                         '("start_field" < make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL) AND (("start_field" + make_interval(0,0,0,0,0,0,"duration"/1000) > make_datetime(2019,3,4,11,12,13)) OR "duration" IS NULL)')
        self.assertEqual(props.createFilterString(date_context, filter_range),
                         '(to_datetime( "start_field" ) < make_datetime(2020,5,6,8,9,10) OR to_datetime( "start_field" ) IS NULL) AND ((to_datetime( "start_field" ) + make_interval(0,0,0,0,0,0,"duration"/1000) > make_datetime(2019,3,4,11,12,13)) OR "duration" IS NULL)')
        # map range              [-------------------------)
        # feature ranges         .                         . [-------]  (false)
        #                        .                         [-------]    (false)
        #                        .                     [---.---]        (true)
        #                        .            [-------]    .            (true)
        #                        [-------]                 .            (true)
        #                    [---.---]                     .            (true)
        #                [-------]                         .            (true)
        #          [-------]     .                         .            (false)
        #
        # => start of feature < end of range AND start + duration >= start of range
        props.setLimitMode(Qgis.VectorTemporalLimitMode.IncludeBeginIncludeEnd)
        self.assertEqual(props.createFilterString(datetime_context, filter_range),
                         '("start_field" < make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL) AND (("start_field" + make_interval(0,0,0,0,0,0,"duration"/1000) >= make_datetime(2019,3,4,11,12,13)) OR "duration" IS NULL)')
        self.assertEqual(props.createFilterString(date_context, filter_range),
                         '(to_datetime( "start_field" ) < make_datetime(2020,5,6,8,9,10) OR to_datetime( "start_field" ) IS NULL) AND ((to_datetime( "start_field" ) + make_interval(0,0,0,0,0,0,"duration"/1000) >= make_datetime(2019,3,4,11,12,13)) OR "duration" IS NULL)')
        props.setLimitMode(Qgis.VectorTemporalLimitMode.IncludeBeginExcludeEnd)  # back to default

        # different units
        filter_range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)),
                                        QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)))

        props.setDurationUnits(QgsUnitTypes.TemporalSeconds)
        self.assertEqual(props.createFilterString(datetime_context, filter_range),
                         '("start_field" <= make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL) AND (("start_field" + make_interval(0,0,0,0,0,0,"duration") > make_datetime(2019,3,4,11,12,13)) OR "duration" IS NULL)')
        self.assertEqual(props.createFilterString(date_context, filter_range),
                         '(to_datetime( "start_field" ) <= make_datetime(2020,5,6,8,9,10) OR to_datetime( "start_field" ) IS NULL) AND ((to_datetime( "start_field" ) + make_interval(0,0,0,0,0,0,"duration") > make_datetime(2019,3,4,11,12,13)) OR "duration" IS NULL)')

        props.setDurationUnits(QgsUnitTypes.TemporalMinutes)
        self.assertEqual(props.createFilterString(datetime_context, filter_range),
                         '("start_field" <= make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL) AND (("start_field" + make_interval(0,0,0,0,0,"duration",0) > make_datetime(2019,3,4,11,12,13)) OR "duration" IS NULL)')
        self.assertEqual(props.createFilterString(date_context, filter_range),
                         '(to_datetime( "start_field" ) <= make_datetime(2020,5,6,8,9,10) OR to_datetime( "start_field" ) IS NULL) AND ((to_datetime( "start_field" ) + make_interval(0,0,0,0,0,"duration",0) > make_datetime(2019,3,4,11,12,13)) OR "duration" IS NULL)')

        props.setDurationUnits(QgsUnitTypes.TemporalHours)
        self.assertEqual(props.createFilterString(datetime_context, filter_range),
                         '("start_field" <= make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL) AND (("start_field" + make_interval(0,0,0,0,"duration",0,0) > make_datetime(2019,3,4,11,12,13)) OR "duration" IS NULL)')
        self.assertEqual(props.createFilterString(date_context, filter_range),
                         '(to_datetime( "start_field" ) <= make_datetime(2020,5,6,8,9,10) OR to_datetime( "start_field" ) IS NULL) AND ((to_datetime( "start_field" ) + make_interval(0,0,0,0,"duration",0,0) > make_datetime(2019,3,4,11,12,13)) OR "duration" IS NULL)')

        props.setDurationUnits(QgsUnitTypes.TemporalDays)
        self.assertEqual(props.createFilterString(datetime_context, filter_range),
                         '("start_field" <= make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL) AND (("start_field" + make_interval(0,0,0,"duration",0,0,0) > make_datetime(2019,3,4,11,12,13)) OR "duration" IS NULL)')
        self.assertEqual(props.createFilterString(date_context, filter_range),
                         '(to_datetime( "start_field" ) <= make_datetime(2020,5,6,8,9,10) OR to_datetime( "start_field" ) IS NULL) AND ((to_datetime( "start_field" ) + make_interval(0,0,0,"duration",0,0,0) > make_datetime(2019,3,4,11,12,13)) OR "duration" IS NULL)')

        props.setDurationUnits(QgsUnitTypes.TemporalWeeks)
        self.assertEqual(props.createFilterString(datetime_context, filter_range),
                         '("start_field" <= make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL) AND (("start_field" + make_interval(0,0,"duration",0,0,0,0) > make_datetime(2019,3,4,11,12,13)) OR "duration" IS NULL)')
        self.assertEqual(props.createFilterString(date_context, filter_range),
                         '(to_datetime( "start_field" ) <= make_datetime(2020,5,6,8,9,10) OR to_datetime( "start_field" ) IS NULL) AND ((to_datetime( "start_field" ) + make_interval(0,0,"duration",0,0,0,0) > make_datetime(2019,3,4,11,12,13)) OR "duration" IS NULL)')

        props.setDurationUnits(QgsUnitTypes.TemporalMonths)
        self.assertEqual(props.createFilterString(datetime_context, filter_range),
                         '("start_field" <= make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL) AND (("start_field" + make_interval(0,"duration",0,0,0,0,0) > make_datetime(2019,3,4,11,12,13)) OR "duration" IS NULL)')
        self.assertEqual(props.createFilterString(date_context, filter_range),
                         '(to_datetime( "start_field" ) <= make_datetime(2020,5,6,8,9,10) OR to_datetime( "start_field" ) IS NULL) AND ((to_datetime( "start_field" ) + make_interval(0,"duration",0,0,0,0,0) > make_datetime(2019,3,4,11,12,13)) OR "duration" IS NULL)')

        props.setDurationUnits(QgsUnitTypes.TemporalYears)
        self.assertEqual(props.createFilterString(datetime_context, filter_range),
                         '("start_field" <= make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL) AND (("start_field" + make_interval("duration",0,0,0,0,0,0) > make_datetime(2019,3,4,11,12,13)) OR "duration" IS NULL)')
        self.assertEqual(props.createFilterString(date_context, filter_range),
                         '(to_datetime( "start_field" ) <= make_datetime(2020,5,6,8,9,10) OR to_datetime( "start_field" ) IS NULL) AND ((to_datetime( "start_field" ) + make_interval("duration",0,0,0,0,0,0) > make_datetime(2019,3,4,11,12,13)) OR "duration" IS NULL)')

        props.setDurationUnits(QgsUnitTypes.TemporalDecades)
        self.assertEqual(props.createFilterString(datetime_context, filter_range),
                         '("start_field" <= make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL) AND (("start_field" + make_interval(10 * "duration",0,0,0,0,0,0) > make_datetime(2019,3,4,11,12,13)) OR "duration" IS NULL)')
        self.assertEqual(props.createFilterString(date_context, filter_range),
                         '(to_datetime( "start_field" ) <= make_datetime(2020,5,6,8,9,10) OR to_datetime( "start_field" ) IS NULL) AND ((to_datetime( "start_field" ) + make_interval(10 * "duration",0,0,0,0,0,0) > make_datetime(2019,3,4,11,12,13)) OR "duration" IS NULL)')

        props.setDurationUnits(QgsUnitTypes.TemporalCenturies)
        self.assertEqual(props.createFilterString(datetime_context, filter_range),
                         '("start_field" <= make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL) AND (("start_field" + make_interval(100 * "duration",0,0,0,0,0,0) > make_datetime(2019,3,4,11,12,13)) OR "duration" IS NULL)')
        self.assertEqual(props.createFilterString(date_context, filter_range),
                         '(to_datetime( "start_field" ) <= make_datetime(2020,5,6,8,9,10) OR to_datetime( "start_field" ) IS NULL) AND ((to_datetime( "start_field" ) + make_interval(100 * "duration",0,0,0,0,0,0) > make_datetime(2019,3,4,11,12,13)) OR "duration" IS NULL)')

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
        # map range              [-------------------------]
        # feature ranges         .                         . [-------)  (false)
        #                        .                         [-------)    (true)
        #                        .                     [---.---)        (true)
        #                        .            [-------)    .            (true)
        #                        [-------)                 .            (true)
        #                    [---.---)                     .            (true)
        #                [-------)                         .            (false)
        #          [-------)     .                         .            (false)
        #
        # => start expression <= end of range AND end expression > start of range
        self.assertEqual(props.createFilterString(context, range), '((to_datetime("my string field",  \'yyyy MM dd hh::mm:ss\')") <= make_datetime(2020,5,6,8,9,10)) AND ((to_datetime("my end field",  \'yyyy MM dd hh::mm:ss\')") > make_datetime(2019,3,4,11,12,13))')

        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeBeginning=False, includeEnd=False)
        # map range              (-------------------------)
        # feature ranges         .                         . [-------)  (false)
        #                        .                         [-------)    (false)
        #                        .                     [---.---)        (true)
        #                        .            [-------)    .            (true)
        #                        [-------)                 .            (true)
        #                    [---.---)                     .            (true)
        #                [-------)                         .            (false)
        #          [-------)     .                         .            (false)
        #
        # => start expression < end of range AND end expression > start of range
        self.assertEqual(props.createFilterString(context, range), '((to_datetime("my string field",  \'yyyy MM dd hh::mm:ss\')") < make_datetime(2020,5,6,8,9,10)) AND ((to_datetime("my end field",  \'yyyy MM dd hh::mm:ss\')") > make_datetime(2019,3,4,11,12,13))')

        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeBeginning=False)
        # map range              (-------------------------]
        # feature ranges         .                         . [-------)  (false)
        #                        .                         [-------)    (true)
        #                        .                     [---.---)        (true)
        #                        .            [-------)    .            (true)
        #                        [-------)                 .            (true)
        #                    [---.---)                     .            (true)
        #                [-------)                         .            (false)
        #          [-------)     .                         .            (false)
        #
        # => start expression <= end of range AND end expression > start of range
        self.assertEqual(props.createFilterString(context, range), '((to_datetime("my string field",  \'yyyy MM dd hh::mm:ss\')") <= make_datetime(2020,5,6,8,9,10)) AND ((to_datetime("my end field",  \'yyyy MM dd hh::mm:ss\')") > make_datetime(2019,3,4,11,12,13))')

        # THIS is the QGIS default: using a range with includeBeginning=true and includeEnd=false
        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeEnd=False)
        # map range              [-------------------------)
        # feature ranges         .                         . [-------)  (false)
        #                        .                         [-------)    (false)
        #                        .                     [---.---)        (true)
        #                        .            [-------)    .            (true)
        #                        [-------)                 .            (true)
        #                    [---.---)                     .            (true)
        #                [-------)                         .            (false)
        #          [-------)     .                         .            (false)
        #
        # => start expression < end of range AND end expression > start of range
        self.assertEqual(props.createFilterString(context, range), '((to_datetime("my string field",  \'yyyy MM dd hh::mm:ss\')") < make_datetime(2020,5,6,8,9,10)) AND ((to_datetime("my end field",  \'yyyy MM dd hh::mm:ss\')") > make_datetime(2019,3,4,11,12,13))')
        # map range              [-------------------------)
        # feature ranges         .                         . [-------]  (false)
        #                        .                         [-------]    (false)
        #                        .                     [---.---]        (true)
        #                        .            [-------]    .            (true)
        #                        [-------]                 .            (true)
        #                    [---.---]                     .            (true)
        #                [-------]                         .            (true)
        #          [-------]     .                         .            (false)
        #
        # => start expression < end of range AND end expression >= start of range
        props.setLimitMode(Qgis.VectorTemporalLimitMode.IncludeBeginIncludeEnd)
        self.assertEqual(props.createFilterString(context, range), '((to_datetime("my string field",  \'yyyy MM dd hh::mm:ss\')") < make_datetime(2020,5,6,8,9,10)) AND ((to_datetime("my end field",  \'yyyy MM dd hh::mm:ss\')") >= make_datetime(2019,3,4,11,12,13))')
        props.setLimitMode(Qgis.VectorTemporalLimitMode.IncludeBeginExcludeEnd)  # back to default

        # features go to +eternity
        props.setEndExpression('')
        # includes beginning AND end
        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)))
        # map range              [-------------------------]
        # feature ranges         .                         . [-------->  (false)
        #                        .                         [---------->  (true)
        #                        .            [------------.---------->  (true)
        #                        [-------------------------.---------->  (true)
        #          [-------------.-------------------------.---------->  (true)
        #
        # => start expression <= end of range
        self.assertEqual(props.createFilterString(context, range), '(to_datetime("my string field",  \'yyyy MM dd hh::mm:ss\')") <= make_datetime(2020,5,6,8,9,10)')

        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeBeginning=False, includeEnd=False)
        # map range              (-------------------------)
        # feature ranges         .                         . [-------->  (false)
        #                        .                         [---------->  (false)
        #                        .            [------------.---------->  (true)
        #                        [-------------------------.---------->  (true)
        #          [-------------.-------------------------.---------->  (true)
        #
        # => start expression < end of range
        self.assertEqual(props.createFilterString(context, range), '(to_datetime("my string field",  \'yyyy MM dd hh::mm:ss\')") < make_datetime(2020,5,6,8,9,10)')

        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeBeginning=False)
        # map range              (-------------------------]
        # feature ranges         .                         . [-------->  (false)
        #                        .                         [---------->  (true)
        #                        .            [------------.---------->  (true)
        #                        [-------------------------.---------->  (true)
        #          [-------------.-------------------------.---------->  (true)
        #
        # => start expression <= end of range
        self.assertEqual(props.createFilterString(context, range), '(to_datetime("my string field",  \'yyyy MM dd hh::mm:ss\')") <= make_datetime(2020,5,6,8,9,10)')

        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeEnd=False)
        # map range              [-------------------------)
        # feature ranges         .                         . [-------->  (false)
        #                        .                         [---------->  (false)
        #                        .            [------------.---------->  (true)
        #                        [-------------------------.---------->  (true)
        #          [-------------.-------------------------.---------->  (true)
        #
        # => start expression < end of range
        self.assertEqual(props.createFilterString(context, range), '(to_datetime("my string field",  \'yyyy MM dd hh::mm:ss\')") < make_datetime(2020,5,6,8,9,10)')

        # features start at -eternity
        props.setStartExpression('')
        props.setEndExpression('to_datetime("my end field",  \'yyyy MM dd hh::mm:ss\')"')
        # includes beginning AND end
        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)))
        # map range              [-------------------------]
        # feature ranges --------.-------------------------.---------)  (true)
        #                --------.-------------------------)            (true)
        #                --------.--------------------)    .            (true)
        #                --------)                         .            (false)
        #                -----)                            .            (false)
        #
        # => end of feature > start of range
        self.assertEqual(props.createFilterString(context, range), '(to_datetime("my end field",  \'yyyy MM dd hh::mm:ss\')") > make_datetime(2019,3,4,11,12,13)')

        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeBeginning=False, includeEnd=False)
        # map range              (-------------------------)
        # feature ranges --------.-------------------------.---------)  (true)
        #                --------.-------------------------)            (true)
        #                --------.--------------------)    .            (true)
        #                --------)                         .            (false)
        #                -----)                            .            (false)
        #
        # => end of feature > start of range
        self.assertEqual(props.createFilterString(context, range), '(to_datetime("my end field",  \'yyyy MM dd hh::mm:ss\')") > make_datetime(2019,3,4,11,12,13)')

        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeBeginning=False)
        # map range              (-------------------------]
        # feature ranges --------.-------------------------.---------)  (true)
        #                --------.-------------------------)            (true)
        #                --------.--------------------)    .            (true)
        #                --------)                         .            (false)
        #                -----)                            .            (false)
        #
        # => end of feature > start of range
        self.assertEqual(props.createFilterString(context, range), '(to_datetime("my end field",  \'yyyy MM dd hh::mm:ss\')") > make_datetime(2019,3,4,11,12,13)')

        # THIS is the QGIS default: using a range with includeBeginning=true and includeEnd=false
        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeEnd=False)
        # map range              [-------------------------)
        # feature ranges --------.-------------------------.---------)  (true)
        #                --------.-------------------------)            (true)
        #                --------.--------------------)    .            (true)
        #                --------)                         .            (false)
        #                -----)                            .            (false)
        #
        # => end of feature > start of range
        self.assertEqual(props.createFilterString(context, range), '(to_datetime("my end field",  \'yyyy MM dd hh::mm:ss\')") > make_datetime(2019,3,4,11,12,13)')
        # map range              [-------------------------)
        # feature ranges --------.-------------------------.---------]  (true)
        #                --------.-------------------------]            (true)
        #                --------.--------------------]    .            (true)
        #                --------]                         .            (true)
        #                -----]                            .            (false)
        #        # => end of feature >= start of range
        props.setLimitMode(Qgis.VectorTemporalLimitMode.IncludeBeginIncludeEnd)
        self.assertEqual(props.createFilterString(context, range), '(to_datetime("my end field",  \'yyyy MM dd hh::mm:ss\')") >= make_datetime(2019,3,4,11,12,13)')
        props.setLimitMode(Qgis.VectorTemporalLimitMode.IncludeBeginExcludeEnd)  # back to default


if __name__ == '__main__':
    unittest.main()
