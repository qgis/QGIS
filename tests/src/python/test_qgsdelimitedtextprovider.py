# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsDelimitedTextProvider.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Chris Crook'
__date__ = '20/04/2013'
__copyright__ = 'Copyright 2013, The Quantum GIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

rebuildTests = False
#rebuildTests = True

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
                        QgsRectangle,
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

def layerData( layer, request={}, offset=0 ):
    first = True
    data = {}
    fields = []
    fr = QgsFeatureRequest()
    if request:
        if 'exact' in request and request['exact']:
            fr.setFlags(QgsFeatureRequest.ExactIntersect)
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
        fielddata = dict ( (name, unicode(f[name].toString()) ) for name in fields )
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
        data[f.id()+offset]=fielddata
    fields.append(fidkey)
    fields.append(geomkey)
    return fields, data

# Retrieve the data for a delimited text url

def delimitedTextData( filename, requests, **params ):
    # Create a layer for the specified file and query parameters
    # and return the data for the layer (fields, data)

    filepath = os.path.join(unitTestDataPath("delimitedtext"),filename);
    url = QUrl.fromLocalFile(filepath);
    if not requests:
        requests=[{}]
    for k in params.keys():
        url.addQueryItem(k,params[k])
    urlstr = url.toString()
    with MessageLogger('DelimitedText') as logger:
        layer = QgsVectorLayer(urlstr,'test','delimitedtext')
        fields = []
        data = {}
        if layer.isValid():
            for nr,r in enumerate(requests):
                rfields,rdata = layerData(layer,r,nr*1000)
                if len(rfields) > len(fields): fields = rfields
                data.update(rdata)
        log=[]
        for msg in logger.messages():
            log.append(msg.replace(filepath,'file'))
        uri = unicode(layer.dataProvider().dataSourceUri())
        uri = uri.replace(filepath,'file')
        return dict( fields=fields, data=data, log=log, uri=uri)

def createTest(  description, filename, requests, **params ):
    # Routine to write a new test for a file.  Need to check the output is right
    # first of course!
    import inspect
    test=inspect.stack()[1][3];
    result = delimitedTextData( filename, requests, **params )
    print
    print "    def {0}(self):".format(test)
    print "        description={0}".format(repr(description))
    print "        filename={0}".format(repr(filename))
    print "        params={0}".format(repr(params))
    print "        requests={0}".format(repr(requests)).replace("{","\n            {")
    print "        if rebuildTests:"
    print "            createTest(description,filename,requests,**params)"
    print "            assert False,\"Set rebuildTests to False to run delimited text tests\""

    data=result['data']
    log=result['log']
    fields=result['fields']
    prefix='        '

    # Dump the data for a layer - used to construct unit tests
    print prefix+"wanted={}"
    print prefix+"wanted['uri']="+repr(result['uri'])
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
    print '        runTest(description,wanted,filename,requests,**params)'
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

def runTest( name, wanted, file, requests, **params ):
    print "Running test:",name

    result = delimitedTextData( file, requests, **params )
    data = result['data']
    log = result['log']
    failures = []
    if result['uri'] != wanted['uri']:
        msg = "Layer Uri ({0}) doesn't match expected ({1})".format(
            result['uri'],wanted['uri'])
        print '    '+msg
        falures.append(msg)
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
    for l in common:
            log_wanted.remove(l)
            log.remove(l)
    for l in log_wanted:
        msg='Missing log message: '+l
        print '    '+msg
        failures.append(msg)
        
    for l in log:
        msg='Extra log message: '+l
        print '    '+msg
        failures.append(msg)
    if len(log)==0 and len(log_wanted)==0:
        print '    Message log correct: Passed'
    assert len(failures) == 0,"\n".join(failures)

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
        requests=None
        if rebuildTests:
            createTest(description,filename,requests,**params)
            assert False,"Set rebuildTests to False to run delimited text tests"
        wanted={}
        wanted['uri']=u'file://file?geomType=none&type=csv'
        wanted['data']={
            2L: {
                'id': u'1',
                'description': u'Basic unquoted record',
                'data': u'Some data',
                'info': u'Some info',
                'field_5': u'',
                '#fid': 2L,
                '#geometry': 'None',
                },
            3L: {
                'id': u'2',
                'description': u'Quoted field',
                'data': u'Quoted data',
                'info': u'Unquoted',
                'field_5': u'',
                '#fid': 3L,
                '#geometry': 'None',
                },
            4L: {
                'id': u'3',
                'description': u'Escaped quotes',
                'data': u'Quoted "citation" data',
                'info': u'Unquoted',
                'field_5': u'',
                '#fid': 4L,
                '#geometry': 'None',
                },
            5L: {
                'id': u'4',
                'description': u'Quoted newlines',
                'data': u'Line 1\nLine 2\n\nLine 4',
                'info': u'No data',
                'field_5': u'',
                '#fid': 5L,
                '#geometry': 'None',
                },
            9L: {
                'id': u'5',
                'description': u'Extra fields',
                'data': u'data',
                'info': u'info',
                'field_5': u'message',
                '#fid': 9L,
                '#geometry': 'None',
                },
            10L: {
                'id': u'6',
                'description': u'Missing fields',
                'data': u'',
                'info': u'',
                'field_5': u'',
                '#fid': 10L,
                '#geometry': 'None',
                },
            }
        wanted['log']=[
            ]
        runTest(description,wanted,filename,requests,**params)


    def test_002a_field_naming(self):
        description='Management of missing/duplicate/invalid field names'
        filename='testfields.csv'
        params={'geomType': 'none', 'type': 'csv'}
        requests=None
        if rebuildTests:
            createTest(description,filename,requests,**params)
            assert False,"Set rebuildTests to False to run delimited text tests"
        wanted={}
        wanted['uri']=u'file://file?geomType=none&type=csv'
        wanted['data']={
            2L: {
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
        wanted['log']=[
            ]
        runTest(description,wanted,filename,requests,**params)


    def test_002b_max_fields(self):
        description='Limiting maximum number of fields'
        filename='testfields.csv'
        params={'geomType': 'none', 'maxFields': '7', 'type': 'csv'}
        requests=None
        if rebuildTests:
            createTest(description,filename,requests,**params)
            assert False,"Set rebuildTests to False to run delimited text tests"
        wanted={}
        wanted['uri']=u'file://file?geomType=none&maxFields=7&type=csv'
        wanted['data']={
            2L: {
                'id': u'1',
                'description': u'Generation of field names',
                'data': u'Some data',
                'field_4': u'Some info',
                'data_1': u'',
                'field_6': u'',
                'field_7': u'',
                '#fid': 2L,
                '#geometry': 'None',
                },
            }
        wanted['log']=[
            ]
        runTest(description,wanted,filename,requests,**params)


    def test_003_load_whitespace(self):
        description='Whitespace file parsing'
        filename='test.space'
        params={'geomType': 'none', 'type': 'whitespace'}
        requests=None
        if rebuildTests:
            createTest(description,filename,requests,**params)
            assert False,"Set rebuildTests to False to run delimited text tests"
        wanted={}
        wanted['uri']=u'file://file?geomType=none&type=whitespace'
        wanted['data']={
            2L: {
                'id': u'1',
                'description': u'Simple_whitespace_file',
                'data': u'data1',
                'info': u'info1',
                'field_5': u'',
                'field_6': u'',
                '#fid': 2L,
                '#geometry': 'None',
                },
            3L: {
                'id': u'2',
                'description': u'Whitespace_at_start_of_line',
                'data': u'data2',
                'info': u'info2',
                'field_5': u'',
                'field_6': u'',
                '#fid': 3L,
                '#geometry': 'None',
                },
            4L: {
                'id': u'3',
                'description': u'Tab_whitespace',
                'data': u'data3',
                'info': u'info3',
                'field_5': u'',
                'field_6': u'',
                '#fid': 4L,
                '#geometry': 'None',
                },
            5L: {
                'id': u'4',
                'description': u'Multiple_whitespace_characters',
                'data': u'data4',
                'info': u'info4',
                'field_5': u'',
                'field_6': u'',
                '#fid': 5L,
                '#geometry': 'None',
                },
            6L: {
                'id': u'5',
                'description': u'Extra_fields',
                'data': u'data5',
                'info': u'info5',
                'field_5': u'message5',
                'field_6': u'rubbish5',
                '#fid': 6L,
                '#geometry': 'None',
                },
            7L: {
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
        wanted['log']=[
            ]
        runTest(description,wanted,filename,requests,**params)


    def test_004_quote_escape(self):
        description='Quote and escape file parsing'
        filename='test.pipe'
        params={'geomType': 'none', 'quote': '"', 'delimiter': '|', 'escape': '\\'}
        requests=None
        if rebuildTests:
            createTest(description,filename,requests,**params)
            assert False,"Set rebuildTests to False to run delimited text tests"
        wanted={}
        wanted['uri']=u'file://file?geomType=none&quote="&delimiter=|&escape=\\'
        wanted['data']={
            2L: {
                'id': u'1',
                'description': u'Using pipe delimiter',
                'data': u'data 1',
                'info': u'info 1',
                'field_5': u'',
                'field_6': u'',
                '#fid': 2L,
                '#geometry': 'None',
                },
            3L: {
                'id': u'2',
                'description': u'Using backslash escape on pipe',
                'data': u'data 2 | piped',
                'info': u'info2',
                'field_5': u'',
                'field_6': u'',
                '#fid': 3L,
                '#geometry': 'None',
                },
            4L: {
                'id': u'3',
                'description': u'Backslash escaped newline',
                'data': u'data3 \nline2 \nline3',
                'info': u'info3',
                'field_5': u'',
                'field_6': u'',
                '#fid': 4L,
                '#geometry': 'None',
                },
            7L: {
                'id': u'4',
                'description': u'Empty field',
                'data': u'',
                'info': u'info4',
                'field_5': u'',
                'field_6': u'',
                '#fid': 7L,
                '#geometry': 'None',
                },
            8L: {
                'id': u'5',
                'description': u'Quoted field',
                'data': u'More | piped data',
                'info': u'info5',
                'field_5': u'',
                'field_6': u'',
                '#fid': 8L,
                '#geometry': 'None',
                },
            9L: {
                'id': u'6',
                'description': u'Escaped quote',
                'data': u'Field "citation" ',
                'info': u'info6',
                'field_5': u'',
                'field_6': u'',
                '#fid': 9L,
                '#geometry': 'None',
                },
            10L: {
                'id': u'7',
                'description': u'Missing fields',
                'data': u'',
                'info': u'',
                'field_5': u'',
                'field_6': u'',
                '#fid': 10L,
                '#geometry': 'None',
                },
            11L: {
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
        wanted['log']=[
            ]
        runTest(description,wanted,filename,requests,**params)


    def test_005_multiple_quote(self):
        description='Multiple quote and escape characters'
        filename='test.quote'
        params={'geomType': 'none', 'quote': '\'"', 'type': 'csv', 'escape': '"\''}
        requests=None
        if rebuildTests:
            createTest(description,filename,requests,**params)
            assert False,"Set rebuildTests to False to run delimited text tests"
        wanted={}
        wanted['uri']=u'file://file?geomType=none&quote=\'"&type=csv&escape="\''
        wanted['data']={
            2L: {
                'id': u'1',
                'description': u'Multiple quotes 1',
                'data': u'Quoted,data1',
                'info': u'info1',
                '#fid': 2L,
                '#geometry': 'None',
                },
            3L: {
                'id': u'2',
                'description': u'Multiple quotes 2',
                'data': u'Quoted,data2',
                'info': u'info2',
                '#fid': 3L,
                '#geometry': 'None',
                },
            4L: {
                'id': u'3',
                'description': u'Leading and following whitespace',
                'data': u'Quoted, data3',
                'info': u'info3',
                '#fid': 4L,
                '#geometry': 'None',
                },
            5L: {
                'id': u'4',
                'description': u'Embedded quotes 1',
                'data': u'Quoted \'\'"\'\' data4',
                'info': u'info4',
                '#fid': 5L,
                '#geometry': 'None',
                },
            6L: {
                'id': u'5',
                'description': u'Embedded quotes 2',
                'data': u'Quoted \'""\' data5',
                'info': u'info5',
                '#fid': 6L,
                '#geometry': 'None',
                },
            10L: {
                'id': u'9',
                'description': u'Final record',
                'data': u'date9',
                'info': u'info9',
                '#fid': 10L,
                '#geometry': 'None',
                },
            }
        wanted['log']=[
            u'Errors in file file',
            u'3 records discarded due to invalid format',
            u'The following lines were not loaded into QGIS due to errors:',
            u'Invalid record format at line 7',
            u'Invalid record format at line 8',
            u'Invalid record format at line 9',
            ]
        runTest(description,wanted,filename,requests,**params)


    def test_005a_badly_formed_quotes(self):
        description='Badly formed quoted fields'
        filename='test.badquote'
        params={'geomType': 'none', 'quote': '"', 'type': 'csv', 'escape': '"'}
        requests=None
        if rebuildTests:
            createTest(description,filename,requests,**params)
            assert False,"Set rebuildTests to False to run delimited text tests"
        wanted={}
        wanted['uri']=u'file://file?geomType=none&quote="&type=csv&escape="'
        wanted['data']={
            4L: {
                'id': u'3',
                'description': u'Recovered after unclosed quore',
                'data': u'Data ok',
                'info': u'inf3',
                '#fid': 4L,
                '#geometry': 'None',
                },
            }
        wanted['log']=[
            u'Errors in file file',
            u'2 records discarded due to invalid format',
            u'The following lines were not loaded into QGIS due to errors:',
            u'Invalid record format at line 2',
            u'Invalid record format at line 5',
            ]
        runTest(description,wanted,filename,requests,**params)


    def test_007_skip_lines(self):
        description='Skip lines'
        filename='test2.csv'
        params={'geomType': 'none', 'useHeader': 'no', 'type': 'csv', 'skipLines': '2'}
        requests=None
        if rebuildTests:
            createTest(description,filename,requests,**params)
            assert False,"Set rebuildTests to False to run delimited text tests"
        wanted={}
        wanted['uri']=u'file://file?geomType=none&skipLines=2&type=csv&useHeader=no'
        wanted['data']={
            3L: {
                'id': u'3',
                'description': u'Less data',
                'field_1': u'3',
                'field_2': u'Less data',
                'field_3': u'data3',
                '#fid': 3L,
                '#geometry': 'None',
                },
            }
        wanted['log']=[
            ]
        runTest(description,wanted,filename,requests,**params)


    def test_008_read_coordinates(self):
        description='Skip lines'
        filename='testpt.csv'
        params={'yField': 'geom_y', 'xField': 'geom_x', 'type': 'csv'}
        requests=None
        if rebuildTests:
            createTest(description,filename,requests,**params)
            assert False,"Set rebuildTests to False to run delimited text tests"
        wanted={}
        wanted['uri']=u'file://file?yField=geom_y&xField=geom_x&type=csv'
        wanted['data']={
            2L: {
                'id': u'1',
                'description': u'Basic point',
                'geom_x': u'10',
                'geom_y': u'20',
                '#fid': 2L,
                '#geometry': 'POINT(10.0 20.0)',
                },
            3L: {
                'id': u'2',
                'description': u'Integer point',
                'geom_x': u'11',
                'geom_y': u'22',
                '#fid': 3L,
                '#geometry': 'POINT(11.0 22.0)',
                },
            5L: {
                'id': u'4',
                'description': u'Final point',
                'geom_x': u'13',
                'geom_y': u'23',
                '#fid': 5L,
                '#geometry': 'POINT(13.0 23.0)',
                },
            }
        wanted['log']=[
            u'Errors in file file',
            u'1 records discarded due to invalid geometry definitions',
            u'The following lines were not loaded into QGIS due to errors:',
            u'Invalid X or Y fields at line 4',
            ]
        runTest(description,wanted,filename,requests,**params)


    def test_009_read_wkt(self):
        description='Reading WKT geometry field'
        filename='testwkt.csv'
        params={'delimiter': '|', 'type': 'csv', 'wktField': 'geom_wkt'}
        requests=None
        if rebuildTests:
            createTest(description,filename,requests,**params)
            assert False,"Set rebuildTests to False to run delimited text tests"
        wanted={}
        wanted['uri']=u'file://file?delimiter=|&type=csv&wktField=geom_wkt'
        wanted['data']={
            2L: {
                'id': u'1',
                'description': u'Point wkt',
                '#fid': 2L,
                '#geometry': 'POINT(10.0 20.0)',
                },
            3L: {
                'id': u'2',
                'description': u'Multipoint wkt',
                '#fid': 3L,
                '#geometry': 'MULTIPOINT(10.0 20.0, 11.0 21.0)',
                },
            9L: {
                'id': u'8',
                'description': u'EWKT prefix',
                '#fid': 9L,
                '#geometry': 'POINT(10.0 10.0)',
                },
            10L: {
                'id': u'9',
                'description': u'Informix prefix',
                '#fid': 10L,
                '#geometry': 'POINT(10.0 10.0)',
                },
            11L: {
                'id': u'10',
                'description': u'Measure in point',
                '#fid': 11L,
                '#geometry': 'POINT(10.0 20.0)',
                },
            }
        wanted['log']=[
            u'Errors in file file',
            u'1 records discarded due to invalid geometry definitions',
            u'7 records discarded due to incompatible geometry types',
            u'The following lines were not loaded into QGIS due to errors:',
            u'Invalid WKT at line 8',
            ]
        runTest(description,wanted,filename,requests,**params)


    def test_010_read_wkt_point(self):
        description='Read WKT points'
        filename='testwkt.csv'
        params={'geomType': 'point', 'delimiter': '|', 'type': 'csv', 'wktField': 'geom_wkt'}
        requests=None
        if rebuildTests:
            createTest(description,filename,requests,**params)
            assert False,"Set rebuildTests to False to run delimited text tests"
        wanted={}
        wanted['uri']=u'file://file?geomType=point&delimiter=|&type=csv&wktField=geom_wkt'
        wanted['data']={
            2L: {
                'id': u'1',
                'description': u'Point wkt',
                '#fid': 2L,
                '#geometry': 'POINT(10.0 20.0)',
                },
            3L: {
                'id': u'2',
                'description': u'Multipoint wkt',
                '#fid': 3L,
                '#geometry': 'MULTIPOINT(10.0 20.0, 11.0 21.0)',
                },
            9L: {
                'id': u'8',
                'description': u'EWKT prefix',
                '#fid': 9L,
                '#geometry': 'POINT(10.0 10.0)',
                },
            10L: {
                'id': u'9',
                'description': u'Informix prefix',
                '#fid': 10L,
                '#geometry': 'POINT(10.0 10.0)',
                },
            11L: {
                'id': u'10',
                'description': u'Measure in point',
                '#fid': 11L,
                '#geometry': 'POINT(10.0 20.0)',
                },
            }
        wanted['log']=[
            u'Errors in file file',
            u'1 records discarded due to invalid geometry definitions',
            u'7 records discarded due to incompatible geometry types',
            u'The following lines were not loaded into QGIS due to errors:',
            u'Invalid WKT at line 8',
            ]
        runTest(description,wanted,filename,requests,**params)


    def test_011_read_wkt_line(self):
        description='Read WKT linestrings'
        filename='testwkt.csv'
        params={'geomType': 'line', 'delimiter': '|', 'type': 'csv', 'wktField': 'geom_wkt'}
        requests=None
        if rebuildTests:
            createTest(description,filename,requests,**params)
            assert False,"Set rebuildTests to False to run delimited text tests"
        wanted={}
        wanted['uri']=u'file://file?geomType=line&delimiter=|&type=csv&wktField=geom_wkt'
        wanted['data']={
            4L: {
                'id': u'3',
                'description': u'Linestring wkt',
                '#fid': 4L,
                '#geometry': 'LINESTRING(10.0 20.0, 11.0 21.0)',
                },
            5L: {
                'id': u'4',
                'description': u'Multiline string wkt',
                '#fid': 5L,
                '#geometry': 'MULTILINESTRING((10.0 20.0, 11.0 21.0), (20.0 30.0, 21.0 31.0))',
                },
            12L: {
                'id': u'11',
                'description': u'Measure in line',
                '#fid': 12L,
                '#geometry': 'LINESTRING(10.0 20.0, 11.0 21.0)',
                },
            13L: {
                'id': u'12',
                'description': u'Z in line',
                '#fid': 13L,
                '#geometry': 'LINESTRING(10.0 20.0, 11.0 21.0)',
                },
            14L: {
                'id': u'13',
                'description': u'Measure and Z in line',
                '#fid': 14L,
                '#geometry': 'LINESTRING(10.0 20.0, 11.0 21.0)',
                },
            }
        wanted['log']=[
            u'Errors in file file',
            u'1 records discarded due to invalid geometry definitions',
            u'7 records discarded due to incompatible geometry types',
            u'The following lines were not loaded into QGIS due to errors:',
            u'Invalid WKT at line 8',
            ]
        runTest(description,wanted,filename,requests,**params)


    def test_012_read_wkt_polygon(self):
        description='Read WKT polygons'
        filename='testwkt.csv'
        params={'geomType': 'polygon', 'delimiter': '|', 'type': 'csv', 'wktField': 'geom_wkt'}
        requests=None
        if rebuildTests:
            createTest(description,filename,requests,**params)
            assert False,"Set rebuildTests to False to run delimited text tests"
        wanted={}
        wanted['uri']=u'file://file?geomType=polygon&delimiter=|&type=csv&wktField=geom_wkt'
        wanted['data']={
            6L: {
                'id': u'5',
                'description': u'Polygon wkt',
                '#fid': 6L,
                '#geometry': 'POLYGON((10.0 10.0,10.0 20.0,20.0 20.0,20.0 10.0,10.0 10.0),(14.0 14.0,14.0 16.0,16.0 16.0,14.0 14.0))',
                },
            7L: {
                'id': u'6',
                'description': u'MultiPolygon wkt',
                '#fid': 7L,
                '#geometry': 'MULTIPOLYGON(((10.0 10.0,10.0 20.0,20.0 20.0,20.0 10.0,10.0 10.0),(14.0 14.0,14.0 16.0,16.0 16.0,14.0 14.0)),((30.0 30.0,30.0 35.0,35.0 35.0,30.0 30.0)))',
                },
            }
        wanted['log']=[
            u'Errors in file file',
            u'1 records discarded due to invalid geometry definitions',
            u'10 records discarded due to incompatible geometry types',
            u'The following lines were not loaded into QGIS due to errors:',
            u'Invalid WKT at line 8',
            ]
        runTest(description,wanted,filename,requests,**params)


    def test_013_read_dms_xy(self):
        description='Reading degrees/minutes/seconds angles'
        filename='testdms.csv'
        params={'yField': 'lat', 'xField': 'lon', 'type': 'csv', 'xyDms': 'yes'}
        requests=None
        if rebuildTests:
            createTest(description,filename,requests,**params)
            assert False,"Set rebuildTests to False to run delimited text tests"
        wanted={}
        wanted['uri']=u'file://file?yField=lat&xField=lon&type=csv&xyDms=yes'
        wanted['data']={
            3L: {
                'id': u'1',
                'description': u'Basic DMS string',
                'lon': u'1 5 30.6',
                'lat': u'35 51 20',
                '#fid': 3L,
                '#geometry': 'POINT(1.09183333 35.85555556)',
                },
            4L: {
                'id': u'2',
                'description': u'Basic DMS string 2',
                'lon': u'1 05 30.6005',
                'lat': u'035 51 20',
                '#fid': 4L,
                '#geometry': 'POINT(1.09183347 35.85555556)',
                },
            5L: {
                'id': u'3',
                'description': u'Basic DMS string 3',
                'lon': u'1 05 30.6',
                'lat': u'35 59 9.99',
                '#fid': 5L,
                '#geometry': 'POINT(1.09183333 35.98610833)',
                },
            7L: {
                'id': u'4',
                'description': u'Prefix sign 1',
                'lon': u'n1 05 30.6',
                'lat': u'e035 51 20',
                '#fid': 7L,
                '#geometry': 'POINT(1.09183333 35.85555556)',
                },
            8L: {
                'id': u'5',
                'description': u'Prefix sign 2',
                'lon': u'N1 05 30.6',
                'lat': u'E035 51 20',
                '#fid': 8L,
                '#geometry': 'POINT(1.09183333 35.85555556)',
                },
            9L: {
                'id': u'6',
                'description': u'Prefix sign 3',
                'lon': u'N 1 05 30.6',
                'lat': u'E 035 51 20',
                '#fid': 9L,
                '#geometry': 'POINT(1.09183333 35.85555556)',
                },
            10L: {
                'id': u'7',
                'description': u'Prefix sign 4',
                'lon': u'S1 05 30.6',
                'lat': u'W035 51 20',
                '#fid': 10L,
                '#geometry': 'POINT(-1.09183333 -35.85555556)',
                },
            11L: {
                'id': u'8',
                'description': u'Prefix sign 5',
                'lon': u'+1 05 30.6',
                'lat': u'+035 51 20',
                '#fid': 11L,
                '#geometry': 'POINT(1.09183333 35.85555556)',
                },
            12L: {
                'id': u'9',
                'description': u'Prefix sign 6',
                'lon': u'-1 05 30.6',
                'lat': u'-035 51 20',
                '#fid': 12L,
                '#geometry': 'POINT(-1.09183333 -35.85555556)',
                },
            14L: {
                'id': u'10',
                'description': u'Postfix sign 1',
                'lon': u'1 05 30.6n',
                'lat': u'035 51 20e',
                '#fid': 14L,
                '#geometry': 'POINT(1.09183333 35.85555556)',
                },
            15L: {
                'id': u'11',
                'description': u'Postfix sign 2',
                'lon': u'1 05 30.6N',
                'lat': u'035 51 20E',
                '#fid': 15L,
                '#geometry': 'POINT(1.09183333 35.85555556)',
                },
            16L: {
                'id': u'12',
                'description': u'Postfix sign 3',
                'lon': u'1 05 30.6 N',
                'lat': u'035 51 20 E',
                '#fid': 16L,
                '#geometry': 'POINT(1.09183333 35.85555556)',
                },
            17L: {
                'id': u'13',
                'description': u'Postfix sign 4',
                'lon': u'1 05 30.6S',
                'lat': u'035 51 20W',
                '#fid': 17L,
                '#geometry': 'POINT(-1.09183333 -35.85555556)',
                },
            18L: {
                'id': u'14',
                'description': u'Postfix sign 5',
                'lon': u'1 05 30.6+',
                'lat': u'035 51 20+',
                '#fid': 18L,
                '#geometry': 'POINT(1.09183333 35.85555556)',
                },
            19L: {
                'id': u'15',
                'description': u'Postfix sign 6',
                'lon': u'1 05 30.6-',
                'lat': u'035 51 20-',
                '#fid': 19L,
                '#geometry': 'POINT(-1.09183333 -35.85555556)',
                },
            21L: {
                'id': u'16',
                'description': u'Leading and trailing blanks 1',
                'lon': u'   1 05 30.6',
                'lat': u'035 51 20   ',
                '#fid': 21L,
                '#geometry': 'POINT(1.09183333 35.85555556)',
                },
            22L: {
                'id': u'17',
                'description': u'Leading and trailing blanks 2',
                'lon': u' N  1 05 30.6',
                'lat': u'035 51 20 E  ',
                '#fid': 22L,
                '#geometry': 'POINT(1.09183333 35.85555556)',
                },
            24L: {
                'id': u'18',
                'description': u'Alternative characters for D,M,S',
                'lon': u'1d05m30.6s S',
                'lat': u"35d51'20",
                '#fid': 24L,
                '#geometry': 'POINT(-1.09183333 35.85555556)',
                },
            25L: {
                'id': u'19',
                'description': u'Degrees/minutes format',
                'lon': u'1 05.23',
                'lat': u'4 55.03',
                '#fid': 25L,
                '#geometry': 'POINT(1.08716667 4.91716667)',
                },
            }
        wanted['log']=[
            u'Errors in file file',
            u'5 records discarded due to invalid geometry definitions',
            u'The following lines were not loaded into QGIS due to errors:',
            u'Invalid X or Y fields at line 27',
            u'Invalid X or Y fields at line 28',
            u'Invalid X or Y fields at line 29',
            u'Invalid X or Y fields at line 30',
            u'Invalid X or Y fields at line 31',
            ]
        runTest(description,wanted,filename,requests,**params)


    def test_014_decimal_point(self):
        description='Reading degrees/minutes/seconds angles'
        filename='testdp.csv'
        params={'yField': 'geom_y', 'xField': 'geom_x', 'type': 'csv', 'delimiter': ';', 'decimalPoint': ','}
        requests=None
        if rebuildTests:
            createTest(description,filename,requests,**params)
            assert False,"Set rebuildTests to False to run delimited text tests"
        wanted={}
        wanted['uri']=u'file://file?yField=geom_y&xField=geom_x&type=csv&delimiter=;&decimalPoint=,'
        wanted['data']={
            2L: {
                'id': u'1',
                'description': u'Comma as decimal point 1',
                'geom_x': u'10',
                'geom_y': u'20',
                'other': u'30',
                'text field': u'Field with , in it',
                '#fid': 2L,
                '#geometry': 'POINT(10.0 20.0)',
                },
            3L: {
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
        wanted['log']=[
            ]
        runTest(description,wanted,filename,requests,**params)


    def test_015_regular_expression_1(self):
        description='Parsing regular expression delimiter'
        filename='testre.txt'
        params={'geomType': 'none', 'trimFields': 'Y', 'delimiter': 'RE(?:GEXP)?', 'type': 'regexp'}
        requests=None
        if rebuildTests:
            createTest(description,filename,requests,**params)
            assert False,"Set rebuildTests to False to run delimited text tests"
        wanted={}
        wanted['uri']=u'file://file?geomType=none&trimFields=Y&delimiter=RE(?:GEXP)?&type=regexp'
        wanted['data']={
            2L: {
                'id': u'1',
                'description': u'Basic regular expression test',
                'data': u'data1',
                'info': u'info',
                '#fid': 2L,
                '#geometry': 'None',
                },
            3L: {
                'id': u'2',
                'description': u'Basic regular expression test 2',
                'data': u'data2',
                'info': u'info2',
                '#fid': 3L,
                '#geometry': 'None',
                },
            }
        wanted['log']=[
            ]
        runTest(description,wanted,filename,requests,**params)


    def test_016_regular_expression_2(self):
        description='Parsing regular expression delimiter with capture groups'
        filename='testre.txt'
        params={'geomType': 'none', 'trimFields': 'Y', 'delimiter': '(RE)(GEXP)?', 'type': 'regexp'}
        requests=None
        if rebuildTests:
            createTest(description,filename,requests,**params)
            assert False,"Set rebuildTests to False to run delimited text tests"
        wanted={}
        wanted['uri']=u'file://file?geomType=none&trimFields=Y&delimiter=(RE)(GEXP)?&type=regexp'
        wanted['data']={
            2L: {
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
            3L: {
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
        wanted['log']=[
            ]
        runTest(description,wanted,filename,requests,**params)


    def test_017_regular_expression_3(self):
        description='Parsing anchored regular expression'
        filename='testre2.txt'
        params={'geomType': 'none', 'trimFields': 'Y', 'delimiter': '^(.{5})(.{30})(.{5,})', 'type': 'regexp'}
        requests=None
        if rebuildTests:
            createTest(description,filename,requests,**params)
            assert False,"Set rebuildTests to False to run delimited text tests"
        wanted={}
        wanted['uri']=u'file://file?geomType=none&trimFields=Y&delimiter=^(.{5})(.{30})(.{5,})&type=regexp'
        wanted['data']={
            2L: {
                'id': u'1',
                'description': u'Anchored regexp',
                'information': u'Some data',
                '#fid': 2L,
                '#geometry': 'None',
                },
            4L: {
                'id': u'3',
                'description': u'Anchored regexp recovered',
                'information': u'Some data',
                '#fid': 4L,
                '#geometry': 'None',
                },
            }
        wanted['log']=[
            u'Errors in file file',
            u'1 records discarded due to invalid format',
            u'The following lines were not loaded into QGIS due to errors:',
            u'Invalid record format at line 3',
            ]
        runTest(description,wanted,filename,requests,**params)


    def test_017a_regular_expression_4(self):
        description='Parsing zero length re'
        filename='testre3.txt'
        params={'geomType': 'none', 'delimiter': 'x?', 'type': 'regexp'}
        requests=None
        if rebuildTests:
            createTest(description,filename,requests,**params)
            assert False,"Set rebuildTests to False to run delimited text tests"
        wanted={}
        wanted['uri']=u'file://file?geomType=none&delimiter=x?&type=regexp'
        wanted['data']={
            2L: {
                'id': u'f',
                'description': u'i',
                's': u'f',
                'm': u'i',
                'a': u'.',
                'l': u'.',
                'l_1': u'i',
                'field_6': u'l',
                'field_7': u'e',
                '#fid': 2L,
                '#geometry': 'None',
                },
            }
        wanted['log']=[
            ]
        runTest(description,wanted,filename,requests,**params)


    def test_017a_regular_expression_5(self):
        description='Parsing zero length re 2'
        filename='testre3.txt'
        params={'geomType': 'none', 'delimiter': '\\b', 'type': 'regexp'}
        requests=None
        if rebuildTests:
            createTest(description,filename,requests,**params)
            assert False,"Set rebuildTests to False to run delimited text tests"
        wanted={}
        wanted['uri']=u'file://file?geomType=none&delimiter=\\b&type=regexp'
        wanted['data']={
            2L: {
                'id': u'fi',
                'description': u'..',
                'small': u'fi',
                'field_2': u'..',
                'field_3': u'ile',
                '#fid': 2L,
                '#geometry': 'None',
                },
            }
        wanted['log']=[
            ]
        runTest(description,wanted,filename,requests,**params)


    def test_018_utf8_encoded_file(self):
        description='UTF8 encoded file test'
        filename='testutf8.csv'
        params={'geomType': 'none', 'delimiter': '|', 'type': 'csv', 'encoding': 'utf-8'}
        requests=None
        if rebuildTests:
            createTest(description,filename,requests,**params)
            assert False,"Set rebuildTests to False to run delimited text tests"
        wanted={}
        wanted['uri']=u'file://file?geomType=none&delimiter=|&type=csv&encoding=utf-8'
        wanted['data']={
            2L: {
                'id': u'1',
                'description': u'Correctly read UTF8 encoding',
                'name': u'Field has \u0101cc\xe8nt\xe9d text',
                '#fid': 2L,
                '#geometry': 'None',
                },
            }
        wanted['log']=[
            ]
        runTest(description,wanted,filename,requests,**params)


    def test_019_latin1_encoded_file(self):
        description='Latin1 encoded file test'
        filename='testlatin1.csv'
        params={'geomType': 'none', 'delimiter': '|', 'type': 'csv', 'encoding': 'latin1'}
        requests=None
        if rebuildTests:
            createTest(description,filename,requests,**params)
            assert False,"Set rebuildTests to False to run delimited text tests"
        wanted={}
        wanted['uri']=u'file://file?geomType=none&delimiter=|&type=csv&encoding=latin1'
        wanted['data']={
            2L: {
                'id': u'1',
                'description': u'Correctly read latin1 encoding',
                'name': u'This test is \xa9',
                '#fid': 2L,
                '#geometry': 'None',
                },
            }
        wanted['log']=[
            ]
        runTest(description,wanted,filename,requests,**params)


    def test_030_filter_rect_xy(self):
        description='Filter extents on XY layer'
        filename='testextpt.txt'
        params={'yField': 'y', 'delimiter': '|', 'type': 'csv', 'xField': 'x'}
        requests=[
            {'extents': [10, 30, 30, 50]}, 
            {'exact': 1, 'extents': [10, 30, 30, 50]}, 
            {'extents': [110, 130, 130, 150]}]
        if rebuildTests:
            createTest(description,filename,requests,**params)
            assert False,"Set rebuildTests to False to run delimited text tests"
        wanted={}
        wanted['uri']=u'file://file?yField=y&delimiter=|&type=csv&xField=x'
        wanted['data']={
            2L: {
                'id': u'1',
                'description': u'Inside',
                'x': u'15',
                'y': u'35',
                '#fid': 2L,
                '#geometry': 'POINT(15.0 35.0)',
                },
            10L: {
                'id': u'9',
                'description': u'Inside 2',
                'x': u'25',
                'y': u'45',
                '#fid': 10L,
                '#geometry': 'POINT(25.0 45.0)',
                },
            1002L: {
                'id': u'1',
                'description': u'Inside',
                'x': u'15',
                'y': u'35',
                '#fid': 2L,
                '#geometry': 'POINT(15.0 35.0)',
                },
            1010L: {
                'id': u'9',
                'description': u'Inside 2',
                'x': u'25',
                'y': u'45',
                '#fid': 10L,
                '#geometry': 'POINT(25.0 45.0)',
                },
            }
        wanted['log']=[
            ]
        runTest(description,wanted,filename,requests,**params)


    def test_031_filter_rect_wkt(self):
        description='Filter extents on WKT layer'
        filename='testextw.txt'
        params={'delimiter': '|', 'type': 'csv', 'wktField': 'wkt'}
        requests=[
            {'extents': [10, 30, 30, 50]}, 
            {'exact': 1, 'extents': [10, 30, 30, 50]}, 
            {'extents': [110, 130, 130, 150]}]
        if rebuildTests:
            createTest(description,filename,requests,**params)
            assert False,"Set rebuildTests to False to run delimited text tests"
        wanted={}
        wanted['uri']=u'file://file?delimiter=|&type=csv&wktField=wkt'
        wanted['data']={
            2L: {
                'id': u'1',
                'description': u'Inside',
                '#fid': 2L,
                '#geometry': 'LINESTRING(12.0 32.0, 28.0 48.0)',
                },
            4L: {
                'id': u'3',
                'description': u'Crossing',
                '#fid': 4L,
                '#geometry': 'LINESTRING(5.0 30.0, 30.0 55.0)',
                },
            5L: {
                'id': u'4',
                'description': u'Bounding box overlap',
                '#fid': 5L,
                '#geometry': 'LINESTRING(5.0 30.0, 5.0 55.0, 30.0 55.0)',
                },
            6L: {
                'id': u'5',
                'description': u'Crossing 2',
                '#fid': 6L,
                '#geometry': 'LINESTRING(25.0 35.0, 35.0 35.0)',
                },
            7L: {
                'id': u'6',
                'description': u'Bounding box overlap 2',
                '#fid': 7L,
                '#geometry': 'LINESTRING(28.0 29.0, 31.0 29.0, 31.0 33.0)',
                },
            1002L: {
                'id': u'1',
                'description': u'Inside',
                '#fid': 2L,
                '#geometry': 'LINESTRING(12.0 32.0, 28.0 48.0)',
                },
            1004L: {
                'id': u'3',
                'description': u'Crossing',
                '#fid': 4L,
                '#geometry': 'LINESTRING(5.0 30.0, 30.0 55.0)',
                },
            1006L: {
                'id': u'5',
                'description': u'Crossing 2',
                '#fid': 6L,
                '#geometry': 'LINESTRING(25.0 35.0, 35.0 35.0)',
                },
            }
        wanted['log']=[
            ]
        runTest(description,wanted,filename,requests,**params)


    def test_032_filter_fid(self):
        description='Filter on feature id'
        filename='test.csv'
        params={'geomType': 'none', 'type': 'csv'}
        requests=[
            {'fid': 3}, 
            {'fid': 9}, 
            {'fid': 5}, 
            {'fid': 7}, 
            {'fid': 3}]
        if rebuildTests:
            createTest(description,filename,requests,**params)
            assert False,"Set rebuildTests to False to run delimited text tests"
        wanted={}
        wanted['uri']=u'file://file?geomType=none&type=csv'
        wanted['data']={
            3L: {
                'id': u'2',
                'description': u'Quoted field',
                'data': u'Quoted data',
                'info': u'Unquoted',
                'field_5': u'',
                '#fid': 3L,
                '#geometry': 'None',
                },
            1009L: {
                'id': u'5',
                'description': u'Extra fields',
                'data': u'data',
                'info': u'info',
                'field_5': u'message',
                '#fid': 9L,
                '#geometry': 'None',
                },
            2005L: {
                'id': u'4',
                'description': u'Quoted newlines',
                'data': u'Line 1\nLine 2\n\nLine 4',
                'info': u'No data',
                'field_5': u'',
                '#fid': 5L,
                '#geometry': 'None',
                },
            4003L: {
                'id': u'2',
                'description': u'Quoted field',
                'data': u'Quoted data',
                'info': u'Unquoted',
                'field_5': u'',
                '#fid': 3L,
                '#geometry': 'None',
                },
            }
        wanted['log']=[
            ]
        runTest(description,wanted,filename,requests,**params)


    def test_033_filter_attributes(self):
        description='Filter on attributes'
        filename='test.csv'
        params={'geomType': 'none', 'type': 'csv'}
        requests=[
            {'attributes': [1, 3]}, 
            {'fid': 9}, 
            {'attributes': [1, 3], 'fid': 9}, 
            {'attributes': [3, 1], 'fid': 9}, 
            {'attributes': [1, 3, 7], 'fid': 9}, 
            {'attributes': [], 'fid': 9}]
        if rebuildTests:
            createTest(description,filename,requests,**params)
            assert False,"Set rebuildTests to False to run delimited text tests"
        wanted={}
        wanted['uri']=u'file://file?geomType=none&type=csv'
        wanted['data']={
            2L: {
                'id': u'',
                'description': u'Basic unquoted record',
                'data': u'',
                'info': u'Some info',
                'field_5': u'',
                '#fid': 2L,
                '#geometry': 'None',
                },
            3L: {
                'id': u'',
                'description': u'Quoted field',
                'data': u'',
                'info': u'Unquoted',
                'field_5': u'',
                '#fid': 3L,
                '#geometry': 'None',
                },
            4L: {
                'id': u'',
                'description': u'Escaped quotes',
                'data': u'',
                'info': u'Unquoted',
                'field_5': u'',
                '#fid': 4L,
                '#geometry': 'None',
                },
            5L: {
                'id': u'',
                'description': u'Quoted newlines',
                'data': u'',
                'info': u'No data',
                'field_5': u'',
                '#fid': 5L,
                '#geometry': 'None',
                },
            9L: {
                'id': u'',
                'description': u'Extra fields',
                'data': u'',
                'info': u'info',
                'field_5': u'',
                '#fid': 9L,
                '#geometry': 'None',
                },
            10L: {
                'id': u'',
                'description': u'Missing fields',
                'data': u'',
                'info': u'',
                'field_5': u'',
                '#fid': 10L,
                '#geometry': 'None',
                },
            1009L: {
                'id': u'5',
                'description': u'Extra fields',
                'data': u'data',
                'info': u'info',
                'field_5': u'message',
                '#fid': 9L,
                '#geometry': 'None',
                },
            2009L: {
                'id': u'',
                'description': u'Extra fields',
                'data': u'',
                'info': u'info',
                'field_5': u'',
                '#fid': 9L,
                '#geometry': 'None',
                },
            3009L: {
                'id': u'',
                'description': u'Extra fields',
                'data': u'',
                'info': u'info',
                'field_5': u'',
                '#fid': 9L,
                '#geometry': 'None',
                },
            4009L: {
                'id': u'',
                'description': u'Extra fields',
                'data': u'',
                'info': u'info',
                'field_5': u'',
                '#fid': 9L,
                '#geometry': 'None',
                },
            5009L: {
                'id': u'',
                'description': u'',
                'data': u'',
                'info': u'',
                'field_5': u'',
                '#fid': 9L,
                '#geometry': 'None',
                },
            }
        wanted['log']=[
            ]
        runTest(description,wanted,filename,requests,**params)

#END

if __name__ == '__main__':
    unittest.main()
