"""

From build dir, run: ctest -R PyQgsServerModules -V

***************************************************************************
    test_qgsserver_modules.py
    ---------------------
    Date                 : December 2016
    Copyright            : (C) 2016 by David Marteau
    Email                : david at innophi dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "David Marteau"
__date__ = "December 2016"
__copyright__ = "(C) 2016, David Marteau"

""" QGIS test for server services
"""
import os

from qgis.PyQt.QtCore import QBuffer, QIODevice
from qgis.core import QgsApplication
from qgis.server import QgsServer, QgsServerResponse, QgsService
from qgis.testing import unittest

from utilities import unitTestDataPath


class Response(QgsServerResponse):

    def __init__(self):
        QgsServerResponse.__init__(self)
        self._buffer = QBuffer()
        self._buffer.open(QIODevice.OpenModeFlag.ReadWrite)

    def setStatusCode(self, code):
        pass

    def setHeader(self, key, val):
        pass

    def sendError(self, code, message):
        pass

    def io(self):
        return self._buffer


class MyService(QgsService):

    def __init__(self, name, version, response):
        QgsService.__init__(self)
        self._response = response
        self._name = name
        self._version = version

    def name(self):
        return self._name

    def version(self):
        return self._version

    def executeRequest(self, request, response):
        response.setReturnCode(201)
        response.write(self._response)


class TestModules(unittest.TestCase):
    """ """

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls.app = QgsApplication([], False)

    @classmethod
    def tearDownClass(cls):
        cls.app.exitQgis()
        super().tearDownClass()

    def setUp(self):
        """Create the server instance"""
        self.testdata_path = unitTestDataPath("qgis_server") + "/"

        d = unitTestDataPath("qgis_server_accesscontrol") + "/"
        self.projectPath = os.path.join(d, "project.qgs")

        # Clean env just to be sure
        env_vars = ["QUERY_STRING", "QGIS_PROJECT_FILE"]
        for ev in env_vars:
            try:
                del os.environ[ev]
            except KeyError:
                pass
        self.server = QgsServer()

    def test_dummy(self):
        """
        A dummy test to avoid empty test suite
        reporting failures on some unittest versions
        """
        pass


if __name__ == "__main__":
    unittest.main()
