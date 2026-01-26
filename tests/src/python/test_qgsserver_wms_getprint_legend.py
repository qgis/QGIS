"""QGIS Unit tests for QgsServer WMS GetPrint legend.

From build dir, run: ctest -R PyQgsServerWMSGetPrintLegend -V


.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""

__author__ = "Sebastien Peillet"
__date__ = "30/06/2021"
__copyright__ = "Copyright 2021, The QGIS Project"

import os
import shutil

# Needed on Qt 5 so that the serialization of XML is consistent among all executions
os.environ["QT_HASH_SEED"] = "1"

from qgis.core import QgsProject
from qgis.PyQt.QtCore import QTemporaryDir
from qgis.PyQt.QtGui import QImage
from qgis.server import QgsBufferServerRequest, QgsBufferServerResponse
from qgis.testing import unittest
from test_qgsserver import QgsServerTestBase
from utilities import unitTestDataPath


class PyQgsServerWMSGetPrintLegend(QgsServerTestBase):
    """Tests for issue GH #42036 QGIS Server GetPrint:
    QGIS server print behaves inconsistently regarding legend content"""

    def test_wms_getprint_legend(self):
        """Test project has 2 layer: red and green and five templates:
        red: follow map theme red
        green: follow map theme green
        blank: no map theme
        full: follow map theme full with both layer
        falsegreen : follow map theme falsegreen (visible layer : green but with blue style)
        """

        tmp_dir = QTemporaryDir()
        shutil.copyfile(
            os.path.join(unitTestDataPath("qgis_server"), "test_project_legend.qgs"),
            os.path.join(tmp_dir.path(), "test_project_legend.qgs"),
        )
        shutil.copyfile(
            os.path.join(unitTestDataPath("qgis_server"), "test_project_legend.gpkg"),
            os.path.join(tmp_dir.path(), "test_project_legend.gpkg"),
        )

        project = QgsProject()
        self.assertTrue(
            project.read(os.path.join(tmp_dir.path(), "test_project_legend.qgs"))
        )

        params = {
            "SERVICE": "WMS",
            "VERSION": "1.3",
            "REQUEST": "GetPrint",
            "TEMPLATE": "blank",
            "FORMAT": "png",
            "LAYERS": "",
            "map0:EXTENT": "778000,5600000,836000,5650000",
            "map0:SCALE": "281285",
            "map0:LAYERS": "red",
            "CRS": "EPSG:3857",
            "DPI": "72",
        }

        ######################################################
        # Template legend tests
        # Legend symbol are displayed at coordinates :
        #   First item  : 600 x , 40 y
        #   Second item : 600 x , 60 y

        # blank template, no theme, no LAYERS, specified map0:LAYERS is red
        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest(
            "?" + "&".join(["%s=%s" % i for i in params.items()])
        )
        self.server.handleRequest(request, response, project)

        image = QImage.fromData(response.body(), "PNG")
        # Only the red layer is displayed, there is no second item
        self._assertRed(image.pixelColor(600, 40))
        self._assertWhite(image.pixelColor(600, 60))

        # blank template, no LAYERS, specified map0:LAYERS is green
        params["map0:LAYERS"] = "green"
        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest(
            "?" + "&".join(["%s=%s" % i for i in params.items()])
        )
        self.server.handleRequest(request, response, project)

        image = QImage.fromData(response.body(), "PNG")
        # Only the green layer is displayed, there is no second item
        self._assertGreen(image.pixelColor(600, 40))
        self._assertWhite(image.pixelColor(600, 60))

        # blank template
        params["map0:LAYERS"] = ""
        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest(
            "?" + "&".join(["%s=%s" % i for i in params.items()])
        )
        self.server.handleRequest(request, response, project)

        image = QImage.fromData(response.body(), "PNG")
        # Only the red layer is displayed, there is no second item
        self._assertRed(image.pixelColor(600, 40))
        self._assertGreen(image.pixelColor(600, 60))

        # red template, red theme, specified map0:LAYERS is red
        params["TEMPLATE"] = "red"
        params["map0:LAYERS"] = "red"
        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest(
            "?" + "&".join(["%s=%s" % i for i in params.items()])
        )
        self.server.handleRequest(request, response, project)

        image = QImage.fromData(response.body(), "PNG")
        # Only the red layer is displayed, there is no second item
        self._assertRed(image.pixelColor(600, 40))
        self._assertWhite(image.pixelColor(600, 60))

        # red template, red theme, specified map0:LAYERS is green
        params["map0:LAYERS"] = "green"
        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest(
            "?" + "&".join(["%s=%s" % i for i in params.items()])
        )
        self.server.handleRequest(request, response, project)

        image = QImage.fromData(response.body(), "PNG")
        # Only the green layer is displayed, there is no second item
        self._assertGreen(image.pixelColor(600, 40))
        self._assertWhite(image.pixelColor(600, 60))

        # red template, red theme, no map0:LAYERS
        params["map0:LAYERS"] = ""
        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest(
            "?" + "&".join(["%s=%s" % i for i in params.items()])
        )
        self.server.handleRequest(request, response, project)

        image = QImage.fromData(response.body(), "PNG")
        # Only the red layer is displayed, there is no second item
        self._assertRed(image.pixelColor(600, 40))
        self._assertWhite(image.pixelColor(600, 60))

        # green template, green theme, specified map0:LAYERS is red
        params["TEMPLATE"] = "green"
        params["map0:LAYERS"] = "red"
        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest(
            "?" + "&".join(["%s=%s" % i for i in params.items()])
        )
        self.server.handleRequest(request, response, project)

        image = QImage.fromData(response.body(), "PNG")
        # Only the red layer is displayed, there is no second item
        self._assertRed(image.pixelColor(600, 40))
        self._assertWhite(image.pixelColor(600, 60))

        # green template, green theme, specified map0:LAYERS is green
        params["map0:LAYERS"] = "green"
        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest(
            "?" + "&".join(["%s=%s" % i for i in params.items()])
        )
        self.server.handleRequest(request, response, project)

        image = QImage.fromData(response.body(), "PNG")
        # Only the green layer is displayed, there is no second item
        self._assertGreen(image.pixelColor(600, 40))
        self._assertWhite(image.pixelColor(600, 60))

        # green template, green theme, no map0:LAYERS
        params["map0:LAYERS"] = ""
        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest(
            "?" + "&".join(["%s=%s" % i for i in params.items()])
        )
        self.server.handleRequest(request, response, project)

        image = QImage.fromData(response.body(), "PNG")
        # Only the green layer is displayed, there is no second item
        self._assertGreen(image.pixelColor(600, 40))
        self._assertWhite(image.pixelColor(600, 60))

        # full template, full theme, specified map0:LAYERS is red
        params["TEMPLATE"] = "full"
        params["map0:LAYERS"] = "red"
        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest(
            "?" + "&".join(["%s=%s" % i for i in params.items()])
        )
        self.server.handleRequest(request, response, project)

        image = QImage.fromData(response.body(), "PNG")
        # Only the red layer is displayed, there is no second item
        self._assertRed(image.pixelColor(600, 40))
        self._assertWhite(image.pixelColor(600, 60))

        # full template, full theme, specified map0:LAYERS is green
        params["map0:LAYERS"] = "green"
        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest(
            "?" + "&".join(["%s=%s" % i for i in params.items()])
        )
        self.server.handleRequest(request, response, project)

        image = QImage.fromData(response.body(), "PNG")
        # Only the green layer is displayed, there is no second item
        self._assertGreen(image.pixelColor(600, 40))
        self._assertWhite(image.pixelColor(600, 60))

        # full template, full theme, no map0:LAYERS
        params["map0:LAYERS"] = ""
        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest(
            "?" + "&".join(["%s=%s" % i for i in params.items()])
        )
        self.server.handleRequest(request, response, project)

        image = QImage.fromData(response.body(), "PNG")
        # Both red and green layers are displayed
        self._assertRed(image.pixelColor(600, 40))
        self._assertGreen(image.pixelColor(600, 60))

        # falsegreen template, falsegreen theme (green layer is blue), specified map0:LAYERS is red
        params["TEMPLATE"] = "falsegreen"
        params["map0:LAYERS"] = "red"
        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest(
            "?" + "&".join(["%s=%s" % i for i in params.items()])
        )
        self.server.handleRequest(request, response, project)

        image = QImage.fromData(response.body(), "PNG")
        # Only the red layer is displayed, there is no second item
        self._assertRed(image.pixelColor(600, 40))
        self._assertWhite(image.pixelColor(600, 60))

        # full template, full theme, specified map0:LAYERS is green
        params["map0:LAYERS"] = "green"
        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest(
            "?" + "&".join(["%s=%s" % i for i in params.items()])
        )
        self.server.handleRequest(request, response, project)

        image = QImage.fromData(response.body(), "PNG")
        # Only the green layer (in blue) is displayed, there is no second item
        self._assertBlue(image.pixelColor(600, 40))
        self._assertWhite(image.pixelColor(600, 60))

        # full template, full theme, no map0:LAYERS
        params["map0:LAYERS"] = ""
        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest(
            "?" + "&".join(["%s=%s" % i for i in params.items()])
        )
        self.server.handleRequest(request, response, project)

        image = QImage.fromData(response.body(), "PNG")
        # Only the green layer (in blue) is displayed, there is no second item
        self._assertBlue(image.pixelColor(600, 40))
        self._assertWhite(image.pixelColor(600, 60))


if __name__ == "__main__":
    unittest.main()
