"""QGIS Unit tests for QgsMapSettingsUtils.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import unittest

from qgis.core import (
    QgsCoordinateReferenceSystem,
    QgsCoordinateTransformContext,
    QgsMapLayerUtils,
    QgsMapSettingsUtils,
    QgsRectangle,
)
from qgis.testing import QgisTestCase, start_app

start_app()


class TestQgsMapSettingsUtils(QgisTestCase):
    def test_is_valid_extent(self):
        """
        Test isValidExtent logic
        """
        # empty rect => not valid
        self.assertFalse(QgsMapSettingsUtils.isValidExtent(QgsRectangle()))
        self.assertFalse(QgsMapSettingsUtils.isValidExtent(QgsRectangle(0, 0, 0, 0)))

        # non-finite rect
        self.assertFalse(
            QgsMapSettingsUtils.isValidExtent(QgsRectangle(0, 0, float("inf"), 10))
        )
        self.assertFalse(
            QgsMapSettingsUtils.isValidExtent(QgsRectangle(0, float("-inf"), 10, 10))
        )
        self.assertFalse(
            QgsMapSettingsUtils.isValidExtent(QgsRectangle(0, 0, float("nan"), 10))
        )

        # width > 1 and height > 1
        self.assertTrue(QgsMapSettingsUtils.isValidExtent(QgsRectangle(0, 0, 10, 10)))
        # width < 1, but height > 1 using an X range that WOULD fail the precision check if evaluated
        self.assertTrue(
            QgsMapSettingsUtils.isValidExtent(QgsRectangle(1.0, 0.0, 1.0 + 1e-14, 2.0))
        )
        # height < 1, but width > 1
        self.assertTrue(
            QgsMapSettingsUtils.isValidExtent(QgsRectangle(0.0, 1.0, 2.0, 1.0 + 1e-14))
        )

        # small extent (< 1) with adequate double precision
        self.assertTrue(
            QgsMapSettingsUtils.isValidExtent(QgsRectangle(0.1, 0.1, 0.6, 0.6))
        )

        # small extent centered around 0
        self.assertTrue(
            QgsMapSettingsUtils.isValidExtent(QgsRectangle(-0.25, -0.25, 0.25, 0.25))
        )

        # invalid when the width is less than 1e-12
        self.assertFalse(
            QgsMapSettingsUtils.isValidExtent(QgsRectangle(1.0, 0.0, 1.0 + 1e-14, 0.5))
        )

        # invalid when the height is less than 1e-12
        self.assertFalse(
            QgsMapSettingsUtils.isValidExtent(QgsRectangle(0.0, 1.0, 0.5, 1.0 + 1e-14))
        )

        # invalid when both the width and height are less than 1e-12
        self.assertFalse(
            QgsMapSettingsUtils.isValidExtent(
                QgsRectangle(1.0, 1.0, 1.0 + 1e-14, 1.0 + 1e-14)
            )
        )


if __name__ == "__main__":
    unittest.main()
