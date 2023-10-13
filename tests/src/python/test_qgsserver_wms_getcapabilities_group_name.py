"""QGIS Unit tests for QgsServer GetCapabilities group name attribute exclusion.

From build dir, run: ctest -R PyQgsServerWMSGetMapSizeProject -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
__author__ = 'Tomas Straupis'
__date__ = '2023-10-13'
__copyright__ = 'Copyright 2023, The QGIS Project'

import os
import urllib.parse

from qgis.testing import unittest
from test_qgsserver import QgsServerTestBase


class TestQgsServerWMSGetCapabilities(QgsServerTestBase):
    """QGIS Server WMS Tests for GetFeatureInfo request"""

    # Set to True to re-generate reference files for this class
    regenerate_reference = False

    def setUp(self):
        super().setUp()
        # First project has "skiping name tag for groups" setting set to false (default)
        self.project_with_name = os.path.join(self.testdata_path, "wms_group_test1.qgz")
        # Second project has "skiping name tag for groups" setting set to true
        self.project_without_name = os.path.join(self.testdata_path, "wms_group_test2.qgz")

    def test_wms_getcapabilities_with(self):
        r = make_request(self, self.project_with_name)
        f = str(r).find('<Name>layer group</Name>')
        # attribute <name> should be specified for a layer group
        self.assertGreater(f, 0)

    def test_wms_getcapabilities_without(self):
        r = make_request(self, self.project_without_name)
        f = str(r).find('<Name>layer group</Name>')
        # attribute <name> should NOT be specified for a layer group
        self.assertEqual(f, -1)


def make_request(instance, project):
    qs = "?" + "&".join(["%s=%s" % i for i in list({
        "MAP": urllib.parse.quote(project),
        "SERVICE": "WMS",
        "REQUEST": "GetCapabilities"
    }.items())])
    r, h = instance._result(instance._execute_request(qs))
    return instance.strip_version_xmlns(r)


if __name__ == '__main__':
    unittest.main()
