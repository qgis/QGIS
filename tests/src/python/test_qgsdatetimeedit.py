# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsDateTimeEdit

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Denis Rouzaud'
__date__ = '2018-01-04'
__copyright__ = 'Copyright 2017, The QGIS Project'

import qgis  # NOQA
from qgis.PyQt.QtCore import Qt, QDateTime, QDate, QTime
from qgis.gui import QgsDateTimeEdit, QgsDateEdit, QgsTimeEdit
from qgis.testing import start_app, unittest

start_app()

DATE = QDateTime.fromString('2018-01-01 01:02:03', Qt.ISODate)


class TestQgsDateTimeEdit(unittest.TestCase):

    def testSettersGetters(self):
        """ test widget handling of null values """
        w = QgsDateTimeEdit()
        w.setAllowNull(False)

        w.setDateTime(DATE)
        self.assertEqual(w.dateTime(), DATE)
        # date should remain when setting an invalid date
        w.setDateTime(QDateTime())
        self.assertEqual(w.dateTime(), DATE)

    def testNullValueHandling(self):
        """ test widget handling of null values """
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


class TestQgsDateEdit(unittest.TestCase):

    def testSettersGetters(self):
        """ test widget handling of null values """
        w = QgsDateEdit()
        w.setAllowNull(False)

        w.setDate(DATE.date())
        self.assertEqual(w.date(), DATE.date())
        # date should remain when setting an invalid date
        w.setDate(QDate())
        self.assertEqual(w.date(), DATE.date())

    def testNullValueHandling(self):
        """ test widget handling of null values """
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


class TestQgsTimeEdit(unittest.TestCase):

    def testSettersGetters(self):
        """ test widget handling of null values """
        w = QgsTimeEdit()
        w.setAllowNull(False)

        w.setTime(DATE.time())
        self.assertEqual(w.time(), DATE.time())
        # time should remain when setting an invalid time
        w.setTime(QTime())
        self.assertEqual(w.time(), DATE.time())

    def testNullValueHandling(self):
        """ test widget handling of null values """
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


if __name__ == '__main__':
    unittest.main()
