"""QGIS Unit tests for scale bar renderers

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import math
import unittest

from qgis.core import QgsScaleBarRenderer
from qgis.testing import QgisTestCase, start_app

start_app()


class TestQgsScaleBarRenderers(QgisTestCase):
    def test_context(self):
        context = QgsScaleBarRenderer.ScaleBarContext()
        context.segmentWidth = 5
        self.assertTrue(context.isValid())
        context.segmentWidth = math.nan
        self.assertFalse(context.isValid())


if __name__ == "__main__":
    unittest.main()
