"""QGIS Unit tests for QgsServer.

From build dir, run: ctest -R PyQgsServerAccessControlWFSTransactional -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Stephane Brunner"
__date__ = "28/08/2015"
__copyright__ = "Copyright 2015, The QGIS Project"

from qgis.testing import unittest
from qgis.core import QgsProject, QgsVectorLayer, QgsGeometry, QgsOgcUtils
from qgis.server import (
    QgsBufferServerRequest,
    QgsBufferServerResponse,
    QgsServer,
    QgsServerFilter,
    QgsServerRequest,
)
from test_qgsserver_accesscontrol import XML_NS, TestQgsServerAccessControl
from test_qgsserver import QgsServerTestBase
from osgeo import ogr

WFS_TRANSACTION_INSERT = """<?xml version="1.0" encoding="UTF-8"?>
<wfs:Transaction {xml_ns}>
  <wfs:Insert idgen="GenerateNew">
    <qgs:db_point>
      <qgs:geometry>
        <gml:Point srsDimension="2" srsName="http://www.opengis.net/def/crs/EPSG/0/4326">
          <gml:coordinates decimal="." cs="," ts=" ">{x},{y}</gml:coordinates>
        </gml:Point>
      </qgs:geometry>
      <qgs:gid>{gid}</qgs:gid>
      <qgs:name>new_name</qgs:name>
      <qgs:color>{color}</qgs:color>
    </qgs:db_point>
  </wfs:Insert>
</wfs:Transaction>"""

WFS_TRANSACTION_Z_INSERT = """<?xml version="1.0" encoding="UTF-8"?>
<wfs:Transaction {xml_ns}>
    <wfs:Insert idgen="GenerateNew">
        <qgs:{layer_name}>
            <qgs:geometry>
                {gml}
            </qgs:geometry>
            <qgs:gid>1</qgs:gid>
            <qgs:name>name</qgs:name>
            <qgs:color>black</qgs:color>
        </qgs:{layer_name}>
    </wfs:Insert>
</wfs:Transaction>"""

WFS_TRANSACTION_Z_UPDATE = """<?xml version="1.0" encoding="UTF-8"?>
<wfs:Transaction {xml_ns}>
    <wfs:Update typeName="{layer_name}">
        <wfs:Property>
            <wfs:Name>color</wfs:Name>
            <wfs:Value>red</wfs:Value>
        </wfs:Property>
        <wfs:Property>
            <wfs:Name>name</wfs:Name>
            <wfs:Value>new_name</wfs:Value>
        </wfs:Property>
        <wfs:Property>
            <wfs:Name>geometry</wfs:Name>
            <wfs:Value>
                {gml}
            </wfs:Value>
        </wfs:Property>
        <ogc:Filter>
            <ogc:FeatureId fid="{id}"/>
        </ogc:Filter>
    </wfs:Update>
</wfs:Transaction>"""

WFS_TRANSACTION_UPDATE = """<?xml version="1.0" encoding="UTF-8"?>
<wfs:Transaction {xml_ns}>
  <wfs:Update typeName="db_point">
    <wfs:Property>
      <wfs:Name>color</wfs:Name>
      <wfs:Value>{color}</wfs:Value>
    </wfs:Property>
    <ogc:Filter>
      <ogc:FeatureId fid="{id}"/>
   </ogc:Filter>
  </wfs:Update>
</wfs:Transaction>"""

WFS_TRANSACTION_DELETE = """<?xml version="1.0" encoding="UTF-8"?>
<wfs:Transaction {xml_ns}>
  <wfs:Delete typeName="db_point">
    <ogc:Filter>
      <ogc:FeatureId fid="{id}"/>
    </ogc:Filter>
  </wfs:Delete>
</wfs:Transaction>"""


class TestQgsServerAccessControlWFSTransactional(TestQgsServerAccessControl):

    @classmethod
    def project_file(cls):
        return "project_shp.qgs"

    def test_wfstransaction_insert(self):
        data = WFS_TRANSACTION_INSERT.format(
            x=1, y=2, name="test", color="{color}", gid="{gid}", xml_ns=XML_NS
        )
        self._test_colors({1: "blue"})

        response, headers = self._post_fullaccess(data.format(color="red", gid=2))
        self.assertEqual(
            headers.get("Content-Type"),
            "text/xml; charset=utf-8",
            f"Content type for Insert is wrong: {headers.get('Content-Type')}",
        )

        self.assertTrue(
            str(response).find("<SUCCESS/>") != -1,
            f"WFS/Transactions Insert don't succeed\n{response}",
        )
        self._test_colors({2: "red"})

        response, headers = self._post_restricted(data.format(color="blue", gid=3))
        self.assertEqual(
            headers.get("Content-Type"),
            "text/xml; charset=utf-8",
            f"Content type for Insert is wrong: {headers.get('Content-Type')}",
        )
        self.assertTrue(
            str(response).find("<SUCCESS/>") == -1,
            f"WFS/Transactions Insert succeed\n{response}",
        )

        response, headers = self._post_restricted(
            data.format(color="red", gid=4), "LAYER_PERM=no"
        )
        self.assertEqual(
            headers.get("Content-Type"),
            "text/xml; charset=utf-8",
            f"Content type for Insert is wrong: {headers.get('Content-Type')}",
        )
        self.assertTrue(
            str(response).find(
                "<ServiceException code=\"Security\">No permissions to do WFS changes on layer \\'db_point\\'</ServiceException>"
            )
            != -1,
            f"WFS/Transactions Insert succeed\n{response}",
        )

        response, headers = self._post_restricted(
            data.format(color="yellow", gid=5), "LAYER_PERM=yes"
        )
        self.assertEqual(
            headers.get("Content-Type"),
            "text/xml; charset=utf-8",
            f"Content type for Insert is wrong: {headers.get('Content-Type')}",
        )
        self.assertTrue(
            str(response).find("<SUCCESS/>") != -1,
            f"WFS/Transactions Insert don't succeed\n{response}",
        )
        self._test_colors({5: "yellow"})

    def test_wfstransaction_update(self):
        data = WFS_TRANSACTION_UPDATE.format(id="0", color="{color}", xml_ns=XML_NS)
        self._test_colors({1: "blue"})

        response, headers = self._post_restricted(data.format(color="yellow"))
        self.assertEqual(
            headers.get("Content-Type"),
            "text/xml; charset=utf-8",
            f"Content type for GetMap is wrong: {headers.get('Content-Type')}",
        )
        self.assertTrue(
            str(response).find("<SUCCESS/>") == -1,
            f"WFS/Transactions Update succeed\n{response}",
        )
        self._test_colors({1: "blue"})

        response, headers = self._post_fullaccess(data.format(color="red"))
        self.assertEqual(
            headers.get("Content-Type"),
            "text/xml; charset=utf-8",
            f"Content type for Update is wrong: {headers.get('Content-Type')}",
        )
        self.assertTrue(
            str(response).find("<SUCCESS/>") != -1,
            f"WFS/Transactions Update don't succeed\n{response}",
        )
        self._test_colors({1: "red"})

        response, headers = self._post_restricted(data.format(color="blue"))
        self.assertEqual(
            headers.get("Content-Type"),
            "text/xml; charset=utf-8",
            f"Content type for Update is wrong: {headers.get('Content-Type')}",
        )
        self.assertTrue(
            str(response).find("<SUCCESS/>") == -1,
            f"WFS/Transactions Update succeed\n{response}",
        )
        self._test_colors({1: "red"})

        response, headers = self._post_restricted(
            data.format(color="yellow"), "LAYER_PERM=no"
        )
        self.assertEqual(
            headers.get("Content-Type"),
            "text/xml; charset=utf-8",
            f"Content type for Update is wrong: {headers.get('Content-Type')}",
        )
        self.assertTrue(
            str(response).find(
                "<ServiceException code=\"Security\">No permissions to do WFS changes on layer \\'db_point\\'</ServiceException>"
            )
            != -1,
            f"WFS/Transactions Update succeed\n{response}",
        )
        self._test_colors({1: "red"})

        response, headers = self._post_restricted(
            data.format(color="yellow"), "LAYER_PERM=yes"
        )
        self.assertEqual(
            headers.get("Content-Type"),
            "text/xml; charset=utf-8",
            f"Content type for Update is wrong: {headers.get('Content-Type')}",
        )
        self.assertTrue(
            str(response).find("<SUCCESS/>") != -1,
            f"WFS/Transactions Update don't succeed\n{response}",
        )
        self._test_colors({1: "yellow"})

    def test_wfstransaction_delete_fullaccess(self):
        data = WFS_TRANSACTION_DELETE.format(id="0", xml_ns=XML_NS)
        self._test_colors({1: "blue"})

        response, headers = self._post_fullaccess(data)
        self.assertEqual(
            headers.get("Content-Type"),
            "text/xml; charset=utf-8",
            f"Content type for GetMap is wrong: {headers.get('Content-Type')}",
        )
        self.assertTrue(
            str(response).find("<SUCCESS/>") != -1,
            f"WFS/Transactions Delete didn't succeed\n{response}",
        )

    def test_wfstransaction_delete_restricted(self):
        data = WFS_TRANSACTION_DELETE.format(id="0", xml_ns=XML_NS)
        self._test_colors({1: "blue"})

        response, headers = self._post_restricted(data)
        self.assertEqual(
            headers.get("Content-Type"),
            "text/xml; charset=utf-8",
            f"Content type for GetMap is wrong: {headers.get('Content-Type')}",
        )
        self.assertTrue(
            str(response).find("<SUCCESS/>") == -1,
            f"WFS/Transactions Delete succeed\n{response}",
        )

        data_update = WFS_TRANSACTION_UPDATE.format(id="0", color="red", xml_ns=XML_NS)
        response, headers = self._post_fullaccess(data_update)
        self._test_colors({1: "red"})

        response, headers = self._post_restricted(data, "LAYER_PERM=no")
        self.assertEqual(
            headers.get("Content-Type"),
            "text/xml; charset=utf-8",
            f"Content type for GetMap is wrong: {headers.get('Content-Type')}",
        )
        self.assertTrue(
            str(response).find(
                "<ServiceException code=\"Security\">No permissions to do WFS changes on layer \\'db_point\\'</ServiceException>"
            )
            != -1,
            f"WFS/Transactions Delete succeed\n{response}",
        )

        response, headers = self._post_restricted(data, "LAYER_PERM=yes")
        self.assertEqual(
            headers.get("Content-Type"),
            "text/xml; charset=utf-8",
            f"Content type for GetMap is wrong: {headers.get('Content-Type')}",
        )
        self.assertTrue(
            str(response).find("<SUCCESS/>") != -1,
            f"WFS/Transactions Delete don't succeed\n{response}",
        )


class TestQgsServerAccessControlWFSTransactionalZ(QgsServerTestBase):
    """Test transactions with Z coordinate support."""

    project = None

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        # Create a GPKG data and project with point, linestring and polygon layers
        # that supports Z coordinates
        drv = ogr.GetDriverByName("GPKG")
        ds = drv.CreateDataSource(cls.temporary_path + "/test_z.gpkg")
        layer_names = []
        sr = ogr.osr.SpatialReference()
        sr.ImportFromEPSG(4326)
        for wkb_type_name in ["Point", "LineString", "Polygon"]:
            for prefix in ["", "Multi"]:
                wkb_type_name = prefix + wkb_type_name
                wkb_type = getattr(ogr, "wkb" + wkb_type_name + "25D", None)
                layer_name = "test_" + wkb_type_name.lower()
                layer_names.append(layer_name)
                layer = ds.CreateLayer(
                    layer_name,
                    srs=sr,
                    geom_type=wkb_type,
                    options=["GEOMETRY_NAME=geom"],
                )
                layer.CreateField(ogr.FieldDefn("name", ogr.OFTString))
                layer.CreateField(ogr.FieldDefn("color", ogr.OFTString))
                layer.CreateField(ogr.FieldDefn("gid", ogr.OFTInteger))

        del ds

        cls.project = QgsProject()

        for layer_name in layer_names:
            layer = QgsVectorLayer(
                cls.temporary_path + "/test_z.gpkg|layername=" + layer_name,
                layer_name,
                "ogr",
            )
            assert cls.project.addMapLayer(layer)

        # Enable WFS-T support for the layer project
        layer_ids = [layer.id() for layer in cls.project.mapLayers().values()]

        cls.project.writeEntry("WFSLayers", "/", layer_ids)
        for method in ["Insert", "Update", "Delete"]:
            cls.project.writeEntry("WFSTLayers", method, layer_ids)

    def do_operation(self, operation, layer_name, wkt):

        geometry = ogr.Geometry(wkt=wkt)

        xml = operation.format(
            layer_name=layer_name, gml=geometry.ExportToGML(), xml_ns=XML_NS, id=1
        )

        request = QgsBufferServerRequest(
            f"http://server.qgis.org/?SERVICE=WFS&REQUEST=Transaction",
            QgsBufferServerRequest.PostMethod,
            {"Content-Type": "application/xml"},
            xml.encode("utf-8"),
        )

        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, self.project)

        return response

    def do_insert(self, layer_name, wkt):

        geometry = QgsGeometry.fromWkt(wkt)
        geom_wkt = geometry.asWkt().upper()

        response = self.do_operation(WFS_TRANSACTION_Z_INSERT, layer_name, geom_wkt)

        # Check layer
        layer = self.project.mapLayersByName(layer_name)[0]
        self.assertEqual(layer.featureCount(), 1)
        feature = next(layer.getFeatures())
        self.assertEqual(feature["name"], "name")
        self.assertEqual(feature["color"], "black")
        self.assertEqual(feature["gid"], 1)
        self.assertEqual(feature.geometry().asWkt().upper(), geom_wkt)

    def do_update(self, layer_name, wkt):

        geometry = QgsGeometry.fromWkt(wkt)
        geom_wkt = geometry.asWkt().upper()

        response = self.do_operation(WFS_TRANSACTION_Z_UPDATE, layer_name, geom_wkt)

        # Check layer
        layer = self.project.mapLayersByName(layer_name)[0]
        self.assertEqual(layer.featureCount(), 1)
        feature = next(layer.getFeatures())
        self.assertEqual(feature["color"], "red")
        self.assertEqual(feature["name"], "new_name")
        self.assertEqual(feature.geometry().asWkt().upper(), geom_wkt)

    def testGeometries(self):

        self.do_insert("test_point", "POINT Z (1 2 3)")
        self.do_update("test_point", "POINT Z (4 5 6)")

        self.do_insert("test_linestring", "LINESTRING Z (1 2 3, 4 5 6, 7 8 9)")
        self.do_update("test_linestring", "LINESTRING Z (10 11 12, 13 14 15, 16 17 18)")

        self.do_insert("test_polygon", "POLYGON Z ((1 2 3, 1 5 6, 2 5 9, 1 2 3))")
        self.do_update(
            "test_polygon", "POLYGON Z ((11 12 3, 11 15 6, 17 15 9, 11 12 3))"
        )

        self.do_insert("test_multipoint", "MULTIPOINT Z (1 2 3, 4 5 6, 7 8 9)")
        self.do_update("test_multipoint", "MULTIPOINT Z (10 11 12, 13 14 15, 16 17 18)")

        self.do_insert(
            "test_multilinestring",
            "MULTILINESTRING Z ((1 2 3, 4 5 6, 7 8 9), (10 11 12, 13 14 15, 16 17 18))",
        )
        self.do_update(
            "test_multilinestring",
            "MULTILINESTRING Z ((19 20 21, 22 23 24, 25 26 27), (28 29 30, 31 32 33, 34 35 36))",
        )

        self.do_insert(
            "test_multipolygon",
            "MULTIPOLYGON Z (((1 2 3, 1 5 6, 7 5 9, 1 2 3)), ((10 11 12, 10 14 15, 16 14 18, 10 11 12)))",
        )
        self.do_update(
            "test_multipolygon",
            "MULTIPOLYGON Z (((10 11 12, 10 14 15, 16 14 18, 10 11 12)), ((19 20 21, 19 23 24, 25 23 27, 19 20 21)))",
        )


if __name__ == "__main__":
    unittest.main()
