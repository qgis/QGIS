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
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

# This module provides unit test for the delimtied text provider.  It uses data files in
# the testdata/delimitedtext directory.
#
# New tests can be created (or existing ones updated), but incorporating a createTest
# call into the test.  This will load the file and generate a test that the features
# loaded from it are correct.  It assumes that the data is correct at the time the
# test is created.  The new test is written to the test output file, and can be edited into
# this module to implement the test.
#
# To recreate all tests, set rebuildTests to true

import os
import os.path
import re
import tempfile
import inspect
import time
import test_qgsdelimitedtextprovider_wanted as want

rebuildTests = 'REBUILD_DELIMITED_TEXT_TESTS' in os.environ;

import qgis

from PyQt4.QtCore import (QVariant,
                          QCoreApplication,
                        QUrl,
                        QObject
                        )

from qgis.core import (QGis,
                        QgsProviderRegistry,
                        QgsVectorLayer,
                        QgsFeature,
                        QgsFeatureRequest,
                        QgsField,
                        QgsGeometry,
                        QgsRectangle,
                        QgsPoint,
                        QgsMessageLog)

from utilities import (getQgisTestApp,
                       TestCase,
                       unitTestDataPath,
                       unittest,
                       compareWkt,
                       #expectedFailure
                       )


QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()

import sip
sipversion=str(sip.getapi('QVariant'))
sipwanted='2'
geomkey = "#geometry"
fidkey = "#fid"

# Thought we could connect to messageReceived signal but doesn't seem to be available
# in python :-(  Not sure why?

class MessageLogger( QObject ):

    def __init__( self, tag=None ):
        QObject.__init__(self)
        self.log=[];
        self.tag = tag

    def __enter__( self ):
        QgsMessageLog.instance().messageReceived.connect( self.logMessage )
        return self

    def __exit__( self, type, value, traceback ):
        QgsMessageLog.instance().messageReceived.disconnect( self.logMessage )

    def logMessage( self, msg, tag, level ):
        if tag == self.tag  or not self.tag:
            self.log.append(unicode(msg))

    def messages( self ):
        return self.log

# Retrieve the data for a layer

def layerData( layer, request={}, offset=0 ):
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
            fr.setFilterFid( request['fid'] )
        elif 'extents' in request:
            fr.setFilterRect(QgsRectangle( *request['extents'] ))
        if 'attributes' in request:
            fr.setSubsetOfAttributes( request['attributes'] )

    for f in layer.getFeatures(fr):
        if first:
            first = False
            for field in f.fields():
                fields.append(str(field.name()))
                fieldTypes.append(str(field.typeName()))
        fielddata = dict ( (name, unicode(f[name]) ) for name in fields )
        g = f.geometry()
        if g:
            fielddata[geomkey] = str(g.exportToWkt());
        else:
            fielddata[geomkey] = "None";

        fielddata[fidkey] = f.id()
        id = fielddata[fields[0]]
        description = fielddata[fields[1]]
        fielddata['id']=id
        fielddata['description']=description
        data[f.id()+offset]=fielddata
    if 'id' not in fields: fields.insert(0,'id')
    if 'description' not in fields: fields.insert(1,'description')
    fields.append(fidkey)
    fields.append(geomkey)
    return fields, fieldTypes, data

# Retrieve the data for a delimited text url

def delimitedTextData( testname, filename, requests, verbose, **params ):
    # Create a layer for the specified file and query parameters
    # and return the data for the layer (fields, data)

    filepath = os.path.join(unitTestDataPath("delimitedtext"),filename);
    url = QUrl.fromLocalFile(filepath);
    if not requests:
        requests=[{}]
    for k in params.keys():
        url.addQueryItem(k,params[k])
    urlstr = url.toString()
    log=[]
    with MessageLogger('DelimitedText') as logger:
        if verbose:
            print testname
        layer = QgsVectorLayer(urlstr,'test','delimitedtext')
        uri = unicode(layer.dataProvider().dataSourceUri())
        if verbose:
            print uri
        basename = os.path.basename(filepath)
        if not basename.startswith('test'):
            basename='file'
        uri = re.sub(r'^file\:\/\/[^\?]*','file://'+basename,uri)
        fields = []
        fieldTypes = []
        data = {}
        if layer.isValid():
            for nr,r in enumerate(requests):
                if verbose: print "Processing request",nr+1,repr(r)
                if callable(r):
                    r( layer )
                    if verbose: print "Request function executed"
                if callable(r):
                    continue
                rfields,rtypes, rdata = layerData(layer,r,nr*1000)
                if len(rfields) > len(fields):
                    fields = rfields
                    fieldTypes=rtypes
                data.update(rdata)
                if not rdata:
                    log.append("Request "+str(nr)+" did not return any data")
                if verbose:
                    print "Request returned",len(rdata.keys()),"features"
        for msg in logger.messages():
            filelogname = 'temp_file' if 'tmp' in filename.lower() else filename
            msg = re.sub(r'file\s+.*'+re.escape(filename),'file '+filelogname,msg)
            msg = msg.replace(filepath,filelogname)
            log.append(msg);
        return dict( fields=fields, fieldTypes=fieldTypes, data=data, log=log, uri=uri)

def printWanted( testname, result ):
    # Routine to export the result as a function definition
    print
    print "def {0}():".format(testname)
    data=result['data']
    log=result['log']
    fields=result['fields']
    prefix='    '

    # Dump the data for a layer - used to construct unit tests
    print prefix+"wanted={}"
    print prefix+"wanted['uri']="+repr(result['uri'])
    print prefix+"wanted['fieldTypes']="+repr(result['fieldTypes'])
    print prefix+"wanted['data']={"
    for k in sorted(data.keys()):
        row = data[k]
        print prefix+"    {0}: {{".format(repr(k))
        for f in fields:
            print prefix+"        "+repr(f)+": "+repr(row[f])+","
        print prefix+"        },";
    print prefix+"    }";

    print prefix+"wanted['log']=["
    for msg in log:
        print prefix+'    '+repr(msg)+','
    print prefix+'    ]'
    print '    return wanted'
    print


def recordDifference( record1, record2 ):
    # Compare a record defined as a dictionary
    for k in record1.keys():
        if k not in record2:
            return "Field {0} is missing".format(k)
        r1k = record1[k]
        r2k = record2[k]
        if k == geomkey:
            if not compareWkt(r1k,r2k):
                return "Geometry differs: {0:.50} versus {1:.50}".format(r1k,r2k)
        else:
            if record1[k] != record2[k]:
                return "Field {0} differs: {1:.50} versus {2:.50}".format(k,repr(r1k),repr(r2k));
    for k in record2.keys():
        if k not in record1:
            return "Output contains extra field {0} is missing".format(k)
    return ''

def runTest( file, requests, **params ):
    # No point doing test if haven't got the right SIP vesion
    if sipversion != sipwanted:
        return
    testname=inspect.stack()[1][3];
    verbose = not rebuildTests
    if verbose:
        print "Running test:",testname
    result = delimitedTextData( testname, file, requests, verbose, **params )
    if rebuildTests:
        printWanted( testname, result )
        assert False,"Test not run - being rebuilt"
    try:
        wanted = eval('want.{0}()'.format(testname))
    except:
        printWanted( testname, result )
        assert False,"Test results not available for {0}".format(testname)

    data = result['data']
    log = result['log']
    failures = []
    if result['uri'] != wanted['uri']:
        msg = "Layer Uri ({0}) doesn't match expected ({1})".format(
            result['uri'],wanted['uri'])
        print '    '+msg
        failures.append(msg)
    if result['fieldTypes'] != wanted['fieldTypes']:
        msg = "Layer field types ({0}) doesn't match expected ({1})".format(
            result['fieldTypes'],wanted['fieldTypes'])
        failures.append(msg)
    wanted_data = wanted['data']
    for id in sorted(wanted_data.keys()):
        wrec = wanted_data[id]
        trec = data.get(id,{})
        description = wrec['description']
        difference = recordDifference(wrec,trec)
        if not difference:
            print '    {0}: Passed'.format(description)
        else:
            print '    {0}: {1}'.format(description,difference)
            failures.append(description+': '+difference)
    for id in sorted(data.keys()):
        if id not in wanted_data:
            msg= "Layer contains unexpected extra data with id: \"{0}\"".format(id)
            print '    '+msg
            failures.append(msg)
    common=[]
    log_wanted = wanted['log']
    for l in log:
        if l in log_wanted:
            common.append(l)
    for l in log_wanted:
        if l not in common:
            msg='Missing log message: '+l
            print '    '+msg
            failures.append(msg)
    for l in log:
        if l not in common:
            msg='Extra log message: '+l
            print '    '+msg
            failures.append(msg)
    if len(log)==len(common) and len(log_wanted)==len(common):
        print '    Message log correct: Passed'

    if failures:
        printWanted( testname, result )

    assert len(failures) == 0,"\n".join(failures)

class TestQgsDelimitedTextProvider(TestCase):

    def test_001_provider_defined( self ):
        registry=QgsProviderRegistry.instance()
        metadata = registry.providerMetadata('delimitedtext')
        assert metadata != None, "Delimited text provider is not installed"
        assert sipversion==sipwanted,"SIP version "+sipversion+" -  require version "+sipwanted+" for delimited text tests"

    def test_002_load_csv_file(self):
        # CSV file parsing
        filename='test.csv'
        params={'geomType': 'none', 'type': 'csv'}
        requests=None
        runTest(filename,requests,**params)

    def test_003_field_naming(self):
        # Management of missing/duplicate/invalid field names
        filename='testfields.csv'
        params={'geomType': 'none', 'type': 'csv'}
        requests=None
        runTest(filename,requests,**params)

    def test_004_max_fields(self):
        # Limiting maximum number of fields
        filename='testfields.csv'
        params={'geomType': 'none', 'maxFields': '7', 'type': 'csv'}
        requests=None
        runTest(filename,requests,**params)

    def test_005_load_whitespace(self):
        # Whitespace file parsing
        filename='test.space'
        params={'geomType': 'none', 'type': 'whitespace'}
        requests=None
        runTest(filename,requests,**params)

    def test_006_quote_escape(self):
        # Quote and escape file parsing
        filename='test.pipe'
        params={'geomType': 'none', 'quote': '"', 'delimiter': '|', 'escape': '\\'}
        requests=None
        runTest(filename,requests,**params)

    def test_007_multiple_quote(self):
        # Multiple quote and escape characters
        filename='test.quote'
        params={'geomType': 'none', 'quote': '\'"', 'type': 'csv', 'escape': '"\''}
        requests=None
        runTest(filename,requests,**params)

    def test_008_badly_formed_quotes(self):
        # Badly formed quoted fields
        filename='test.badquote'
        params={'geomType': 'none', 'quote': '"', 'type': 'csv', 'escape': '"'}
        requests=None
        runTest(filename,requests,**params)

    def test_009_skip_lines(self):
        # Skip lines
        filename='test2.csv'
        params={'geomType': 'none', 'useHeader': 'no', 'type': 'csv', 'skipLines': '2'}
        requests=None
        runTest(filename,requests,**params)

    def test_010_read_coordinates(self):
        # Skip lines
        filename='testpt.csv'
        params={'yField': 'geom_y', 'xField': 'geom_x', 'type': 'csv'}
        requests=None
        runTest(filename,requests,**params)

    def test_011_read_wkt(self):
        # Reading WKT geometry field
        filename='testwkt.csv'
        params={'delimiter': '|', 'type': 'csv', 'wktField': 'geom_wkt'}
        requests=None
        runTest(filename,requests,**params)

    def test_012_read_wkt_point(self):
        # Read WKT points
        filename='testwkt.csv'
        params={'geomType': 'point', 'delimiter': '|', 'type': 'csv', 'wktField': 'geom_wkt'}
        requests=None
        runTest(filename,requests,**params)

    def test_013_read_wkt_line(self):
        # Read WKT linestrings
        filename='testwkt.csv'
        params={'geomType': 'line', 'delimiter': '|', 'type': 'csv', 'wktField': 'geom_wkt'}
        requests=None
        runTest(filename,requests,**params)

    def test_014_read_wkt_polygon(self):
        # Read WKT polygons
        filename='testwkt.csv'
        params={'geomType': 'polygon', 'delimiter': '|', 'type': 'csv', 'wktField': 'geom_wkt'}
        requests=None
        runTest(filename,requests,**params)

    def test_015_read_dms_xy(self):
        # Reading degrees/minutes/seconds angles
        filename='testdms.csv'
        params={'yField': 'lat', 'xField': 'lon', 'type': 'csv', 'xyDms': 'yes'}
        requests=None
        runTest(filename,requests,**params)

    def test_016_decimal_point(self):
        # Reading degrees/minutes/seconds angles
        filename='testdp.csv'
        params={'yField': 'geom_y', 'xField': 'geom_x', 'type': 'csv', 'delimiter': ';', 'decimalPoint': ','}
        requests=None
        runTest(filename,requests,**params)

    def test_017_regular_expression_1(self):
        # Parsing regular expression delimiter
        filename='testre.txt'
        params={'geomType': 'none', 'trimFields': 'Y', 'delimiter': 'RE(?:GEXP)?', 'type': 'regexp'}
        requests=None
        runTest(filename,requests,**params)

    def test_018_regular_expression_2(self):
        # Parsing regular expression delimiter with capture groups
        filename='testre.txt'
        params={'geomType': 'none', 'trimFields': 'Y', 'delimiter': '(RE)(GEXP)?', 'type': 'regexp'}
        requests=None
        runTest(filename,requests,**params)

    def test_019_regular_expression_3(self):
        # Parsing anchored regular expression
        filename='testre2.txt'
        params={'geomType': 'none', 'trimFields': 'Y', 'delimiter': '^(.{5})(.{30})(.{5,})', 'type': 'regexp'}
        requests=None
        runTest(filename,requests,**params)

    def test_020_regular_expression_4(self):
        # Parsing zero length re
        filename='testre3.txt'
        params={'geomType': 'none', 'delimiter': 'x?', 'type': 'regexp'}
        requests=None
        runTest(filename,requests,**params)

    def test_021_regular_expression_5(self):
        # Parsing zero length re 2
        filename='testre3.txt'
        params={'geomType': 'none', 'delimiter': '\\b', 'type': 'regexp'}
        requests=None
        runTest(filename,requests,**params)

    def test_022_utf8_encoded_file(self):
        # UTF8 encoded file test
        filename='testutf8.csv'
        params={'geomType': 'none', 'delimiter': '|', 'type': 'csv', 'encoding': 'utf-8'}
        requests=None
        runTest(filename,requests,**params)

    def test_023_latin1_encoded_file(self):
        # Latin1 encoded file test
        filename='testlatin1.csv'
        params={'geomType': 'none', 'delimiter': '|', 'type': 'csv', 'encoding': 'latin1'}
        requests=None
        runTest(filename,requests,**params)

    def test_024_filter_rect_xy(self):
        # Filter extents on XY layer
        filename='testextpt.txt'
        params={'yField': 'y', 'delimiter': '|', 'type': 'csv', 'xField': 'x'}
        requests=[
            {'extents': [10, 30, 30, 50]},
            {'extents': [10, 30, 30, 50], 'exact': 1},
            {'extents': [110, 130, 130, 150]}]
        runTest(filename,requests,**params)

    def test_025_filter_rect_wkt(self):
        # Filter extents on WKT layer
        filename='testextw.txt'
        params={'delimiter': '|', 'type': 'csv', 'wktField': 'wkt'}
        requests=[
            {'extents': [10, 30, 30, 50]},
            {'extents': [10, 30, 30, 50], 'exact': 1},
            {'extents': [110, 130, 130, 150]}]
        runTest(filename,requests,**params)

    def test_026_filter_fid(self):
        # Filter on feature id
        filename='test.csv'
        params={'geomType': 'none', 'type': 'csv'}
        requests=[
            {'fid': 3},
            {'fid': 9},
            {'fid': 20},
            {'fid': 3}]
        runTest(filename,requests,**params)

    def test_027_filter_attributes(self):
        # Filter on attributes
        filename='test.csv'
        params={'geomType': 'none', 'type': 'csv'}
        requests=[
            {'attributes': [1, 3]},
            {'fid': 9},
            {'attributes': [1, 3], 'fid': 9},
            {'attributes': [3, 1], 'fid': 9},
            {'attributes': [1, 3, 7], 'fid': 9},
            {'attributes': [], 'fid': 9}]
        runTest(filename,requests,**params)

    def test_028_substring_test(self):
        # CSV file parsing
        filename='test.csv'
        params={'geomType': 'none', 'subset': 'id % 2 = 1', 'type': 'csv' }
        requests=None
        runTest(filename,requests,**params)

    def test_029_file_watcher(self):
        # Testing file watcher
        (filehandle,filename) = tempfile.mkstemp()
        with os.fdopen(filehandle,"w") as f:
            f.write("id,name\n1,rabbit\n2,pooh\n")
        def appendfile( layer ):
            with file(filename,'a') as f:
                f.write('3,tigger\n')
            # print "Appended to file - sleeping"
            time.sleep(1);
            QCoreApplication.instance().processEvents()
        def rewritefile( layer ):
            with file(filename,'w') as f:
                f.write("name,size,id\ntoad,small,5\nmole,medium,6\nbadger,big,7\n")
            # print "Rewritten file - sleeping"
            time.sleep(1);
            QCoreApplication.instance().processEvents()
        def deletefile( layer ):
            os.remove(filename)
            # print "Deleted file - sleeping"
            time.sleep(1);
            QCoreApplication.instance().processEvents()
        params={'geomType': 'none', 'type': 'csv', 'watchFile' : 'yes' }
        requests=[
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
        runTest(filename,requests,**params)

    def test_030_filter_rect_xy_spatial_index(self):
        # Filter extents on XY layer with spatial index
        filename='testextpt.txt'
        params={'yField': 'y', 'delimiter': '|', 'type': 'csv', 'xField': 'x', 'spatialIndex': 'Y' }
        requests=[
            {'extents': [10, 30, 30, 50]},
            {'extents': [10, 30, 30, 50], 'exact': 1},
            {'extents': [110, 130, 130, 150]},
            {},
            {'extents': [-1000, -1000, 1000, 1000]}
        ]
        runTest(filename,requests,**params)

    def test_031_filter_rect_wkt_spatial_index(self):
        # Filter extents on WKT layer with spatial index
        filename='testextw.txt'
        params={'delimiter': '|', 'type': 'csv', 'wktField': 'wkt', 'spatialIndex': 'Y' }
        requests=[
            {'extents': [10, 30, 30, 50]},
            {'extents': [10, 30, 30, 50], 'exact': 1},
            {'extents': [110, 130, 130, 150]},
            {},
            {'extents': [-1000, -1000, 1000, 1000]}
        ]
        runTest(filename,requests,**params)

    def test_032_filter_rect_wkt_create_spatial_index(self):
        # Filter extents on WKT layer building spatial index
        filename='testextw.txt'
        params={'delimiter': '|', 'type': 'csv', 'wktField': 'wkt' }
        requests=[
            {'extents': [10, 30, 30, 50]},
            {},
            lambda layer: layer.dataProvider().createSpatialIndex(),
            {'extents': [10, 30, 30, 50]},
            {'extents': [10, 30, 30, 50], 'exact': 1},
            {'extents': [110, 130, 130, 150]},
            {},
            {'extents': [-1000, -1000, 1000, 1000]}
        ]
        runTest(filename,requests,**params)


    def test_033_reset_subset_string(self):
        # CSV file parsing
        filename='test.csv'
        params={'geomType': 'none', 'type': 'csv'}
        requests=[
            {},
            lambda layer: layer.dataProvider().setSubsetString("id % 2 = 1",True),
            {},
            lambda layer: layer.dataProvider().setSubsetString("id = 6",False),
            {},
            lambda layer: layer.dataProvider().setSubsetString("id = 3",False),
            {},
            lambda layer: layer.dataProvider().setSubsetString("id % 2 = 1",True),
            {},
            lambda layer: layer.dataProvider().setSubsetString("id % 2 = 0",True),
            {},
        ]
        runTest(filename,requests,**params)

    def test_034_csvt_file(self):
        # CSVT field types
        filename='testcsvt.csv'
        params={'geomType': 'none', 'type': 'csv'}
        requests=None
        runTest(filename,requests,**params)

    def test_035_csvt_file2(self):
        # CSV field types 2
        filename='testcsvt2.txt'
        params={'geomType': 'none', 'type': 'csv', 'delimiter': '|'}
        requests=None
        runTest(filename,requests,**params)

    def test_036_csvt_file_invalid_types(self):
        # CSV field types invalid string format
        filename='testcsvt3.csv'
        params={'geomType': 'none', 'type': 'csv'}
        requests=None
        runTest(filename,requests,**params)

    def test_037_csvt_file_invalid_file(self):
        # CSV field types invalid file
        filename='testcsvt4.csv'
        params={'geomType': 'none', 'type': 'csv'}
        requests=None
        runTest(filename,requests,**params)


if __name__ == '__main__':
    unittest.main()
