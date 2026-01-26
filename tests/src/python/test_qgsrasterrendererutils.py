"""QGIS Unit tests for QgsRasterRendererUtils.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "(C) 2020 by Nyall Dawson"
__date__ = "15/09/2020"
__copyright__ = "Copyright 2020, The QGIS Project"

from qgis.PyQt.QtCore import QTemporaryDir
from qgis.PyQt.QtGui import QColor
from qgis.core import QgsColorRampShader, QgsRasterRendererUtils
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsRasterRendererUtils(QgisTestCase):

    def testSaveRestoreColorMap(self):
        items = [
            QgsColorRampShader.ColorRampItem(5.5, QColor(255, 100, 120, 60), "my item"),
            QgsColorRampShader.ColorRampItem(15.7, QColor(150, 90, 220)),
            QgsColorRampShader.ColorRampItem(
                22, QColor(255, 0, 0, 100), "my, & ^ \"' item"
            ),
            QgsColorRampShader.ColorRampItem(
                25.5, QColor(255, 200, 220, 10), "my item 3"
            ),
        ]

        tmp_dir = QTemporaryDir()
        tmp_file = f"{tmp_dir.path()}/ramp.txt"

        self.assertTrue(
            QgsRasterRendererUtils.saveColorMapFile(
                tmp_file, items, QgsColorRampShader.Type.Interpolated
            )
        )
        res, read_items, type, errors = QgsRasterRendererUtils.parseColorMapFile("")
        self.assertFalse(res)

        res, read_items, type, errors = QgsRasterRendererUtils.parseColorMapFile(
            tmp_file
        )
        self.assertTrue(res)
        self.assertEqual(type, QgsColorRampShader.Type.Interpolated)
        self.assertFalse(errors)
        self.assertEqual([i.value for i in read_items], [i.value for i in items])
        self.assertEqual(
            [i.color.name() for i in read_items], [i.color.name() for i in items]
        )
        self.assertEqual(
            [i.label for i in read_items],
            ["my item", "Color entry 2", "my, & ^ \"' item", "my item 3"],
        )

        self.assertTrue(
            QgsRasterRendererUtils.saveColorMapFile(
                tmp_file, items, QgsColorRampShader.Type.Discrete
            )
        )
        res, read_items, type, errors = QgsRasterRendererUtils.parseColorMapFile(
            tmp_file
        )
        self.assertTrue(res)
        self.assertEqual(type, QgsColorRampShader.Type.Discrete)
        self.assertFalse(errors)

        self.assertTrue(
            QgsRasterRendererUtils.saveColorMapFile(
                tmp_file, items, QgsColorRampShader.Type.Exact
            )
        )
        res, read_items, type, errors = QgsRasterRendererUtils.parseColorMapFile(
            tmp_file
        )
        self.assertTrue(res)
        self.assertEqual(type, QgsColorRampShader.Type.Exact)
        self.assertFalse(errors)


if __name__ == "__main__":
    unittest.main()
