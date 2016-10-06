# -*- coding: ISO-8859-15 -*-
# =============================================================================
# Copyright (c) 2008 Tom Kralidis
#
# Authors : Tom Kralidis <tomkralidis@gmail.com>
#
# Contact email: tomkralidis@gmail.com
# =============================================================================

from __future__ import (absolute_import, division, print_function)

import sys
from dateutil import parser
from datetime import datetime
import pytz
from owslib.etree import etree, ParseError
from owslib.namespaces import Namespaces
try:                    # Python 3
    from urllib.parse import urlsplit, urlencode
except ImportError:     # Python 2
    from urlparse import urlsplit
    from urllib import urlencode

try:
    from StringIO import StringIO  # Python 2
    BytesIO = StringIO
except ImportError:
    from io import StringIO, BytesIO  # Python 3

import cgi
import re
from copy import deepcopy
import warnings
import six
import requests

"""
Utility functions and classes
"""

class ServiceException(Exception):
    #TODO: this should go in ows common module when refactored.  
    pass

# http://stackoverflow.com/questions/6256183/combine-two-dictionaries-of-dictionaries-python
dict_union = lambda d1,d2: dict((x,(dict_union(d1.get(x,{}),d2[x]) if
  isinstance(d2.get(x),dict) else d2.get(x,d1.get(x)))) for x in
  set(list(d1.keys())+list(d2.keys())))


# Infinite DateTimes for Python.  Used in SWE 2.0 and other OGC specs as "INF" and "-INF"
class InfiniteDateTime(object):
    def __lt__(self, other):
        return False
    def __gt__(self, other):
        return True
    def timetuple(self):
        return tuple()
class NegativeInfiniteDateTime(object):
    def __lt__(self, other):
        return True
    def __gt__(self, other):
        return False
    def timetuple(self):
        return tuple()


first_cap_re = re.compile('(.)([A-Z][a-z]+)')
all_cap_re = re.compile('([a-z0-9])([A-Z])')
def format_string(prop_string):
    """
        Formats a property string to remove spaces and go from CamelCase to pep8
        from: http://stackoverflow.com/questions/1175208/elegant-python-function-to-convert-camelcase-to-camel-case
    """
    if prop_string is None:
        return ''
    st_r = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', prop_string)
    st_r = st_r.replace(' ','')
    return re.sub('([a-z0-9])([A-Z])', r'\1_\2', st_r).lower()

def xml_to_dict(root, prefix=None, depth=1, diction=None):
    """
        Recursively iterates through an xml element to convert each element in the tree to a (key,val). Where key is the element
        tag and val is the inner-text of the element. Note that this recursively go through the tree until the depth specified.

        Parameters
        ===========
        :root - root xml element, starting point of iteration
        :prefix - a string to prepend to the resulting key (optional)
        :depth - the number of depths to process in the tree (optional)
        :diction - the dictionary to insert the (tag,text) pairs into (optional)

        Return
        =======
        Dictionary of (key,value); where key is the element tag stripped of namespace and cleaned up to be pep8 and
        value is the inner-text of the element. Note that duplicate elements will be replaced by the last element of the 
        same tag in the tree.
    """
    ret = diction if diction is not None else dict()
    for child in root:
        val = testXMLValue(child)
        # skip values that are empty or None
        if val is None or val == '':
            if depth > 1:
                ret = xml_to_dict(child,prefix=prefix,depth=(depth-1),diction=ret)
            continue

        key = format_string(child.tag.split('}')[-1])

        if prefix is not None:
            key = prefix + key

        ret[key] = val
        if depth > 1:
            ret = xml_to_dict(child,prefix=prefix,depth=(depth-1),diction=ret)

    return ret

class ResponseWrapper(object):
    """
    Return object type from openURL.

    Provides a thin shim around requests response object to maintain code compatibility.
    """
    def __init__(self, response):
        self._response = response

    def info(self):
        return self._response.headers

    def read(self):
        return self._response.content

    def geturl(self):
        return self._response.url

    # @TODO: __getattribute__ for poking at response

def openURL(url_base, data=None, method='Get', cookies=None, username=None, password=None, timeout=30, headers=None):
    """
    Function to open URLs.

    Uses requests library but with additional checks for OGC service exceptions and url formatting.
    Also handles cookies and simple user password authentication.
    """
    headers = headers if headers is not None else {}
    rkwargs = {}

    rkwargs['timeout'] = timeout

    auth = None
    if username and password:
        auth = (username, password)

    rkwargs['auth'] = auth

    # FIXUP for WFS in particular, remove xml style namespace
    # @TODO does this belong here?
    method = method.split("}")[-1]

    if method.lower() == 'post':
        try:
            xml = etree.fromstring(data)
            headers['Content-Type'] = 'text/xml'
        except (ParseError, UnicodeEncodeError):
            pass

        rkwargs['data'] = data

    elif method.lower() == 'get':
        rkwargs['params'] = data
        
    else:
        raise ValueError("Unknown method ('%s'), expected 'get' or 'post'" % method)

    if cookies is not None:
        rkwargs['cookies'] = cookies

    req = requests.request(method.upper(),
                           url_base,
                           headers=headers,
                           **rkwargs)

    if req.status_code in [400, 401]:
        raise ServiceException(req.text)

    if req.status_code in [404]:    # add more if needed
        req.raise_for_status()

    # check for service exceptions without the http header set
    if 'Content-Type' in req.headers and req.headers['Content-Type'] in ['text/xml', 'application/xml']:
        #just in case 400 headers were not set, going to have to read the xml to see if it's an exception report.
        se_tree = etree.fromstring(req.content)
        serviceException=se_tree.find('{http://www.opengis.net/ows}Exception')
        if serviceException is None:
            serviceException=se_tree.find('ServiceException')
        if serviceException is not None:
            raise ServiceException(str(serviceException.text).strip())

    return ResponseWrapper(req)

#default namespace for nspath is OWS common
OWS_NAMESPACE = 'http://www.opengis.net/ows/1.1'
def nspath(path, ns=OWS_NAMESPACE):

    """

    Prefix the given path with the given namespace identifier.
    
    Parameters
    ----------

    - path: ElementTree API Compatible path expression
    - ns: the XML namespace URI.

    """

    if ns is None or path is None:
        return -1

    components = []
    for component in path.split('/'):
        if component != '*':
            component = '{%s}%s' % (ns, component)
        components.append(component)
    return '/'.join(components)

def nspath_eval(xpath, namespaces):
    ''' Return an etree friendly xpath '''
    out = []
    for chunks in xpath.split('/'):
        namespace, element = chunks.split(':')
        out.append('{%s}%s' % (namespaces[namespace], element))
    return '/'.join(out)

def cleanup_namespaces(element):
    """ Remove unused namespaces from an element """
    if etree.__name__ == 'lxml.etree':
        etree.cleanup_namespaces(element)
        return element
    else:
        return etree.fromstring(etree.tostring(element))


def add_namespaces(root, ns_keys):
    if isinstance(ns_keys, six.string_types):
        ns_keys = [ns_keys]

    namespaces = Namespaces()

    ns_keys = [(x, namespaces.get_namespace(x)) for x in ns_keys]

    if etree.__name__ != 'lxml.etree':
        # We can just add more namespaces when not using lxml.
        # We can't re-add an existing namespaces.  Get a list of current
        # namespaces in use
        existing_namespaces = set()
        for elem in root.getiterator():
            if elem.tag[0] == "{":
                uri, tag = elem.tag[1:].split("}")
                existing_namespaces.add(namespaces.get_namespace_from_url(uri))
        for key, link in ns_keys:
            if link is not None and key not in existing_namespaces:
                root.set("xmlns:%s" % key, link)
        return root
    else:
        # lxml does not support setting xmlns attributes
        # Update the elements nsmap with new namespaces
        new_map = root.nsmap
        for key, link in ns_keys:
            if link is not None:
                new_map[key] = link
        # Recreate the root element with updated nsmap
        new_root = etree.Element(root.tag, nsmap=new_map)
        # Carry over attributes
        for a, v in list(root.items()):
            new_root.set(a, v)
        # Carry over children
        for child in root:
            new_root.append(deepcopy(child))
        return new_root


def getXMLInteger(elem, tag):
    """
    Return the text within the named tag as an integer.

    Raises an exception if the tag cannot be found or if its textual
    value cannot be converted to an integer.

    Parameters
    ----------

    - elem: the element to search within
    - tag: the name of the tag to look for

    """
    e = elem.find(tag)
    if e is None:
        raise ValueError('Missing %s in %s' % (tag, elem))
    return int(e.text.strip())


def testXMLValue(val, attrib=False):
    """

    Test that the XML value exists, return val.text, else return None

    Parameters
    ----------

    - val: the value to be tested

    """

    if val is not None:
        if attrib:
            return val.strip()
        elif val.text:  
            return val.text.strip()
        else:
            return None	
    else:
        return None

def testXMLAttribute(element, attribute):
    """

    Test that the XML element and attribute exist, return attribute's value, else return None

    Parameters
    ----------

    - element: the element containing the attribute
    - attribute: the attribute name

    """
    if element is not None:
        return element.get(attribute)

    return None

def http_post(url=None, request=None, lang='en-US', timeout=10, username=None, password=None):
    """

    Invoke an HTTP POST request 

    Parameters
    ----------

    - url: the URL of the server
    - request: the request message
    - lang: the language
    - timeout: timeout in seconds

    """

    if url is None:
        raise ValueError("URL required")

    u = urlsplit(url)

    headers = {
        'User-Agent'      : 'OWSLib (https://geopython.github.io/OWSLib)',
        'Content-type'    : 'text/xml',
        'Accept'          : 'text/xml',
        'Accept-Language' : lang,
        'Accept-Encoding' : 'gzip,deflate',
        'Host'            : u.netloc,
    }

    rkwargs = {}

    if username is not None and password is not None:
        rkwargs['auth'] = (username, password)

    up = requests.post(url, request, headers=headers, **rkwargs)
    return up.content

def element_to_string(element, encoding=None, xml_declaration=False):
    """
    Returns a string from a XML object

    Parameters
    ----------
    - element: etree Element
    - encoding (optional): encoding in string form. 'utf-8', 'ISO-8859-1', etc.
    - xml_declaration (optional): whether to include xml declaration

    """

    output = None

    if encoding is None:
        encoding = "ISO-8859-1"

    if etree.__name__ == 'lxml.etree':
        if xml_declaration:
            if encoding in ['unicode', 'utf-8']:
                output = '<?xml version="1.0" encoding="utf-8" standalone="no"?>\n%s' % \
                       etree.tostring(element, encoding='unicode')
            else:
                output = etree.tostring(element, encoding=encoding, xml_declaration=True)
        else:
                output = etree.tostring(element)
    else:
        if xml_declaration:
            output = '<?xml version="1.0" encoding="%s" standalone="no"?>\n%s' % (encoding,
                   etree.tostring(element, encoding=encoding))
        else:
            output = etree.tostring(element)

    return output


def xml2string(xml):
    """

    Return a string of XML object

    Parameters
    ----------

    - xml: xml string

    """
    warnings.warn("DEPRECIATION WARNING!  You should now use the 'element_to_string' method \
                   The 'xml2string' method will be removed in a future version of OWSLib.")
    return '<?xml version="1.0" encoding="ISO-8859-1" standalone="no"?>\n' + xml

def xmlvalid(xml, xsd):
    """

    Test whether an XML document is valid

    Parameters
    ----------

    - xml: XML content
    - xsd: pointer to XML Schema (local file path or URL)

    """

    xsd1 = etree.parse(xsd)
    xsd2 = etree.XMLSchema(xsd1)

    doc = etree.parse(StringIO(xml))
    return xsd2.validate(doc)

def xmltag_split(tag):
    ''' Return XML element bare tag name (without prefix) '''
    try:
        return tag.split('}')[1]
    except:
        return tag

def getNamespace(element):
    ''' Utility method to extract the namespace from an XML element tag encoded as {namespace}localname. '''
    if element.tag[0]=='{':
        return element.tag[1:].split("}")[0]
    else:
        return ""

def build_get_url(base_url, params):
    ''' Utility function to build a full HTTP GET URL from the service base URL and a dictionary of HTTP parameters. '''
    
    qs = []
    if base_url.find('?') != -1:
        qs = cgi.parse_qsl(base_url.split('?')[1])

    pars = [x[0] for x in qs]

    for key,value in six.iteritems(params):
        if key not in pars:
            qs.append( (key,value) )

    urlqs = urlencode(tuple(qs))
    return base_url.split('?')[0] + '?' + urlqs

def dump(obj, prefix=''):
    '''Utility function to print to standard output a generic object with all its attributes.'''

    print("%s %s.%s : %s" % (prefix, obj.__module__, obj.__class__.__name__, obj.__dict__))

def getTypedValue(data_type, value):
    '''Utility function to cast a string value to the appropriate XSD type. '''

    if data_type == 'boolean':
        return bool(value)
    elif data_type == 'integer':
        return int(value)
    elif data_type == 'float':
        return float(value)
    elif data_type == 'string':
        return str(value)
    else:
        return value # no type casting


def extract_time(element):
    ''' return a datetime object based on a gml text string

ex:
<gml:beginPosition>2006-07-27T21:10:00Z</gml:beginPosition>
<gml:endPosition indeterminatePosition="now"/>

If there happens to be a strange element with both attributes and text,
use the text.
ex: <gml:beginPosition indeterminatePosition="now">2006-07-27T21:10:00Z</gml:beginPosition>
Would be 2006-07-27T21:10:00Z, not 'now'

'''
    if element is None:
        return None

    try:
        dt = parser.parse(element.text)
    except Exception:
        att = testXMLValue(element.attrib.get('indeterminatePosition'), True)
        if att and att == 'now':
            dt = datetime.utcnow()
            dt.replace(tzinfo=pytz.utc)
        else:
            dt = None
    return dt


def extract_xml_list(elements):
    """
Some people don't have seperate tags for their keywords and seperate them with
a newline. This will extract out all of the keywords correctly.
"""
    if elements:
        keywords = [re.split(r'[\n\r]+',f.text) for f in elements if f.text]
        flattened = [item.strip() for sublist in keywords for item in sublist]
        remove_blank = [_f for _f in flattened if _f]
        return remove_blank
    else:
        return []


def bind_url(url):
    """binds an HTTP GET query string endpiont"""
    if url.find('?') == -1: # like http://host/wms
        binder = '?'

    # if like http://host/wms?foo=bar& or http://host/wms?foo=bar
    if url.find('=') != -1:
        if url.find('&', -1) != -1: # like http://host/wms?foo=bar&
            binder = ''
        else: # like http://host/wms?foo=bar
            binder = '&'

    # if like http://host/wms?foo
    if url.find('?') != -1:
        if url.find('?', -1) != -1: # like http://host/wms?
            binder = ''
        elif url.find('&', -1) == -1: # like http://host/wms?foo=bar
            binder = '&'
    return '%s%s' % (url, binder)

import logging
# Null logging handler
try:
    # Python 2.7
    NullHandler = logging.NullHandler
except AttributeError:
    # Python < 2.7
    class NullHandler(logging.Handler):
        def emit(self, record):
            pass
log = logging.getLogger('owslib')
log.addHandler(NullHandler())

# OrderedDict
try:  # 2.7
    from collections import OrderedDict
except:  # 2.6
    from ordereddict import OrderedDict


def which_etree():
    """decipher which etree library is being used by OWSLib"""

    which_etree = None

    if 'lxml' in etree.__file__:
        which_etree = 'lxml.etree'
    elif 'xml/etree' in etree.__file__:
        which_etree = 'xml.etree'
    elif 'elementree' in etree.__file__:
        which_etree = 'elementtree.ElementTree'

    return which_etree

def findall(root, xpath, attribute_name=None, attribute_value=None):
    """Find elements recursively from given root element based on
    xpath and possibly given attribute

    :param root: Element root element where to start search
    :param xpath: xpath defintion, like {http://foo/bar/namespace}ElementName
    :param attribute_name: name of possible attribute of given element
    :param attribute_value: value of the attribute
    :return: list of elements or None
    """

    found_elements = []


    # python 2.6 < does not support complicated XPATH expressions used lower
    if (2, 6) == sys.version_info[0:2] and which_etree() != 'lxml.etree':

        elements = root.getiterator(xpath)

        if attribute_name is not None and attribute_value is not None:
            for element in elements:
                if element.attrib.get(attribute_name) == attribute_value:
                    found_elements.append(element)
        else:
            found_elements = elements
    # python at least 2.7 and/or lxml can do things much simplier
    else:
        if attribute_name is not None and attribute_value is not None:
            xpath = '%s[@%s="%s"]' % (xpath, attribute_name, attribute_value)
        found_elements = root.findall('.//' + xpath)

    if found_elements == []:
        found_elements = None
    return found_elements
