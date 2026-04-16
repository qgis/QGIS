"""QGIS Unit tests for QgsServer WMS with opaque groups.

From build dir, run: ctest -R PyQgsServerWMSOpaqueGroups -V


.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""

__author__ = "Dave Signer"
__date__ = "11/04/2026"
__copyright__ = "Copyright 2026, The QGIS Project"

import os

# Needed on Qt 5 so that the serialization of XML is consistent among all
# executions
os.environ["QT_HASH_SEED"] = "1"

import json
import urllib.error
import urllib.parse
import urllib.request
import xml.etree.ElementTree as ET

import osgeo.gdal  # NOQA
from qgis.core import (
    Qgis,
    QgsAttributeEditorField,
    QgsCoordinateReferenceSystem,
    QgsFeature,
    QgsField,
    QgsFields,
    QgsGeometry,
    QgsMapLayer,
    QgsMemoryProviderUtils,
    QgsPointXY,
    QgsProject,
    QgsWkbTypes,
)
from qgis.PyQt.QtCore import QPoint, QSize, QUrl, QVariant
from qgis.server import (
    QgsBufferServerRequest,
    QgsBufferServerResponse,
    QgsServer,
    QgsServerRequest,
)
from qgis.testing import QgisTestCase, unittest
from test_qgsserver_wms import TestQgsServerWMSTestBase


class TestQgsServerWMSOpaqueGroups(TestQgsServerWMSTestBase):
    """QGIS Server WMS Tests for several request with opaque groups"""

    """ Project to test

    - trafficsign (based on point1)
    - street-group
        - streetline (based on streetline)
        - streetarea (based on streetarea)
    - road-group (opaque)
        - roadline (based on roadline)
        - roadarea (based on roadarea)
    - way-group (opaque)
        - wayline (based on wayline)
        - wayarea (based on wayarea) (excluded)
    - smallway-group (opaque)
        - smallway-subgroup (not opaque)
            - smallwayline (based on smallwayline)
    - sign-group (opaque)
        - trafficsign (based on point2)
        - trafficsign (based on point3)
    - anothersign (opaque)
        - trafficsign (based on point1)
        - trafficsign (based on point2)
    - anothersign (based on point 3)

    The street-group is a normal group. 
    The road-group is an opaque group.
    The way-group is an opaque group and the wayarea is excluded.
    The sign-group is an opaque group with samenamed child-layers (and having same name like external layer)
    The anothersign is an opaque group havaing a samenamed external layer
    """

    def setUp(self):
        super().setUp()
        self.project = os.path.join(self.testdata_path, "test_project_opaque.qgz")

    def tearDown(self):
        super().tearDown()
        os.putenv("QGIS_SERVER_ALLOWED_EXTRA_SQL_TOKENS", "")

    def testGetCapabilities(self):
        """
        Test getcapabilities response.

        We will find there
        - trafficsign as layer
        - street-group as group with layer objects:
            - streetline as layer
            - streetarea as layer
        - road-group as layer without any layer objects
        - way-group as layer without any layer objects
        - smallway-group as layer without any layer objects
        - sign-group as layer without any layer objects
        - anothersign as one layer (without any layer objects) and no duplicate

        We will not find there
        - roadline
        - roadarea
        - wayline
        - wayarea
        - smallwayline
        """

        (_, body, request) = self.wms_request("GetCapabilities", project=self.project)

        xml_result = body.decode("utf-8")

        received_layernames = []
        received_opaquelayernames = []
        root = ET.fromstring(xml_result)
        for layer in root.iter("{http://www.opengis.net/wms}Layer"):
            name_elem = layer.find("{*}Name")
            if name_elem is not None:
                # add the name to the list
                received_layernames.append(name_elem.text)

        expected_layernames = {
            "trafficsign",
            "street-group",
            "streetline",
            "streetarea",
            "road-group",
            "way-group",
            "smallway-group",
            "sign-group",
            "anothersign",
        }
        # Check if ther right layers are in the result
        self.assertEqual(
            set(received_layernames),
            expected_layernames,
            f"\nIncorrect set of layernames found in GetCapabilities:\n\n{request}\nResult:\n{xml_result}",
        )

    def testGetContext(self):
        """
        Test getcontext response.

        We will find there
        - trafficsign as layer
        - street-group as group with layer objects:
            - streetline as layer
            - streetarea as layer
        - road-group as layer without any layer objects
        - way-group as layer without any layer objects
        - smallway-group as layer without any layer objects
        - sign-group as layer without any layer objects
        - anothersign as one layer (without any layer objects) and no duplicate

        We will not find there
        - roadline
        - roadarea
        - wayline
        - wayarea
        - smallwayline

        The following groups should be marked as opaque:
        - road-group
        - way-group
        - smallway-group
        - sign-group
        - anothersign
        """
        pass

    def testGetProjectSettings(self):
        """
        Test getprojectsettings response.

        We will find there
        - trafficsign as layer
        - street-group as group with layer objects:
            - streetline as layer
            - streetarea as layer
        - road-group as layer without any layer objects
        - way-group as layer without any layer objects
        - smallway-group as layer without any layer objects
        - sign-group as layer without any layer objects
        - anothersign as one layer (without any layer objects) and no duplicate

        We will not find there
        - roadline
        - roadarea
        - wayline
        - wayarea
        - smallwayline

        The following groups should be marked as opaque:
        - road-group
        - way-group
        - smallway-group
        - sign-group
        - anothersign
        """

        (_, body, request) = self.wms_request(
            "GetProjectSettings", project=self.project
        )

        xml_result = body.decode("utf-8")

        received_layernames = []
        received_opaquelayernames = []
        root = ET.fromstring(xml_result)
        for layer in root.iter("{http://www.opengis.net/wms}Layer"):
            name_elem = layer.find("{*}Name")
            if name_elem is not None:
                # add the name to the list
                received_layernames.append(name_elem.text)
                # if the layer is opaque, add it to the list
                opaque_attribute_value = layer.get("opaque")
                if opaque_attribute_value == "1":
                    received_opaquelayernames.append(name_elem.text)

        expected_layernames = {
            "trafficsign",
            "street-group",
            "streetline",
            "streetarea",
            "road-group",
            "way-group",
            "smallway-group",
            "sign-group",
            "anothersign",
        }
        # Check if ther right layers are in the result - same as in getCapabilities
        self.assertEqual(
            set(received_layernames),
            expected_layernames,
            f"\nIncorrect set of layernames found in GetProjectSettings:\n\n{request}\nResult:\n{xml_result}",
        )

        # as well these are the layers that should be stored in LayerDrawingOrder
        layer_drawing_order_commalist = next(
            root.iter("{http://www.opengis.net/wms}LayerDrawingOrder"), None
        ).text
        layer_drawing_order_names_set = set(
            [name.strip() for name in layer_drawing_order_commalist.split(",")]
        )

        expected_layernames = {"trafficsign", "streetline", "streetarea", "anothersign"}
        self.assertEqual(
            layer_drawing_order_names_set,
            expected_layernames,
            f"\nIncorrect LayerDrawingOrder found in GetProjectSettings:\n\n{request}\nResult:\n{xml_result}",
        )

        # Check if the right layers are in the result
        expected_layernames = {
            "road-group",
            "way-group",
            "smallway-group",
            "sign-group",
            "anothersign",
        }
        self.assertEqual(
            set(received_opaquelayernames),
            expected_layernames,
            f"\nIncorrect set of opaque layers found in GetProjectSettings:\n\n{request}\n{xml_result}",
        )

    def testGetMap(self):
        """
        Test getmap response.

        Request on opaque groups should return like a normal group the features.
        Request on a layer in a normal group returns the features.
        Request on a layer in an opaque group returns an OGC_LayerNotDefined.
        Request on a group in an opaque group returns an OGC_LayerNotDefined.
        Request on a same-named layer should merge
        - but not with a same-named layer in an opaque group
        - but with a same-named opaque group
        """

        """
        http://qgis.demo/cgi-bin/qgis_mapserv.fcgi?MAP=/home/dave/dev/qgis/QGIS2/tests/testdata/qgis_server/test_project_opaque.qgz&SERVICE=WMS&VERSION=1.3.0&REQUEST=GetMap
        &BBOX=47.51411822721124167,8.76265867349436078,47.51802447721124167,8.76723198210506993
        &CRS=EPSG:4326
        &WIDTH=600
        &HEIGHT=600
        &LAYERS=street-group
        &STYLES=
        &FORMAT=image/png


        (header, body, request) = self.wms_request(
            "GetMap",
            "&layers=street-group%20%C3%A8%C3%A9&styles=&"
            + "format=image%2Fpng&transparent=true&"
            + "width=600&height=600&CRS=EPSG%3A4326&bbox="
            + "47.51411822721124167,8.76265867349436078,47.51802447721124167,8.76723198210506993&",
            project=self.project,
        )
        """

        # Request on normal group
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetMap",
                        "LAYERS": "street-group",
                        "TRANSPARENT": "true",
                        "STYLES": "",
                        "FORMAT": "image/png",
                        "BBOX": "47.51411822721124167,8.76265867349436078,47.51802447721124167,8.76723198210506993",
                        "HEIGHT": "600",
                        "WIDTH": "600",
                        "CRS": "EPSG:4326",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        # self._img_diff_error(
        #     r, h, "WMS_GetMap_OpaqueGroup_street-group", max_size_diff=QSize(1, 1)
        # )
        self.assertNotIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be no server exception for the normal layer group: street-group",
        )

        # Request on opaque group
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetMap",
                        "LAYERS": "road-group",
                        "TRANSPARENT": "true",
                        "STYLES": "",
                        "FORMAT": "image/png",
                        "BBOX": "47.51411822721124167,8.76265867349436078,47.51802447721124167,8.76723198210506993",
                        "HEIGHT": "600",
                        "WIDTH": "600",
                        "CRS": "EPSG:4326",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        # self._img_diff_error(
        #     r, h, "WMS_GetMap_OpaqueGroup_road-group", max_size_diff=QSize(1, 1)
        # )
        self.assertNotIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be no server exception for the opaque layer group: road-group",
        )

        # Request on opaque group with a excluded layers
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetMap",
                        "LAYERS": "way-group",
                        "TRANSPARENT": "true",
                        "STYLES": "",
                        "FORMAT": "image/png",
                        "BBOX": "47.51411822721124167,8.76265867349436078,47.51802447721124167,8.76723198210506993",
                        "HEIGHT": "600",
                        "WIDTH": "600",
                        "CRS": "EPSG:4326",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))

        self.assertNotIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be no server exception for the opaque layer group: way-group",
        )

        # Request on opaque group with a excluded layers
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetMap",
                        "LAYERS": "way-group",
                        "TRANSPARENT": "true",
                        "STYLES": "",
                        "FORMAT": "image/png",
                        "BBOX": "47.51411822721124167,8.76265867349436078,47.51802447721124167,8.76723198210506993",
                        "HEIGHT": "600",
                        "WIDTH": "600",
                        "CRS": "EPSG:4326",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        # self._img_diff_error(
        #     r, h, "WMS_GetMap_OpaqueGroup_way-group", max_size_diff=QSize(1, 1)
        # )
        self.assertNotIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be no server exception for the opaque layer group: way-group",
        )

        # Request on opaque group with a subgroup
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetMap",
                        "LAYERS": "smallway-group",
                        "TRANSPARENT": "true",
                        "STYLES": "",
                        "FORMAT": "image/png",
                        "BBOX": "47.51411822721124167,8.76265867349436078,47.51802447721124167,8.76723198210506993",
                        "HEIGHT": "600",
                        "WIDTH": "600",
                        "CRS": "EPSG:4326",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        # self._img_diff_error(
        #     r, h, "WMS_GetMap_OpaqueGroup_smallway-group", max_size_diff=QSize(1, 1)
        # )
        self.assertNotIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be no server exception for the opaque layer group: smallway-group",
        )

        # Request on group in opaque groups - should OGC_LayerNotDefined
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetMap",
                        "LAYERS": "smallway-subgroup",
                        "TRANSPARENT": "true",
                        "STYLES": "",
                        "FORMAT": "image/png",
                        "BBOX": "47.51411822721124167,8.76265867349436078,47.51802447721124167,8.76723198210506993",
                        "HEIGHT": "600",
                        "WIDTH": "600",
                        "CRS": "EPSG:4326",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self.assertIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be a server exception for a group inside an opaque layer group: smallway-subgroup",
        )

        # Request on layer in normal group
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetMap",
                        "LAYERS": "streetline",
                        "TRANSPARENT": "true",
                        "STYLES": "",
                        "FORMAT": "image/png",
                        "BBOX": "47.51411822721124167,8.76265867349436078,47.51802447721124167,8.76723198210506993",
                        "HEIGHT": "600",
                        "WIDTH": "600",
                        "CRS": "EPSG:4326",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        # self._img_diff_error(
        #     r, h, "WMS_GetMap_OpaqueGroup_streetline", max_size_diff=QSize(1, 1)
        # )
        self.assertNotIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be no server exception for a layer inside a normal layer group: streetline",
        )

        # Request on layer in opaque group - should OGC_LayerNotDefined
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetMap",
                        "LAYERS": "roadline",
                        "TRANSPARENT": "true",
                        "STYLES": "",
                        "FORMAT": "image/png",
                        "BBOX": "47.51411822721124167,8.76265867349436078,47.51802447721124167,8.76723198210506993",
                        "HEIGHT": "600",
                        "WIDTH": "600",
                        "CRS": "EPSG:4326",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self.assertIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be a server exception for a layer inside an opaque layer group: roadline",
        )

        # Request on layer in non-opaque group of opaque group - should OGC_LayerNotDefined
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetMap",
                        "LAYERS": "smallwayline",
                        "TRANSPARENT": "true",
                        "STYLES": "",
                        "FORMAT": "image/png",
                        "BBOX": "47.51411822721124167,8.76265867349436078,47.51802447721124167,8.76723198210506993",
                        "HEIGHT": "600",
                        "WIDTH": "600",
                        "CRS": "EPSG:4326",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self.assertIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be a server exception for a layer in a non-opaque group of an opaque layer group: smallwayline",
        )

        # Request on a layer with a same-named layer in an opaque group
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetMap",
                        "LAYERS": "trafficsign",
                        "TRANSPARENT": "true",
                        "STYLES": "",
                        "FORMAT": "image/png",
                        "BBOX": "47.51411822721124167,8.76265867349436078,47.51802447721124167,8.76723198210506993",
                        "HEIGHT": "600",
                        "WIDTH": "600",
                        "CRS": "EPSG:4326",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        # self._img_diff_error(
        #     r, h, "WMS_GetMap_OpaqueGroup_trafficsign", max_size_diff=QSize(1, 1)
        # )
        self.assertNotIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be no server exception for a layer with a samenamed layer inside an opaque group: trafficsign",
        )

        # Request on same-named layer as an opaque group
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetMap",
                        "LAYERS": "anothersign",
                        "TRANSPARENT": "true",
                        "STYLES": "",
                        "FORMAT": "image/png",
                        "BBOX": "47.51411822721124167,8.76265867349436078,47.51802447721124167,8.76723198210506993",
                        "HEIGHT": "600",
                        "WIDTH": "600",
                        "CRS": "EPSG:4326",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        # self._img_diff_error(
        #     r, h, "WMS_GetMap_OpaqueGroup_anothersign", max_size_diff=QSize(1, 1)
        # )
        self.assertNotIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be no server exception for a layer with a samenamed opaque group: anothersign",
        )

    def testGetFeatureInfo(self):
        """
        Test getfeatureinfo response.

        Request on opaque groups should return like a normal group the features.
        Request on a layer in a normal group returns the features.
        Request on a layer in an opaque group returns an OGC_LayerNotDefined.
        Request on a group in an opaque group returns an OGC_LayerNotDefined.
        Request on a same-named layer should merge
        - but not with a same-named layer in an opaque group
        - but with a same-named opaque group
        """

        # Request on normal group
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetFeatureInfo",
                        "LAYERS": "street-group",
                        "STYLES": "",
                        "INFO_FORMAT": "text/xml",
                        "TRANSPARENT": "true",
                        "WIDTH": "600",
                        "HEIGHT": "600",
                        "CRS": "EPSG:4326",
                        "BBOX": "47.51411822721124167,8.76265867349436078,47.51802447721124167,8.76723198210506993",
                        "QUERY_LAYERS": "street-group",
                        "I": "300",
                        "J": "300",
                        "FI_LINE_TOLERANCE": "300",
                        "FI_POLYGON_TOLERANCE": "300",
                        "FI_LINE_TOLERANCE": "300",
                        "FEATURE_COUNT": "100",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))

        self.assertNotIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be no server exception for the normal layer group: street-group",
        )

        # Request on opaque group
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetFeatureInfo",
                        "LAYERS": "road-group",
                        "STYLES": "",
                        "INFO_FORMAT": "text/xml",
                        "TRANSPARENT": "true",
                        "WIDTH": "600",
                        "HEIGHT": "600",
                        "CRS": "EPSG:4326",
                        "BBOX": "47.51411822721124167,8.76265867349436078,47.51802447721124167,8.76723198210506993",
                        "QUERY_LAYERS": "road-group",
                        "I": "300",
                        "J": "300",
                        "FI_LINE_TOLERANCE": "300",
                        "FI_POLYGON_TOLERANCE": "300",
                        "FI_LINE_TOLERANCE": "300",
                        "FEATURE_COUNT": "100",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))

        self.assertNotIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be no server exception for the opaque layer group: road-group",
        )

        # Request on opaque group with a excluded layers
        # This should return a valid response, but does not. It's IMO a bug https://github.com/qgis/QGIS/issues/65801 but let's clarify. In the end not part of the opaque implementation.
        """
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetFeatureInfo",
                        "LAYERS": "way-group",
                        "STYLES": "",
                        "INFO_FORMAT": "text/xml",
                        "TRANSPARENT": "true",
                        "WIDTH": "600",
                        "HEIGHT": "600",
                        "CRS": "EPSG:4326",
                        "BBOX": "47.51411822721124167,8.76265867349436078,47.51802447721124167,8.76723198210506993",
                        "QUERY_LAYERS": "way-group",
                        "I": "300",
                        "J": "300",
                        "FI_LINE_TOLERANCE": "300",
                        "FI_POLYGON_TOLERANCE": "300",
                        "FI_LINE_TOLERANCE": "300",
                        "FEATURE_COUNT": "100",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))

        self.assertNotIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be no server exception for the opaque layer group: way-group",
        )
        """

        # check if excluded layer is not there

        # Request on opaque group with a subgroup
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetFeatureInfo",
                        "LAYERS": "smallway-group",
                        "STYLES": "",
                        "INFO_FORMAT": "text/xml",
                        "TRANSPARENT": "true",
                        "WIDTH": "600",
                        "HEIGHT": "600",
                        "CRS": "EPSG:4326",
                        "BBOX": "47.51411822721124167,8.76265867349436078,47.51802447721124167,8.76723198210506993",
                        "QUERY_LAYERS": "smallway-group",
                        "I": "300",
                        "J": "300",
                        "FI_LINE_TOLERANCE": "300",
                        "FI_POLYGON_TOLERANCE": "300",
                        "FI_LINE_TOLERANCE": "300",
                        "FEATURE_COUNT": "100",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))

        self.assertNotIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be no server exception for the opaque layer group: smallway-group",
        )

        # Request on group in opaque groups - should OGC_LayerNotDefined
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetFeatureInfo",
                        "LAYERS": "smallway-subgroup",
                        "STYLES": "",
                        "INFO_FORMAT": "text/xml",
                        "TRANSPARENT": "true",
                        "WIDTH": "600",
                        "HEIGHT": "600",
                        "CRS": "EPSG:4326",
                        "BBOX": "47.51411822721124167,8.76265867349436078,47.51802447721124167,8.76723198210506993",
                        "QUERY_LAYERS": "smallway-subgroup",
                        "I": "300",
                        "J": "300",
                        "FI_LINE_TOLERANCE": "300",
                        "FI_POLYGON_TOLERANCE": "300",
                        "FI_LINE_TOLERANCE": "300",
                        "FEATURE_COUNT": "100",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))

        self.assertIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be a server exception for a group inside an opaque layer group: smallway-subgroup",
        )

        # Request on layer in normal group
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetFeatureInfo",
                        "LAYERS": "streetline",
                        "STYLES": "",
                        "INFO_FORMAT": "text/xml",
                        "TRANSPARENT": "true",
                        "WIDTH": "600",
                        "HEIGHT": "600",
                        "CRS": "EPSG:4326",
                        "BBOX": "47.51411822721124167,8.76265867349436078,47.51802447721124167,8.76723198210506993",
                        "QUERY_LAYERS": "streetline",
                        "I": "300",
                        "J": "300",
                        "FI_LINE_TOLERANCE": "300",
                        "FI_POLYGON_TOLERANCE": "300",
                        "FI_LINE_TOLERANCE": "300",
                        "FEATURE_COUNT": "100",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))

        self.assertNotIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be no server exception for a layer inside a normal layer group: streetline",
        )

        # Request on layer in opaque group - should OGC_LayerNotDefined
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetFeatureInfo",
                        "LAYERS": "roadline",
                        "STYLES": "",
                        "INFO_FORMAT": "text/xml",
                        "TRANSPARENT": "true",
                        "WIDTH": "600",
                        "HEIGHT": "600",
                        "CRS": "EPSG:4326",
                        "BBOX": "47.51411822721124167,8.76265867349436078,47.51802447721124167,8.76723198210506993",
                        "QUERY_LAYERS": "roadline",
                        "I": "300",
                        "J": "300",
                        "FI_LINE_TOLERANCE": "300",
                        "FI_POLYGON_TOLERANCE": "300",
                        "FI_LINE_TOLERANCE": "300",
                        "FEATURE_COUNT": "100",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))

        self.assertIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be a server exception for a layer inside an opaque layer group: roadline",
        )

        # Request on layer in non-opaque group of opaque group - should OGC_LayerNotDefined
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetFeatureInfo",
                        "LAYERS": "smallwayline",
                        "STYLES": "",
                        "INFO_FORMAT": "text/xml",
                        "TRANSPARENT": "true",
                        "WIDTH": "600",
                        "HEIGHT": "600",
                        "CRS": "EPSG:4326",
                        "BBOX": "47.51411822721124167,8.76265867349436078,47.51802447721124167,8.76723198210506993",
                        "QUERY_LAYERS": "smallwayline",
                        "I": "300",
                        "J": "300",
                        "FI_LINE_TOLERANCE": "300",
                        "FI_POLYGON_TOLERANCE": "300",
                        "FEATURE_COUNT": "100",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))

        self.assertIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be a server exception for a layer in a non-opaque group of an opaque layer group: smallwayline",
        )

        # Request on a layer with a same-named layer in an opaque group
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetFeatureInfo",
                        "LAYERS": "trafficsign",
                        "STYLES": "",
                        "INFO_FORMAT": "text/xml",
                        "TRANSPARENT": "true",
                        "WIDTH": "600",
                        "HEIGHT": "600",
                        "CRS": "EPSG:4326",
                        "BBOX": "47.51411822721124167,8.76265867349436078,47.51802447721124167,8.76723198210506993",
                        "QUERY_LAYERS": "trafficsign",
                        "I": "300",
                        "J": "300",
                        "FI_LINE_TOLERANCE": "300",
                        "FI_POLYGON_TOLERANCE": "300",
                        "FI_LINE_TOLERANCE": "300",
                        "FEATURE_COUNT": "100",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))

        self.assertNotIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be no server exception for a layer with a samenamed layer inside an opaque group: trafficsign",
        )

        # check if only feature of the layer based on point1 (and not the samenamed in opaque (based on point2, and point3 and point1 again)) are here
        # but they are, because it's still merged - NOT YET IMPLEMENTED

        # Request on same-named layer as an opaque group
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetFeatureInfo",
                        "LAYERS": "anothersign",
                        "STYLES": "",
                        "INFO_FORMAT": "text/xml",
                        "TRANSPARENT": "true",
                        "WIDTH": "600",
                        "HEIGHT": "600",
                        "CRS": "EPSG:4326",
                        "BBOX": "47.51411822721124167,8.76265867349436078,47.51802447721124167,8.76723198210506993",
                        "QUERY_LAYERS": "anothersign",
                        "I": "300",
                        "J": "300",
                        "FI_LINE_TOLERANCE": "300",
                        "FI_POLYGON_TOLERANCE": "300",
                        "FI_LINE_TOLERANCE": "300",
                        "FEATURE_COUNT": "100",
                        "FI_LINE_TOLERANCE": "300",
                        "FI_POLYGON_TOLERANCE": "300",
                        "FI_LINE_TOLERANCE": "300",
                        "FEATURE_COUNT": "100",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))

        self.assertNotIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be no server exception for a layer wiht a samenamed opaque group): anothersign",
        )

        # check if all features are there (layer + opaque group)
        # but they are not, because same-named groups and same-named layers are not merged - NOT YET IMPLEMENTED

    def testGetLegendGraphics(self):
        """
        Test getlegendgraphics response.

        Request on opaque groups should return the legend like a normal group.
        Request on a layer in a normal group returns the legend.
        Request on a layer in an opaque group returns an OGC_LayerNotDefined.
        Request on a group in an opaque group returns an OGC_LayerNotDefined.
        Request on a same-named layer should merge
        - but not with a same-named layer in an opaque group
        - but with a same-named opaque group
        """

        # Request on normal group
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetLegendGraphics",
                        "LAYER": "street-group",
                        "FORMAT": "image/png",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self.assertNotIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be no server exception for the normal layer group: street-group",
        )

        # Request on opaque group
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetLegendGraphics",
                        "LAYER": "road-group",
                        "FORMAT": "image/png",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self.assertNotIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be no server exception for the opaque layer group: road-group",
        )

        # Request on opaque group with a subgroup
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetLegendGraphics",
                        "LAYER": "smallway-group",
                        "FORMAT": "image/png",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self.assertNotIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be no server exception for the opaque layer group: smallway-group",
        )

        # Request on group in opaque groups - should OGC_LayerNotDefined
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetLegendGraphics",
                        "LAYER": "smallway-subgroup",
                        "FORMAT": "image/png",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self.assertIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be a server exception for a group inside an opaque layer group: smallway-subgroup",
        )

        # Request on layer in normal group
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetLegendGraphics",
                        "LAYER": "streetline",
                        "FORMAT": "image/png",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self.assertNotIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be no server exception for a layer inside a normal layer group: streetline",
        )

        # Request on layer in opaque group - should OGC_LayerNotDefined
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetLegendGraphics",
                        "LAYER": "roadline",
                        "FORMAT": "image/png",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self.assertIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be a server exception for a layer inside an opaque layer group: roadline",
        )

        # Request on layer in non-opaque group of opaque group - should OGC_LayerNotDefined
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetLegendGraphics",
                        "LAYER": "smallwayline",
                        "FORMAT": "image/png",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self.assertIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be a server exception for a layer in a non-opaque group of an opaque layer group: smallwayline",
        )

        # Request on a layer with a same-named layer in an opaque group
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetLegendGraphics",
                        "LAYER": "trafficsign",
                        "FORMAT": "image/png",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self.assertNotIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be no server exception for a layer with a samenamed layer inside an opaque group: trafficsign",
        )

        # Request on same-named layer as an opaque group
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetLegendGraphics",
                        "LAYER": "anothersign",
                        "FORMAT": "image/png",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self.assertNotIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be no server exception for a layer with a samenamed opaque group: anothersign",
        )

    def testGetPrint(self):
        """
        Test getprint response.

        Request on opaque groups should return like a normal group the print.
        Request on a layer in a normal group returns the print.
        Request on a layer in an opaque group returns an OGC_LayerNotDefined.
        Request on a group in an opaque group returns an OGC_LayerNotDefined.
        Request on a same-named layer should merge
        - but not with a same-named layer in an opaque group
        - but with a same-named opaque group
        """

        # Request on normal group
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetPrint",
                        "TEMPLATE": "testlayout",
                        "CRS": "EPSG:4326",
                        "map0:EXTENT": "8.76265867,47.51411822,8.76723198,47.51802447",
                        "LAYERS": "street-group",
                        "FORMAT": "pdf",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self.assertNotIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be no server exception for the normal layer group: street-group",
        )

        # Request on opaque group
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetPrint",
                        "TEMPLATE": "testlayout",
                        "CRS": "EPSG:4326",
                        "map0:EXTENT": "8.76265867,47.51411822,8.76723198,47.51802447",
                        "LAYERS": "road-group",
                        "FORMAT": "pdf",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self.assertNotIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be no server exception for the opaque layer group: road-group",
        )

        # Request on opaque group with a excluded layers
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetPrint",
                        "TEMPLATE": "testlayout",
                        "CRS": "EPSG:4326",
                        "map0:EXTENT": "8.76265867,47.51411822,8.76723198,47.51802447",
                        "LAYERS": "way-group",
                        "FORMAT": "pdf",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self.assertNotIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be no server exception for the opaque layer group: way-group",
        )

        # Request on opaque group with a subgroup
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetPrint",
                        "TEMPLATE": "testlayout",
                        "CRS": "EPSG:4326",
                        "map0:EXTENT": "8.76265867,47.51411822,8.76723198,47.51802447",
                        "LAYERS": "smallway-group",
                        "FORMAT": "pdf",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self.assertNotIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be no server exception for the opaque layer group: smallway-group",
        )

        # Request on group in opaque groups - should OGC_LayerNotDefined
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetPrint",
                        "TEMPLATE": "testlayout",
                        "CRS": "EPSG:4326",
                        "map0:EXTENT": "8.76265867,47.51411822,8.76723198,47.51802447",
                        "LAYERS": "smallway-subgroup",
                        "FORMAT": "pdf",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self.assertIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be a server exception for a group inside an opaque layer group: smallway-subgroup",
        )

        # Request on layer in normal group
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetPrint",
                        "TEMPLATE": "testlayout",
                        "CRS": "EPSG:4326",
                        "map0:EXTENT": "8.76265867,47.51411822,8.76723198,47.51802447",
                        "LAYERS": "streetline",
                        "FORMAT": "pdf",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self.assertNotIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be no server exception for a layer inside a normal layer group: streetline",
        )

        # Request on layer in opaque group - should OGC_LayerNotDefined
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetPrint",
                        "TEMPLATE": "testlayout",
                        "CRS": "EPSG:4326",
                        "map0:EXTENT": "8.76265867,47.51411822,8.76723198,47.51802447",
                        "LAYERS": "roadline",
                        "FORMAT": "pdf",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self.assertIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be a server exception for a layer inside an opaque layer group: roadline",
        )

        # Request on layer in non-opaque group of opaque group - should OGC_LayerNotDefined
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetPrint",
                        "TEMPLATE": "testlayout",
                        "CRS": "EPSG:4326",
                        "map0:EXTENT": "8.76265867,47.51411822,8.76723198,47.51802447",
                        "LAYERS": "smallwayline",
                        "FORMAT": "pdf",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self.assertIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be a server exception for a layer in a non-opaque group of an opaque layer group: smallwayline",
        )

        # Request on a layer with a same-named layer in an opaque group
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetPrint",
                        "TEMPLATE": "testlayout",
                        "CRS": "EPSG:4326",
                        "map0:EXTENT": "8.76265867,47.51411822,8.76723198,47.51802447",
                        "LAYERS": "trafficsign",
                        "FORMAT": "pdf",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self.assertNotIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be no server exception for a layer with a samenamed layer inside an opaque group: trafficsign",
        )

        # Request on same-named layer as an opaque group
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetPrint",
                        "TEMPLATE": "testlayout",
                        "CRS": "EPSG:4326",
                        "map0:EXTENT": "8.76265867,47.51411822,8.76723198,47.51802447",
                        "LAYERS": "anothersign",
                        "FORMAT": "pdf",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self.assertNotIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be no server exception for a layer with a samenamed opaque group: anothersign",
        )

    def testDescribeLayer(self):
        """
        Test describelayer response.

        Request on opaque groups should return like a normal group the description.
        Request on a layer in a normal group returns the description.
        Request on a layer in an opaque group returns an OGC_LayerNotDefined.
        Request on a group in an opaque group returns an OGC_LayerNotDefined.
        Request on a same-named layer should merge
        - but not with a same-named layer in an opaque group
        - but with a same-named opaque group
        """

        # Request on normal group
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.1.0",
                        "REQUEST": "DescribeLayer",
                        "LAYERS": "street-group",
                        "SLD_VERSION": "1.1.0",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self.assertNotIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be no server exception for the normal layer group: street-group",
        )

        # Request on opaque group
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.1.0",
                        "REQUEST": "DescribeLayer",
                        "LAYERS": "road-group",
                        "SLD_VERSION": "1.1.0",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self.assertNotIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be no server exception for the opaque layer group: road-group",
        )

        # Request on opaque group with a subgroup
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.1.0",
                        "REQUEST": "DescribeLayer",
                        "LAYERS": "smallway-group",
                        "SLD_VERSION": "1.1.0",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self.assertNotIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be no server exception for the opaque layer group: smallway-group",
        )

        # Request on group in opaque groups - should OGC_LayerNotDefined
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.1.0",
                        "REQUEST": "DescribeLayer",
                        "LAYERS": "smallway-subgroup",
                        "SLD_VERSION": "1.1.0",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self.assertIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be a server exception for a group inside an opaque layer group: smallway-subgroup",
        )

        # Request on layer in normal group
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.1.0",
                        "REQUEST": "DescribeLayer",
                        "LAYERS": "streetline",
                        "SLD_VERSION": "1.1.0",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self.assertNotIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be no server exception for a layer inside a normal layer group: streetline",
        )

        # Request on layer in opaque group - should OGC_LayerNotDefined
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.1.0",
                        "REQUEST": "DescribeLayer",
                        "LAYERS": "roadline",
                        "SLD_VERSION": "1.1.0",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self.assertIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be a server exception for a layer inside an opaque layer group: roadline",
        )

        # Request on layer in non-opaque group of opaque group - should OGC_LayerNotDefined
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.1.0",
                        "REQUEST": "DescribeLayer",
                        "LAYERS": "smallwayline",
                        "SLD_VERSION": "1.1.0",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self.assertIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be a server exception for a layer in a non-opaque group of an opaque layer group: smallwayline",
        )

        # Request on a layer with a same-named layer in an opaque group
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.1.0",
                        "REQUEST": "DescribeLayer",
                        "LAYERS": "trafficsign",
                        "SLD_VERSION": "1.1.0",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self.assertNotIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be no server exception for a layer with a samenamed layer inside an opaque group: trafficsign",
        )

        # Request on same-named layer as an opaque group
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project),
                        "SERVICE": "WMS",
                        "VERSION": "1.1.0",
                        "REQUEST": "DescribeLayer",
                        "LAYERS": "anothersign",
                        "SLD_VERSION": "1.1.0",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self.assertNotIn(
            b'<ServiceException code="LayerNotDefined">',
            r,
            "there should be no server exception for a layer with a samenamed opaque group: anothersign",
        )


if __name__ == "__main__":
    unittest.main()
