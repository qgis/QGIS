"""QGIS Unit tests for QgsServer WMS.

From build dir, run: ctest -R PyQgsServerWMS -V


.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""

__author__ = "Alessandro Pasotti"
__date__ = "25/05/2015"
__copyright__ = "Copyright 2015, The QGIS Project"

import json
import os

# Needed on Qt 5 so that the serialization of XML is consistent among all executions
os.environ["QT_HASH_SEED"] = "1"

import re
import urllib.error
import urllib.parse
import urllib.request

import osgeo.gdal  # NOQA

from owslib.wms import WebMapService
from qgis.core import (
    QgsProject,
    QgsMemoryProviderUtils,
    QgsWkbTypes,
    QgsCoordinateReferenceSystem,
    QgsFields,
    QgsField,
    QgsPalLayerSettings,
    QgsVectorLayerSimpleLabeling,
)
from qgis.PyQt.QtCore import QUrl, QUrlQuery, QVariant
from qgis.server import (
    QgsBufferServerResponse,
    QgsServer,
    QgsServerRequest,
)
from qgis.testing import unittest
from test_qgsserver import QgsServerTestBase

# Strip path and content length because path may vary
RE_STRIP_UNCHECKABLE = b'MAP=[^"]+|SERVICE=[^"]+|Content-Length: \\d+'
RE_STRIP_EXTENTS = b"<(north|east|south|west)Bound(Lat|Long)itude>.*</(north|east|south|west)Bound(Lat|Long)itude>|<BoundingBox .*/>"
RE_ATTRIBUTES = b"[^>\\s]+=[^>\\s]+"


class TestQgsServerWMSTestBase(QgsServerTestBase):
    """QGIS Server WMS Tests"""

    # Set to True to re-generate reference files for this class
    regenerate_reference = False

    def wms_request(
        self, request, extra=None, project="test_project.qgs", version="1.3.0"
    ):
        if not os.path.exists(project):
            project = os.path.join(self.testdata_path, project)
        assert os.path.exists(project), "Project file not found: " + project
        query_string = f"https://www.qgis.org/?MAP={urllib.parse.quote(project)}&SERVICE=WMS&VERSION={version}&REQUEST={request}"
        if extra is not None:
            query_string += extra
        header, body = self._execute_request(query_string)
        return (header, body, query_string)

    def wms_request_compare(
        self,
        request,
        extra=None,
        reference_file=None,
        project="test_project.qgs",
        version="1.3.0",
        ignoreExtent=False,
        normalizeJson=False,
        raw=False,
    ):
        response_header, response_body, query_string = self.wms_request(
            request, extra, project, version
        )
        response = response_header + response_body

        if not isinstance(reference_file, (list, tuple)):
            reference_files = [reference_file]
        else:
            reference_files = reference_file

        last_exception = None
        found_match = False
        for reference_file in reference_files:
            reference_path = os.path.join(
                self.testdata_path,
                (request.lower() if not reference_file else reference_file) + ".txt",
            )
            self.store_reference(reference_path, response)
            f = open(reference_path, "rb")
            expected = f.read()

            def _n(r):
                lines = r.split(b"\n")
                b = lines[2:]
                h = lines[:2]
                try:
                    return b"\n".join(h) + json.dumps(json.loads(b"\n".join(b))).encode(
                        "utf8"
                    )
                except:
                    return r

            response = _n(response)
            expected = _n(expected)

            f.close()
            response = re.sub(RE_STRIP_UNCHECKABLE, b"*****", response)
            expected = re.sub(RE_STRIP_UNCHECKABLE, b"*****", expected)
            if ignoreExtent:
                response = re.sub(RE_STRIP_EXTENTS, b"*****", response)
                expected = re.sub(RE_STRIP_EXTENTS, b"*****", expected)

            msg = f"request {query_string} failed.\nQuery: {request}\nExpected file: {reference_path}\nResponse:\n{response.decode('utf-8')}"

            try:
                self.assertXMLEqual(response, expected, msg=msg, raw=raw)
                found_match = True
            except AssertionError as e:
                last_exception = e

        if not found_match:
            raise last_exception


class TestQgsServerWMS(TestQgsServerWMSTestBase):
    """QGIS Server WMS Tests"""

    def test_getcapabilities(self):
        self.wms_request_compare(
            "GetCapabilities", reference_file="getcapabilities-map"
        )

    def test_getcapabilities_case_insensitive(self):
        self.wms_request_compare(
            "getcapabilities", reference_file="getcapabilities-map"
        )
        self.wms_request_compare(
            "GETCAPABILITIES", reference_file="getcapabilities-map"
        )

    def test_getcapabilities_advertised_url(self):
        server = QgsServer()
        request = QgsServerRequest()
        projectPath = os.path.join(self.testdata_path, "test_project.qgs")
        request.setUrl(
            QUrl(
                "http://localhost/qgis_mapserv.fcgi?MAP="
                + projectPath
                + "&SERVICE=WMS&REQUEST=GetCapabilities"
            )
        )
        request.setOriginalUrl(QUrl("http://localhost/wms/test_project"))
        response = QgsBufferServerResponse()
        server.handleRequest(request, response)
        response.flush()

        headers = []
        rh = response.headers()
        rk = sorted(rh.keys())
        for k in rk:
            headers.append((f"{k}: {rh[k]}").encode())

        reference_path = os.path.join(
            self.testdata_path, "wms_getcapabilities_rewriting.txt"
        )
        f = open(reference_path, "rb")
        expected = f.read()
        self.assertXMLEqual(
            b"\n".join(headers) + b"\n\n" + bytes(response.body()), expected
        )

    def test_getcapabilities_scale_denominator(self):
        self.wms_request_compare(
            "GetCapabilities",
            None,
            "wms_getcapabilities_scale_denominator",
            "test_project_scale_denominator.qgs",
        )

    def test_getprojectsettings(self):
        self.wms_request_compare("GetProjectSettings")

    def test_getprojectsettings_opacity(self):
        self.wms_request_compare(
            "GetProjectSettings",
            None,
            "getprojectsettings_opacity",
            "test_opacity_project.qgs",
        )

    def test_getcontext(self):
        self.wms_request_compare("GetContext")

    def test_operation_not_supported(self):
        qs = f"?MAP={urllib.parse.quote(self.projectPath)}&SERVICE=WMS&VERSION=1.3.0&REQUEST=NotAValidRequest"
        self._assert_status_code(501, qs)

    def test_describelayer(self):
        # Test DescribeLayer
        self.wms_request_compare(
            "DescribeLayer",
            "&layers=testlayer%20%C3%A8%C3%A9&" + "SLD_VERSION=1.1.0",
            "describelayer",
        )

    def test_getstyles(self):
        # Test GetStyles
        self.wms_request_compare(
            "GetStyles", "&layers=testlayer%20%C3%A8%C3%A9&", "getstyles"
        )

        # Test GetStyles with labeling
        self.wms_request_compare(
            "GetStyles",
            "&layers=pointlabel",
            "getstyles_pointlabel",
            project=self.projectPath,
        )

        # Test GetStyle with labeling
        self.wms_request_compare(
            "GetStyle",
            "&layers=pointlabel",
            "getstyles_pointlabel",
            project=self.projectPath,
        )

        # Test SLD unchange after GetMap SLD_BODY
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.projectPath),
                        "REQUEST": "GetStyles",
                        "VERSION": "1.1.1",
                        "SERVICE": "WMS",
                        "LAYERS": "db_point",
                    }.items()
                )
            ]
        )
        r, h = self._result(self._execute_request(qs))
        assert "StyledLayerDescriptor" in str(r), f"StyledLayerDescriptor not in {r}"
        assert "__sld_style" not in str(r), f"__sld_style in {r}"

        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.projectPath),
                        "REQUEST": "GetMap",
                        "VERSION": "1.1.1",
                        "SERVICE": "WMS",
                        "SLD_BODY": '<?xml version="1.0" encoding="UTF-8"?><StyledLayerDescriptor xmlns="http://www.opengis.net/sld" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:ogc="http://www.opengis.net/ogc" xsi:schemaLocation="http://www.opengis.net/sld http://schemas.opengis.net/sld/1.1.0/StyledLayerDescriptor.xsd" version="1.1.0" xmlns:se="http://www.opengis.net/se" xmlns:xlink="http://www.w3.org/1999/xlink"> <NamedLayer> <se:Name>db_point</se:Name> <UserStyle> <se:Name>db_point_style</se:Name> <se:FeatureTypeStyle> <se:Rule> <se:Name>Single symbol</se:Name> <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc"> <ogc:PropertyIsEqualTo> <ogc:PropertyName>gid</ogc:PropertyName> <ogc:Literal>1</ogc:Literal> </ogc:PropertyIsEqualTo> </ogc:Filter> <se:PointSymbolizer uom="http://www.opengeospatial.org/se/units/metre"> <se:Graphic> <se:Mark> <se:WellKnownName>square</se:WellKnownName> <se:Fill> <se:SvgParameter name="fill">5e86a1</se:SvgParameter> </se:Fill> <se:Stroke> <se:SvgParameter name="stroke">000000</se:SvgParameter> </se:Stroke> </se:Mark> <se:Size>0.007</se:Size> </se:Graphic> </se:PointSymbolizer> </se:Rule> </se:FeatureTypeStyle> </UserStyle> </NamedLayer> </StyledLayerDescriptor>',
                        "BBOX": "-16817707,-4710778,5696513,14587125",
                        "WIDTH": "500",
                        "HEIGHT": "500",
                        "LAYERS": "db_point",
                        "STYLES": "",
                        "FORMAT": "image/png",
                        "CRS": "EPSG:3857",
                    }.items()
                )
            ]
        )
        self._assert_status_code(200, qs)

        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.projectPath),
                        "REQUEST": "GetStyles",
                        "VERSION": "1.1.1",
                        "SERVICE": "WMS",
                        "LAYERS": "db_point",
                    }.items()
                )
            ]
        )
        r, h = self._result(self._execute_request(qs))
        assert "StyledLayerDescriptor" in str(r), f"StyledLayerDescriptor not in {r}"
        assert "__sld_style" not in str(r), f"__sld_style in {r}"

    def test_wms_getschemaextension(self):
        self.wms_request_compare("GetSchemaExtension", "", "getschemaextension")

    def wms_request_compare_project(
        self, request, extra=None, reference_file=None, project_name="test_project.qgs"
    ):
        projectPath = self.testdata_path + project_name
        assert os.path.exists(projectPath), "Project file not found: " + projectPath

        project = QgsProject()
        project.read(projectPath)

        query_string = (
            f"https://www.qgis.org/?SERVICE=WMS&VERSION=1.3.0&REQUEST={request}"
        )
        if extra is not None:
            query_string += extra
        header, body = self._execute_request_project(query_string, project)
        response = header + body
        reference_path = (
            self.testdata_path
            + (request.lower() if not reference_file else reference_file)
            + ".txt"
        )
        self.store_reference(reference_path, response)
        f = open(reference_path, "rb")
        expected = f.read()
        f.close()
        response = re.sub(RE_STRIP_UNCHECKABLE, b"*****", response)
        expected = re.sub(RE_STRIP_UNCHECKABLE, b"*****", expected)

        self.assertXMLEqual(
            response,
            expected,
            msg=f"request {query_string} failed.\nQuery: {request}\nExpected file: {reference_path}\nResponse:\n{response.decode('utf-8')}",
        )

    def test_wms_getcapabilities_project(self):
        """WMS GetCapabilities without map parameter"""
        self.wms_request_compare_project("GetCapabilities")
        # reference_file='getcapabilities_without_map_param' could be the right response

    def test_wms_getcapabilities_project_empty_layer(self):
        """WMS GetCapabilities with empty layer different CRS: wrong bbox - Regression GH 30264"""
        self.wms_request_compare_project(
            "GetCapabilities",
            reference_file="wms_getcapabilities_empty_layer",
            project_name="bug_gh30264_empty_layer_wrong_bbox.qgs",
        )

    def wms_inspire_request_compare(self, request):
        """WMS INSPIRE tests"""
        project = self.testdata_path + "test_project_inspire.qgs"
        assert os.path.exists(project), "Project file not found: " + project

        query_string = f"?MAP={urllib.parse.quote(project)}&SERVICE=WMS&VERSION=1.3.0&REQUEST={request}"
        header, body = self._execute_request(query_string)
        response = header + body
        reference_path = self.testdata_path + request.lower() + "_inspire.txt"
        self.store_reference(reference_path, response)
        f = open(reference_path, "rb")
        expected = f.read()
        f.close()
        response = re.sub(RE_STRIP_UNCHECKABLE, b"", response)
        expected = re.sub(RE_STRIP_UNCHECKABLE, b"", expected)
        self.assertXMLEqual(
            response,
            expected,
            msg=f"request {query_string} failed.\nQuery: {request}\nExpected file: {reference_path}\nResponse:\n{response.decode('utf-8')}",
        )

    def test_project_wms_inspire(self):
        """Test some WMS request"""
        for request in ("GetCapabilities",):
            self.wms_inspire_request_compare(request)

    def test_wms_getcapabilities_without_title(self):
        # Empty title in project leads to a Layer element without Name, Title
        # and Abstract tags. However, it should still have a CRS and a BBOX
        # according to OGC specifications tests.
        self.wms_request_compare(
            "GetCapabilities",
            reference_file="wms_getcapabilities_without_title",
            project="test_project_without_title.qgs",
        )

    def test_wms_getcapabilities_empty_spatial_layer(self):
        # The project contains a spatial layer without feature and the WMS
        # extent is not configured in the project.
        self.wms_request_compare(
            "GetCapabilities",
            reference_file="wms_getcapabilities_empty_spatial_layer",
            project="test_project_empty_spatial_layer.qgz",
            ignoreExtent=True,
        )

    def test_wms_getcapabilities_versions(self):
        # default version 1.3.0 when empty VERSION parameter
        project = os.path.join(self.testdata_path, "test_project.qgs")
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(project),
                        "SERVICE": "WMS",
                        "REQUEST": "GetCapabilities",
                    }.items()
                )
            ]
        )

        self.wms_request_compare(
            qs, reference_file="wms_getcapabilities_1_3_0", version=""
        )

        # default version 1.3.0 when VERSION = 1.3.0 parameter
        project = os.path.join(self.testdata_path, "test_project.qgs")
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(project),
                        "SERVICE": "WMS",
                        "REQUEST": "GetCapabilities",
                    }.items()
                )
            ]
        )

        self.wms_request_compare(
            qs, reference_file="wms_getcapabilities_1_3_0", version="1.3.0"
        )

        # version 1.1.1
        project = os.path.join(self.testdata_path, "test_project.qgs")
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(project),
                        "SERVICE": "WMS",
                        "REQUEST": "GetCapabilities",
                    }.items()
                )
            ]
        )

        self.wms_request_compare(
            qs, reference_file="wms_getcapabilities_1_1_1", version="1.1.1"
        )

        # default version 1.3.0 when invalid VERSION parameter
        project = os.path.join(self.testdata_path, "test_project.qgs")
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(project),
                        "SERVICE": "WMS",
                        "REQUEST": "GetCapabilities",
                    }.items()
                )
            ]
        )

        self.wms_request_compare(
            qs, reference_file="wms_getcapabilities_1_3_0", version="33.33.33"
        )

    def test_wms_getcapabilities_url(self):
        # empty url in project
        project = os.path.join(self.testdata_path, "test_project_without_urls.qgs")
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(project),
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetCapabilities",
                        "STYLES": "",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))

        item_found = False
        for item in str(r).split("\\n"):
            if "OnlineResource" in item:
                self.assertEqual('xlink:href="?' in item, True)
                item_found = True
        self.assertTrue(item_found)

        # url passed in query string
        # verify that GetCapabilities isn't put into the url for non-uppercase parameter names
        project = os.path.join(self.testdata_path, "test_project_without_urls.qgs")
        qs = "https://www.qgis-server.org?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(project),
                        "SeRvIcE": "WMS",
                        "VeRsIoN": "1.3.0",
                        "ReQuEsT": "GetCapabilities",
                        "STYLES": "",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))

        item_found = False
        for item in str(r).split("\\n"):
            if "OnlineResource" in item:
                self.assertEqual(
                    'xlink:href="https://www.qgis-server.org?' in item, True
                )
                self.assertEqual("GetCapabilities" in item, False)
                item_found = True
        self.assertTrue(item_found)

        # url well defined in project
        project = os.path.join(self.testdata_path, "test_project_with_urls.qgs")
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(project),
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetCapabilities",
                        "STYLES": "",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))

        item_found = False
        for item in str(r).split("\\n"):
            if (
                "OnlineResource" in item
                and 'xlink:href="my_wms_advertised_url?' in item
            ):
                item_found = True
        self.assertTrue(item_found)

    @unittest.skip("Timeout issues")
    def test_wms_GetProjectSettings_wms_print_layers(self):
        projectPath = self.testdata_path + "test_project_wms_printlayers.qgs"
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": projectPath,
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetProjectSettings",
                    }.items()
                )
            ]
        )
        header, body = self._execute_request(qs)
        xmlResult = body.decode("utf-8")
        self.assertNotEqual(
            xmlResult.find("<WMSBackgroundLayer>1</WMSBackgroundLayer>"), -1
        )
        self.assertNotEqual(
            xmlResult.find(
                "<WMSDataSource>contextualWMSLegend=0&amp;crs=EPSG:21781&amp;dpiMode=7&amp;featureCount=10&amp;format=image/png&amp;layers=public_geo_gemeinden&amp;styles=&amp;url=https://qgiscloud.com/mhugent/qgis_unittest_wms/wms?</WMSDataSource>"
            ),
            -1,
        )
        self.assertNotEqual(
            xmlResult.find(
                "<WMSPrintLayer>contextualWMSLegend=0&amp;amp;crs=EPSG:21781&amp;amp;dpiMode=7&amp;amp;featureCount=10&amp;amp;format=image/png&amp;amp;layers=public_geo_gemeinden&amp;amp;styles=&amp;amp;url=https://qgiscloud.com/mhugent/qgis_unittest_wms_print/wms?</WMSPrintLayer>"
            ),
            -1,
        )

    def test_getcapabilities_owslib(self):

        # read getcapabilities document
        docPath = self.testdata_path + "getcapabilities.txt"
        f = open(docPath)
        doc = f.read()
        f.close()

        # clean header in doc
        doc = doc.replace("Content-Length: 15066\n", "")
        doc = doc.replace("Content-Type: text/xml; charset=utf-8\n\n", "")
        doc = doc.replace('<?xml version="1.0" encoding="utf-8"?>\n', "")

        # read capabilities document with owslib
        w = WebMapService(None, xml=doc, version="1.3.0")

        # check content
        rootLayerName = "QGIS Test Project"
        self.assertIn(rootLayerName, w.contents.keys())

    def test_get_styles_sld_label_negative_offset(self):
        """Test issue GH #458862"""

        # Create a point layer with a negative offset for the label
        project = QgsProject()
        fields = QgsFields()
        fields.append(QgsField("int", QVariant.Int))
        layer = QgsMemoryProviderUtils.createMemoryLayer(
            "pointlabel",
            fields,
            QgsWkbTypes.Type.Point,
            QgsCoordinateReferenceSystem(4326),
        )
        layer.setLabelsEnabled(True)
        settings = QgsPalLayerSettings()
        settings.xOffset = -10
        settings.yOffset = -5
        settings.fieldName = "int"
        settings.enabled = True
        settings.placement = QgsPalLayerSettings.Placement.OverPoint

        labeling = QgsVectorLayerSimpleLabeling(settings)
        layer.setLabeling(labeling)

        project.addMapLayer(layer)

        # Test GetStyles with labeling
        server = QgsServer()
        request = QgsServerRequest()
        request.setUrl(QUrl("?SERVICE=WMS&REQUEST=GetStyles&LAYERS=pointlabel"))
        response = QgsBufferServerResponse()
        server.handleRequest(request, response, project)
        body = response.body().data().decode("utf8").replace("\n", "")
        self.assertIn("<se:DisplacementX>-36</se:DisplacementX>", body)
        self.assertIn("<se:DisplacementY>-18</se:DisplacementY>", body)


if __name__ == "__main__":
    unittest.main()
