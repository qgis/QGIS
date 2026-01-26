"""QGIS Unit tests for QgsPercentageWidget

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from qgis.PyQt.QtTest import QSignalSpy
from qgis.gui import QgsPercentageWidget
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsPercentageWidget(QgisTestCase):

    def testGettersSetters(self):
        """test widget getters/setters"""
        w = QgsPercentageWidget()

        w.setValue(0.2)
        self.assertEqual(w.value(), 0.2)

        # bad values
        w.setValue(-0.2)
        self.assertEqual(w.value(), 0.0)
        w.setValue(100)
        self.assertEqual(w.value(), 1.0)

    def test_ChangedSignals(self):
        """test that signals are correctly emitted when setting value"""

        w = QgsPercentageWidget()

        spy = QSignalSpy(w.valueChanged)
        w.setValue(0.2)

        self.assertEqual(len(spy), 1)
        self.assertEqual(spy[0][0], 0.2)

        # bad value
        w.setValue(100)
        self.assertEqual(len(spy), 2)
        self.assertEqual(spy[1][0], 1.0)


if __name__ == "__main__":
    unittest.main()
