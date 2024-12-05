"""QGIS Unit tests for QgsVectorLayer.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Tim Sutton"
__date__ = "20/08/2012"
__copyright__ = "Copyright 2012, The QGIS Project"

import os

from qgis.core import QgsVectorLayer
from qgis.gui import QgsGui
import unittest
from qgis.testing import start_app, QgisTestCase

from featuresourcetestbase import FeatureSourceTestCase
from utilities import unitTestDataPath

TEST_DATA_DIR = unitTestDataPath()

start_app()


class TestQgsVectorLayerShapefile(QgisTestCase, FeatureSourceTestCase):
    """
    Tests a vector layer against the feature source tests, using a real layer source (not a memory layer)
    """

    @classmethod
    def getSource(cls):
        vl = QgsVectorLayer(
            os.path.join(TEST_DATA_DIR, "provider", "shapefile.shp"), "test"
        )
        assert vl.isValid()
        return vl

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()
        QgsGui.editorWidgetRegistry().initEditors()
        # Create test layer for FeatureSourceTestCase
        cls.source = cls.getSource()

    @classmethod
    def tearDownClass(cls):
        cls.source = None
        super().tearDownClass()

    def treat_time_as_string(self):
        return True

    def treat_datetime_as_string(self):
        return True


if __name__ == "__main__":
    unittest.main()
