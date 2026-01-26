"""QGIS Unit tests for QgsScreenProperties.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "22/06/2023"
__copyright__ = "Copyright 2023, The QGIS Project"

from qgis.PyQt.QtGui import QGuiApplication
from qgis.core import QgsScreenProperties, QgsRenderContext
from qgis.testing import unittest, start_app

qgis_app = start_app()


class TestQgsScreenProperties(unittest.TestCase):

    def test_invalid(self):
        invalid = QgsScreenProperties()
        self.assertFalse(invalid.isValid())

    def test_properties(self):
        properties = QgsScreenProperties()
        properties.setDevicePixelRatio(2)
        self.assertEqual(properties.devicePixelRatio(), 2)
        self.assertTrue(properties.isValid())

        properties = QgsScreenProperties()
        properties.setPhysicalDpi(200)
        self.assertEqual(properties.physicalDpi(), 200)
        self.assertTrue(properties.isValid())

    def test_equality(self):
        properties = QgsScreenProperties()
        properties2 = QgsScreenProperties()
        self.assertEqual(properties, properties2)

        properties.setDevicePixelRatio(2)
        self.assertNotEqual(properties, properties2)

        properties2.setDevicePixelRatio(2)
        self.assertEqual(properties, properties2)

        properties.setPhysicalDpi(999)
        self.assertNotEqual(properties, properties2)

        properties2.setPhysicalDpi(999)
        self.assertEqual(properties, properties2)

    def test_from_screen(self):
        screen = QGuiApplication.instance().primaryScreen()
        if not screen:
            return

        properties = QgsScreenProperties(screen)
        self.assertTrue(properties.isValid())
        self.assertEqual(properties.devicePixelRatio(), screen.devicePixelRatio())
        self.assertEqual(properties.physicalDpi(), screen.physicalDotsPerInch())

    def test_update_render_context(self):
        context = QgsRenderContext()

        self.assertEqual(context.devicePixelRatio(), 1)
        self.assertEqual(context.scaleFactor(), 1)

        properties = QgsScreenProperties()

        # updating with invalid properties shouldn't change anything
        context.setDevicePixelRatio(3)
        properties.updateRenderContextForScreen(context)
        self.assertEqual(context.devicePixelRatio(), 3)
        self.assertEqual(context.scaleFactor(), 1)

        properties.setDevicePixelRatio(2)
        properties.setPhysicalDpi(200)

        properties.updateRenderContextForScreen(context)

        self.assertEqual(context.devicePixelRatio(), 2)
        self.assertAlmostEqual(context.scaleFactor(), 7.87401, 3)


if __name__ == "__main__":
    unittest.main()
