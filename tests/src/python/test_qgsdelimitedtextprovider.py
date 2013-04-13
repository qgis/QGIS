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

printTests = True


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
tolerance = 0.00001 # Tolerance for coordinate comparisons in checkWktEqual

# Thought we could connect to messageReceived signal but doesn't seem to be available
# in python :-(  Not sure why?

class MessageLogger( QObject ):

    def __init__( self, tag=None ):
        QObject.__init__(self)
        self.log=[];
        self.tag = tag

    def __enter__( self ):
        # QgsMessageLog.instance().messageReceived.connect( self.logMessage )
        pass

    def __exit__( self ):
        # QgsMessageLog.instance().emitMessage.disconnect( self.logMessage )
        pass
        
    def logMessage( self, msg, tag, level ):
        if tag == self.tag  or not self.tag:
            self.log.append(msg)

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

        id = fielddata[fields[0]]
        data[id]=fielddata
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
    # with MessageLogger('DelimitedText') as logger:
    if True:
        layer = QgsVectorLayer(urlstr,'test','delimitedtext')
        fields = []
        data = {}
        log=[]
        if layer.isValid():
            fields,data = layerData(layer)
        return dict( fields=fields, data=data, log=log)

def sortKey( id ):
    return re.sub(r'^(\d*)',lambda x: '{0:05}'.format(int(x.group(0) or 0)),id)

def createTest(  name, description, filename, **params ):
    # Routine to write a new test for a file.  Need to check the output is right
    # first of course!
    result = delimitedTextData( filename, **params )
    print
    print "    def test_{0}(self):".format(name)
    print "        description={0}".format(repr(description))
    print "        filename={0}".format(repr(filename))
    print "        params={0}".format(repr(params))
    print "        if printTests: createTest({0},description,filename,**params)".format(repr(name))
    
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
    p1=re.split(wkt1)
    p2=re.split(wkt2)
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
            print '    ',description,": Passed"
        else:
            print '    ',description,":",difference
            failures.append(description+': '+difference)
    for id in data.keys():
        if id not in wanted:
            msg= "Layer contains unexpected extra data with id: \"{0}\"".format(id)
            print '    ',msg
            failures.append(msg)
            break
    assert len(failures) == 0,"\n".join(failures)
    for l in log:
        if l in log_wanted:
            log_wanted.remove(l)
            log.remove(l)
    for l in log_wanted:
        print '    ','Missing log message:',l
    for l in log:
        print '    ','Extra log message:',l
    assert len(log_wanted) == 0, "Missing log messages:\n"+"\n".join(log_wanted)
    assert len(log) == 0, "Extra log messages:\n"+"\n".join(log)

class TestQgsDelimitedTextProvider(TestCase):

    def test_001_ProviderDefined( self ):
        registry=QgsProviderRegistry.instance()
        metadata = registry.providerMetadata('delimitedtext')
        assert metadata != None, "Delimited text provider is not installed"

    def test_002_LoadCSVFile(self):
        description='CSV file parsing'
        filename='test.csv'
        params={'geomType': 'none', 'type': 'csv'}
        if printTests: createTest('002_LoadCSVFile',description,filename,**params)
        wanted={
            u'1': {
                'id': u'1',
                'description': u'Basic unquoted record',
                'data': u'Some data',
                'info': u'Some info',
                '#geometry': 'None',
                },
            u'2': {
                'id': u'2',
                'description': u'Quoted field',
                'data': u'Quoted data',
                'info': u'Unquoted',
                '#geometry': 'None',
                },
            u'3': {
                'id': u'3',
                'description': u'Escaped quotes',
                'data': u'Quoted "citation" data',
                'info': u'Unquoted',
                '#geometry': 'None',
                },
            u'4': {
                'id': u'4',
                'description': u'Quoted newlines',
                'data': u'Line 1\nLine 2\n\nLine 4',
                'info': u'No data',
                '#geometry': 'None',
                },
            u'5': {
                'id': u'5',
                'description': u'Extra fields',
                'data': u'data',
                'info': u'info',
                '#geometry': 'None',
                },
            u'6': {
                'id': u'6',
                'description': u'Missing fields',
                'data': u'',
                'info': u'',
                '#geometry': 'None',
                },
            }
        log_wanted=[
            ]
        runTest(description,wanted,log_wanted,filename,**params)

    def test_003_LoadWhitespace(self):
        description='Whitespace file parsing'
        filename='test.space'
        params={'geomType': 'none', 'type': 'whitespace'}
        if printTests: createTest('003_LoadWhitespace',description,filename,**params)
        wanted={
            u'1': {
                'id': u'1',
                'description': u'Simple_whitespace_file',
                'data': u'data1',
                'info': u'info1',
                '#geometry': 'None',
                },
            u'2': {
                'id': u'2',
                'description': u'Whitespace_at_start_of_line',
                'data': u'data2',
                'info': u'info2',
                '#geometry': 'None',
                },
            u'3': {
                'id': u'3',
                'description': u'Tab_whitespace',
                'data': u'data3',
                'info': u'info3',
                '#geometry': 'None',
                },
            u'4': {
                'id': u'4',
                'description': u'Multiple_whitespace_characters',
                'data': u'data4',
                'info': u'info4',
                '#geometry': 'None',
                },
            u'5': {
                'id': u'5',
                'description': u'Extra_fields',
                'data': u'data5',
                'info': u'info5',
                '#geometry': 'None',
                },
            u'6': {
                'id': u'6',
                'description': u'Missing_fields',
                'data': u'',
                'info': u'',
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
        if printTests: createTest('004_quote_escape',description,filename,**params)
        wanted={
            u'1': {
                'id': u'1',
                'description': u'Using pipe delimiter',
                'data': u'data 1',
                'info': u'info 1',
                '#geometry': 'None',
                },
            u'2': {
                'id': u'2',
                'description': u'Using backslash escape on pipe',
                'data': u'data 2 | piped',
                'info': u'info2',
                '#geometry': 'None',
                },
            u'3': {
                'id': u'3',
                'description': u'Backslash escaped newline',
                'data': u'data3 \nline2 \nline3',
                'info': u'info3',
                '#geometry': 'None',
                },
            u'4': {
                'id': u'4',
                'description': u'Empty field',
                'data': u'',
                'info': u'info4',
                '#geometry': 'None',
                },
            u'5': {
                'id': u'5',
                'description': u'Quoted field',
                'data': u'More | piped data',
                'info': u'info5',
                '#geometry': 'None',
                },
            u'6': {
                'id': u'6',
                'description': u'Escaped quote',
                'data': u'Field "citation" ',
                'info': u'info6',
                '#geometry': 'None',
                },
            u'7': {
                'id': u'7',
                'description': u'Missing fields',
                'data': u'',
                'info': u'',
                '#geometry': 'None',
                },
            u'8': {
                'id': u'8',
                'description': u'Extra fields',
                'data': u'data8',
                'info': u'info8',
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
        if printTests: createTest('005_multiple_quote',description,filename,**params)
        wanted={
            u'1': {
                'id': u'1',
                'description': u'Multiple quotes 1',
                'data': u'Quoted,data1',
                'info': u'info1',
                '#geometry': 'None',
                },
            u'2': {
                'id': u'2',
                'description': u'Multiple quotes 2',
                'data': u'Quoted,data2',
                'info': u'info2',
                '#geometry': 'None',
                },
            u'3': {
                'id': u'3',
                'description': u'Leading and following whitespace',
                'data': u'Quoted, data3',
                'info': u'info3',
                '#geometry': 'None',
                },
            u'4': {
                'id': u'4',
                'description': u'Embedded quotes 1',
                'data': u'Quoted \'\'"\'\' data4',
                'info': u'info4',
                '#geometry': 'None',
                },
            u'5': {
                'id': u'5',
                'description': u'Embedded quotes 2',
                'data': u'Quoted \'""\' data5',
                'info': u'info5',
                '#geometry': 'None',
                },
            u'9': {
                'id': u'9',
                'description': u'Final record',
                'data': u'date9',
                'info': u'info9',
                '#geometry': 'None',
                },
            }
        log_wanted=[
            ]
        runTest(description,wanted,log_wanted,filename,**params)


if __name__ == '__main__':
    unittest.main()
