# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsServer.

From build dir, run: ctest -R PyQgsServerCacheManager -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'René-Luc DHONT'
__date__ = '19/07/2018'
__copyright__ = 'Copyright 2015, The QGIS Project'

import qgis  # NOQA

import os
import urllib.request
import urllib.parse
import urllib.error
import tempfile
import hashlib

from qgis.testing import unittest
from utilities import unitTestDataPath
from qgis.server import QgsServer, QgsServerCacheFilter, QgsServerRequest, QgsBufferServerRequest, \
    QgsBufferServerResponse
from qgis.core import QgsApplication, QgsFontUtils, QgsProject
from qgis.PyQt.QtCore import QIODevice, QFile, QByteArray, QBuffer, QSize
from qgis.PyQt.QtGui import QImage
from qgis.PyQt.QtXml import QDomDocument

from test_qgsserver import QgsServerTestBase


class PyServerCache(QgsServerCacheFilter):
    """ Used to have restriction access """

    # Be able to deactivate the access control to have a reference point
    _active = False

    def __init__(self, server_iface):
        super(QgsServerCacheFilter, self).__init__(server_iface)

        self._cache_dir = os.path.join(tempfile.gettempdir(), "qgs_server_cache")
        if not os.path.exists(self._cache_dir):
            os.mkdir(self._cache_dir)

        self._tile_cache_dir = os.path.join(self._cache_dir, 'tiles')
        if not os.path.exists(self._tile_cache_dir):
            os.mkdir(self._tile_cache_dir)

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
                print("Could not read or find the contents document. Error at line %d, column %d:\n%s" % (
                    errorLine, errorColumn, errorStr))
                return QByteArray()

        return doc.toByteArray()

    def setCachedDocument(self, doc, project, request, key):
        if not doc:
            print("Could not cache None document")
            return False
        m = hashlib.md5()
        paramMap = request.parameters()
        urlParam = "&".join(["%s=%s" % (k, paramMap[k]) for k in paramMap.keys()])
        m.update(urlParam.encode('utf8'))
        with open(os.path.join(self._cache_dir, m.hexdigest() + ".xml"), "w") as f:
            f.write(doc.toString())
        return os.path.exists(os.path.join(self._cache_dir, m.hexdigest() + ".xml"))

    def deleteCachedDocument(self, project, request, key):
        m = hashlib.md5()
        paramMap = request.parameters()
        urlParam = "&".join(["%s=%s" % (k, paramMap[k]) for k in paramMap.keys()])
        m.update(urlParam.encode('utf8'))
        if os.path.exists(os.path.join(self._cache_dir, m.hexdigest() + ".xml")):
            os.remove(os.path.join(self._cache_dir, m.hexdigest() + ".xml"))
        return not os.path.exists(os.path.join(self._cache_dir, m.hexdigest() + ".xml"))

    def deleteCachedDocuments(self, project):
        filelist = [f for f in os.listdir(self._cache_dir) if f.endswith(".xml")]
        for f in filelist:
            os.remove(os.path.join(self._cache_dir, f))
        filelist = [f for f in os.listdir(self._cache_dir) if f.endswith(".xml")]
        return len(filelist) == 0

    def getCachedImage(self, project, request, key):
        m = hashlib.md5()
        paramMap = request.parameters()
        urlParam = "&".join(["%s=%s" % (k, paramMap[k]) for k in paramMap.keys()])
        m.update(urlParam.encode('utf8'))

        if not os.path.exists(os.path.join(self._tile_cache_dir, m.hexdigest() + ".png")):
            return QByteArray()

        img = QImage(m.hexdigest() + ".png")
        with open(os.path.join(self._tile_cache_dir, m.hexdigest() + ".png"), "rb") as f:
            statusOK = img.loadFromData(f.read())
            if not statusOK:
                print("Could not read or find the contents document.")
                return QByteArray()

        ba = QByteArray()
        buff = QBuffer(ba)
        buff.open(QIODevice.WriteOnly)
        img.save(buff, 'PNG')
        return ba

    def setCachedImage(self, img, project, request, key):
        m = hashlib.md5()
        paramMap = request.parameters()
        urlParam = "&".join(["%s=%s" % (k, paramMap[k]) for k in paramMap.keys()])
        m.update(urlParam.encode('utf8'))
        with open(os.path.join(self._tile_cache_dir, m.hexdigest() + ".png"), "wb") as f:
            f.write(img)
        return os.path.exists(os.path.join(self._tile_cache_dir, m.hexdigest() + ".png"))

    def deleteCachedImage(self, project, request, key):
        m = hashlib.md5()
        paramMap = request.parameters()
        urlParam = "&".join(["%s=%s" % (k, paramMap[k]) for k in paramMap.keys()])
        m.update(urlParam.encode('utf8'))
        if os.path.exists(os.path.join(self._tile_cache_dir, m.hexdigest() + ".png")):
            os.remove(os.path.join(self._tile_cache_dir, m.hexdigest() + ".png"))
        return not os.path.exists(os.path.join(self._tile_cache_dir, m.hexdigest() + ".png"))

    def deleteCachedImages(self, project):
        filelist = [f for f in os.listdir(self._tile_cache_dir) if f.endswith(".png")]
        for f in filelist:
            os.remove(os.path.join(self._tile_cache_dir, f))
        filelist = [f for f in os.listdir(self._tile_cache_dir) if f.endswith(".png")]
        return len(filelist) == 0


class TestQgsServerCacheManager(QgsServerTestBase):

    @classmethod
    def _handle_request(cls, qs, requestMethod=QgsServerRequest.GetMethod, data=None):
        if data is not None:
            data = data.encode('utf-8')
        request = QgsBufferServerRequest(qs, requestMethod, {}, data)
        response = QgsBufferServerResponse()
        cls.server.handleRequest(request, response)
        headers = []
        rh = response.headers()
        rk = sorted(rh.keys())
        for k in rk:
            headers.append(("%s: %s" % (k, rh[k])).encode('utf-8'))
        return b"\n".join(headers) + b"\n\n", bytes(response.body())

    @classmethod
    def setUpClass(cls):

        super().setUpClass()

        cls._handle_request("")
        cls._server_iface = cls.server.serverInterface()
        cls._servercache = PyServerCache(cls._server_iface)
        cls._server_iface.registerServerCache(cls._servercache, 100)

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
        self.server.handleRequest(request, response)
        headers = []
        rh = response.headers()
        rk = sorted(rh.keys())
        for k in rk:
            headers.append(("%s: %s" % (k, rh[k])).encode('utf-8'))
        return b"\n".join(headers) + b"\n\n", bytes(response.body())

    def test_getcapabilities(self):
        project = self.projectPath
        assert os.path.exists(project), "Project file not found: " + project

        # without cache
        query_string = '?MAP=%s&SERVICE=WMS&VERSION=1.3.0&REQUEST=%s' % (urllib.parse.quote(project), 'GetCapabilities')
        header, body = self._execute_request(query_string)
        doc = QDomDocument("wms_getcapabilities_130.xml")
        doc.setContent(body)
        # with cache
        header, body = self._execute_request(query_string)

        # without cache
        query_string = '?MAP=%s&SERVICE=WMS&VERSION=1.1.1&REQUEST=%s' % (urllib.parse.quote(project), 'GetCapabilities')
        header, body = self._execute_request(query_string)
        # with cache
        header, body = self._execute_request(query_string)

        # without cache
        query_string = '?MAP=%s&SERVICE=WFS&VERSION=1.1.0&REQUEST=%s' % (urllib.parse.quote(project), 'GetCapabilities')
        header, body = self._execute_request(query_string)
        # with cache
        header, body = self._execute_request(query_string)

        # without cache
        query_string = '?MAP=%s&SERVICE=WFS&VERSION=1.0.0&REQUEST=%s' % (urllib.parse.quote(project), 'GetCapabilities')
        header, body = self._execute_request(query_string)
        # with cache
        header, body = self._execute_request(query_string)

        # without cache
        query_string = '?MAP=%s&SERVICE=WCS&VERSION=1.0.0&REQUEST=%s' % (urllib.parse.quote(project), 'GetCapabilities')
        header, body = self._execute_request(query_string)
        # with cache
        header, body = self._execute_request(query_string)

        # without cache
        query_string = '?MAP=%s&SERVICE=WMTS&VERSION=1.0.0&REQUEST=%s' % (
            urllib.parse.quote(project), 'GetCapabilities')
        header, body = self._execute_request(query_string)
        # with cache
        header, body = self._execute_request(query_string)

        filelist = [f for f in os.listdir(self._servercache._cache_dir) if f.endswith(".xml")]
        self.assertEqual(len(filelist), 6, 'Not enough file in cache')

        cacheManager = self._server_iface.cacheManager()

        self.assertTrue(cacheManager.deleteCachedDocuments(None), 'deleteCachedDocuments does not return True')

        filelist = [f for f in os.listdir(self._servercache._cache_dir) if f.endswith(".xml")]
        self.assertEqual(len(filelist), 0, 'All files in cache are not deleted ')

        prj = QgsProject()
        prj.read(project)

        query_string = '?MAP=%s&SERVICE=WMS&VERSION=1.3.0&REQUEST=%s' % (urllib.parse.quote(project), 'GetCapabilities')
        request = QgsBufferServerRequest(query_string, QgsServerRequest.GetMethod, {}, None)

        accessControls = self._server_iface.accessControls()

        cDoc = QDomDocument("wms_getcapabilities_130.xml")
        self.assertFalse(cacheManager.getCachedDocument(cDoc, prj, request, accessControls),
                         'getCachedDocument is not None')

        self.assertTrue(cacheManager.setCachedDocument(doc, prj, request, accessControls), 'setCachedDocument false')

        self.assertTrue(cacheManager.getCachedDocument(cDoc, prj, request, accessControls), 'getCachedDocument is None')
        self.assertEqual(doc.documentElement().tagName(), cDoc.documentElement().tagName(),
                         'cachedDocument not equal to provide document')

        self.assertTrue(cacheManager.deleteCachedDocuments(None), 'deleteCachedDocuments does not return True')

    def test_getcontext(self):
        project = self.projectPath
        assert os.path.exists(project), "Project file not found: " + project

        # without cache
        query_string = '?MAP=%s&SERVICE=WMS&VERSION=1.3.0&REQUEST=%s' % (urllib.parse.quote(project), 'GetContext')
        header, body = self._execute_request(query_string)
        # with cache
        header, body = self._execute_request(query_string)

        filelist = [f for f in os.listdir(self._servercache._cache_dir) if f.endswith(".xml")]
        self.assertEqual(len(filelist), 1, 'Not enough file in cache')

        cacheManager = self._server_iface.cacheManager()

        self.assertTrue(cacheManager.deleteCachedDocuments(None), 'deleteCachedImages does not return True')

        filelist = [f for f in os.listdir(self._servercache._cache_dir) if f.endswith(".xml")]
        self.assertEqual(len(filelist), 0, 'All files in cache are not deleted ')

    def test_describefeaturetype(self):
        project = self.projectPath
        assert os.path.exists(project), "Project file not found: " + project

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(project),
            "SERVICE": "WFS",
            "VERSION": "1.1.0",
            "REQUEST": "DescribeFeatureType",
            "FEATURETYPE": "Country"
        }.items())])

        header, body = self._execute_request(qs)
        # with cache
        header, body = self._execute_request(qs)

        filelist = [f for f in os.listdir(self._servercache._cache_dir) if f.endswith(".xml")]
        self.assertEqual(len(filelist), 1, 'Not enough file in cache')

        cacheManager = self._server_iface.cacheManager()

        self.assertTrue(cacheManager.deleteCachedDocuments(None), 'deleteCachedImages does not return True')

        filelist = [f for f in os.listdir(self._servercache._cache_dir) if f.endswith(".xml")]
        self.assertEqual(len(filelist), 0, 'All files in cache are not deleted ')

    def test_getlegendgraphic(self):
        project = self.projectPath
        assert os.path.exists(project), "Project file not found: " + project

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(project),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetLegendGraphic",
            "LAYER": "Country,Hello",
            "LAYERTITLE": "FALSE",
            "RULELABEL": "FALSE",
            "FORMAT": "image/png",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        # without cache
        r, h = self._result(self._execute_request(qs))
        self.assertEqual(
            h.get("Content-Type"), "image/png",
            "Content type is wrong: %s\n%s" % (h.get("Content-Type"), r))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_Basic", max_size_diff=QSize(1, 1))
        # with cache
        r, h = self._result(self._execute_request(qs))
        self.assertEqual(
            h.get("Content-Type"), "image/png",
            "Content type is wrong: %s\n%s" % (h.get("Content-Type"), r))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_Basic", max_size_diff=QSize(1, 1))

        filelist = [f for f in os.listdir(self._servercache._tile_cache_dir) if f.endswith(".png")]
        self.assertEqual(len(filelist), 1, 'Not enough image in cache')

        cacheManager = self._server_iface.cacheManager()

        self.assertTrue(cacheManager.deleteCachedImages(None), 'deleteCachedImages does not return True')

        filelist = [f for f in os.listdir(self._servercache._tile_cache_dir) if f.endswith(".png")]
        self.assertEqual(len(filelist), 0, 'All images in cache are not deleted ')

    def test_gettile(self):
        project = self.projectPath
        assert os.path.exists(project), "Project file not found: " + project

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(project),
            "SERVICE": "WMTS",
            "VERSION": "1.0.0",
            "REQUEST": "GetTile",
            "LAYER": "Country",
            "STYLE": "",
            "TILEMATRIXSET": "EPSG:3857",
            "TILEMATRIX": "0",
            "TILEROW": "0",
            "TILECOL": "0",
            "FORMAT": "image/png"
        }.items())])

        # without cache
        r, h = self._result(self._execute_request(qs))
        self.assertEqual(
            h.get("Content-Type"), "image/png",
            "Content type is wrong: %s\n%s" % (h.get("Content-Type"), r))
        # with cache
        r, h = self._result(self._execute_request(qs))
        self.assertEqual(
            h.get("Content-Type"), "image/png",
            "Content type is wrong: %s\n%s" % (h.get("Content-Type"), r))

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(project),
            "SERVICE": "WMTS",
            "VERSION": "1.0.0",
            "REQUEST": "GetTile",
            "LAYER": "Country",
            "STYLE": "",
            "TILEMATRIXSET": "EPSG:4326",
            "TILEMATRIX": "0",
            "TILEROW": "0",
            "TILECOL": "0",
            "FORMAT": "image/png"
        }.items())])

        # without cache
        r, h = self._result(self._execute_request(qs))
        self.assertEqual(
            h.get("Content-Type"), "image/png",
            "Content type is wrong: %s\n%s" % (h.get("Content-Type"), r))
        # with cache
        r, h = self._result(self._execute_request(qs))
        self.assertEqual(
            h.get("Content-Type"), "image/png",
            "Content type is wrong: %s\n%s" % (h.get("Content-Type"), r))

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(project),
            "SERVICE": "WMTS",
            "VERSION": "1.0.0",
            "REQUEST": "GetTile",
            "LAYER": "QGIS Server Hello World",
            "STYLE": "",
            "TILEMATRIXSET": "EPSG:3857",
            "TILEMATRIX": "0",
            "TILEROW": "0",
            "TILECOL": "0",
            "FORMAT": "image/png"
        }.items())])

        # without cache
        r, h = self._result(self._execute_request(qs))
        self.assertEqual(
            h.get("Content-Type"), "image/png",
            "Content type is wrong: %s\n%s" % (h.get("Content-Type"), r))
        self._img_diff_error(r, h, "WMTS_GetTile_Project_3857_0", 20000)
        # with cache
        r, h = self._result(self._execute_request(qs))
        self.assertEqual(
            h.get("Content-Type"), "image/png",
            "Content type is wrong: %s\n%s" % (h.get("Content-Type"), r))
        self._img_diff_error(r, h, "WMTS_GetTile_Project_3857_0", 20000)

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(project),
            "SERVICE": "WMTS",
            "VERSION": "1.0.0",
            "REQUEST": "GetTile",
            "LAYER": "QGIS Server Hello World",
            "STYLE": "",
            "TILEMATRIXSET": "EPSG:4326",
            "TILEMATRIX": "0",
            "TILEROW": "0",
            "TILECOL": "0",
            "FORMAT": "image/png"
        }.items())])

        # without cache
        r, h = self._result(self._execute_request(qs))
        self.assertEqual(
            h.get("Content-Type"), "image/png",
            "Content type is wrong: %s\n%s" % (h.get("Content-Type"), r))
        self._img_diff_error(r, h, "WMTS_GetTile_Project_4326_0", 20000)
        # with cache
        r, h = self._result(self._execute_request(qs))
        self.assertEqual(
            h.get("Content-Type"), "image/png",
            "Content type is wrong: %s\n%s" % (h.get("Content-Type"), r))
        self._img_diff_error(r, h, "WMTS_GetTile_Project_4326_0", 20000)

        filelist = [f for f in os.listdir(self._servercache._tile_cache_dir) if f.endswith(".png")]
        self.assertEqual(len(filelist), 4, 'Not enough image in cache')

        cacheManager = self._server_iface.cacheManager()

        self.assertTrue(cacheManager.deleteCachedImages(None), 'deleteCachedImages does not return True')

        filelist = [f for f in os.listdir(self._servercache._tile_cache_dir) if f.endswith(".png")]
        self.assertEqual(len(filelist), 0, 'All images in cache are not deleted ')

    def test_gettile_invalid_parameters(self):
        project = self.projectPath
        assert os.path.exists(project), "Project file not found: " + project

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(project),
            "SERVICE": "WMTS",
            "VERSION": "1.0.0",
            "REQUEST": "GetTile",
            "LAYER": "Country",
            "STYLE": "",
            "TILEMATRIXSET": "EPSG:3857",
            "TILEMATRIX": "0",
            "TILEROW": "0",
            "TILECOL": "FOO",
            "FORMAT": "image/png"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        err = b"TILECOL (\'FOO\') cannot be converted into int" in r
        self.assertTrue(err)

        filelist = [f for f in os.listdir(self._servercache._tile_cache_dir) if f.endswith(".png")]
        self.assertEqual(len(filelist), 0, 'Exception has been cached ')


if __name__ == "__main__":
    unittest.main()
