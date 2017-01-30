# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsServerProject.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
__author__ = 'Paul Blottiere'
__date__ = '26/12/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os

from qgis.server import QgsServerProjectUtils
from qgis.core import QgsApplication, QgsProject
from qgis.testing import unittest
from utilities import unitTestDataPath


class TestQgsServerProjectUtils(unittest.TestCase):

    def setUp(self):
        self.testdata_path = unitTestDataPath('qgis_server_project') + '/'

        self.prj = QgsProject()
        prjPath = os.path.join(self.testdata_path, "project.qgs")
        self.prj.setFileName(prjPath)
        self.prj.read()

    def tearDown(self):
        pass

    def test_size(self):
        self.assertEqual(QgsServerProjectUtils.wmsMaxWidth(self.prj), 400)
        self.assertEqual(QgsServerProjectUtils.wmsMaxHeight(self.prj), 500)

    def test_url(self):
        self.assertEqual(QgsServerProjectUtils.wmsServiceUrl(self.prj), "my_wms_advertised_url")
        self.assertEqual(QgsServerProjectUtils.wcsServiceUrl(self.prj), "my_wcs_advertised_url")
        self.assertEqual(QgsServerProjectUtils.wfsServiceUrl(self.prj), "my_wfs_advertised_url")

if __name__ == '__main__':
    unittest.main()
