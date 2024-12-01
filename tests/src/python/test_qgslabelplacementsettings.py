"""QGIS Unit tests for QgsLabelPlacementSettings

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "2024-02-02"
__copyright__ = "Copyright 2024, The QGIS Project"


from qgis.core import Qgis, QgsPalLayerSettings
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsLabelPlacementSettings(QgisTestCase):

    def test_placement_settings(self):
        """
        Test placement settings
        """

        # check that compatibility code works
        pal_settings = QgsPalLayerSettings()
        pal_settings.displayAll = True
        self.assertTrue(pal_settings.displayAll)
        self.assertEqual(
            pal_settings.placementSettings().overlapHandling(),
            Qgis.LabelOverlapHandling.AllowOverlapIfRequired,
        )
        self.assertTrue(pal_settings.placementSettings().allowDegradedPlacement())
        pal_settings.displayAll = False
        self.assertFalse(pal_settings.displayAll)
        self.assertEqual(
            pal_settings.placementSettings().overlapHandling(),
            Qgis.LabelOverlapHandling.PreventOverlap,
        )
        self.assertFalse(pal_settings.placementSettings().allowDegradedPlacement())


if __name__ == "__main__":
    unittest.main()
