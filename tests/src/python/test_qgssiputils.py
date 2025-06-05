"""QGIS Unit tests for QgsSipUtils

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from qgis.core import (
    QgsSipUtils,
    QgsGeometry,
    QgsSingleSymbolRenderer,
    QgsPoint,
    QgsFillSymbol,
)
import unittest
from qgis.testing import QgisTestCase


class TestQgsSipUtils(QgisTestCase):

    def test_isPyOwned(self):
        # not a sip object
        self.assertTrue(QgsSipUtils.isPyOwned(5))

        p = QgsPoint()
        self.assertTrue(QgsSipUtils.isPyOwned(p))
        # assign ownership to other object
        g = QgsGeometry(p)
        self.assertTrue(QgsSipUtils.isPyOwned(g))
        self.assertFalse(QgsSipUtils.isPyOwned(p))
        self.assertFalse(QgsSipUtils.isPyOwned(g.get()))
        self.assertTrue(QgsSipUtils.isPyOwned(g.get().clone()))

        renderer = QgsSingleSymbolRenderer(QgsFillSymbol.createSimple({}))
        self.assertFalse(QgsSipUtils.isPyOwned(renderer.symbol()))
        renderer2 = renderer.clone()
        # an object created in c++, never seen before by sip/python
        self.assertFalse(QgsSipUtils.isPyOwned(renderer2.symbol()))


if __name__ == "__main__":
    unittest.main()
