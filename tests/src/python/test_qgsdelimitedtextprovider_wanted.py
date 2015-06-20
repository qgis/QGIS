
def test_002_load_csv_file():
    wanted={}
    wanted['uri']=u'file://test.csv?geomType=none&type=csv'
    wanted['fieldTypes']=['integer', 'text', 'text', 'text', 'text']
    wanted['data']={
        2L: {
            'id': u'1',
            'description': u'Basic unquoted record',
            'data': u'Some data',
            'info': u'Some info',
            'field_5': u'NULL',
            '#fid': 2L,
            '#geometry': 'None',
        },
        3L: {
            'id': u'2',
            'description': u'Quoted field',
            'data': u'Quoted data',
            'info': u'Unquoted',
            'field_5': u'NULL',
            '#fid': 3L,
            '#geometry': 'None',
        },
        4L: {
            'id': u'3',
            'description': u'Escaped quotes',
            'data': u'Quoted "citation" data',
            'info': u'Unquoted',
            'field_5': u'NULL',
            '#fid': 4L,
            '#geometry': 'None',
        },
        5L: {
            'id': u'4',
            'description': u'Quoted newlines',
            'data': u'Line 1\nLine 2\n\nLine 4',
            'info': u'No data',
            'field_5': u'NULL',
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
            'data': u'NULL',
            'info': u'NULL',
            'field_5': u'NULL',
            '#fid': 10L,
            '#geometry': 'None',
        },
    }
    wanted['log']=[]
    return wanted


def test_003_field_naming():
    wanted={}
    wanted['uri']=u'file://testfields.csv?geomType=none&type=csv'
    wanted['fieldTypes']=['integer', 'text', 'text', 'text', 'text', 'text', 'text', 'text', 'text', 'text', 'text', 'text']
    wanted['data']={
        2L: {
            'id': u'1',
            'description': u'Generation of field names',
            'data': u'Some data',
            'field_4': u'Some info',
            'data_2': u'NULL',
            'field_6': u'NULL',
            'field_7': u'NULL',
            'field_3_1': u'NULL',
            'data_1': u'NULL',
            'field_10': u'NULL',
            'field_11': u'NULL',
            'field_12': u'last data',
            '#fid': 2L,
            '#geometry': 'None',
        },
    }
    wanted['log']=[]
    return wanted


def test_004_max_fields():
    wanted={}
    wanted['uri']=u'file://testfields.csv?geomType=none&maxFields=7&type=csv'
    wanted['fieldTypes']=['integer', 'text', 'text', 'text', 'text', 'text', 'text']
    wanted['data']={
        2L: {
            'id': u'1',
            'description': u'Generation of field names',
            'data': u'Some data',
            'field_4': u'Some info',
            'data_1': u'NULL',
            'field_6': u'NULL',
            'field_7': u'NULL',
            '#fid': 2L,
            '#geometry': 'None',
        },
    }
    wanted['log']=[]
    return wanted


def test_005_load_whitespace():
    wanted={}
    wanted['uri']=u'file://test.space?geomType=none&type=whitespace'
    wanted['fieldTypes']=['integer', 'text', 'text', 'text', 'text', 'text']
    wanted['data']={
        2L: {
            'id': u'1',
            'description': u'Simple_whitespace_file',
            'data': u'data1',
            'info': u'info1',
            'field_5': u'NULL',
            'field_6': u'NULL',
            '#fid': 2L,
            '#geometry': 'None',
        },
        3L: {
            'id': u'2',
            'description': u'Whitespace_at_start_of_line',
            'data': u'data2',
            'info': u'info2',
            'field_5': u'NULL',
            'field_6': u'NULL',
            '#fid': 3L,
            '#geometry': 'None',
        },
        4L: {
            'id': u'3',
            'description': u'Tab_whitespace',
            'data': u'data3',
            'info': u'info3',
            'field_5': u'NULL',
            'field_6': u'NULL',
            '#fid': 4L,
            '#geometry': 'None',
        },
        5L: {
            'id': u'4',
            'description': u'Multiple_whitespace_characters',
            'data': u'data4',
            'info': u'info4',
            'field_5': u'NULL',
            'field_6': u'NULL',
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
            'data': u'NULL',
            'info': u'NULL',
            'field_5': u'NULL',
            'field_6': u'NULL',
            '#fid': 7L,
            '#geometry': 'None',
        },
    }
    wanted['log']=[]
    return wanted


def test_006_quote_escape():
    wanted={}
    wanted['uri']=u'file://test.pipe?geomType=none&quote="&delimiter=|&escape=\\'
    wanted['fieldTypes']=['integer', 'text', 'text', 'text', 'text', 'text']
    wanted['data']={
        2L: {
            'id': u'1',
            'description': u'Using pipe delimiter',
            'data': u'data 1',
            'info': u'info 1',
            'field_5': u'NULL',
            'field_6': u'NULL',
            '#fid': 2L,
            '#geometry': 'None',
        },
        3L: {
            'id': u'2',
            'description': u'Using backslash escape on pipe',
            'data': u'data 2 | piped',
            'info': u'info2',
            'field_5': u'NULL',
            'field_6': u'NULL',
            '#fid': 3L,
            '#geometry': 'None',
        },
        4L: {
            'id': u'3',
            'description': u'Backslash escaped newline',
            'data': u'data3 \nline2 \nline3',
            'info': u'info3',
            'field_5': u'NULL',
            'field_6': u'NULL',
            '#fid': 4L,
            '#geometry': 'None',
        },
        7L: {
            'id': u'4',
            'description': u'Empty field',
            'data': u'NULL',
            'info': u'info4',
            'field_5': u'NULL',
            'field_6': u'NULL',
            '#fid': 7L,
            '#geometry': 'None',
        },
        8L: {
            'id': u'5',
            'description': u'Quoted field',
            'data': u'More | piped data',
            'info': u'info5',
            'field_5': u'NULL',
            'field_6': u'NULL',
            '#fid': 8L,
            '#geometry': 'None',
        },
        9L: {
            'id': u'6',
            'description': u'Escaped quote',
            'data': u'Field "citation" ',
            'info': u'info6',
            'field_5': u'NULL',
            'field_6': u'NULL',
            '#fid': 9L,
            '#geometry': 'None',
        },
        10L: {
            'id': u'7',
            'description': u'Missing fields',
            'data': u'NULL',
            'info': u'NULL',
            'field_5': u'NULL',
            'field_6': u'NULL',
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
    wanted['log']=[]
    return wanted


def test_007_multiple_quote():
    wanted={}
    wanted['uri']=u'file://test.quote?geomType=none&quote=\'"&type=csv&escape="\''
    wanted['fieldTypes']=['integer', 'text', 'text', 'text']
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
        u'Errors in file test.quote',
        u'3 records discarded due to invalid format',
        u'The following lines were not loaded into QGIS due to errors:',
        u'Invalid record format at line 7',
        u'Invalid record format at line 8',
        u'Invalid record format at line 9',
    ]
    return wanted


def test_008_badly_formed_quotes():
    wanted={}
    wanted['uri']=u'file://test.badquote?geomType=none&quote="&type=csv&escape="'
    wanted['fieldTypes']=['integer', 'text', 'text', 'text']
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
        u'Errors in file test.badquote',
        u'2 records discarded due to invalid format',
        u'The following lines were not loaded into QGIS due to errors:',
        u'Invalid record format at line 2',
        u'Invalid record format at line 5',
    ]
    return wanted


def test_009_skip_lines():
    wanted={}
    wanted['uri']=u'file://test2.csv?geomType=none&skipLines=2&type=csv&useHeader=no'
    wanted['fieldTypes']=['integer', 'text', 'text']
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
    wanted['log']=[]
    return wanted


def test_010_read_coordinates():
    wanted={}
    wanted['uri']=u'file://testpt.csv?yField=geom_y&xField=geom_x&type=csv'
    wanted['fieldTypes']=['integer', 'text', 'double', 'double']
    wanted['data']={
        2L: {
            'id': u'1',
            'description': u'Basic point',
            'geom_x': u'10.5',
            'geom_y': u'20.82',
            '#fid': 2L,
            '#geometry': 'Point (10.5 20.82)',
        },
        3L: {
            'id': u'2',
            'description': u'Integer point',
            'geom_x': u'11.0',
            'geom_y': u'22.0',
            '#fid': 3L,
            '#geometry': 'Point (11 22)',
        },
        5L: {
            'id': u'4',
            'description': u'Final point',
            'geom_x': u'13.0',
            'geom_y': u'23.0',
            '#fid': 5L,
            '#geometry': 'Point (13 23)',
        },
    }
    wanted['log']=[
        u'Errors in file testpt.csv',
        u'1 records discarded due to invalid geometry definitions',
        u'The following lines were not loaded into QGIS due to errors:',
        u'Invalid X or Y fields at line 4',
    ]
    return wanted


def test_011_read_wkt():
    wanted={}
    wanted['uri']=u'file://testwkt.csv?delimiter=|&type=csv&wktField=geom_wkt'
    wanted['fieldTypes']=['integer', 'text']
    wanted['data']={
        2L: {
            'id': u'1',
            'description': u'Point wkt',
            '#fid': 2L,
            '#geometry': 'Point (10 20)',
        },
        3L: {
            'id': u'2',
            'description': u'Multipoint wkt',
            '#fid': 3L,
            '#geometry': 'MultiPoint ((10 20),(11 21))',
        },
        9L: {
            'id': u'8',
            'description': u'EWKT prefix',
            '#fid': 9L,
            '#geometry': 'Point (10 10)',
        },
        10L: {
            'id': u'9',
            'description': u'Informix prefix',
            '#fid': 10L,
            '#geometry': 'Point (10 10)',
        },
        11L: {
            'id': u'10',
            'description': u'Measure in point',
            '#fid': 11L,
            '#geometry': 'Point (10 20)',
        },
    }
    wanted['log']=[
        u'Errors in file testwkt.csv',
        u'1 records discarded due to invalid geometry definitions',
        u'7 records discarded due to incompatible geometry types',
        u'The following lines were not loaded into QGIS due to errors:',
        u'Invalid WKT at line 8',
    ]
    return wanted


def test_012_read_wkt_point():
    wanted={}
    wanted['uri']=u'file://testwkt.csv?geomType=point&delimiter=|&type=csv&wktField=geom_wkt'
    wanted['fieldTypes']=['integer', 'text']
    wanted['data']={
        2L: {
            'id': u'1',
            'description': u'Point wkt',
            '#fid': 2L,
            '#geometry': 'Point (10 20)',
        },
        3L: {
            'id': u'2',
            'description': u'Multipoint wkt',
            '#fid': 3L,
            '#geometry': 'MultiPoint ((10 20),(11 21))',
        },
        9L: {
            'id': u'8',
            'description': u'EWKT prefix',
            '#fid': 9L,
            '#geometry': 'Point (10 10)',
        },
        10L: {
            'id': u'9',
            'description': u'Informix prefix',
            '#fid': 10L,
            '#geometry': 'Point (10 10)',
        },
        11L: {
            'id': u'10',
            'description': u'Measure in point',
            '#fid': 11L,
            '#geometry': 'Point (10 20)',
        },
    }
    wanted['log']=[
        u'Errors in file testwkt.csv',
        u'1 records discarded due to invalid geometry definitions',
        u'7 records discarded due to incompatible geometry types',
        u'The following lines were not loaded into QGIS due to errors:',
        u'Invalid WKT at line 8',
    ]
    return wanted


def test_013_read_wkt_line():
    wanted={}
    wanted['uri']=u'file://testwkt.csv?geomType=line&delimiter=|&type=csv&wktField=geom_wkt'
    wanted['fieldTypes']=['integer', 'text']
    wanted['data']={
        4L: {
            'id': u'3',
            'description': u'Linestring wkt',
            '#fid': 4L,
            '#geometry': 'LineString (10 20, 11 21)',
        },
        5L: {
            'id': u'4',
            'description': u'Multiline string wkt',
            '#fid': 5L,
            '#geometry': 'MultiLineString ((10 20, 11 21), (20 30, 21 31))',
        },
        12L: {
            'id': u'11',
            'description': u'Measure in line',
            '#fid': 12L,
            '#geometry': 'LineString (10 20, 11 21)',
        },
        13L: {
            'id': u'12',
            'description': u'Z in line',
            '#fid': 13L,
            '#geometry': 'LineString (10 20, 11 21)',
        },
        14L: {
            'id': u'13',
            'description': u'Measure and Z in line',
            '#fid': 14L,
            '#geometry': 'LineString (10 20, 11 21)',
        },
    }
    wanted['log']=[
        u'Errors in file testwkt.csv',
        u'1 records discarded due to invalid geometry definitions',
        u'7 records discarded due to incompatible geometry types',
        u'The following lines were not loaded into QGIS due to errors:',
        u'Invalid WKT at line 8',
    ]
    return wanted


def test_014_read_wkt_polygon():
    wanted={}
    wanted['uri']=u'file://testwkt.csv?geomType=polygon&delimiter=|&type=csv&wktField=geom_wkt'
    wanted['fieldTypes']=['integer', 'text']
    wanted['data']={
        6L: {
            'id': u'5',
            'description': u'Polygon wkt',
            '#fid': 6L,
            '#geometry': 'Polygon ((10 10,10 20,20 20,20 10,10 10),(14 14,14 16,16 16,14 14))',
        },
        7L: {
            'id': u'6',
            'description': u'MultiPolygon wkt',
            '#fid': 7L,
            '#geometry': 'MultiPolygon (((10 10,10 20,20 20,20 10,10 10),(14 14,14 16,16 16,14 14)),((30 30,30 35,35 35,30 30)))',
        },
    }
    wanted['log']=[
        u'Errors in file testwkt.csv',
        u'1 records discarded due to invalid geometry definitions',
        u'10 records discarded due to incompatible geometry types',
        u'The following lines were not loaded into QGIS due to errors:',
        u'Invalid WKT at line 8',
    ]
    return wanted


def test_015_read_dms_xy():
    wanted={}
    wanted['uri']=u'file://testdms.csv?yField=lat&xField=lon&type=csv&xyDms=yes'
    wanted['fieldTypes']=['integer', 'text', 'text', 'text']
    wanted['data']={
        3L: {
            'id': u'1',
            'description': u'Basic DMS string',
            'lon': u'1 5 30.6',
            'lat': u'35 51 20',
            '#fid': 3L,
            '#geometry': 'Point (1.09183333 35.85555556)',
        },
        4L: {
            'id': u'2',
            'description': u'Basic DMS string 2',
            'lon': u'1 05 30.6005',
            'lat': u'035 51 20',
            '#fid': 4L,
            '#geometry': 'Point (1.09183347 35.85555556)',
        },
        5L: {
            'id': u'3',
            'description': u'Basic DMS string 3',
            'lon': u'1 05 30.6',
            'lat': u'35 59 9.99',
            '#fid': 5L,
            '#geometry': 'Point (1.09183333 35.98610833)',
        },
        7L: {
            'id': u'4',
            'description': u'Prefix sign 1',
            'lon': u'n1 05 30.6',
            'lat': u'e035 51 20',
            '#fid': 7L,
            '#geometry': 'Point (1.09183333 35.85555556)',
        },
        8L: {
            'id': u'5',
            'description': u'Prefix sign 2',
            'lon': u'N1 05 30.6',
            'lat': u'E035 51 20',
            '#fid': 8L,
            '#geometry': 'Point (1.09183333 35.85555556)',
        },
        9L: {
            'id': u'6',
            'description': u'Prefix sign 3',
            'lon': u'N 1 05 30.6',
            'lat': u'E 035 51 20',
            '#fid': 9L,
            '#geometry': 'Point (1.09183333 35.85555556)',
        },
        10L: {
            'id': u'7',
            'description': u'Prefix sign 4',
            'lon': u'S1 05 30.6',
            'lat': u'W035 51 20',
            '#fid': 10L,
            '#geometry': 'Point (-1.09183333 -35.85555556)',
        },
        11L: {
            'id': u'8',
            'description': u'Prefix sign 5',
            'lon': u'+1 05 30.6',
            'lat': u'+035 51 20',
            '#fid': 11L,
            '#geometry': 'Point (1.09183333 35.85555556)',
        },
        12L: {
            'id': u'9',
            'description': u'Prefix sign 6',
            'lon': u'-1 05 30.6',
            'lat': u'-035 51 20',
            '#fid': 12L,
            '#geometry': 'Point (-1.09183333 -35.85555556)',
        },
        14L: {
            'id': u'10',
            'description': u'Postfix sign 1',
            'lon': u'1 05 30.6n',
            'lat': u'035 51 20e',
            '#fid': 14L,
            '#geometry': 'Point (1.09183333 35.85555556)',
        },
        15L: {
            'id': u'11',
            'description': u'Postfix sign 2',
            'lon': u'1 05 30.6N',
            'lat': u'035 51 20E',
            '#fid': 15L,
            '#geometry': 'Point (1.09183333 35.85555556)',
        },
        16L: {
            'id': u'12',
            'description': u'Postfix sign 3',
            'lon': u'1 05 30.6 N',
            'lat': u'035 51 20 E',
            '#fid': 16L,
            '#geometry': 'Point (1.09183333 35.85555556)',
        },
        17L: {
            'id': u'13',
            'description': u'Postfix sign 4',
            'lon': u'1 05 30.6S',
            'lat': u'035 51 20W',
            '#fid': 17L,
            '#geometry': 'Point (-1.09183333 -35.85555556)',
        },
        18L: {
            'id': u'14',
            'description': u'Postfix sign 5',
            'lon': u'1 05 30.6+',
            'lat': u'035 51 20+',
            '#fid': 18L,
            '#geometry': 'Point (1.09183333 35.85555556)',
        },
        19L: {
            'id': u'15',
            'description': u'Postfix sign 6',
            'lon': u'1 05 30.6-',
            'lat': u'035 51 20-',
            '#fid': 19L,
            '#geometry': 'Point (-1.09183333 -35.85555556)',
        },
        21L: {
            'id': u'16',
            'description': u'Leading and trailing blanks 1',
            'lon': u'   1 05 30.6',
            'lat': u'035 51 20   ',
            '#fid': 21L,
            '#geometry': 'Point (1.09183333 35.85555556)',
        },
        22L: {
            'id': u'17',
            'description': u'Leading and trailing blanks 2',
            'lon': u' N  1 05 30.6',
            'lat': u'035 51 20 E  ',
            '#fid': 22L,
            '#geometry': 'Point (1.09183333 35.85555556)',
        },
        24L: {
            'id': u'18',
            'description': u'Alternative characters for D,M,S',
            'lon': u'1d05m30.6s S',
            'lat': u"35d51'20",
            '#fid': 24L,
            '#geometry': 'Point (-1.09183333 35.85555556)',
        },
        25L: {
            'id': u'19',
            'description': u'Degrees/minutes format',
            'lon': u'1 05.23',
            'lat': u'4 55.03',
            '#fid': 25L,
            '#geometry': 'Point (1.08716667 4.91716667)',
        },
    }
    wanted['log']=[
        u'Errors in file testdms.csv',
        u'5 records discarded due to invalid geometry definitions',
        u'The following lines were not loaded into QGIS due to errors:',
        u'Invalid X or Y fields at line 27',
        u'Invalid X or Y fields at line 28',
        u'Invalid X or Y fields at line 29',
        u'Invalid X or Y fields at line 30',
        u'Invalid X or Y fields at line 31',
    ]
    return wanted


def test_016_decimal_point():
    wanted={}
    wanted['uri']=u'file://testdp.csv?yField=geom_y&xField=geom_x&type=csv&delimiter=;&decimalPoint=,'
    wanted['fieldTypes']=['integer', 'text', 'double', 'double', 'double', 'text']
    wanted['data']={
        2L: {
            'id': u'1',
            'description': u'Comma as decimal point 1',
            'geom_x': u'10.0',
            'geom_y': u'20.0',
            'other': u'30.0',
            'text field': u'Field with , in it',
            '#fid': 2L,
            '#geometry': 'Point (10 20)',
        },
        3L: {
            'id': u'2',
            'description': u'Comma as decimal point 2',
            'geom_x': u'12.0',
            'geom_y': u'25.003',
            'other': u'-38.55',
            'text field': u'Plain text field',
            '#fid': 3L,
            '#geometry': 'Point (12 25.003)',
        },
    }
    wanted['log']=[]
    return wanted


def test_017_regular_expression_1():
    wanted={}
    wanted['uri']=u'file://testre.txt?geomType=none&trimFields=Y&delimiter=RE(?:GEXP)?&type=regexp'
    wanted['fieldTypes']=['integer', 'text', 'text', 'text']
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
    wanted['log']=[]
    return wanted


def test_018_regular_expression_2():
    wanted={}
    wanted['uri']=u'file://testre.txt?geomType=none&trimFields=Y&delimiter=(RE)(GEXP)?&type=regexp'
    wanted['fieldTypes']=['integer', 'text', 'text', 'text', 'text', 'text', 'text', 'text', 'text', 'text']
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
            'GEXP_1': u'NULL',
            'data': u'data2',
            'RE_2': u'RE',
            'GEXP_2': u'NULL',
            'info': u'info2',
            '#fid': 3L,
            '#geometry': 'None',
        },
    }
    wanted['log']=[]
    return wanted


def test_019_regular_expression_3():
    wanted={}
    wanted['uri']=u'file://testre2.txt?geomType=none&trimFields=Y&delimiter=^(.{5})(.{30})(.{5,})&type=regexp'
    wanted['fieldTypes']=['integer', 'text', 'text']
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
        u'Errors in file testre2.txt',
        u'1 records discarded due to invalid format',
        u'The following lines were not loaded into QGIS due to errors:',
        u'Invalid record format at line 3',
    ]
    return wanted


def test_020_regular_expression_4():
    wanted={}
    wanted['uri']=u'file://testre3.txt?geomType=none&delimiter=x?&type=regexp'
    wanted['fieldTypes']=['text', 'text', 'text', 'text', 'text', 'text', 'text']
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
    wanted['log']=[]
    return wanted


def test_021_regular_expression_5():
    wanted={}
    wanted['uri']=u'file://testre3.txt?geomType=none&delimiter=\\b&type=regexp'
    wanted['fieldTypes']=['text', 'text', 'text']
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
    wanted['log']=[]
    return wanted


def test_022_utf8_encoded_file():
    wanted={}
    wanted['uri']=u'file://testutf8.csv?geomType=none&delimiter=|&type=csv&encoding=utf-8'
    wanted['fieldTypes']=['integer', 'text', 'text']
    wanted['data']={
        2L: {
            'id': u'1',
            'description': u'Correctly read UTF8 encoding',
            'name': u'Field has \u0101cc\xe8nt\xe9d text',
            '#fid': 2L,
            '#geometry': 'None',
        },
    }
    wanted['log']=[]
    return wanted


def test_023_latin1_encoded_file():
    wanted={}
    wanted['uri']=u'file://testlatin1.csv?geomType=none&delimiter=|&type=csv&encoding=latin1'
    wanted['fieldTypes']=['integer', 'text', 'text']
    wanted['data']={
        2L: {
            'id': u'1',
            'description': u'Correctly read latin1 encoding',
            'name': u'This test is \xa9',
            '#fid': 2L,
            '#geometry': 'None',
        },
    }
    wanted['log']=[]
    return wanted


def test_024_filter_rect_xy():
    wanted={}
    wanted['uri']=u'file://testextpt.txt?yField=y&delimiter=|&type=csv&xField=x'
    wanted['fieldTypes']=['integer', 'text', 'integer', 'integer']
    wanted['data']={
        2L: {
            'id': u'1',
            'description': u'Inside',
            'x': u'15',
            'y': u'35',
            '#fid': 2L,
            '#geometry': 'Point (15 35)',
        },
        10L: {
            'id': u'9',
            'description': u'Inside 2',
            'x': u'25',
            'y': u'45',
            '#fid': 10L,
            '#geometry': 'Point (25 45)',
        },
        1002L: {
            'id': u'1',
            'description': u'Inside',
            'x': u'15',
            'y': u'35',
            '#fid': 2L,
            '#geometry': 'Point (15 35)',
        },
        1010L: {
            'id': u'9',
            'description': u'Inside 2',
            'x': u'25',
            'y': u'45',
            '#fid': 10L,
            '#geometry': 'Point (25 45)',
        },
    }
    wanted['log']=[
        'Request 2 did not return any data',
    ]
    return wanted


def test_025_filter_rect_wkt():
    wanted={}
    wanted['uri']=u'file://testextw.txt?delimiter=|&type=csv&wktField=wkt'
    wanted['fieldTypes']=['integer', 'text']
    wanted['data']={
        2L: {
            'id': u'1',
            'description': u'Inside',
            '#fid': 2L,
            '#geometry': 'LineString (12 32, 28 48)',
        },
        4L: {
            'id': u'3',
            'description': u'Crossing',
            '#fid': 4L,
            '#geometry': 'LineString (5 30, 30 55)',
        },
        5L: {
            'id': u'4',
            'description': u'Bounding box overlap',
            '#fid': 5L,
            '#geometry': 'LineString (5 30, 5 55, 30 55)',
        },
        6L: {
            'id': u'5',
            'description': u'Crossing 2',
            '#fid': 6L,
            '#geometry': 'LineString (25 35, 35 35)',
        },
        7L: {
            'id': u'6',
            'description': u'Bounding box overlap 2',
            '#fid': 7L,
            '#geometry': 'LineString (28 29, 31 29, 31 33)',
        },
        1002L: {
            'id': u'1',
            'description': u'Inside',
            '#fid': 2L,
            '#geometry': 'LineString (12 32, 28 48)',
        },
        1004L: {
            'id': u'3',
            'description': u'Crossing',
            '#fid': 4L,
            '#geometry': 'LineString (5 30, 30 55)',
        },
        1006L: {
            'id': u'5',
            'description': u'Crossing 2',
            '#fid': 6L,
            '#geometry': 'LineString (25 35, 35 35)',
        },
    }
    wanted['log']=[
        'Request 2 did not return any data',
    ]
    return wanted


def test_026_filter_fid():
    wanted={}
    wanted['uri']=u'file://test.csv?geomType=none&type=csv'
    wanted['fieldTypes']=['integer', 'text', 'text', 'text', 'text']
    wanted['data']={
        3L: {
            'id': u'2',
            'description': u'Quoted field',
            'data': u'Quoted data',
            'info': u'Unquoted',
            'field_5': u'NULL',
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
        3003L: {
            'id': u'2',
            'description': u'Quoted field',
            'data': u'Quoted data',
            'info': u'Unquoted',
            'field_5': u'NULL',
            '#fid': 3L,
            '#geometry': 'None',
        },
    }
    wanted['log']=[
        'Request 2 did not return any data',
    ]
    return wanted


def test_027_filter_attributes():
    wanted={}
    wanted['uri']=u'file://test.csv?geomType=none&type=csv'
    wanted['fieldTypes']=['integer', 'text', 'text', 'text', 'text']
    wanted['data']={
        2L: {
            'id': u'None',
            'description': u'Basic unquoted record',
            'data': u'None',
            'info': u'Some info',
            'field_5': u'None',
            '#fid': 2L,
            '#geometry': 'None',
        },
        3L: {
            'id': u'None',
            'description': u'Quoted field',
            'data': u'None',
            'info': u'Unquoted',
            'field_5': u'None',
            '#fid': 3L,
            '#geometry': 'None',
        },
        4L: {
            'id': u'None',
            'description': u'Escaped quotes',
            'data': u'None',
            'info': u'Unquoted',
            'field_5': u'None',
            '#fid': 4L,
            '#geometry': 'None',
        },
        5L: {
            'id': u'None',
            'description': u'Quoted newlines',
            'data': u'None',
            'info': u'No data',
            'field_5': u'None',
            '#fid': 5L,
            '#geometry': 'None',
        },
        9L: {
            'id': u'None',
            'description': u'Extra fields',
            'data': u'None',
            'info': u'info',
            'field_5': u'None',
            '#fid': 9L,
            '#geometry': 'None',
        },
        10L: {
            'id': u'None',
            'description': u'Missing fields',
            'data': u'None',
            'info': u'NULL',
            'field_5': u'None',
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
            'id': u'None',
            'description': u'Extra fields',
            'data': u'None',
            'info': u'info',
            'field_5': u'None',
            '#fid': 9L,
            '#geometry': 'None',
        },
        3009L: {
            'id': u'None',
            'description': u'Extra fields',
            'data': u'None',
            'info': u'info',
            'field_5': u'None',
            '#fid': 9L,
            '#geometry': 'None',
        },
        4009L: {
            'id': u'None',
            'description': u'Extra fields',
            'data': u'None',
            'info': u'info',
            'field_5': u'None',
            '#fid': 9L,
            '#geometry': 'None',
        },
        5009L: {
            'id': u'None',
            'description': u'None',
            'data': u'None',
            'info': u'None',
            'field_5': u'None',
            '#fid': 9L,
            '#geometry': 'None',
        },
    }
    wanted['log']=[]
    return wanted


def test_028_substring_test():
    wanted={}
    wanted['uri']=u'file://test.csv?geomType=none&type=csv&subset=id%20%25%202%20%3D%201'
    wanted['fieldTypes']=['integer', 'text', 'text', 'text', 'text']
    wanted['data']={
        2L: {
            'id': u'1',
            'description': u'Basic unquoted record',
            'data': u'Some data',
            'info': u'Some info',
            'field_5': u'NULL',
            '#fid': 2L,
            '#geometry': 'None',
        },
        4L: {
            'id': u'3',
            'description': u'Escaped quotes',
            'data': u'Quoted "citation" data',
            'info': u'Unquoted',
            'field_5': u'NULL',
            '#fid': 4L,
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
    }
    wanted['log']=[]
    return wanted


def test_029_file_watcher():
    wanted={}
    wanted['uri']=u'file://file?geomType=none&type=csv&watchFile=yes'
    wanted['fieldTypes']=['integer', 'text']
    wanted['data']={
        3L: {
            'id': u'2',
            'description': u'pooh',
            'name': u'pooh',
            '#fid': 3L,
            '#geometry': 'None',
        },
        1002L: {
            'id': u'1',
            'description': u'rabbit',
            'name': u'rabbit',
            '#fid': 2L,
            '#geometry': 'None',
        },
        1003L: {
            'id': u'2',
            'description': u'pooh',
            'name': u'pooh',
            '#fid': 3L,
            '#geometry': 'None',
        },
        4003L: {
            'id': u'2',
            'description': u'pooh',
            'name': u'pooh',
            '#fid': 3L,
            '#geometry': 'None',
        },
        5004L: {
            'id': u'3',
            'description': u'tigger',
            'name': u'tigger',
            '#fid': 4L,
            '#geometry': 'None',
        },
        6002L: {
            'id': u'1',
            'description': u'rabbit',
            'name': u'rabbit',
            '#fid': 2L,
            '#geometry': 'None',
        },
        6003L: {
            'id': u'2',
            'description': u'pooh',
            'name': u'pooh',
            '#fid': 3L,
            '#geometry': 'None',
        },
        6004L: {
            'id': u'3',
            'description': u'tigger',
            'name': u'tigger',
            '#fid': 4L,
            '#geometry': 'None',
        },
        9002L: {
            'id': u'5',
            'description': u'toad',
            'name': u'toad',
            '#fid': 2L,
            '#geometry': 'None',
        },
        10002L: {
            'id': u'5',
            'description': u'toad',
            'name': u'toad',
            '#fid': 2L,
            '#geometry': 'None',
        },
        10003L: {
            'id': u'6',
            'description': u'mole',
            'name': u'mole',
            '#fid': 3L,
            '#geometry': 'None',
        },
        10004L: {
            'id': u'7',
            'description': u'badger',
            'name': u'badger',
            '#fid': 4L,
            '#geometry': 'None',
        },
        16002L: {
            'id': u'5',
            'description': u'toad',
            'name': u'toad',
            '#fid': 2L,
            '#geometry': 'None',
        },
    }
    wanted['log']=[
        'Request 2 did not return any data',
        'Request 7 did not return any data',
        'Request 11 did not return any data',
        'Request 13 did not return any data',
        'Request 14 did not return any data',
        u'Errors in file temp_file',
        u'The file has been updated by another application - reloading',
        u'Errors in file temp_file',
        u'The file has been updated by another application - reloading',
        u'Errors in file temp_file',
        u'The file has been updated by another application - reloading',
    ]
    return wanted


def test_030_filter_rect_xy_spatial_index():
    wanted={}
    wanted['uri']=u'file://testextpt.txt?spatialIndex=Y&yField=y&delimiter=|&type=csv&xField=x'
    wanted['fieldTypes']=['integer', 'text', 'integer', 'integer']
    wanted['data']={
        2L: {
            'id': u'1',
            'description': u'Inside',
            'x': u'15',
            'y': u'35',
            '#fid': 2L,
            '#geometry': 'Point (15 35)',
        },
        10L: {
            'id': u'9',
            'description': u'Inside 2',
            'x': u'25',
            'y': u'45',
            '#fid': 10L,
            '#geometry': 'Point (25 45)',
        },
        1002L: {
            'id': u'1',
            'description': u'Inside',
            'x': u'15',
            'y': u'35',
            '#fid': 2L,
            '#geometry': 'Point (15 35)',
        },
        1010L: {
            'id': u'9',
            'description': u'Inside 2',
            'x': u'25',
            'y': u'45',
            '#fid': 10L,
            '#geometry': 'Point (25 45)',
        },
        3002L: {
            'id': u'1',
            'description': u'Inside',
            'x': u'15',
            'y': u'35',
            '#fid': 2L,
            '#geometry': 'Point (15 35)',
        },
        3003L: {
            'id': u'2',
            'description': u'Outside 1',
            'x': u'5',
            'y': u'35',
            '#fid': 3L,
            '#geometry': 'Point (5 35)',
        },
        3004L: {
            'id': u'3',
            'description': u'Outside 2',
            'x': u'5',
            'y': u'55',
            '#fid': 4L,
            '#geometry': 'Point (5 55)',
        },
        3005L: {
            'id': u'4',
            'description': u'Outside 3',
            'x': u'15',
            'y': u'55',
            '#fid': 5L,
            '#geometry': 'Point (15 55)',
        },
        3006L: {
            'id': u'5',
            'description': u'Outside 4',
            'x': u'35',
            'y': u'55',
            '#fid': 6L,
            '#geometry': 'Point (35 55)',
        },
        3007L: {
            'id': u'6',
            'description': u'Outside 5',
            'x': u'35',
            'y': u'45',
            '#fid': 7L,
            '#geometry': 'Point (35 45)',
        },
        3008L: {
            'id': u'7',
            'description': u'Outside 7',
            'x': u'35',
            'y': u'25',
            '#fid': 8L,
            '#geometry': 'Point (35 25)',
        },
        3009L: {
            'id': u'8',
            'description': u'Outside 8',
            'x': u'15',
            'y': u'25',
            '#fid': 9L,
            '#geometry': 'Point (15 25)',
        },
        3010L: {
            'id': u'9',
            'description': u'Inside 2',
            'x': u'25',
            'y': u'45',
            '#fid': 10L,
            '#geometry': 'Point (25 45)',
        },
        4002L: {
            'id': u'1',
            'description': u'Inside',
            'x': u'15',
            'y': u'35',
            '#fid': 2L,
            '#geometry': 'Point (15 35)',
        },
        4003L: {
            'id': u'2',
            'description': u'Outside 1',
            'x': u'5',
            'y': u'35',
            '#fid': 3L,
            '#geometry': 'Point (5 35)',
        },
        4004L: {
            'id': u'3',
            'description': u'Outside 2',
            'x': u'5',
            'y': u'55',
            '#fid': 4L,
            '#geometry': 'Point (5 55)',
        },
        4005L: {
            'id': u'4',
            'description': u'Outside 3',
            'x': u'15',
            'y': u'55',
            '#fid': 5L,
            '#geometry': 'Point (15 55)',
        },
        4006L: {
            'id': u'5',
            'description': u'Outside 4',
            'x': u'35',
            'y': u'55',
            '#fid': 6L,
            '#geometry': 'Point (35 55)',
        },
        4007L: {
            'id': u'6',
            'description': u'Outside 5',
            'x': u'35',
            'y': u'45',
            '#fid': 7L,
            '#geometry': 'Point (35 45)',
        },
        4008L: {
            'id': u'7',
            'description': u'Outside 7',
            'x': u'35',
            'y': u'25',
            '#fid': 8L,
            '#geometry': 'Point (35 25)',
        },
        4009L: {
            'id': u'8',
            'description': u'Outside 8',
            'x': u'15',
            'y': u'25',
            '#fid': 9L,
            '#geometry': 'Point (15 25)',
        },
        4010L: {
            'id': u'9',
            'description': u'Inside 2',
            'x': u'25',
            'y': u'45',
            '#fid': 10L,
            '#geometry': 'Point (25 45)',
        },
    }
    wanted['log']=[
        'Request 2 did not return any data',
    ]
    return wanted


def test_031_filter_rect_wkt_spatial_index():
    wanted={}
    wanted['uri']=u'file://testextw.txt?spatialIndex=Y&delimiter=|&type=csv&wktField=wkt'
    wanted['fieldTypes']=['integer', 'text']
    wanted['data']={
        2L: {
            'id': u'1',
            'description': u'Inside',
            '#fid': 2L,
            '#geometry': 'LineString (12 32, 28 48)',
        },
        4L: {
            'id': u'3',
            'description': u'Crossing',
            '#fid': 4L,
            '#geometry': 'LineString (5 30, 30 55)',
        },
        5L: {
            'id': u'4',
            'description': u'Bounding box overlap',
            '#fid': 5L,
            '#geometry': 'LineString (5 30, 5 55, 30 55)',
        },
        6L: {
            'id': u'5',
            'description': u'Crossing 2',
            '#fid': 6L,
            '#geometry': 'LineString (25 35, 35 35)',
        },
        7L: {
            'id': u'6',
            'description': u'Bounding box overlap 2',
            '#fid': 7L,
            '#geometry': 'LineString (28 29, 31 29, 31 33)',
        },
        1002L: {
            'id': u'1',
            'description': u'Inside',
            '#fid': 2L,
            '#geometry': 'LineString (12 32, 28 48)',
        },
        1004L: {
            'id': u'3',
            'description': u'Crossing',
            '#fid': 4L,
            '#geometry': 'LineString (5 30, 30 55)',
        },
        1006L: {
            'id': u'5',
            'description': u'Crossing 2',
            '#fid': 6L,
            '#geometry': 'LineString (25 35, 35 35)',
        },
        3002L: {
            'id': u'1',
            'description': u'Inside',
            '#fid': 2L,
            '#geometry': 'LineString (12 32, 28 48)',
        },
        3003L: {
            'id': u'2',
            'description': u'Outside',
            '#fid': 3L,
            '#geometry': 'LineString (0 0, 0 10)',
        },
        3004L: {
            'id': u'3',
            'description': u'Crossing',
            '#fid': 4L,
            '#geometry': 'LineString (5 30, 30 55)',
        },
        3005L: {
            'id': u'4',
            'description': u'Bounding box overlap',
            '#fid': 5L,
            '#geometry': 'LineString (5 30, 5 55, 30 55)',
        },
        3006L: {
            'id': u'5',
            'description': u'Crossing 2',
            '#fid': 6L,
            '#geometry': 'LineString (25 35, 35 35)',
        },
        3007L: {
            'id': u'6',
            'description': u'Bounding box overlap 2',
            '#fid': 7L,
            '#geometry': 'LineString (28 29, 31 29, 31 33)',
        },
        4002L: {
            'id': u'1',
            'description': u'Inside',
            '#fid': 2L,
            '#geometry': 'LineString (12 32, 28 48)',
        },
        4003L: {
            'id': u'2',
            'description': u'Outside',
            '#fid': 3L,
            '#geometry': 'LineString (0 0, 0 10)',
        },
        4004L: {
            'id': u'3',
            'description': u'Crossing',
            '#fid': 4L,
            '#geometry': 'LineString (5 30, 30 55)',
        },
        4005L: {
            'id': u'4',
            'description': u'Bounding box overlap',
            '#fid': 5L,
            '#geometry': 'LineString (5 30, 5 55, 30 55)',
        },
        4006L: {
            'id': u'5',
            'description': u'Crossing 2',
            '#fid': 6L,
            '#geometry': 'LineString (25 35, 35 35)',
        },
        4007L: {
            'id': u'6',
            'description': u'Bounding box overlap 2',
            '#fid': 7L,
            '#geometry': 'LineString (28 29, 31 29, 31 33)',
        },
    }
    wanted['log']=[
        'Request 2 did not return any data',
    ]
    return wanted


def test_032_filter_rect_wkt_create_spatial_index():
    wanted={}
    wanted['uri']=u'file://testextw.txt?delimiter=|&type=csv&wktField=wkt'
    wanted['fieldTypes']=['integer', 'text']
    wanted['data']={
        2L: {
            'id': u'1',
            'description': u'Inside',
            '#fid': 2L,
            '#geometry': 'LineString (12 32, 28 48)',
        },
        4L: {
            'id': u'3',
            'description': u'Crossing',
            '#fid': 4L,
            '#geometry': 'LineString (5 30, 30 55)',
        },
        5L: {
            'id': u'4',
            'description': u'Bounding box overlap',
            '#fid': 5L,
            '#geometry': 'LineString (5 30, 5 55, 30 55)',
        },
        6L: {
            'id': u'5',
            'description': u'Crossing 2',
            '#fid': 6L,
            '#geometry': 'LineString (25 35, 35 35)',
        },
        7L: {
            'id': u'6',
            'description': u'Bounding box overlap 2',
            '#fid': 7L,
            '#geometry': 'LineString (28 29, 31 29, 31 33)',
        },
        1002L: {
            'id': u'1',
            'description': u'Inside',
            '#fid': 2L,
            '#geometry': 'LineString (12 32, 28 48)',
        },
        1003L: {
            'id': u'2',
            'description': u'Outside',
            '#fid': 3L,
            '#geometry': 'LineString (0 0, 0 10)',
        },
        1004L: {
            'id': u'3',
            'description': u'Crossing',
            '#fid': 4L,
            '#geometry': 'LineString (5 30, 30 55)',
        },
        1005L: {
            'id': u'4',
            'description': u'Bounding box overlap',
            '#fid': 5L,
            '#geometry': 'LineString (5 30, 5 55, 30 55)',
        },
        1006L: {
            'id': u'5',
            'description': u'Crossing 2',
            '#fid': 6L,
            '#geometry': 'LineString (25 35, 35 35)',
        },
        1007L: {
            'id': u'6',
            'description': u'Bounding box overlap 2',
            '#fid': 7L,
            '#geometry': 'LineString (28 29, 31 29, 31 33)',
        },
        3002L: {
            'id': u'1',
            'description': u'Inside',
            '#fid': 2L,
            '#geometry': 'LineString (12 32, 28 48)',
        },
        3004L: {
            'id': u'3',
            'description': u'Crossing',
            '#fid': 4L,
            '#geometry': 'LineString (5 30, 30 55)',
        },
        3005L: {
            'id': u'4',
            'description': u'Bounding box overlap',
            '#fid': 5L,
            '#geometry': 'LineString (5 30, 5 55, 30 55)',
        },
        3006L: {
            'id': u'5',
            'description': u'Crossing 2',
            '#fid': 6L,
            '#geometry': 'LineString (25 35, 35 35)',
        },
        3007L: {
            'id': u'6',
            'description': u'Bounding box overlap 2',
            '#fid': 7L,
            '#geometry': 'LineString (28 29, 31 29, 31 33)',
        },
        4002L: {
            'id': u'1',
            'description': u'Inside',
            '#fid': 2L,
            '#geometry': 'LineString (12 32, 28 48)',
        },
        4004L: {
            'id': u'3',
            'description': u'Crossing',
            '#fid': 4L,
            '#geometry': 'LineString (5 30, 30 55)',
        },
        4006L: {
            'id': u'5',
            'description': u'Crossing 2',
            '#fid': 6L,
            '#geometry': 'LineString (25 35, 35 35)',
        },
        6002L: {
            'id': u'1',
            'description': u'Inside',
            '#fid': 2L,
            '#geometry': 'LineString (12 32, 28 48)',
        },
        6003L: {
            'id': u'2',
            'description': u'Outside',
            '#fid': 3L,
            '#geometry': 'LineString (0 0, 0 10)',
        },
        6004L: {
            'id': u'3',
            'description': u'Crossing',
            '#fid': 4L,
            '#geometry': 'LineString (5 30, 30 55)',
        },
        6005L: {
            'id': u'4',
            'description': u'Bounding box overlap',
            '#fid': 5L,
            '#geometry': 'LineString (5 30, 5 55, 30 55)',
        },
        6006L: {
            'id': u'5',
            'description': u'Crossing 2',
            '#fid': 6L,
            '#geometry': 'LineString (25 35, 35 35)',
        },
        6007L: {
            'id': u'6',
            'description': u'Bounding box overlap 2',
            '#fid': 7L,
            '#geometry': 'LineString (28 29, 31 29, 31 33)',
        },
        7002L: {
            'id': u'1',
            'description': u'Inside',
            '#fid': 2L,
            '#geometry': 'LineString (12 32, 28 48)',
        },
        7003L: {
            'id': u'2',
            'description': u'Outside',
            '#fid': 3L,
            '#geometry': 'LineString (0 0, 0 10)',
        },
        7004L: {
            'id': u'3',
            'description': u'Crossing',
            '#fid': 4L,
            '#geometry': 'LineString (5 30, 30 55)',
        },
        7005L: {
            'id': u'4',
            'description': u'Bounding box overlap',
            '#fid': 5L,
            '#geometry': 'LineString (5 30, 5 55, 30 55)',
        },
        7006L: {
            'id': u'5',
            'description': u'Crossing 2',
            '#fid': 6L,
            '#geometry': 'LineString (25 35, 35 35)',
        },
        7007L: {
            'id': u'6',
            'description': u'Bounding box overlap 2',
            '#fid': 7L,
            '#geometry': 'LineString (28 29, 31 29, 31 33)',
        },
    }
    wanted['log']=[
        'Request 5 did not return any data',
    ]
    return wanted


def test_033_reset_subset_string():
    wanted={}
    wanted['uri']=u'file://test.csv?geomType=none&type=csv'
    wanted['fieldTypes']=['integer', 'text', 'text', 'text', 'text']
    wanted['data']={
        2L: {
            'id': u'1',
            'description': u'Basic unquoted record',
            'data': u'Some data',
            'info': u'Some info',
            'field_5': u'NULL',
            '#fid': 2L,
            '#geometry': 'None',
        },
        3L: {
            'id': u'2',
            'description': u'Quoted field',
            'data': u'Quoted data',
            'info': u'Unquoted',
            'field_5': u'NULL',
            '#fid': 3L,
            '#geometry': 'None',
        },
        4L: {
            'id': u'3',
            'description': u'Escaped quotes',
            'data': u'Quoted "citation" data',
            'info': u'Unquoted',
            'field_5': u'NULL',
            '#fid': 4L,
            '#geometry': 'None',
        },
        5L: {
            'id': u'4',
            'description': u'Quoted newlines',
            'data': u'Line 1\nLine 2\n\nLine 4',
            'info': u'No data',
            'field_5': u'NULL',
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
            'data': u'NULL',
            'info': u'NULL',
            'field_5': u'NULL',
            '#fid': 10L,
            '#geometry': 'None',
        },
        2002L: {
            'id': u'1',
            'description': u'Basic unquoted record',
            'data': u'Some data',
            'info': u'Some info',
            'field_5': u'NULL',
            '#fid': 2L,
            '#geometry': 'None',
        },
        2004L: {
            'id': u'3',
            'description': u'Escaped quotes',
            'data': u'Quoted "citation" data',
            'info': u'Unquoted',
            'field_5': u'NULL',
            '#fid': 4L,
            '#geometry': 'None',
        },
        2009L: {
            'id': u'5',
            'description': u'Extra fields',
            'data': u'data',
            'info': u'info',
            'field_5': u'message',
            '#fid': 9L,
            '#geometry': 'None',
        },
        4010L: {
            'id': u'6',
            'description': u'Missing fields',
            'data': u'NULL',
            'info': u'NULL',
            'field_5': u'NULL',
            '#fid': 10L,
            '#geometry': 'None',
        },
        6004L: {
            'id': u'3',
            'description': u'Escaped quotes',
            'data': u'Quoted "citation" data',
            'info': u'Unquoted',
            'field_5': u'NULL',
            '#fid': 4L,
            '#geometry': 'None',
        },
        8002L: {
            'id': u'1',
            'description': u'Basic unquoted record',
            'data': u'Some data',
            'info': u'Some info',
            'field_5': u'NULL',
            '#fid': 2L,
            '#geometry': 'None',
        },
        8004L: {
            'id': u'3',
            'description': u'Escaped quotes',
            'data': u'Quoted "citation" data',
            'info': u'Unquoted',
            'field_5': u'NULL',
            '#fid': 4L,
            '#geometry': 'None',
        },
        8009L: {
            'id': u'5',
            'description': u'Extra fields',
            'data': u'data',
            'info': u'info',
            'field_5': u'message',
            '#fid': 9L,
            '#geometry': 'None',
        },
        10003L: {
            'id': u'2',
            'description': u'Quoted field',
            'data': u'Quoted data',
            'info': u'Unquoted',
            'field_5': u'NULL',
            '#fid': 3L,
            '#geometry': 'None',
        },
        10005L: {
            'id': u'4',
            'description': u'Quoted newlines',
            'data': u'Line 1\nLine 2\n\nLine 4',
            'info': u'No data',
            'field_5': u'NULL',
            '#fid': 5L,
            '#geometry': 'None',
        },
        10010L: {
            'id': u'6',
            'description': u'Missing fields',
            'data': u'NULL',
            'info': u'NULL',
            'field_5': u'NULL',
            '#fid': 10L,
            '#geometry': 'None',
        },
    }
    wanted['log']=[]
    return wanted

def test_034_csvt_file():
    wanted={}
    wanted['uri']=u'file://testcsvt.csv?geomType=none&type=csv'
    wanted['fieldTypes']=['integer', 'text', 'integer', 'double', 'text', 'text', 'text', 'text', 'text', 'text', 'longlong', 'longlong']
    wanted['data']={
        2L: {
            'id': u'1',
            'description': u'Test csvt 1',
            'fint': u'1',
            'freal': u'1.2',
            'fstr': u'1',
            'fstr_1': u'text',
            'fdatetime': u'2015-03-02T12:30:00',
            'fdate': u'2014-12-30',
            'ftime': u'23:55',
            'flong': u'-456',
            'flonglong': u'-678',
            'field_12': u'NULL',
            '#fid': 2L,
            '#geometry': 'None',
        },
        3L: {
            'id': u'2',
            'description': u'Test csvt 2',
            'fint': u'3',
            'freal': u'1.5',
            'fstr': u'99',
            'fstr_1': u'23.5',
            'fdatetime': u'80',
            'fdate': u'2015-03-28',
            'ftime': u'2014-12-30',
            'flong': u'01:55',
            'flonglong': u'9189304972279762602',
            'field_12': u'-3123724580211819352',
            '#fid': 3L,
            '#geometry': 'None',
        },
    }
    wanted['log']=[]
    return wanted


def test_035_csvt_file2():
    wanted={}
    wanted['uri']=u'file://testcsvt2.txt?geomType=none&delimiter=|&type=csv'
    wanted['fieldTypes']=['integer', 'text', 'integer', 'double', 'integer', 'text', 'integer']
    wanted['data']={
        2L: {
            'id': u'1',
            'description': u'Test csvt 1',
            'f1': u'1',
            'f2': u'1.2',
            'f3': u'1',
            'f4': u'text',
            'f5': u'NULL',
            '#fid': 2L,
            '#geometry': 'None',
        },
        3L: {
            'id': u'2',
            'description': u'Test csvt 2',
            'f1': u'3',
            'f2': u'1.5',
            'f3': u'99',
            'f4': u'23.5',
            'f5': u'80',
            '#fid': 3L,
            '#geometry': 'None',
        },
    }
    wanted['log']=[]
    return wanted


def test_036_csvt_file_invalid_types():
    wanted={}
    wanted['uri']=u'file://testcsvt3.csv?geomType=none&type=csv'
    wanted['fieldTypes']=['integer', 'text', 'integer', 'double', 'integer', 'text', 'text']
    wanted['data']={
        2L: {
            'id': u'1',
            'description': u'Test csvt 1',
            'f1': u'1',
            'f2': u'1.2',
            'f3': u'1',
            'f4': u'text',
            'f5': u'times',
            '#fid': 2L,
            '#geometry': 'None',
        },
        3L: {
            'id': u'2',
            'description': u'Test csvt 2',
            'f1': u'3',
            'f2': u'1.5',
            'f3': u'99',
            'f4': u'23.5',
            'f5': u'80',
            '#fid': 3L,
            '#geometry': 'None',
        },
    }
    wanted['log']=[
        u'Errors in file testcsvt3.csv',
        u'File type string in testcsvt3.csvt is not correctly formatted',
    ]
    return wanted


def test_037_csvt_file_invalid_file():
    wanted={}
    wanted['uri']=u'file://testcsvt4.csv?geomType=none&type=csv'
    wanted['fieldTypes']=['integer', 'text', 'integer', 'double', 'integer', 'text', 'text']
    wanted['data']={
        2L: {
            'id': u'1',
            'description': u'Test csvt 1',
            'f1': u'1',
            'f2': u'1.2',
            'f3': u'1',
            'f4': u'text',
            'f5': u'times',
            '#fid': 2L,
            '#geometry': 'None',
        },
        3L: {
            'id': u'2',
            'description': u'Test csvt 2',
            'f1': u'3',
            'f2': u'1.5',
            'f3': u'99',
            'f4': u'23.5',
            'f5': u'80',
            '#fid': 3L,
            '#geometry': 'None',
        },
    }
    wanted['log']=[]
    return wanted

def test_038_type_inference():
    wanted={}
    wanted['uri']=u'file://testtypes.csv?yField=lat&xField=lon&type=csv'
    wanted['fieldTypes']=['text', 'double', 'double', 'text', 'text', 'integer', 'longlong', 'double', 'text']
    wanted['data']={
        2L: {
            'id': u'line1',
            'description': u'1.0',
            'lon': u'1.0',
            'lat': u'1.0',
            'empty': u'NULL',
            'text': u'NULL',
            'int': u'0',
            'longlong': u'0',
            'real': u'NULL',
            'text2': u'1',
            '#fid': 2L,
            '#geometry': 'Point (1 1)',
        },
        3L: {
            'id': u'line2',
            'description': u'1.0',
            'lon': u'1.0',
            'lat': u'5.0',
            'empty': u'NULL',
            'text': u'1',
            'int': u'NULL',
            'longlong': u'9189304972279762602',
            'real': u'1.3',
            'text2': u'-4',
            '#fid': 3L,
            '#geometry': 'Point (1 5)',
        },
        4L: {
            'id': u'line3',
            'description': u'5.0',
            'lon': u'5.0',
            'lat': u'5.0',
            'empty': u'NULL',
            'text': u'1xx',
            'int': u'2',
            'longlong': u'345',
            'real': u'2.0',
            'text2': u'1x',
            '#fid': 4L,
            '#geometry': 'Point (5 5)',
        },
        5L: {
            'id': u'line4',
            'description': u'5.0',
            'lon': u'5.0',
            'lat': u'1.0',
            'empty': u'NULL',
            'text': u'A string',
            'int': u'-3456',
            'longlong': u'-3123724580211819352',
            'real': u'-123.56',
            'text2': u'NULL',
            '#fid': 5L,
            '#geometry': 'Point (5 1)',
        },
        6L: {
            'id': u'line5',
            'description': u'3.0',
            'lon': u'3.0',
            'lat': u'1.0',
            'empty': u'NULL',
            'text': u'NULL',
            'int': u'NULL',
            'longlong': u'NULL',
            'real': u'0.00023',
            'text2': u'23',
            '#fid': 6L,
            '#geometry': 'Point (3 1)',
        },
        7L: {
            'id': u'line6',
            'description': u'1.0',
            'lon': u'1.0',
            'lat': u'3.0',
            'empty': u'NULL',
            'text': u'1.5',
            'int': u'9',
            'longlong': u'42',
            'real': u'99.0',
            'text2': u'0',
            '#fid': 7L,
            '#geometry': 'Point (1 3)',
        },
    }
    wanted['log']=[]
    return wanted
