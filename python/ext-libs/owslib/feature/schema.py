# =============================================================================
# OWSLib. Copyright (C) 2015 Jachym Cepicky
#
# Contact email: jachym.cepicky@gmail.com
#
# =============================================================================
"""
Set of functions, which are suitable for DescribeFeatureType parsing and
generating layer schema description compatible with `fiona`
"""

import cgi, sys
from owslib.util import openURL
try:
    from urllib import urlencode
except ImportError:
    from urllib.parse import urlencode
from owslib.etree import etree
from owslib.namespaces import Namespaces
from owslib.util import which_etree, findall

MYNS = Namespaces()
XS_NAMESPACE = MYNS.get_namespace('xs')
GML_NAMESPACES = (MYNS.get_namespace('gml'),
                  MYNS.get_namespace('gml311'),
                  MYNS.get_namespace('gml32'))


def get_schema(url, typename, version='1.0.0', timeout=30):
    """Parses DescribeFeatureType response and creates schema compatible
    with :class:`fiona`

    :param str url: url of the service
    :param str version: version of the service
    :param str typename: name of the layer
    :param int timeout: request timeout
    """

    url = _get_describefeaturetype_url(url, version, typename)
    res = openURL(url, timeout=timeout)
    root = etree.fromstring(res.read())
    type_element = findall(root, '{%s}element' % XS_NAMESPACE,
                           attribute_name='name', attribute_value=typename)[0]
    complex_type = type_element.attrib['type'].split(":")[1]
    elements = _get_elements(complex_type, root)
    nsmap = None
    if hasattr(root, 'nsmap'):
        nsmap = root.nsmap
    return _construct_schema(elements, nsmap)


def _get_elements(complex_type, root):
    """Get attribute elements
    """

    found_elements = []
    element = findall(root, '{%s}complexType' % XS_NAMESPACE,
                       attribute_name='name', attribute_value=complex_type)[0]
    found_elements = findall(element, '{%s}element' % XS_NAMESPACE)

    return found_elements

def _construct_schema(elements, nsmap):
    """Consruct fiona schema based on given elements

    :param list Element: list of elements
    :param dict nsmap: namespace map

    :return dict: schema
    """

    schema = {
        'properties': {},
        'geometry': None
    }

    schema_key = None
    gml_key = None

    # if nsmap is defined, use it
    if nsmap:
        for key in nsmap:
            if nsmap[key] == XS_NAMESPACE:
                schema_key = key
            if nsmap[key] in GML_NAMESPACES:
                gml_key = key
    # if no nsmap is defined, we have to guess
    else:
        gml_key = 'gml'
        schema_key = 'xsd'

    for element in elements:
        data_type = element.attrib['type'].replace(gml_key + ':', '')
        name = element.attrib['name']

        if data_type == 'PointPropertyType':
            schema['geometry'] = 'Point'
        elif data_type == 'PolygonPropertyType':
            schema['geometry'] = 'Polygon'
        elif data_type == 'LineStringPropertyType':
            schema['geometry'] = 'LineString'
        elif data_type == 'MultiPointPropertyType':
            schema['geometry'] = 'MultiPoint'
        elif data_type == 'MultiLineStringPropertyType':
            schema['geometry'] = 'MultiLineString'
        elif data_type == 'MultiPolygonPropertyType':
            schema['geometry'] = 'MultiPolygon'
        elif data_type == 'MultiGeometryPropertyType':
            schema['geometry'] = 'MultiGeometry'
        elif data_type == 'GeometryPropertyType':
            schema['geometry'] = 'GeometryCollection'
        elif data_type == 'SurfacePropertyType':
            schema['geometry'] = '3D Polygon'
        else:
            schema['properties'][name] = data_type.replace(schema_key+':', '')

    if schema['properties'] or schema['geometry']:
        return schema
    else:
        return None

def _get_describefeaturetype_url(url, version, typename):
    """Get url for describefeaturetype request

    :return str: url
    """

    query_string = []
    if url.find('?') != -1:
        query_string = cgi.parse_qsl(url.split('?')[1])

    params = [x[0] for x in query_string]

    if 'service' not in params:
        query_string.append(('service', 'WFS'))
    if 'request' not in params:
        query_string.append(('request', 'DescribeFeatureType'))
    if 'version' not in params:
        query_string.append(('version', version))

    query_string.append(('typeName', typename))

    urlqs = urlencode(tuple(query_string))
    return url.split('?')[0] + '?' + urlqs

