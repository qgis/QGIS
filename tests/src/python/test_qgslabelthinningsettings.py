"""QGIS Unit tests for QgsLabelThinningSettings

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "2019-12-07"
__copyright__ = "Copyright 2019, The QGIS Project"


from qgis.core import QgsLabelThinningSettings, QgsPalLayerSettings
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsLabelThinningSettings(QgisTestCase):

    def test_thinning_settings(self):
        """
        Test thinning settings
        """
        settings = QgsLabelThinningSettings()
        settings.setLimitNumberLabelsEnabled(True)
        self.assertTrue(settings.limitNumberOfLabelsEnabled())
        settings.setLimitNumberLabelsEnabled(False)
        self.assertFalse(settings.limitNumberOfLabelsEnabled())

        settings.setMaximumNumberLabels(12)
        self.assertEqual(settings.maximumNumberLabels(), 12)

        settings.setMinimumFeatureSize(13.5)
        self.assertEqual(settings.minimumFeatureSize(), 13.5)

        # check that compatibility code works
        pal_settings = QgsPalLayerSettings()
        pal_settings.limitNumLabels = True
        self.assertTrue(pal_settings.limitNumLabels)
        self.assertTrue(pal_settings.thinningSettings().limitNumberOfLabelsEnabled())
        pal_settings.limitNumLabels = False
        self.assertFalse(pal_settings.limitNumLabels)
        self.assertFalse(pal_settings.thinningSettings().limitNumberOfLabelsEnabled())

        pal_settings.maxNumLabels = 22
        self.assertEqual(pal_settings.maxNumLabels, 22)
        self.assertEqual(pal_settings.thinningSettings().maximumNumberLabels(), 22)

        pal_settings.minFeatureSize = 4.6
        self.assertEqual(pal_settings.minFeatureSize, 4.6)
        self.assertEqual(pal_settings.thinningSettings().minimumFeatureSize(), 4.6)


if __name__ == "__main__":
    unittest.main()
