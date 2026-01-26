"""

From build dir, run: ctest -R PyQgsServerServices -V

***************************************************************************
    test_qgsserver_services.py
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
from qgis.PyQt.QtCore import QBuffer, QIODevice, QTextStream
from qgis.core import QgsApplication
from qgis.server import (
    QgsServerRequest,
    QgsServerResponse,
    QgsService,
    QgsServiceRegistry,
)
from qgis.testing import unittest


class Response(QgsServerResponse):

    def __init__(self):
        QgsServerResponse.__init__(self)
        self._buffer = QBuffer()
        self._buffer.open(QIODevice.OpenModeFlag.ReadWrite)

    def setStatusCode(self, code):
        self.code = code

    def statusCode(self):
        return self.code

    def setHeader(self, key, val):
        pass

    def clearHeader(self, key):
        pass

    def sendError(self, code, message):
        pass

    def io(self):
        return self._buffer

    def finish(self):
        pass

    def flush(self):
        pass

    def clear(self):
        pass


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
        response.setStatusCode(201)
        response.write(self._response)


class TestServices(unittest.TestCase):
    """ """

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls.app = QgsApplication([], False)

    @classmethod
    def tearDownClass(cls):
        cls.app.exitQgis()
        super().tearDownClass()

    def test_register(self):

        reg = QgsServiceRegistry()

        myserv = MyService("TEST", "1.0", "Hello world")

        reg.registerService(myserv)

        # Retrieve service
        request = QgsServerRequest("http://DoStufff", QgsServerRequest.GetMethod)
        response = Response()

        service = reg.getService("TEST")
        if service:
            service.executeRequest(request, response)

        io = response.io()
        io.seek(0)

        self.assertEqual(QTextStream(io).readLine(), "Hello world")
        self.assertEqual(response.statusCode(), 201)

    def test_0_version_registration(self):

        reg = QgsServiceRegistry()
        myserv11 = MyService("TEST", "1.1", "Hello")
        myserv10 = MyService("TEST", "1.0", "Hello")

        reg.registerService(myserv11)
        reg.registerService(myserv10)

        service = reg.getService("TEST")
        self.assertIsNotNone(service)
        self.assertEqual(service.version(), "1.1")

        service = reg.getService("TEST", "2.0")
        self.assertIsNotNone(service)
        self.assertEqual(service.version(), "1.1")

        service = reg.getService("TEST", "1.0")
        self.assertIsNotNone(service)
        self.assertEqual(service.version(), "1.0")

    def test_1_unregister_services(self):

        reg = QgsServiceRegistry()
        serv1 = MyService("TEST", "1.0a", "Hello")
        serv2 = MyService("TEST", "1.0b", "Hello")
        serv3 = MyService("TEST", "1.0c", "Hello")

        reg.registerService(serv1)
        reg.registerService(serv2)
        reg.registerService(serv3)

        # Check we get the default version
        service = reg.getService("TEST")
        self.assertEqual(service.version(), "1.0a")

        # Remove one service
        removed = reg.unregisterService("TEST", "1.0a")
        self.assertEqual(removed, 1)

        # Check that we get the highest version
        service = reg.getService("TEST")
        self.assertEqual(service.version(), "1.0c")

        # Remove all services
        removed = reg.unregisterService("TEST")
        self.assertEqual(removed, 2)

        # Check that there is no more services available
        service = reg.getService("TEST")
        self.assertIsNone(service)


if __name__ == "__main__":
    unittest.main()
