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

import os

from qgis.server import QgsServerProjectUtils
from qgis.core import QgsProject
from qgis.testing import unittest
from utilities import unitTestDataPath


class TestQgsServerProjectUtils(unittest.TestCase):

    def setUp(self):
        self.testdata_path = unitTestDataPath('qgis_server_project') + '/'

        self.prj = QgsProject()
        self.prjPath = os.path.join(self.testdata_path, "project.qgs")
        self.prj.read(self.prjPath)

        self.prj2 = QgsProject()
        self.prj2Path = os.path.join(self.testdata_path, "project2.qgs")
        self.prj2.read(self.prj2Path)

    def tearDown(self):
        pass

    def test_size(self):
        self.assertEqual(QgsServerProjectUtils.wmsMaxWidth(self.prj), 400)
        self.assertEqual(QgsServerProjectUtils.wmsMaxHeight(self.prj), 500)

    def test_url(self):
        self.assertEqual(QgsServerProjectUtils.wmsServiceUrl(self.prj), "my_wms_advertised_url")
        self.assertEqual(QgsServerProjectUtils.wcsServiceUrl(self.prj), "my_wcs_advertised_url")
        self.assertEqual(QgsServerProjectUtils.wfsServiceUrl(self.prj), "my_wfs_advertised_url")

    def test_wmsuselayerids(self):
        self.assertEqual(QgsServerProjectUtils.wmsUseLayerIds(self.prj), False)
        self.assertEqual(QgsServerProjectUtils.wmsUseLayerIds(self.prj2), True)

    def test_wmsrestrictedlayers(self):
        # retrieve entry from project
        result = QgsServerProjectUtils.wmsRestrictedLayers(self.prj)
        expected = []
        expected.append('points')  # layer
        expected.append('group1')  # local group
        expected.append('groupEmbedded')  # embedded group

        self.assertListEqual(sorted(expected), sorted(result))

    def test_wfslayersids(self):
        # retrieve entry from project
        result = QgsServerProjectUtils.wfsLayerIds(self.prj)

        expected = []
        expected.append('multipoint20170309173637804')  # from embedded group
        expected.append('points20170309173738552')  # local layer
        expected.append('polys20170309173913723')  # from local group

        self.assertEqual(expected, result)

    def test_wcslayersids(self):
        # retrieve entry from project
        result = QgsServerProjectUtils.wcsLayerIds(self.prj)

        expected = []
        expected.append('landsat20170313142548073')

        self.assertEqual(expected, result)


if __name__ == '__main__':
    unittest.main()
