""" QGIS test for server services
"""
from qgis.PyQt.QtCore import QBuffer, QIODevice, QTextStream
from qgis.testing import unittest
from qgis.server import (QgsServiceRegistry, 
                         QgsService,
                         QgsServerRequest,
                         QgsServerResponse)

from qgis.core import QgsApplication

class Response(QgsServerResponse):

    def __init__( self ):
        QgsServerResponse.__init__(self)
        self._buffer = QBuffer()
        self._buffer.open(QIODevice.ReadWrite)


    def setReturnCode( self, code ):
        pass

    def setHeader( self, key, val ):
        pass

    def clearHeader( self, key ):
        pass

    def sendError( self, code, message ):
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
        self._name     = name
        self._version  = version

    def name(self):
        return self._name

    def version(self):
        return self._version

    def executeRequest( self, request, response ):
        
        url = request.url()

        response.setReturnCode(201)
        response.write(self._response)


class TestServices(unittest.TestCase):
    """ 
    """

    @classmethod
    def setUpClass(cls):
        cls.app = QgsApplication([], False)

    @classmethod
    def tearDownClass(cls):
        cls.app.exitQgis()

    def test_register(self):

        reg = QgsServiceRegistry()

        myserv = MyService("TEST", "1.0", "Hello world")

        reg.registerService( myserv )

        # Retrieve service
        request  = QgsServerRequest("http://DoStufff", QgsServerRequest.GetMethod)
        response = Response()

        service = reg.getService("TEST")
        if service:
            service.executeRequest(request, response)
       
        io = response.io();
        io.seek(0)

        self.assertEqual(QTextStream(io).readLine(), "Hello world")

    def test_0_version_registration(self):

        reg     = QgsServiceRegistry()
        myserv1 = MyService("TEST", "1.0", "Hello")
        myserv2 = MyService("TEST", "1.1", "Hello")
   
        reg.registerService( myserv1 )
        reg.registerService( myserv2)

        service = reg.getService("TEST")
        self.assertIsNotNone(service)
        self.assertEqual(service.version(), "1.1")

        service = reg.getService("TEST", "2.0")
        self.assertIsNone(service)

    def test_1_unregister_services(self):

        reg  = QgsServiceRegistry()
        serv1 = MyService("TEST", "1.0a", "Hello")
        serv2 = MyService("TEST", "1.0b", "Hello")
        serv3 = MyService("TEST", "1.0c", "Hello")

        reg.registerService(serv1)
        reg.registerService(serv2)
        reg.registerService(serv3)

        # Check we get the highest version
        service = reg.getService("TEST")
        self.assertEqual( service.version(), "1.0c" )
        
        # Remove one service
        removed = reg.unregisterService("TEST", "1.0c")
        self.assertEqual( removed, 1 )

        # Check that we get the highest version
        service = reg.getService("TEST")
        self.assertEqual( service.version(), "1.0b" )
        
         # Remove all services
        removed = reg.unregisterService("TEST")
        self.assertEqual( removed, 2 )

         # Check that there is no more services available
        service = reg.getService("TEST")
        self.assertIsNone(service)


if __name__ == '__main__':
    unittest.main()
