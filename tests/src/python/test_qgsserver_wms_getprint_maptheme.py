# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsServer WMS GetPrint map theme.

From build dir, run: ctest -R PyQgsServerWMSGetPrintMapTheme -V


.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
__author__ = 'Alessandro Pasotti'
__date__ = '24/05/2021'
__copyright__ = 'Copyright 2021, The QGIS Project'

import os
import shutil

# Needed on Qt 5 so that the serialization of XML is consistent among all executions
os.environ['QT_HASH_SEED'] = '1'

from qgis.testing import unittest
from qgis.server import QgsBufferServerRequest, QgsBufferServerResponse
from qgis.core import QgsProject
from qgis.PyQt.QtCore import QTemporaryDir
from qgis.PyQt.QtGui import QImage


from test_qgsserver import QgsServerTestBase
from utilities import unitTestDataPath


class PyQgsServerWMSGetPrintMapTheme(QgsServerTestBase):
    """Tests for issue GH #34178 QGIS Server GetPrint:
    HIGHLIGHT_GEOM is not printed if map layers are configured to follow a map theme"""

    def test_wms_getprint_maptheme(self):
        """Test project has 2 layer: red and green and three templates:
            red: follow map theme red
            green: follow map theme green
            blank: no map theme
        """

        tmp_dir = QTemporaryDir()
        shutil.copyfile(os.path.join(unitTestDataPath('qgis_server'), 'test_project_mapthemes.qgs'), os.path.join(tmp_dir.path(), 'test_project_mapthemes.qgs'))
        shutil.copyfile(os.path.join(unitTestDataPath('qgis_server'), 'test_project_mapthemes.gpkg'), os.path.join(tmp_dir.path(), 'test_project_mapthemes.gpkg'))

        project = QgsProject()
        self.assertTrue(project.read(os.path.join(tmp_dir.path(), 'test_project_mapthemes.qgs')))

        params = {
            "SERVICE": "WMS",
            "VERSION": "1.3",
            "REQUEST": "GetPrint",
            "TEMPLATE": "blank",
            "FORMAT": "png",
            "LAYERS": "",
            "map0:EXTENT": "44.92867722467413216,7.097696894150993252,45.0714498943264914,7.378188333645007368",
            "map0:LAYERS": "red",
            "CRS": "EPSG:4326",
            "DPI": '72'
        }

        polygon = 'POLYGON((7.09769689415099325 44.92867722467413216, 7.37818833364500737 44.92867722467413216, 7.37818833364500737 45.0714498943264914, 7.09769689415099325 45.0714498943264914, 7.09769689415099325 44.92867722467413216))'

        ######################################################
        # Template map theme tests, no HIGHLIGHT

        # blank template, specified layer is red
        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest('?' + '&'.join(["%s=%s" % i for i in params.items()]))
        self.server.handleRequest(request, response, project)

        image = QImage.fromData(response.body(), "PNG")
        color = image.pixelColor(100, 100)
        self.assertEqual(color.red(), 255)
        self.assertEqual(color.green(), 0)
        self.assertEqual(color.blue(), 0)

        # blank template, specified layer is green
        params["map0:LAYERS"] = "green"
        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest('?' + '&'.join(["%s=%s" % i for i in params.items()]))
        self.server.handleRequest(request, response, project)

        image = QImage.fromData(response.body(), "PNG")
        color = image.pixelColor(100, 100)
        self.assertEqual(color.red(), 0)
        self.assertEqual(color.green(), 255)
        self.assertEqual(color.blue(), 0)

        # red template, no specified layers
        params["map0:LAYERS"] = ""
        params["TEMPLATE"] = "red"
        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest('?' + '&'.join(["%s=%s" % i for i in params.items()]))
        self.server.handleRequest(request, response, project)

        image = QImage.fromData(response.body(), "PNG")
        color = image.pixelColor(100, 100)
        self.assertEqual(color.red(), 255)
        self.assertEqual(color.green(), 0)
        self.assertEqual(color.blue(), 0)

        # green template, no specified layers
        params["map0:LAYERS"] = ""
        params["TEMPLATE"] = "green"
        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest('?' + '&'.join(["%s=%s" % i for i in params.items()]))
        self.server.handleRequest(request, response, project)

        image = QImage.fromData(response.body(), "PNG")
        color = image.pixelColor(100, 100)
        self.assertEqual(color.red(), 0)
        self.assertEqual(color.green(), 255)
        self.assertEqual(color.blue(), 0)

        # green template, specified layer is red
        # This is a conflict situation: the green template map is set to follow green theme
        # but we tell the server to render the red layer, red is what we get.
        params["map0:LAYERS"] = "red"
        params["TEMPLATE"] = "green"
        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest('?' + '&'.join(["%s=%s" % i for i in params.items()]))
        self.server.handleRequest(request, response, project)

        image = QImage.fromData(response.body(), "PNG")
        color = image.pixelColor(100, 100)
        self.assertEqual(color.red(), 255)
        self.assertEqual(color.green(), 0)
        self.assertEqual(color.blue(), 0)

        ######################################################
        # Start HIGHLIGHT tests

        params["TEMPLATE"] = "blank"
        params["map0:LAYERS"] = "red"
        params["map0:HIGHLIGHT_GEOM"] = polygon
        params["map0:HIGHLIGHT_SYMBOL"] = r'<StyledLayerDescriptor><UserStyle><FeatureTypeStyle><Rule><PolygonSymbolizer><Fill><CssParameter name="fill">%230000FF</CssParameter></Fill></PolygonSymbolizer></Rule></FeatureTypeStyle></UserStyle></StyledLayerDescriptor>'

        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest('?' + '&'.join(["%s=%s" % i for i in params.items()]))
        self.server.handleRequest(request, response, project)

        image = QImage.fromData(response.body(), "PNG")
        color = image.pixelColor(100, 100)
        self.assertEqual(color.red(), 0)
        self.assertEqual(color.green(), 0)
        self.assertEqual(color.blue(), 255)

        # Test highlight without layers
        params["TEMPLATE"] = "blank"
        params["map0:LAYERS"] = ""

        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest('?' + '&'.join(["%s=%s" % i for i in params.items()]))
        self.server.handleRequest(request, response, project)

        image = QImage.fromData(response.body(), "PNG")
        color = image.pixelColor(100, 100)
        self.assertEqual(color.red(), 0)
        self.assertEqual(color.green(), 0)
        self.assertEqual(color.blue(), 255)

        # Test highlight on follow theme (issue GH #34178)
        params["TEMPLATE"] = "red"

        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest('?' + '&'.join(["%s=%s" % i for i in params.items()]))
        self.server.handleRequest(request, response, project)

        image = QImage.fromData(response.body(), "PNG")
        color = image.pixelColor(100, 100)
        self.assertEqual(color.red(), 0)
        self.assertEqual(color.green(), 0)
        self.assertEqual(color.blue(), 255)

        # Test highlight on follow theme (issue GH #34178)
        params["TEMPLATE"] = "green"

        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest('?' + '&'.join(["%s=%s" % i for i in params.items()]))
        self.server.handleRequest(request, response, project)

        image = QImage.fromData(response.body(), "PNG")
        color = image.pixelColor(100, 100)
        self.assertEqual(color.red(), 0)
        self.assertEqual(color.green(), 0)
        self.assertEqual(color.blue(), 255)

        # Test highlight on follow theme, but add LAYERS (issue GH #34178)
        params["TEMPLATE"] = "green"
        params["LAYERS"] = "red"

        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest('?' + '&'.join(["%s=%s" % i for i in params.items()]))
        self.server.handleRequest(request, response, project)

        image = QImage.fromData(response.body(), "PNG")
        color = image.pixelColor(100, 100)
        self.assertEqual(color.red(), 0)
        self.assertEqual(color.green(), 0)
        self.assertEqual(color.blue(), 255)


if __name__ == '__main__':
    unittest.main()
