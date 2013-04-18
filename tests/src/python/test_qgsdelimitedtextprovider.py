# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsDelimitedTextProvider.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Alexander Bruy'
__date__ = '20/08/2012'
__copyright__ = 'Copyright 2012, The Quantum GIS Project'
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
# To recreate all tests, set printTests to true

printTests = False

import os.path;
import re

from PyQt4.QtCore import (QVariant,
                        QUrl,
                        QObject,
                        QString,
                        pyqtSignal
                        )

from qgis.core import (QGis,
                        QgsProviderRegistry,
                        QgsVectorLayer,
                        QgsFeature,
                        QgsFeatureRequest,
                        QgsField,
                        QgsGeometry,
                        QgsPoint,
                        QgsMessageLog)

from utilities import (getQgisTestApp,
                       TestCase,
                       unitTestDataPath,
                       unittest
                       #expectedFailure
                       )
QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()


geomkey = "#geometry"
fidkey = "#fid"
tolerance = 0.000001 # Tolerance for coordinate comparisons in checkWktEqual

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

def layerData( layer ):
    first = True
    data = {}
    fields = []
    for f in layer.getFeatures():
        if first:
            first = False
            for field in f.fields():
                fields.append(str(field.name()))
        fielddata = { name: unicode(f[name].toString()) for name in fields }
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
        if 'id' not in fields: fields.insert(0,'id')
        if 'description' not in fields: fields.insert(1,'description')
        data[id]=fielddata
    fields.append(fidkey)
    fields.append(geomkey)
    return fields, data

# Retrieve the data for a delimited text url

def delimitedTextData( filename, **params ):
    # Create a layer for the specified file and query parameters
    # and return the data for the layer (fields, data)

    filepath = os.path.join(unitTestDataPath("delimitedtext"),filename);
    url = QUrl.fromLocalFile(filepath);
    for k in params.keys():
        url.addQueryItem(k,params[k])
    urlstr = url.toString()
    with MessageLogger('DelimitedText') as logger:
        layer = QgsVectorLayer(urlstr,'test','delimitedtext')
        fields = []
        data = {}
        if layer.isValid():
            fields,data = layerData(layer)
        log=[]
        for msg in logger.messages():
            log.append(msg.replace(filepath,'file'))
        return dict( fields=fields, data=data, log=log)

def sortKey( id ):
    return re.sub(r'^(\d*)',lambda x: '{0:05}'.format(int(x.group(0) or 0)),id)

def createTest(  description, filename, **params ):
    # Routine to write a new test for a file.  Need to check the output is right
    # first of course!
    import inspect
    test=inspect.stack()[1][3];
    result = delimitedTextData( filename, **params )
    print
    print "    def {0}(self):".format(test)
    print "        description={0}".format(repr(description))
    print "        filename={0}".format(repr(filename))
    print "        params={0}".format(repr(params))
    print "        if printTests:"
    print "            createTest(description,filename,**params)"
    print "            assert False,\"Set printTests to False to run delimited text tests\""

    data=result['data']
    log=result['log']
    fields=result['fields']
    prefix='        '

    # Dump the data for a layer - used to construct unit tests
    print prefix+"wanted={"
    for k in sorted(data.keys(),key=sortKey):
        row = data[k]
        print prefix+"    {0}: {{".format(repr(k))
        for f in fields:
            print prefix+"        "+repr(f)+": "+repr(row[f])+","
        print prefix+"        },";
    print prefix+"    }";

    print prefix+"log_wanted=["
    for msg in log:
        print prefix+'    '+repr(msg)+','
    print prefix+'    ]'
    print '        runTest(description,wanted,log_wanted,filename,**params)'
    print


def checkWktEqual( wkt1, wkt2 ):
    # Compare to WKT exported generated by exportToWkt
    # Slightly complex to allow for small numeric difference in
    # coordinates...
    if wkt1 == wkt2: return True
    # Use regex split with capture gropu to split into text and numbers
    numberre=re.compile(r'(\-?\d+(?:\.\d+)?)')
    p1=numberre.split(wkt1)
    p2=numberre.split(wkt2)
    if len(p1) != len(p2): return False
    for i in range(len(p1)):
        if i%2 == 1:
            # Numeric comparison
            diff=abs(float(p1[i])-float(p2[i]))
            if diff > tolerance: return False
        else:
            # Could be more fancy here in terms of text comparison if
            # turn out to be necessary.
            if p1 != p2: return False
    return True

def recordDifference( record1, record2 ):
    # Compare a record defined as a dictionary
    for k in record1.keys():
        if k not in record2:
            return "Field {0} is missing".format(k)
        r1k = record1[k]
        r2k = record2[k]
        if k == geomkey:
            if not checkWktEqual(r1k,r2k):
                return "Geometry differs: {0:.50} versus {1:.50}".format(r1k,r2k)
        else:
            if record1[k] != record2[k]:
                return "Field {0} differs: {1:.50} versus {2:.50}".format(k,repr(r1k),repr(r2k));
    for k in record2.keys():
        if k not in record1:
            return "Output contains extra field {0} is missing".format(k)
    return ''

def runTest( name, wanted, log_wanted, file, **params ):
    print "Running test:",name

    result = delimitedTextData( file, **params )
    data = result['data']
    log = result['log']

    failures = []
    for id in sorted(wanted.keys(),key=sortKey):
        wrec = wanted[id]
        trec = data.get(id,{})
        description = wrec['description']
        difference = recordDifference(wrec,trec)
        if not difference:
            print '    {0}: Passed'.format(description)
        else:
            print '    {0}: {1}'.format(description,difference)
            failures.append(description+': '+difference)
    for id in data.keys():
        if id not in wanted:
            msg= "Layer contains unexpected extra data with id: \"{0}\"".format(id)
            print '    '+msg
            failures.append(msg)
            break
    assert len(failures) == 0,"\n".join(failures)
    common=[]
    for l in log:
        if l in log_wanted:
            common.append(l)
    for l in common:
            log_wanted.remove(l)
            log.remove(l)
    for l in log_wanted:
        print '    Missing log message:',l
    for l in log:
        print '    Extra log message:',l
    if len(log)==0 and len(log_wanted)==0:
        print '    Message log correct: Passed'
    assert len(log_wanted) == 0, "Missing log messages:\n"+"\n".join(log_wanted)
    assert len(log) == 0, "Extra log messages:\n"+"\n".join(log)

class TestQgsDelimitedTextProvider(TestCase):

    def test_001_provider_defined( self ):
        registry=QgsProviderRegistry.instance()
        metadata = registry.providerMetadata('delimitedtext')
        assert metadata != None, "Delimited text provider is not installed"

#START

    def test_002_load_csv_file(self):
        description='CSV file parsing'
        filename='test.csv'
        params={'geomType': 'none', 'type': 'csv'}
        if printTests:
            createTest(description,filename,**params)
            assert False,"Set printTests to False to run delimited text tests"
        wanted={
            u'1': {
                'id': u'1',
                'description': u'Basic unquoted record',
                'data': u'Some data',
                'info': u'Some info',
                'field_5': u'',
                '#fid': 2L,
                '#geometry': 'None',
                },
            u'2': {
                'id': u'2',
                'description': u'Quoted field',
                'data': u'Quoted data',
                'info': u'Unquoted',
                'field_5': u'',
                '#fid': 3L,
                '#geometry': 'None',
                },
            u'3': {
                'id': u'3',
                'description': u'Escaped quotes',
                'data': u'Quoted "citation" data',
                'info': u'Unquoted',
                'field_5': u'',
                '#fid': 4L,
                '#geometry': 'None',
                },
            u'4': {
                'id': u'4',
                'description': u'Quoted newlines',
                'data': u'Line 1\nLine 2\n\nLine 4',
                'info': u'No data',
                'field_5': u'',
                '#fid': 5L,
                '#geometry': 'None',
                },
            u'5': {
                'id': u'5',
                'description': u'Extra fields',
                'data': u'data',
                'info': u'info',
                'field_5': u'message',
                '#fid': 9L,
                '#geometry': 'None',
                },
            u'6': {
                'id': u'6',
                'description': u'Missing fields',
                'data': u'',
                'info': u'',
                'field_5': u'',
                '#fid': 10L,
                '#geometry': 'None',
                },
            }
        log_wanted=[
            ]
        runTest(description,wanted,log_wanted,filename,**params)


    def test_002a_field_naming(self):
        description='Management of missing/duplicate/invalid field names'
        filename='testfields.csv'
        params={'geomType': 'none', 'type': 'csv'}
        if printTests:
            createTest(description,filename,**params)
            assert False,"Set printTests to False to run delimited text tests"
        wanted={
            u'1': {
                'id': u'1',
                'description': u'Generation of field names',
                'data': u'Some data',
                'field_4': u'Some info',
                'data_2': u'',
                'field_6': u'',
                'field_7': u'',
                'field_3_1': u'',
                'data_1': u'',
                'field_10': u'',
                'field_11': u'',
                'field_12': u'last data',
                '#fid': 2L,
                '#geometry': 'None',
                },
            }
        log_wanted=[
            ]
        runTest(description,wanted,log_wanted,filename,**params)


    def test_003_load_whitespace(self):
        description='Whitespace file parsing'
        filename='test.space'
        params={'geomType': 'none', 'type': 'whitespace'}
        if printTests:
            createTest(description,filename,**params)
            assert False,"Set printTests to False to run delimited text tests"
        wanted={
            u'1': {
                'id': u'1',
                'description': u'Simple_whitespace_file',
                'data': u'data1',
                'info': u'info1',
                'field_5': u'',
                'field_6': u'',
                '#fid': 2L,
                '#geometry': 'None',
                },
            u'2': {
                'id': u'2',
                'description': u'Whitespace_at_start_of_line',
                'data': u'data2',
                'info': u'info2',
                'field_5': u'',
                'field_6': u'',
                '#fid': 3L,
                '#geometry': 'None',
                },
            u'3': {
                'id': u'3',
                'description': u'Tab_whitespace',
                'data': u'data3',
                'info': u'info3',
                'field_5': u'',
                'field_6': u'',
                '#fid': 4L,
                '#geometry': 'None',
                },
            u'4': {
                'id': u'4',
                'description': u'Multiple_whitespace_characters',
                'data': u'data4',
                'info': u'info4',
                'field_5': u'',
                'field_6': u'',
                '#fid': 5L,
                '#geometry': 'None',
                },
            u'5': {
                'id': u'5',
                'description': u'Extra_fields',
                'data': u'data5',
                'info': u'info5',
                'field_5': u'message5',
                'field_6': u'rubbish5',
                '#fid': 6L,
                '#geometry': 'None',
                },
            u'6': {
                'id': u'6',
                'description': u'Missing_fields',
                'data': u'',
                'info': u'',
                'field_5': u'',
                'field_6': u'',
                '#fid': 7L,
                '#geometry': 'None',
                },
            }
        log_wanted=[
            ]
        runTest(description,wanted,log_wanted,filename,**params)


    def test_004_quote_escape(self):
        description='Quote and escape file parsing'
        filename='test.pipe'
        params={'geomType': 'none', 'quote': '"', 'delimiter': '|', 'escape': '\\'}
        if printTests:
            createTest(description,filename,**params)
            assert False,"Set printTests to False to run delimited text tests"
        wanted={
            u'1': {
                'id': u'1',
                'description': u'Using pipe delimiter',
                'data': u'data 1',
                'info': u'info 1',
                'field_5': u'',
                'field_6': u'',
                '#fid': 2L,
                '#geometry': 'None',
                },
            u'2': {
                'id': u'2',
                'description': u'Using backslash escape on pipe',
                'data': u'data 2 | piped',
                'info': u'info2',
                'field_5': u'',
                'field_6': u'',
                '#fid': 3L,
                '#geometry': 'None',
                },
            u'3': {
                'id': u'3',
                'description': u'Backslash escaped newline',
                'data': u'data3 \nline2 \nline3',
                'info': u'info3',
                'field_5': u'',
                'field_6': u'',
                '#fid': 4L,
                '#geometry': 'None',
                },
            u'4': {
                'id': u'4',
                'description': u'Empty field',
                'data': u'',
                'info': u'info4',
                'field_5': u'',
                'field_6': u'',
                '#fid': 7L,
                '#geometry': 'None',
                },
            u'5': {
                'id': u'5',
                'description': u'Quoted field',
                'data': u'More | piped data',
                'info': u'info5',
                'field_5': u'',
                'field_6': u'',
                '#fid': 8L,
                '#geometry': 'None',
                },
            u'6': {
                'id': u'6',
                'description': u'Escaped quote',
                'data': u'Field "citation" ',
                'info': u'info6',
                'field_5': u'',
                'field_6': u'',
                '#fid': 9L,
                '#geometry': 'None',
                },
            u'7': {
                'id': u'7',
                'description': u'Missing fields',
                'data': u'',
                'info': u'',
                'field_5': u'',
                'field_6': u'',
                '#fid': 10L,
                '#geometry': 'None',
                },
            u'8': {
                'id': u'8',
                'description': u'Extra fields',
                'data': u'data8',
                'info': u'info8',
                'field_5': u'message8',
                'field_6': u'more',
                '#fid': 11L,
                '#geometry': 'None',
                },
            }
        log_wanted=[
            ]
        runTest(description,wanted,log_wanted,filename,**params)


    def test_005_multiple_quote(self):
        description='Multiple quote and escape characters'
        filename='test.quote'
        params={'geomType': 'none', 'quote': '\'"', 'type': 'csv', 'escape': '"\''}
        if printTests:
            createTest(description,filename,**params)
            assert False,"Set printTests to False to run delimited text tests"
        wanted={
            u'1': {
                'id': u'1',
                'description': u'Multiple quotes 1',
                'data': u'Quoted,data1',
                'info': u'info1',
                '#fid': 2L,
                '#geometry': 'None',
                },
            u'2': {
                'id': u'2',
                'description': u'Multiple quotes 2',
                'data': u'Quoted,data2',
                'info': u'info2',
                '#fid': 3L,
                '#geometry': 'None',
                },
            u'3': {
                'id': u'3',
                'description': u'Leading and following whitespace',
                'data': u'Quoted, data3',
                'info': u'info3',
                '#fid': 4L,
                '#geometry': 'None',
                },
            u'4': {
                'id': u'4',
                'description': u'Embedded quotes 1',
                'data': u'Quoted \'\'"\'\' data4',
                'info': u'info4',
                '#fid': 5L,
                '#geometry': 'None',
                },
            u'5': {
                'id': u'5',
                'description': u'Embedded quotes 2',
                'data': u'Quoted \'""\' data5',
                'info': u'info5',
                '#fid': 6L,
                '#geometry': 'None',
                },
            u'9': {
                'id': u'9',
                'description': u'Final record',
                'data': u'date9',
                'info': u'info9',
                '#fid': 10L,
                '#geometry': 'None',
                },
            }
        log_wanted=[
            u'Errors in file file',
            u'The following lines were not loaded from file into QGIS due to errors:\n',
            u'Invalid record format at line 7',
            u'Invalid record format at line 8',
            u'Invalid record format at line 9',
            ]
        runTest(description,wanted,log_wanted,filename,**params)


    def test_005a_badly_formed_quotes(self):
        description='Badly formed quoted fields'
        filename='test.badquote'
        params={'geomType': 'none', 'quote': '"', 'type': 'csv', 'escape': '"'}
        if printTests:
            createTest(description,filename,**params)
            assert False,"Set printTests to False to run delimited text tests"
        wanted={
            u'3': {
                'id': u'3',
                'description': u'Recovered after unclosed quore',
                'data': u'Data ok',
                'info': u'inf3',
                '#fid': 4L,
                '#geometry': 'None',
                },
            }
        log_wanted=[
            u'Errors in file file',
            u'The following lines were not loaded from file into QGIS due to errors:\n',
            u'Invalid record format at line 2',
            u'Invalid record format at line 5',
            ]
        runTest(description,wanted,log_wanted,filename,**params)


    def test_007_skip_lines(self):
        description='Skip lines'
        filename='test2.csv'
        params={'geomType': 'none', 'useHeader': 'no', 'type': 'csv', 'skipLines': '2'}
        if printTests:
            createTest(description,filename,**params)
            assert False,"Set printTests to False to run delimited text tests"
        wanted={
            u'3': {
                'id': u'3',
                'description': u'Less data',
                'field_1': u'3',
                'field_2': u'Less data',
                'field_3': u'data3',
                '#fid': 3L,
                '#geometry': 'None',
                },
            }
        log_wanted=[
            ]
        runTest(description,wanted,log_wanted,filename,**params)


    def test_008_read_coordinates(self):
        description='Skip lines'
        filename='testpt.csv'
        params={'yField': 'geom_y', 'xField': 'geom_x', 'type': 'csv'}
        if printTests:
            createTest(description,filename,**params)
            assert False,"Set printTests to False to run delimited text tests"
        wanted={
            u'1': {
                'id': u'1',
                'description': u'Basic point',
                'geom_x': u'10',
                'geom_y': u'20',
                '#fid': 2L,
                '#geometry': 'POINT(10.0 20.0)',
                },
            u'2': {
                'id': u'2',
                'description': u'Integer point',
                'geom_x': u'11',
                'geom_y': u'22',
                '#fid': 3L,
                '#geometry': 'POINT(11.0 22.0)',
                },
            u'4': {
                'id': u'4',
                'description': u'Final point',
                'geom_x': u'13',
                'geom_y': u'23',
                '#fid': 5L,
                '#geometry': 'POINT(13.0 23.0)',
                },
            }
        log_wanted=[
            u'Errors in file file',
            u'The following lines were not loaded from file into QGIS due to errors:\n',
            u'Invalid X or Y fields at line 4',
            ]
        runTest(description,wanted,log_wanted,filename,**params)


    def test_009_read_wkt(self):
        description='Reading WKT geometry field'
        filename='testwkt.csv'
        params={'delimiter': '|', 'type': 'csv', 'wktField': 'geom_wkt'}
        if printTests:
            createTest(description,filename,**params)
            assert False,"Set printTests to False to run delimited text tests"
        wanted={
            u'1': {
                'id': u'1',
                'description': u'Point wkt',
                '#fid': 2L,
                '#geometry': 'POINT(10.0 20.0)',
                },
            u'2': {
                'id': u'2',
                'description': u'Multipoint wkt',
                '#fid': 3L,
                '#geometry': 'MULTIPOINT(10.0 20.0, 11.0 21.0)',
                },
            u'8': {
                'id': u'8',
                'description': u'EWKT prefix',
                '#fid': 9L,
                '#geometry': 'POINT(10.0 10.0)',
                },
            u'9': {
                'id': u'9',
                'description': u'Informix prefix',
                '#fid': 10L,
                '#geometry': 'POINT(10.0 10.0)',
                },
            u'10': {
                'id': u'10',
                'description': u'Measure in point',
                '#fid': 11L,
                '#geometry': 'POINT(10.0 20.0)',
                },
            }
        log_wanted=[
            u'Errors in file file',
            u'The following lines were not loaded from file into QGIS due to errors:\n',
            u'Invalid WKT at line 8',
            ]
        runTest(description,wanted,log_wanted,filename,**params)


    def test_010_read_wkt_point(self):
        description='Read WKT points'
        filename='testwkt.csv'
        params={'geomType': 'point', 'delimiter': '|', 'type': 'csv', 'wktField': 'geom_wkt'}
        if printTests:
            createTest(description,filename,**params)
            assert False,"Set printTests to False to run delimited text tests"
        wanted={
            u'1': {
                'id': u'1',
                'description': u'Point wkt',
                '#fid': 2L,
                '#geometry': 'POINT(10.0 20.0)',
                },
            u'2': {
                'id': u'2',
                'description': u'Multipoint wkt',
                '#fid': 3L,
                '#geometry': 'MULTIPOINT(10.0 20.0, 11.0 21.0)',
                },
            u'8': {
                'id': u'8',
                'description': u'EWKT prefix',
                '#fid': 9L,
                '#geometry': 'POINT(10.0 10.0)',
                },
            u'9': {
                'id': u'9',
                'description': u'Informix prefix',
                '#fid': 10L,
                '#geometry': 'POINT(10.0 10.0)',
                },
            u'10': {
                'id': u'10',
                'description': u'Measure in point',
                '#fid': 11L,
                '#geometry': 'POINT(10.0 20.0)',
                },
            }
        log_wanted=[
            u'Errors in file file',
            u'The following lines were not loaded from file into QGIS due to errors:\n',
            u'Invalid WKT at line 8',
            ]
        runTest(description,wanted,log_wanted,filename,**params)


    def test_011_read_wkt_line(self):
        description='Read WKT linestrings'
        filename='testwkt.csv'
        params={'geomType': 'line', 'delimiter': '|', 'type': 'csv', 'wktField': 'geom_wkt'}
        if printTests:
            createTest(description,filename,**params)
            assert False,"Set printTests to False to run delimited text tests"
        wanted={
            u'3': {
                'id': u'3',
                'description': u'Linestring wkt',
                '#fid': 4L,
                '#geometry': 'LINESTRING(10.0 20.0, 11.0 21.0)',
                },
            u'4': {
                'id': u'4',
                'description': u'Multiline string wkt',
                '#fid': 5L,
                '#geometry': 'MULTILINESTRING((10.0 20.0, 11.0 21.0), (20.0 30.0, 21.0 31.0))',
                },
            u'11': {
                'id': u'11',
                'description': u'Measure in line',
                '#fid': 12L,
                '#geometry': 'LINESTRING(10.0 20.0, 11.0 21.0)',
                },
            u'12': {
                'id': u'12',
                'description': u'Z in line',
                '#fid': 13L,
                '#geometry': 'LINESTRING(10.0 20.0, 11.0 21.0)',
                },
            u'13': {
                'id': u'13',
                'description': u'Measure and Z in line',
                '#fid': 14L,
                '#geometry': 'LINESTRING(10.0 20.0, 11.0 21.0)',
                },
            }
        log_wanted=[
            u'Errors in file file',
            u'The following lines were not loaded from file into QGIS due to errors:\n',
            u'Invalid WKT at line 8',
            ]
        runTest(description,wanted,log_wanted,filename,**params)


    def test_012_read_wkt_polygon(self):
        description='Read WKT polygons'
        filename='testwkt.csv'
        params={'geomType': 'polygon', 'delimiter': '|', 'type': 'csv', 'wktField': 'geom_wkt'}
        if printTests:
            createTest(description,filename,**params)
            assert False,"Set printTests to False to run delimited text tests"
        wanted={
            u'5': {
                'id': u'5',
                'description': u'Polygon wkt',
                '#fid': 6L,
                '#geometry': 'POLYGON((10.0 10.0,10.0 20.0,20.0 20.0,20.0 10.0,10.0 10.0),(14.0 14.0,14.0 16.0,16.0 16.0,14.0 14.0))',
                },
            u'6': {
                'id': u'6',
                'description': u'MultiPolygon wkt',
                '#fid': 7L,
                '#geometry': 'MULTIPOLYGON(((10.0 10.0,10.0 20.0,20.0 20.0,20.0 10.0,10.0 10.0),(14.0 14.0,14.0 16.0,16.0 16.0,14.0 14.0)),((30.0 30.0,30.0 35.0,35.0 35.0,30.0 30.0)))',
                },
            }
        log_wanted=[
            u'Errors in file file',
            u'The following lines were not loaded from file into QGIS due to errors:\n',
            u'Invalid WKT at line 8',
            ]
        runTest(description,wanted,log_wanted,filename,**params)


    def test_013_read_dms_xy(self):
        description='Reading degrees/minutes/seconds angles'
        filename='testdms.csv'
        params={'yField': 'lat', 'xField': 'lon', 'type': 'csv', 'xyDms': 'yes'}
        if printTests:
            createTest(description,filename,**params)
            assert False,"Set printTests to False to run delimited text tests"
        wanted={
            u'1': {
                'id': u'1',
                'description': u'Basic DMS string',
                'lon': u'1 5 30.6',
                'lat': u'35 51 20',
                '#fid': 3L,
                '#geometry': 'POINT(1.09183333 35.85555556)',
                },
            u'2': {
                'id': u'2',
                'description': u'Basic DMS string 2',
                'lon': u'1 05 30.6005',
                'lat': u'035 51 20',
                '#fid': 4L,
                '#geometry': 'POINT(1.09183347 35.85555556)',
                },
            u'3': {
                'id': u'3',
                'description': u'Basic DMS string 3',
                'lon': u'1 05 30.6',
                'lat': u'35 59 9.99',
                '#fid': 5L,
                '#geometry': 'POINT(1.09183333 35.98610833)',
                },
            u'4': {
                'id': u'4',
                'description': u'Prefix sign 1',
                'lon': u'n1 05 30.6',
                'lat': u'e035 51 20',
                '#fid': 7L,
                '#geometry': 'POINT(1.09183333 35.85555556)',
                },
            u'5': {
                'id': u'5',
                'description': u'Prefix sign 2',
                'lon': u'N1 05 30.6',
                'lat': u'E035 51 20',
                '#fid': 8L,
                '#geometry': 'POINT(1.09183333 35.85555556)',
                },
            u'6': {
                'id': u'6',
                'description': u'Prefix sign 3',
                'lon': u'N 1 05 30.6',
                'lat': u'E 035 51 20',
                '#fid': 9L,
                '#geometry': 'POINT(1.09183333 35.85555556)',
                },
            u'7': {
                'id': u'7',
                'description': u'Prefix sign 4',
                'lon': u'S1 05 30.6',
                'lat': u'W035 51 20',
                '#fid': 10L,
                '#geometry': 'POINT(-1.09183333 -35.85555556)',
                },
            u'8': {
                'id': u'8',
                'description': u'Prefix sign 5',
                'lon': u'+1 05 30.6',
                'lat': u'+035 51 20',
                '#fid': 11L,
                '#geometry': 'POINT(1.09183333 35.85555556)',
                },
            u'9': {
                'id': u'9',
                'description': u'Prefix sign 6',
                'lon': u'-1 05 30.6',
                'lat': u'-035 51 20',
                '#fid': 12L,
                '#geometry': 'POINT(-1.09183333 -35.85555556)',
                },
            u'10': {
                'id': u'10',
                'description': u'Postfix sign 1',
                'lon': u'1 05 30.6n',
                'lat': u'035 51 20e',
                '#fid': 14L,
                '#geometry': 'POINT(1.09183333 35.85555556)',
                },
            u'11': {
                'id': u'11',
                'description': u'Postfix sign 2',
                'lon': u'1 05 30.6N',
                'lat': u'035 51 20E',
                '#fid': 15L,
                '#geometry': 'POINT(1.09183333 35.85555556)',
                },
            u'12': {
                'id': u'12',
                'description': u'Postfix sign 3',
                'lon': u'1 05 30.6 N',
                'lat': u'035 51 20 E',
                '#fid': 16L,
                '#geometry': 'POINT(1.09183333 35.85555556)',
                },
            u'13': {
                'id': u'13',
                'description': u'Postfix sign 4',
                'lon': u'1 05 30.6S',
                'lat': u'035 51 20W',
                '#fid': 17L,
                '#geometry': 'POINT(-1.09183333 -35.85555556)',
                },
            u'14': {
                'id': u'14',
                'description': u'Postfix sign 5',
                'lon': u'1 05 30.6+',
                'lat': u'035 51 20+',
                '#fid': 18L,
                '#geometry': 'POINT(1.09183333 35.85555556)',
                },
            u'15': {
                'id': u'15',
                'description': u'Postfix sign 6',
                'lon': u'1 05 30.6-',
                'lat': u'035 51 20-',
                '#fid': 19L,
                '#geometry': 'POINT(-1.09183333 -35.85555556)',
                },
            u'16': {
                'id': u'16',
                'description': u'Leading and trailing blanks 1',
                'lon': u'   1 05 30.6',
                'lat': u'035 51 20   ',
                '#fid': 21L,
                '#geometry': 'POINT(1.09183333 35.85555556)',
                },
            u'17': {
                'id': u'17',
                'description': u'Leading and trailing blanks 2',
                'lon': u' N  1 05 30.6',
                'lat': u'035 51 20 E  ',
                '#fid': 22L,
                '#geometry': 'POINT(1.09183333 35.85555556)',
                },
            u'18': {
                'id': u'18',
                'description': u'Alternative characters for D,M,S',
                'lon': u'1d05m30.6s S',
                'lat': u"35d51'20",
                '#fid': 24L,
                '#geometry': 'POINT(-1.09183333 35.85555556)',
                },
            u'19': {
                'id': u'19',
                'description': u'Degrees/minutes format',
                'lon': u'1 05.23',
                'lat': u'4 55.03',
                '#fid': 25L,
                '#geometry': 'POINT(1.08716667 4.91716667)',
                },
            }
        log_wanted=[
            u'Errors in file file',
            u'The following lines were not loaded from file into QGIS due to errors:\n',
            u'Invalid X or Y fields at line 27',
            u'Invalid X or Y fields at line 28',
            u'Invalid X or Y fields at line 29',
            u'Invalid X or Y fields at line 30',
            u'Invalid X or Y fields at line 31',
            ]
        runTest(description,wanted,log_wanted,filename,**params)


    def test_014_decimal_point(self):
        description='Reading degrees/minutes/seconds angles'
        filename='testdp.csv'
        params={'yField': 'geom_y', 'xField': 'geom_x', 'type': 'csv', 'delimiter': ';', 'decimalPoint': ','}
        if printTests:
            createTest(description,filename,**params)
            assert False,"Set printTests to False to run delimited text tests"
        wanted={
            u'1': {
                'id': u'1',
                'description': u'Comma as decimal point 1',
                'geom_x': u'10',
                'geom_y': u'20',
                'other': u'30',
                'text field': u'Field with , in it',
                '#fid': 2L,
                '#geometry': 'POINT(10.0 20.0)',
                },
            u'2': {
                'id': u'2',
                'description': u'Comma as decimal point 2',
                'geom_x': u'12',
                'geom_y': u'25.003',
                'other': u'-38.55',
                'text field': u'Plain text field',
                '#fid': 3L,
                '#geometry': 'POINT(12.0 25.003)',
                },
            }
        log_wanted=[
            ]
        runTest(description,wanted,log_wanted,filename,**params)


    def test_015_regular_expression_1(self):
        description='Parsing regular expression delimiter'
        filename='testre.txt'
        params={'geomType': 'none', 'trimFields': 'Y', 'delimiter': 'RE(?:GEXP)?', 'type': 'regexp'}
        if printTests:
            createTest(description,filename,**params)
            assert False,"Set printTests to False to run delimited text tests"
        wanted={
            u'1': {
                'id': u'1',
                'description': u'Basic regular expression test',
                'data': u'data1',
                'info': u'info',
                '#fid': 2L,
                '#geometry': 'None',
                },
            u'2': {
                'id': u'2',
                'description': u'Basic regular expression test 2',
                'data': u'data2',
                'info': u'info2',
                '#fid': 3L,
                '#geometry': 'None',
                },
            }
        log_wanted=[
            ]
        runTest(description,wanted,log_wanted,filename,**params)


    def test_016_regular_expression_2(self):
        description='Parsing regular expression delimiter with capture groups'
        filename='testre.txt'
        params={'geomType': 'none', 'trimFields': 'Y', 'delimiter': '(RE)(GEXP)?', 'type': 'regexp'}
        if printTests:
            createTest(description,filename,**params)
            assert False,"Set printTests to False to run delimited text tests"
        wanted={
            u'1': {
                'id': u'1',
                'RE': u'RE',
                'GEXP': u'GEXP',
                'description': u'RE',
                'RE_1': u'RE',
                'GEXP_1': u'GEXP',
                'data': u'data1',
                'RE_2': u'RE',
                'GEXP_2': u'GEXP',
                'info': u'info',
                '#fid': 2L,
                '#geometry': 'None',
                },
            u'2': {
                'id': u'2',
                'RE': u'RE',
                'GEXP': u'GEXP',
                'description': u'RE',
                'RE_1': u'RE',
                'GEXP_1': u'',
                'data': u'data2',
                'RE_2': u'RE',
                'GEXP_2': u'',
                'info': u'info2',
                '#fid': 3L,
                '#geometry': 'None',
                },
            }
        log_wanted=[
            ]
        runTest(description,wanted,log_wanted,filename,**params)


    def test_017_regular_expression_3(self):
        description='Parsing anchored regular expression'
        filename='testre2.txt'
        params={'geomType': 'none', 'trimFields': 'Y', 'delimiter': '^(.{5})(.{30})(.{5,})', 'type': 'regexp'}
        if printTests:
            createTest(description,filename,**params)
            assert False,"Set printTests to False to run delimited text tests"
        wanted={
            u'1': {
                'id': u'1',
                'description': u'Anchored regexp',
                'information': u'Some data',
                '#fid': 2L,
                '#geometry': 'None',
                },
            u'3': {
                'id': u'3',
                'description': u'Anchored regexp recovered',
                'information': u'Some data',
                '#fid': 4L,
                '#geometry': 'None',
                },
            }
        log_wanted=[
            u'Errors in file file',
            u'The following lines were not loaded from file into QGIS due to errors:\n',
            u'Invalid record format at line 3',
            ]
        runTest(description,wanted,log_wanted,filename,**params)


    def test_018_utf8_encoded_file(self):
        description='UTF8 encoded file test'
        filename='testutf8.csv'
        params={'geomType': 'none', 'delimiter': '|', 'type': 'csv', 'encoding': 'utf-8'}
        if printTests:
            createTest(description,filename,**params)
            assert False,"Set printTests to False to run delimited text tests"
        wanted={
            u'1': {
                'id': u'1',
                'description': u'Correctly read UTF8 encoding',
                'name': u'Field has \u0101cc\xe8nt\xe9d text',
                '#fid': 2L,
                '#geometry': 'None',
                },
            }
        log_wanted=[
            ]
        runTest(description,wanted,log_wanted,filename,**params)


    def test_019_latin1_encoded_file(self):
        description='Latin1 encoded file test'
        filename='testlatin1.csv'
        params={'geomType': 'none', 'delimiter': '|', 'type': 'csv', 'encoding': 'latin1'}
        if printTests:
            createTest(description,filename,**params)
            assert False,"Set printTests to False to run delimited text tests"
        wanted={
            u'1': {
                'id': u'1',
                'description': u'Correctly read latin1 encoding',
                'name': u'This test is \xa9',
                '#fid': 2L,
                '#geometry': 'None',
                },
            }
        log_wanted=[
            ]
        runTest(description,wanted,log_wanted,filename,**params)



#END

if __name__ == '__main__':
    unittest.main()
