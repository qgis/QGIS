"""QGIS Unit tests for server security.

From build dir, run: ctest -R PyQgsServerSecurity -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""

__author__ = "Paul Blottiere"
__date__ = "31/01/2017"
__copyright__ = "Copyright 2017, The QGIS Project"

import os

from qgis.utils import spatialite_connect

# Needed on Qt 5 so that the serialization of XML is consistent among all executions
os.environ["QT_HASH_SEED"] = "1"

import time
import urllib.parse

import tempfile
from shutil import copyfile

from qgis.core import QgsApplication
from qgis.server import QgsServer
from qgis.testing import unittest
from test_qgsserver import QgsServerTestBase
from utilities import unitTestDataPath


class TestQgsServerSecurity(QgsServerTestBase):

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls.testdatapath = unitTestDataPath("qgis_server_security") + "/"
        cls.db = os.path.join(cls.testdatapath, "db.sqlite")
        cls.db_clone = os.path.join(tempfile.gettempdir(), "db_clone.sqlite")
        cls.project = os.path.join(cls.testdatapath, "project.qgs")
        cls.project_clone = os.path.join(tempfile.gettempdir(), "project.qgs")

    @classmethod
    def tearDownClass(cls):

        try:
            os.remove(cls.db_clone)
        except OSError:
            pass

        super().tearDownClass()

    def setUp(self):
        self.server = QgsServer()
        copyfile(self.db, self.db_clone)
        copyfile(self.project, self.project_clone)

    def test_wms_getfeatureinfo_filter_and_based_blind(self):
        """
        And-based blind attack to check the kind of database currently used (if
        the result is valid for the point named 'b', then sqlite_version()
        function exist).

        But does not work because of the allowlist.

        If you remove the safety check, this is a valid injection.
        """

        filter_sql = "point:\"name\" = 'b'"
        injection_sql = ") and (select sqlite_version()"

        query = f"{filter_sql} {injection_sql}"
        d, h = self.handle_request_wms_getfeatureinfo(query)

        self.assertNotIn(b"name = 'b'", d)

    def test_wms_getfeatureinfo_filter_time_based_blind(self):
        """
        Time-based blind to check the current version of database. If the
        server is too long to respond, then we have the answer!

        But it does not work because of the allowlist.

        If you remove the safety check, this is a valid injection.
        """

        # first step, retrieve the version of sqlite by a regular way
        conn = spatialite_connect(self.db_clone)
        cur = conn.cursor()
        sql = "select sqlite_version()"
        sqlite_version = ""
        for row in cur.execute(sql):
            sqlite_version = row[0]
        conn.close()

        # second step, check the time of response for an invalid version
        filter_sql = "point:\"name\" = 'b'"
        injection_sql = ") and (select case sqlite_version() when '0.0.0' then substr(upper(hex(randomblob(99999999))),0,1) end)--"

        query = f"{filter_sql} {injection_sql}"
        start = time.time()
        d, h = self.handle_request_wms_getfeatureinfo(query)
        duration_invalid_version = time.time() - start

        # third step, check the time of response for a valid version
        # maximum: several seconds
        injection_sql = ") and (select case sqlite_version() when '{}' then substr(upper(hex(randomblob(99999999))),0,1) end)--".format(
            sqlite_version
        )

        query = f"{filter_sql} {injection_sql}"
        start = time.time()
        d, h = self.handle_request_wms_getfeatureinfo(query)
        duration_valid_version = time.time() - start

        # compare duration. On my computer when safety check is deactivated:
        # duration_invalid_version: 0.012360334396362305
        # duration_valid_version: 2.8810460567474365
        self.assertAlmostEqual(
            duration_valid_version, duration_invalid_version, delta=0.5
        )

    def test_wms_getfeatureinfo_filter_stacked(self):
        """
        The aim is to execute some staked queries. Here the 'drop' function is
        used but it could be done with create, insert, ...

        But the filter string is split thanks to the semicolon so it seems
        totally ignored whatever the query is (even without the safety check).
        """

        filter_sql = "point:\"name\" = 'fake'"
        injection_sql = "); drop table point"

        query = f"{filter_sql} {injection_sql}"
        d, h = self.handle_request_wms_getfeatureinfo(query)

        self.assertTrue(self.is_point_table_still_exist())

    def test_wms_getfeatureinfo_filter_union_0(self):
        """
        The aim is to retrieve name of tables within the database (like
        'SpatialIndex').

        But the allowlist blocks this request because of invalid tokens.

        If you remove the safety check, this is a valid injection.
        """

        filter_sql = "point:\"name\" = 'fake'"
        injection_sql = ') union select 1,1,name,1,1 from sqlite_master where type = "table" order by name--'

        query = f"{filter_sql} {injection_sql}"
        d, h = self.handle_request_wms_getfeatureinfo(query)

        self.assertNotIn(b"SpatialIndex", d)

    def test_wms_getfeatureinfo_filter_union_1(self):
        """
        The aim is to retrieve data from an excluded layer.

        But the allowlist blocks this request because of invalid tokens.

        If you remove the safety check, this is a valid injection.
        """

        filter_sql = "point:\"name\" = 'fake'"
        injection_sql = ") union select 1,1,* from aoi--"

        query = f"{filter_sql} {injection_sql}"
        d, h = self.handle_request_wms_getfeatureinfo(query)

        self.assertNotIn(b"private_value", d)

    def test_wms_getfeatureinfo_filter_unicode(self):
        """
        The aim is to send some invalid token in unicode to bypass the
        allowlist.

        But unicode is interpreted and checked by the safety function.
        """

        # %3B -> semicolon
        filter_sql = "point:\"name\" = 'fake %3B'"
        d, h = self.handle_request_wms_getfeatureinfo(filter_sql)
        self.assertTrue(self.check_service_exception_report(d))

    def test_wms_getfeatureinfo_filter_patternmatching(self):
        """
        The aim is to retrieve the table's name thanks to pattern matching.

        If you remove the safety check, this is a valid injection.
        """

        filter_sql = "point:\"name\" = 'b'"
        injection_sql = "or ( select name from sqlite_master where type='table' and name like '{0}') != ''"
        query = f"{filter_sql} {injection_sql}"

        # there's no table named as 'az%'
        name = "az%"
        sql = query.format(name)
        d, h = self.handle_request_wms_getfeatureinfo(sql)
        # self.assertTrue(b"name = 'b'" in d) #true if sanity check deactivated
        self.assertTrue(self.check_service_exception_report(d))

        # a table named as 'ao%' exist
        name = "ao%"
        sql = query.format(name)
        d, h = self.handle_request_wms_getfeatureinfo(sql)
        # self.assertTrue(b"name = 'a'" in d) #true if sanity check deactivated
        self.assertTrue(self.check_service_exception_report(d))

        # a table named as 'aoi' exist
        name = "aoi"
        sql = query.format(name)
        d, h = self.handle_request_wms_getfeatureinfo(sql)
        # self.assertTrue(b"name = 'a'" in d) #true if sanity check deactivated
        self.assertTrue(self.check_service_exception_report(d))

    def test_wms_getfeatureinfo_filter_allowlist(self):
        """
        The aim is to check that some tokens cannot pass the safety check
        whatever their positions in the filter string.
        """

        # create
        filter_sql = "point:\"name\" = 'a'create"
        d, h = self.handle_request_wms_getfeatureinfo(filter_sql)
        self.assertTrue(self.check_service_exception_report(d))

        filter_sql = "point:\"name\" = 'a' create"
        d, h = self.handle_request_wms_getfeatureinfo(filter_sql)
        self.assertTrue(self.check_service_exception_report(d))

        # invalid token and escape single quote
        filter_sql = "point:\"name\" = 'a\\'create"
        d, h = self.handle_request_wms_getfeatureinfo(filter_sql)
        self.assertTrue(self.check_service_exception_report(d))

        # drop
        filter_sql = "point:\"name\" = 'a' drop"
        d, h = self.handle_request_wms_getfeatureinfo(filter_sql)
        self.assertTrue(self.check_service_exception_report(d))

        # select
        filter_sql = "point:\"name\" = 'a' select"
        d, h = self.handle_request_wms_getfeatureinfo(filter_sql)
        self.assertTrue(self.check_service_exception_report(d))

        # comments
        # filter_sql = "point:\"name\" = 'a' #"
        # d, h = self.handle_request_wms_getfeatureinfo(filter_sql)
        # self.assertTrue(self.check_service_exception_report(d))

        filter_sql = "point:\"name\" = 'a' -"
        d, h = self.handle_request_wms_getfeatureinfo(filter_sql)
        self.assertTrue(self.check_service_exception_report(d))

    def test_wfs_getfeature_filter_stacked(self):
        """
        The aim is to execute some staked queries within the 'Literal'
        and 'PropertyName' field. Here the 'drop' function is used but it
        could be done with create, insert, ...

        But due to the implementation, these filters are not resolved on
        database side but in server side with QgsExpression. So, there's no
        'WHERE' clause and the query never reach the database. By the way,
        it's exactly the same thing whatever the kind of attacks and for
        the EXP_FILTER parameter too (filter described with QgsExpression).

        It's typically the kind of SQL injection which has been fixed in
        mapserver several years ago:
        https://trac.osgeo.org/mapserver/ticket/3874
        """

        # ogc:Literal / ogc:PropertyIsEqualTo
        literal = "4')); drop table point --"
        filter_xml = '<ogc:Filter%20xmlns:ogc="http://www.opengis.net/ogc"><ogc:PropertyIsEqualTo><ogc:PropertyName>pkuid</ogc:PropertyName><ogc:Literal>{}</ogc:Literal></ogc:PropertyIsEqualTo></ogc:Filter>'.format(
            literal
        )
        self.handle_request_wfs_getfeature_filter(filter_xml)
        self.assertTrue(self.is_point_table_still_exist())

        # ogc:Literal / ogc:PropertyIsLike
        literal = "4')); drop table point --"
        filter_xml = '<ogc:Filter%20xmlns:ogc="http://www.opengis.net/ogc"><ogc:PropertyIsLike><ogc:PropertyName>pkuid</ogc:PropertyName><ogc:Literal>{}</ogc:Literal></ogc:PropertyIsLike></ogc:Filter>'.format(
            literal
        )
        self.handle_request_wfs_getfeature_filter(filter_xml)
        self.assertTrue(self.is_point_table_still_exist())

        # ogc:PropertyName / ogc:PropertyIsLike
        propname = "name = 'a')); drop table point --"
        filter_xml = '<ogc:Filter%20xmlns:ogc="http://www.opengis.net/ogc"><ogc:PropertyIsLike><ogc:PropertyName>{}</ogc:PropertyName><ogc:Literal>4</ogc:Literal></ogc:PropertyIsLike></ogc:Filter>'.format(
            propname
        )
        self.handle_request_wfs_getfeature_filter(filter_xml)
        self.assertTrue(self.is_point_table_still_exist())

    def test_wms_getmap_filter_stacked(self):
        """
        The aim is to execute some staked queries within the 'Literal'
        and 'PropertyName' field. Here the 'drop' function is used but it
        could be done with create, insert, ...

        But due to the implementation, these filters quoted before being sent to the DB.

        It's typically the kind of SQL injection which has been fixed in
        mapserver several years ago:
        https://trac.osgeo.org/mapserver/ticket/3874
        """

        # ogc:Literal / ogc:PropertyIsEqualTo
        literal = "4')); drop table point --"
        filter_xml = '<ogc:Filter%20xmlns:ogc="http://www.opengis.net/ogc"><ogc:PropertyIsEqualTo><ogc:PropertyName>pkuid</ogc:PropertyName><ogc:Literal>{}</ogc:Literal></ogc:PropertyIsEqualTo></ogc:Filter>'.format(
            literal
        )
        self.handle_request_wms_getmap(filter=filter_xml)
        self.assertTrue(self.is_point_table_still_exist())

        # ogc:Literal / ogc:PropertyIsLike
        literal = "4')); drop table point --"
        filter_xml = '<ogc:Filter%20xmlns:ogc="http://www.opengis.net/ogc"><ogc:PropertyIsLike><ogc:PropertyName>pkuid</ogc:PropertyName><ogc:Literal>{}</ogc:Literal></ogc:PropertyIsLike></ogc:Filter>'.format(
            literal
        )
        self.handle_request_wms_getmap(filter=filter_xml)
        self.assertTrue(self.is_point_table_still_exist())

        # ogc:PropertyName / ogc:PropertyIsLike
        propname = "name = 'a')); drop table point --"
        filter_xml = '<ogc:Filter%20xmlns:ogc="http://www.opengis.net/ogc"><ogc:PropertyIsLike><ogc:PropertyName>{}</ogc:PropertyName><ogc:Literal>4</ogc:Literal></ogc:PropertyIsLike></ogc:Filter>'.format(
            propname
        )
        self.handle_request_wms_getmap(filter=filter_xml)
        self.assertTrue(self.is_point_table_still_exist())

    def test_wms_getmap_sld_stacked(self):
        """
        The aim is to execute some staked queries within the 'Literal'
        and 'PropertyName' field. Here the 'drop' function is used but it
        could be done with create, insert, ...

        However it's not working because special characters are duplicated. For
        example, with 'Literal' as "4')); drop table point --", the underlying
        query is:
            SELECT .... AND (("pkuid" = '4'')); drop table point --'))
        """

        literal = "4')); drop table point --"
        sld = '<?xml version="1.0" encoding="UTF-8"?><StyledLayerDescriptor xmlns="http://www.opengis.net/sld" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:ogc="http://www.opengis.net/ogc" xsi:schemaLocation="http://www.opengis.net/sld http://schemas.opengis.net/sld/1.1.0/StyledLayerDescriptor.xsd" version="1.1.0" xmlns:se="http://www.opengis.net/se" xmlns:xlink="http://www.w3.org/1999/xlink"> <NamedLayer> <se:Name>point</se:Name> <UserStyle> <se:Name>point</se:Name> <se:FeatureTypeStyle> <se:Rule> <se:Name>Single symbol</se:Name> <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc"> <ogc:PropertyIsEqualTo> <ogc:PropertyName>pkuid</ogc:PropertyName> <ogc:Literal>{}</ogc:Literal> </ogc:PropertyIsEqualTo> </ogc:Filter> <se:PointSymbolizer> <se:Graphic> <se:Mark> <se:WellKnownName>circle</se:WellKnownName> <se:Fill><se:SvgParameter name="fill">5e86a1</se:SvgParameter></se:Fill><se:Stroke><se:SvgParameter name="stroke">000000</se:SvgParameter></se:Stroke></se:Mark><se:Size>7</se:Size></se:Graphic></se:PointSymbolizer></se:Rule></se:FeatureTypeStyle></UserStyle></NamedLayer></StyledLayerDescriptor>'.format(
            literal
        )
        self.handle_request_wms_getmap(sld=sld)
        self.assertTrue(self.is_point_table_still_exist())

    def check_service_exception_report(self, d):
        """
        Return True if a ServiceExceptionReport is raised, False otherwise
        """

        if b"<ServiceExceptionReport" in d:
            return True
        else:
            return False

    def handle_request_wfs_getfeature_filter(self, filter_xml):
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in {
                    "MAP": urllib.parse.quote(self.project_clone),
                    "SERVICE": "WFS",
                    "VERSION": "1.1.1",
                    "REQUEST": "GetFeature",
                    "TYPENAME": "point",
                    "STYLES": "",
                    "CRS": "EPSG:32613",
                    "FILTER": filter_xml,
                }.items()
            ]
        )

        return self._execute_request(qs)

    def handle_request_wms_getfeatureinfo(self, filter_sql):
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in {
                    "MAP": urllib.parse.quote(self.project_clone),
                    "SERVICE": "WMS",
                    "VERSION": "1.1.1",
                    "REQUEST": "GetFeatureInfo",
                    "QUERY_LAYERS": "point",
                    "LAYERS": "point",
                    "STYLES": "",
                    "FORMAT": "image/png",
                    "HEIGHT": "500",
                    "WIDTH": "500",
                    "BBOX": "606171,4822867,612834,4827375",
                    "CRS": "EPSG:32613",
                    "FILTER": filter_sql,
                }.items()
            ]
        )

        return self._result(self._execute_request(qs))

    def handle_request_wms_getmap(self, sld=None, filter=None):
        params = {
            "MAP": urllib.parse.quote(self.project_clone),
            "SERVICE": "WMS",
            "VERSION": "1.0.0",
            "REQUEST": "GetMap",
            "QUERY_LAYERS": "point",
            "LAYERS": "point",
            "STYLES": "",
            "FORMAT": "image/png",
            "HEIGHT": "500",
            "WIDTH": "500",
            "BBOX": "606171,4822867,612834,4827375",
            "CRS": "EPSG:32613",
        }
        if sld is not None:
            params["SLD"] = sld
        if filter is not None:
            params["FILTER"] = urllib.parse.quote(filter)
        qs = "?" + "&".join(["%s=%s" % i for i in params.items()])

        return self._result(self._execute_request(qs))

    def is_point_table_still_exist(self):
        conn = spatialite_connect(self.db_clone)
        cur = conn.cursor()
        sql = "select * from point"
        point_table_exist = True
        try:
            cur.execute(sql)
        except:
            point_table_exist = False
        conn.close()

        return point_table_exist

    def _result(self, data):
        headers = {}
        for line in data[0].decode("UTF-8").split("\n"):
            if line != "":
                header = line.split(":")
                self.assertEqual(len(header), 2, line)
                headers[str(header[0])] = str(header[1]).strip()

        return data[1], headers


if __name__ == "__main__":
    unittest.main()
