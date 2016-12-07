""" QGIS test for server services
"""
from qgis.PyQt.QtCore import QBuffer, QIODevice, QTextStream
from qgis.testing import unittest
from qgis.server import (QgsServiceRegistry, 
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
    
    def __init__(self, response):
        QgsService.__init__(self)
        self._response = response

    def executeRequest( self, request, response ):
        
        url = request.url()

        response.setReturnCode(201)
        response.write(self._response)


class TestServices(unittest.TestCase):
    """ 
    """

    def test_register(self):

        reg = QgsServiceRegistry()

        myserv = MyService("Hello world")

        reg.registerService("STUFF", myserv, "1.0" )

        # Retrieve service
        request  = QgsServerRequest("http://DoStufff", QgsServerRequest.GetMethod)
        response = Response()

        service = reg.getService("STUFF")
        if service:
            service.executeRequest(request, response)
        
        io = response.io();
        io.seek(0)

        self.assertEquals(QTextStream(io).readLine(), "Hello world")

    def test_version_registration(self):

        reg     = QgsServiceRegistry()
        myserv1 = MyService("1.0")
        myserv2 = MyService("1.1")
   
        reg.registerService("STUFF", myserv1, myserv1._response)
        reg.registerService("STUFF", myserv2, myserv2._response)


        service = reg.getService("STUFF")
        self.assertIsNotNone(service)
        self.assertEquals(service._response, myserv2._response)

        service = reg.getService("STUFF", "2.0")
        self.assertIsNone(service)

if __name__ == '__main__':
    unittest.main()
