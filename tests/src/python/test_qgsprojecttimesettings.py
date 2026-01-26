"""QGIS Unit tests for QgsProjectTimeSettings.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Samweli Mwakisambwe"
__date__ = "6/3/2020"
__copyright__ = "Copyright 2020, The QGIS Project"

from qgis.PyQt.QtCore import QDate, QDateTime, QTime
from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    QgsDateTimeRange,
    QgsProjectTimeSettings,
    QgsReadWriteContext,
    QgsUnitTypes,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

app = start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsProjectTimeSettings(QgisTestCase):

    def testTemporalRange(self):
        p = QgsProjectTimeSettings()
        self.assertTrue(p.temporalRange().isInfinite())
        spy = QSignalSpy(p.temporalRangeChanged)

        r = QgsDateTimeRange(
            QDateTime(QDate(2020, 1, 1), QTime(8, 0, 0)),
            QDateTime(QDate(2020, 12, 1), QTime(8, 0, 0)),
        )

        rc = QgsDateTimeRange(
            QDateTime(QDate(2020, 1, 1), QTime(8, 0, 0)),
            QDateTime(QDate(2020, 12, 1), QTime(8, 0, 0)),
        )

        p.setTemporalRange(r)
        self.assertEqual(p.temporalRange(), r)
        self.assertEqual(len(spy), 1)

        p.setTemporalRange(rc)
        self.assertEqual(len(spy), 1)

        p.reset()
        self.assertEqual(len(spy), 2)

    def testGettersSetters(self):
        p = QgsProjectTimeSettings()

        p.setTimeStep(4.8)
        self.assertEqual(p.timeStep(), 4.8)
        p.setTimeStepUnit(QgsUnitTypes.TemporalUnit.TemporalDecades)
        self.assertEqual(p.timeStepUnit(), QgsUnitTypes.TemporalUnit.TemporalDecades)
        p.setFramesPerSecond(90)
        self.assertEqual(p.framesPerSecond(), 90)
        p.setIsTemporalRangeCumulative(True)
        self.assertTrue(p.isTemporalRangeCumulative())

    def testReadWrite(self):
        p = QgsProjectTimeSettings()
        self.assertTrue(p.temporalRange().isInfinite())
        doc = QDomDocument("testdoc")
        elem = p.writeXml(doc, QgsReadWriteContext())

        p2 = QgsProjectTimeSettings()
        spy = QSignalSpy(p2.temporalRangeChanged)
        self.assertTrue(p2.readXml(elem, QgsReadWriteContext()))
        self.assertEqual(p2.temporalRange(), p.temporalRange())
        self.assertEqual(len(spy), 0)

        r = QgsDateTimeRange(
            QDateTime(QDate(2020, 1, 1), QTime(8, 0, 0)),
            QDateTime(QDate(2020, 12, 1), QTime(8, 0, 0)),
        )
        p.setTemporalRange(r)
        p.setTimeStep(4.8)
        p.setTimeStepUnit(QgsUnitTypes.TemporalUnit.TemporalDecades)
        p.setFramesPerSecond(90)
        p.setIsTemporalRangeCumulative(True)
        elem = p.writeXml(doc, QgsReadWriteContext())

        p2 = QgsProjectTimeSettings()
        spy = QSignalSpy(p2.temporalRangeChanged)
        self.assertTrue(p2.readXml(elem, QgsReadWriteContext()))
        self.assertEqual(p2.temporalRange(), r)
        self.assertEqual(len(spy), 1)
        self.assertEqual(p2.timeStep(), 4.8)
        self.assertEqual(p2.timeStepUnit(), QgsUnitTypes.TemporalUnit.TemporalDecades)
        self.assertEqual(p2.framesPerSecond(), 90)
        self.assertTrue(p.isTemporalRangeCumulative())


if __name__ == "__main__":
    unittest.main()
