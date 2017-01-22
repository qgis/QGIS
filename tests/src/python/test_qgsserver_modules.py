""" QGIS test for server services
"""
import os
from qgis.PyQt.QtCore import QBuffer, QIODevice, QTextStream
from qgis.testing import unittest
from qgis.core import QgsApplication
from qgis.server import (QgsServer,
                         QgsServiceRegistry,
                         QgsService,
                         QgsServerRequest,
                         QgsServerResponse)

from utilities import unitTestDataPath


class Response(QgsServerResponse):

    def __init__(self):
        QgsServerResponse.__init__(self)
        self._buffer = QBuffer()
        self._buffer.open(QIODevice.ReadWrite)

    def setReturnCode(self, code):
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

        url = request.url()

        response.setReturnCode(201)
        response.write(self._response)


class TestModules(unittest.TestCase):
    """ 
    """

    @classmethod
    def setUpClass(cls):
        cls.app = QgsApplication([], False)

    @classmethod
    def tearDownClass(cls):
        cls.app.exitQgis()

    def setUp(self):
        """Create the server instance"""
        self.testdata_path = unitTestDataPath('qgis_server') + '/'

        d = unitTestDataPath('qgis_server_accesscontrol') + '/'
        self.projectPath = os.path.join(d, "project.qgs")

        # Clean env just to be sure
        env_vars = ['QUERY_STRING', 'QGIS_PROJECT_FILE']
        for ev in env_vars:
            try:
                del os.environ[ev]
            except KeyError:
                pass
        self.server = QgsServer()

    def test_modules(self):
        """ Tests that modules are loaded """

        # Check that our 'SampleService is registered
        iface = self.server.serverInterface()
        service = iface.serviceRegistry().getService('SampleService')

        self.assertIsNotNone(service)


if __name__ == '__main__':
    unittest.main()
