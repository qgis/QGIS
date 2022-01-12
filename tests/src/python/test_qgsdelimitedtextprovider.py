# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsDelimitedTextProvider.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Chris Crook'
__date__ = '20/04/2013'
__copyright__ = 'Copyright 2013, The QGIS Project'

# This module provides unit test for the delimited text provider.  It uses data files in
# the testdata/delimitedtext directory.
#
# New tests can be created (or existing ones updated), but incorporating a createTest
# call into the test.  This will load the file and generate a test that the features
# loaded from it are correct.  It assumes that the data is correct at the time the
# test is created.  The new test is written to the test output file, and can be edited into
# this module to implement the test.
#
# To recreate all tests, set rebuildTests to true

import qgis  # NOQA

import os
import re
import tempfile
import inspect
import time
import test_qgsdelimitedtextprovider_wanted as want  # NOQA

from collections.abc import Callable

rebuildTests = 'REBUILD_DELIMITED_TEXT_TESTS' in os.environ

from qgis.PyQt.QtCore import QCoreApplication, QVariant, QUrl, QObject, QTemporaryDir, QDate

from qgis.core import (
    QgsGeometry,
    QgsProviderRegistry,
    QgsVectorLayer,
    QgsFeatureRequest,
    QgsRectangle,
    QgsApplication,
    QgsFeature,
    QgsWkbTypes,
    QgsFeatureSource,
    NULL)

from qgis.testing import start_app, unittest
from utilities import unitTestDataPath, compareWkt, compareUrl

from providertestbase import ProviderTestCase

start_app()
TEST_DATA_DIR = unitTestDataPath()

geomkey = "#geometry"
fidkey = "#fid"

try:
    # Qt 5
    from qgis.PyQt.QtCore import QUrlQuery

    class MyUrl:

        def __init__(self, url):
            self.url = url
            self.query = QUrlQuery()

        @classmethod
        def fromLocalFile(cls, filename):
            return cls(QUrl.fromLocalFile(filename))

        def addQueryItem(self, k, v):
            self.query.addQueryItem(k, v)

        def toString(self):
            urlstr = self.url.toString()
            querystr = self.query.toString(QUrl.FullyDecoded)
            if querystr != '':
                urlstr += '?'
                urlstr += querystr
            return urlstr
except:
    MyUrl = QUrl


def normalize_query_items_order(s):
    split_url = s.split('?')
    urlstr = split_url[0]
    if len(split_url) == 2:
        items_list = split_url[1].split('&')
        items_map = {}
        for item in items_list:
            split_item = item.split('=')
            items_map[split_item[0]] = split_item[1]
        first_arg = True
        for k in sorted(items_map.keys()):
            if first_arg:
                urlstr += '?'
                first_arg = False
            else:
                urlstr += '&'
            urlstr += k + '=' + items_map[k]
    return urlstr

# Thought we could connect to messageReceived signal but doesn't seem to be available
# in python :-(  Not sure why?


class MessageLogger(QObject):

    def __init__(self, tag=None):
        QObject.__init__(self)
        self.log = []
        self.tag = tag

    def __enter__(self):
        QgsApplication.messageLog().messageReceived.connect(self.logMessage)
        return self

    def __exit__(self, type, value, traceback):
        QgsApplication.messageLog().messageReceived.disconnect(self.logMessage)

    def logMessage(self, msg, tag, level):
        if tag == self.tag or not self.tag:
            self.log.append(str(msg))

    def messages(self):
        return self.log


class TestQgsDelimitedTextProviderXY(unittest.TestCase, ProviderTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        # Create test layer
        srcpath = os.path.join(TEST_DATA_DIR, 'provider')
        cls.basetestfile = os.path.join(srcpath, 'delimited_xy.csv')

        url = MyUrl.fromLocalFile(cls.basetestfile)
        url.addQueryItem("crs", "epsg:4326")
        url.addQueryItem("type", "csv")
        url.addQueryItem("xField", "X")
        url.addQueryItem("yField", "Y")
        url.addQueryItem("spatialIndex", "no")
        url.addQueryItem("subsetIndex", "no")
        url.addQueryItem("watchFile", "no")

        cls.vl = QgsVectorLayer(url.toString(), 'test', 'delimitedtext')
        assert cls.vl.isValid(), "{} is invalid".format(cls.basetestfile)
        cls.source = cls.vl.dataProvider()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""

    def treat_time_as_string(self):
        return False

    def treat_date_as_string(self):
        return False

    def treat_datetime_as_string(self):
        return False


class TestQgsDelimitedTextProviderWKT(unittest.TestCase, ProviderTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        # Create test layer
        srcpath = os.path.join(TEST_DATA_DIR, 'provider')
        cls.basetestfile = os.path.join(srcpath, 'delimited_wkt.csv')

        url = MyUrl.fromLocalFile(cls.basetestfile)
        url.addQueryItem("crs", "epsg:4326")
        url.addQueryItem("type", "csv")
        url.addQueryItem("wktField", "wkt")
        url.addQueryItem("spatialIndex", "no")
        url.addQueryItem("subsetIndex", "no")
        url.addQueryItem("watchFile", "no")

        cls.vl = QgsVectorLayer(url.toString(), 'test', 'delimitedtext')
        assert cls.vl.isValid(), "{} is invalid".format(cls.basetestfile)
        cls.source = cls.vl.dataProvider()

        cls.basetestpolyfile = os.path.join(srcpath, 'delimited_wkt_poly.csv')

        url = MyUrl.fromLocalFile(cls.basetestpolyfile)
        url.addQueryItem("crs", "epsg:4326")
        url.addQueryItem("type", "csv")
        url.addQueryItem("wktField", "wkt")
        url.addQueryItem("spatialIndex", "no")
        url.addQueryItem("subsetIndex", "no")
        url.addQueryItem("watchFile", "no")

        cls.vl_poly = QgsVectorLayer(url.toString(), 'test_polygon', 'delimitedtext')
        assert cls.vl_poly.isValid(), "{} is invalid".format(cls.basetestpolyfile)
        cls.poly_provider = cls.vl_poly.dataProvider()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""

    def treat_time_as_string(self):
        return False

    def treat_date_as_string(self):
        return False

    def treat_datetime_as_string(self):
        return False


class TestQgsDelimitedTextProviderOther(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        # toggle full ctest output to debug flaky CI test
        print('CTEST_FULL_OUTPUT')
        cls.tmp_dir = QTemporaryDir()
        cls.tmp_path = cls.tmp_dir.path()
        cls._text_index = 0

    def layerData(self, layer, request={}, offset=0):
        # Retrieve the data for a layer
        first = True
        data = {}
        fields = []
        fieldTypes = []
        fr = QgsFeatureRequest()
        if request:
            if 'exact' in request and request['exact']:
                fr.setFlags(QgsFeatureRequest.ExactIntersect)
            if 'nogeom' in request and request['nogeom']:
                fr.setFlags(QgsFeatureRequest.NoGeometry)
            if 'fid' in request:
                fr.setFilterFid(request['fid'])
            elif 'extents' in request:
                fr.setFilterRect(QgsRectangle(*request['extents']))
            if 'attributes' in request:
                fr.setSubsetOfAttributes(request['attributes'])

        # IMPORTANT - we do not use `for f in layer.getFeatures(fr):` as we need
        # to verify that existing attributes and geometry are correctly cleared
        # from the feature when calling nextFeature()
        it = layer.getFeatures(fr)
        f = QgsFeature()
        while it.nextFeature(f):
            if first:
                first = False
                for field in f.fields():
                    fields.append(str(field.name()))
                    fieldTypes.append(str(field.typeName()))
            fielddata = dict((name, str(f[name])) for name in fields)
            g = f.geometry()
            if not g.isNull():
                fielddata[geomkey] = str(g.asWkt())
            else:
                fielddata[geomkey] = "None"

            fielddata[fidkey] = f.id()
            id = fielddata[fields[0]]
            description = fielddata[fields[1]]
            fielddata['id'] = id
            fielddata['description'] = description
            data[f.id() + offset] = fielddata

        if 'id' not in fields:
            fields.insert(0, 'id')
        if 'description' not in fields:
            fields.insert(1, 'description')
        fields.append(fidkey)
        fields.append(geomkey)
        return fields, fieldTypes, data

    def delimitedTextData(self, testname, filename, requests, verbose, **params):
        # Retrieve the data for a delimited text url
        # Create a layer for the specified file and query parameters
        # and return the data for the layer (fields, data)

        filepath = os.path.join(unitTestDataPath("delimitedtext"), filename)
        url = MyUrl.fromLocalFile(filepath)
        if not requests:
            requests = [{}]
        for k in list(params.keys()):
            url.addQueryItem(k, params[k])
        urlstr = url.toString()
        log = []
        with MessageLogger('DelimitedText') as logger:
            if verbose:
                print(testname)
            layer = QgsVectorLayer(urlstr, 'test', 'delimitedtext')

            # decodeUri / encodeUri check
            self.assertTrue(compareUrl(layer.source(), QgsProviderRegistry.instance().encodeUri('delimitedtext', QgsProviderRegistry.instance().decodeUri('delimitedtext', layer.source()))))

            uri = layer.dataProvider().dataSourceUri()
            if verbose:
                print(uri)
            basename = os.path.basename(filepath)
            if not basename.startswith('test'):
                basename = 'file'
            uri = re.sub(r'^file\:\/\/[^\?]*', 'file://' + basename, uri)
            fields = []
            fieldTypes = []
            data = {}
            if layer.isValid():
                for nr, r in enumerate(requests):
                    if verbose:
                        print(("Processing request", nr + 1, repr(r)))
                    if isinstance(r, Callable):
                        r(layer)
                        if verbose:
                            print("Request function executed")
                    if isinstance(r, Callable):
                        continue
                    rfields, rtypes, rdata = self.layerData(layer, r, nr * 1000)
                    if len(rfields) > len(fields):
                        fields = rfields
                        fieldTypes = rtypes
                    data.update(rdata)
                    if not rdata:
                        log.append("Request " + str(nr) + " did not return any data")
                    if verbose:
                        print(("Request returned", len(list(rdata.keys())), "features"))
            for msg in logger.messages():
                filelogname = 'temp_file' if 'tmp' in filename.lower() else filename
                msg = re.sub(r'file\s+.*' + re.escape(filename), 'file ' + filelogname, msg)
                msg = msg.replace(filepath, filelogname)
                log.append(msg)
            return dict(fields=fields, fieldTypes=fieldTypes, data=data, log=log, uri=uri, geometryType=layer.geometryType())

    def printWanted(self, testname, result):
        # Routine to export the result as a function definition
        print()
        print(("def {0}():".format(testname)))
        data = result['data']
        log = result['log']
        fields = result['fields']
        prefix = '    '

        # Dump the data for a layer - used to construct unit tests
        print((prefix + "wanted={}"))
        print((prefix + "wanted['uri']=" + repr(result['uri'])))
        print((prefix + "wanted['fieldTypes']=" + repr(result['fieldTypes'])))
        print((prefix + "wanted['geometryType']=" + repr(result['geometryType'])))
        print((prefix + "wanted['data']={"))
        for k in sorted(data.keys()):
            row = data[k]
            print((prefix + "    {0}: {{".format(repr(k))))
            for f in fields:
                print((prefix + "        " + repr(f) + ": " + repr(row[f]) + ","))
            print((prefix + "        },"))
        print((prefix + "    }"))

        print((prefix + "wanted['log']=["))
        for msg in log:
            print((prefix + '    ' + repr(msg) + ','))
        print((prefix + '    ]'))
        print('    return wanted')
        print('', flush=True)

    def recordDifference(self, record1, record2):
        # Compare a record defined as a dictionary
        for k in list(record1.keys()):
            if k not in record2:
                return "Field {0} is missing".format(k)
            r1k = record1[k]
            r2k = record2[k]
            if k == geomkey:
                if not compareWkt(r1k, r2k):
                    return "Geometry differs: {0:.50} versus {1:.50}".format(r1k, r2k)
            else:
                if record1[k] != record2[k]:
                    return "Field {0} differs: {1:.50} versus {2:.50}".format(k, repr(r1k), repr(r2k))
        for k in list(record2.keys()):
            if k not in record1:
                return "Output contains extra field {0}".format(k)
        return ''

    def runTest(self, file, requests, **params):
        testname = inspect.stack()[1][3]
        verbose = not rebuildTests
        if verbose:
            print(("Running test:", testname))
        result = self.delimitedTextData(testname, file, requests, verbose, **params)
        if rebuildTests:
            self.printWanted(testname, result)
            assert False, "Test not run - being rebuilt"
        try:
            wanted = eval('want.{0}()'.format(testname))
        except:
            self.printWanted(testname, result)
            assert False, "Test results not available for {0}".format(testname)

        data = result['data']
        log = result['log']
        failures = []
        if normalize_query_items_order(result['uri']) != normalize_query_items_order(wanted['uri']):
            msg = "Layer Uri ({0}) doesn't match expected ({1})".format(
                normalize_query_items_order(result['uri']), normalize_query_items_order(wanted['uri']))
            print(('    ' + msg))
            failures.append(msg)
        if result['fieldTypes'] != wanted['fieldTypes']:
            msg = "Layer field types ({0}) doesn't match expected ({1})".format(
                result['fieldTypes'], wanted['fieldTypes'])
            failures.append(msg)
        if result['geometryType'] != wanted['geometryType']:
            msg = "Layer geometry type ({0}) doesn't match expected ({1})".format(
                result['geometryType'], wanted['geometryType'])
            failures.append(msg)
        wanted_data = wanted['data']
        for id in sorted(wanted_data.keys()):
            print('getting wanted data')
            wrec = wanted_data[id]
            print('getting received data')
            trec = data.get(id, {})
            print('getting description')
            description = wrec['description']
            print('getting difference')
            difference = self.recordDifference(wrec, trec)
            if not difference:
                print(('    {0}: Passed'.format(description)))
            else:
                print(('    {0}: {1}'.format(description, difference)))
                failures.append(description + ': ' + difference)
        for id in sorted(data.keys()):
            if id not in wanted_data:
                msg = "Layer contains unexpected extra data with id: \"{0}\"".format(id)
                print(('    ' + msg))
                failures.append(msg)
        common = []
        log_wanted = wanted['log']
        for l in log:
            if l in log_wanted:
                common.append(l)
        for l in log_wanted:
            if l not in common:
                msg = 'Missing log message: ' + l
                print(('    ' + msg))
                failures.append(msg)
        for l in log:
            if l not in common:
                msg = 'Extra log message: ' + l
                print(('    ' + msg))
                failures.append(msg)
        if len(log) == len(common) and len(log_wanted) == len(common):
            print('    Message log correct: Passed')

        if failures:
            self.printWanted(testname, result)

        assert len(failures) == 0, "\n".join(failures)

    def test_001_provider_defined(self):
        registry = QgsProviderRegistry.instance()
        metadata = registry.providerMetadata('delimitedtext')
        assert metadata is not None, "Delimited text provider is not installed"

    def test_002_load_csv_file(self):
        # CSV file parsing
        filename = 'test.csv'
        params = {'geomType': 'none', 'type': 'csv'}
        requests = None
        self.runTest(filename, requests, **params)

    def test_003_field_naming(self):
        # Management of missing/duplicate/invalid field names
        filename = 'testfields.csv'
        params = {'geomType': 'none', 'type': 'csv'}
        requests = None
        self.runTest(filename, requests, **params)

    def test_004_max_fields(self):
        # Limiting maximum number of fields
        filename = 'testfields.csv'
        params = {'geomType': 'none', 'maxFields': '7', 'type': 'csv'}
        requests = None
        self.runTest(filename, requests, **params)

    def test_005_load_whitespace(self):
        # Whitespace file parsing
        filename = 'test.space'
        params = {'geomType': 'none', 'type': 'whitespace'}
        requests = None
        self.runTest(filename, requests, **params)

    def test_006_quote_escape(self):
        # Quote and escape file parsing
        filename = 'test.pipe'
        params = {'geomType': 'none', 'quote': '"', 'delimiter': '|', 'escape': '\\'}
        requests = None
        self.runTest(filename, requests, **params)

    def test_007_multiple_quote(self):
        # Multiple quote and escape characters
        filename = 'test.quote'
        params = {'geomType': 'none', 'quote': '\'"', 'type': 'csv', 'escape': '"\''}
        requests = None
        self.runTest(filename, requests, **params)

    def test_008_badly_formed_quotes(self):
        # Badly formed quoted fields
        filename = 'test.badquote'
        params = {'geomType': 'none', 'quote': '"', 'type': 'csv', 'escape': '"'}
        requests = None
        self.runTest(filename, requests, **params)

    def test_009_skip_lines(self):
        # Skip lines
        filename = 'test2.csv'
        params = {'geomType': 'none', 'useHeader': 'no', 'type': 'csv', 'skipLines': '2'}
        requests = None
        self.runTest(filename, requests, **params)

    def test_010_read_coordinates(self):
        # Skip lines
        filename = 'testpt.csv'
        params = {'yField': 'geom_y', 'xField': 'geom_x', 'type': 'csv'}
        requests = None
        self.runTest(filename, requests, **params)

    def test_011_read_wkt(self):
        # Reading WKT geometry field
        filename = 'testwkt.csv'
        params = {'delimiter': '|', 'type': 'csv', 'wktField': 'geom_wkt'}
        requests = None
        self.runTest(filename, requests, **params)

    def test_012_read_wkt_point(self):
        # Read WKT points
        filename = 'testwkt.csv'
        params = {'geomType': 'point', 'delimiter': '|', 'type': 'csv', 'wktField': 'geom_wkt'}
        requests = None
        self.runTest(filename, requests, **params)

    def test_013_read_wkt_line(self):
        # Read WKT linestrings
        filename = 'testwkt.csv'
        params = {'geomType': 'line', 'delimiter': '|', 'type': 'csv', 'wktField': 'geom_wkt'}
        requests = None
        self.runTest(filename, requests, **params)

    def test_014_read_wkt_polygon(self):
        # Read WKT polygons
        filename = 'testwkt.csv'
        params = {'geomType': 'polygon', 'delimiter': '|', 'type': 'csv', 'wktField': 'geom_wkt'}
        requests = None
        self.runTest(filename, requests, **params)

    def test_015_read_dms_xy(self):
        # Reading degrees/minutes/seconds angles
        filename = 'testdms.csv'
        params = {'yField': 'lat', 'xField': 'lon', 'type': 'csv', 'xyDms': 'yes'}
        requests = None
        self.runTest(filename, requests, **params)

    def test_016_decimal_point(self):
        # Reading degrees/minutes/seconds angles
        filename = 'testdp.csv'
        params = {'yField': 'geom_y', 'xField': 'geom_x', 'type': 'csv', 'delimiter': ';', 'decimalPoint': ','}
        requests = None
        self.runTest(filename, requests, **params)

    def test_017_regular_expression_1(self):
        # Parsing regular expression delimiter
        filename = 'testre.txt'
        params = {'geomType': 'none', 'trimFields': 'Y', 'delimiter': 'RE(?:GEXP)?', 'type': 'regexp'}
        requests = None
        self.runTest(filename, requests, **params)

    def test_018_regular_expression_2(self):
        # Parsing regular expression delimiter with capture groups
        filename = 'testre.txt'
        params = {'geomType': 'none', 'trimFields': 'Y', 'delimiter': '(RE)((?:GEXP)?)', 'type': 'regexp'}
        requests = None
        self.runTest(filename, requests, **params)

    def test_019_regular_expression_3(self):
        # Parsing anchored regular expression
        filename = 'testre2.txt'
        params = {'geomType': 'none', 'trimFields': 'Y', 'delimiter': '^(.{5})(.{30})(.{5,})', 'type': 'regexp'}
        requests = None
        self.runTest(filename, requests, **params)

    def test_020_regular_expression_4(self):
        # Parsing zero length re
        filename = 'testre3.txt'
        params = {'geomType': 'none', 'delimiter': 'x?', 'type': 'regexp'}
        requests = None
        self.runTest(filename, requests, **params)

    def test_021_regular_expression_5(self):
        # Parsing zero length re 2
        filename = 'testre3.txt'
        params = {'geomType': 'none', 'delimiter': '\\b', 'type': 'regexp'}
        requests = None
        self.runTest(filename, requests, **params)

    def test_022_utf8_encoded_file(self):
        # UTF8 encoded file test
        filename = 'testutf8.csv'
        params = {'geomType': 'none', 'delimiter': '|', 'type': 'csv', 'encoding': 'utf-8'}
        requests = None
        self.runTest(filename, requests, **params)

    def test_023_latin1_encoded_file(self):
        # Latin1 encoded file test
        filename = 'testlatin1.csv'
        params = {'geomType': 'none', 'delimiter': '|', 'type': 'csv', 'encoding': 'latin1'}
        requests = None
        self.runTest(filename, requests, **params)

    def test_024_filter_rect_xy(self):
        # Filter extents on XY layer
        filename = 'testextpt.txt'
        params = {'yField': 'y', 'delimiter': '|', 'type': 'csv', 'xField': 'x'}
        requests = [
            {'extents': [10, 30, 30, 50]},
            {'extents': [10, 30, 30, 50], 'exact': 1},
            {'extents': [110, 130, 130, 150]}]
        self.runTest(filename, requests, **params)

    def test_025_filter_rect_wkt(self):
        # Filter extents on WKT layer
        filename = 'testextw.txt'
        params = {'delimiter': '|', 'type': 'csv', 'wktField': 'wkt'}
        requests = [
            {'extents': [10, 30, 30, 50]},
            {'extents': [10, 30, 30, 50], 'exact': 1},
            {'extents': [110, 130, 130, 150]}]
        self.runTest(filename, requests, **params)

    def test_026_filter_fid(self):
        # Filter on feature id
        filename = 'test.csv'
        params = {'geomType': 'none', 'type': 'csv'}
        requests = [
            {'fid': 3},
            {'fid': 9},
            {'fid': 20},
            {'fid': 3}]
        self.runTest(filename, requests, **params)

    def test_027_filter_attributes(self):
        # Filter on attributes
        filename = 'test.csv'
        params = {'geomType': 'none', 'type': 'csv'}
        requests = [
            {'attributes': [1, 3]},
            {'fid': 9},
            {'attributes': [1, 3], 'fid': 9},
            {'attributes': [3, 1], 'fid': 9},
            {'attributes': [1, 3, 7], 'fid': 9},
            {'attributes': [], 'fid': 9}]
        self.runTest(filename, requests, **params)

    def test_028_substring_test(self):
        # CSV file parsing
        filename = 'test.csv'
        params = {'geomType': 'none', 'subset': 'id % 2 = 1', 'type': 'csv'}
        requests = None
        self.runTest(filename, requests, **params)

    def test_029_file_watcher(self):
        # Testing file watcher
        (filehandle, filename) = tempfile.mkstemp()
        if os.name == "nt":
            filename = filename.replace("\\", "/")
        with os.fdopen(filehandle, "w") as f:
            f.write("id,name\n1,rabbit\n2,pooh\n")

        def appendfile(layer):
            with open(filename, 'a') as f:
                f.write('3,tiger\n')
            # print "Appended to file - sleeping"
            time.sleep(1)
            QCoreApplication.instance().processEvents()

        def rewritefile(layer):
            with open(filename, 'w') as f:
                f.write("name,size,id\ntoad,small,5\nmole,medium,6\nbadger,big,7\n")
            # print "Rewritten file - sleeping"
            time.sleep(1)
            QCoreApplication.instance().processEvents()

        def deletefile(layer):
            try:
                os.remove(filename)
            except:
                open(filename, "w").close()
                assert os.path.getsize(filename) == 0, "removal and truncation of {} failed".format(filename)
            # print "Deleted file - sleeping"
            time.sleep(1)
            QCoreApplication.instance().processEvents()

        params = {'geomType': 'none', 'type': 'csv', 'watchFile': 'yes'}
        requests = [
            {'fid': 3},
            {},
            {'fid': 7},
            appendfile,
            {'fid': 3},
            {'fid': 4},
            {},
            {'fid': 7},
            rewritefile,
            {'fid': 2},
            {},
            {'fid': 7},
            deletefile,
            {'fid': 2},
            {},
            rewritefile,
            {'fid': 2},
        ]
        self.runTest(filename, requests, **params)

    def test_030_filter_rect_xy_spatial_index(self):
        # Filter extents on XY layer with spatial index
        filename = 'testextpt.txt'
        params = {'yField': 'y', 'delimiter': '|', 'type': 'csv', 'xField': 'x', 'spatialIndex': 'Y'}
        requests = [
            {'extents': [10, 30, 30, 50]},
            {'extents': [10, 30, 30, 50], 'exact': 1},
            {'extents': [110, 130, 130, 150]},
            {},
            {'extents': [-1000, -1000, 1000, 1000]}
        ]
        self.runTest(filename, requests, **params)

    def test_031_filter_rect_wkt_spatial_index(self):
        # Filter extents on WKT layer with spatial index
        filename = 'testextw.txt'
        params = {'delimiter': '|', 'type': 'csv', 'wktField': 'wkt', 'spatialIndex': 'Y'}
        requests = [
            {'extents': [10, 30, 30, 50]},
            {'extents': [10, 30, 30, 50], 'exact': 1},
            {'extents': [110, 130, 130, 150]},
            {},
            {'extents': [-1000, -1000, 1000, 1000]}
        ]
        self.runTest(filename, requests, **params)

    def test_032_filter_rect_wkt_create_spatial_index(self):
        # Filter extents on WKT layer building spatial index
        filename = 'testextw.txt'
        params = {'delimiter': '|', 'type': 'csv', 'wktField': 'wkt'}
        requests = [
            {'extents': [10, 30, 30, 50]},
            {},
            lambda layer: layer.dataProvider().createSpatialIndex(),
            {'extents': [10, 30, 30, 50]},
            {'extents': [10, 30, 30, 50], 'exact': 1},
            {'extents': [110, 130, 130, 150]},
            {},
            {'extents': [-1000, -1000, 1000, 1000]}
        ]
        self.runTest(filename, requests, **params)

    def test_033_reset_subset_string(self):
        # CSV file parsing
        filename = 'test.csv'
        params = {'geomType': 'none', 'type': 'csv'}
        requests = [
            {},
            lambda layer: layer.dataProvider().setSubsetString("id % 2 = 1", True),
            {},
            lambda layer: layer.dataProvider().setSubsetString("id = 6", False),
            {},
            lambda layer: layer.dataProvider().setSubsetString("id = 3", False),
            {},
            lambda layer: layer.dataProvider().setSubsetString("id % 2 = 1", True),
            {},
            lambda layer: layer.dataProvider().setSubsetString("id % 2 = 0", True),
            {},
        ]
        self.runTest(filename, requests, **params)

    def test_034_csvt_file(self):
        # CSVT field types
        filename = 'testcsvt.csv'
        params = {'geomType': 'none', 'type': 'csv'}
        requests = None
        self.runTest(filename, requests, **params)

    def test_035_csvt_file2(self):
        # CSV field types 2
        filename = 'testcsvt2.txt'
        params = {'geomType': 'none', 'type': 'csv', 'delimiter': '|'}
        requests = None
        self.runTest(filename, requests, **params)

    def test_036_csvt_file_invalid_types(self):
        # CSV field types invalid string format
        filename = 'testcsvt3.csv'
        params = {'geomType': 'none', 'type': 'csv'}
        requests = None
        self.runTest(filename, requests, **params)

    def test_037_csvt_file_invalid_file(self):
        # CSV field types invalid file
        filename = 'testcsvt4.csv'
        params = {'geomType': 'none', 'type': 'csv'}
        requests = None
        self.runTest(filename, requests, **params)

    def test_038_type_inference(self):
        # Skip lines
        filename = 'testtypes.csv'
        params = {'yField': 'lat', 'xField': 'lon', 'type': 'csv'}
        requests = None
        self.runTest(filename, requests, **params)

    def test_039_issue_13749(self):
        # First record contains missing geometry
        filename = 'test13749.csv'
        params = {'yField': 'geom_y', 'xField': 'geom_x', 'type': 'csv'}
        requests = None
        self.runTest(filename, requests, **params)

    def test_040_issue_14666(self):
        # x/y containing some null geometries
        filename = 'test14666.csv'
        params = {'yField': 'y', 'xField': 'x', 'type': 'csv', 'delimiter': '\\t'}
        requests = None
        self.runTest(filename, requests, **params)

    def test_041_no_detect_type(self):
        # CSV file parsing
        # Skip lines
        filename = 'testtypes.csv'
        params = {'yField': 'lat', 'xField': 'lon', 'type': 'csv', 'detectTypes': 'no'}
        requests = None
        self.runTest(filename, requests, **params)

    def test_042_no_detect_types_csvt(self):
        # CSVT field types
        filename = 'testcsvt.csv'
        params = {'geomType': 'none', 'type': 'csv', 'detectTypes': 'no'}
        requests = None
        self.runTest(filename, requests, **params)

    def test_043_decodeuri(self):
        # URI decoding
        filename = '/home/to/path/test.csv'
        uri = 'file://{}?geomType=none'.format(filename)
        registry = QgsProviderRegistry.instance()
        components = registry.decodeUri('delimitedtext', uri)
        self.assertEqual(components['path'], filename)

    def test_044_ZM(self):
        # Create test layer
        srcpath = os.path.join(TEST_DATA_DIR, 'provider')
        basetestfile = os.path.join(srcpath, 'delimited_xyzm.csv')

        url = MyUrl.fromLocalFile(basetestfile)
        url.addQueryItem("crs", "epsg:4326")
        url.addQueryItem("type", "csv")
        url.addQueryItem("xField", "X")
        url.addQueryItem("yField", "Y")
        url.addQueryItem("zField", "Z")
        url.addQueryItem("mField", "M")
        url.addQueryItem("spatialIndex", "no")
        url.addQueryItem("subsetIndex", "no")
        url.addQueryItem("watchFile", "no")

        vl = QgsVectorLayer(url.toString(), 'test', 'delimitedtext')
        assert vl.isValid(), "{} is invalid".format(basetestfile)
        assert vl.wkbType() == QgsWkbTypes.PointZM, "wrong wkb type, should be PointZM"
        assert vl.getFeature(2).geometry().asWkt() == "PointZM (-71.12300000000000466 78.23000000000000398 1 2)", "wrong PointZM geometry"

    def test_045_Z(self):
        # Create test layer
        srcpath = os.path.join(TEST_DATA_DIR, 'provider')
        basetestfile = os.path.join(srcpath, 'delimited_xyzm.csv')

        url = MyUrl.fromLocalFile(basetestfile)
        url.addQueryItem("crs", "epsg:4326")
        url.addQueryItem("type", "csv")
        url.addQueryItem("xField", "X")
        url.addQueryItem("yField", "Y")
        url.addQueryItem("zField", "Z")
        url.addQueryItem("spatialIndex", "no")
        url.addQueryItem("subsetIndex", "no")
        url.addQueryItem("watchFile", "no")

        vl = QgsVectorLayer(url.toString(), 'test', 'delimitedtext')
        assert vl.isValid(), "{} is invalid".format(basetestfile)
        assert vl.wkbType() == QgsWkbTypes.PointZ, "wrong wkb type, should be PointZ"
        assert vl.getFeature(2).geometry().asWkt() == "PointZ (-71.12300000000000466 78.23000000000000398 1)", "wrong PointZ geometry"

    def test_046_M(self):
        # Create test layer
        srcpath = os.path.join(TEST_DATA_DIR, 'provider')
        basetestfile = os.path.join(srcpath, 'delimited_xyzm.csv')

        url = MyUrl.fromLocalFile(basetestfile)
        url.addQueryItem("crs", "epsg:4326")
        url.addQueryItem("type", "csv")
        url.addQueryItem("xField", "X")
        url.addQueryItem("yField", "Y")
        url.addQueryItem("mField", "M")
        url.addQueryItem("spatialIndex", "no")
        url.addQueryItem("subsetIndex", "no")
        url.addQueryItem("watchFile", "no")

        vl = QgsVectorLayer(url.toString(), 'test', 'delimitedtext')
        assert vl.isValid(), "{} is invalid".format(basetestfile)
        assert vl.wkbType() == QgsWkbTypes.PointM, "wrong wkb type, should be PointM"
        assert vl.getFeature(2).geometry().asWkt() == "PointM (-71.12300000000000466 78.23000000000000398 2)", "wrong PointM geometry"

    def test_047_datetime(self):
        # Create test layer
        srcpath = os.path.join(TEST_DATA_DIR, 'provider')
        basetestfile = os.path.join(srcpath, 'delimited_datetime.csv')

        url = MyUrl.fromLocalFile(basetestfile)
        url.addQueryItem("crs", "epsg:4326")
        url.addQueryItem("type", "csv")
        url.addQueryItem("xField", "X")
        url.addQueryItem("yField", "Y")
        url.addQueryItem("spatialIndex", "no")
        url.addQueryItem("subsetIndex", "no")
        url.addQueryItem("watchFile", "no")

        vl = QgsVectorLayer(url.toString(), 'test', 'delimitedtext')
        assert vl.isValid(), "{} is invalid".format(basetestfile)
        assert vl.fields().at(4).type() == QVariant.DateTime
        assert vl.fields().at(5).type() == QVariant.Date
        assert vl.fields().at(6).type() == QVariant.Time
        assert vl.fields().at(9).type() == QVariant.String

    def test_048_csvt_file(self):
        # CSVT field types non lowercase
        filename = 'testcsvt5.csv'
        params = {'geomType': 'none', 'type': 'csv'}
        requests = None
        self.runTest(filename, requests, **params)

    def testSpatialIndex(self):
        srcpath = os.path.join(TEST_DATA_DIR, 'provider')
        basetestfile = os.path.join(srcpath, 'delimited_xyzm.csv')

        url = MyUrl.fromLocalFile(basetestfile)
        url.addQueryItem("crs", "epsg:4326")
        url.addQueryItem("type", "csv")
        url.addQueryItem("xField", "X")
        url.addQueryItem("yField", "Y")
        url.addQueryItem("spatialIndex", "no")

        vl = QgsVectorLayer(url.toString(), 'test', 'delimitedtext')
        self.assertTrue(vl.isValid())

        self.assertEqual(vl.hasSpatialIndex(), QgsFeatureSource.SpatialIndexNotPresent)
        vl.dataProvider().createSpatialIndex()
        self.assertEqual(vl.hasSpatialIndex(), QgsFeatureSource.SpatialIndexPresent)

    def testEncodeDecodeUri(self):
        registry = QgsProviderRegistry.instance()

        # URI decoding
        filename = '/home/to/path/test.csv'
        parts = {'path': filename}
        uri = registry.encodeUri('delimitedtext', parts)
        self.assertEqual(uri, 'file://' + filename)

        # URI encoding / decoding with unicode characters
        filename = '/höme/to/path/pöints.txt'
        parts = {'path': filename}
        uri = registry.encodeUri('delimitedtext', parts)
        self.assertEqual(uri, 'file:///h%C3%B6me/to/path/p%C3%B6ints.txt')
        parts = registry.decodeUri('delimitedtext', uri)
        self.assertEqual(parts['path'], filename)

    def testCREndOfLineAndWorkingBuffer(self):
        # Test CSV file with \r (CR) endings
        # Test also that the logic to refill the buffer works properly
        os.environ['QGIS_DELIMITED_TEXT_FILE_BUFFER_SIZE'] = '17'
        try:
            basetestfile = os.path.join(unitTestDataPath("delimitedtext"), 'test_cr_end_of_line.csv')

            url = MyUrl.fromLocalFile(basetestfile)
            url.addQueryItem("type", "csv")
            url.addQueryItem("geomType", "none")

            vl = QgsVectorLayer(url.toString(), 'test', 'delimitedtext')
            assert vl.isValid(), "{} is invalid".format(basetestfile)

            fields = vl.fields()
            self.assertEqual(len(fields), 2)
            self.assertEqual(fields[0].name(), 'col0')
            self.assertEqual(fields[1].name(), 'col1')

            features = [f for f in vl.getFeatures()]
            self.assertEqual(len(features), 2)
            self.assertEqual(features[0]['col0'], 'value00')
            self.assertEqual(features[0]['col1'], 'value01')
            self.assertEqual(features[1]['col0'], 'value10')
            self.assertEqual(features[1]['col1'], 'value11')

        finally:
            del os.environ['QGIS_DELIMITED_TEXT_FILE_BUFFER_SIZE']

    def testSaturationOfWorkingBuffer(self):
        # 10 bytes is sufficient to detect the header line, but not enough for the
        # first record
        os.environ['QGIS_DELIMITED_TEXT_FILE_BUFFER_SIZE'] = '10'
        try:
            basetestfile = os.path.join(unitTestDataPath("delimitedtext"), 'test_cr_end_of_line.csv')

            url = MyUrl.fromLocalFile(basetestfile)
            url.addQueryItem("type", "csv")
            url.addQueryItem("geomType", "none")

            vl = QgsVectorLayer(url.toString(), 'test', 'delimitedtext')
            assert vl.isValid(), "{} is invalid".format(basetestfile)

            fields = vl.fields()
            self.assertEqual(len(fields), 2)
            self.assertEqual(fields[0].name(), 'col0')
            self.assertEqual(fields[1].name(), 'col1')

            features = [f for f in vl.getFeatures()]
            self.assertEqual(len(features), 1)
            self.assertEqual(features[0]['col0'], 'value00')
            self.assertEqual(features[0]['col1'], 'va')  # truncated

        finally:
            del os.environ['QGIS_DELIMITED_TEXT_FILE_BUFFER_SIZE']

    def _make_test_file(self, csv_content, csvt_content='', uri_options=''):

        TestQgsDelimitedTextProviderOther._text_index += 1

        basename = 'test_type_detection_{}'.format(self._text_index)

        csv_file = os.path.join(self.tmp_path, basename + '.csv')
        with open(csv_file, 'w+') as f:
            f.write(csv_content)

        if csvt_content:
            csvt_file = os.path.join(self.tmp_path, basename + '.csvt')
            with open(csvt_file, 'w+') as f:
                f.write(csvt_content)

        uri = 'file:///{}'.format(csv_file)
        if uri_options:
            uri += '?{}'.format(uri_options)

        vl = QgsVectorLayer(uri, 'test_{}'.format(basename), 'delimitedtext')
        return vl

    def test_type_detection_csvt(self):
        """Type detection from CSVT"""

        vl = self._make_test_file("f1,f2,f3,f4,f5\n1,1,1,\"1\",3\n", "Integer,Longlong,Real,String,Real\n")
        self.assertTrue(vl.isValid())
        fields = {f.name(): (f.type(), f.typeName()) for f in vl.fields()}
        self.assertEqual(fields, {
            'f1': (QVariant.Int, 'integer'),
            'f2': (QVariant.LongLong, 'longlong'),
            'f3': (QVariant.Double, 'double'),
            'f4': (QVariant.String, 'text'),
            'f5': (QVariant.Double, 'double')})

        # Missing last field in CSVT
        vl = self._make_test_file("f1,f2,f3,f4,f5\n1,1,1,\"1\",3\n", "Integer,Long,Real,String\n")
        self.assertTrue(vl.isValid())
        fields = {f.name(): (f.type(), f.typeName()) for f in vl.fields()}
        self.assertEqual(fields, {
            'f1': (QVariant.Int, 'integer'),
            'f2': (QVariant.LongLong, 'longlong'),
            'f3': (QVariant.Double, 'double'),
            'f4': (QVariant.String, 'text'),
            'f5': (QVariant.Int, 'integer')})

        # No CSVT and detectTypes=no
        vl = self._make_test_file("f1,f2,f3,f4,f5\n1,1,1,\"1\",3\n", uri_options='detectTypes=no')
        self.assertTrue(vl.isValid())
        fields = {f.name(): (f.type(), f.typeName()) for f in vl.fields()}
        self.assertEqual(fields, {
            'f1': (QVariant.String, 'text'),
            'f2': (QVariant.String, 'text'),
            'f3': (QVariant.String, 'text'),
            'f4': (QVariant.String, 'text'),
            'f5': (QVariant.String, 'text')})

        # Test OGR generated CSVT, exported from QGIS
        vl = self._make_test_file('\n'.join((
            "fid,freal,ftext,fint,flong,fdate,fbool",
            '"1",1.234567,a text,"2000000000","4000000000",2021/11/12,true',
            '"2",3.4567889,another text,"2147483646","4000000000",2021/11/12,false',
            '"3",6.789,text,"2000000000","4000000000",2021/11/13,false',
        )), "Integer64(20),Real,String,Integer(10),Integer64(20),Date,String")
        self.assertTrue(vl.isValid())
        fields = {f.name(): (f.type(), f.typeName()) for f in vl.fields()}
        self.assertEqual(fields, {
            'fid': (4, 'longlong'),
            'freal': (6, 'double'),
            'ftext': (10, 'text'),
            'fint': (2, 'integer'),
            'flong': (4, 'longlong'),
            'fdate': (14, 'date'),
            'fbool': (10, 'text')})

        attrs = [f.attributes() for f in vl.getFeatures()]
        self.assertEqual(attrs, [
            [1,
             1.234567,
             'a text',
             2000000000,
             4000000000,
             QDate(2021, 11, 12),
             'true'],
            [2,
             3.4567889,
             'another text',
             2147483646,
             4000000000,
             QDate(2021, 11, 12),
             'false'],
            [3,
             6.789,
             'text',
             2000000000,
             4000000000,
             QDate(2021, 11, 13),
             'false']
        ])

        # Try bool Integer(Boolean)
        vl = self._make_test_file('\n'.join((
            "fid,freal,ftext,fint,flong,fdate,fbool",
            '"1",1.234567,a text,"2000000000","4000000000",2021/11/12,true',
            '"2",3.4567889,another text,"2147483646","4000000000",2021/11/12,false',
            '"3",6.789,text,"2000000000","4000000000",2021/11/13,false',
        )), "Integer64(20),Real,String,Integer(10),Integer64(20),Date,Integer(Boolean)")

        self.assertTrue(vl.isValid())
        fields = {f.name(): (f.type(), f.typeName()) for f in vl.fields()}
        self.assertEqual(fields, {
            'fid': (4, 'longlong'),
            'freal': (6, 'double'),
            'ftext': (10, 'text'),
            'fint': (2, 'integer'),
            'flong': (4, 'longlong'),
            'fdate': (14, 'date'),
            'fbool': (1, 'bool')})

        attrs = [f.attributes() for f in vl.getFeatures()]
        self.assertEqual(attrs, [
            [1,
             1.234567,
             'a text',
             2000000000,
             4000000000,
             QDate(2021, 11, 12),
             True],
            [2,
             3.4567889,
             'another text',
             2147483646,
             4000000000,
             QDate(2021, 11, 12),
             False],
            [3,
             6.789,
             'text',
             2000000000,
             4000000000,
             QDate(2021, 11, 13),
             False]
        ])

        # XY no args
        vl = self._make_test_file('\n'.join((
            "X,Y,fid,freal,ftext,fint,flong,fdate,fbool",
            '-106.13127068692,36.0554327720544,"4",1.234567,a text,"2000000000","4000000000",2021/11/12,true',
            '-105.781333374658,35.7216962612865,"5",3.4567889,another text,"2147483646","4000000000",2021/11/12,false',
            '-106.108589564828,35.407400712311,"6",6.789,text,"2000000000","4000000000",2021/11/13,false',
        )), "CoordX,CoordY,Integer64(20),Real,String,Integer(10),Integer64(20),Date,Integer(Boolean)")

        self.assertTrue(vl.isValid())
        fields = {f.name(): (f.type(), f.typeName()) for f in vl.fields()}
        self.assertEqual(fields, {
            'X': (6, 'double'),
            'Y': (6, 'double'),
            'fid': (4, 'longlong'),
            'freal': (6, 'double'),
            'ftext': (10, 'text'),
            'fint': (2, 'integer'),
            'flong': (4, 'longlong'),
            'fdate': (14, 'date'),
            'fbool': (1, 'bool')})

        attrs = [f.attributes() for f in vl.getFeatures()]
        self.assertEqual(attrs, [
            [-106.13127068692,
             36.0554327720544,
             4,
             1.234567,
             'a text',
             2000000000,
             4000000000,
             QDate(2021, 11, 12),
             True],
            [-105.781333374658,
             35.7216962612865,
             5,
             3.4567889,
             'another text',
             2147483646,
             4000000000,
             QDate(2021, 11, 12),
             False],
            [-106.108589564828,
             35.407400712311,
             6,
             6.789,
             'text',
             2000000000,
             4000000000,
             QDate(2021, 11, 13),
             False]])

        vl = self._make_test_file('\n'.join((
            "X,Y,fid,freal,ftext,fint,flong,fdate,fbool",
            '-106.13127068692,36.0554327720544,"1",1.234567,a text,"2000000000","4000000000",2021/11/12,true',
            '-105.781333374658,35.7216962612865,"2",3.4567889,another text,"2147483646","4000000000",2021/11/12,false',
            '-106.108589564828,35.407400712311,"3",6.789,text,"2000000000","4000000000",2021/11/13,false',
        )), "CoordX,CoordY,Integer64(20),Real,String,Integer(10),Integer64(20),Date,Integer(Boolean)", uri_options='xField=X&yField=Y')
        self.assertTrue(vl.isSpatial())

        # Test Z
        vl = self._make_test_file('\n'.join((
            "X,Y,Z,fid,freal,ftext,fint,flong,fdate,fbool",
            '-106.13127068692,36.0554327720544,1,"1",1.234567,a text,"2000000000","4000000000",2021/11/12,true',
            '-105.781333374658,35.7216962612865,123,"2",3.4567889,another text,"2147483646","4000000000",2021/11/12,false',
            '-106.108589564828,35.407400712311,"456","3",6.789,text,"2000000000","4000000000",2021/11/13,false',
        )), "Point(x),CoordY,Point(z),Integer64(20),Real(float32),String,Integer(10),Integer64(20),Date,Integer(Boolean)", uri_options='xField=X&yField=Y&zField=Z')

        fields = {f.name(): (f.type(), f.typeName()) for f in vl.fields()}
        self.assertEqual(fields, {
            'X': (6, 'double'),
            'Y': (6, 'double'),
            'Z': (6, 'double'),
            'fid': (4, 'longlong'),
            'freal': (6, 'double'),
            'ftext': (10, 'text'),
            'fint': (2, 'integer'),
            'flong': (4, 'longlong'),
            'fdate': (14, 'date'),
            'fbool': (1, 'bool')})

        geometries = [f.geometry() for f in vl.getFeatures()]
        self.assertGeometriesEqual(geometries[-1], QgsGeometry.fromWkt('PointZ (-106.10858956482799442 35.40740071231100217 456)'))

    def test_booleans(self):
        """Test bool detection with user defined literals"""

        vl = self._make_test_file('\n'.join((
            "id,bool_true_false,bool_0_1,bool_t_f,invalid_bool",
            "2,true,1,t,nope",
            "3,false,0,f,dope",
            "4,TRUE,1,T,NOPE",
        )))

        fields = {f.name(): (f.type(), f.typeName()) for f in vl.fields()}
        self.assertEqual(fields, {
            'id': (2, 'integer'),
            'bool_true_false': (QVariant.Bool, 'bool'),
            'bool_0_1': (QVariant.Bool, 'bool'),
            'bool_t_f': (QVariant.Bool, 'bool'),
            'invalid_bool': (QVariant.String, 'text')},
        )

        attrs = [f.attributes() for f in vl.getFeatures()]
        self.assertEqual(attrs, [
            [2, True, True, True, 'nope'],
            [3, False, False, False, 'dope'],
            [4, True, True, True, 'NOPE'],
        ])

        vl = self._make_test_file('\n'.join((
            "id,bool_true_false,bool_0_1,bool_t_f,invalid_bool",
            "2,true,1,t,nope",
            "3,false,0,f,dope",
            "4,TRUE,1,T,NOPE",
        )), uri_options='booleanTrue=DOPE&booleanFalse=NoPe')

        fields = {f.name(): (f.type(), f.typeName()) for f in vl.fields()}
        self.assertEqual(fields, {
            'id': (2, 'integer'),
            'bool_true_false': (QVariant.Bool, 'bool'),
            'bool_0_1': (QVariant.Bool, 'bool'),
            'bool_t_f': (QVariant.Bool, 'bool'),
            'invalid_bool': (QVariant.Bool, 'bool')},
        )

        attrs = [f.attributes() for f in vl.getFeatures()]
        self.assertEqual(attrs, [
            [2, True, True, True, False],
            [3, False, False, False, True],
            [4, True, True, True, False],
        ])

        vl = self._make_test_file('\n'.join((
            "id,bool_true_false,bool_0_1,bool_t_f,invalid_bool",
            "2,true,1,t,nope",
            "3,false,0,f,dope",
            "4,TRUE,1,T,",
        )), uri_options='booleanTrue=DOPE&booleanFalse=NoPe')

        attrs = [f.attributes() for f in vl.getFeatures()]
        self.assertEqual(attrs, [
            [2, True, True, True, False],
            [3, False, False, False, True],
            [4, True, True, True, NULL],
        ])

    def test_type_override(self):
        """Test type overrides"""

        vl = self._make_test_file('\n'.join((
            "integer,bool,long,real,text",
            "1,0,9189304972279762602,1.234,text",
            "2,1,,5.678,another text",
        )))
        self.assertTrue(vl.isValid())
        fields = {f.name(): (f.type(), f.typeName()) for f in vl.fields()}
        self.assertEqual(fields, {
            'integer': (QVariant.Int, 'integer'),
            'bool': (QVariant.Bool, 'bool'),
            'long': (QVariant.LongLong, 'longlong'),
            'real': (QVariant.Double, 'double'),
            'text': (QVariant.String, 'text')})

        attrs = [f.attributes() for f in vl.getFeatures()]
        self.assertEqual(attrs, [[1, False, 9189304972279762602, 1.234, 'text'],
                                 [2, True, NULL, 5.678, 'another text']])

        vl = self._make_test_file('\n'.join((
            "integer,bool,long,real,text",
            "1,0,9189304972279762602,1.234,text",
            "2,1,,5.678,another text",
        )), uri_options='field=bool:integer&field=integer:double&field=long:double&field=real:text')
        self.assertTrue(vl.isValid())
        fields = {f.name(): (f.type(), f.typeName()) for f in vl.fields()}
        self.assertEqual(fields, {
            'integer': (QVariant.Double, 'double'),
            'bool': (QVariant.Int, 'integer'),
            'long': (QVariant.Double, 'double'),
            'real': (QVariant.String, 'text'),
            'text': (QVariant.String, 'text')})

        attrs = [f.attributes() for f in vl.getFeatures()]
        self.assertEqual(attrs, [[1.0, 0, 9.189304972279763e+18, '1.234', 'text'],
                                 [2.0, 1, NULL, '5.678', 'another text']])

    def test_regression_gh46749(self):
        """Test regression GH #46749"""

        vl = self._make_test_file('\n'.join((
            "integer,wkt,bool",
            "1,POINT(0 0),1",
            "2,POINT(1 1),0",
            "3,POINT(2 2),1",
        )), uri_options='geomType=Point&crs=EPSG:4326&wktField=wkt')

        self.assertTrue(vl.isValid())
        fields = {f.name(): (f.type(), f.typeName()) for f in vl.fields()}
        self.assertEqual(fields, {
            'integer': (QVariant.Int, 'integer'),
            'bool': (QVariant.Bool, 'bool'),
        })

        # This was crashing!
        features = [f for f in vl.getFeatures()]


if __name__ == '__main__':
    unittest.main()
