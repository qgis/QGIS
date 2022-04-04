# -*- coding: utf-8 -*-

"""
***************************************************************************
    test_qgsdelimitedtextprovider_wanted.py
    ---------------------
    Date                 : May 2013
    Copyright            : (C) 2013 by Chris Crook
    Email                : ccrook at linz dot govt dot nz
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Chris Crook'
__date__ = 'May 2013'
__copyright__ = '(C) 2013, Chris Crook'


def test_002_load_csv_file():
    wanted = {}
    wanted['uri'] = 'file://test.csv?geomType=none&type=csv'
    wanted['fieldTypes'] = ['integer', 'text', 'text', 'text', 'text']
    wanted['geometryType'] = 4
    wanted['data'] = {
        2: {
            'id': '1',
            'description': 'Basic unquoted record',
            'data': 'Some data',
            'info': 'Some info',
            'field_5': 'NULL',
            '#fid': 2,
            '#geometry': 'None',
        },
        3: {
            'id': '2',
            'description': 'Quoted field',
            'data': 'Quoted data',
            'info': 'Unquoted',
            'field_5': 'NULL',
            '#fid': 3,
            '#geometry': 'None',
        },
        4: {
            'id': '3',
            'description': 'Escaped quotes',
            'data': 'Quoted "citation" data',
            'info': 'Unquoted',
            'field_5': 'NULL',
            '#fid': 4,
            '#geometry': 'None',
        },
        5: {
            'id': '4',
            'description': 'Quoted newlines',
            'data': 'Line 1\nLine 2\n\nLine 4',
            'info': 'No data',
            'field_5': 'NULL',
            '#fid': 5,
            '#geometry': 'None',
        },
        9: {
            'id': '5',
            'description': 'Extra fields',
            'data': 'data',
            'info': 'info',
            'field_5': 'message',
            '#fid': 9,
            '#geometry': 'None',
        },
        10: {
            'id': '6',
            'description': 'Missing fields',
            'data': 'NULL',
            'info': 'NULL',
            'field_5': 'NULL',
            '#fid': 10,
            '#geometry': 'None',
        },
    }
    wanted['log'] = []
    return wanted


def test_003_field_naming():
    wanted = {}
    wanted['uri'] = 'file://testfields.csv?geomType=none&type=csv'
    wanted['fieldTypes'] = ['integer', 'text', 'text', 'text', 'text', 'text', 'text', 'text', 'text', 'text', 'text', 'text']
    wanted['geometryType'] = 4
    wanted['data'] = {
        2: {
            'id': '2',
            'description': 'Generation of field names',
            'data': 'Some data',
            'field_4': 'Some info',
            'data_2': 'NULL',
            '28': 'NULL',
            '24.5': 'NULL',
            'field_3_1': 'NULL',
            'data_1': 'NULL',
            'field_10': 'NULL',
            'field_11': 'NULL',
            'field_12': 'last data',
            '#fid': 2,
            '#geometry': 'None',
        },
    }
    wanted['log'] = []
    return wanted


def test_004_max_fields():
    wanted = {}
    wanted['uri'] = 'file://testfields.csv?geomType=none&maxFields=7&type=csv'
    wanted['fieldTypes'] = ['integer', 'text', 'text', 'text', 'text', 'text', 'text']
    wanted['geometryType'] = 4
    wanted['data'] = {
        2: {
            'id': '2',
            'description': 'Generation of field names',
            'data': 'Some data',
            'field_4': 'Some info',
            'data_1': 'NULL',
            '28': 'NULL',
            '24.5': 'NULL',
            '#fid': 2,
            '#geometry': 'None',
        },
    }
    wanted['log'] = []
    return wanted


def test_005_load_whitespace():
    wanted = {}
    wanted['uri'] = 'file://test.space?geomType=none&type=whitespace'
    wanted['fieldTypes'] = ['integer', 'text', 'text', 'text', 'text', 'text']
    wanted['geometryType'] = 4
    wanted['data'] = {
        2: {
            'id': '1',
            'description': 'Simple_whitespace_file',
            'data': 'data1',
            'info': 'info1',
            'field_5': 'NULL',
            'field_6': 'NULL',
            '#fid': 2,
            '#geometry': 'None',
        },
        3: {
            'id': '2',
            'description': 'Whitespace_at_start_of_line',
            'data': 'data2',
            'info': 'info2',
            'field_5': 'NULL',
            'field_6': 'NULL',
            '#fid': 3,
            '#geometry': 'None',
        },
        4: {
            'id': '3',
            'description': 'Tab_whitespace',
            'data': 'data3',
            'info': 'info3',
            'field_5': 'NULL',
            'field_6': 'NULL',
            '#fid': 4,
            '#geometry': 'None',
        },
        5: {
            'id': '4',
            'description': 'Multiple_whitespace_characters',
            'data': 'data4',
            'info': 'info4',
            'field_5': 'NULL',
            'field_6': 'NULL',
            '#fid': 5,
            '#geometry': 'None',
        },
        6: {
            'id': '5',
            'description': 'Extra_fields',
            'data': 'data5',
            'info': 'info5',
            'field_5': 'message5',
            'field_6': 'rubbish5',
            '#fid': 6,
            '#geometry': 'None',
        },
        7: {
            'id': '6',
            'description': 'Missing_fields',
            'data': 'NULL',
            'info': 'NULL',
            'field_5': 'NULL',
            'field_6': 'NULL',
            '#fid': 7,
            '#geometry': 'None',
        },
    }
    wanted['log'] = []
    return wanted


def test_006_quote_escape():
    wanted = {}
    wanted['uri'] = 'file://test.pipe?geomType=none&quote="&delimiter=|&escape=\\'
    wanted['fieldTypes'] = ['integer', 'text', 'text', 'text', 'text', 'text']
    wanted['geometryType'] = 4
    wanted['data'] = {
        2: {
            'id': '1',
            'description': 'Using pipe delimiter',
            'data': 'data 1',
            'info': 'info 1',
            'field_5': 'NULL',
            'field_6': 'NULL',
            '#fid': 2,
            '#geometry': 'None',
        },
        3: {
            'id': '2',
            'description': 'Using backslash escape on pipe',
            'data': 'data 2 | piped',
            'info': 'info2',
            'field_5': 'NULL',
            'field_6': 'NULL',
            '#fid': 3,
            '#geometry': 'None',
        },
        4: {
            'id': '3',
            'description': 'Backslash escaped newline',
            'data': 'data3 \nline2 \nline3',
            'info': 'info3',
            'field_5': 'NULL',
            'field_6': 'NULL',
            '#fid': 4,
            '#geometry': 'None',
        },
        7: {
            'id': '4',
            'description': 'Empty field',
            'data': 'NULL',
            'info': 'info4',
            'field_5': 'NULL',
            'field_6': 'NULL',
            '#fid': 7,
            '#geometry': 'None',
        },
        8: {
            'id': '5',
            'description': 'Quoted field',
            'data': 'More | piped data',
            'info': 'info5',
            'field_5': 'NULL',
            'field_6': 'NULL',
            '#fid': 8,
            '#geometry': 'None',
        },
        9: {
            'id': '6',
            'description': 'Escaped quote',
            'data': 'Field "citation" ',
            'info': 'info6',
            'field_5': 'NULL',
            'field_6': 'NULL',
            '#fid': 9,
            '#geometry': 'None',
        },
        10: {
            'id': '7',
            'description': 'Missing fields',
            'data': 'NULL',
            'info': 'NULL',
            'field_5': 'NULL',
            'field_6': 'NULL',
            '#fid': 10,
            '#geometry': 'None',
        },
        11: {
            'id': '8',
            'description': 'Extra fields',
            'data': 'data8',
            'info': 'info8',
            'field_5': 'message8',
            'field_6': 'more',
            '#fid': 11,
            '#geometry': 'None',
        },
    }
    wanted['log'] = []
    return wanted


def test_007_multiple_quote():
    wanted = {}
    wanted['uri'] = 'file://test.quote?geomType=none&quote=\'"&type=csv&escape="\''
    wanted['fieldTypes'] = ['integer', 'text', 'text', 'text']
    wanted['geometryType'] = 4
    wanted['data'] = {
        2: {
            'id': '1',
            'description': 'Multiple quotes 1',
            'data': 'Quoted,data1',
            'info': 'info1',
            '#fid': 2,
            '#geometry': 'None',
        },
        3: {
            'id': '2',
            'description': 'Multiple quotes 2',
            'data': 'Quoted,data2',
            'info': 'info2',
            '#fid': 3,
            '#geometry': 'None',
        },
        4: {
            'id': '3',
            'description': 'Leading and following whitespace',
            'data': 'Quoted, data3',
            'info': 'info3',
            '#fid': 4,
            '#geometry': 'None',
        },
        5: {
            'id': '4',
            'description': 'Embedded quotes 1',
            'data': 'Quoted \'\'"\'\' data4',
            'info': 'info4',
            '#fid': 5,
            '#geometry': 'None',
        },
        6: {
            'id': '5',
            'description': 'Embedded quotes 2',
            'data': 'Quoted \'""\' data5',
            'info': 'info5',
            '#fid': 6,
            '#geometry': 'None',
        },
        10: {
            'id': '9',
            'description': 'Final record',
            'data': 'date9',
            'info': 'info9',
            '#fid': 10,
            '#geometry': 'None',
        },
    }
    wanted['log'] = [
        'Errors in file test.quote',
        '3 record(s) discarded due to invalid format',
        'The following lines were not loaded into QGIS due to errors:',
        'Invalid record format at line 7',
        'Invalid record format at line 8',
        'Invalid record format at line 9',
    ]
    return wanted


def test_008_badly_formed_quotes():
    wanted = {}
    wanted['uri'] = 'file://test.badquote?geomType=none&quote="&type=csv&escape="'
    wanted['fieldTypes'] = ['integer', 'text', 'text', 'text']
    wanted['geometryType'] = 4
    wanted['data'] = {
        4: {
            'id': '3',
            'description': 'Recovered after unclosed quore',
            'data': 'Data ok',
            'info': 'inf3',
            '#fid': 4,
            '#geometry': 'None',
        },
    }
    wanted['log'] = [
        'Errors in file test.badquote',
        '2 record(s) discarded due to invalid format',
        'The following lines were not loaded into QGIS due to errors:',
        'Invalid record format at line 2',
        'Invalid record format at line 5',
    ]
    return wanted


def test_009_skip_lines():
    wanted = {}
    wanted['uri'] = 'file://test2.csv?geomType=none&skipLines=2&type=csv&useHeader=no'
    wanted['fieldTypes'] = ['integer', 'text', 'text']
    wanted['geometryType'] = 4
    wanted['data'] = {
        3: {
            'id': '3',
            'description': 'Less data',
            'field_1': '3',
            'field_2': 'Less data',
            'field_3': 'data3',
            '#fid': 3,
            '#geometry': 'None',
        },
    }
    wanted['log'] = []
    return wanted


def test_010_read_coordinates():
    wanted = {}
    wanted['uri'] = 'file://testpt.csv?yField=geom_y&xField=geom_x&type=csv'
    wanted['fieldTypes'] = ['integer', 'text', 'double', 'double']
    wanted['geometryType'] = 0
    wanted['data'] = {
        2: {
            'id': '1',
            'description': 'Basic point',
            'geom_x': '10.5',
            'geom_y': '20.82',
            '#fid': 2,
            '#geometry': 'Point (10.5 20.82)',
        },
        3: {
            'id': '2',
            'description': 'Integer point',
            'geom_x': '11.0',
            'geom_y': '22.0',
            '#fid': 3,
            '#geometry': 'Point (11 22)',
        },
        5: {
            'id': '4',
            'description': 'Final point',
            'geom_x': '13.0',
            'geom_y': '23.0',
            '#fid': 5,
            '#geometry': 'Point (13 23)',
        },
    }
    wanted['log'] = [
        'Errors in file testpt.csv',
        '1 record(s) discarded due to invalid geometry definitions',
        'The following lines were not loaded into QGIS due to errors:',
        'Invalid X or Y fields at line 4',
    ]
    return wanted


def test_011_read_wkt():
    wanted = {}
    wanted['uri'] = 'file://testwkt.csv?delimiter=|&type=csv&wktField=geom_wkt'
    wanted['fieldTypes'] = ['integer', 'text']
    wanted['geometryType'] = 0
    wanted['data'] = {
        2: {
            'id': '1',
            'description': 'Point wkt',
            '#fid': 2,
            '#geometry': 'Point (10 20)',
        },
        3: {
            'id': '2',
            'description': 'Multipoint wkt',
            '#fid': 3,
            '#geometry': 'MultiPoint ((10 20),(11 21))',
        },
        9: {
            'id': '8',
            'description': 'EWKT prefix',
            '#fid': 9,
            '#geometry': 'Point (10 10)',
        },
        10: {
            'id': '9',
            'description': 'Informix prefix',
            '#fid': 10,
            '#geometry': 'Point (10 10)',
        },
        11: {
            'id': '10',
            'description': 'Measure in point',
            '#fid': 11,
            '#geometry': 'PointM (10 20 30)',
        },
    }
    wanted['log'] = [
        'Errors in file testwkt.csv',
        '1 record(s) discarded due to invalid geometry definitions',
        '10 record(s) discarded due to incompatible geometry types',
        'The following lines were not loaded into QGIS due to errors:',
        'Invalid WKT at line 8',
    ]
    return wanted


def test_012_read_wkt_point():
    wanted = {}
    wanted['uri'] = 'file://testwkt.csv?geomType=point&delimiter=|&type=csv&wktField=geom_wkt'
    wanted['fieldTypes'] = ['integer', 'text']
    wanted['geometryType'] = 0
    wanted['data'] = {
        2: {
            'id': '1',
            'description': 'Point wkt',
            '#fid': 2,
            '#geometry': 'Point (10 20)',
        },
        3: {
            'id': '2',
            'description': 'Multipoint wkt',
            '#fid': 3,
            '#geometry': 'MultiPoint ((10 20),(11 21))',
        },
        9: {
            'id': '8',
            'description': 'EWKT prefix',
            '#fid': 9,
            '#geometry': 'Point (10 10)',
        },
        10: {
            'id': '9',
            'description': 'Informix prefix',
            '#fid': 10,
            '#geometry': 'Point (10 10)',
        },
        11: {
            'id': '10',
            'description': 'Measure in point',
            '#fid': 11,
            '#geometry': 'PointM (10 20 30)',
        },
    }
    wanted['log'] = [
        'Errors in file testwkt.csv',
        '1 record(s) discarded due to invalid geometry definitions',
        '10 record(s) discarded due to incompatible geometry types',
        'The following lines were not loaded into QGIS due to errors:',
        'Invalid WKT at line 8',
    ]
    return wanted


def test_013_read_wkt_line():
    wanted = {}
    wanted['uri'] = 'file://testwkt.csv?geomType=line&delimiter=|&type=csv&wktField=geom_wkt'
    wanted['fieldTypes'] = ['integer', 'text']
    wanted['geometryType'] = 1
    wanted['data'] = {
        4: {
            'id': '3',
            'description': 'Linestring wkt',
            '#fid': 4,
            '#geometry': 'LineString (10 20, 11 21)',
        },
        5: {
            'id': '4',
            'description': 'Multiline string wkt',
            '#fid': 5,
            '#geometry': 'MultiLineString ((10 20, 11 21), (20 30, 21 31))',
        },
        12: {
            'id': '11',
            'description': 'Measure in line',
            '#fid': 12,
            '#geometry': 'LineStringM (10 20 30, 11 21 31)',
        },
        13: {
            'id': '12',
            'description': 'Z in line',
            '#fid': 13,
            '#geometry': 'LineStringZ (10 20 30, 11 21 31)',
        },
        14: {
            'id': '13',
            'description': 'Measure and Z in line',
            '#fid': 14,
            '#geometry': 'LineStringZM (10 20 30 40, 11 21 31 41)',
        },
        15: {
            'id': '14',
            'description': 'CircularString',
            '#fid': 15,
            '#geometry': 'CircularString (268 415, 227 505, 227 406)',
        },
        17: {
            'id': '16',
            'description': 'CompoundCurve',
            '#fid': 17,
            '#geometry': 'CompoundCurve ((5 3, 5 13), CircularString(5 13, 7 15, 9 13), (9 13, 9 3), CircularString(9 3, 7 1, 5 3))',
        },
    }
    wanted['log'] = [
        'Errors in file testwkt.csv',
        '1 record(s) discarded due to invalid geometry definitions',
        '8 record(s) discarded due to incompatible geometry types',
        'The following lines were not loaded into QGIS due to errors:',
        'Invalid WKT at line 8',
    ]
    return wanted


def test_014_read_wkt_polygon():
    wanted = {}
    wanted['uri'] = 'file://testwkt.csv?geomType=polygon&delimiter=|&type=csv&wktField=geom_wkt'
    wanted['fieldTypes'] = ['integer', 'text']
    wanted['geometryType'] = 2
    wanted['data'] = {
        6: {
            'id': '5',
            'description': 'Polygon wkt',
            '#fid': 6,
            '#geometry': 'Polygon ((10 10,10 20,20 20,20 10,10 10),(14 14,14 16,16 16,14 14))',
        },
        7: {
            'id': '6',
            'description': 'MultiPolygon wkt',
            '#fid': 7,
            '#geometry': 'MultiPolygon (((10 10,10 20,20 20,20 10,10 10),(14 14,14 16,16 16,14 14)),((30 30,30 35,35 35,30 30)))',
        },
        16: {
            'id': '15',
            'description': 'CurvePolygon',
            '#fid': 16,
            '#geometry': 'CurvePolygon (CircularString (1 3, 3 5, 4 7, 7 3, 1 3))',
        },
    }
    wanted['log'] = [
        'Errors in file testwkt.csv',
        '1 record(s) discarded due to invalid geometry definitions',
        '12 record(s) discarded due to incompatible geometry types',
        'The following lines were not loaded into QGIS due to errors:',
        'Invalid WKT at line 8',
    ]
    return wanted


def test_015_read_dms_xy():
    wanted = {}
    wanted['uri'] = 'file://testdms.csv?yField=lat&xField=lon&type=csv&xyDms=yes'
    wanted['fieldTypes'] = ['integer', 'text', 'text', 'text']
    wanted['geometryType'] = 0
    wanted['data'] = {
        3: {
            'id': '1',
            'description': 'Basic DMS string',
            'lon': '1 5 30.6',
            'lat': '35 51 20',
            '#fid': 3,
            '#geometry': 'Point (1.09183333 35.85555556)',
        },
        4: {
            'id': '2',
            'description': 'Basic DMS string 2',
            'lon': '1 05 30.6005',
            'lat': '035 51 20',
            '#fid': 4,
            '#geometry': 'Point (1.09183347 35.85555556)',
        },
        5: {
            'id': '3',
            'description': 'Basic DMS string 3',
            'lon': '1 05 30.6',
            'lat': '35 59 9.99',
            '#fid': 5,
            '#geometry': 'Point (1.09183333 35.98610833)',
        },
        7: {
            'id': '4',
            'description': 'Prefix sign 1',
            'lon': 'n1 05 30.6',
            'lat': 'e035 51 20',
            '#fid': 7,
            '#geometry': 'Point (1.09183333 35.85555556)',
        },
        8: {
            'id': '5',
            'description': 'Prefix sign 2',
            'lon': 'N1 05 30.6',
            'lat': 'E035 51 20',
            '#fid': 8,
            '#geometry': 'Point (1.09183333 35.85555556)',
        },
        9: {
            'id': '6',
            'description': 'Prefix sign 3',
            'lon': 'N 1 05 30.6',
            'lat': 'E 035 51 20',
            '#fid': 9,
            '#geometry': 'Point (1.09183333 35.85555556)',
        },
        10: {
            'id': '7',
            'description': 'Prefix sign 4',
            'lon': 'S1 05 30.6',
            'lat': 'W035 51 20',
            '#fid': 10,
            '#geometry': 'Point (-1.09183333 -35.85555556)',
        },
        11: {
            'id': '8',
            'description': 'Prefix sign 5',
            'lon': '+1 05 30.6',
            'lat': '+035 51 20',
            '#fid': 11,
            '#geometry': 'Point (1.09183333 35.85555556)',
        },
        12: {
            'id': '9',
            'description': 'Prefix sign 6',
            'lon': '-1 05 30.6',
            'lat': '-035 51 20',
            '#fid': 12,
            '#geometry': 'Point (-1.09183333 -35.85555556)',
        },
        14: {
            'id': '10',
            'description': 'Postfix sign 1',
            'lon': '1 05 30.6n',
            'lat': '035 51 20e',
            '#fid': 14,
            '#geometry': 'Point (1.09183333 35.85555556)',
        },
        15: {
            'id': '11',
            'description': 'Postfix sign 2',
            'lon': '1 05 30.6N',
            'lat': '035 51 20E',
            '#fid': 15,
            '#geometry': 'Point (1.09183333 35.85555556)',
        },
        16: {
            'id': '12',
            'description': 'Postfix sign 3',
            'lon': '1 05 30.6 N',
            'lat': '035 51 20 E',
            '#fid': 16,
            '#geometry': 'Point (1.09183333 35.85555556)',
        },
        17: {
            'id': '13',
            'description': 'Postfix sign 4',
            'lon': '1 05 30.6S',
            'lat': '035 51 20W',
            '#fid': 17,
            '#geometry': 'Point (-1.09183333 -35.85555556)',
        },
        18: {
            'id': '14',
            'description': 'Postfix sign 5',
            'lon': '1 05 30.6+',
            'lat': '035 51 20+',
            '#fid': 18,
            '#geometry': 'Point (1.09183333 35.85555556)',
        },
        19: {
            'id': '15',
            'description': 'Postfix sign 6',
            'lon': '1 05 30.6-',
            'lat': '035 51 20-',
            '#fid': 19,
            '#geometry': 'Point (-1.09183333 -35.85555556)',
        },
        21: {
            'id': '16',
            'description': 'Leading and trailing blanks 1',
            'lon': '   1 05 30.6',
            'lat': '035 51 20   ',
            '#fid': 21,
            '#geometry': 'Point (1.09183333 35.85555556)',
        },
        22: {
            'id': '17',
            'description': 'Leading and trailing blanks 2',
            'lon': ' N  1 05 30.6',
            'lat': '035 51 20 E  ',
            '#fid': 22,
            '#geometry': 'Point (1.09183333 35.85555556)',
        },
        24: {
            'id': '18',
            'description': 'Alternative characters for D,M,S',
            'lon': '1d05m30.6s S',
            'lat': "35d51'20",
            '#fid': 24,
            '#geometry': 'Point (-1.09183333 35.85555556)',
        },
        25: {
            'id': '19',
            'description': 'Degrees/minutes format',
            'lon': '1 05.23',
            'lat': '4 55.03',
            '#fid': 25,
            '#geometry': 'Point (1.08716667 4.91716667)',
        },
    }
    wanted['log'] = [
        'Errors in file testdms.csv',
        '5 record(s) discarded due to invalid geometry definitions',
        'The following lines were not loaded into QGIS due to errors:',
        'Invalid X or Y fields at line 27',
        'Invalid X or Y fields at line 28',
        'Invalid X or Y fields at line 29',
        'Invalid X or Y fields at line 30',
        'Invalid X or Y fields at line 31',
    ]
    return wanted


def test_016_decimal_point():
    wanted = {}
    wanted['uri'] = 'file://testdp.csv?yField=geom_y&xField=geom_x&type=csv&delimiter=;&decimalPoint=,'
    wanted['fieldTypes'] = ['integer', 'text', 'double', 'double', 'double', 'text']
    wanted['geometryType'] = 0
    wanted['data'] = {
        2: {
            'id': '1',
            'description': 'Comma as decimal point 1',
            'geom_x': '10.0',
            'geom_y': '20.0',
            'other': '30.0',
            'text field': 'Field with , in it',
            '#fid': 2,
            '#geometry': 'Point (10 20)',
        },
        3: {
            'id': '2',
            'description': 'Comma as decimal point 2',
            'geom_x': '12.0',
            'geom_y': '25.003',
            'other': '-38.55',
            'text field': 'Plain text field',
            '#fid': 3,
            '#geometry': 'Point (12 25.003)',
        },
    }
    wanted['log'] = []
    return wanted


def test_017_regular_expression_1():
    wanted = {}
    wanted['uri'] = 'file://testre.txt?geomType=none&trimFields=Y&delimiter=RE(?:GEXP)?&type=regexp'
    wanted['fieldTypes'] = ['integer', 'text', 'text', 'text']
    wanted['geometryType'] = 4
    wanted['data'] = {
        2: {
            'id': '1',
            'description': 'Basic regular expression test',
            'data': 'data1',
            'info': 'info',
            '#fid': 2,
            '#geometry': 'None',
        },
        3: {
            'id': '2',
            'description': 'Basic regular expression test 2',
            'data': 'data2',
            'info': 'info2',
            '#fid': 3,
            '#geometry': 'None',
        },
    }
    wanted['log'] = []
    return wanted


def test_018_regular_expression_2():
    wanted = {}
    wanted['uri'] = 'file://testre.txt?geomType=none&trimFields=Y&delimiter=(RE)((?:GEXP)?)&type=regexp'
    wanted['fieldTypes'] = ['integer', 'text', 'text', 'text', 'text', 'text', 'text', 'text', 'text', 'text']
    wanted['geometryType'] = 4
    wanted['data'] = {
        2: {
            'id': '1',
            'RE': 'RE',
            'GEXP': 'GEXP',
            'description': 'RE',
            'RE_1': 'RE',
            'GEXP_1': 'GEXP',
            'data': 'data1',
            'RE_2': 'RE',
            'GEXP_2': 'GEXP',
            'info': 'info',
            '#fid': 2,
            '#geometry': 'None',
        },
        3: {
            'id': '2',
            'RE': 'RE',
            'GEXP': 'GEXP',
            'description': 'RE',
            'RE_1': 'RE',
            'GEXP_1': '',
            'data': 'data2',
            'RE_2': 'RE',
            'GEXP_2': '',
            'info': 'info2',
            '#fid': 3,
            '#geometry': 'None',
        },
    }
    wanted['log'] = []
    return wanted


def test_019_regular_expression_3():
    wanted = {}
    wanted['uri'] = 'file://testre2.txt?geomType=none&trimFields=Y&delimiter=^(.{5})(.{30})(.{5,})&type=regexp'
    wanted['fieldTypes'] = ['integer', 'text', 'text']
    wanted['geometryType'] = 4
    wanted['data'] = {
        2: {
            'id': '1',
            'description': 'Anchored regexp',
            'information': 'Some data',
            '#fid': 2,
            '#geometry': 'None',
        },
        4: {
            'id': '3',
            'description': 'Anchored regexp recovered',
            'information': 'Some data',
            '#fid': 4,
            '#geometry': 'None',
        },
    }
    wanted['log'] = [
        'Errors in file testre2.txt',
        '1 record(s) discarded due to invalid format',
        'The following lines were not loaded into QGIS due to errors:',
        'Invalid record format at line 3',
    ]
    return wanted


def test_020_regular_expression_4():
    wanted = {}
    wanted['uri'] = 'file://testre3.txt?geomType=none&delimiter=x?&type=regexp'
    wanted['fieldTypes'] = ['text', 'text', 'text', 'text', 'text', 'text', 'text']
    wanted['geometryType'] = 4
    wanted['data'] = {
        2: {
            'id': 'g',
            'description': 'i',
            's': 'g',
            'm': 'i',
            'a': '.',
            'l': '.',
            'l_1': 'i',
            'field_6': 'l',
            'field_7': 'e',
            '#fid': 2,
            '#geometry': 'None',
        },
    }
    wanted['log'] = []
    return wanted


def test_021_regular_expression_5():
    wanted = {}
    wanted['uri'] = 'file://testre3.txt?geomType=none&delimiter=\\b&type=regexp'
    wanted['fieldTypes'] = ['text', 'text', 'text']
    wanted['geometryType'] = 4
    wanted['data'] = {
        2: {
            'id': 'gi',
            'description': '..',
            'small': 'gi',
            'field_2': '..',
            'field_3': 'ile',
            '#fid': 2,
            '#geometry': 'None',
        },
    }
    wanted['log'] = []
    return wanted


def test_022_utf8_encoded_file():
    wanted = {}
    wanted['uri'] = 'file://testutf8.csv?geomType=none&delimiter=|&type=csv&encoding=utf-8'
    wanted['fieldTypes'] = ['integer', 'text', 'text']
    wanted['geometryType'] = 4
    wanted['data'] = {
        2: {
            'id': '2',
            'description': 'Correctly read UTF8 encoding',
            'name': 'Field has \u0101cc\xe8nt\xe9d text',
            '#fid': 2,
            '#geometry': 'None',
        },
    }
    wanted['log'] = []
    return wanted


def test_023_latin1_encoded_file():
    wanted = {}
    wanted['uri'] = 'file://testlatin1.csv?geomType=none&delimiter=|&type=csv&encoding=latin1'
    wanted['fieldTypes'] = ['integer', 'text', 'text']
    wanted['geometryType'] = 4
    wanted['data'] = {
        2: {
            'id': '2',
            'description': 'Correctly read latin1 encoding',
            'name': 'This test is \xa9',
            '#fid': 2,
            '#geometry': 'None',
        },
    }
    wanted['log'] = []
    return wanted


def test_024_filter_rect_xy():
    wanted = {}
    wanted['uri'] = 'file://testextpt.txt?yField=y&delimiter=|&type=csv&xField=x'
    wanted['fieldTypes'] = ['integer', 'text', 'integer', 'integer']
    wanted['geometryType'] = 0
    wanted['data'] = {
        2: {
            'id': '1',
            'description': 'Inside',
            'x': '15',
            'y': '35',
            '#fid': 2,
            '#geometry': 'Point (15 35)',
        },
        10: {
            'id': '9',
            'description': 'Inside 2',
            'x': '25',
            'y': '45',
            '#fid': 10,
            '#geometry': 'Point (25 45)',
        },
        1002: {
            'id': '1',
            'description': 'Inside',
            'x': '15',
            'y': '35',
            '#fid': 2,
            '#geometry': 'Point (15 35)',
        },
        1010: {
            'id': '9',
            'description': 'Inside 2',
            'x': '25',
            'y': '45',
            '#fid': 10,
            '#geometry': 'Point (25 45)',
        },
    }
    wanted['log'] = [
        'Request 2 did not return any data',
    ]
    return wanted


def test_025_filter_rect_wkt():
    wanted = {}
    wanted['uri'] = 'file://testextw.txt?delimiter=|&type=csv&wktField=wkt'
    wanted['fieldTypes'] = ['integer', 'text']
    wanted['geometryType'] = 1
    wanted['data'] = {
        2: {
            'id': '1',
            'description': 'Inside',
            '#fid': 2,
            '#geometry': 'LineString (12 32, 28 48)',
        },
        4: {
            'id': '3',
            'description': 'Crossing',
            '#fid': 4,
            '#geometry': 'LineString (5 30, 30 55)',
        },
        5: {
            'id': '4',
            'description': 'Bounding box overlap',
            '#fid': 5,
            '#geometry': 'LineString (5 30, 5 55, 30 55)',
        },
        6: {
            'id': '5',
            'description': 'Crossing 2',
            '#fid': 6,
            '#geometry': 'LineString (25 35, 35 35)',
        },
        7: {
            'id': '6',
            'description': 'Bounding box overlap 2',
            '#fid': 7,
            '#geometry': 'LineString (28 29, 31 29, 31 33)',
        },
        1002: {
            'id': '1',
            'description': 'Inside',
            '#fid': 2,
            '#geometry': 'LineString (12 32, 28 48)',
        },
        1004: {
            'id': '3',
            'description': 'Crossing',
            '#fid': 4,
            '#geometry': 'LineString (5 30, 30 55)',
        },
        1006: {
            'id': '5',
            'description': 'Crossing 2',
            '#fid': 6,
            '#geometry': 'LineString (25 35, 35 35)',
        },
    }
    wanted['log'] = [
        'Request 2 did not return any data',
    ]
    return wanted


def test_026_filter_fid():
    wanted = {}
    wanted['uri'] = 'file://test.csv?geomType=none&type=csv'
    wanted['fieldTypes'] = ['integer', 'text', 'text', 'text', 'text']
    wanted['geometryType'] = 4
    wanted['data'] = {
        3: {
            'id': '2',
            'description': 'Quoted field',
            'data': 'Quoted data',
            'info': 'Unquoted',
            'field_5': 'NULL',
            '#fid': 3,
            '#geometry': 'None',
        },
        1009: {
            'id': '5',
            'description': 'Extra fields',
            'data': 'data',
            'info': 'info',
            'field_5': 'message',
            '#fid': 9,
            '#geometry': 'None',
        },
        3003: {
            'id': '2',
            'description': 'Quoted field',
            'data': 'Quoted data',
            'info': 'Unquoted',
            'field_5': 'NULL',
            '#fid': 3,
            '#geometry': 'None',
        },
    }
    wanted['log'] = [
        'Request 2 did not return any data',
    ]
    return wanted


def test_027_filter_attributes():
    wanted = {}
    wanted['uri'] = 'file://test.csv?geomType=none&type=csv'
    wanted['fieldTypes'] = ['integer', 'text', 'text', 'text', 'text']
    wanted['geometryType'] = 4
    wanted['data'] = {
        2: {
            'id': 'None',
            'description': 'Basic unquoted record',
            'data': 'None',
            'info': 'Some info',
            'field_5': 'None',
            '#fid': 2,
            '#geometry': 'None',
        },
        3: {
            'id': 'None',
            'description': 'Quoted field',
            'data': 'None',
            'info': 'Unquoted',
            'field_5': 'None',
            '#fid': 3,
            '#geometry': 'None',
        },
        4: {
            'id': 'None',
            'description': 'Escaped quotes',
            'data': 'None',
            'info': 'Unquoted',
            'field_5': 'None',
            '#fid': 4,
            '#geometry': 'None',
        },
        5: {
            'id': 'None',
            'description': 'Quoted newlines',
            'data': 'None',
            'info': 'No data',
            'field_5': 'None',
            '#fid': 5,
            '#geometry': 'None',
        },
        9: {
            'id': 'None',
            'description': 'Extra fields',
            'data': 'None',
            'info': 'info',
            'field_5': 'None',
            '#fid': 9,
            '#geometry': 'None',
        },
        10: {
            'id': 'None',
            'description': 'Missing fields',
            'data': 'None',
            'info': 'NULL',
            'field_5': 'None',
            '#fid': 10,
            '#geometry': 'None',
        },
        1009: {
            'id': '5',
            'description': 'Extra fields',
            'data': 'data',
            'info': 'info',
            'field_5': 'message',
            '#fid': 9,
            '#geometry': 'None',
        },
        2009: {
            'id': 'None',
            'description': 'Extra fields',
            'data': 'None',
            'info': 'info',
            'field_5': 'None',
            '#fid': 9,
            '#geometry': 'None',
        },
        3009: {
            'id': 'None',
            'description': 'Extra fields',
            'data': 'None',
            'info': 'info',
            'field_5': 'None',
            '#fid': 9,
            '#geometry': 'None',
        },
        4009: {
            'id': 'None',
            'description': 'Extra fields',
            'data': 'None',
            'info': 'info',
            'field_5': 'None',
            '#fid': 9,
            '#geometry': 'None',
        },
        5009: {
            'id': 'None',
            'description': 'None',
            'data': 'None',
            'info': 'None',
            'field_5': 'None',
            '#fid': 9,
            '#geometry': 'None',
        },
    }
    wanted['log'] = []
    return wanted


def test_028_substring_test():
    wanted = {}
    wanted['uri'] = 'file://test.csv?geomType=none&type=csv&subset=id%20%25%202%20%3D%201'
    wanted['fieldTypes'] = ['integer', 'text', 'text', 'text', 'text']
    wanted['geometryType'] = 4
    wanted['data'] = {
        2: {
            'id': '1',
            'description': 'Basic unquoted record',
            'data': 'Some data',
            'info': 'Some info',
            'field_5': 'NULL',
            '#fid': 2,
            '#geometry': 'None',
        },
        4: {
            'id': '3',
            'description': 'Escaped quotes',
            'data': 'Quoted "citation" data',
            'info': 'Unquoted',
            'field_5': 'NULL',
            '#fid': 4,
            '#geometry': 'None',
        },
        9: {
            'id': '5',
            'description': 'Extra fields',
            'data': 'data',
            'info': 'info',
            'field_5': 'message',
            '#fid': 9,
            '#geometry': 'None',
        },
    }
    wanted['log'] = []
    return wanted


def test_029_file_watcher():
    wanted = {}
    wanted['uri'] = 'file://file?geomType=none&type=csv&watchFile=yes'
    wanted['fieldTypes'] = ['integer', 'text']
    wanted['geometryType'] = 4
    wanted['data'] = {
        3: {
            'id': '2',
            'description': 'pooh',
            'name': 'pooh',
            '#fid': 3,
            '#geometry': 'None',
        },
        1002: {
            'id': '1',
            'description': 'rabbit',
            'name': 'rabbit',
            '#fid': 2,
            '#geometry': 'None',
        },
        1003: {
            'id': '2',
            'description': 'pooh',
            'name': 'pooh',
            '#fid': 3,
            '#geometry': 'None',
        },
        4003: {
            'id': '2',
            'description': 'pooh',
            'name': 'pooh',
            '#fid': 3,
            '#geometry': 'None',
        },
        5004: {
            'id': '3',
            'description': 'tiger',
            'name': 'tiger',
            '#fid': 4,
            '#geometry': 'None',
        },
        6002: {
            'id': '1',
            'description': 'rabbit',
            'name': 'rabbit',
            '#fid': 2,
            '#geometry': 'None',
        },
        6003: {
            'id': '2',
            'description': 'pooh',
            'name': 'pooh',
            '#fid': 3,
            '#geometry': 'None',
        },
        6004: {
            'id': '3',
            'description': 'tiger',
            'name': 'tiger',
            '#fid': 4,
            '#geometry': 'None',
        },
        9002: {
            'id': '5',
            'description': 'toad',
            'name': 'toad',
            '#fid': 2,
            '#geometry': 'None',
        },
        10002: {
            'id': '5',
            'description': 'toad',
            'name': 'toad',
            '#fid': 2,
            '#geometry': 'None',
        },
        10003: {
            'id': '6',
            'description': 'mole',
            'name': 'mole',
            '#fid': 3,
            '#geometry': 'None',
        },
        10004: {
            'id': '7',
            'description': 'badger',
            'name': 'badger',
            '#fid': 4,
            '#geometry': 'None',
        },
        16002: {
            'id': '5',
            'description': 'toad',
            'name': 'toad',
            '#fid': 2,
            '#geometry': 'None',
        },
    }
    wanted['log'] = [
        'Request 2 did not return any data',
        'Request 7 did not return any data',
        'Request 11 did not return any data',
        'Request 13 did not return any data',
        'Request 14 did not return any data',
        'Errors in file temp_file',
        'The file has been updated by another application - reloading',
        'Errors in file temp_file',
        'The file has been updated by another application - reloading',
        'Errors in file temp_file',
        'The file has been updated by another application - reloading',
    ]
    return wanted


def test_030_filter_rect_xy_spatial_index():
    wanted = {}
    wanted['uri'] = 'file://testextpt.txt?spatialIndex=Y&yField=y&delimiter=|&type=csv&xField=x'
    wanted['fieldTypes'] = ['integer', 'text', 'integer', 'integer']
    wanted['geometryType'] = 0
    wanted['data'] = {
        2: {
            'id': '1',
            'description': 'Inside',
            'x': '15',
            'y': '35',
            '#fid': 2,
            '#geometry': 'Point (15 35)',
        },
        10: {
            'id': '9',
            'description': 'Inside 2',
            'x': '25',
            'y': '45',
            '#fid': 10,
            '#geometry': 'Point (25 45)',
        },
        1002: {
            'id': '1',
            'description': 'Inside',
            'x': '15',
            'y': '35',
            '#fid': 2,
            '#geometry': 'Point (15 35)',
        },
        1010: {
            'id': '9',
            'description': 'Inside 2',
            'x': '25',
            'y': '45',
            '#fid': 10,
            '#geometry': 'Point (25 45)',
        },
        3002: {
            'id': '1',
            'description': 'Inside',
            'x': '15',
            'y': '35',
            '#fid': 2,
            '#geometry': 'Point (15 35)',
        },
        3003: {
            'id': '2',
            'description': 'Outside 1',
            'x': '5',
            'y': '35',
            '#fid': 3,
            '#geometry': 'Point (5 35)',
        },
        3004: {
            'id': '3',
            'description': 'Outside 2',
            'x': '5',
            'y': '55',
            '#fid': 4,
            '#geometry': 'Point (5 55)',
        },
        3005: {
            'id': '4',
            'description': 'Outside 3',
            'x': '15',
            'y': '55',
            '#fid': 5,
            '#geometry': 'Point (15 55)',
        },
        3006: {
            'id': '5',
            'description': 'Outside 4',
            'x': '35',
            'y': '55',
            '#fid': 6,
            '#geometry': 'Point (35 55)',
        },
        3007: {
            'id': '6',
            'description': 'Outside 5',
            'x': '35',
            'y': '45',
            '#fid': 7,
            '#geometry': 'Point (35 45)',
        },
        3008: {
            'id': '7',
            'description': 'Outside 7',
            'x': '35',
            'y': '25',
            '#fid': 8,
            '#geometry': 'Point (35 25)',
        },
        3009: {
            'id': '8',
            'description': 'Outside 8',
            'x': '15',
            'y': '25',
            '#fid': 9,
            '#geometry': 'Point (15 25)',
        },
        3010: {
            'id': '9',
            'description': 'Inside 2',
            'x': '25',
            'y': '45',
            '#fid': 10,
            '#geometry': 'Point (25 45)',
        },
        4002: {
            'id': '1',
            'description': 'Inside',
            'x': '15',
            'y': '35',
            '#fid': 2,
            '#geometry': 'Point (15 35)',
        },
        4003: {
            'id': '2',
            'description': 'Outside 1',
            'x': '5',
            'y': '35',
            '#fid': 3,
            '#geometry': 'Point (5 35)',
        },
        4004: {
            'id': '3',
            'description': 'Outside 2',
            'x': '5',
            'y': '55',
            '#fid': 4,
            '#geometry': 'Point (5 55)',
        },
        4005: {
            'id': '4',
            'description': 'Outside 3',
            'x': '15',
            'y': '55',
            '#fid': 5,
            '#geometry': 'Point (15 55)',
        },
        4006: {
            'id': '5',
            'description': 'Outside 4',
            'x': '35',
            'y': '55',
            '#fid': 6,
            '#geometry': 'Point (35 55)',
        },
        4007: {
            'id': '6',
            'description': 'Outside 5',
            'x': '35',
            'y': '45',
            '#fid': 7,
            '#geometry': 'Point (35 45)',
        },
        4008: {
            'id': '7',
            'description': 'Outside 7',
            'x': '35',
            'y': '25',
            '#fid': 8,
            '#geometry': 'Point (35 25)',
        },
        4009: {
            'id': '8',
            'description': 'Outside 8',
            'x': '15',
            'y': '25',
            '#fid': 9,
            '#geometry': 'Point (15 25)',
        },
        4010: {
            'id': '9',
            'description': 'Inside 2',
            'x': '25',
            'y': '45',
            '#fid': 10,
            '#geometry': 'Point (25 45)',
        },
    }
    wanted['log'] = [
        'Request 2 did not return any data',
    ]
    return wanted


def test_031_filter_rect_wkt_spatial_index():
    wanted = {}
    wanted['uri'] = 'file://testextw.txt?spatialIndex=Y&delimiter=|&type=csv&wktField=wkt'
    wanted['fieldTypes'] = ['integer', 'text']
    wanted['geometryType'] = 1
    wanted['data'] = {
        2: {
            'id': '1',
            'description': 'Inside',
            '#fid': 2,
            '#geometry': 'LineString (12 32, 28 48)',
        },
        4: {
            'id': '3',
            'description': 'Crossing',
            '#fid': 4,
            '#geometry': 'LineString (5 30, 30 55)',
        },
        5: {
            'id': '4',
            'description': 'Bounding box overlap',
            '#fid': 5,
            '#geometry': 'LineString (5 30, 5 55, 30 55)',
        },
        6: {
            'id': '5',
            'description': 'Crossing 2',
            '#fid': 6,
            '#geometry': 'LineString (25 35, 35 35)',
        },
        7: {
            'id': '6',
            'description': 'Bounding box overlap 2',
            '#fid': 7,
            '#geometry': 'LineString (28 29, 31 29, 31 33)',
        },
        1002: {
            'id': '1',
            'description': 'Inside',
            '#fid': 2,
            '#geometry': 'LineString (12 32, 28 48)',
        },
        1004: {
            'id': '3',
            'description': 'Crossing',
            '#fid': 4,
            '#geometry': 'LineString (5 30, 30 55)',
        },
        1006: {
            'id': '5',
            'description': 'Crossing 2',
            '#fid': 6,
            '#geometry': 'LineString (25 35, 35 35)',
        },
        3002: {
            'id': '1',
            'description': 'Inside',
            '#fid': 2,
            '#geometry': 'LineString (12 32, 28 48)',
        },
        3003: {
            'id': '2',
            'description': 'Outside',
            '#fid': 3,
            '#geometry': 'LineString (0 0, 0 10)',
        },
        3004: {
            'id': '3',
            'description': 'Crossing',
            '#fid': 4,
            '#geometry': 'LineString (5 30, 30 55)',
        },
        3005: {
            'id': '4',
            'description': 'Bounding box overlap',
            '#fid': 5,
            '#geometry': 'LineString (5 30, 5 55, 30 55)',
        },
        3006: {
            'id': '5',
            'description': 'Crossing 2',
            '#fid': 6,
            '#geometry': 'LineString (25 35, 35 35)',
        },
        3007: {
            'id': '6',
            'description': 'Bounding box overlap 2',
            '#fid': 7,
            '#geometry': 'LineString (28 29, 31 29, 31 33)',
        },
        4002: {
            'id': '1',
            'description': 'Inside',
            '#fid': 2,
            '#geometry': 'LineString (12 32, 28 48)',
        },
        4003: {
            'id': '2',
            'description': 'Outside',
            '#fid': 3,
            '#geometry': 'LineString (0 0, 0 10)',
        },
        4004: {
            'id': '3',
            'description': 'Crossing',
            '#fid': 4,
            '#geometry': 'LineString (5 30, 30 55)',
        },
        4005: {
            'id': '4',
            'description': 'Bounding box overlap',
            '#fid': 5,
            '#geometry': 'LineString (5 30, 5 55, 30 55)',
        },
        4006: {
            'id': '5',
            'description': 'Crossing 2',
            '#fid': 6,
            '#geometry': 'LineString (25 35, 35 35)',
        },
        4007: {
            'id': '6',
            'description': 'Bounding box overlap 2',
            '#fid': 7,
            '#geometry': 'LineString (28 29, 31 29, 31 33)',
        },
    }
    wanted['log'] = [
        'Request 2 did not return any data',
    ]
    return wanted


def test_032_filter_rect_wkt_create_spatial_index():
    wanted = {}
    wanted['uri'] = 'file://testextw.txt?delimiter=|&type=csv&wktField=wkt'
    wanted['fieldTypes'] = ['integer', 'text']
    wanted['geometryType'] = 1
    wanted['data'] = {
        2: {
            'id': '1',
            'description': 'Inside',
            '#fid': 2,
            '#geometry': 'LineString (12 32, 28 48)',
        },
        4: {
            'id': '3',
            'description': 'Crossing',
            '#fid': 4,
            '#geometry': 'LineString (5 30, 30 55)',
        },
        5: {
            'id': '4',
            'description': 'Bounding box overlap',
            '#fid': 5,
            '#geometry': 'LineString (5 30, 5 55, 30 55)',
        },
        6: {
            'id': '5',
            'description': 'Crossing 2',
            '#fid': 6,
            '#geometry': 'LineString (25 35, 35 35)',
        },
        7: {
            'id': '6',
            'description': 'Bounding box overlap 2',
            '#fid': 7,
            '#geometry': 'LineString (28 29, 31 29, 31 33)',
        },
        1002: {
            'id': '1',
            'description': 'Inside',
            '#fid': 2,
            '#geometry': 'LineString (12 32, 28 48)',
        },
        1003: {
            'id': '2',
            'description': 'Outside',
            '#fid': 3,
            '#geometry': 'LineString (0 0, 0 10)',
        },
        1004: {
            'id': '3',
            'description': 'Crossing',
            '#fid': 4,
            '#geometry': 'LineString (5 30, 30 55)',
        },
        1005: {
            'id': '4',
            'description': 'Bounding box overlap',
            '#fid': 5,
            '#geometry': 'LineString (5 30, 5 55, 30 55)',
        },
        1006: {
            'id': '5',
            'description': 'Crossing 2',
            '#fid': 6,
            '#geometry': 'LineString (25 35, 35 35)',
        },
        1007: {
            'id': '6',
            'description': 'Bounding box overlap 2',
            '#fid': 7,
            '#geometry': 'LineString (28 29, 31 29, 31 33)',
        },
        3002: {
            'id': '1',
            'description': 'Inside',
            '#fid': 2,
            '#geometry': 'LineString (12 32, 28 48)',
        },
        3004: {
            'id': '3',
            'description': 'Crossing',
            '#fid': 4,
            '#geometry': 'LineString (5 30, 30 55)',
        },
        3005: {
            'id': '4',
            'description': 'Bounding box overlap',
            '#fid': 5,
            '#geometry': 'LineString (5 30, 5 55, 30 55)',
        },
        3006: {
            'id': '5',
            'description': 'Crossing 2',
            '#fid': 6,
            '#geometry': 'LineString (25 35, 35 35)',
        },
        3007: {
            'id': '6',
            'description': 'Bounding box overlap 2',
            '#fid': 7,
            '#geometry': 'LineString (28 29, 31 29, 31 33)',
        },
        4002: {
            'id': '1',
            'description': 'Inside',
            '#fid': 2,
            '#geometry': 'LineString (12 32, 28 48)',
        },
        4004: {
            'id': '3',
            'description': 'Crossing',
            '#fid': 4,
            '#geometry': 'LineString (5 30, 30 55)',
        },
        4006: {
            'id': '5',
            'description': 'Crossing 2',
            '#fid': 6,
            '#geometry': 'LineString (25 35, 35 35)',
        },
        6002: {
            'id': '1',
            'description': 'Inside',
            '#fid': 2,
            '#geometry': 'LineString (12 32, 28 48)',
        },
        6003: {
            'id': '2',
            'description': 'Outside',
            '#fid': 3,
            '#geometry': 'LineString (0 0, 0 10)',
        },
        6004: {
            'id': '3',
            'description': 'Crossing',
            '#fid': 4,
            '#geometry': 'LineString (5 30, 30 55)',
        },
        6005: {
            'id': '4',
            'description': 'Bounding box overlap',
            '#fid': 5,
            '#geometry': 'LineString (5 30, 5 55, 30 55)',
        },
        6006: {
            'id': '5',
            'description': 'Crossing 2',
            '#fid': 6,
            '#geometry': 'LineString (25 35, 35 35)',
        },
        6007: {
            'id': '6',
            'description': 'Bounding box overlap 2',
            '#fid': 7,
            '#geometry': 'LineString (28 29, 31 29, 31 33)',
        },
        7002: {
            'id': '1',
            'description': 'Inside',
            '#fid': 2,
            '#geometry': 'LineString (12 32, 28 48)',
        },
        7003: {
            'id': '2',
            'description': 'Outside',
            '#fid': 3,
            '#geometry': 'LineString (0 0, 0 10)',
        },
        7004: {
            'id': '3',
            'description': 'Crossing',
            '#fid': 4,
            '#geometry': 'LineString (5 30, 30 55)',
        },
        7005: {
            'id': '4',
            'description': 'Bounding box overlap',
            '#fid': 5,
            '#geometry': 'LineString (5 30, 5 55, 30 55)',
        },
        7006: {
            'id': '5',
            'description': 'Crossing 2',
            '#fid': 6,
            '#geometry': 'LineString (25 35, 35 35)',
        },
        7007: {
            'id': '6',
            'description': 'Bounding box overlap 2',
            '#fid': 7,
            '#geometry': 'LineString (28 29, 31 29, 31 33)',
        },
    }
    wanted['log'] = [
        'Request 5 did not return any data',
    ]
    return wanted


def test_033_reset_subset_string():
    wanted = {}
    wanted['uri'] = 'file://test.csv?geomType=none&type=csv'
    wanted['fieldTypes'] = ['integer', 'text', 'text', 'text', 'text']
    wanted['geometryType'] = 4
    wanted['data'] = {
        2: {
            'id': '1',
            'description': 'Basic unquoted record',
            'data': 'Some data',
            'info': 'Some info',
            'field_5': 'NULL',
            '#fid': 2,
            '#geometry': 'None',
        },
        3: {
            'id': '2',
            'description': 'Quoted field',
            'data': 'Quoted data',
            'info': 'Unquoted',
            'field_5': 'NULL',
            '#fid': 3,
            '#geometry': 'None',
        },
        4: {
            'id': '3',
            'description': 'Escaped quotes',
            'data': 'Quoted "citation" data',
            'info': 'Unquoted',
            'field_5': 'NULL',
            '#fid': 4,
            '#geometry': 'None',
        },
        5: {
            'id': '4',
            'description': 'Quoted newlines',
            'data': 'Line 1\nLine 2\n\nLine 4',
            'info': 'No data',
            'field_5': 'NULL',
            '#fid': 5,
            '#geometry': 'None',
        },
        9: {
            'id': '5',
            'description': 'Extra fields',
            'data': 'data',
            'info': 'info',
            'field_5': 'message',
            '#fid': 9,
            '#geometry': 'None',
        },
        10: {
            'id': '6',
            'description': 'Missing fields',
            'data': 'NULL',
            'info': 'NULL',
            'field_5': 'NULL',
            '#fid': 10,
            '#geometry': 'None',
        },
        2002: {
            'id': '1',
            'description': 'Basic unquoted record',
            'data': 'Some data',
            'info': 'Some info',
            'field_5': 'NULL',
            '#fid': 2,
            '#geometry': 'None',
        },
        2004: {
            'id': '3',
            'description': 'Escaped quotes',
            'data': 'Quoted "citation" data',
            'info': 'Unquoted',
            'field_5': 'NULL',
            '#fid': 4,
            '#geometry': 'None',
        },
        2009: {
            'id': '5',
            'description': 'Extra fields',
            'data': 'data',
            'info': 'info',
            'field_5': 'message',
            '#fid': 9,
            '#geometry': 'None',
        },
        4010: {
            'id': '6',
            'description': 'Missing fields',
            'data': 'NULL',
            'info': 'NULL',
            'field_5': 'NULL',
            '#fid': 10,
            '#geometry': 'None',
        },
        6004: {
            'id': '3',
            'description': 'Escaped quotes',
            'data': 'Quoted "citation" data',
            'info': 'Unquoted',
            'field_5': 'NULL',
            '#fid': 4,
            '#geometry': 'None',
        },
        8002: {
            'id': '1',
            'description': 'Basic unquoted record',
            'data': 'Some data',
            'info': 'Some info',
            'field_5': 'NULL',
            '#fid': 2,
            '#geometry': 'None',
        },
        8004: {
            'id': '3',
            'description': 'Escaped quotes',
            'data': 'Quoted "citation" data',
            'info': 'Unquoted',
            'field_5': 'NULL',
            '#fid': 4,
            '#geometry': 'None',
        },
        8009: {
            'id': '5',
            'description': 'Extra fields',
            'data': 'data',
            'info': 'info',
            'field_5': 'message',
            '#fid': 9,
            '#geometry': 'None',
        },
        10003: {
            'id': '2',
            'description': 'Quoted field',
            'data': 'Quoted data',
            'info': 'Unquoted',
            'field_5': 'NULL',
            '#fid': 3,
            '#geometry': 'None',
        },
        10005: {
            'id': '4',
            'description': 'Quoted newlines',
            'data': 'Line 1\nLine 2\n\nLine 4',
            'info': 'No data',
            'field_5': 'NULL',
            '#fid': 5,
            '#geometry': 'None',
        },
        10010: {
            'id': '6',
            'description': 'Missing fields',
            'data': 'NULL',
            'info': 'NULL',
            'field_5': 'NULL',
            '#fid': 10,
            '#geometry': 'None',
        },
    }
    wanted['log'] = []
    return wanted


def test_034_csvt_file():
    """In the test file we have two rows with 11 and 12 fields, the CSV has only 11 headers:
    id,description,fint,freal,fstr,fstr,fdatetime,fdate,ftime,flong,flonglong
    The CSVT contains 11 field types (note "long" which is not supported but interpreted as an alias for "longlong"):
    integer,string,integer,real,string,string,string,string,string,long,longlong
    """
    wanted = {}
    wanted['uri'] = 'file://testcsvt.csv?geomType=none&type=csv'
    wanted['fieldTypes'] = ['integer', 'text', 'integer', 'double', 'text', 'text', 'text', 'text', 'text', 'longlong', 'longlong', 'longlong']
    wanted['geometryType'] = 4
    wanted['data'] = {
        2: {
            'id': '1',
            'description': 'Test csvt 1',
            'fint': '1',
            'freal': '1.2',
            'fstr': '1',
            'fstr_1': 'text',
            'fdatetime': '2015-03-02T12:30:00',
            'fdate': '2014-12-30',
            'ftime': '23:55',
            'flong': '-456',
            'flonglong': '-678',
            'field_12': 'NULL',
            '#fid': 2,
            '#geometry': 'None',
        },
        3: {
            'id': '2',
            'description': 'Test csvt 2',
            'fint': '3',
            'freal': '1.5',
            'fstr': '99',
            'fstr_1': '23.5',
            'fdatetime': '80',
            'fdate': '2015-03-28',
            'ftime': '2014-12-30',
            'flong': 'NULL',
            'flonglong': '9189304972279762602',
            'field_12': '-3123724580211819352',
            '#fid': 3,
            '#geometry': 'None',
        },
    }
    wanted['log'] = []
    return wanted


def test_035_csvt_file2():
    wanted = {}
    wanted['uri'] = 'file://testcsvt2.txt?geomType=none&delimiter=|&type=csv'
    wanted['fieldTypes'] = ['integer', 'text', 'integer', 'double', 'integer', 'text', 'integer']
    wanted['geometryType'] = 4
    wanted['data'] = {
        2: {
            'id': '1',
            'description': 'Test csvt 1',
            'f1': '1',
            'f2': '1.2',
            'f3': '1',
            'f4': 'text',
            'f5': 'NULL',
            '#fid': 2,
            '#geometry': 'None',
        },
        3: {
            'id': '2',
            'description': 'Test csvt 2',
            'f1': '3',
            'f2': '1.5',
            'f3': '99',
            'f4': '23.5',
            'f5': '80',
            '#fid': 3,
            '#geometry': 'None',
        },
    }
    wanted['log'] = []
    return wanted


def test_036_csvt_file_invalid_types():
    wanted = {}
    wanted['uri'] = 'file://testcsvt3.csv?geomType=none&type=csv'
    wanted['fieldTypes'] = ['integer', 'text', 'integer', 'double', 'integer', 'text', 'text']
    wanted['geometryType'] = 4
    wanted['data'] = {
        2: {
            'id': '1',
            'description': 'Test csvt 1',
            'f1': '1',
            'f2': '1.2',
            'f3': '1',
            'f4': 'text',
            'f5': 'times',
            '#fid': 2,
            '#geometry': 'None',
        },
        3: {
            'id': '2',
            'description': 'Test csvt 2',
            'f1': '3',
            'f2': '1.5',
            'f3': '99',
            'f4': '23.5',
            'f5': '80',
            '#fid': 3,
            '#geometry': 'None',
        },
    }
    wanted['log'] = [
        'Errors in file testcsvt3.csv',
        'File type string in testcsvt3.csvt is not correctly formatted',
    ]
    return wanted


def test_037_csvt_file_invalid_file():
    wanted = {}
    wanted['uri'] = 'file://testcsvt4.csv?geomType=none&type=csv'
    wanted['fieldTypes'] = ['integer', 'text', 'integer', 'double', 'integer', 'text', 'text']
    wanted['geometryType'] = 4
    wanted['data'] = {
        2: {
            'id': '1',
            'description': 'Test csvt 1',
            'f1': '1',
            'f2': '1.2',
            'f3': '1',
            'f4': 'text',
            'f5': 'times',
            '#fid': 2,
            '#geometry': 'None',
        },
        3: {
            'id': '2',
            'description': 'Test csvt 2',
            'f1': '3',
            'f2': '1.5',
            'f3': '99',
            'f4': '23.5',
            'f5': '80',
            '#fid': 3,
            '#geometry': 'None',
        },
    }
    wanted['log'] = []
    return wanted


def test_038_type_inference():
    wanted = {}
    wanted['uri'] = 'file://testtypes.csv?yField=lat&xField=lon&type=csv'
    wanted['fieldTypes'] = ['text', 'double', 'double', 'text', 'text', 'integer', 'longlong', 'double', 'text']
    wanted['geometryType'] = 0
    wanted['data'] = {
        2: {
            'id': 'line1',
            'description': '1.0',
            'lon': '1.0',
            'lat': '1.0',
            'empty': 'NULL',
            'text': 'NULL',
            'int': '0',
            'longlong': '0',
            'real': 'NULL',
            'text2': '1',
            '#fid': 2,
            '#geometry': 'Point (1 1)',
        },
        3: {
            'id': 'line2',
            'description': '1.0',
            'lon': '1.0',
            'lat': '5.0',
            'empty': 'NULL',
            'text': '1',
            'int': 'NULL',
            'longlong': '9189304972279762602',
            'real': '1.3',
            'text2': '-4',
            '#fid': 3,
            '#geometry': 'Point (1 5)',
        },
        4: {
            'id': 'line3',
            'description': '5.0',
            'lon': '5.0',
            'lat': '5.0',
            'empty': 'NULL',
            'text': '1xx',
            'int': '2',
            'longlong': '345',
            'real': '2.0',
            'text2': '1x',
            '#fid': 4,
            '#geometry': 'Point (5 5)',
        },
        5: {
            'id': 'line4',
            'description': '5.0',
            'lon': '5.0',
            'lat': '1.0',
            'empty': 'NULL',
            'text': 'A string',
            'int': '-3456',
            'longlong': '-3123724580211819352',
            'real': '-123.56',
            'text2': 'NULL',
            '#fid': 5,
            '#geometry': 'Point (5 1)',
        },
        6: {
            'id': 'line5',
            'description': '3.0',
            'lon': '3.0',
            'lat': '1.0',
            'empty': 'NULL',
            'text': 'NULL',
            'int': 'NULL',
            'longlong': 'NULL',
            'real': '0.00023',
            'text2': '23',
            '#fid': 6,
            '#geometry': 'Point (3 1)',
        },
        7: {
            'id': 'line6',
            'description': '1.0',
            'lon': '1.0',
            'lat': '3.0',
            'empty': 'NULL',
            'text': '1.5',
            'int': '9',
            'longlong': '42',
            'real': '99.0',
            'text2': '0',
            '#fid': 7,
            '#geometry': 'Point (1 3)',
        },
    }
    wanted['log'] = []
    return wanted


def test_039_issue_13749():
    wanted = {}
    wanted['uri'] = 'file://test13749.csv?yField=geom_y&xField=geom_x&type=csv'
    wanted['fieldTypes'] = ['integer', 'text', 'double', 'double']
    wanted['geometryType'] = 0
    wanted['data'] = {
        2: {
            'id': '1',
            'description': 'No geom',
            'geom_x': 'NULL',
            'geom_y': 'NULL',
            '#fid': 2,
            '#geometry': 'None',
        },
        3: {
            'id': '2',
            'description': 'Point1',
            'geom_x': '11.0',
            'geom_y': '22.0',
            '#fid': 3,
            '#geometry': 'Point (11 22)',
        },
        4: {
            'id': '3',
            'description': 'Point2',
            'geom_x': '15.0',
            'geom_y': '23.0',
            '#fid': 4,
            '#geometry': 'Point (15 23)',
        },
        5: {
            'id': '4',
            'description': 'Point3',
            'geom_x': '13.0',
            'geom_y': '23.0',
            '#fid': 5,
            '#geometry': 'Point (13 23)',
        },
    }
    wanted['log'] = [
        'Errors in file test13749.csv',
        '1 record(s) have missing geometry definitions',
    ]
    return wanted


def test_040_issue_14666():
    wanted = {}
    wanted['uri'] = 'file://test14666.csv?yField=y&xField=x&type=csv&delimiter=\\t'
    wanted['fieldTypes'] = ['integer', 'double', 'double']
    wanted['geometryType'] = 0
    wanted['data'] = {
        2: {
            'id': '1',
            'description': '7.15417',
            'x': '7.15417',
            'y': '50.680622',
            '#fid': 2,
            '#geometry': 'Point (7.1541699999999997 50.68062199999999962)',
        },
        3: {
            'id': '2',
            'description': '7.119219',
            'x': '7.119219',
            'y': '50.739814',
            '#fid': 3,
            '#geometry': 'Point (7.11921900000000019 50.73981400000000264)',
        },
        4: {
            'id': '3',
            'description': 'NULL',
            'x': 'NULL',
            'y': 'NULL',
            '#fid': 4,
            '#geometry': 'None',
        },
        5: {
            'id': '4',
            'description': 'NULL',
            'x': 'NULL',
            'y': 'NULL',
            '#fid': 5,
            '#geometry': 'None',
        },
        6: {
            'id': '5',
            'description': '7.129229',
            'x': '7.129229',
            'y': '50.703692',
            '#fid': 6,
            '#geometry': 'Point (7.12922899999999959 50.70369199999999665)',
        },
    }
    wanted['log'] = [
        'Errors in file test14666.csv',
        '2 record(s) have missing geometry definitions',
    ]
    return wanted


def test_041_no_detect_type():
    wanted = {}
    wanted['uri'] = 'file://testtypes.csv?yField=lat&xField=lon&type=csv&detectTypes=no'
    wanted['fieldTypes'] = ['text', 'text', 'text', 'text', 'text', 'text', 'text', 'text', 'text']
    wanted['geometryType'] = 0
    wanted['data'] = {
        2: {
            'id': 'line1',
            'description': '1.0',
            'lon': '1.0',
            'lat': '1.0',
            'empty': 'NULL',
            'text': 'NULL',
            'int': '0',
            'longlong': '0',
            'real': 'NULL',
            'text2': '1',
            '#fid': 2,
            '#geometry': 'Point (1 1)',
        },
        3: {
            'id': 'line2',
            'description': '1.0',
            'lon': '1.0',
            'lat': '5.0',
            'empty': 'NULL',
            'text': '1',
            'int': 'NULL',
            'longlong': '9189304972279762602',
            'real': '1.3',
            'text2': '-4',
            '#fid': 3,
            '#geometry': 'Point (1 5)',
        },
        4: {
            'id': 'line3',
            'description': '5.0',
            'lon': '5.0',
            'lat': '5.0',
            'empty': 'NULL',
            'text': '1xx',
            'int': '2',
            'longlong': '345',
            'real': '2',
            'text2': '1x',
            '#fid': 4,
            '#geometry': 'Point (5 5)',
        },
        5: {
            'id': 'line4',
            'description': '5.0',
            'lon': '5.0',
            'lat': '1.0',
            'empty': 'NULL',
            'text': 'A string',
            'int': '-3456',
            'longlong': '-3123724580211819352',
            'real': '-123.56',
            'text2': 'NULL',
            '#fid': 5,
            '#geometry': 'Point (5 1)',
        },
        6: {
            'id': 'line5',
            'description': '3.0',
            'lon': '3.0',
            'lat': '1.0',
            'empty': 'NULL',
            'text': 'NULL',
            'int': 'NULL',
            'longlong': 'NULL',
            'real': '23e-5',
            'text2': '23',
            '#fid': 6,
            '#geometry': 'Point (3 1)',
        },
        7: {
            'id': 'line6',
            'description': '1.0',
            'lon': '1.0',
            'lat': '3.0',
            'empty': 'NULL',
            'text': '1.5',
            'int': '9',
            'longlong': '42',
            'real': '99',
            'text2': '0',
            '#fid': 7,
            '#geometry': 'Point (1 3)',
        },
    }
    wanted['log'] = [
    ]
    return wanted


def test_042_no_detect_types_csvt():
    """detectTypes is no, the types are taken from the CSVT except the last one (which is not in the CSVT
    and it is not detected)"""

    wanted = {}
    wanted['uri'] = 'file://testcsvt.csv?geomType=none&type=csv&detectTypes=no'
    wanted['fieldTypes'] = ['integer', 'text', 'integer', 'double', 'text', 'text', 'text', 'text', 'text', 'longlong', 'longlong', 'text']
    wanted['geometryType'] = 4
    wanted['data'] = {
        2: {
            'id': '1',
            'description': 'Test csvt 1',
            'fint': '1',
            'freal': '1.2',
            'fstr': '1',
            'fstr_1': 'text',
            'fdatetime': '2015-03-02T12:30:00',
            'fdate': '2014-12-30',
            'ftime': '23:55',
            'flong': '-456',
            'flonglong': '-678',
            'field_12': 'NULL',
            '#fid': 2,
            '#geometry': 'None',
        },
        3: {
            'id': '2',
            'description': 'Test csvt 2',
            'fint': '3',
            'freal': '1.5',
            'fstr': '99',
            'fstr_1': '23.5',
            'fdatetime': '80',
            'fdate': '2015-03-28',
            'ftime': '2014-12-30',
            'flong': 'NULL',
            'flonglong': '9189304972279762602',
            'field_12': '-3123724580211819352',
            '#fid': 3,
            '#geometry': 'None',
        },
    }
    wanted['log'] = [
    ]
    return wanted


def test_048_csvt_file():
    wanted = {}
    wanted['uri'] = 'file://testcsvt5.csv?geomType=none&type=csv'
    wanted['fieldTypes'] = ['text', 'text', 'double', 'double', 'double']
    wanted['geometryType'] = 4
    wanted['data'] = {
        2: {
            'id': '01',
            'description': 'Test csvt 1',
            'f1': '0.3',
            'f2': '0.8',
            'f3': '1.4',
            '#fid': 2,
            '#geometry': 'None',
        },
        3: {
            'id': '12',
            'description': 'Test csvt 2',
            'f1': '0.2',
            'f2': '78.0',
            'f3': '13.4',
            '#fid': 3,
            '#geometry': 'None',
        },
    }
    wanted['log'] = []
    return wanted
