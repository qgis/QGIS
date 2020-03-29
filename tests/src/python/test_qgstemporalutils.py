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
                       QgsDateTimeRange)

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


if __name__ == '__main__':
    unittest.main()
