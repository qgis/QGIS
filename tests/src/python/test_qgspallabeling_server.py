"""QGIS unit tests for QgsPalLabeling: label rendering output via QGIS Server

From build dir, run: ctest -R PyQgsPalLabelingServer -V

See <qgis-src-dir>/tests/testdata/labeling/README.rst for description.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Larry Shaffer"
__date__ = "07/12/2013"
__copyright__ = "Copyright 2013, The QGIS Project"

import sys

from qgis.PyQt.QtGui import QImage
from qgis.core import QgsProject, QgsVectorLayerSimpleLabeling

from qgis.server import (
    QgsBufferServerRequest,
    QgsBufferServerResponse,
    QgsServer,
    QgsServerRequest,
)

from test_qgspallabeling_base import TestQgsPalLabeling, runSuite
from test_qgspallabeling_tests import TestLineBase, TestPointBase, suiteTests


class TestServerBase(TestQgsPalLabeling):

    _TestProj = None
    """:type: QgsProject"""
    _TestProjName = ""
    layer = None
    """:type: QgsVectorLayer"""
    params = dict()

    @classmethod
    def setUpClass(cls):
        if not cls._BaseSetup:
            TestQgsPalLabeling.setUpClass()

        cls.server = QgsServer()

        # noinspection PyArgumentList
        cls._TestProj = QgsProject()

        # the blue background (set via layer style) to match renderchecker's
        background_layer = TestQgsPalLabeling.loadFeatureLayer("background", True)
        if background_layer:
            cls._TestProj.addMapLayer(background_layer)

    def setUp(self):
        """Run before each test."""
        # web server stays up across all tests
        super().setUp()
        # ensure per test map settings stay encapsulated
        self._TestMapSettings = self.cloneMapSettings(self._MapSettings)

    # noinspection PyPep8Naming
    def get_request_params(self) -> str:
        # TODO: support other types of servers, besides WMS
        ms = self._TestMapSettings
        osize = ms.outputSize()
        dpi = str(int(ms.outputDpi()))
        lyrs = [str(layer.name()) for layer in ms.layers()]
        lyrs.reverse()
        return "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetMap",
                        # layer stacking order for rendering: bottom,to,top
                        "LAYERS": ",".join(lyrs),  # or 'name,name'
                        "STYLES": ",",
                        # authid str or QgsCoordinateReferenceSystem obj
                        "CRS": str(ms.destinationCrs().authid()),
                        # self.aoiExtent(),
                        "BBOX": str(ms.extent().toString(True).replace(" : ", ",")),
                        "FORMAT": "image/png",  # or: 'image/png; mode=8bit'
                        "WIDTH": str(osize.width()),
                        "HEIGHT": str(osize.height()),
                        "DPI": dpi,
                        "MAP_RESOLUTION": dpi,
                        "FORMAT_OPTIONS": f"dpi:{dpi}",
                        "TRANSPARENT": "FALSE",
                        "IgnoreGetMapUrl": "1",
                    }.items()
                )
            ]
        )

    def _result(self, data):
        headers = {}
        for line in data[0].decode("UTF-8").split("\n"):
            if line != "":
                header = line.split(":")
                self.assertEqual(len(header), 2, line)
                headers[str(header[0])] = str(header[1]).strip()

        return data[1], headers

    def _execute_request_project(
        self,
        qs: str,
        project: QgsProject,
        requestMethod=QgsServerRequest.GetMethod,
        data=None,
    ):
        request = QgsBufferServerRequest(qs, requestMethod, {}, data)
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, project)
        headers = []
        rh = response.headers()
        rk = sorted(rh.keys())
        for k in rk:
            headers.append((f"{k}: {rh[k]}").encode())
        return b"\n".join(headers) + b"\n\n", bytes(response.body())

    def checkTest(self, **kwargs):
        self.layer.setLabeling(QgsVectorLayerSimpleLabeling(self.lyr))

        headers, response = self._execute_request_project(
            self.get_request_params(), self._TestProj
        )
        rendered_image = QImage.fromData(response)
        self.assertFalse(rendered_image.isNull())

        self.assertTrue(
            self.image_check(
                self._Test,
                self._Test,
                rendered_image,
                self._Test,
                color_tolerance=0,
                allowed_mismatch=0,
                control_path_prefix="expected_" + self._TestGroupPrefix,
            )
        )


class TestServerBasePoint(TestServerBase):

    @classmethod
    def setUpClass(cls):
        TestServerBase.setUpClass()
        cls.layer = TestQgsPalLabeling.loadFeatureLayer("point")
        cls._TestProj.addMapLayer(cls.layer)


class TestServerPoint(TestServerBasePoint, TestPointBase):

    def setUp(self):
        super().setUp()
        self.configTest("pal_server", "sp")

    def test_partials_labels_disabled(self):
        # these are ALWAYS enabled for server
        pass


class TestServerVsCanvasPoint(TestServerBasePoint, TestPointBase):

    def setUp(self):
        super().setUp()
        self.configTest("pal_canvas", "sp")

    def test_partials_labels_disabled(self):
        # these are ALWAYS enabled for server
        pass


class TestServerBaseLine(TestServerBase):

    @classmethod
    def setUpClass(cls):
        TestServerBase.setUpClass()
        cls.layer = TestQgsPalLabeling.loadFeatureLayer("line")
        cls._TestProj.addMapLayer(cls.layer)


class TestServerLine(TestServerBaseLine, TestLineBase):

    def setUp(self):
        """Run before each test."""
        super().setUp()
        self.configTest("pal_server_line", "sp")


class TestServerVsCanvasLine(TestServerBaseLine, TestLineBase):

    def setUp(self):
        super().setUp()
        self.configTest("pal_canvas_line", "sp")


if __name__ == "__main__":
    # NOTE: unless PAL_SUITE env var is set all test class methods will be run
    # SEE: test_qgspallabeling_tests.suiteTests() to define suite
    suite = ["TestServerPoint." + t for t in suiteTests()["sp_suite"]] + [
        "TestServerVsCanvasPoint." + t for t in suiteTests()["sp_vs_suite"]
    ]
    res = runSuite(sys.modules[__name__], suite)
    sys.exit(not res.wasSuccessful())
