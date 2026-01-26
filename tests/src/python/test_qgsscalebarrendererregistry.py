"""QGIS Unit tests for QgsScaleBarRendererRegistry

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "20/03/2020"
__copyright__ = "Copyright 2020, The QGIS Project"


from qgis.core import QgsScaleBarRenderer, QgsScaleBarRendererRegistry
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestRenderer(QgsScaleBarRenderer):

    def id(self):
        return "test"

    def sortKey(self):
        return 45

    def visibleName(self):
        return "TesT"

    def clone(self):
        return TestRenderer()


class TestQgsScaleBarRendererRegistry(QgisTestCase):

    def testRegistry(self):
        registry = QgsScaleBarRendererRegistry()
        self.assertTrue(registry.renderers())
        for f in registry.renderers():
            self.assertEqual(registry.renderer(f).id(), f)
            self.assertEqual(
                registry.visibleName(f), registry.renderer(f).visibleName()
            )
            self.assertEqual(registry.sortKey(f), registry.renderer(f).sortKey())

        self.assertIsNone(registry.renderer("bad"))
        self.assertFalse(registry.visibleName("bad"))
        self.assertFalse(registry.sortKey("bad"))

        self.assertIn("Double Box", registry.renderers())

        registry.addRenderer(TestRenderer())
        self.assertIn("test", registry.renderers())
        self.assertTrue(isinstance(registry.renderer("test"), TestRenderer))
        self.assertEqual(registry.visibleName("test"), "TesT")
        self.assertEqual(registry.sortKey("test"), 45)

        registry.removeRenderer("test")

        self.assertNotIn("test", registry.renderers())
        self.assertIsNone(registry.renderer("test"))
        self.assertFalse(registry.visibleName("test"))

        registry.removeRenderer("test")


if __name__ == "__main__":
    unittest.main()
