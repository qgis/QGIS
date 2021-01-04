# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsTemporalUtils.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '13/3/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import qgis  # NOQA

from qgis.core import (QgsProject,
                       QgsTemporalUtils,
                       QgsRasterLayer,
                       QgsDateTimeRange,
                       QgsDateTimeRange,
                       QgsInterval,
                       QgsUnitTypes)

from qgis.PyQt.QtCore import (QDate,
                              QTime,
                              QDateTime,
                              Qt)

from qgis.testing import start_app, unittest
from utilities import (unitTestDataPath)

app = start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsTemporalUtils(unittest.TestCase):

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


if __name__ == '__main__':
    unittest.main()
