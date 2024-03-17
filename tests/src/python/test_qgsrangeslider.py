"""QGIS Unit tests for QgsRangeSlider

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '2020-11-25'
__copyright__ = 'Copyright 2020, The QGIS Project'

from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtWidgets import QSlider
from qgis.gui import QgsRangeSlider
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsRangeSlider(QgisTestCase):

    def testSettersGetters(self):
        w = QgsRangeSlider()
        w.setOrientation(Qt.Orientation.Horizontal)
        self.assertEqual(w.orientation(), Qt.Orientation.Horizontal)
        w.setOrientation(Qt.Orientation.Vertical)
        self.assertEqual(w.orientation(), Qt.Orientation.Vertical)

        w.setTickPosition(QSlider.TickPosition.TicksAbove)
        self.assertEqual(w.tickPosition(), QSlider.TickPosition.TicksAbove)
        w.setTickInterval(5)
        self.assertEqual(w.tickInterval(), 5)
        w.setFlippedDirection(True)
        self.assertTrue(w.flippedDirection())

        w.setSingleStep(2)
        self.assertEqual(w.singleStep(), 2)

        w.setPageStep(5)
        self.assertEqual(w.pageStep(), 5)

        self.assertEqual(w.fixedRangeSize(), -1)
        w.setFixedRangeSize(5)
        self.assertEqual(w.fixedRangeSize(), 5)

    def testLimits(self):
        w = QgsRangeSlider()
        spy = QSignalSpy(w.rangeLimitsChanged)

        w.setMaximum(50)
        self.assertEqual(w.minimum(), 0)
        self.assertEqual(w.maximum(), 50)
        self.assertEqual(len(spy), 1)
        self.assertEqual(spy[-1], [0, 50])
        w.setMaximum(50)
        self.assertEqual(len(spy), 1)

        w.setMinimum(40)
        self.assertEqual(w.minimum(), 40)
        self.assertEqual(w.maximum(), 50)
        self.assertEqual(len(spy), 2)
        self.assertEqual(spy[-1], [40, 50])
        w.setMinimum(40)
        self.assertEqual(len(spy), 2)

        w.setRangeLimits(40, 50)
        self.assertEqual(w.minimum(), 40)
        self.assertEqual(w.maximum(), 50)
        self.assertEqual(len(spy), 2)
        w.setRangeLimits(40, 60)
        self.assertEqual(w.minimum(), 40)
        self.assertEqual(w.maximum(), 60)
        self.assertEqual(len(spy), 3)
        self.assertEqual(spy[-1], [40, 60])
        w.setRangeLimits(45, 60)
        self.assertEqual(w.minimum(), 45)
        self.assertEqual(w.maximum(), 60)
        self.assertEqual(len(spy), 4)
        self.assertEqual(spy[-1], [45, 60])
        w.setRangeLimits(30, 70)
        self.assertEqual(w.minimum(), 30)
        self.assertEqual(w.maximum(), 70)
        self.assertEqual(len(spy), 5)
        self.assertEqual(spy[-1], [30, 70])

        # inconsistent ranges
        w.setMinimum(80)
        self.assertEqual(w.minimum(), 80)
        self.assertEqual(w.maximum(), 80)
        self.assertEqual(len(spy), 6)
        self.assertEqual(spy[-1], [80, 80])

        w.setRangeLimits(10, 20)
        self.assertEqual(len(spy), 7)
        w.setMaximum(8)
        self.assertEqual(w.minimum(), 8)
        self.assertEqual(w.maximum(), 8)
        self.assertEqual(len(spy), 8)
        self.assertEqual(spy[-1], [8, 8])

        w.setRangeLimits(20, 10)
        self.assertEqual(w.minimum(), 10)
        self.assertEqual(w.maximum(), 20)
        self.assertEqual(len(spy), 9)
        self.assertEqual(spy[-1], [10, 20])
        w.setRangeLimits(20, 10)
        self.assertEqual(w.minimum(), 10)
        self.assertEqual(w.maximum(), 20)
        self.assertEqual(len(spy), 9)

    def testValues(self):
        w = QgsRangeSlider()
        w.setRangeLimits(0, 10)

        w.setUpperValue(7)

        spy = QSignalSpy(w.rangeChanged)

        w.setLowerValue(5)
        self.assertEqual(w.lowerValue(), 5)
        self.assertEqual(len(spy), 1)
        self.assertEqual(spy[-1], [5, 7])
        w.setLowerValue(5)
        self.assertEqual(w.lowerValue(), 5)
        self.assertEqual(len(spy), 1)

        w.setUpperValue(8)
        self.assertEqual(w.lowerValue(), 5)
        self.assertEqual(w.upperValue(), 8)
        self.assertEqual(len(spy), 2)
        self.assertEqual(spy[-1], [5, 8])
        w.setUpperValue(8)
        self.assertEqual(w.lowerValue(), 5)
        self.assertEqual(w.upperValue(), 8)
        self.assertEqual(len(spy), 2)

        w.setRange(3, 7)
        self.assertEqual(w.lowerValue(), 3)
        self.assertEqual(w.upperValue(), 7)
        self.assertEqual(len(spy), 3)
        self.assertEqual(spy[-1], [3, 7])
        w.setRange(3, 7)
        self.assertEqual(w.lowerValue(), 3)
        self.assertEqual(w.upperValue(), 7)
        self.assertEqual(len(spy), 3)

        w.setRange(3, 8)
        self.assertEqual(w.lowerValue(), 3)
        self.assertEqual(w.upperValue(), 8)
        self.assertEqual(len(spy), 4)
        self.assertEqual(spy[-1], [3, 8])

        w.setRange(4, 8)
        self.assertEqual(w.lowerValue(), 4)
        self.assertEqual(w.upperValue(), 8)
        self.assertEqual(len(spy), 5)
        self.assertEqual(spy[-1], [4, 8])

        # set min > max, max should be raised
        w.setLowerValue(9)
        self.assertEqual(w.lowerValue(), 9)
        self.assertEqual(w.upperValue(), 9)
        self.assertEqual(len(spy), 6)
        self.assertEqual(spy[-1], [9, 9])

        w.setRange(4, 8)
        self.assertEqual(len(spy), 7)

        # set max < min, min should be lowered
        w.setUpperValue(3)
        self.assertEqual(w.lowerValue(), 3)
        self.assertEqual(w.upperValue(), 3)
        self.assertEqual(len(spy), 8)
        self.assertEqual(spy[-1], [3, 3])

        # set to values outside limit, should be clamped
        w.setUpperValue(12)
        self.assertEqual(w.lowerValue(), 3)
        self.assertEqual(w.upperValue(), 10)
        self.assertEqual(len(spy), 9)
        self.assertEqual(spy[-1], [3, 10])

        w.setLowerValue(-2)
        self.assertEqual(w.lowerValue(), 0)
        self.assertEqual(w.upperValue(), 10)
        self.assertEqual(len(spy), 10)
        self.assertEqual(spy[-1], [0, 10])

        w.setUpperValue(-3)
        self.assertEqual(w.lowerValue(), 0)
        self.assertEqual(w.upperValue(), 0)
        self.assertEqual(len(spy), 11)
        self.assertEqual(spy[-1], [0, 0])

        w.setLowerValue(13)
        self.assertEqual(w.lowerValue(), 10)
        self.assertEqual(w.upperValue(), 10)
        self.assertEqual(len(spy), 12)
        self.assertEqual(spy[-1], [10, 10])

        w.setRange(-2, 3)
        self.assertEqual(w.lowerValue(), 0)
        self.assertEqual(w.upperValue(), 3)
        self.assertEqual(len(spy), 13)
        self.assertEqual(spy[-1], [0, 3])

        w.setRange(3, 13)
        self.assertEqual(w.lowerValue(), 3)
        self.assertEqual(w.upperValue(), 10)
        self.assertEqual(len(spy), 14)
        self.assertEqual(spy[-1], [3, 10])

        w.setRange(-3, -2)
        self.assertEqual(w.lowerValue(), 0)
        self.assertEqual(w.upperValue(), 0)
        self.assertEqual(len(spy), 15)
        self.assertEqual(spy[-1], [0, 0])

        w.setRange(12, 13)
        self.assertEqual(w.lowerValue(), 10)
        self.assertEqual(w.upperValue(), 10)
        self.assertEqual(len(spy), 16)
        self.assertEqual(spy[-1], [10, 10])

        # flipped ranges
        w.setRange(7, 4)
        self.assertEqual(w.lowerValue(), 4)
        self.assertEqual(w.upperValue(), 7)
        self.assertEqual(len(spy), 17)
        self.assertEqual(spy[-1], [4, 7])

    def testChangeLimitsOutsideValue(self):
        # force value changes via limit changes
        w = QgsRangeSlider()
        w.setRangeLimits(0, 10)

        w.setUpperValue(7)

        spy = QSignalSpy(w.rangeChanged)

        w.setMaximum(5)
        self.assertEqual(w.lowerValue(), 0)
        self.assertEqual(w.upperValue(), 5)
        self.assertEqual(len(spy), 1)
        self.assertEqual(spy[-1], [0, 5])

        w.setMinimum(2)
        self.assertEqual(w.lowerValue(), 2)
        self.assertEqual(w.upperValue(), 5)
        self.assertEqual(len(spy), 2)
        self.assertEqual(spy[-1], [2, 5])

        w.setRangeLimits(0, 10)
        w.setRange(0, 10)
        self.assertEqual(len(spy), 3)

        w.setRangeLimits(3, 7)
        self.assertEqual(w.lowerValue(), 3)
        self.assertEqual(w.upperValue(), 7)
        self.assertEqual(len(spy), 4)
        self.assertEqual(spy[-1], [3, 7])

        w.setRangeLimits(0, 10)
        w.setRange(0, 10)
        self.assertEqual(len(spy), 5)

        w.setRangeLimits(7, 3)  # flipped
        self.assertEqual(w.lowerValue(), 3)
        self.assertEqual(w.upperValue(), 7)
        self.assertEqual(len(spy), 6)
        self.assertEqual(spy[-1], [3, 7])

    def test_fixed_range_width(self):
        """
        Test interactions with fixed range widths
        """
        w = QgsRangeSlider()
        w.setRangeLimits(0, 100)
        w.setFixedRangeSize(10)
        self.assertEqual(w.upperValue() - w.lowerValue(), 10)

        w.setUpperValue(70)
        self.assertEqual(w.upperValue(), 70)
        self.assertEqual(w.lowerValue(), 60)

        w.setLowerValue(5)
        self.assertEqual(w.upperValue(), 15)
        self.assertEqual(w.lowerValue(), 5)

        # try to force value outside range
        w.setUpperValue(5)
        self.assertEqual(w.upperValue(), 10)
        self.assertEqual(w.lowerValue(), 0)

        w.setLowerValue(95)
        self.assertEqual(w.upperValue(), 100)
        self.assertEqual(w.lowerValue(), 90)

        w.setRange(0, 5)
        self.assertEqual(w.upperValue(), 10)
        self.assertEqual(w.lowerValue(), 0)

        w.setRange(95, 100)
        self.assertEqual(w.upperValue(), 100)
        self.assertEqual(w.lowerValue(), 90)

        # with zero width fixed range
        w.setFixedRangeSize(0)
        self.assertEqual(w.upperValue() - w.lowerValue(), 0)

        w.setUpperValue(70)
        self.assertEqual(w.upperValue(), 70)
        self.assertEqual(w.lowerValue(), 70)

        w.setLowerValue(5)
        self.assertEqual(w.upperValue(), 5)
        self.assertEqual(w.lowerValue(), 5)

        w.setRange(0, 5)
        self.assertEqual(w.upperValue(), 0)
        self.assertEqual(w.lowerValue(), 0)


if __name__ == '__main__':
    unittest.main()
