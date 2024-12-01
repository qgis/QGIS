"""QGIS Unit tests for QgsMaskRenderSettings.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "2024-06"
__copyright__ = "Copyright 2024, The QGIS Project"


from qgis.core import QgsMaskRenderSettings
from qgis.testing import unittest


class TestQgsMaskRenderSettings(unittest.TestCase):

    def testGetSet(self):
        settings = QgsMaskRenderSettings()
        self.assertEqual(settings.simplifyTolerance(), 0)
        settings.setSimplificationTolerance(10)
        self.assertEqual(settings.simplifyTolerance(), 10)


if __name__ == "__main__":
    unittest.main()
