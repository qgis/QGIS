"""QGIS Unit tests for QgsIconUtils

From build dir, run: ctest -R QgsIconUtils -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from qgis.core import QgsApplication, QgsFields, QgsIconUtils
from qgis.PyQt.QtCore import QMetaType, QSize
from qgis.PyQt.QtGui import QIcon
from qgis.testing import QgisTestCase, start_app, unittest

start_app()


class TestQgsIconUtils(QgisTestCase):
    @classmethod
    def control_path_prefix(cls):
        return "icon_utils"

    def test_add_overlay(self):
        overlay_path = QgsApplication.iconPath(
            "/field_indicators/mIndicatorFieldDomain.svg"
        )

        icon = QgsFields.iconForFieldType(QMetaType.Type.QString)
        self.assertFalse(icon.isNull())

        size = QSize(32, 32)
        res = QgsIconUtils.addOverlay(icon, overlay_path, size)
        self.assertFalse(res.isNull())

        image = res.pixmap(size).toImage()
        self.assertEqual(image.size(), size)
        self.assertTrue(
            self.image_check(
                "icon_overlay_string",
                "icon_overlay_string",
                image,
                use_checkerboard_background=True,
            )
        )


if __name__ == "__main__":
    unittest.main()
