"""QGIS Unit tests for QgsServer plugins and filters.


From build dir, run: ctest -R PyQgsServerPlugins -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
__author__ = 'Alessandro Pasotti'
__date__ = '22/04/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'

import os

import osgeo.gdal  # NOQA
from qgis.core import QgsMessageLog
from qgis.server import QgsServer, QgsService
from qgis.testing import unittest

from test_qgsserver import QgsServerTestBase
from utilities import unitTestDataPath

# Strip path and content length because path may vary
RE_STRIP_UNCHECKABLE = br'MAP=[^"]+|Content-Length: \d+'
RE_ATTRIBUTES = br'[^>\s]+=[^>\s]+'


class TestQgsServerPlugins(QgsServerTestBase):

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

    def test_pluginfilters(self):
        """Test python plugins filters"""
        try:
            from qgis.server import QgsServerFilter
        except ImportError:
            print("QGIS Server plugins are not compiled. Skipping test")
            return

        class SimpleHelloFilter(QgsServerFilter):

            def requestReady(self):
                QgsMessageLog.logMessage("SimpleHelloFilter.requestReady")

            def sendResponse(self):
                QgsMessageLog.logMessage("SimpleHelloFilter.sendResponse")

            def onSendResponse(self):
                QgsMessageLog.logMessage("SimpleHelloFilter.onSendResponse")

            def responseComplete(self):
                request = self.serverInterface().requestHandler()
                params = request.parameterMap()
                QgsMessageLog.logMessage("SimpleHelloFilter.responseComplete")
                if params.get('SERVICE', '').upper() == 'SIMPLE':
                    request.clear()
                    request.setResponseHeader('Content-type', 'text/plain')
                    request.appendBody(b'Hello from SimpleServer!')

        serverIface = self.server.serverInterface()
        filter = SimpleHelloFilter(serverIface)
        serverIface.registerFilter(filter, 100)
        # Get registered filters
        self.assertEqual(filter, serverIface.filters()[100][0])

        # global to be modified inside plugin filters
        globals()['status_code'] = 0
        # body to be checked inside plugin filters
        globals()['body2'] = None
        # headers to be checked inside plugin filters
        globals()['headers2'] = None

        # Register some more filters
        class Filter1(QgsServerFilter):

            def responseComplete(self):
                request = self.serverInterface().requestHandler()
                params = request.parameterMap()
                if params.get('SERVICE', '').upper() == 'SIMPLE':
                    request.appendBody(b'Hello from Filter1!')

        class Filter2(QgsServerFilter):

            def responseComplete(self):
                request = self.serverInterface().requestHandler()
                params = request.parameterMap()
                if params.get('SERVICE', '').upper() == 'SIMPLE':
                    request.appendBody(b'Hello from Filter2!')

        class Filter3(QgsServerFilter):
            """Test get and set status code"""

            def responseComplete(self):
                global status_code
                request = self.serverInterface().requestHandler()
                request.setStatusCode(999)
                status_code = request.statusCode()

        class Filter4(QgsServerFilter):
            """Body getter"""

            def responseComplete(self):
                global body2
                request = self.serverInterface().requestHandler()
                body2 = request.body()

        class Filter5(QgsServerFilter):
            """Body setter, clear body, keep headers"""

            def responseComplete(self):
                global headers2
                request = self.serverInterface().requestHandler()
                request.clearBody()
                headers2 = request.responseHeaders()
                request.appendBody(b'new body, new life!')

        filter1 = Filter1(serverIface)
        filter2 = Filter2(serverIface)
        filter3 = Filter3(serverIface)
        filter4 = Filter4(serverIface)
        serverIface.registerFilter(filter1, 101)
        serverIface.registerFilter(filter2, 200)
        serverIface.registerFilter(filter2, 100)
        serverIface.registerFilter(filter3, 300)
        serverIface.registerFilter(filter4, 400)
        self.assertIn(filter2, serverIface.filters()[100])
        self.assertEqual(filter1, serverIface.filters()[101][0])
        self.assertEqual(filter2, serverIface.filters()[200][0])
        header, body = (_v for _v in self._execute_request('?service=simple'))
        response = header + body
        expected = b'Content-Length: 62\nContent-type: text/plain\n\nHello from SimpleServer!Hello from Filter1!Hello from Filter2!'
        self.assertEqual(response, expected)

        # Check status code
        self.assertEqual(status_code, 999)

        # Check body getter from filter
        self.assertEqual(body2, b'Hello from SimpleServer!Hello from Filter1!Hello from Filter2!')

        # Check that the bindings for complex type QgsServerFiltersMap are working
        filters = {100: [filter, filter2], 101: [filter1], 200: [filter2]}
        serverIface.setFilters(filters)
        self.assertIn(filter, serverIface.filters()[100])
        self.assertIn(filter2, serverIface.filters()[100])
        self.assertEqual(filter1, serverIface.filters()[101][0])
        self.assertEqual(filter2, serverIface.filters()[200][0])
        header, body = self._execute_request('?service=simple')
        response = header + body
        expected = b'Content-Length: 62\nContent-type: text/plain\n\nHello from SimpleServer!Hello from Filter1!Hello from Filter2!'
        self.assertEqual(response, expected)

        # Now, re-run with body setter
        filter5 = Filter5(serverIface)
        serverIface.registerFilter(filter5, 500)
        header, body = self._execute_request('?service=simple')
        response = header + body
        expected = b'Content-Length: 19\nContent-type: text/plain\n\nnew body, new life!'
        self.assertEqual(response, expected)
        self.assertEqual(headers2, {'Content-type': 'text/plain'})

    def test_configpath(self):
        """ Test plugin can read confif path
        """
        try:
            from qgis.core import QgsProject
            from qgis.server import QgsServerFilter
        except ImportError:
            print("QGIS Server plugins are not compiled. Skipping test")
            return

        d = unitTestDataPath('qgis_server_accesscontrol') + '/'
        self.projectPath = os.path.join(d, "project.qgs")
        self.server = QgsServer()

        # global to be modified inside plugin filters
        globals()['configFilePath2'] = None

        class Filter0(QgsServerFilter):
            """Body setter, clear body, keep headers"""

            def requestReady(self):
                global configFilePath2
                configFilePath2 = self.serverInterface().configFilePath()

        serverIface = self.server.serverInterface()
        serverIface.registerFilter(Filter0(serverIface), 100)

        # Test using MAP
        self._execute_request(f'?service=simple&MAP={self.projectPath}')

        # Check config file path
        self.assertEqual(configFilePath2, self.projectPath)

        # Reset result
        globals()['configFilePath2'] = None

        # Test with prqject as argument
        project = QgsProject()
        project.read(self.projectPath)

        self._execute_request_project('?service=simple', project=project)

        # Check config file path
        self.assertEqual(configFilePath2, project.fileName())

    def test_exceptions(self):
        """Test that plugin filter Python exceptions can be caught"""

        try:
            from qgis.server import QgsServerFilter
        except ImportError:
            print("QGIS Server plugins are not compiled. Skipping test")
            return

        class FilterBroken(QgsServerFilter):

            def responseComplete(self):
                raise Exception("There was something very wrong!")

        serverIface = self.server.serverInterface()
        filter1 = FilterBroken(serverIface)
        filters = {100: [filter1]}
        serverIface.setFilters(filters)
        header, body = self._execute_request('')
        self.assertEqual(body, b'Internal Server Error')
        serverIface.setFilters({})

    def test_get_path(self):
        """Test get url and path from plugins"""

        try:
            from qgis.server import QgsServerFilter
        except ImportError:
            print("QGIS Server plugins are not compiled. Skipping test")
            return

        class Filter1(QgsServerFilter):

            def responseComplete(self):
                handler = self.serverInterface().requestHandler()
                self.url = handler.url()
                self.path = handler.path()

        serverIface = self.server.serverInterface()
        filter1 = Filter1(serverIface)
        filters = {100: [filter1]}
        serverIface.setFilters(filters)
        header, body = self._execute_request('http://myserver/mypath/?myparam=1')
        self.assertEqual(filter1.url, 'http://myserver/mypath/?myparam=1')
        self.assertEqual(filter1.path, '/mypath/')

        serverIface.setFilters({})

    def test_streaming_pipeline(self):
        """ Test streaming pipeline propagation
        """
        try:
            from qgis.core import QgsProject
            from qgis.server import QgsServerFilter
        except ImportError:
            print("QGIS Server plugins are not compiled. Skipping test")
            return

        # create a service for streaming data
        class StreamedService(QgsService):

            def __init__(self):
                super().__init__()
                self._response = b"Should never appear"
                self._name = "TestStreamedService"
                self._version = "1.0"

            def name(self):
                return self._name

            def version(self):
                return self._version

            def executeRequest(self, request, response, project):
                response.setStatusCode(206)
                response.write(self._response)
                response.flush()

        class Filter1(QgsServerFilter):

            def onRequestReady(self):
                request = self.serverInterface().requestHandler()
                return self.propagate

            def onProjectReady(self):
                request = self.serverInterface().requestHandler()
                return self.propagate

            def onSendResponse(self):
                request = self.serverInterface().requestHandler()
                request.clearBody()
                request.appendBody(b'A')
                request.sendResponse()
                request.appendBody(b'B')
                request.sendResponse()
                # Stop propagating
                return self.propagate

            def onResponseComplete(self):
                request = self.serverInterface().requestHandler()
                request.appendBody(b'C')
                return self.propagate

        # Methods should be called only if filter1 propagate
        class Filter2(QgsServerFilter):
            def __init__(self, iface):
                super().__init__(iface)
                self.request_ready = False
                self.project_ready = False

            def onRequestReady(self):
                request = self.serverInterface().requestHandler()
                self.request_ready = True
                return True

            def onProjectReady(self):
                request = self.serverInterface().requestHandler()
                self.project_ready = True
                return True

            def onSendResponse(self):
                request = self.serverInterface().requestHandler()
                request.appendBody(b'D')
                return True

            def onResponseComplete(self):
                request = self.serverInterface().requestHandler()
                request.appendBody(b'E')
                return True

        # Methods to manage propagate filter
        class Filter3(QgsServerFilter):
            def __init__(self, iface, propagate_filter):
                super().__init__(iface)
                self.propagate_filter = propagate_filter
                self.step_to_stop_propagate = None

            def onRequestReady(self):
                request = self.serverInterface().requestHandler()
                if self.step_to_stop_propagate == 'onRequestReady':
                    self.propagate_filter.propagate = False
                return True

            def onProjectReady(self):
                request = self.serverInterface().requestHandler()
                if self.step_to_stop_propagate == 'onProjectReady':
                    self.propagate_filter.propagate = False
                return True

            def onSendResponse(self):
                request = self.serverInterface().requestHandler()
                if self.step_to_stop_propagate == 'onSendResponse':
                    self.propagate_filter.propagate = False
                return True

            def onResponseComplete(self):
                request = self.serverInterface().requestHandler()
                if self.step_to_stop_propagate == 'onResponseComplete':
                    self.propagate_filter.propagate = False
                return True

        serverIface = self.server.serverInterface()
        serverIface.setFilters({})

        service0 = StreamedService()

        reg = serverIface.serviceRegistry()
        reg.registerService(service0)

        filter1 = Filter1(serverIface)
        filter2 = Filter2(serverIface)
        serverIface.registerFilter(filter1, 200)
        serverIface.registerFilter(filter2, 300)

        project = QgsProject()

        # Test no propagation
        filter1.propagate = False
        _, body = self._execute_request_project(f'?service={service0.name()}', project=project)
        self.assertFalse(filter2.request_ready)
        self.assertFalse(filter2.project_ready)
        self.assertEqual(body, b'ABC')

        # Test with propagation
        filter1.propagate = True
        _, body = self._execute_request_project(f'?service={service0.name()}', project=project)
        self.assertTrue(filter2.request_ready)
        self.assertTrue(filter2.project_ready)
        self.assertEqual(body, b'ABDCE')

        # Manage propagation
        filter3 = Filter3(serverIface, filter1)
        serverIface.registerFilter(filter3, 100)

        # Stop at onResponseComplete
        filter1.propagate = True
        filter2.request_ready = False
        filter2.project_ready = False
        filter3.step_to_stop_propagate = 'onResponseComplete'
        _, body = self._execute_request_project(f'?service={service0.name()}', project=project)
        self.assertTrue(filter2.request_ready)
        self.assertTrue(filter2.project_ready)
        self.assertEqual(body, b'ABDC')

        # Stop at onSendResponse
        filter1.propagate = True
        filter2.request_ready = False
        filter2.project_ready = False
        filter3.step_to_stop_propagate = 'onSendResponse'
        _, body = self._execute_request_project(f'?service={service0.name()}', project=project)
        self.assertTrue(filter2.request_ready)
        self.assertTrue(filter2.project_ready)
        self.assertEqual(body, b'ABC')

        # Stop at onProjectReady
        filter1.propagate = True
        filter2.request_ready = False
        filter2.project_ready = False
        filter3.step_to_stop_propagate = 'onProjectReady'
        _, body = self._execute_request_project(f'?service={service0.name()}', project=project)
        self.assertTrue(filter2.request_ready)
        self.assertFalse(filter2.project_ready)
        self.assertEqual(body, b'ABC')

        # Stop at onRequestReady
        filter1.propagate = True
        filter2.request_ready = False
        filter2.project_ready = False
        filter3.step_to_stop_propagate = 'onRequestReady'
        _, body = self._execute_request_project(f'?service={service0.name()}', project=project)
        self.assertFalse(filter2.request_ready)
        self.assertFalse(filter2.project_ready)
        self.assertEqual(body, b'ABC')

        serverIface.setFilters({})
        reg.unregisterService(service0.name(), service0.version())


if __name__ == '__main__':
    unittest.main()
