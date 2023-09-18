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

# Needed on Qt 5 so that the serialization of XML is consistent among all executions
os.environ['QT_HASH_SEED'] = '1'

from qgis.testing import unittest
from qgis.server import QgsBufferServerRequest, QgsBufferServerResponse
from qgis.core import QgsProject
from qgis.PyQt.QtGui import QImage


from test_qgsserver import QgsServerTestBase


class PyQgsServerWMSGetPrintMapTheme(QgsServerTestBase):
    """Tests for GetPrint layouts following map themes"""

    @classmethod
    def setUpClass(cls):

        super().setUpClass()

        project = QgsProject()
        assert (project.read(os.path.join(cls.temporary_path, 'qgis_server', 'test_project_mapthemes.qgs')))

        cls.project = project
        cls.polygon = 'POLYGON((7.09769689415099325 44.92867722467413216, 7.37818833364500737 44.92867722467413216, 7.37818833364500737 45.0714498943264914, 7.09769689415099325 45.0714498943264914, 7.09769689415099325 44.92867722467413216))'

    def test_wms_getprint_maptheme(self):
        """Test templates green and red have 2 layers: red and green
            template red: follow map theme red
            template green: follow map theme green
            template blank: no map theme
        """

        project = self.project

        params = {
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetPrint",
            "TEMPLATE": "blank",
            "FORMAT": "png",
            "LAYERS": "",
            "map0:EXTENT": "44.92867722467413216,7.097696894150993252,45.0714498943264914,7.378188333645007368",
            "map0:LAYERS": "red",
            "CRS": "EPSG:4326",
            "DPI": '72'
        }

        ######################################################
        # Template map theme tests, no HIGHLIGHT

        # blank template, specified layer is red
        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest('?' + '&'.join(["%s=%s" % i for i in params.items()]))
        self.server.handleRequest(request, response, project)

        image = QImage.fromData(response.body(), "PNG")
        self._assertRed(image.pixelColor(100, 100))

        # blank template, specified layer is green
        params["map0:LAYERS"] = "green"
        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest('?' + '&'.join(["%s=%s" % i for i in params.items()]))
        self.server.handleRequest(request, response, project)

        image = QImage.fromData(response.body(), "PNG")
        self._assertGreen(image.pixelColor(100, 100))

        # red template, no specified layers
        params["map0:LAYERS"] = ""
        params["TEMPLATE"] = "red"
        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest('?' + '&'.join(["%s=%s" % i for i in params.items()]))
        self.server.handleRequest(request, response, project)

        image = QImage.fromData(response.body(), "PNG")
        self._assertRed(image.pixelColor(100, 100))

        # green template, no specified layers
        params["map0:LAYERS"] = ""
        params["TEMPLATE"] = "green"
        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest('?' + '&'.join(["%s=%s" % i for i in params.items()]))
        self.server.handleRequest(request, response, project)

        image = QImage.fromData(response.body(), "PNG")
        self._assertGreen(image.pixelColor(100, 100))

        # green template, specified layer is red
        # This is a conflict situation: the green template map is set to follow green theme
        # but we tell the server to render the red layer, red is what we get.
        params["map0:LAYERS"] = "red"
        params["TEMPLATE"] = "green"
        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest('?' + '&'.join(["%s=%s" % i for i in params.items()]))
        self.server.handleRequest(request, response, project)

        image = QImage.fromData(response.body(), "PNG")
        self._assertRed(image.pixelColor(100, 100))

        # Same situation as above but LAYERS is not map0 prefixed
        params["LAYERS"] = "red"
        params["TEMPLATE"] = "green"
        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest('?' + '&'.join(["%s=%s" % i for i in params.items()]))
        self.server.handleRequest(request, response, project)

        image = QImage.fromData(response.body(), "PNG")
        self._assertRed(image.pixelColor(100, 100))

        # Same as above but we have a conflict situation: we pass both LAYERS
        # and map0:LAYERS, the second must prevail because it is more specific
        params["LAYERS"] = "red"
        params["map0:LAYERS"] = "green"
        params["TEMPLATE"] = "red"
        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest('?' + '&'.join(["%s=%s" % i for i in params.items()]))
        self.server.handleRequest(request, response, project)

        image = QImage.fromData(response.body(), "PNG")
        self._assertGreen(image.pixelColor(100, 100))

    def test_wms_getprint_maptheme_multiple_maps(self):
        """Test template points has 4 layers: points_black, points_red, points_green, points_blue
            the template has two maps (from top to bottom) map1 and map0 using
            respectively the 4points-red and 4points-green map themes
        """

        project = self.project

        # No LAYERS specified
        params = {
            'SERVICE': 'WMS',
            'VERSION': '1.3.0',
            'REQUEST': 'GetPrint',
            'TEMPLATE': 'points',
            'FORMAT': 'png',
            'map0:EXTENT': '44.66151222233335716,6.71202136069002187,45.25042454764368927,7.83398711866607833',
            'CRS': 'EPSG:4326',
            'DPI': '72',
            'map1:EXTENT': '44.66151222233335716,6.71202136069002187,45.25042454764368927,7.83398711866607833'
        }

        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest('?' + '&'.join(["%s=%s" % i for i in params.items()]))
        self.server.handleRequest(request, response, project)

        image = QImage.fromData(response.body(), "PNG")
        # Expected: green and red
        # map1 (top map)
        self._assertRed(image.pixelColor(325, 184))  # RED
        self._assertWhite(image.pixelColor(474, 184))  # GREEN
        self._assertWhite(image.pixelColor(332, 262))  # BLUE
        self._assertWhite(image.pixelColor(485, 258))  # BLACK
        # map0 (bottom map)
        self._assertWhite(image.pixelColor(315, 461))  # RED
        self._assertGreen(image.pixelColor(475, 473))  # GREEN
        self._assertWhite(image.pixelColor(329, 553))  # BLUE
        self._assertWhite(image.pixelColor(481, 553))  # BLACK

        # Black LAYERS
        params["LAYERS"] = "points_black"
        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest('?' + '&'.join(["%s=%s" % i for i in params.items()]))
        self.server.handleRequest(request, response, project)

        image = QImage.fromData(response.body(), "PNG")
        # Expected black
        # map1 (top map)
        self._assertWhite(image.pixelColor(325, 184))  # RED
        self._assertWhite(image.pixelColor(474, 184))  # GREEN
        self._assertWhite(image.pixelColor(332, 262))  # BLUE
        self._assertBlack(image.pixelColor(485, 258))  # BLACK
        # map0 (bottom map)
        self._assertWhite(image.pixelColor(315, 461))  # RED
        self._assertWhite(image.pixelColor(475, 473))  # GREEN
        self._assertWhite(image.pixelColor(329, 553))  # BLUE
        self._assertBlack(image.pixelColor(481, 553))  # BLACK

        # Black map0:LAYERS
        del params["LAYERS"]
        params["map0:LAYERS"] = "points_black"
        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest('?' + '&'.join(["%s=%s" % i for i in params.items()]))
        self.server.handleRequest(request, response, project)

        image = QImage.fromData(response.body(), "PNG")
        # Expected black on map0, green on map1
        # map1 (top map)
        self._assertRed(image.pixelColor(325, 184))  # RED
        self._assertWhite(image.pixelColor(474, 184))  # GREEN
        self._assertWhite(image.pixelColor(332, 262))  # BLUE
        self._assertWhite(image.pixelColor(485, 258))  # BLACK
        #  map0 (bottom map)
        self._assertWhite(image.pixelColor(315, 461))  # RED
        self._assertWhite(image.pixelColor(475, 473))  # GREEN
        self._assertWhite(image.pixelColor(329, 553))  # BLUE
        self._assertBlack(image.pixelColor(481, 553))  # BLACK

        # Conflicting information: Black LAYERS and Green map0:LAYERS
        # The second gets precedence on map0 while LAYERS is applied to map1
        params["map0:LAYERS"] = "points_blue"
        params["LAYERS"] = "points_black"
        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest('?' + '&'.join(["%s=%s" % i for i in params.items()]))
        self.server.handleRequest(request, response, project)

        image = QImage.fromData(response.body(), "PNG")
        # Expected green on map0, black on map1
        # map1 (top map)
        self._assertWhite(image.pixelColor(325, 184))  # RED
        self._assertWhite(image.pixelColor(474, 184))  # GREEN
        self._assertWhite(image.pixelColor(332, 262))  # BLUE
        self._assertBlack(image.pixelColor(485, 258))  # BLACK
        #  map0 (bottom map)
        self._assertWhite(image.pixelColor(315, 461))  # RED
        self._assertWhite(image.pixelColor(475, 473))  # GREEN
        self._assertBlue(image.pixelColor(329, 553))  # BLUE
        self._assertWhite(image.pixelColor(481, 553))  # BLACK

    def test_wms_getprint_maptheme_highlight(self):
        """Test templates green and red have 2 layers: red and green
            template red: follow map theme red
            template green: follow map theme green
            template blank: no map theme
        """

        project = self.project

        params = {
            'SERVICE': 'WMS',
            'VERSION': '1.3.0',
            'REQUEST': 'GetPrint',
            'TEMPLATE': 'blank',
            'FORMAT': 'png',
            'LAYERS': '',
            'map0:EXTENT': '44.92867722467413216,7.097696894150993252,45.0714498943264914,7.378188333645007368',
            'map0:LAYERS': 'red',
            'CRS': 'EPSG:4326',
            'DPI': '72',
            'map0:HIGHLIGHT_GEOM': self.polygon,
            'map0:HIGHLIGHT_SYMBOL': r'<StyledLayerDescriptor><UserStyle><FeatureTypeStyle><Rule><PolygonSymbolizer><Fill><CssParameter name="fill">%230000FF</CssParameter></Fill></PolygonSymbolizer></Rule></FeatureTypeStyle></UserStyle></StyledLayerDescriptor>'
        }

        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest('?' + '&'.join(["%s=%s" % i for i in params.items()]))
        self.server.handleRequest(request, response, project)

        image = QImage.fromData(response.body(), "PNG")
        self._assertBlue(image.pixelColor(100, 100))

        # Test highlight without layers
        params["TEMPLATE"] = "blank"
        params["map0:LAYERS"] = ""

        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest('?' + '&'.join(["%s=%s" % i for i in params.items()]))
        self.server.handleRequest(request, response, project)

        image = QImage.fromData(response.body(), "PNG")
        self._assertBlue(image.pixelColor(100, 100))

        # Test highlight on follow theme (issue GH #34178)
        params["TEMPLATE"] = "red"

        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest('?' + '&'.join(["%s=%s" % i for i in params.items()]))
        self.server.handleRequest(request, response, project)

        image = QImage.fromData(response.body(), "PNG")
        self._assertBlue(image.pixelColor(100, 100))

        # Test highlight on follow theme (issue GH #34178)
        params["TEMPLATE"] = "green"

        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest('?' + '&'.join(["%s=%s" % i for i in params.items()]))
        self.server.handleRequest(request, response, project)

        image = QImage.fromData(response.body(), "PNG")
        self._assertBlue(image.pixelColor(100, 100))

        # Test highlight on follow theme, but add LAYERS (issue GH #34178)
        params["TEMPLATE"] = "green"
        params["LAYERS"] = "red"

        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest('?' + '&'.join(["%s=%s" % i for i in params.items()]))
        self.server.handleRequest(request, response, project)

        image = QImage.fromData(response.body(), "PNG")
        self._assertBlue(image.pixelColor(100, 100))

    def test_wms_getprint_atlas_dd_theme_(self):
        """Test a template with atlas DD theme"""

        project = self.project

        # No LAYERS specified
        params = {
            'SERVICE': 'WMS',
            'VERSION': '1.3.0',
            'REQUEST': 'GetPrint',
            'TEMPLATE': 'data_defined_theme',
            'FORMAT': 'png',
            'CRS': 'EPSG:4326',
            'DPI': '72',
        }

        def _test_red():
            params['ATLAS_PK'] = '2'

            response = QgsBufferServerResponse()
            request = QgsBufferServerRequest('?' + '&'.join(["%s=%s" % i for i in params.items()]))
            self.server.handleRequest(request, response, project)

            image = QImage.fromData(response.body(), "PNG")
            # Expected: white and red
            self._assertRed(image.pixelColor(325, 184))
            self._assertWhite(image.pixelColor(685, 150))

        def _test_green():
            params['ATLAS_PK'] = '4'

            response = QgsBufferServerResponse()
            request = QgsBufferServerRequest('?' + '&'.join(["%s=%s" % i for i in params.items()]))
            self.server.handleRequest(request, response, project)

            image = QImage.fromData(response.body(), "PNG")
            # Expected: green and white
            self._assertGreen(image.pixelColor(325, 184))
            self._assertWhite(image.pixelColor(685, 150))

        # Alternate test to make sure nothing is cached
        _test_red()
        _test_green()
        _test_red()
        _test_green()


if __name__ == '__main__':
    unittest.main()
