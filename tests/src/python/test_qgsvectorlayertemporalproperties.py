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
                       QgsVectorDataProviderTemporalCapabilities)
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
        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)))
        # ALWAYS must be empty for ModeFixedTemporalRange
        self.assertFalse(props.createFilterString(layer, range))

    def testSingleFieldMode(self):
        layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer&field=start_field:datetime", "test", "memory")
        self.assertTrue(layer.isValid())
        self.assertEqual(layer.fields()[2].type(), QVariant.DateTime)

        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)))

        props = QgsVectorLayerTemporalProperties(enabled=False)
        props.setMode(QgsVectorLayerTemporalProperties.ModeFeatureDateTimeInstantFromField)
        props.setStartField('start_field')
        self.assertFalse(props.createFilterString(layer, range))

        props.setIsActive(True)
        self.assertEqual(props.createFilterString(layer, range), '("start_field" >= make_datetime(2019,3,4,11,12,13) AND "start_field" <= make_datetime(2020,5,6,8,9,10)) OR "start_field" IS NULL')

        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeBeginning=False)
        self.assertEqual(props.createFilterString(layer, range), '("start_field" > make_datetime(2019,3,4,11,12,13) AND "start_field" <= make_datetime(2020,5,6,8,9,10)) OR "start_field" IS NULL')

        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeEnd=False)
        self.assertEqual(props.createFilterString(layer, range), '("start_field" >= make_datetime(2019,3,4,11,12,13) AND "start_field" < make_datetime(2020,5,6,8,9,10)) OR "start_field" IS NULL')

    def testDualFieldMode(self):
        layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer&field=start_field:datetime&field=end_field:datetime", "test", "memory")
        self.assertTrue(layer.isValid())
        self.assertEqual(layer.fields()[2].type(), QVariant.DateTime)
        self.assertEqual(layer.fields()[3].type(), QVariant.DateTime)

        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)))

        props = QgsVectorLayerTemporalProperties(enabled=False)
        props.setMode(QgsVectorLayerTemporalProperties.ModeFeatureDateTimeStartAndEndFromFields)
        props.setStartField('start_field')
        props.setEndField('end_field')
        self.assertFalse(props.createFilterString(layer, range))

        props.setIsActive(True)
        self.assertEqual(props.createFilterString(layer, range), '("start_field" <= make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL) AND ("end_field" >= make_datetime(2019,3,4,11,12,13) OR "end_field" IS NULL)')

        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeBeginning=False)
        self.assertEqual(props.createFilterString(layer, range), '("start_field" <= make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL) AND ("end_field" > make_datetime(2019,3,4,11,12,13) OR "end_field" IS NULL)')

        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeEnd=False)
        self.assertEqual(props.createFilterString(layer, range), '("start_field" < make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL) AND ("end_field" >= make_datetime(2019,3,4,11,12,13) OR "end_field" IS NULL)')

        props.setEndField('')
        self.assertEqual(props.createFilterString(layer, range), '"start_field" <= make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL')

        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeBeginning=False)
        self.assertEqual(props.createFilterString(layer, range), '"start_field" < make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL')

        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeEnd=False)
        self.assertEqual(props.createFilterString(layer, range), '"start_field" <= make_datetime(2020,5,6,8,9,10) OR "start_field" IS NULL')

        props.setStartField('')
        props.setEndField('end_field')
        self.assertEqual(props.createFilterString(layer, range), '"end_field" >= make_datetime(2019,3,4,11,12,13) OR "end_field" IS NULL')

        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeBeginning=False)
        self.assertEqual(props.createFilterString(layer, range), '"end_field" > make_datetime(2019,3,4,11,12,13) OR "end_field" IS NULL')

        range = QgsDateTimeRange(QDateTime(QDate(2019, 3, 4), QTime(11, 12, 13)), QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)), includeEnd=False)
        self.assertEqual(props.createFilterString(layer, range), '"end_field" >= make_datetime(2019,3,4,11,12,13) OR "end_field" IS NULL')


if __name__ == '__main__':
    unittest.main()
