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

from qgis.gui import QgsDateTimeEdit
from qgis.PyQt.QtCore import Qt, QDateTime
from qgis.testing import start_app, unittest

start_app()

DATE = QDateTime.fromString('2018-01-01 01:02:03', Qt.ISODate)


class TestQgsDateTimeEdit(unittest.TestCase):

    def testSettersGetters(self):
        """ test widget handling of null values """
        w = qgis.gui.QgsDateTimeEdit()
        w.setAllowNull(False)

        w.setDateTime(DATE)
        self.assertEqual(DATE, w.dateTime())
        # date should remain when setting an invalid date
        w.setDateTime(QDateTime())
        self.assertEqual(DATE, w.dateTime())

    def testNullValueHandling(self):
        """ test widget handling of null values """
        w = qgis.gui.QgsDateTimeEdit()
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


if __name__ == '__main__':
    unittest.main()
