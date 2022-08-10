# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsServer.

From build dir, run: ctest -R PyQgsServerAccessControlWFSTransactional -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Stephane Brunner'
__date__ = '28/08/2015'
__copyright__ = 'Copyright 2015, The QGIS Project'

from qgis.testing import unittest
from test_qgsserver_accesscontrol import TestQgsServerAccessControl, XML_NS


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
      <qgs:name>{name}</qgs:name>
      <qgs:color>{color}</qgs:color>
    </qgs:db_point>
  </wfs:Insert>
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
        return 'project_shp.qgs'

    def test_wfstransaction_insert(self):
        data = WFS_TRANSACTION_INSERT.format(x=1, y=2, name="test", color="{color}", gid="{gid}", xml_ns=XML_NS)
        self._test_colors({1: "blue"})

        response, headers = self._post_fullaccess(data.format(color="red", gid=2))
        self.assertEqual(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for Insert is wrong: %s" % headers.get("Content-Type"))

        self.assertTrue(
            str(response).find("<SUCCESS/>") != -1,
            "WFS/Transactions Insert don't succeed\n%s" % response)
        self._test_colors({2: "red"})

        response, headers = self._post_restricted(data.format(color="blue", gid=3))
        self.assertEqual(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for Insert is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find("<SUCCESS/>") == -1,
            "WFS/Transactions Insert succeed\n%s" % response)

        response, headers = self._post_restricted(data.format(color="red", gid=4), "LAYER_PERM=no")
        self.assertEqual(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for Insert is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find(
                '<ServiceException code="Security">No permissions to do WFS changes on layer \\\'db_point\\\'</ServiceException>') != -1,
            "WFS/Transactions Insert succeed\n%s" % response)

        response, headers = self._post_restricted(data.format(color="yellow", gid=5), "LAYER_PERM=yes")
        self.assertEqual(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for Insert is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find("<SUCCESS/>") != -1,
            "WFS/Transactions Insert don't succeed\n%s" % response)
        self._test_colors({5: "yellow"})

    def test_wfstransaction_update(self):
        data = WFS_TRANSACTION_UPDATE.format(id="0", color="{color}", xml_ns=XML_NS)
        self._test_colors({1: "blue"})

        response, headers = self._post_restricted(data.format(color="yellow"))
        self.assertEqual(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for GetMap is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find("<SUCCESS/>") == -1,
            "WFS/Transactions Update succeed\n%s" % response)
        self._test_colors({1: "blue"})

        response, headers = self._post_fullaccess(data.format(color="red"))
        self.assertEqual(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for Update is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find("<SUCCESS/>") != -1,
            "WFS/Transactions Update don't succeed\n%s" % response)
        self._test_colors({1: "red"})

        response, headers = self._post_restricted(data.format(color="blue"))
        self.assertEqual(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for Update is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find("<SUCCESS/>") == -1,
            "WFS/Transactions Update succeed\n%s" % response)
        self._test_colors({1: "red"})

        response, headers = self._post_restricted(data.format(color="yellow"), "LAYER_PERM=no")
        self.assertEqual(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for Update is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find(
                '<ServiceException code="Security">No permissions to do WFS changes on layer \\\'db_point\\\'</ServiceException>') != -1,
            "WFS/Transactions Update succeed\n%s" % response)
        self._test_colors({1: "red"})

        response, headers = self._post_restricted(data.format(color="yellow"), "LAYER_PERM=yes")
        self.assertEqual(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for Update is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find("<SUCCESS/>") != -1,
            "WFS/Transactions Update don't succeed\n%s" % response)
        self._test_colors({1: "yellow"})

    def test_wfstransaction_delete_fullaccess(self):
        data = WFS_TRANSACTION_DELETE.format(id="0", xml_ns=XML_NS)
        self._test_colors({1: "blue"})

        response, headers = self._post_fullaccess(data)
        self.assertEqual(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for GetMap is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find("<SUCCESS/>") != -1,
            "WFS/Transactions Delete didn't succeed\n%s" % response)

    def test_wfstransaction_delete_restricted(self):
        data = WFS_TRANSACTION_DELETE.format(id="0", xml_ns=XML_NS)
        self._test_colors({1: "blue"})

        response, headers = self._post_restricted(data)
        self.assertEqual(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for GetMap is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find("<SUCCESS/>") == -1,
            "WFS/Transactions Delete succeed\n%s" % response)

        data_update = WFS_TRANSACTION_UPDATE.format(id="0", color="red", xml_ns=XML_NS)
        response, headers = self._post_fullaccess(data_update)
        self._test_colors({1: "red"})

        response, headers = self._post_restricted(data, "LAYER_PERM=no")
        self.assertEqual(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for GetMap is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find(
                '<ServiceException code="Security">No permissions to do WFS changes on layer \\\'db_point\\\'</ServiceException>') != -1,
            "WFS/Transactions Delete succeed\n%s" % response)

        response, headers = self._post_restricted(data, "LAYER_PERM=yes")
        self.assertEqual(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for GetMap is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find("<SUCCESS/>") != -1,
            "WFS/Transactions Delete don't succeed\n%s" % response)


if __name__ == "__main__":
    unittest.main()
