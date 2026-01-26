"""QGIS unit tests for QgsPalLabeling: label rendering output to map canvas

From build dir, run: ctest -R PyQgsPalLabelingCanvas -V

See <qgis-src-dir>/tests/testdata/labeling/README.rst for description.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Larry Shaffer"
__date__ = "07/09/2013"
__copyright__ = "Copyright 2013, The QGIS Project"

import sys

from qgis.core import QgsVectorLayerSimpleLabeling

from test_qgspallabeling_base import TestQgsPalLabeling, runSuite
from test_qgspallabeling_tests import TestLineBase, TestPointBase, suiteTests


class TestCanvasBase(TestQgsPalLabeling):

    layer = None
    """:type: QgsVectorLayer"""

    @classmethod
    def setUpClass(cls):
        if not cls._BaseSetup:
            TestQgsPalLabeling.setUpClass()

    @classmethod
    def tearDownClass(cls):
        super().tearDownClass()
        cls.removeMapLayer(cls.layer)
        cls.layer = None

    def setUp(self):
        """Run before each test."""
        super().setUp()
        # ensure per test map settings stay encapsulated
        self._TestMapSettings = self.cloneMapSettings(self._MapSettings)

    def checkTest(self, **kwargs):
        self.layer.setLabeling(QgsVectorLayerSimpleLabeling(self.lyr))

        ms = self._MapSettings  # class settings
        if self._TestMapSettings is not None:
            ms = self._TestMapSettings  # per test settings

        self.assertTrue(
            self.render_map_settings_check(
                self._Test,
                self._Test,
                ms,
                self._Test,
                color_tolerance=0,
                allowed_mismatch=0,
                control_path_prefix="expected_" + self._TestGroupPrefix,
            )
        )


class TestCanvasBasePoint(TestCanvasBase):

    @classmethod
    def setUpClass(cls):
        TestCanvasBase.setUpClass()
        cls.layer = TestQgsPalLabeling.loadFeatureLayer("point")


class TestCanvasPoint(TestCanvasBasePoint, TestPointBase):

    def setUp(self):
        """Run before each test."""
        super().setUp()
        self.configTest("pal_canvas", "sp")


class TestCanvasBaseLine(TestCanvasBase):

    @classmethod
    def setUpClass(cls):
        TestCanvasBase.setUpClass()
        cls.layer = TestQgsPalLabeling.loadFeatureLayer("line")


class TestCanvasLine(TestCanvasBaseLine, TestLineBase):

    def setUp(self):
        """Run before each test."""
        super().setUp()
        self.configTest("pal_canvas_line", "sp")


if __name__ == "__main__":
    # NOTE: unless PAL_SUITE env var is set all test class methods will be run
    # SEE: test_qgspallabeling_tests.suiteTests() to define suite
    suite = ["TestCanvasPoint." + t for t in suiteTests()["sp_suite"]]
    res = runSuite(sys.modules[__name__], suite)
    sys.exit(not res.wasSuccessful())
