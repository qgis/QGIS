
def test_002_load_csv_file():
    wanted={}
    wanted['uri']=u'file://test.csv?geomType=none&type=csv'
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
    return wanted


def test_003_field_naming():
    wanted={}
    wanted['uri']=u'file://testfields.csv?geomType=none&type=csv'
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
    return wanted


def test_004_max_fields():
    wanted={}
    wanted['uri']=u'file://testfields.csv?geomType=none&maxFields=7&type=csv'
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
    return wanted


def test_005_load_whitespace():
    wanted={}
    wanted['uri']=u'file://test.space?geomType=none&type=whitespace'
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
    return wanted


def test_006_quote_escape():
    wanted={}
    wanted['uri']=u'file://test.pipe?geomType=none&quote="&delimiter=|&escape=\\'
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
    return wanted


def test_007_multiple_quote():
    wanted={}
    wanted['uri']=u'file://test.quote?geomType=none&quote=\'"&type=csv&escape="\''
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
    return wanted


def test_008_badly_formed_quotes():
    wanted={}
    wanted['uri']=u'file://test.badquote?geomType=none&quote="&type=csv&escape="'
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
    return wanted


def test_009_skip_lines():
    wanted={}
    wanted['uri']=u'file://test2.csv?geomType=none&skipLines=2&type=csv&useHeader=no'
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
    return wanted


def test_010_read_coordinates():
    wanted={}
    wanted['uri']=u'file://testpt.csv?yField=geom_y&xField=geom_x&type=csv'
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
    return wanted


def test_011_read_wkt():
    wanted={}
    wanted['uri']=u'file://testwkt.csv?delimiter=|&type=csv&wktField=geom_wkt'
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
    return wanted


def test_012_read_wkt_point():
    wanted={}
    wanted['uri']=u'file://testwkt.csv?geomType=point&delimiter=|&type=csv&wktField=geom_wkt'
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
    return wanted


def test_013_read_wkt_line():
    wanted={}
    wanted['uri']=u'file://testwkt.csv?geomType=line&delimiter=|&type=csv&wktField=geom_wkt'
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
    return wanted


def test_014_read_wkt_polygon():
    wanted={}
    wanted['uri']=u'file://testwkt.csv?geomType=polygon&delimiter=|&type=csv&wktField=geom_wkt'
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
    return wanted


def test_015_read_dms_xy():
    wanted={}
    wanted['uri']=u'file://testdms.csv?yField=lat&xField=lon&type=csv&xyDms=yes'
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
    return wanted


def test_016_decimal_point():
    wanted={}
    wanted['uri']=u'file://testdp.csv?yField=geom_y&xField=geom_x&type=csv&delimiter=;&decimalPoint=,'
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
    return wanted


def test_017_regular_expression_1():
    wanted={}
    wanted['uri']=u'file://testre.txt?geomType=none&trimFields=Y&delimiter=RE(?:GEXP)?&type=regexp'
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
    return wanted


def test_018_regular_expression_2():
    wanted={}
    wanted['uri']=u'file://testre.txt?geomType=none&trimFields=Y&delimiter=(RE)(GEXP)?&type=regexp'
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
    return wanted


def test_019_regular_expression_3():
    wanted={}
    wanted['uri']=u'file://testre2.txt?geomType=none&trimFields=Y&delimiter=^(.{5})(.{30})(.{5,})&type=regexp'
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
    return wanted


def test_020_regular_expression_4():
    wanted={}
    wanted['uri']=u'file://testre3.txt?geomType=none&delimiter=x?&type=regexp'
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
    return wanted


def test_021_regular_expression_5():
    wanted={}
    wanted['uri']=u'file://testre3.txt?geomType=none&delimiter=\\b&type=regexp'
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
    return wanted


def test_022_utf8_encoded_file():
    wanted={}
    wanted['uri']=u'file://testutf8.csv?geomType=none&delimiter=|&type=csv&encoding=utf-8'
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
    return wanted


def test_023_latin1_encoded_file():
    wanted={}
    wanted['uri']=u'file://testlatin1.csv?geomType=none&delimiter=|&type=csv&encoding=latin1'
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
    return wanted


def test_024_filter_rect_xy():
    wanted={}
    wanted['uri']=u'file://testextpt.txt?yField=y&delimiter=|&type=csv&xField=x'
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
        'Request 2 did not return any data',
        ]
    return wanted


def test_025_filter_rect_wkt():
    wanted={}
    wanted['uri']=u'file://testextw.txt?delimiter=|&type=csv&wktField=wkt'
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
        'Request 2 did not return any data',
        ]
    return wanted


def test_026_filter_fid():
    wanted={}
    wanted['uri']=u'file://test.csv?geomType=none&type=csv'
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
        3003L: {
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
        'Request 2 did not return any data',
        ]
    return wanted


def test_027_filter_attributes():
    wanted={}
    wanted['uri']=u'file://test.csv?geomType=none&type=csv'
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
    return wanted


def test_028_substring_test():
    wanted={}
    wanted['uri']=u'file://test.csv?geomType=none&type=csv&subset=id%20%25%202%20%3D%201'
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
        4L: {
            'id': u'3',
            'description': u'Escaped quotes',
            'data': u'Quoted "citation" data',
            'info': u'Unquoted',
            'field_5': u'',
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
    wanted['log']=[
        ]
    return wanted


def test_029_file_watcher():
    wanted={}
    wanted['uri']=u'file://file?geomType=none&type=csv'
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
        13002L: {
            'id': u'5',
            'description': u'toad',
            'name': u'toad',
            '#fid': 2L,
            '#geometry': 'None',
            },
        14003L: {
            'id': u'6',
            'description': u'mole',
            'name': u'mole',
            '#fid': 3L,
            '#geometry': 'None',
            },
        14004L: {
            'id': u'7',
            'description': u'badger',
            'name': u'badger',
            '#fid': 4L,
            '#geometry': 'None',
            },
        }
    wanted['log']=[
        'Request 2 did not return any data',
        'Request 7 did not return any data',
        'Request 11 did not return any data',
        u'Errors in file file',
        u'The file has been updated by another application - reloading',
        u'Errors in file file',
        u'The file has been updated by another application - reloading',
        ]
    return wanted


def test_030_filter_rect_xy_spatial_index():
    wanted={}
    wanted['uri']=u'file://testextpt.txt?spatialIndex=Y&yField=y&delimiter=|&type=csv&xField=x'
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
        3002L: {
            'id': u'1',
            'description': u'Inside',
            'x': u'15',
            'y': u'35',
            '#fid': 2L,
            '#geometry': 'POINT(15.0 35.0)',
            },
        3003L: {
            'id': u'2',
            'description': u'Outside 1',
            'x': u'5',
            'y': u'35',
            '#fid': 3L,
            '#geometry': 'POINT(5.0 35.0)',
            },
        3004L: {
            'id': u'3',
            'description': u'Outside 2',
            'x': u'5',
            'y': u'55',
            '#fid': 4L,
            '#geometry': 'POINT(5.0 55.0)',
            },
        3005L: {
            'id': u'4',
            'description': u'Outside 3',
            'x': u'15',
            'y': u'55',
            '#fid': 5L,
            '#geometry': 'POINT(15.0 55.0)',
            },
        3006L: {
            'id': u'5',
            'description': u'Outside 4',
            'x': u'35',
            'y': u'55',
            '#fid': 6L,
            '#geometry': 'POINT(35.0 55.0)',
            },
        3007L: {
            'id': u'6',
            'description': u'Outside 5',
            'x': u'35',
            'y': u'45',
            '#fid': 7L,
            '#geometry': 'POINT(35.0 45.0)',
            },
        3008L: {
            'id': u'7',
            'description': u'Outside 7',
            'x': u'35',
            'y': u'25',
            '#fid': 8L,
            '#geometry': 'POINT(35.0 25.0)',
            },
        3009L: {
            'id': u'8',
            'description': u'Outside 8',
            'x': u'15',
            'y': u'25',
            '#fid': 9L,
            '#geometry': 'POINT(15.0 25.0)',
            },
        3010L: {
            'id': u'9',
            'description': u'Inside 2',
            'x': u'25',
            'y': u'45',
            '#fid': 10L,
            '#geometry': 'POINT(25.0 45.0)',
            },
        4002L: {
            'id': u'1',
            'description': u'Inside',
            'x': u'15',
            'y': u'35',
            '#fid': 2L,
            '#geometry': 'POINT(15.0 35.0)',
            },
        4003L: {
            'id': u'2',
            'description': u'Outside 1',
            'x': u'5',
            'y': u'35',
            '#fid': 3L,
            '#geometry': 'POINT(5.0 35.0)',
            },
        4004L: {
            'id': u'3',
            'description': u'Outside 2',
            'x': u'5',
            'y': u'55',
            '#fid': 4L,
            '#geometry': 'POINT(5.0 55.0)',
            },
        4005L: {
            'id': u'4',
            'description': u'Outside 3',
            'x': u'15',
            'y': u'55',
            '#fid': 5L,
            '#geometry': 'POINT(15.0 55.0)',
            },
        4006L: {
            'id': u'5',
            'description': u'Outside 4',
            'x': u'35',
            'y': u'55',
            '#fid': 6L,
            '#geometry': 'POINT(35.0 55.0)',
            },
        4007L: {
            'id': u'6',
            'description': u'Outside 5',
            'x': u'35',
            'y': u'45',
            '#fid': 7L,
            '#geometry': 'POINT(35.0 45.0)',
            },
        4008L: {
            'id': u'7',
            'description': u'Outside 7',
            'x': u'35',
            'y': u'25',
            '#fid': 8L,
            '#geometry': 'POINT(35.0 25.0)',
            },
        4009L: {
            'id': u'8',
            'description': u'Outside 8',
            'x': u'15',
            'y': u'25',
            '#fid': 9L,
            '#geometry': 'POINT(15.0 25.0)',
            },
        4010L: {
            'id': u'9',
            'description': u'Inside 2',
            'x': u'25',
            'y': u'45',
            '#fid': 10L,
            '#geometry': 'POINT(25.0 45.0)',
            },
        }
    wanted['log']=[
        'Request 2 did not return any data',
        ]
    return wanted


def test_031_filter_rect_wkt_spatial_index():
    wanted={}
    wanted['uri']=u'file://testextw.txt?spatialIndex=Y&delimiter=|&type=csv&wktField=wkt'
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
        3002L: {
            'id': u'1',
            'description': u'Inside',
            '#fid': 2L,
            '#geometry': 'LINESTRING(12.0 32.0, 28.0 48.0)',
            },
        3003L: {
            'id': u'2',
            'description': u'Outside',
            '#fid': 3L,
            '#geometry': 'LINESTRING(0.0 0.0, 0.0 10.0)',
            },
        3004L: {
            'id': u'3',
            'description': u'Crossing',
            '#fid': 4L,
            '#geometry': 'LINESTRING(5.0 30.0, 30.0 55.0)',
            },
        3005L: {
            'id': u'4',
            'description': u'Bounding box overlap',
            '#fid': 5L,
            '#geometry': 'LINESTRING(5.0 30.0, 5.0 55.0, 30.0 55.0)',
            },
        3006L: {
            'id': u'5',
            'description': u'Crossing 2',
            '#fid': 6L,
            '#geometry': 'LINESTRING(25.0 35.0, 35.0 35.0)',
            },
        3007L: {
            'id': u'6',
            'description': u'Bounding box overlap 2',
            '#fid': 7L,
            '#geometry': 'LINESTRING(28.0 29.0, 31.0 29.0, 31.0 33.0)',
            },
        4002L: {
            'id': u'1',
            'description': u'Inside',
            '#fid': 2L,
            '#geometry': 'LINESTRING(12.0 32.0, 28.0 48.0)',
            },
        4003L: {
            'id': u'2',
            'description': u'Outside',
            '#fid': 3L,
            '#geometry': 'LINESTRING(0.0 0.0, 0.0 10.0)',
            },
        4004L: {
            'id': u'3',
            'description': u'Crossing',
            '#fid': 4L,
            '#geometry': 'LINESTRING(5.0 30.0, 30.0 55.0)',
            },
        4005L: {
            'id': u'4',
            'description': u'Bounding box overlap',
            '#fid': 5L,
            '#geometry': 'LINESTRING(5.0 30.0, 5.0 55.0, 30.0 55.0)',
            },
        4006L: {
            'id': u'5',
            'description': u'Crossing 2',
            '#fid': 6L,
            '#geometry': 'LINESTRING(25.0 35.0, 35.0 35.0)',
            },
        4007L: {
            'id': u'6',
            'description': u'Bounding box overlap 2',
            '#fid': 7L,
            '#geometry': 'LINESTRING(28.0 29.0, 31.0 29.0, 31.0 33.0)',
            },
        }
    wanted['log']=[
        'Request 2 did not return any data',
        ]
    return wanted


def test_032_filter_rect_wkt_create_spatial_index():
    wanted={}
    wanted['uri']=u'file://testextw.txt?delimiter=|&type=csv&wktField=wkt'
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
        1003L: {
            'id': u'2',
            'description': u'Outside',
            '#fid': 3L,
            '#geometry': 'LINESTRING(0.0 0.0, 0.0 10.0)',
            },
        1004L: {
            'id': u'3',
            'description': u'Crossing',
            '#fid': 4L,
            '#geometry': 'LINESTRING(5.0 30.0, 30.0 55.0)',
            },
        1005L: {
            'id': u'4',
            'description': u'Bounding box overlap',
            '#fid': 5L,
            '#geometry': 'LINESTRING(5.0 30.0, 5.0 55.0, 30.0 55.0)',
            },
        1006L: {
            'id': u'5',
            'description': u'Crossing 2',
            '#fid': 6L,
            '#geometry': 'LINESTRING(25.0 35.0, 35.0 35.0)',
            },
        1007L: {
            'id': u'6',
            'description': u'Bounding box overlap 2',
            '#fid': 7L,
            '#geometry': 'LINESTRING(28.0 29.0, 31.0 29.0, 31.0 33.0)',
            },
        3002L: {
            'id': u'1',
            'description': u'Inside',
            '#fid': 2L,
            '#geometry': 'LINESTRING(12.0 32.0, 28.0 48.0)',
            },
        3004L: {
            'id': u'3',
            'description': u'Crossing',
            '#fid': 4L,
            '#geometry': 'LINESTRING(5.0 30.0, 30.0 55.0)',
            },
        3005L: {
            'id': u'4',
            'description': u'Bounding box overlap',
            '#fid': 5L,
            '#geometry': 'LINESTRING(5.0 30.0, 5.0 55.0, 30.0 55.0)',
            },
        3006L: {
            'id': u'5',
            'description': u'Crossing 2',
            '#fid': 6L,
            '#geometry': 'LINESTRING(25.0 35.0, 35.0 35.0)',
            },
        3007L: {
            'id': u'6',
            'description': u'Bounding box overlap 2',
            '#fid': 7L,
            '#geometry': 'LINESTRING(28.0 29.0, 31.0 29.0, 31.0 33.0)',
            },
        4002L: {
            'id': u'1',
            'description': u'Inside',
            '#fid': 2L,
            '#geometry': 'LINESTRING(12.0 32.0, 28.0 48.0)',
            },
        4004L: {
            'id': u'3',
            'description': u'Crossing',
            '#fid': 4L,
            '#geometry': 'LINESTRING(5.0 30.0, 30.0 55.0)',
            },
        4006L: {
            'id': u'5',
            'description': u'Crossing 2',
            '#fid': 6L,
            '#geometry': 'LINESTRING(25.0 35.0, 35.0 35.0)',
            },
        6002L: {
            'id': u'1',
            'description': u'Inside',
            '#fid': 2L,
            '#geometry': 'LINESTRING(12.0 32.0, 28.0 48.0)',
            },
        6003L: {
            'id': u'2',
            'description': u'Outside',
            '#fid': 3L,
            '#geometry': 'LINESTRING(0.0 0.0, 0.0 10.0)',
            },
        6004L: {
            'id': u'3',
            'description': u'Crossing',
            '#fid': 4L,
            '#geometry': 'LINESTRING(5.0 30.0, 30.0 55.0)',
            },
        6005L: {
            'id': u'4',
            'description': u'Bounding box overlap',
            '#fid': 5L,
            '#geometry': 'LINESTRING(5.0 30.0, 5.0 55.0, 30.0 55.0)',
            },
        6006L: {
            'id': u'5',
            'description': u'Crossing 2',
            '#fid': 6L,
            '#geometry': 'LINESTRING(25.0 35.0, 35.0 35.0)',
            },
        6007L: {
            'id': u'6',
            'description': u'Bounding box overlap 2',
            '#fid': 7L,
            '#geometry': 'LINESTRING(28.0 29.0, 31.0 29.0, 31.0 33.0)',
            },
        7002L: {
            'id': u'1',
            'description': u'Inside',
            '#fid': 2L,
            '#geometry': 'LINESTRING(12.0 32.0, 28.0 48.0)',
            },
        7003L: {
            'id': u'2',
            'description': u'Outside',
            '#fid': 3L,
            '#geometry': 'LINESTRING(0.0 0.0, 0.0 10.0)',
            },
        7004L: {
            'id': u'3',
            'description': u'Crossing',
            '#fid': 4L,
            '#geometry': 'LINESTRING(5.0 30.0, 30.0 55.0)',
            },
        7005L: {
            'id': u'4',
            'description': u'Bounding box overlap',
            '#fid': 5L,
            '#geometry': 'LINESTRING(5.0 30.0, 5.0 55.0, 30.0 55.0)',
            },
        7006L: {
            'id': u'5',
            'description': u'Crossing 2',
            '#fid': 6L,
            '#geometry': 'LINESTRING(25.0 35.0, 35.0 35.0)',
            },
        7007L: {
            'id': u'6',
            'description': u'Bounding box overlap 2',
            '#fid': 7L,
            '#geometry': 'LINESTRING(28.0 29.0, 31.0 29.0, 31.0 33.0)',
            },
        }
    wanted['log']=[
        'Request 5 did not return any data',
        ]
    return wanted

def test_033_reset_subset_string():
    wanted={}
    wanted['uri']=u'file://test.csv?geomType=none&type=csv'
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
        2002L: {
            'id': u'1',
            'description': u'Basic unquoted record',
            'data': u'Some data',
            'info': u'Some info',
            'field_5': u'',
            '#fid': 2L,
            '#geometry': 'None',
            },
        2004L: {
            'id': u'3',
            'description': u'Escaped quotes',
            'data': u'Quoted "citation" data',
            'info': u'Unquoted',
            'field_5': u'',
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
            'data': u'',
            'info': u'',
            'field_5': u'',
            '#fid': 10L,
            '#geometry': 'None',
            },
        6004L: {
            'id': u'3',
            'description': u'Escaped quotes',
            'data': u'Quoted "citation" data',
            'info': u'Unquoted',
            'field_5': u'',
            '#fid': 4L,
            '#geometry': 'None',
            },
        8002L: {
            'id': u'1',
            'description': u'Basic unquoted record',
            'data': u'Some data',
            'info': u'Some info',
            'field_5': u'',
            '#fid': 2L,
            '#geometry': 'None',
            },
        8004L: {
            'id': u'3',
            'description': u'Escaped quotes',
            'data': u'Quoted "citation" data',
            'info': u'Unquoted',
            'field_5': u'',
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
            'field_5': u'',
            '#fid': 3L,
            '#geometry': 'None',
            },
        10005L: {
            'id': u'4',
            'description': u'Quoted newlines',
            'data': u'Line 1\nLine 2\n\nLine 4',
            'info': u'No data',
            'field_5': u'',
            '#fid': 5L,
            '#geometry': 'None',
            },
        10010L: {
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
    return wanted

