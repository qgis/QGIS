"""QGIS Unit tests for QgsRubberBand.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from qgis.gui import QgsRubberBand
import unittest
from qgis.testing import start_app, QgisTestCase
from qgis.testing.mocked import get_iface

start_app()


class TestQgsRubberBand(QgisTestCase):

    def setUp(self):
        self.iface = get_iface()

    def testBugfix48471(self):
        """Test scenario of https://github.com/qgis/QGIS/issues/48471"""

        countBefore = 0
        for item in self.iface.mapCanvas().scene().items():
            if isinstance(item, QgsRubberBand):
                countBefore += 1

        rubberband = QgsRubberBand(self.iface.mapCanvas())

        try:
            count = 0
            for item in self.iface.mapCanvas().scene().items():
                if isinstance(item, QgsRubberBand):
                    count += 1
                    self.assertTrue(item.asGeometry().isNull())
            self.assertEqual(count, countBefore + 1)
        finally:
            self.iface.mapCanvas().scene().removeItem(rubberband)


if __name__ == "__main__":
    unittest.main()
