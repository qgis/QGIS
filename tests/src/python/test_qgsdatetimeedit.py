"""QGIS Unit tests for QgsDateTimeEdit

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Denis Rouzaud"
__date__ = "2018-01-04"
__copyright__ = "Copyright 2017, The QGIS Project"

from qgis.PyQt.QtCore import QDate, QDateTime, Qt, QTime
from qgis.gui import QgsDateEdit, QgsDateTimeEdit, QgsTimeEdit
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()

DATE = QDateTime.fromString("2018-01-01 01:02:03", Qt.DateFormat.ISODate)
DATE_Z = QDateTime.fromString("2018-01-01 01:02:03Z", Qt.DateFormat.ISODate)


class TestQgsDateTimeEdit(QgisTestCase):

    def testSettersGetters(self):
        """test widget handling of null values"""
        w = QgsDateTimeEdit()
        w.setAllowNull(False)

        w.setDateTime(DATE)
        self.assertEqual(w.dateTime(), DATE)
        # date should remain when setting an invalid date
        w.setDateTime(QDateTime())
        self.assertEqual(w.dateTime(), DATE)

    def testSettersGetters_DATE_Z(self):
        """test widget handling with Z time spec"""
        w = QgsDateTimeEdit()
        w.setAllowNull(False)

        w.setDateTime(DATE_Z)
        self.assertEqual(w.dateTime(), DATE_Z)
        # date should remain when setting an invalid date
        w.setDateTime(QDateTime())
        self.assertEqual(w.dateTime(), DATE_Z)

    def testNullValueHandling(self):
        """test widget handling of null values"""
        w = QgsDateTimeEdit()
        w.setAllowNull(True)

        # date should be valid again when not allowing NULL values
        w.setDateTime(QDateTime())
        w.setAllowNull(False)
        self.assertTrue(w.dateTime().isValid())

        w.setAllowNull(True)
        w.setDateTime(QDateTime())
        self.assertFalse(w.dateTime().isValid())

        w.setAllowNull(False)
        self.assertTrue(w.dateTime().isValid())


class TestQgsDateEdit(QgisTestCase):

    def testSettersGetters(self):
        """test widget handling of null values"""
        w = QgsDateEdit()
        w.setAllowNull(False)

        w.setDate(DATE.date())
        self.assertEqual(w.date(), DATE.date())
        # date should remain when setting an invalid date
        w.setDate(QDate())
        self.assertEqual(w.date(), DATE.date())

    def testNullValueHandling(self):
        """test widget handling of null values"""
        w = QgsDateEdit()
        w.setAllowNull(True)

        # date should be valid again when not allowing NULL values
        w.setDate(QDate())
        w.setAllowNull(False)
        self.assertTrue(w.date().isValid())

        w.setAllowNull(True)
        w.setDate(QDate())
        self.assertFalse(w.date().isValid())

        w.setAllowNull(False)
        self.assertTrue(w.date().isValid())


class TestQgsTimeEdit(QgisTestCase):

    def testSettersGetters(self):
        """test widget handling of null values"""
        w = QgsTimeEdit()
        w.setAllowNull(False)

        w.setTime(DATE.time())
        self.assertEqual(w.time(), DATE.time())
        # time should remain when setting an invalid time
        w.setTime(QTime())
        self.assertEqual(w.time(), DATE.time())

    def testNullValueHandling(self):
        """test widget handling of null values"""
        w = QgsTimeEdit()
        w.setAllowNull(True)

        # time should be valid again when not allowing NULL values
        w.setTime(QTime())
        w.setAllowNull(False)
        self.assertTrue(w.time().isValid())

        w.setAllowNull(True)
        w.setTime(QTime())
        self.assertFalse(w.time().isValid())

        w.setAllowNull(False)
        self.assertTrue(w.time().isValid())


if __name__ == "__main__":
    unittest.main()
