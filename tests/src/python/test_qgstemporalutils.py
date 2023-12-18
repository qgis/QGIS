"""QGIS Unit tests for QgsTemporalUtils.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '13/3/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

from qgis.PyQt.QtCore import QDate, QDateTime, Qt, QTime

from qgis.core import (
    QgsDateTimeRange,
    QgsFeature,
    QgsField,
    QgsGeometry,
    QgsInterval,
    QgsMeshLayer,
    QgsTemporalController,
    QgsTemporalNavigationObject,
    QgsPointXY,
    QgsProject,
    QgsRasterLayer,
    QgsRasterLayerTemporalProperties,
    QgsTemporalUtils,
    QgsUnitTypes,
    QgsVectorLayer,
    QgsVectorLayerTemporalProperties,
)
from qgis.gui import (
    QgsMapCanvas,
)
import unittest
import os
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

app = start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsTemporalUtils(QgisTestCase):

    def testTemporalRangeForProject(self):
        p = QgsProject()
        r1 = QgsRasterLayer('', '', 'wms')
        r2 = QgsRasterLayer('', '', 'wms')
        r3 = QgsRasterLayer('', '', 'wms')
        r1.temporalProperties().setIsActive(True)
        r1.temporalProperties().setFixedTemporalRange(QgsDateTimeRange(QDateTime(QDate(2020, 1, 1), QTime(), Qt.UTC),
                                                                       QDateTime(QDate(2020, 3, 31), QTime(), Qt.UTC)))
        r2.temporalProperties().setIsActive(True)
        r2.temporalProperties().setFixedTemporalRange(QgsDateTimeRange(QDateTime(QDate(2020, 4, 1), QTime(), Qt.UTC),
                                                                       QDateTime(QDate(2020, 7, 31), QTime(), Qt.UTC)))
        r3.temporalProperties().setIsActive(True)
        r3.temporalProperties().setFixedTemporalRange(QgsDateTimeRange(QDateTime(QDate(2019, 1, 1), QTime(), Qt.UTC),
                                                                       QDateTime(QDate(2020, 2, 28), QTime(), Qt.UTC)))

        p.addMapLayers([r1, r2, r3])

        range = QgsTemporalUtils.calculateTemporalRangeForProject(p)
        self.assertEqual(range.begin(), QDateTime(QDate(2019, 1, 1), QTime(), Qt.UTC))
        self.assertEqual(range.end(), QDateTime(QDate(2020, 7, 31), QTime(), Qt.UTC))

    def testUsedTemporalRangesForProject(self):
        p = QgsProject()
        r1 = QgsRasterLayer('', '', 'wms')
        r2 = QgsRasterLayer('', '', 'wms')
        r3 = QgsRasterLayer('', '', 'wms')
        r4 = QgsRasterLayer('', '', 'wms')
        r1.temporalProperties().setIsActive(True)
        r1.temporalProperties().setMode(QgsRasterLayerTemporalProperties.ModeTemporalRangeFromDataProvider)
        r1.dataProvider().temporalCapabilities().setAvailableTemporalRange(QgsDateTimeRange(QDateTime(QDate(2020, 1, 1), QTime(), Qt.UTC),
                                                                                            QDateTime(QDate(2020, 3, 31), QTime(), Qt.UTC)))
        r2.temporalProperties().setIsActive(True)
        r2.temporalProperties().setMode(QgsRasterLayerTemporalProperties.ModeTemporalRangeFromDataProvider)
        r2.dataProvider().temporalCapabilities().setAllAvailableTemporalRanges([QgsDateTimeRange(QDateTime(QDate(2020, 4, 1), QTime(), Qt.UTC),
                                                                                                 QDateTime(QDate(2020, 7, 31), QTime(), Qt.UTC))])
        r3.temporalProperties().setIsActive(True)
        r3.temporalProperties().setMode(QgsRasterLayerTemporalProperties.ModeTemporalRangeFromDataProvider)
        r3.dataProvider().temporalCapabilities().setAllAvailableTemporalRanges([QgsDateTimeRange(QDateTime(QDate(2019, 1, 1), QTime(), Qt.UTC),
                                                                                                 QDateTime(QDate(2020, 2, 28), QTime(), Qt.UTC))])
        r4.temporalProperties().setIsActive(True)
        r4.temporalProperties().setMode(QgsRasterLayerTemporalProperties.ModeTemporalRangeFromDataProvider)
        r4.dataProvider().temporalCapabilities().setAllAvailableTemporalRanges([QgsDateTimeRange(QDateTime(QDate(2021, 1, 1), QTime(), Qt.UTC),
                                                                                                 QDateTime(QDate(2021, 2, 28), QTime(), Qt.UTC))])

        p.addMapLayers([r1, r2, r3, r4])

        ranges = QgsTemporalUtils.usedTemporalRangesForProject(p)
        self.assertEqual(ranges, [QgsDateTimeRange(QDateTime(QDate(2019, 1, 1), QTime(), Qt.UTC),
                                                   QDateTime(QDate(2020, 3, 31), QTime(), Qt.UTC)),
                                  QgsDateTimeRange(QDateTime(QDate(2020, 4, 1), QTime(), Qt.UTC),
                                                   QDateTime(QDate(2020, 7, 31), QTime(), Qt.UTC)),
                                  QgsDateTimeRange(QDateTime(QDate(2021, 1, 1), QTime(), Qt.UTC),
                                                   QDateTime(QDate(2021, 2, 28), QTime(), Qt.UTC))])

    def testFrameTimeCalculation(self):
        expected = {QgsUnitTypes.TemporalMilliseconds: QDateTime(QDate(2021, 1, 1), QTime(12, 0, 0, 10), Qt.UTC),
                    QgsUnitTypes.TemporalSeconds: QDateTime(QDate(2021, 1, 1), QTime(12, 0, 10, 0), Qt.UTC),
                    QgsUnitTypes.TemporalMinutes: QDateTime(QDate(2021, 1, 1), QTime(12, 10, 0, 0), Qt.UTC),
                    QgsUnitTypes.TemporalHours: QDateTime(QDate(2021, 1, 1), QTime(22, 0, 0, 0), Qt.UTC),
                    QgsUnitTypes.TemporalDays: QDateTime(QDate(2021, 1, 11), QTime(12, 0, 0, 0), Qt.UTC),
                    QgsUnitTypes.TemporalWeeks: QDateTime(QDate(2021, 3, 12), QTime(12, 0, 0, 0), Qt.UTC),
                    QgsUnitTypes.TemporalMonths: QDateTime(QDate(2021, 11, 1), QTime(12, 0, 0, 0), Qt.UTC),
                    QgsUnitTypes.TemporalYears: QDateTime(QDate(2031, 1, 1), QTime(12, 0, 0, 0), Qt.UTC),
                    QgsUnitTypes.TemporalDecades: QDateTime(QDate(2121, 1, 1), QTime(12, 0, 0, 0), Qt.UTC),
                    QgsUnitTypes.TemporalCenturies: QDateTime(QDate(3021, 1, 1), QTime(12, 0, 0, 0), Qt.UTC)
                    }

        for unit in list(expected.keys()):
            f = QgsTemporalUtils.calculateFrameTime(QDateTime(QDate(2021, 1, 1), QTime(12, 0, 0, 0), Qt.UTC),
                                                    1,
                                                    QgsInterval(10, unit))
            self.assertEqual(f, expected[unit])

        expected2 = {QgsUnitTypes.TemporalMilliseconds: QDateTime(QDate(2021, 1, 1), QTime(12, 0, 0, 10), Qt.UTC),
                     QgsUnitTypes.TemporalSeconds: QDateTime(QDate(2021, 1, 1), QTime(12, 0, 10, 500), Qt.UTC),
                     QgsUnitTypes.TemporalMinutes: QDateTime(QDate(2021, 1, 1), QTime(12, 10, 30, 0), Qt.UTC),
                     QgsUnitTypes.TemporalHours: QDateTime(QDate(2021, 1, 1), QTime(22, 30, 0, 0), Qt.UTC),
                     QgsUnitTypes.TemporalDays: QDateTime(QDate(2021, 1, 12), QTime(0, 0, 0), Qt.UTC),
                     QgsUnitTypes.TemporalWeeks: QDateTime(QDate(2021, 3, 16), QTime(0, 0, 0), Qt.UTC),
                     QgsUnitTypes.TemporalMonths: QDateTime(QDate(2021, 11, 12), QTime(12, 0, 0, 0), Qt.UTC),
                     QgsUnitTypes.TemporalYears: QDateTime(QDate(2031, 7, 3), QTime(15, 0, 0, 0), Qt.UTC),
                     QgsUnitTypes.TemporalDecades: QDateTime(QDate(2126, 1, 2), QTime(18, 0, 0, 0), Qt.UTC),
                     QgsUnitTypes.TemporalCenturies: QDateTime(QDate(3071, 1, 10), QTime(0, 0, 0, 0), Qt.UTC)
                     }

        for unit in list(expected2.keys()):
            f = QgsTemporalUtils.calculateFrameTime(QDateTime(QDate(2021, 1, 1), QTime(12, 0, 0, 0), Qt.UTC),
                                                    1,
                                                    QgsInterval(10.5, unit))
            self.assertEqual(f, expected2[unit])

        # frame number > 1
        expected2a = {QgsUnitTypes.TemporalMilliseconds: QDateTime(QDate(2021, 1, 1), QTime(12, 0, 0, 31), Qt.UTC),
                      QgsUnitTypes.TemporalSeconds: QDateTime(QDate(2021, 1, 1), QTime(12, 0, 31, 500), Qt.UTC),
                      QgsUnitTypes.TemporalMinutes: QDateTime(QDate(2021, 1, 1), QTime(12, 31, 30, 0), Qt.UTC),
                      QgsUnitTypes.TemporalHours: QDateTime(QDate(2021, 1, 2), QTime(19, 30, 0, 0), Qt.UTC),
                      QgsUnitTypes.TemporalDays: QDateTime(QDate(2021, 2, 2), QTime(0, 0, 0), Qt.UTC),
                      QgsUnitTypes.TemporalWeeks: QDateTime(QDate(2021, 8, 10), QTime(0, 0, 0), Qt.UTC),
                      QgsUnitTypes.TemporalMonths: QDateTime(QDate(2023, 8, 4), QTime(12, 0, 0, 0), Qt.UTC),
                      QgsUnitTypes.TemporalYears: QDateTime(QDate(2052, 7, 2), QTime(21, 0, 0, 0), Qt.UTC),
                      QgsUnitTypes.TemporalDecades: QDateTime(QDate(2336, 1, 5), QTime(6, 0, 0, 0), Qt.UTC),
                      QgsUnitTypes.TemporalCenturies: QDateTime(QDate(5171, 1, 26), QTime(0, 0, 0, 0), Qt.UTC)
                      }

        for unit in list(expected2.keys()):
            f = QgsTemporalUtils.calculateFrameTime(QDateTime(QDate(2021, 1, 1), QTime(12, 0, 0, 0), Qt.UTC),
                                                    3,
                                                    QgsInterval(10.5, unit))
            self.assertEqual(f, expected2a[unit])

        expected3 = {QgsUnitTypes.TemporalMilliseconds: QDateTime(QDate(2021, 1, 1), QTime(12, 0, 0, 0), Qt.UTC),
                     QgsUnitTypes.TemporalSeconds: QDateTime(QDate(2021, 1, 1), QTime(12, 0, 0, 200), Qt.UTC),
                     QgsUnitTypes.TemporalMinutes: QDateTime(QDate(2021, 1, 1), QTime(12, 0, 12, 0), Qt.UTC),
                     QgsUnitTypes.TemporalHours: QDateTime(QDate(2021, 1, 1), QTime(12, 12, 0, 0), Qt.UTC),
                     QgsUnitTypes.TemporalDays: QDateTime(QDate(2021, 1, 1), QTime(16, 48, 0), Qt.UTC),
                     QgsUnitTypes.TemporalWeeks: QDateTime(QDate(2021, 1, 2), QTime(21, 36, 0), Qt.UTC),
                     QgsUnitTypes.TemporalMonths: QDateTime(QDate(2021, 1, 7), QTime(12, 0, 0, 0), Qt.UTC),
                     QgsUnitTypes.TemporalYears: QDateTime(QDate(2021, 3, 15), QTime(13, 12, 0, 0), Qt.UTC),
                     QgsUnitTypes.TemporalDecades: QDateTime(QDate(2023, 1, 2), QTime(0, 0, 0, 0), Qt.UTC),
                     QgsUnitTypes.TemporalCenturies: QDateTime(QDate(2041, 1, 1), QTime(12, 0, 0, 0), Qt.UTC)
                     }

        for unit in list(expected3.keys()):
            f = QgsTemporalUtils.calculateFrameTime(QDateTime(QDate(2021, 1, 1), QTime(12, 0, 0, 0), Qt.UTC),
                                                    1,
                                                    QgsInterval(0.2, unit))
            self.assertEqual(f, expected3[unit])

        expected3a = {QgsUnitTypes.TemporalMilliseconds: QDateTime(QDate(2021, 1, 1), QTime(12, 0, 0, 0), Qt.UTC),
                      QgsUnitTypes.TemporalSeconds: QDateTime(QDate(2021, 1, 1), QTime(12, 0, 0, 600), Qt.UTC),
                      QgsUnitTypes.TemporalMinutes: QDateTime(QDate(2021, 1, 1), QTime(12, 0, 36, 0), Qt.UTC),
                      QgsUnitTypes.TemporalHours: QDateTime(QDate(2021, 1, 1), QTime(12, 36, 0, 0), Qt.UTC),
                      QgsUnitTypes.TemporalDays: QDateTime(QDate(2021, 1, 2), QTime(2, 24, 0), Qt.UTC),
                      QgsUnitTypes.TemporalWeeks: QDateTime(QDate(2021, 1, 5), QTime(16, 48, 0), Qt.UTC),
                      QgsUnitTypes.TemporalMonths: QDateTime(QDate(2021, 1, 19), QTime(12, 0, 0, 0), Qt.UTC),
                      QgsUnitTypes.TemporalYears: QDateTime(QDate(2021, 8, 8), QTime(15, 36, 0, 0), Qt.UTC),
                      QgsUnitTypes.TemporalDecades: QDateTime(QDate(2027, 1, 2), QTime(0, 0, 0, 0), Qt.UTC),
                      QgsUnitTypes.TemporalCenturies: QDateTime(QDate(2081, 1, 1), QTime(12, 0, 0, 0), Qt.UTC)
                      }

        for unit in list(expected3.keys()):
            f = QgsTemporalUtils.calculateFrameTime(QDateTime(QDate(2021, 1, 1), QTime(12, 0, 0, 0), Qt.UTC),
                                                    3,
                                                    QgsInterval(0.2, unit))
            self.assertEqual(f, expected3a[unit])

    def testCalculateDateTimesUsingDuration(self):
        # invalid duration string
        vals, ok, exceeded = QgsTemporalUtils.calculateDateTimesUsingDuration(
            QDateTime(QDate(2021, 3, 23), QTime(0, 0, 0)),
            QDateTime(QDate(2021, 3, 24), QTime(12, 0, 0)), 'xT12H')
        self.assertFalse(ok)
        # null duration string
        vals, ok, exceeded = QgsTemporalUtils.calculateDateTimesUsingDuration(
            QDateTime(QDate(2021, 3, 23), QTime(0, 0, 0)),
            QDateTime(QDate(2021, 3, 24), QTime(12, 0, 0)), '')
        self.assertFalse(ok)

        vals, ok, exceeded = QgsTemporalUtils.calculateDateTimesUsingDuration(
            QDateTime(QDate(2021, 3, 23), QTime(0, 0, 0)),
            QDateTime(QDate(2021, 3, 24), QTime(12, 0, 0)), 'P')
        self.assertFalse(ok)

        # valid durations
        vals, ok, exceeded = QgsTemporalUtils.calculateDateTimesUsingDuration(
            QDateTime(QDate(2021, 3, 23), QTime(0, 0, 0)),
            QDateTime(QDate(2021, 3, 24), QTime(12, 0, 0)), 'PT12H')
        self.assertEqual(vals, [QDateTime(2021, 3, 23, 0, 0),
                                QDateTime(2021, 3, 23, 12, 0),
                                QDateTime(2021, 3, 24, 0, 0),
                                QDateTime(2021, 3, 24, 12, 0)])
        self.assertTrue(ok)
        self.assertFalse(exceeded)

        vals, ok, exceeded = QgsTemporalUtils.calculateDateTimesUsingDuration(
            QDateTime(QDate(2021, 3, 23), QTime(0, 0, 0)),
            QDateTime(QDate(2021, 3, 24), QTime(12, 0, 0)), 'PT12H', maxValues=2)
        self.assertEqual(vals, [QDateTime(2021, 3, 23, 0, 0),
                                QDateTime(2021, 3, 23, 12, 0),
                                QDateTime(2021, 3, 24, 0, 0)])
        self.assertTrue(ok)
        self.assertTrue(exceeded)

        vals, ok, exceeded = QgsTemporalUtils.calculateDateTimesUsingDuration(
            QDateTime(QDate(2021, 3, 23), QTime(0, 0, 0)),
            QDateTime(QDate(2021, 3, 24), QTime(12, 0, 0)), 'PT10H2M5S')
        self.assertEqual(vals, [QDateTime(2021, 3, 23, 0, 0), QDateTime(2021, 3, 23, 10, 2, 5),
                                QDateTime(2021, 3, 23, 20, 4, 10), QDateTime(2021, 3, 24, 6, 6, 15)])
        self.assertTrue(ok)
        self.assertFalse(exceeded)

        vals, ok, exceeded = QgsTemporalUtils.calculateDateTimesUsingDuration(
            QDateTime(QDate(2010, 3, 23), QTime(0, 0, 0)),
            QDateTime(QDate(2021, 5, 24), QTime(12, 0, 0)), 'P2Y')
        self.assertEqual(vals,
                         [QDateTime(2010, 3, 23, 0, 0), QDateTime(2012, 3, 23, 0, 0), QDateTime(2014, 3, 23, 0, 0),
                          QDateTime(2016, 3, 23, 0, 0), QDateTime(2018, 3, 23, 0, 0), QDateTime(2020, 3, 23, 0, 0)])
        self.assertTrue(ok)
        self.assertFalse(exceeded)

        vals, ok, exceeded = QgsTemporalUtils.calculateDateTimesUsingDuration(
            QDateTime(QDate(2020, 3, 23), QTime(0, 0, 0)),
            QDateTime(QDate(2021, 5, 24), QTime(12, 0, 0)), 'P2M')
        self.assertEqual(vals,
                         [QDateTime(2020, 3, 23, 0, 0), QDateTime(2020, 5, 23, 0, 0), QDateTime(2020, 7, 23, 0, 0),
                          QDateTime(2020, 9, 23, 0, 0), QDateTime(2020, 11, 23, 0, 0), QDateTime(2021, 1, 23, 0, 0),
                          QDateTime(2021, 3, 23, 0, 0), QDateTime(2021, 5, 23, 0, 0)])
        self.assertTrue(ok)
        self.assertFalse(exceeded)

        vals, ok, exceeded = QgsTemporalUtils.calculateDateTimesUsingDuration(
            QDateTime(QDate(2021, 3, 23), QTime(0, 0, 0)),
            QDateTime(QDate(2021, 5, 24), QTime(12, 0, 0)), 'P2W')
        self.assertEqual(vals, [QDateTime(2021, 3, 23, 0, 0), QDateTime(2021, 4, 6, 0, 0), QDateTime(2021, 4, 20, 0, 0),
                                QDateTime(2021, 5, 4, 0, 0), QDateTime(2021, 5, 18, 0, 0)])
        self.assertTrue(ok)
        self.assertFalse(exceeded)

        vals, ok, exceeded = QgsTemporalUtils.calculateDateTimesUsingDuration(
            QDateTime(QDate(2021, 3, 23), QTime(0, 0, 0)),
            QDateTime(QDate(2021, 4, 7), QTime(12, 0, 0)), 'P2D')
        self.assertEqual(vals,
                         [QDateTime(2021, 3, 23, 0, 0), QDateTime(2021, 3, 25, 0, 0), QDateTime(2021, 3, 27, 0, 0),
                          QDateTime(2021, 3, 29, 0, 0), QDateTime(2021, 3, 31, 0, 0), QDateTime(2021, 4, 2, 0, 0),
                          QDateTime(2021, 4, 4, 0, 0), QDateTime(2021, 4, 6, 0, 0)])
        self.assertTrue(ok)
        self.assertFalse(exceeded)

        # complex mix
        vals, ok, exceeded = QgsTemporalUtils.calculateDateTimesUsingDuration(
            QDateTime(QDate(2010, 3, 23), QTime(0, 0, 0)),
            QDateTime(QDate(2021, 5, 24), QTime(12, 0, 0)), 'P2Y1M3W4DT5H10M22S')
        self.assertEqual(vals, [QDateTime(2010, 3, 23, 0, 0), QDateTime(2012, 5, 18, 5, 10, 22),
                                QDateTime(2014, 7, 13, 10, 20, 44), QDateTime(2016, 9, 7, 15, 31, 6),
                                QDateTime(2018, 11, 1, 20, 41, 28), QDateTime(2020, 12, 27, 1, 51, 50)])
        self.assertTrue(ok)
        self.assertFalse(exceeded)

    def testCalculateDateTimesFromISO8601(self):
        # invalid duration string
        vals, ok, exceeded = QgsTemporalUtils.calculateDateTimesFromISO8601('x')
        self.assertFalse(ok)

        vals, ok, exceeded = QgsTemporalUtils.calculateDateTimesFromISO8601(
            'a-03-23T00:00:00Z/2021-03-24T12:00:00Z/PT12H')
        self.assertFalse(ok)
        vals, ok, exceeded = QgsTemporalUtils.calculateDateTimesFromISO8601(
            '2021-03-23T00:00:00Z/b-03-24T12:00:00Z/PT12H')
        self.assertFalse(ok)
        vals, ok, exceeded = QgsTemporalUtils.calculateDateTimesFromISO8601(
            '2021-03-23T00:00:00Z/2021-03-24T12:00:00Z/xc')
        self.assertFalse(ok)

        vals, ok, exceeded = QgsTemporalUtils.calculateDateTimesFromISO8601(
            '2021-03-23T00:00:00Z/2021-03-24T12:00:00Z/PT12H')
        self.assertEqual(vals, [QDateTime(2021, 3, 23, 0, 0, 0, 0, Qt.TimeSpec(1)),
                                QDateTime(2021, 3, 23, 12, 0, 0, 0, Qt.TimeSpec(1)),
                                QDateTime(2021, 3, 24, 0, 0, 0, 0, Qt.TimeSpec(1)),
                                QDateTime(2021, 3, 24, 12, 0, 0, 0, Qt.TimeSpec(1))])
        self.assertTrue(ok)
        self.assertFalse(exceeded)

    def testVectorLayerFrameCount(self):
        """
        Testing the actual framecount here, mostly with instantanous events
        which fall exactly on the end of a timestep.
        Besides we test if the temporalExtents() of the navigator includes or
        excludes te upper limit (because that is used in counting the frames)
        """
        tl = createTemporalLayerWithThreePoints()
        self.assertTrue(tl.isValid())

        project = QgsProject.instance()
        project.clear()
        project.addMapLayer(tl)

        props = tl.temporalProperties()
        props.setIsActive(True)

        canvas = QgsMapCanvas()
        canvas.setLayers([tl])

        navigator = QgsTemporalNavigationObject()
        frame_duration = QgsInterval()
        frame_duration.setSeconds(24 * 60 * 60)
        navigator.setFrameDuration(frame_duration)
        canvas.setTemporalController(navigator)

        # 3 features as instantanious times
        # which exactly fall on the endtimes of the timesteps
        # because of that we will need 3(!) steps because the end of the
        # filter is NOT inclusive

        # map range              1----2----3----4--------days
        # map range              [----)------------------days
        # map range              -----[----)-------------days
        # map range              ----------[----)--------days

        # map range              1----2----3----4--------days
        # features               .    .    .
        # feature range          [---------]-----------------

        props.setMode(QgsVectorLayerTemporalProperties.ModeFeatureDateTimeInstantFromField)
        props.setStartField('start_field')
        dt_range = QgsTemporalUtils.calculateTemporalRangeForProject(project)
        navigator.setTemporalExtents(dt_range)
        self.assertEqual(navigator.totalFrameCount(), 3)
        self.assertEqual(navigator.temporalExtents(),
                         QgsDateTimeRange(QDateTime(2020, 1, 1, 0, 0), QDateTime(2020, 1, 3, 0, 0), True, True))

        # same three features, but using the end_field too

        # map range              1----2----3----4--------days
        # feature                [----)----------------------
        # feature                -----[----)-----------------
        # feature                ----------[----)------------
        # feature range          [--------------)------------
        props.setMode(QgsVectorLayerTemporalProperties.ModeFeatureDateTimeStartAndEndFromFields)
        props.setStartField('start_field')
        props.setEndField('end_field')

        dt_range = QgsTemporalUtils.calculateTemporalRangeForProject(project)
        navigator.setTemporalExtents(dt_range)
        self.assertEqual(navigator.totalFrameCount(), 3)
        self.assertEqual(navigator.temporalExtents(),
                         QgsDateTimeRange(QDateTime(2020, 1, 1, 0, 0), QDateTime(2020, 1, 4, 0, 0), True, False))

        # Moving the start_field last feature 12 hours back, so we should have enough on 2 timesteps:

        # map range              1----2----3----4--------days
        # features               .    .  .
        # feature range          [-------)-------------------
        tl.startEditing()
        tl.changeAttributeValue(3, 0, QDateTime(2020, 1, 2, 12, 0))
        tl.commitChanges()
        props.setMode(QgsVectorLayerTemporalProperties.ModeFeatureDateTimeInstantFromField)
        props.setStartField('start_field')
        dt_range = QgsTemporalUtils.calculateTemporalRangeForProject(project)
        navigator.setTemporalExtents(dt_range)
        self.assertEqual(navigator.totalFrameCount(), 2)
        self.assertEqual(navigator.temporalExtents(),
                         QgsDateTimeRange(QDateTime(2020, 1, 1, 0, 0), QDateTime(2020, 1, 2, 12, 0), True, True))

    def testMeshLayerFrameCount(self):
        tl = createTemporalMeshLayer()
        """
        ncinfo -v time .../temporal/ugrid3timesteps.nc
        <class 'netCDF4._netCDF4.Variable'>
        float64 time(time)
            units: hours since 2020-01-01 00:00:00.0
            calendar: gregorian
            standard_name: time
            axis: T
        unlimited dimensions: time
        current shape = (3,)
        filling on, default _FillValue of 9.969209968386869e+36 used
        """
        self.assertTrue(tl.isValid())

        project = QgsProject.instance()
        project.clear()
        project.addMapLayer(tl)

        props = tl.temporalProperties()
        props.setIsActive(True)

        canvas = QgsMapCanvas()
        canvas.setLayers([tl])

        navigator = QgsTemporalNavigationObject()
        frame_duration = QgsInterval()
        frame_duration.setSeconds(24 * 60 * 60)
        navigator.setFrameDuration(frame_duration)
        canvas.setTemporalController(navigator)

        navigator.setNavigationMode(QgsTemporalNavigationObject.Animated)  # will show controller
        navigator.rewindToStart()

        dt_range = QgsTemporalUtils.calculateTemporalRangeForProject(project)
        navigator.setTemporalExtents(dt_range)
        self.assertEqual(navigator.totalFrameCount(), 3)
        self.assertEqual(navigator.temporalExtents(),
                         #  <QgsDateTimeRange:[2020-01-01T00:00:00Z, 2020-01-03T00:00:00Z]>
                         QgsDateTimeRange(QDateTime(2020, 1, 1, 0, 0, 0, 0, Qt.TimeSpec(Qt.UTC)), QDateTime(2020, 1, 3, 0, 0, 0, 0, Qt.TimeSpec(Qt.UTC)), True, True))


def createTemporalLayerWithThreePoints():
    l = QgsVectorLayer('Point?crs=EPSG:4326&field=start_field:datetime(0,0)&field=end_field:datetime(0,0)&field=id:integer(10,0)', 'spatial_test_vector_layer', "memory")
    data = [
        [3.31355770, 47.97473506, '2020-01-01 00:00:00.0', '2020-01-02 00:00:00.0', 1],
        [3.31368860, 47.97473717, '2020-01-02 00:00:00.0', '2020-01-03 00:00:00.0', 2],
        [3.31382443, 47.97473365, '2020-01-03 00:00:00.0', '2020-01-04 00:00:00.0', 3],
    ]
    for d in data:
        f = QgsFeature()
        f.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(d[0], d[1])))
        f.setAttributes([QDateTime.fromString(d[2], Qt.ISODate), QDateTime.fromString(d[3], Qt.ISODate), d[4]])
        l.dataProvider().addFeature(f)
    l.commitChanges()
    return l


def createTemporalMeshLayer():
    p = os.path.join(unitTestDataPath(), 'temporal/ugrid3timesteps.nc')
    # NOTE!! using NETCDF as type here will NOT return the right framecount
    # l =QgsMeshLayer(f'NETCDF:"{p}":mesh2d', 'spatial_test_mesh_layer', 'mdal')
    # you really need
    l = QgsMeshLayer(f'Ugrid:"{p}":mesh2d', 'spatial_test_mesh_layer', 'mdal')
    return l


if __name__ == '__main__':
    unittest.main()
