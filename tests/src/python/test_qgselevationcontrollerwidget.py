"""QGIS Unit tests for QgsElevationControllerWidget

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtTest import QSignalSpy
from qgis.core import (
    QgsDoubleRange,
    QgsProject
)
from qgis.gui import QgsElevationControllerWidget
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsElevationControllerWidget(QgisTestCase):

    def testRange(self):
        w = QgsElevationControllerWidget()
        spy = QSignalSpy(w.rangeChanged)
        w.setRangeLimits(QgsDoubleRange(100.5, 1000))
        self.assertEqual(w.rangeLimits(),
                         QgsDoubleRange(100.5, 1000))
        self.assertEqual(len(spy), 1)

        # ensure that range is losslessly maintained if the user doesn't
        # move the slider
        w.setRange(QgsDoubleRange(130.3, 920.6))
        self.assertEqual(len(spy), 2)
        self.assertEqual(w.range(),
                         QgsDoubleRange(130.3, 920.6))
        self.assertEqual(spy[-1][0],
                         QgsDoubleRange(130.3, 920.6))
        # no change = no signal
        w.setRange(QgsDoubleRange(130.3, 920.6))
        self.assertEqual(len(spy), 2)
        # tiny change, not enough to be within widget precision, should still
        # raise signal
        w.setRange(QgsDoubleRange(130.300001, 920.6))
        self.assertEqual(len(spy), 3)
        self.assertEqual(spy[-1][0],
                         QgsDoubleRange(130.300001, 920.6))

        # change visible limits to something which fits the old range
        # make sure this is lossless
        w.setRangeLimits(QgsDoubleRange(50, 1050))
        self.assertEqual(w.range(),
                         QgsDoubleRange(130.300001, 920.6))
        self.assertEqual(len(spy), 3)

        # change visible limits to something which fits only part of the old range
        w.setRangeLimits(QgsDoubleRange(160, 1050))
        self.assertEqual(w.range(),
                         QgsDoubleRange(160.0, 920.6))
        self.assertEqual(len(spy), 4)
        self.assertEqual(spy[-1][0],
                         QgsDoubleRange(160.0, 920.6))

        w.setRangeLimits(QgsDoubleRange(120, 917.5))
        self.assertEqual(w.range(),
                         QgsDoubleRange(160.0, 917.5))
        self.assertEqual(len(spy), 5)
        self.assertEqual(spy[-1][0], QgsDoubleRange(160.0, 917.5))

        w.setRangeLimits(QgsDoubleRange(171, 815.5))
        self.assertEqual(w.range(),
                         QgsDoubleRange(171, 815.5))
        self.assertEqual(len(spy), 6)
        self.assertEqual(spy[-1][0],
                         QgsDoubleRange(171, 815.5))

        # infinite range => should be ignored
        w.setRangeLimits(QgsDoubleRange())
        self.assertEqual(w.rangeLimits(),
                         QgsDoubleRange(171, 815.5))
        self.assertEqual(w.range(),
                         QgsDoubleRange(171, 815.5))
        self.assertEqual(len(spy), 6)

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

        w.slider().setRange(int(w.slider().minimum() + slider_range * 0.4),
                            int(w.slider().minimum() + slider_range * 0.7))
        self.assertEqual(len(spy), 3)
        self.assertAlmostEqual(spy[-1][0].lower(), 459.644, 3)
        self.assertAlmostEqual(spy[-1][0].upper(), 729.495, 3)
        self.assertAlmostEqual(w.range().lower(), 459.644, 3)
        self.assertAlmostEqual(w.range().upper(), 729.495, 3)

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

    def test_project_interaction(self):
        """
        Test interaction of widget with project
        """
        elevation_properties = QgsProject.instance().elevationProperties()
        elevation_properties.setElevationRange(QgsDoubleRange(50, 160))
        w = QgsElevationControllerWidget()
        spy = QSignalSpy(w.rangeChanged)
        self.assertEqual(w.rangeLimits(),
                         QgsDoubleRange(50, 160)
                         )
        # initially selected range should be full range
        self.assertEqual(w.range(),
                         QgsDoubleRange(50, 160)
                         )

        # change range limits for project
        elevation_properties.setElevationRange(QgsDoubleRange(80, 130))
        self.assertEqual(w.rangeLimits(),
                         QgsDoubleRange(80, 130)
                         )
        self.assertEqual(w.range(), QgsDoubleRange(80, 130))
        self.assertEqual(len(spy), 1)
        self.assertEqual(spy[-1][0], QgsDoubleRange(80, 130))

        # expand out range from current value
        elevation_properties.setElevationRange(QgsDoubleRange(40, 190))
        self.assertEqual(w.rangeLimits(),
                         QgsDoubleRange(40, 190)
                         )
        # selected range should be unchanged
        self.assertEqual(len(spy), 1)
        self.assertEqual(w.range(), QgsDoubleRange(80, 130))

        # a project with no elevation range
        elevation_properties.setElevationRange(QgsDoubleRange())
        w = QgsElevationControllerWidget()
        # ensure some initial range is set, even if we are just guessing!
        self.assertEqual(w.rangeLimits(),
                         QgsDoubleRange(0, 100)
                         )


if __name__ == '__main__':
    unittest.main()
