# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsServer.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'René-Luc DHONT'
__date__ = '19/07/2018'
__copyright__ = 'Copyright 2015, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

print('CTEST_FULL_OUTPUT')

import qgis  # NOQA

import os
import urllib.request
import urllib.parse
import urllib.error
import tempfile
import hashlib

from qgis.testing import unittest
from utilities import unitTestDataPath
from qgis.server import QgsServer, QgsServerCacheFilter, QgsServerRequest, QgsBufferServerRequest, QgsBufferServerResponse
from qgis.core import QgsApplication, QgsFontUtils
from qgis.PyQt.QtCore import QFile, QByteArray
from qgis.PyQt.QtXml import QDomDocument


class PyServerCache(QgsServerCacheFilter):

    """ Used to have restriction access """

    # Be able to deactivate the access control to have a reference point
    _active = False

    def __init__(self, server_iface):
        super(QgsServerCacheFilter, self).__init__(server_iface)
        self._cache_dir = os.path.join(tempfile.gettempdir(), "qgs_server_cache")
        if not os.path.exists(self._cache_dir):
            os.mkdir(self._cache_dir)

    def getCachedDocument(self, project, request, key):
        m = hashlib.md5()
        paramMap = request.parameters()
        urlParam = "&".join(["%s=%s" % (k, paramMap[k]) for k in paramMap.keys()])
        m.update(urlParam.encode('utf8'))

        if not os.path.exists(os.path.join(self._cache_dir, m.hexdigest() + ".xml")):
            return QByteArray()

        doc = QDomDocument(m.hexdigest() + ".xml")
        with open(os.path.join(self._cache_dir, m.hexdigest() + ".xml"), "r") as f:
            statusOK, errorStr, errorLine, errorColumn = doc.setContent(f.read(), True)
            if not statusOK:
                print("Could not read or find the contents document. Error at line %d, column %d:\n%s" % (errorLine, errorColumn, errorStr))
                return QByteArray()

        return doc.toByteArray()

    def setCachedDocument(self, doc, project, request, key):
        m = hashlib.md5()
        paramMap = request.parameters()
        urlParam = "&".join(["%s=%s" % (k, paramMap[k]) for k in paramMap.keys()])
        m.update(urlParam.encode('utf8'))
        with open(os.path.join(self._cache_dir, m.hexdigest() + ".xml"), "w") as f:
            f.write(doc.toString())
        return os.path.exists(os.path.join(self._cache_dir, m.hexdigest() + ".xml"))


class TestQgsServerCacheManager(unittest.TestCase):

    @classmethod
    def _handle_request(cls, qs, requestMethod=QgsServerRequest.GetMethod, data=None):
        if data is not None:
            data = data.encode('utf-8')
        request = QgsBufferServerRequest(qs, requestMethod, {}, data)
        response = QgsBufferServerResponse()
        cls._server.handleRequest(request, response)
        headers = []
        rh = response.headers()
        rk = sorted(rh.keys())
        for k in rk:
            headers.append(("%s: %s" % (k, rh[k])).encode('utf-8'))
        return b"\n".join(headers) + b"\n\n", bytes(response.body())

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        cls._app = QgsApplication([], False)
        cls._server = QgsServer()
        cls._handle_request("")
        cls._server_iface = cls._server.serverInterface()
        cls._servercache = PyServerCache(cls._server_iface)
        cls._server_iface.registerServerCache(cls._servercache, 100)

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        filelist = [f for f in os.listdir(cls._servercache._cache_dir) if f.endswith(".xml")]
        for f in filelist:
            os.remove(os.path.join(cls._servercache._cache_dir, f))
        del cls._server
        cls._app.exitQgis

    def _result(self, data):
        headers = {}
        for line in data[0].decode('UTF-8').split("\n"):
            if line != "":
                header = line.split(":")
                self.assertEqual(len(header), 2, line)
                headers[str(header[0])] = str(header[1]).strip()

        return data[1], headers

    def _execute_request(self, qs, requestMethod=QgsServerRequest.GetMethod, data=None):
        request = QgsBufferServerRequest(qs, requestMethod, {}, data)
        response = QgsBufferServerResponse()
        self._server.handleRequest(request, response)
        headers = []
        rh = response.headers()
        rk = sorted(rh.keys())
        for k in rk:
            headers.append(("%s: %s" % (k, rh[k])).encode('utf-8'))
        return b"\n".join(headers) + b"\n\n", bytes(response.body())

    def setUp(self):
        """Create the server instance"""
        self.fontFamily = QgsFontUtils.standardTestFontFamily()
        QgsFontUtils.loadStandardTestFonts(['All'])

        d = unitTestDataPath('qgis_server_accesscontrol') + '/'
        self._project_path = os.path.join(d, "project.qgs")

    def test_getcapabilities(self):
        project = self._project_path
        assert os.path.exists(project), "Project file not found: " + project

        query_string = '?MAP=%s&SERVICE=WMS&VERSION=1.3.0&REQUEST=%s' % (urllib.parse.quote(project), 'GetCapabilities')
        header, body = self._execute_request(query_string)

        query_string = '?MAP=%s&SERVICE=WFS&VERSION=1.1.0&REQUEST=%s' % (urllib.parse.quote(project), 'GetCapabilities')
        header, body = self._execute_request(query_string)

        query_string = '?MAP=%s&SERVICE=WCS&VERSION=1.0.0&REQUEST=%s' % (urllib.parse.quote(project), 'GetCapabilities')
        header, body = self._execute_request(query_string)


if __name__ == "__main__":
    unittest.main()
