# -*- coding: UTF-8 -*-
# =============================================================================
# Copyright (C) 2012 Brad Hards <bradh@frogmouth.net>
#
# Based on wms.py, which has the following copyright statement:
# Copyright (c) 2004, 2006 Sean C. Gillies
# Copyright (c) 2005 Nuxeo SARL <http://nuxeo.com>
#
# Authors : Sean Gillies <sgillies@frii.com>
#           Julien Anguenot <ja@nuxeo.com>
#
# Contact email: sgillies@frii.com
# =============================================================================

"""

Abstract
--------
The wmts module of the OWSlib package provides client-side functionality for fetching tiles from an OGC Web Map Tile Service (WMTS)


Disclaimer
----------
PLEASE NOTE: the owslib wmts module should be considered in early-beta state: it has been tested against only one WMTS server (NASA EODSIS).
More extensive testing is needed and feedback (to bradh@frogmouth.net) would be appreciated.

"""

import warnings
import urlparse
import urllib2
from urllib import urlencode
from etree import etree
from .util import openURL, testXMLValue, getXMLInteger
from fgdc import Metadata
from iso import MD_Metadata
from ows import ServiceProvider, ServiceIdentification, OperationsMetadata


_WMTS_NS = '{http://www.opengis.net/wmts/1.0}'
_TILE_MATRIX_SET_LINK_TAG = _WMTS_NS + 'TileMatrixSetLink'
_TILE_MATRIX_SET_TAG = _WMTS_NS + 'TileMatrixSet'
_TILE_MATRIX_SET_LIMITS_TAG = _WMTS_NS + 'TileMatrixSetLimits'
_TILE_MATRIX_LIMITS_TAG = _WMTS_NS + 'TileMatrixLimits'
_TILE_MATRIX_TAG = _WMTS_NS + 'TileMatrix'
_MIN_TILE_ROW_TAG = _WMTS_NS + 'MinTileRow'
_MAX_TILE_ROW_TAG = _WMTS_NS + 'MaxTileRow'
_MIN_TILE_COL_TAG = _WMTS_NS + 'MinTileCol'
_MAX_TILE_COL_TAG = _WMTS_NS + 'MaxTileCol'


class ServiceException(Exception):
    """WMTS ServiceException

    Attributes:
        message -- short error message
        xml  -- full xml error message from server
    """

    def __init__(self, message, xml):
        self.message = message
        self.xml = xml

    def __str__(self):
        return repr(self.message)


class CapabilitiesError(Exception):
    pass


class WebMapTileService(object):
    """Abstraction for OGC Web Map Tile Service (WMTS).

    Implements IWebMapService.
    """

    def __getitem__(self,name):
        ''' check contents dictionary to allow dict like access to service layers'''
        if name in self.__getattribute__('contents').keys():
            return self.__getattribute__('contents')[name]
        else:
            raise KeyError, "No content named %s" % name


    def __init__(self, url, version='1.0.0', xml=None,
                username=None, password=None, parse_remote_metadata=False
                ):
        """Initialize."""
        self.url = url
        self.username = username
        self.password = password
        self.version = version
        self._capabilities = None

        # Authentication handled by Reader
        reader = WMTSCapabilitiesReader(
                self.version, url=self.url, un=self.username, pw=self.password
                )

        if xml:  # read from stored xml
            self._capabilities = reader.readString(xml)
        else:  # read from server
            self._capabilities = reader.read(self.url)

        # avoid building capabilities metadata if the response is a ServiceExceptionReport
        # TODO: check if this needs a namespace
        se = self._capabilities.find('ServiceException')
        if se is not None:
            err_message = str(se.text).strip()
            raise ServiceException(err_message, xml)

        # build metadata objects
        self._buildMetadata(parse_remote_metadata)

    def _getcapproperty(self):
        if not self._capabilities:
            reader = WMTSCapabilitiesReader(
                self.version, url=self.url, un=self.username, pw=self.password
                )
            self._capabilities = ServiceMetadata(reader.read(self.url))
        return self._capabilities

    def _buildMetadata(self, parse_remote_metadata=False):
        ''' set up capabilities metadata objects '''

        #serviceIdentification metadata
        serviceident=self._capabilities.find('{http://www.opengis.net/ows/1.1}ServiceIdentification')
        self.identification=ServiceIdentification(serviceident)

        #serviceProvider metadata
        serviceprov=self._capabilities.find('{http://www.opengis.net/ows/1.1}ServiceProvider')
        self.provider=ServiceProvider(serviceprov)

        #serviceOperations metadata
        self.operations=[]
        for elem in self._capabilities.find('{http://www.opengis.net/ows/1.1}OperationsMetadata')[:]:
            self.operations.append(OperationsMetadata(elem))

        #serviceContents metadata: our assumption is that services use a top-level
        #layer as a metadata organizer, nothing more.
        self.contents={}
        caps = self._capabilities.find('{http://www.opengis.net/wmts/1.0}Contents')

        def gather_layers(parent_elem, parent_metadata):
            for index, elem in enumerate(parent_elem.findall('{http://www.opengis.net/wmts/1.0}Layer')):
                cm = ContentMetadata(elem, parent=parent_metadata, index=index+1, parse_remote_metadata=parse_remote_metadata)
                if cm.id:
                    if cm.id in self.contents:
                        raise KeyError('Content metadata for layer "%s" already exists' % cm.id)
                    self.contents[cm.id] = cm
                gather_layers(elem, cm)
        gather_layers(caps, None)

        self.tilematrixsets = {}
        for elem in caps.findall('{http://www.opengis.net/wmts/1.0}TileMatrixSet'):
            tms = TileMatrixSet(elem)
            if tms.identifier:
                if tms.identifier in self.tilematrixsets:
                    raise KeyError('TileMatrixSet with identifier "%s" already exists' % tms.identifier)
                self.tilematrixsets[tms.identifier] = tms

        self.themes = {}
        for elem in self._capabilities.findall('{http://www.opengis.net/wmts/1.0}Themes/{http://www.opengis.net/wmts/1.0}Theme'):
            theme = Theme(elem)
            if theme.identifier:
                if theme.identifier in self.themes:
                    raise KeyError('Theme with identifier "%s" already exists' % theme.identifier)
                self.themes[theme.identifier] = theme

        serviceMetadataURL = self._capabilities.find('{http://www.opengis.net/wmts/1.0}ServiceMetadataURL')
        if serviceMetadataURL is not None:
            self.serviceMetadataURL = serviceMetadataURL.attrib['{http://www.w3.org/1999/xlink}href']
        else:
            self.serviceMetadataURL = None

    def items(self):
        '''supports dict-like items() access'''
        items=[]
        for item in self.contents:
            items.append((item,self.contents[item]))
        return items

    def buildTileRequest(self, layer=None, style=None, format=None, tilematrixset=None, tilematrix=None, row=None, column=None, **kwargs):
        """Return the URL-encoded parameters for a GetTile request.

        Parameters
        ----------
        layer : string
            Content layer name.
        style : string
            Optional style name. Defaults to the first style defined for
            the relevant layer in the GetCapabilities response.
        format : string
            Optional output image format,  such as 'image/jpeg'.
            Defaults to the first format defined for the relevant layer
            in the GetCapabilities response.
        tilematrixset : string
            Optional name of tile matrix set to use.
            Defaults to the first tile matrix set defined for the
            relevant layer in the GetCapabilities response.
        tilematrix : string
            Name of the tile matrix to use.
        row : integer
            Row index of tile to request.
        column : integer
            Column index of tile to request.
        **kwargs : extra arguments
            anything else e.g. vendor specific parameters

        Example
        -------
            >>> url = 'http://map1c.vis.earthdata.nasa.gov/wmts-geo/wmts.cgi'
            >>> wmts = WebMapTileService(url)
            >>> wmts.buildTileRequest(layer='VIIRS_CityLights_2012',
            ...                       tilematrixset='EPSG4326_500m',
            ...                       tilematrix='6',
            ...                       row=4, column=4)
            'SERVICE=WMTS&REQUEST=GetTile&VERSION=1.0.0&\
LAYER=VIIRS_CityLights_2012&STYLE=default&TILEMATRIXSET=EPSG4326_500m&\
TILEMATRIX=6&TILEROW=4&TILECOL=4&FORMAT=image%2Fjpeg'

        """
        request = {'version': self.version, 'request': 'GetTile'}

        if (layer is None):
            raise ValueError("layer is mandatory (cannot be None)")
        if style is None:
            style = self[layer].styles.keys()[0]
        if format is None:
            format = self[layer].formats[0]
        if tilematrixset is None:
            tilematrixset = sorted(self[layer].tilematrixsetlinks.keys())[0]
        if tilematrix is None:
            msg = 'tilematrix (zoom level) is mandatory (cannot be None)'
            raise ValueError(msg)
        if row is None:
                raise ValueError("row is mandatory (cannot be None)")
        if column is None:
                raise ValueError("column is mandatory (cannot be None)")

        request = list()
        request.append(('SERVICE', 'WMTS'))
        request.append(('REQUEST', 'GetTile'))
        request.append(('VERSION', '1.0.0'))
        request.append(('LAYER', layer))
        request.append(('STYLE', style))
        request.append(('TILEMATRIXSET', tilematrixset))
        request.append(('TILEMATRIX', tilematrix))
        request.append(('TILEROW', str(row)))
        request.append(('TILECOL', str(column)))
        request.append(('FORMAT', format))

        for key, value in kwargs.iteritems():
            request.append(key, value)

        data = urlencode(request, True)
        return data

    def gettile(self, base_url=None, layer=None, style=None, format=None, tilematrixset=None, tilematrix=None, row=None, column=None, **kwargs):
        """Return a tile from the WMTS.

        Returns the tile image as a file-like object.

        Parameters
        ----------
        base_url : string
            Optional URL for request submission. Defaults to the URL of
            the GetTile operation as declared in the GetCapabilities
            response.
        layer : string
            Content layer name.
        style : string
            Optional style name. Defaults to the first style defined for
            the relevant layer in the GetCapabilities response.
        format : string
            Optional output image format,  such as 'image/jpeg'.
            Defaults to the first format defined for the relevant layer
            in the GetCapabilities response.
        tilematrixset : string
            Optional name of tile matrix set to use.
            Defaults to the first tile matrix set defined for the
            relevant layer in the GetCapabilities response.
        tilematrix : string
            Name of the tile matrix to use.
        row : integer
            Row index of tile to request.
        column : integer
            Column index of tile to request.
        **kwargs : extra arguments
            anything else e.g. vendor specific parameters

        Example
        -------
            >>> url = 'http://map1c.vis.earthdata.nasa.gov/wmts-geo/wmts.cgi'
            >>> wmts = WebMapTileService(url)
            >>> img = wmts.gettile(layer='VIIRS_CityLights_2012',\
                                   tilematrixset='EPSG4326_500m',\
                                   tilematrix='6',\
                                   row=4, column=4)
            >>> out = open('tile.jpg', 'wb')
            >>> out.write(img.read())
            >>> out.close()

        """
        data = self.buildTileRequest(layer, style, format, tilematrixset, tilematrix, row, column, **kwargs)

        if base_url is None:
            base_url = self.url
            try:
                get_verbs = filter(lambda x: x.get('type').lower() == 'get', self.getOperationByName('GetTile').methods)
                if len(get_verbs) > 1:
                    # Filter by constraints
                    base_url = next(x for x in filter(list, ([pv.get('url') for const in pv.get('constraints') if 'kvp' in map(lambda x: x.lower(), const.values)] for pv in get_verbs if pv.get('constraints'))))[0]
                elif len(get_verbs) == 1:
                    base_url = get_verbs[0].get('url')
            except StopIteration:
                pass
        u = openURL(base_url, data, username=self.username,
                    password=self.password)

        # check for service exceptions, and return
        if u.info()['Content-Type'] == 'application/vnd.ogc.se_xml':
            se_xml = u.read()
            se_tree = etree.fromstring(se_xml)
            err_message = unicode(se_tree.find('ServiceException').text)
            raise ServiceException(err_message.strip(), se_xml)
        return u

    def getServiceXML(self):
        xml = None
        if self._capabilities is not None:
            xml = etree.tostring(self._capabilities)
        return xml

    def getfeatureinfo(self):
        raise NotImplementedError

    def getOperationByName(self, name):
        """Return a named content item."""
        for item in self.operations:
            if item.name == name:
                return item
        raise KeyError, "No operation named %s" % name

class TileMatrixSet(object):
    '''Holds one TileMatrixSet'''
    def __init__(self, elem):
        if elem.tag != '{http://www.opengis.net/wmts/1.0}TileMatrixSet':
            raise ValueError('%s should be a TileMatrixSet' % (elem,))
        self.identifier = testXMLValue(elem.find('{http://www.opengis.net/ows/1.1}Identifier')).strip()
        self.crs = testXMLValue(elem.find('{http://www.opengis.net/ows/1.1}SupportedCRS')).strip()
        if (self.crs == None) or (self.identifier == None):
            raise ValueError('%s incomplete TileMatrixSet' % (elem,))
        self.tilematrix = {}
        for tilematrix in elem.findall('{http://www.opengis.net/wmts/1.0}TileMatrix'):
            tm = TileMatrix(tilematrix)
            if tm.identifier:
                if tm.identifier in self.tilematrix:
                    raise KeyError('TileMatrix with identifier "%s" already exists' % tm.identifier)
                self.tilematrix[tm.identifier] = tm

class TileMatrix(object):
    '''Holds one TileMatrix'''
    def __init__(self, elem):
        if elem.tag != '{http://www.opengis.net/wmts/1.0}TileMatrix':
            raise ValueError('%s should be a TileMatrix' % (elem,))
        self.identifier = testXMLValue(elem.find('{http://www.opengis.net/ows/1.1}Identifier')).strip()
        sd = testXMLValue(elem.find('{http://www.opengis.net/wmts/1.0}ScaleDenominator'))
        if sd is None:
            raise ValueError('%s is missing ScaleDenominator' % (elem,))
        self.scaledenominator = float(sd)
        tl = testXMLValue(elem.find('{http://www.opengis.net/wmts/1.0}TopLeftCorner'))
        if tl is None:
            raise ValueError('%s is missing TopLeftCorner' % (elem,))
        (lon, lat) = tl.split(" ")
        self.topleftcorner = (float(lon), float(lat))
        width = testXMLValue(elem.find('{http://www.opengis.net/wmts/1.0}TileWidth'))
        height = testXMLValue(elem.find('{http://www.opengis.net/wmts/1.0}TileHeight'))
        if (width is None) or (height is None):
            raise ValueError('%s is missing TileWidth and/or TileHeight' % (elem,))
        self.tilewidth = int(width)
        self.tileheight = int(height)
        mw = testXMLValue(elem.find('{http://www.opengis.net/wmts/1.0}MatrixWidth'))
        mh = testXMLValue(elem.find('{http://www.opengis.net/wmts/1.0}MatrixHeight'))
        if (mw is None) or (mh is None):
            raise ValueError('%s is missing MatrixWidth and/or MatrixHeight' % (elem,))
        self.matrixwidth = int(mw)
        self.matrixheight = int(mh)

class Theme:
    """
    Abstraction for a WMTS theme
    """
    def __init__(self, elem):
        if elem.tag != '{http://www.opengis.net/wmts/1.0}Theme':
            raise ValueError('%s should be a Theme' % (elem,))
        self.identifier = testXMLValue(elem.find('{http://www.opengis.net/ows/1.1}Identifier')).strip()
        title = testXMLValue(elem.find('{http://www.opengis.net/ows/1.1}Title'))
        if title is not None:
            self.title = title.strip()
        else:
            self.title = None
        abstract = testXMLValue(elem.find('{http://www.opengis.net/ows/1.1}Abstract'))
        if abstract is not None:
            self.abstract = abstract.strip()
        else:
            self.abstract = None

        self.layerRefs = []
        layerRefs = elem.findall('{http://www.opengis.net/wmts/1.0}LayerRef')
        for layerRef in layerRefs:
            if layerRef.text is not None:
                self.layerRefs.append(layerRef.text)


class TileMatrixLimits(object):
    """
    Represents a WMTS TileMatrixLimits element.

    """
    def __init__(self, elem):
        if elem.tag != _TILE_MATRIX_LIMITS_TAG:
            raise ValueError('%s should be a TileMatrixLimits' % elem)

        tm = elem.find(_TILE_MATRIX_TAG)
        if tm is None:
            raise ValueError('Missing TileMatrix in %s' % elem)
        self.tilematrix = tm.text.strip()

        self.mintilerow = getXMLInteger(elem, _MIN_TILE_ROW_TAG)
        self.maxtilerow = getXMLInteger(elem, _MAX_TILE_ROW_TAG)
        self.mintilecol = getXMLInteger(elem, _MIN_TILE_COL_TAG)
        self.maxtilecol = getXMLInteger(elem, _MAX_TILE_COL_TAG)

    def __repr__(self):
        fmt = ('<TileMatrixLimits: {self.tilematrix}'
               ', minRow={self.mintilerow}, maxRow={self.maxtilerow}'
               ', minCol={self.mintilecol}, maxCol={self.maxtilecol}>')
        return fmt.format(self=self)


class TileMatrixSetLink(object):
    """
    Represents a WMTS TileMatrixSetLink element.

    """
    @staticmethod
    def from_elements(link_elements):
        """
        Return a list of TileMatrixSetLink instances derived from the
        given list of <TileMatrixSetLink> XML elements.

        """
        # NB. The WMTS spec is contradictory re. the multiplicity
        # relationships between Layer and TileMatrixSetLink, and
        # TileMatrixSetLink and tileMatrixSet (URI).
        # Try to figure out which model has been used by the server.
        links = []
        for link_element in link_elements:
            matrix_set_elements = link_element.findall(_TILE_MATRIX_SET_TAG)
            if len(matrix_set_elements) == 0:
                raise ValueError('Missing TileMatrixSet in %s' % link_element)
            elif len(matrix_set_elements) > 1:
                set_limits_elements = link_element.findall(
                    _TILE_MATRIX_SET_LIMITS_TAG)
                if set_limits_elements:
                   raise ValueError('Multiple instances of TileMatrixSet'
                                    ' plus TileMatrixSetLimits in %s' %
                                    link_element)
                for matrix_set_element in matrix_set_elements:
                    uri = matrix_set_element.text.strip()
                    links.append(TileMatrixSetLink(uri))
            else:
                uri = matrix_set_elements[0].text.strip()

                tilematrixlimits = {}
                path = '%s/%s' % (_TILE_MATRIX_SET_LIMITS_TAG, _TILE_MATRIX_LIMITS_TAG)
                for limits_element in link_element.findall(path):
                    tml = TileMatrixLimits(limits_element)
                    if tml.tilematrix:
                        if tml.tilematrix in tilematrixlimits:
                            raise KeyError('TileMatrixLimits with tileMatrix "%s" already exists' % tml.tilematrix)
                        tilematrixlimits[tml.tilematrix] = tml

                links.append(TileMatrixSetLink(uri, tilematrixlimits))
        return links

    def __init__(self, tilematrixset, tilematrixlimits=None):
        self.tilematrixset = tilematrixset

        if tilematrixlimits is None:
            self.tilematrixlimits = {}
        else:
            self.tilematrixlimits = tilematrixlimits

    def __repr__(self):
        fmt = ('<TileMatrixSetLink: {self.tilematrixset}'
               ', tilematrixlimits={{...}}>')
        return fmt.format(self=self)


class ContentMetadata:
    """
    Abstraction for WMTS layer metadata.

    Implements IContentMetadata.
    """
    def __init__(self, elem, parent=None, index=0, parse_remote_metadata=False):
        if elem.tag != '{http://www.opengis.net/wmts/1.0}Layer':
            raise ValueError('%s should be a Layer' % (elem,))

        self.parent = parent
        if parent:
            self.index = "%s.%d" % (parent.index, index)
        else:
            self.index = str(index)

        self.id = self.name = testXMLValue(elem.find('{http://www.opengis.net/ows/1.1}Identifier'))
        # title is mandatory property
        self.title = None
        title = testXMLValue(elem.find('{http://www.opengis.net/ows/1.1}Title'))
        if title is not None:
            self.title = title.strip()

        self.abstract = testXMLValue(elem.find('{http://www.opengis.net/ows/1.1}Abstract'))

        # bboxes
        b = elem.find('{http://www.opengis.net/ows/1.1}WGS84BoundingBox')
        self.boundingBox = None
        if b is not None:
            lc = b.find("{http://www.opengis.net/ows/1.1}LowerCorner")
            uc = b.find("{http://www.opengis.net/ows/1.1}UpperCorner")
            ll = [float(s) for s in lc.text.split()]
            ur = [float(s) for s in uc.text.split()]
            self.boundingBoxWGS84 = (ll[0],ll[1],ur[0],ur[1])
        # TODO: there is probably some more logic here, and it should probably be shared code

        self._tilematrixsets = [f.text.strip() for f in
                                elem.findall(_TILE_MATRIX_SET_LINK_TAG + '/' +
                                             _TILE_MATRIX_SET_TAG)]

        link_elements = elem.findall(_TILE_MATRIX_SET_LINK_TAG)
        tile_matrix_set_links = TileMatrixSetLink.from_elements(link_elements)
        self.tilematrixsetlinks = {}
        for tmsl in tile_matrix_set_links:
            if tmsl.tilematrixset:
                if tmsl.tilematrixset in self.tilematrixsetlinks:
                    raise KeyError('TileMatrixSetLink with tilematrixset "%s"'
                                   ' already exists' %
                                   tmsl.tilematrixset)
                self.tilematrixsetlinks[tmsl.tilematrixset] = tmsl

        self.resourceURLs = []
        for resourceURL in elem.findall('{http://www.opengis.net/wmts/1.0}ResourceURL'):
            resource = {}
            for attrib in ['format', 'resourceType', 'template']:
                resource[attrib] = resourceURL.attrib[attrib]
            self.resourceURLs.append(resource)

        #Styles
        self.styles = {}
        for s in elem.findall('{http://www.opengis.net/wmts/1.0}Style'):
            style = {}
            isdefaulttext = s.attrib.get('isDefault')
            style['isDefault'] = (isdefaulttext == "true")
            identifier = s.find('{http://www.opengis.net/ows/1.1}Identifier')
            if identifier is None:
                raise ValueError('%s missing identifier' % (s,))
            title = s.find('{http://www.opengis.net/ows/1.1}Title')
            if title is not None:
                style['title'] = title.text
            self.styles[identifier.text] = style

        self.formats = [f.text for f in elem.findall('{http://www.opengis.net/wmts/1.0}Format')]

        self.infoformats = [f.text for f in elem.findall('{http://www.opengis.net/wmts/1.0}InfoFormat')]

        self.layers = []
        for child in elem.findall('{http://www.opengis.net/wmts/1.0}Layer'):
            self.layers.append(ContentMetadata(child, self))

    @property
    def tilematrixsets(self):
        # NB. This attribute has been superseeded by the
        # `tilematrixsetlinks` attribute defined below, but is included
        # for now to provide continuity.
        warnings.warn("The 'tilematrixsets' attribute has been deprecated"
                      " and will be removed in a future version of OWSLib."
                      " Please use 'tilematrixsetlinks' instead.")
        return self._tilematrixsets

    def __str__(self):
        return 'Layer Name: %s Title: %s' % (self.name, self.title)


class WMTSCapabilitiesReader:
    """Read and parse capabilities document into a lxml.etree infoset
    """

    def __init__(self, version='1.0.0', url=None, un=None, pw=None):
        """Initialize"""
        self.version = version
        self._infoset = None
        self.url = url
        self.username = un
        self.password = pw

    def capabilities_url(self, service_url):
        """Return a capabilities url
        """
        qs = []
        if service_url.find('?') != -1:
            qs = urlparse.parse_qsl(service_url.split('?')[1])

        params = [x[0] for x in qs]

        if 'service' not in params:
            qs.append(('service', 'WMTS'))
        if 'request' not in params:
            qs.append(('request', 'GetCapabilities'))
        if 'version' not in params:
            qs.append(('version', self.version))

        urlqs = urlencode(tuple(qs))
        return service_url.split('?')[0] + '?' + urlqs

    def read(self, service_url):
        """Get and parse a WMTS capabilities document, returning an
        elementtree instance

        service_url is the base url, to which is appended the service,
        version, and request parameters
        """
        getcaprequest = self.capabilities_url(service_url)

        #now split it up again to use the generic openURL function...
        spliturl=getcaprequest.split('?')
        u = openURL(spliturl[0], spliturl[1], method='Get', username = self.username, password = self.password)
        return etree.fromstring(u.read())

    def readString(self, st):
        """Parse a WMTS capabilities document, returning an elementtree instance

        string should be an XML capabilities document
        """
        if not isinstance(st, str):
            raise ValueError("String must be of type string, not %s" % type(st))
        return etree.fromstring(st)
