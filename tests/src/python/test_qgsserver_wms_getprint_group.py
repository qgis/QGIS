# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsServer WMS GetPrint.

From build dir, run: ctest -R PyQgsServerWMSGetPrintGroup -V


.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
__author__ = 'René-Luc DHONT'
__date__ = '23/06/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import os

# Needed on Qt 5 so that the serialization of XML is consistent among all executions
os.environ['QT_HASH_SEED'] = '1'

import urllib.parse

from qgis.testing import unittest

class TestQgsServerWMSGetPrintGroup(QgsServerTestBase):
    """QGIS Server WMS Tests for GetPrint group request"""

    def test_wms_getprint_group(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectGroupsPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetPrint",
            "TEMPLATE": "layoutA4",
            "FORMAT": "png",
            "map0:EXTENT": "-33626185.498,-13032965.185,33978427.737,16020257.031",
            "map0:LAYERS": "Country_Diagrams,Country_Labels,Country",
            "CRS": "EPSG:3857"
        }.items())])

        r_individual, h = self._result(self._execute_request(qs))

        # test reference image
        self._img_diff_error(r_individual, h, "WMS_GetPrint_Group")

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectGroupsPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetPrint",
            "TEMPLATE": "layoutA4",
            "FORMAT": "png",
            "map0:EXTENT": "-33626185.498,-13032965.185,33978427.737,16020257.031",
            "map0:LAYERS": "CountryGroup",
            "CRS": "EPSG:3857"
        }.items())])

        r_group, h = self._result(self._execute_request(qs))

        # Test group image
        self._img_diff_error(r_group, h, "WMS_GetPrint_Group")

        """ Debug check:
        f = open('grouped.png', 'wb+')
        f.write(r_group)
        f.close()
        f = open('individual.png', 'wb+')
        f.write(r_individual)
        f.close()
        #"""

        # This test is too strict, it can fail
        # self.assertEqual(r_individual, r_group, 'Individual layers query and group layers query results should be identical')


if __name__ == '__main__':
    unittest.main()
