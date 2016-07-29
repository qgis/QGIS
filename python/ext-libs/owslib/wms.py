# -*- coding: iso-8859-15 -*-
# =============================================================================
# Copyright (c) 2004, 2006 Sean C. Gillies
# Copyright (c) 2005 Nuxeo SARL <http://nuxeo.com>
#
# Authors : Sean Gillies <sgillies@frii.com>
#           Julien Anguenot <ja@nuxeo.com>
#
# Contact email: sgillies@frii.com
# =============================================================================

"""
API for Web Map Service (WMS) methods and metadata.

Currently supports only version 1.1.1 of the WMS protocol.
"""

from __future__ import (absolute_import, division, print_function)

import cgi
try:                    # Python 3
    from urllib.parse import urlencode
except ImportError:     # Python 2
    from urllib import urlencode

import warnings

import six

from .etree import etree
from .util import openURL, testXMLValue, extract_xml_list, xmltag_split, OrderedDict
from .fgdc import Metadata
from .iso import MD_Metadata


class ServiceException(Exception):
    """WMS ServiceException

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


class WebMapService(object):
    """Abstraction for OGC Web Map Service (WMS).

    Implements IWebMapService.
    """
    
    def __getitem__(self,name):
        ''' check contents dictionary to allow dict like access to service layers'''
        if name in self.__getattribute__('contents'):
            return self.__getattribute__('contents')[name]
        else:
            raise KeyError("No content named %s" % name)

    
    def __init__(self, url, version='1.1.1', xml=None, username=None, password=None, parse_remote_metadata=False, timeout=30, headers=None):
        """Initialize."""
        self.url = url
        self.username = username
        self.password = password
        self.version = version
        self.timeout = timeout
        self.headers = headers
        self._capabilities = None

        # Authentication handled by Reader
        reader = WMSCapabilitiesReader(self.version, url=self.url, un=self.username, pw=self.password, headers=self.headers)
        if xml:  # read from stored xml
            self._capabilities = reader.readString(xml)
        else:  # read from server
            self._capabilities = reader.read(self.url, timeout=self.timeout)

        # avoid building capabilities metadata if the response is a ServiceExceptionReport
        se = self._capabilities.find('ServiceException') 
        if se is not None: 
            err_message = str(se.text).strip() 
            raise ServiceException(err_message, xml) 

        # build metadata objects
        self._buildMetadata(parse_remote_metadata)

    def _getcapproperty(self):
        if not self._capabilities:
            reader = WMSCapabilitiesReader(
                self.version, url=self.url, un=self.username, pw=self.password, headers=self.headers
                )
            self._capabilities = ServiceMetadata(reader.read(self.url))
        return self._capabilities

    def _buildMetadata(self, parse_remote_metadata=False):
        ''' set up capabilities metadata objects '''
        
        #serviceIdentification metadata
        serviceelem=self._capabilities.find('Service')
        self.identification=ServiceIdentification(serviceelem, self.version)   
        
        #serviceProvider metadata
        self.provider=ServiceProvider(serviceelem)   
            
        #serviceOperations metadata 
        self.operations=[]
        for elem in self._capabilities.find('Capability/Request')[:]:
            self.operations.append(OperationMetadata(elem))
          
        #serviceContents metadata: our assumption is that services use a top-level 
        #layer as a metadata organizer, nothing more.
        self.contents = OrderedDict()
        caps = self._capabilities.find('Capability')
        
        #recursively gather content metadata for all layer elements.
        #To the WebMapService.contents store only metadata of named layers.
        def gather_layers(parent_elem, parent_metadata):
            layers = []
            for index, elem in enumerate(parent_elem.findall('Layer')):
                cm = ContentMetadata(elem, parent=parent_metadata, index=index+1, parse_remote_metadata=parse_remote_metadata)
                if cm.id:
                    if cm.id in self.contents:
                        warnings.warn('Content metadata for layer "%s" already exists. Using child layer' % cm.id)
                    layers.append(cm)
                    self.contents[cm.id] = cm
                cm.children = gather_layers(elem, cm)
            return layers
        gather_layers(caps, None)
        
        #exceptions
        self.exceptions = [f.text for f \
                in self._capabilities.findall('Capability/Exception/Format')]
            
    def items(self):
        '''supports dict-like items() access'''
        items=[]
        for item in self.contents:
            items.append((item,self.contents[item]))
        return items
    
    def getcapabilities(self):
        """Request and return capabilities document from the WMS as a 
        file-like object.
        NOTE: this is effectively redundant now"""
        
        reader = WMSCapabilitiesReader(
            self.version, url=self.url, un=self.username, pw=self.password, headers=self.headers
            )
        u = self._open(reader.capabilities_url(self.url))
        # check for service exceptions, and return
        if u.info()['Content-Type'] == 'application/vnd.ogc.se_xml':
            se_xml = u.read()
            se_tree = etree.fromstring(se_xml)
            err_message = str(se_tree.find('ServiceException').text).strip()
            raise ServiceException(err_message, se_xml)
        return u

    def __build_getmap_request(self, layers=None, styles=None, srs=None, bbox=None,
               format=None, size=None, time=None, transparent=False,
               bgcolor=None, exceptions=None, **kwargs):

        request = {'version': self.version, 'request': 'GetMap'}

        # check layers and styles
        assert len(layers) > 0
        request['layers'] = ','.join(layers)
        if styles:
            assert len(styles) == len(layers)
            request['styles'] = ','.join(styles)
        else:
            request['styles'] = ''

        # size
        request['width'] = str(size[0])
        request['height'] = str(size[1])

        request['srs'] = str(srs)
        request['bbox'] = ','.join([repr(x) for x in bbox])
        request['format'] = str(format)
        request['transparent'] = str(transparent).upper()
        request['bgcolor'] = '0x' + bgcolor[1:7]
        request['exceptions'] = str(exceptions)

        if time is not None:
            request['time'] = str(time)

        if kwargs:
            for kw in kwargs:
                request[kw]=kwargs[kw]
        return request

    def getmap(self, layers=None, styles=None, srs=None, bbox=None,
               format=None, size=None, time=None, transparent=False,
               bgcolor='#FFFFFF',
               exceptions='application/vnd.ogc.se_xml',
               method='Get',
               timeout=None,
               **kwargs
               ):
        """Request and return an image from the WMS as a file-like object.
        
        Parameters
        ----------
        layers : list
            List of content layer names.
        styles : list
            Optional list of named styles, must be the same length as the
            layers list.
        srs : string
            A spatial reference system identifier.
        bbox : tuple
            (left, bottom, right, top) in srs units.
        format : string
            Output image format such as 'image/jpeg'.
        size : tuple
            (width, height) in pixels.
        transparent : bool
            Optional. Transparent background if True.
        bgcolor : string
            Optional. Image background color.
        method : string
            Optional. HTTP DCP method name: Get or Post.
        **kwargs : extra arguments
            anything else e.g. vendor specific parameters
        
        Example
        -------
            >>> wms = WebMapService('http://giswebservices.massgis.state.ma.us/geoserver/wms', version='1.1.1')
            >>> img = wms.getmap(layers=['massgis:GISDATA.SHORELINES_ARC'],\
                                 styles=[''],\
                                 srs='EPSG:4326',\
                                 bbox=(-70.8, 42, -70, 42.8),\
                                 size=(300, 300),\
                                 format='image/jpeg',\
                                 transparent=True)
            >>> out = open('example.jpg', 'wb')
            >>> bytes_written = out.write(img.read())
            >>> out.close()

        """        
        try:
            base_url = next((m.get('url') for m in self.getOperationByName('GetMap').methods if m.get('type').lower() == method.lower()))
        except StopIteration:
            base_url = self.url

        request = self.__build_getmap_request(layers=layers, styles=styles, srs=srs, bbox=bbox,
               format=format, size=size, time=time, transparent=transparent,
               bgcolor=bgcolor, exceptions=exceptions, **kwargs)
        
        data = urlencode(request)
        
        u = openURL(base_url, data, method, username=self.username, password=self.password, timeout=timeout or self.timeout, headers=self.headers)

        # check for service exceptions, and return
        if u.info()['Content-Type'] == 'application/vnd.ogc.se_xml':
            se_xml = u.read()
            se_tree = etree.fromstring(se_xml)
            err_message = six.text_type(se_tree.find('ServiceException').text).strip()
            raise ServiceException(err_message, se_xml)
        return u


    def getfeatureinfo(self, layers=None, styles=None, srs=None, bbox=None,
               format=None, size=None, time=None, transparent=False,
               bgcolor='#FFFFFF',
               exceptions='application/vnd.ogc.se_xml',
               query_layers = None, xy=None, info_format=None, feature_count=20,
               method='Get',
               timeout=None,
               **kwargs
               ):
        try:
            base_url = next((m.get('url') for m in self.getOperationByName('GetFeatureInfo').methods if m.get('type').lower() == method.lower()))
        except StopIteration:
            base_url = self.url

        # GetMap-Request
        request = self.__build_getmap_request(layers=layers, styles=styles, srs=srs, bbox=bbox,
               format=format, size=size, time=time, transparent=transparent,
               bgcolor=bgcolor, exceptions=exceptions, kwargs=kwargs)

        # extend to GetFeatureInfo-Request
        request['request'] = 'GetFeatureInfo'

        if not query_layers:
            __str_query_layers = ','.join(layers)
        else:
            __str_query_layers = ','.join(query_layers)

        request['query_layers'] = __str_query_layers
        request['x'] = str(xy[0])
        request['y'] = str(xy[1])
        request['info_format'] = info_format
        request['feature_count'] = str(feature_count)

        data = urlencode(request)

        u = openURL(base_url, data, method, username=self.username, password=self.password, timeout=timeout or self.timeout, headers=self.headers)

        # check for service exceptions, and return
        if u.info()['Content-Type'] == 'application/vnd.ogc.se_xml':
            se_xml = u.read()
            se_tree = etree.fromstring(se_xml)
            err_message = six.text_type(se_tree.find('ServiceException').text).strip()
            raise ServiceException(err_message, se_xml)
        return u

    def getServiceXML(self):
        xml = None
        if self._capabilities is not None:
            xml = etree.tostring(self._capabilities)
        return xml

    def getOperationByName(self, name): 
        """Return a named content item."""
        for item in self.operations:
            if item.name == name:
                return item
        raise KeyError("No operation named %s" % name)
    
class ServiceIdentification(object):
    ''' Implements IServiceIdentificationMetadata '''
    
    def __init__(self, infoset, version):
        self._root=infoset
        self.type = testXMLValue(self._root.find('Name'))
        self.version = version
        self.title = testXMLValue(self._root.find('Title'))
        self.abstract = testXMLValue(self._root.find('Abstract'))
        self.keywords = extract_xml_list(self._root.findall('KeywordList/Keyword'))
        self.accessconstraints = testXMLValue(self._root.find('AccessConstraints'))
        self.fees = testXMLValue(self._root.find('Fees'))

class ServiceProvider(object):
    ''' Implements IServiceProviderMetatdata '''
    def __init__(self, infoset):
        self._root=infoset
        name=self._root.find('ContactInformation/ContactPersonPrimary/ContactOrganization')
        if name is not None:
            self.name=name.text
        else:
            self.name=None
        self.url=self._root.find('OnlineResource').attrib.get('{http://www.w3.org/1999/xlink}href', '')
        #contact metadata
        contact = self._root.find('ContactInformation')
        ## sometimes there is a contact block that is empty, so make
        ## sure there are children to parse
        if contact is not None and contact[:] != []:
            self.contact = ContactMetadata(contact)
        else:
            self.contact = None
            
    def getContentByName(self, name):
        """Return a named content item."""
        for item in self.contents:
            if item.name == name:
                return item
        raise KeyError("No content named %s" % name)

    def getOperationByName(self, name):
        """Return a named content item."""
        for item in self.operations:
            if item.name == name:
                return item
        raise KeyError("No operation named %s" % name)
        
class ContentMetadata:
    """
    Abstraction for WMS layer metadata.

    Implements IContentMetadata.
    """
    def __init__(self, elem, parent=None, children=None, index=0, parse_remote_metadata=False, timeout=30):
        if elem.tag != 'Layer':
            raise ValueError('%s should be a Layer' % (elem,))
        
        self.parent = parent
        if parent:
            self.index = "%s.%d" % (parent.index, index)
        else:
            self.index = str(index)

        self._children = children
        
        self.id = self.name = testXMLValue(elem.find('Name'))

        # layer attributes
        self.queryable = int(elem.attrib.get('queryable', 0))
        self.cascaded = int(elem.attrib.get('cascaded', 0))
        self.opaque = int(elem.attrib.get('opaque', 0))
        self.noSubsets = int(elem.attrib.get('noSubsets', 0))
        self.fixedWidth = int(elem.attrib.get('fixedWidth', 0))
        self.fixedHeight = int(elem.attrib.get('fixedHeight', 0))

        # title is mandatory property
        self.title = None
        title = testXMLValue(elem.find('Title'))
        if title is not None:
            self.title = title.strip()

        self.abstract = testXMLValue(elem.find('Abstract'))
        
        # bboxes
        b = elem.find('BoundingBox')
        self.boundingBox = None
        if b is not None:
            try: #sometimes the SRS attribute is (wrongly) not provided
                srs=b.attrib['SRS']
            except KeyError:
                srs=None
            self.boundingBox = (
                float(b.attrib['minx']),
                float(b.attrib['miny']),
                float(b.attrib['maxx']),
                float(b.attrib['maxy']),
                srs,
                )
        elif self.parent:
            if hasattr(self.parent, 'boundingBox'):
                self.boundingBox = self.parent.boundingBox

        # ScaleHint 
        sh = elem.find('ScaleHint') 
        self.scaleHint = None 
        if sh is not None: 
            if 'min' in sh.attrib and 'max' in sh.attrib:
                self.scaleHint = {'min': sh.attrib['min'], 'max': sh.attrib['max']} 

        attribution = elem.find('Attribution')
        if attribution is not None:
            self.attribution = dict()
            title = attribution.find('Title')
            url = attribution.find('OnlineResource')
            logo = attribution.find('LogoURL')
            if title is not None: 
                self.attribution['title'] = title.text
            if url is not None:
                self.attribution['url'] = url.attrib['{http://www.w3.org/1999/xlink}href']
            if logo is not None: 
                self.attribution['logo_size'] = (int(logo.attrib['width']), int(logo.attrib['height']))
                self.attribution['logo_url'] = logo.find('OnlineResource').attrib['{http://www.w3.org/1999/xlink}href']

        b = elem.find('LatLonBoundingBox')
        if b is not None:
            self.boundingBoxWGS84 = (
                float(b.attrib['minx']),
                float(b.attrib['miny']),
                float(b.attrib['maxx']),
                float(b.attrib['maxy']),
            )
        elif self.parent:
            self.boundingBoxWGS84 = self.parent.boundingBoxWGS84
        else:
            self.boundingBoxWGS84 = None
            
        #SRS options
        self.crsOptions = []
            
        #Copy any parent SRS options (they are inheritable properties)
        if self.parent:
            self.crsOptions = list(self.parent.crsOptions)

        #Look for SRS option attached to this layer
        if elem.find('SRS') is not None:
            ## some servers found in the wild use a single SRS
            ## tag containing a whitespace separated list of SRIDs
            ## instead of several SRS tags. hence the inner loop
            for srslist in [x.text for x in elem.findall('SRS')]:
                if srslist:
                    for srs in srslist.split():
                        self.crsOptions.append(srs)
                        
        #Get rid of duplicate entries
        self.crsOptions = list(set(self.crsOptions))

        #Set self.crsOptions to None if the layer (and parents) had no SRS options
        if len(self.crsOptions) == 0:
            #raise ValueError('%s no SRS available!?' % (elem,))
            #Comment by D Lowe.
            #Do not raise ValueError as it is possible that a layer is purely a parent layer and does not have SRS specified. Instead set crsOptions to None
            # Comment by Jachym:
            # Do not set it to None, but to [], which will make the code
            # work further. Fixed by anthonybaxter
            self.crsOptions=[]
            
        #Styles
        self.styles = {}
        
        #Copy any parent styles (they are inheritable properties)
        if self.parent:
            self.styles = self.parent.styles.copy()
 
        #Get the styles for this layer (items with the same name are replaced)
        for s in elem.findall('Style'):
            name = s.find('Name')
            title = s.find('Title')
            if name is None or title is None:
                raise ValueError('%s missing name or title' % (s,))
            style = { 'title' : title.text }
            # legend url
            legend = s.find('LegendURL/OnlineResource')
            if legend is not None:
                style['legend'] = legend.attrib['{http://www.w3.org/1999/xlink}href']
            self.styles[name.text] = style

        # keywords
        self.keywords = [f.text for f in elem.findall('KeywordList/Keyword')]

        # timepositions - times for which data is available.
        self.timepositions=None
        self.defaulttimeposition = None
        for extent in elem.findall('Extent'):
            if extent.attrib.get("name").lower() =='time':
                if extent.text:
                    self.timepositions=extent.text.split(',')
                    self.defaulttimeposition = extent.attrib.get("default")
                    break
                
        # Elevations - available vertical levels
        self.elevations=None
        for extent in elem.findall('Extent'):
            if extent.attrib.get("name").lower() =='elevation':
                if extent.text:
                    self.elevations=extent.text.split(',')
                    break                

        # MetadataURLs
        self.metadataUrls = []
        for m in elem.findall('MetadataURL'):
            metadataUrl = {
                'type': testXMLValue(m.attrib['type'], attrib=True),
                'format': testXMLValue(m.find('Format')),
                'url': testXMLValue(m.find('OnlineResource').attrib['{http://www.w3.org/1999/xlink}href'], attrib=True)
            }

            if metadataUrl['url'] is not None and parse_remote_metadata:  # download URL
                try:
                    content = openURL(metadataUrl['url'], timeout=timeout)
                    doc = etree.parse(content)
                    if metadataUrl['type'] is not None:
                        if metadataUrl['type'] == 'FGDC':
                            metadataUrl['metadata'] = Metadata(doc)
                        if metadataUrl['type'] == 'TC211':
                            metadataUrl['metadata'] = MD_Metadata(doc)
                except Exception:
                    metadataUrl['metadata'] = None

            self.metadataUrls.append(metadataUrl)

        # DataURLs
        self.dataUrls = []
        for m in elem.findall('DataURL'):
            dataUrl = {
                'format': m.find('Format').text.strip(),
                'url': m.find('OnlineResource').attrib['{http://www.w3.org/1999/xlink}href']
            }
            self.dataUrls.append(dataUrl)
                
        self.layers = []
        for child in elem.findall('Layer'):
            self.layers.append(ContentMetadata(child, self))

    @property
    def children(self):
        return self._children

    @children.setter
    def children(self, value):
        if self._children is None:
            self._children = value
        else:
            self._children.extend(value)

    def __str__(self):
        return 'Layer Name: %s Title: %s' % (self.name, self.title)


class OperationMetadata:
    """Abstraction for WMS OperationMetadata.
    
    Implements IOperationMetadata.
    """
    def __init__(self, elem):
        """."""
        self.name = xmltag_split(elem.tag)
        # formatOptions
        self.formatOptions = [f.text for f in elem.findall('Format')]
        self.methods = []
        for verb in elem.findall('DCPType/HTTP/*'):
            url = verb.find('OnlineResource').attrib['{http://www.w3.org/1999/xlink}href']
            self.methods.append({'type' : xmltag_split(verb.tag), 'url': url})


class ContactMetadata:
    """Abstraction for contact details advertised in GetCapabilities.
    """
    def __init__(self, elem):
        name = elem.find('ContactPersonPrimary/ContactPerson')
        if name is not None:
            self.name=name.text
        else:
            self.name=None
        email = elem.find('ContactElectronicMailAddress')
        if email is not None:
            self.email=email.text
        else:
            self.email=None
        self.address = self.city = self.region = None
        self.postcode = self.country = None

        address = elem.find('ContactAddress')
        if address is not None:
            street = address.find('Address')
            if street is not None: self.address = street.text

            city = address.find('City')
            if city is not None: self.city = city.text

            region = address.find('StateOrProvince')
            if region is not None: self.region = region.text

            postcode = address.find('PostCode')
            if postcode is not None: self.postcode = postcode.text

            country = address.find('Country')
            if country is not None: self.country = country.text

        organization = elem.find('ContactPersonPrimary/ContactOrganization')
        if organization is not None: self.organization = organization.text
        else:self.organization = None

        position = elem.find('ContactPosition')
        if position is not None: self.position = position.text
        else: self.position = None

      
class WMSCapabilitiesReader:
    """Read and parse capabilities document into a lxml.etree infoset
    """

    def __init__(self, version='1.1.1', url=None, un=None, pw=None, headers=None):
        """Initialize"""
        self.version = version
        self._infoset = None
        self.url = url
        self.username = un
        self.password = pw
        self.headers = headers

        #if self.username and self.password:
            ## Provide login information in order to use the WMS server
            ## Create an OpenerDirector with support for Basic HTTP 
            ## Authentication...
            #passman = HTTPPasswordMgrWithDefaultRealm()
            #passman.add_password(None, self.url, self.username, self.password)
            #auth_handler = HTTPBasicAuthHandler(passman)
            #opener = build_opener(auth_handler)
            #self._open = opener.open

    def capabilities_url(self, service_url):
        """Return a capabilities url
        """
        qs = []
        if service_url.find('?') != -1:
            qs = cgi.parse_qsl(service_url.split('?')[1])

        params = [x[0] for x in qs]

        if 'service' not in params:
            qs.append(('service', 'WMS'))
        if 'request' not in params:
            qs.append(('request', 'GetCapabilities'))
        if 'version' not in params:
            qs.append(('version', self.version))

        urlqs = urlencode(tuple(qs))
        return service_url.split('?')[0] + '?' + urlqs

    def read(self, service_url, timeout=30):
        """Get and parse a WMS capabilities document, returning an
        elementtree instance

        service_url is the base url, to which is appended the service,
        version, and request parameters
        """
        getcaprequest = self.capabilities_url(service_url)

        #now split it up again to use the generic openURL function...
        spliturl=getcaprequest.split('?')
        u = openURL(spliturl[0], spliturl[1], method='Get', username=self.username, password=self.password, timeout=timeout, headers=self.headers)
        return etree.fromstring(u.read())

    def readString(self, st):
        """Parse a WMS capabilities document, returning an elementtree instance

        string should be an XML capabilities document
        """
        if not isinstance(st, str) and not isinstance(st, bytes):
            raise ValueError("String must be of type string or bytes, not %s" % type(st))
        return etree.fromstring(st)
