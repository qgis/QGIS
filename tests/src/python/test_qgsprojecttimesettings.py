# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsProjectTimeSettings.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Samweli Mwakisambwe'
__date__ = '6/3/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import qgis  # NOQA

from qgis.core import (QgsProject,
                       QgsProjectTimeSettings,
                       QgsReadWriteContext,
                       QgsDateTimeRange )

from qgis.PyQt.QtCore import (QDate,
                             QTime,
                             QDateTime)

from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtXml import QDomDocument, QDomElement
from qgis.testing import start_app, unittest
from utilities import (unitTestDataPath)

app = start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsProjectTimeSettings(unittest.TestCase):

    def testTemporalRange(self):
        p = QgsProjectTimeSettings()
        self.assertTrue(p.temporalRange().isInfinite()))

        spy = QSignalSpy(p.temporalRangeChanged)
        p.setTemporalRange( QgsDateTimeRange () )
        self.assertEqual(len(spy), 0)
        p.setTemporalRange(False)
        self.assertEqual(len(spy), 0)

        r = QgsDateTimeRange(
        QDateTime(QDate(2020, 1, 1), QTime(8, 0, 0)),
        QDateTime(QDate(2020, 12, 1), QTime(8, 0, 0))
        )

        p.setTemporalRange( r )
        self.assertEqual(p.temporaRange(), r )
        self.assertEqual(len(spy), 1)

        p.reset()
        self.assertEqual(len(spy), 2)

    def testReadWrite(self):
        p = QgsProjectTimeSettings()
        self.assertFalse(p.temporalRange())
        doc = QDomDocument("testdoc")
        elem = p.writeXml(doc, QgsReadWriteContext())

        p2 = QgsProjectTimeSettings()
        spy = QSignalSpy(p2.temporalRangeChanged)
        self.assertTrue(p2.readXml(elem, QgsReadWriteContext()))
        self.assertFalse(p2.temporalRange())
        self.assertEqual(len(spy), 0)

        r = QgsDateTimeRange(
        QDateTime(QDate(2020, 1, 1), QTime(8, 0, 0)),
        QDateTime(QDate(2020, 12, 1), QTime(8, 0, 0))
        )
        p.setTemporalRange(r)
        elem = p.writeXml(doc, QgsReadWriteContext())

        p2 = QgsProjectTimeSettings()
        spy = QSignalSpy(p2.temporalRangeChanged)
        self.assertTrue(p2.readXml(elem, QgsReadWriteContext()))
        self.assertEqual(p2.temporalRange(), r)
        self.assertEqual(len(spy), 1)

if __name__ == '__main__':
    unittest.main()
