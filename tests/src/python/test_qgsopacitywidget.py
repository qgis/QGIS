"""QGIS Unit tests for QgsOpacityWidget

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "30/05/2017"
__copyright__ = "Copyright 2017, The QGIS Project"


from qgis.PyQt.QtTest import QSignalSpy
from qgis.gui import QgsOpacityWidget
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsOpacityWidget(QgisTestCase):

    def testGettersSetters(self):
        """test widget getters/setters"""
        w = QgsOpacityWidget()

        w.setOpacity(0.2)
        self.assertEqual(w.opacity(), 0.2)

        # bad values
        w.setOpacity(-0.2)
        self.assertEqual(w.opacity(), 0.0)
        w.setOpacity(100)
        self.assertEqual(w.opacity(), 1.0)

    def test_ChangedSignals(self):
        """test that signals are correctly emitted when setting opacity"""

        w = QgsOpacityWidget()

        spy = QSignalSpy(w.opacityChanged)
        w.setOpacity(0.2)

        self.assertEqual(len(spy), 1)
        self.assertEqual(spy[0][0], 0.2)

        # bad value
        w.setOpacity(100)
        self.assertEqual(len(spy), 2)
        self.assertEqual(spy[1][0], 1.0)


if __name__ == "__main__":
    unittest.main()
