"""QGIS Unit tests for core additions

From build dir, run: ctest -R PyCoreAdditions -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Denis Rouzaud"
__date__ = "15.5.2018"
__copyright__ = "Copyright 2015, The QGIS Project"


from qgis.core import (
    Qgis,
    QgsMapLayer,
    QgsTolerance,
    metaEnumFromValue,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestCoreAdditions(QgisTestCase):

    def testMetaEnum(self):
        me = metaEnumFromValue(Qgis.MapToolUnit.Pixels)
        self.assertIsNotNone(me)
        self.assertEqual(me.valueToKey(Qgis.MapToolUnit.Pixels), "Pixels")

        # check that using same variable twice doesn't segfault
        me = metaEnumFromValue(Qgis.MapToolUnit.Pixels, QgsTolerance)
        self.assertIsNotNone(me)
        self.assertEqual(me.valueToKey(Qgis.MapToolUnit.Pixels), "Pixels")

        # do not raise error
        self.assertIsNone(metaEnumFromValue(1, QgsTolerance, False))

        # do not provide an int
        with self.assertRaises(TypeError):
            metaEnumFromValue(1)

        # QgsMapLayer.LayerType is not a Q_ENUM
        with self.assertRaises(ValueError):
            metaEnumFromValue(QgsMapLayer.LayerType)


if __name__ == "__main__":
    unittest.main()
