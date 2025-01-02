"""QGIS Unit tests for QgsServer GetCapabilities/GetMap group name attribute exclusion.

From build dir, run: ctest -R PyQgsServerWMSGetCapabilitiesGroupName -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""

__author__ = "Tomas Straupis"
__date__ = "2023-10-13"
__copyright__ = "Copyright 2023, The QGIS Project"

import os
import urllib.parse

from qgis.testing import unittest
from test_qgsserver import QgsServerTestBase


class TestQgsServerWMSGetCapabilities(QgsServerTestBase):
    """QGIS Server WMS Tests for GetCapabilities/GetMap with group name skipping"""

    # Set to True to re-generate reference files for this class
    regenerate_reference = False

    def setUp(self):
        super().setUp()
        # First project has "skipping name tag for groups" setting set to false (default)
        self.project_with_name = os.path.join(self.testdata_path, "wms_group_test1.qgs")
        # Second project has "skipping name tag for groups" setting set to true
        self.project_without_name = os.path.join(
            self.testdata_path, "wms_group_test2.qgs"
        )

    def test_wms_getcapabilities_with(self):
        r = make_capabilities_request(self, self.project_with_name)
        self.assertIn(
            b"<Name>layer_group</Name>",
            r,
            "attribute <name> should be specified for a layer group",
        )

    def test_wms_getmap_with(self):
        r = make_map_request(self, self.project_with_name)
        self.assertNotIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be no server exception for a layer group in LAYERS=",
        )

    def test_wms_getcapabilities_without(self):
        r = make_capabilities_request(self, self.project_without_name)
        self.assertNotIn(
            b"<Name>layer_group</Name>",
            r,
            "attribute <name> should NOT be specified for a layer group",
        )

    def test_wms_getmap_without(self):
        r = make_map_request(self, self.project_without_name)
        self.assertIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should a server exception for a layer group in LAYERS=",
        )


def make_capabilities_request(instance, project):
    qs = "?" + "&".join(
        [
            "%s=%s" % i
            for i in list(
                {
                    "MAP": urllib.parse.quote(project),
                    "SERVICE": "WMS",
                    "REQUEST": "GetCapabilities",
                }.items()
            )
        ]
    )
    r, h = instance._result(instance._execute_request(qs))
    return instance.strip_version_xmlns(r)


def make_map_request(instance, project):
    qs = "?" + "&".join(
        [
            "%s=%s" % i
            for i in list(
                {
                    "MAP": urllib.parse.quote(project),
                    "SERVICE": "WMS",
                    "REQUEST": "GetMap",
                    "BBOX": "6053181%2C594460%2C6053469%2C595183",
                    "CRS": "EPSG%3A3346",
                    "WIDTH": "1500",
                    "HEIGHT": "600",
                    "LAYERS": "layer_group",
                    "VERSION": "1.3.0",
                }.items()
            )
        ]
    )
    r, h = instance._result(instance._execute_request(qs))
    return instance.strip_version_xmlns(r)


if __name__ == "__main__":
    unittest.main()
