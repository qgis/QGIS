"""QGIS Unit tests for time zone widgets

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import math
import unittest

from qgis.gui import (
    QgsDateTimeWithTimeZoneWidget,
    QgsTimeZoneComboBox,
    QgsTimeZoneItemModel,
    QgsTimeZoneSortFilterModel,
    QgsTimeZoneWidget,
)
from qgis.PyQt.QtCore import QDate, QDateTime, Qt, QTime, QTimeZone
from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtWidgets import QComboBox
from qgis.testing import QgisTestCase, start_app

start_app()


class TestQgsTimeZoneItemModel(QgisTestCase):
    def test_model(self):
        model = QgsTimeZoneItemModel()
        self.assertEqual(model.rowCount(), len(QTimeZone.availableTimeZoneIds()))
        self.assertEqual(model.columnCount(), 1)

        utc_index = model.indexFromTimeZone(QTimeZone(b"UTC"))
        self.assertTrue(utc_index.isValid())
        self.assertEqual(
            model.data(utc_index, Qt.ItemDataRole.DisplayRole),
            "UTC+00:00: UTC (Coordinated Universal Time)",
        )
        self.assertEqual(
            model.data(utc_index, QgsTimeZoneItemModel.CustomRole.TimeZoneId), "UTC"
        )
        self.assertEqual(
            model.data(utc_index, QgsTimeZoneItemModel.CustomRole.TimeZone).id(), b"UTC"
        )
        self.assertEqual(
            model.data(utc_index, QgsTimeZoneItemModel.CustomRole.UtcOffset), 0
        )

        utc_index = model.indexFromTimeZone(QTimeZone(b"UTC+03:00"))
        self.assertTrue(utc_index.isValid())
        self.assertEqual(
            model.data(utc_index, Qt.ItemDataRole.DisplayRole),
            "UTC+03:00: UTC+03:00 (UTC+03:00)",
        )
        self.assertEqual(
            model.data(utc_index, QgsTimeZoneItemModel.CustomRole.TimeZoneId),
            "UTC+03:00",
        )
        self.assertEqual(
            model.data(utc_index, QgsTimeZoneItemModel.CustomRole.TimeZone).id(),
            b"UTC+03:00",
        )
        self.assertEqual(
            model.data(utc_index, QgsTimeZoneItemModel.CustomRole.UtcOffset), 10800
        )

        self.assertFalse(model.indexFromTimeZone(QTimeZone()).isValid())


class TestQgsTimeZoneSortFilterModel(QgisTestCase):
    def test_model(self):
        source_model = QgsTimeZoneItemModel()
        model = QgsTimeZoneSortFilterModel(source_model)

        self.assertEqual(model.rowCount(), len(QTimeZone.availableTimeZoneIds()))
        self.assertEqual(model.columnCount(), 1)

        model.setFilterString("brisbane")
        self.assertEqual(model.rowCount(), 1)

        self.assertIn(
            "brisbane",
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole).lower(),
        )
        tz = model.data(model.index(0, 0), QgsTimeZoneItemModel.CustomRole.TimeZone)
        self.assertEqual(tz.offsetFromUtc(QDateTime.currentDateTime()), 36000)

        index = model.indexFromTimeZone(tz)
        self.assertTrue(index.isValid())
        self.assertEqual(index.row(), 0)

        model.setFilterString("xxxxxxxxxxxxx")
        self.assertEqual(model.rowCount(), 0)

        self.assertFalse(model.indexFromTimeZone(tz).isValid())


class TestQgsTimeZoneWidget(QgisTestCase):
    def test_widget(self):
        widget = QgsTimeZoneWidget()
        self.assertFalse(widget.timeZone().isValid())

        spy = QSignalSpy(widget.timeZoneChanged)
        widget.setTimeZone(QTimeZone(b"UTC"))
        self.assertEqual(widget.timeZone().id(), b"UTC")
        self.assertEqual(len(spy), 1)
        self.assertEqual(spy[0][0].id(), b"UTC")

        widget.setTimeZone(QTimeZone(b"UTC+03:00"))
        self.assertEqual(widget.timeZone().id(), b"UTC+03:00")
        self.assertEqual(len(spy), 2)
        self.assertEqual(spy[-1][0].id(), b"UTC+03:00")

        widget.setTimeZone(QTimeZone())
        self.assertFalse(widget.timeZone().isValid())
        self.assertEqual(len(spy), 3)
        self.assertFalse(spy[-1][0].isValid())


class TestQgsTimeZoneComboBox(QgisTestCase):
    def test_widget(self):
        widget = QgsTimeZoneComboBox()
        child_combo = widget.findChildren(QComboBox)[0]
        self.assertFalse(widget.timeZone().isValid())
        # current value should not be displayed
        self.assertEqual(child_combo.count(), 2)

        spy = QSignalSpy(widget.timeZoneChanged)
        widget.setTimeZone(QTimeZone(b"UTC"))
        self.assertEqual(widget.timeZone().id(), b"UTC")
        self.assertEqual(len(spy), 1)
        self.assertEqual(spy[0][0].id(), b"UTC")
        # current value should now be displayed
        self.assertEqual(child_combo.count(), 3)

        # no extra signal when setting to same
        widget.setTimeZone(QTimeZone(b"UTC"))
        self.assertEqual(len(spy), 1)

        widget.setTimeZone(QTimeZone(b"UTC+03:00"))
        self.assertEqual(widget.timeZone().id(), b"UTC+03:00")
        self.assertEqual(len(spy), 2)
        self.assertEqual(spy[-1][0].id(), b"UTC+03:00")

        widget.setTimeZone(QTimeZone())
        self.assertFalse(widget.timeZone().isValid())
        self.assertEqual(len(spy), 3)
        self.assertFalse(spy[-1][0].isValid())
        # current value should not be displayed
        self.assertEqual(child_combo.count(), 2)

        # no extra signal when setting to same
        widget.setTimeZone(QTimeZone())
        self.assertEqual(len(spy), 3)


class TestQgsDateTimeWithTimeZoneWidget(QgisTestCase):
    def test_widget(self):
        widget = QgsDateTimeWithTimeZoneWidget()
        self.assertFalse(widget.dateTime().isValid())

        spy = QSignalSpy(widget.dateTimeChanged)

        dt = QDateTime(QDate(2020, 1, 2), QTime(1, 2, 3))
        dt.setTimeZone(QTimeZone(b"UTC+03:00"))

        widget.setDateTime(dt)
        self.assertEqual(widget.dateTime(), dt)
        self.assertEqual(widget.dateTime().timeZone().id(), b"UTC+03:00")

        self.assertEqual(widget.dateTimeEdit().dateTime(), dt)
        self.assertEqual(widget.timeZoneComboBox().timeZone(), dt.timeZone())

        self.assertEqual(len(spy), 1)
        widget.setDateTime(dt)
        self.assertEqual(len(spy), 1)

        widget.setDateTime(QDateTime())
        self.assertEqual(len(spy), 2)
        self.assertFalse(widget.dateTime().isValid())


if __name__ == "__main__":
    unittest.main()
