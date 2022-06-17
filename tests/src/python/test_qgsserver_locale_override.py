# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsServer Locale Override Options.

From build dir, run: ctest -R PyQgsServerLocaleOverride -V

.. note:: This test needs env vars to be set before the server is
          configured for the first time, for this
          reason it cannot run as a test case of another server
          test.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
__author__ = 'Alessandro Pasotti'
__date__ = '01/04/2019'
__copyright__ = 'Copyright 2019, The QGIS Project'

import os

# Needed on Qt 5 so that the serialization of XML is consistent among all
# executions
os.environ['QT_HASH_SEED'] = '1'

from utilities import (
    unitTestDataPath,
)
from qgis.testing import unittest

from test_qgsserver_wms import TestQgsServerWMSTestBase
from qgis.core import QgsProject, QgsFontUtils
from qgis.server import QgsServer


class TestQgsServerWMSLocaleOverride(TestQgsServerWMSTestBase):
    """QGIS Server WMS Tests for GetFeatureInfo request"""

    # Set to True to re-generate reference files for this class
    regenerate_reference = False

    @classmethod
    def setUpClass(self):

        os.environ['QGIS_SERVER_OVERRIDE_SYSTEM_LOCALE'] = 'EN_us'
        os.environ['QGIS_SERVER_SHOW_GROUP_SEPARATOR'] = '0'

        super().setUpClass()

        # d = os.path.join(self.temporary_path, 'qgis_server')
        # self.projectPath = os.path.join(d, "project.qgs")

    def testGetFeatureInfoThousandSeparator(self):

        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=testlayer_thousands&styles=&' +
                                 'info_format=text%2Fxml&transparent=true&' +
                                 'width=600&height=400&srs=EPSG%3A3857&bbox=913190.6389747962%2C' +
                                 '5606005.488876367%2C913235.426296057%2C5606035.347090538&' +
                                 'query_layers=testlayer_thousands&X=190&Y=320',
                                 'wms_getfeatureinfo-thousands-text-xml',
                                 project='test_project_gfi_thousands.qgs',)


if __name__ == '__main__':
    unittest.main()
