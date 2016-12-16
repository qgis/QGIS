""" QGIS test for server services
"""
from qgis.PyQt.QtCore import QBuffer, QIODevice, QTextStream
from qgis.testing import unittest
from qgis.core import QgsApplication
from qgis.server import (QgsServer,
                         QgsServiceRegistry, 
                         QgsService,
                         QgsServerRequest,
                         QgsServerResponse)


class Response(QgsServerResponse):

    def __init__( self ):
        QgsServerResponse.__init__(self)
        self._buffer = QBuffer()
        self._buffer.open(QIODevice.ReadWrite)


    def setReturnCode( self, code ):
        pass

    def setHeader( self, key, val ):
        pass

    def sendError( self, code, message ):
        pass

    def io(self):
        return self._buffer


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

    def test_register(self):

        reg = QgsServiceRegistry()

        myserv = MyService("STUFF", "1.0", "Hello world")

        reg.registerService( myserv )

        # Retrieve service
        request  = QgsServerRequest("http://DoStufff", QgsServerRequest.GetMethod)
        response = Response()

        service = reg.getService("STUFF")
        if service:
            service.executeRequest(request, response)
        
        io = response.io();
        io.seek(0)

        self.assertEqual(QTextStream(io).readLine(), "Hello world")

    def test_0_version_registration(self):

        reg     = QgsServiceRegistry()
        myserv1 = MyService("STUFF", "1.0", "Hello")
        myserv2 = MyService("STUFF", "1.1", "Hello")
   
        reg.registerService( myserv1 )
        reg.registerService( myserv2)

        service = reg.getService("STUFF")
        self.assertIsNotNone(service)
        self.assertEqual(service.version(), "1.1")

        service = reg.getService("STUFF", "2.0")
        self.assertIsNone(service)

    def test_1_unregister_services(self):

        reg  = QgsServiceRegistry()
        serv1 = MyService("STUFF", "1.0a", "Hello")
        serv2 = MyService("STUFF", "1.0b", "Hello")
        serv3 = MyService("STUFF", "1.0c", "Hello")

        reg.registerService(serv1)
        reg.registerService(serv2)
        reg.registerService(serv3)

        # Check we get the highest version
        service = reg.getService("STUFF")
        self.assertEqual( service.version(), "1.0c" )
        
        # Remove one service
        removed = reg.unregisterService("STUFF", "1.0c")
        self.assertEqual( removed, 1 )

        # Check that we get the highest version
        service = reg.getService("STUFF")
        self.assertEqual( service.version(), "1.0b" )
        
         # Remove all services
        removed = reg.unregisterService("STUFF")
        self.assertEqual( removed, 2 )

         # Check that there is no more services available
        service = reg.getService("STUFF")
        self.assertIsNone(service)
       
    def test_2_server_initialization(self):

        qgisapp = QgsApplication([], False)
        server  = QgsServer()

        # Check that our 'SampleService is registered
        iface   = server.serverInterface()
        service = iface.serviceRegistry().getService('SampleService')
        
        self.assertIsNotNone(service)



if __name__ == '__main__':
    unittest.main()
