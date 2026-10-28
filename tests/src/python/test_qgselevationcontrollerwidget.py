"""QGIS Unit tests for QgsElevationControllerWidget

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import unittest

from qgis.core import QgsDoubleRange, QgsProject
from qgis.gui import QgsElevationControllerWidget, QgsMapCanvas
from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtTest import QSignalSpy
from qgis.testing import QgisTestCase, start_app

start_app()


class TestQgsElevationControllerWidget(QgisTestCase):
    def testRange(self):
        w = QgsElevationControllerWidget()
        spy = QSignalSpy(w.rangeChanged)
        w.setRangeLimits(QgsDoubleRange(100.5, 1000))
        self.assertEqual(w.rangeLimits(), QgsDoubleRange(100.5, 1000))
        self.assertEqual(len(spy), 1)

        # ensure that range is losslessly maintained if the user doesn't
        # move the slider
        w.setRange(QgsDoubleRange(130.3, 920.6))
        self.assertEqual(len(spy), 2)
        self.assertEqual(w.range(), QgsDoubleRange(130.3, 920.6))
        self.assertEqual(spy[-1][0], QgsDoubleRange(130.3, 920.6))
        # no change = no signal
        w.setRange(QgsDoubleRange(130.3, 920.6))
        self.assertEqual(len(spy), 2)
        # tiny change, not enough to be within widget precision, should still
        # raise signal
        w.setRange(QgsDoubleRange(130.300001, 920.6))
        self.assertEqual(len(spy), 3)
        self.assertEqual(spy[-1][0], QgsDoubleRange(130.300001, 920.6))

        # change visible limits to something which fits the old range
        # make sure this is lossless
        w.setRangeLimits(QgsDoubleRange(50, 1050))
        self.assertEqual(w.range(), QgsDoubleRange(130.300001, 920.6))
        self.assertEqual(len(spy), 3)

        # change visible limits to something which fits only part of the old range
        w.setRangeLimits(QgsDoubleRange(160, 1050))
        self.assertEqual(w.range(), QgsDoubleRange(160.0, 920.6))
        self.assertEqual(len(spy), 4)
        self.assertEqual(spy[-1][0], QgsDoubleRange(160.0, 920.6))

        w.setRangeLimits(QgsDoubleRange(120, 917.5))
        self.assertEqual(w.range(), QgsDoubleRange(160.0, 917.5))
        self.assertEqual(len(spy), 5)
        self.assertEqual(spy[-1][0], QgsDoubleRange(160.0, 917.5))

        w.setRangeLimits(QgsDoubleRange(171, 815.5))
        self.assertEqual(w.range(), QgsDoubleRange(171, 815.5))
        self.assertEqual(len(spy), 6)
        self.assertEqual(spy[-1][0], QgsDoubleRange(171, 815.5))

        # infinite range => should be ignored
        w.setRangeLimits(QgsDoubleRange())
        self.assertEqual(w.rangeLimits(), QgsDoubleRange(171, 815.5))
        self.assertEqual(w.range(), QgsDoubleRange(171, 815.5))
        self.assertEqual(len(spy), 6)

        # zero width or inverted limits => should be ignored
        w.setRangeLimits(QgsDoubleRange(200, 200))
        self.assertEqual(w.rangeLimits(), QgsDoubleRange(171, 815.5))
        w.setRangeLimits(QgsDoubleRange(500, 200))
        self.assertEqual(w.rangeLimits(), QgsDoubleRange(171, 815.5))
        self.assertEqual(len(spy), 6)

    def test_map_canvas(self):
        """
        The canvas provides the layers the limits entries work on
        """
        w = QgsElevationControllerWidget()
        self.assertIsNone(w.mapCanvas())

        canvas = QgsMapCanvas()
        w.setMapCanvas(canvas)
        self.assertEqual(w.mapCanvas(), canvas)

        # a canvas without layers must not alter the limits
        limits = w.rangeLimits()
        w.setMapCanvas(canvas)
        self.assertEqual(w.rangeLimits(), limits)

    def test_slider_interaction(self):
        """
        Simulate user interaction with slider
        """
        w = QgsElevationControllerWidget()
        spy = QSignalSpy(w.rangeChanged)
        w.setRangeLimits(QgsDoubleRange(100.5, 1000))
        self.assertEqual(w.rangeLimits(), QgsDoubleRange(100.5, 1000))
        self.assertEqual(len(spy), 1)
        w.setRange(QgsDoubleRange(130.3, 920.6))
        self.assertEqual(len(spy), 2)
        self.assertEqual(w.range(), QgsDoubleRange(130.3, 920.6))
        self.assertEqual(spy[-1][0], QgsDoubleRange(130.3, 920.6))

        slider_range = w.slider().maximum() - w.slider().minimum()
        # slider should have a decent integer precision:
        self.assertGreaterEqual(slider_range, 500)

        w.slider().setRange(
            int(w.slider().minimum() + slider_range * 0.4),
            int(w.slider().minimum() + slider_range * 0.7),
        )
        # slider values are snapped to round values, here multiples of 1
        self.assertEqual(len(spy), 3)
        self.assertEqual(spy[-1][0], QgsDoubleRange(460, 729))
        self.assertEqual(w.range(), QgsDoubleRange(460, 729))

    def test_slider_snapping(self):
        """
        Slider interaction should snap to round values and to significant elevations
        """
        w = QgsElevationControllerWidget()
        w.setRangeLimits(QgsDoubleRange(0, 1000))

        slider = w.slider()

        def slider_pos(elevation):
            """slider position matching an elevation, whatever precision the slider uses"""
            return slider.minimum() + round(
                elevation * (slider.maximum() - slider.minimum()) / 1000
            )

        slider.setRange(slider_pos(123), slider_pos(457))
        # a 0 - 1000 range is rounded to multiples of 100, so the slider snaps to multiples of 10
        self.assertEqual(w.range(), QgsDoubleRange(120, 460))

        # an elevation which is significant for the layers is a closer snapping target than 120
        w.setSignificantElevations([123.4])
        slider.setRange(slider_pos(122), slider_pos(457))
        self.assertEqual(w.range(), QgsDoubleRange(123.4, 460))

        # a locked range size must not be altered by snapping
        w.setFixedRangeSize(35.5)
        slider.setRange(slider_pos(223), slider_pos(457))
        self.assertAlmostEqual(w.range().lower(), 220, 6)
        self.assertAlmostEqual(w.range().upper() - w.range().lower(), 35.5, 6)

    def testFixedRangeSize(self):
        """
        Test that fixed range size is correctly handled
        """
        w = QgsElevationControllerWidget()
        w.setRangeLimits(QgsDoubleRange(100.5, 1000))
        w.setFixedRangeSize(10.0001)
        self.assertEqual(w.fixedRangeSize(), 10.0001)
        w.setRange(QgsDoubleRange(130.3, 920.6))
        self.assertAlmostEqual(w.range().upper() - w.range().lower(), 10.0001, 6)

        w.slider().setLowerValue(50)
        self.assertAlmostEqual(w.range().upper() - w.range().lower(), 10.0001, 6)

    def test_fixed_range_size_with_new_limits(self):
        """
        A locked range size must survive a change of the limits
        """
        w = QgsElevationControllerWidget()
        w.setRangeLimits(QgsDoubleRange(0, 100))
        w.setFixedRangeSize(10)
        w.setRange(QgsDoubleRange(20, 30))
        self.assertAlmostEqual(w.range().upper() - w.range().lower(), 10, 6)

        # the slider holds the fixed size in its own integer units, which follow the limits
        w.setRangeLimits(QgsDoubleRange(0, 1000))
        self.assertEqual(w.slider().fixedRangeSize(), round(w.slider().maximum() / 100))
        self.assertAlmostEqual(w.range().upper() - w.range().lower(), 10, 6)

        # limits which no longer fit the range move it instead of shrinking it
        w.setRange(QgsDoubleRange(20, 30))
        w.setRangeLimits(QgsDoubleRange(0, 25))
        self.assertAlmostEqual(w.range().lower(), 15, 6)
        self.assertAlmostEqual(w.range().upper(), 25, 6)

    def test_set_range_with_locked_size(self):
        """
        A locked size must win over the range passed to setRange
        """
        w = QgsElevationControllerWidget()
        w.setRangeLimits(QgsDoubleRange(0, 1000))
        w.setFixedRangeSize(10)

        w.setRange(QgsDoubleRange(100, 900))
        self.assertEqual(w.range(), QgsDoubleRange(100, 110))

        # a range which would reach past the upper limit moves down instead of growing
        w.setRange(QgsDoubleRange(995, 999))
        self.assertEqual(w.range(), QgsDoubleRange(990, 1000))

    def test_project_interaction(self):
        """
        Test interaction of widget with project
        """
        elevation_properties = QgsProject.instance().elevationProperties()
        elevation_properties.setElevationRange(QgsDoubleRange(50, 160))
        w = QgsElevationControllerWidget()
        spy = QSignalSpy(w.rangeChanged)
        self.assertEqual(w.rangeLimits(), QgsDoubleRange(50, 160))
        # initially selected range should be full range
        self.assertEqual(w.range(), QgsDoubleRange(50, 160))

        # change range limits for project
        elevation_properties.setElevationRange(QgsDoubleRange(80, 130))
        self.assertEqual(w.rangeLimits(), QgsDoubleRange(80, 130))
        self.assertEqual(w.range(), QgsDoubleRange(80, 130))
        self.assertEqual(len(spy), 1)
        self.assertEqual(spy[-1][0], QgsDoubleRange(80, 130))

        # expand out range from current value
        elevation_properties.setElevationRange(QgsDoubleRange(40, 190))
        self.assertEqual(w.rangeLimits(), QgsDoubleRange(40, 190))
        # selected range should be unchanged
        self.assertEqual(len(spy), 1)
        self.assertEqual(w.range(), QgsDoubleRange(80, 130))

        # a project with no elevation range
        elevation_properties.setElevationRange(QgsDoubleRange())
        w = QgsElevationControllerWidget()
        # ensure some initial range is set, even if we are just guessing!
        self.assertEqual(w.rangeLimits(), QgsDoubleRange(0, 100))


if __name__ == "__main__":
    unittest.main()
