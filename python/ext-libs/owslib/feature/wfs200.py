# =============================================================================
# OWSLib. Copyright (C) 2005 Sean C. Gillies
#
# Contact email: sgillies@frii.com
#
# $Id: wfs.py 503 2006-02-01 17:09:12Z dokai $
# =============================================================================

from __future__ import (absolute_import, division, print_function)

#owslib imports:
from owslib.ows import ServiceIdentification, ServiceProvider, OperationsMetadata
from owslib.etree import etree
from owslib.util import nspath, testXMLValue, openURL
from owslib.crs import Crs
from owslib.feature import WebFeatureService_
from owslib.namespaces import Namespaces

#other imports
import cgi
from six import PY2
from six.moves import cStringIO as StringIO
try:
    from urllib import urlencode
except ImportError:
    from urllib.parse import urlencode

import logging
from owslib.util import log

n = Namespaces()
WFS_NAMESPACE = n.get_namespace("wfs20")
OWS_NAMESPACE = n.get_namespace("ows110")
OGC_NAMESPACE = n.get_namespace("ogc")
GML_NAMESPACE = n.get_namespace("gml")
FES_NAMESPACE = n.get_namespace("fes")


class ServiceException(Exception):
    pass


class WebFeatureService_2_0_0(WebFeatureService_):
    """Abstraction for OGC Web Feature Service (WFS).

    Implements IWebFeatureService.
    """
    def __new__(self,url, version, xml, parse_remote_metadata=False, timeout=30):
        """ overridden __new__ method 
        
        @type url: string
        @param url: url of WFS capabilities document
        @type xml: string
        @param xml: elementtree object
        @type parse_remote_metadata: boolean
        @param parse_remote_metadata: whether to fully process MetadataURL elements
        @param timeout: time (in seconds) after which requests should timeout
        @return: initialized WebFeatureService_2_0_0 object
        """
        obj=object.__new__(self)
        obj.__init__(url, version, xml, parse_remote_metadata, timeout)
        return obj
    
    def __getitem__(self,name):
        ''' check contents dictionary to allow dict like access to service layers'''
        if name in self.__getattribute__('contents').keys():
            return self.__getattribute__('contents')[name]
        else:
            raise KeyError("No content named %s" % name)
    
    
    def __init__(self, url,  version, xml=None, parse_remote_metadata=False, timeout=30):
        """Initialize."""
        if log.isEnabledFor(logging.DEBUG):
            log.debug('building WFS %s'%url)
        self.url = url
        self.version = version
        self.timeout = timeout
        self._capabilities = None
        reader = WFSCapabilitiesReader(self.version)
        if xml:
            self._capabilities = reader.readString(xml)
        else:
            self._capabilities = reader.read(self.url)
        self._buildMetadata(parse_remote_metadata)
    
    def _buildMetadata(self, parse_remote_metadata=False):
        '''set up capabilities metadata objects: '''
        
        #serviceIdentification metadata
        serviceidentelem=self._capabilities.find(nspath('ServiceIdentification'))
        self.identification=ServiceIdentification(serviceidentelem)  
        #need to add to keywords list from featuretypelist information:
        featuretypelistelem=self._capabilities.find(nspath('FeatureTypeList', ns=WFS_NAMESPACE))
        featuretypeelems=featuretypelistelem.findall(nspath('FeatureType', ns=WFS_NAMESPACE))
        for f in featuretypeelems:  
            kwds=f.findall(nspath('Keywords/Keyword',ns=OWS_NAMESPACE))
            if kwds is not None:
                for kwd in kwds[:]:
                    if kwd.text not in self.identification.keywords:
                        self.identification.keywords.append(kwd.text)
	
   
        #TODO: update serviceProvider metadata, miss it out for now
        serviceproviderelem=self._capabilities.find(nspath('ServiceProvider'))
        self.provider=ServiceProvider(serviceproviderelem)   
        
        #serviceOperations metadata 
        self.operations=[]
        
        for elem in self._capabilities.find(nspath('OperationsMetadata'))[:]:
            if elem.tag !=nspath('ExtendedCapabilities'):
                self.operations.append(OperationsMetadata(elem))
                   
        #serviceContents metadata: our assumption is that services use a top-level 
        #layer as a metadata organizer, nothing more. 
        
        self.contents={} 
        featuretypelist=self._capabilities.find(nspath('FeatureTypeList',ns=WFS_NAMESPACE))
        features = self._capabilities.findall(nspath('FeatureTypeList/FeatureType', ns=WFS_NAMESPACE))
        for feature in features:
            cm=ContentMetadata(feature, featuretypelist, parse_remote_metadata)
            self.contents[cm.id]=cm       
        
        #exceptions
        self.exceptions = [f.text for f \
                in self._capabilities.findall('Capability/Exception/Format')]
      
    def getcapabilities(self):
        """Request and return capabilities document from the WFS as a 
        file-like object.
        NOTE: this is effectively redundant now"""
        reader = WFSCapabilitiesReader(self.version)
        return openURL(reader.capabilities_url(self.url), timeout=self.timeout)
    
    def items(self):
        '''supports dict-like items() access'''
        items=[]
        for item in self.contents:
            items.append((item,self.contents[item]))
        return items

    def _makeStringIO(self, strval):
        """
        Helper method to make sure the StringIO being returned will work.

        Differences between Python 2.6/2.7/3.x mean we have a lot of cases to handle.
        """
        if PY2:
            return StringIO(strval)

        return StringIO(strval.decode())

    def getfeature(self, typename=None, filter=None, bbox=None, featureid=None,
                   featureversion=None, propertyname=None, maxfeatures=None,storedQueryID=None, storedQueryParams=None,
                   method='Get', outputFormat=None, startindex=None):
        """Request and return feature data as a file-like object.
        #TODO: NOTE: have changed property name from ['*'] to None - check the use of this in WFS 2.0
        Parameters
        ----------
        typename : list
            List of typenames (string)
        filter : string 
            XML-encoded OGC filter expression.
        bbox : tuple
            (left, bottom, right, top) in the feature type's coordinates == (minx, miny, maxx, maxy)
        featureid : list
            List of unique feature ids (string)
        featureversion : string
            Default is most recent feature version.
        propertyname : list
            List of feature property names. '*' matches all.
        maxfeatures : int
            Maximum number of features to be returned.
        method : string
            Qualified name of the HTTP DCP method to use.
        outputFormat: string (optional)
            Requested response format of the request.
        startindex: int (optional)
            Start position to return feature set (paging in combination with maxfeatures)

        There are 3 different modes of use

        1) typename and bbox (simple spatial query)
        2) typename and filter (==query) (more expressive)
        3) featureid (direct access to known features)
        """
        storedQueryParams = storedQueryParams or {}
        url = data = None
        if typename and type(typename) == type(""):
            typename = [typename]
        if method.upper() == "GET":
            (url) = self.getGETGetFeatureRequest(typename, filter, bbox, featureid,
                                                 featureversion, propertyname,
                                                 maxfeatures, storedQueryID,
                                                 storedQueryParams, outputFormat, 'Get', startindex)
            if log.isEnabledFor(logging.DEBUG):
                log.debug('GetFeature WFS GET url %s'% url)
        else:
            (url,data) = self.getPOSTGetFeatureRequest()


        # If method is 'Post', data will be None here
        u = openURL(url, data, method, timeout=self.timeout)

        # check for service exceptions, rewrap, and return
        # We're going to assume that anything with a content-length > 32k
        # is data. We'll check anything smaller.
        if 'Content-Length' in u.info():
            length = int(u.info()['Content-Length'])
            have_read = False
        else:
            data = u.read()
            have_read = True
            length = len(data)

        if length < 32000:
            if not have_read:
                data = u.read()

            try:
                tree = etree.fromstring(data)
            except BaseException:
                # Not XML
                return self._makeStringIO(data)
            else:
                if tree.tag == "{%s}ServiceExceptionReport" % OGC_NAMESPACE:
                    se = tree.find(nspath('ServiceException', OGC_NAMESPACE))
                    raise ServiceException(str(se.text).strip())
                else:
                    return self._makeStringIO(data)
        else:
            if have_read:
                return self._makeStringIO(data)
            return u


    def getpropertyvalue(self, query=None, storedquery_id=None, valuereference=None, typename=None, method=nspath('Get'),**kwargs):
        ''' the WFS GetPropertyValue method'''
        try:
            base_url = next((m.get('url') for m in self.getOperationByName('GetPropertyValue').methods if m.get('type').lower() == method.lower()))
        except StopIteration:
            base_url = self.url
        request = {'service': 'WFS', 'version': self.version, 'request': 'GetPropertyValue'}
        if query:
            request['query'] = str(query)
        if valuereference: 
            request['valueReference'] = str(valuereference)
        if storedquery_id: 
            request['storedQuery_id'] = str(storedquery_id)
        if typename:
            request['typename']=str(typename)
        if kwargs:
            for kw in kwargs.keys():
                request[kw]=str(kwargs[kw])
        encoded_request=urlencode(request)
        u = openURL(base_url + encoded_request)
        return u.read()
        
        
    def _getStoredQueries(self):
        ''' gets descriptions of the stored queries available on the server '''
        sqs=[]
        #This method makes two calls to the WFS - one ListStoredQueries, and one DescribeStoredQueries. The information is then
        #aggregated in 'StoredQuery' objects
        method=nspath('Get')
        
        #first make the ListStoredQueries response and save the results in a dictionary if form {storedqueryid:(title, returnfeaturetype)}
        try:
            base_url = next((m.get('url') for m in self.getOperationByName('ListStoredQueries').methods if m.get('type').lower() == method.lower()))
        except StopIteration:
            base_url = self.url

        request = {'service': 'WFS', 'version': self.version, 'request': 'ListStoredQueries'}
        encoded_request = urlencode(request)
        u = openURL(base_url, data=encoded_request, timeout=self.timeout)
        tree=etree.fromstring(u.read())
        tempdict={}       
        for sqelem in tree[:]:
            title=rft=id=None
            id=sqelem.get('id')
            for elem in sqelem[:]:
                if elem.tag==nspath('Title', WFS_NAMESPACE):
                    title=elem.text
                elif elem.tag==nspath('ReturnFeatureType', WFS_NAMESPACE):
                    rft=elem.text
            tempdict[id]=(title,rft)        #store in temporary dictionary
        
        #then make the DescribeStoredQueries request and get the rest of the information about the stored queries 
        try:
            base_url = next((m.get('url') for m in self.getOperationByName('DescribeStoredQueries').methods if m.get('type').lower() == method.lower()))
        except StopIteration:
            base_url = self.url
        request = {'service': 'WFS', 'version': self.version, 'request': 'DescribeStoredQueries'}
        encoded_request = urlencode(request)
        u = openURL(base_url, data=encoded_request, timeout=self.timeout)
        tree=etree.fromstring(u.read())
        tempdict2={} 
        for sqelem in tree[:]:
            params=[] #list to store parameters for the stored query description
            id =sqelem.get('id')
            for elem in sqelem[:]:
                abstract = ''
                if elem.tag==nspath('Abstract', WFS_NAMESPACE):
                    abstract=elem.text
                elif elem.tag==nspath('Parameter', WFS_NAMESPACE):
                    newparam=Parameter(elem.get('name'), elem.get('type'))
                    params.append(newparam)
            tempdict2[id]=(abstract, params) #store in another temporary dictionary
        
        #now group the results into StoredQuery objects:
        for key in tempdict.keys():
            sqs.append(StoredQuery(key, tempdict[key][0], tempdict[key][1], tempdict2[key][0], tempdict2[key][1]))
        return sqs
    storedqueries = property(_getStoredQueries, None)

    def getOperationByName(self, name):
        """Return a named content item."""
        for item in self.operations:
            if item.name == name:
                return item
        raise KeyError("No operation named %s" % name)

class StoredQuery(object):
    '''' Class to describe a storedquery '''
    def __init__(self, id, title, returntype, abstract, parameters):
        self.id=id
        self.title=title
        self.returnfeaturetype=returntype
        self.abstract=abstract
        self.parameters=parameters
        
class Parameter(object):
    def __init__(self, name, type):
        self.name=name
        self.type=type
        
    
class ContentMetadata:
    """Abstraction for WFS metadata.
    
    Implements IMetadata.
    """

    def __init__(self, elem, parent, parse_remote_metadata=False, timeout=30):
        """."""
        self.id = elem.find(nspath('Name',ns=WFS_NAMESPACE)).text
        self.title = elem.find(nspath('Title',ns=WFS_NAMESPACE)).text
        abstract = elem.find(nspath('Abstract',ns=WFS_NAMESPACE))
        if abstract is not None:
            self.abstract = abstract.text
        else:
            self.abstract = None
        self.keywords = [f.text for f in elem.findall(nspath('Keywords',ns=WFS_NAMESPACE))]

        # bboxes
        self.boundingBoxWGS84 = None
        b = elem.find(nspath('WGS84BoundingBox',ns=OWS_NAMESPACE))
        if b is not None:
            lc = b.find(nspath("LowerCorner",ns=OWS_NAMESPACE))
            uc = b.find(nspath("UpperCorner",ns=OWS_NAMESPACE))
            ll = [float(s) for s in lc.text.split()]
            ur = [float(s) for s in uc.text.split()]
            self.boundingBoxWGS84 = (ll[0],ll[1],ur[0],ur[1])

        # there is no such think as bounding box
        # make copy of the WGS84BoundingBox
        self.boundingBox = (self.boundingBoxWGS84[0],
                            self.boundingBoxWGS84[1],
                            self.boundingBoxWGS84[2],
                            self.boundingBoxWGS84[3],
                            Crs("epsg:4326"))
        # crs options
        self.crsOptions = [Crs(srs.text) for srs in elem.findall(nspath('OtherCRS',ns=WFS_NAMESPACE))]
        defaultCrs =  elem.findall(nspath('DefaultCRS',ns=WFS_NAMESPACE))
        if len(defaultCrs) > 0:
            self.crsOptions.insert(0,Crs(defaultCrs[0].text))


        # verbs
        self.verbOptions = [op.tag for op \
            in parent.findall(nspath('Operations/*',ns=WFS_NAMESPACE))]
        self.verbOptions + [op.tag for op \
            in elem.findall(nspath('Operations/*',ns=WFS_NAMESPACE)) \
            if op.tag not in self.verbOptions]
        
        #others not used but needed for iContentMetadata harmonisation
        self.styles=None
        self.timepositions=None
        self.defaulttimeposition=None

        # MetadataURLs
        self.metadataUrls = []
        for m in elem.findall('MetadataURL'):
            metadataUrl = {
                'type': testXMLValue(m.attrib['type'], attrib=True),
                'format': m.find('Format').text.strip(),
                'url': testXMLValue(m.find('OnlineResource').attrib['{http://www.w3.org/1999/xlink}href'], attrib=True)
            }

            if metadataUrl['url'] is not None and parse_remote_metadata:  # download URL
                try:
                    content = openURL(metadataUrl['url'], timeout=timeout)
                    doc = etree.parse(content)
                    try:  # FGDC
                        metadataUrl['metadata'] = Metadata(doc)
                    except:  # ISO
                        metadataUrl['metadata'] = MD_Metadata(doc)
                except Exception:
                    metadataUrl['metadata'] = None

            self.metadataUrls.append(metadataUrl)


class WFSCapabilitiesReader(object):
    """Read and parse capabilities document into a lxml.etree infoset
    """

    def __init__(self, version='2.0.0'):
        """Initialize"""
        self.version = version
        self._infoset = None

    def capabilities_url(self, service_url):
        """Return a capabilities url
        """
        qs = []
        if service_url.find('?') != -1:
            qs = cgi.parse_qsl(service_url.split('?')[1])

        params = [x[0] for x in qs]

        if 'service' not in params:
            qs.append(('service', 'WFS'))
        if 'request' not in params:
            qs.append(('request', 'GetCapabilities'))
        if 'version' not in params:
            qs.append(('version', self.version))

        urlqs = urlencode(tuple(qs))
        return service_url.split('?')[0] + '?' + urlqs

    def read(self, url, timeout=30):
        """Get and parse a WFS capabilities document, returning an
        instance of WFSCapabilitiesInfoset

        Parameters
        ----------
        url : string
            The URL to the WFS capabilities document.
        timeout : number
            A timeout value (in seconds) for the request.
        """
        request = self.capabilities_url(url)
        u = openURL(request, timeout=timeout)
        return etree.fromstring(u.read())

    def readString(self, st):
        """Parse a WFS capabilities document, returning an
        instance of WFSCapabilitiesInfoset

        string should be an XML capabilities document
        """
        if not isinstance(st, str) and not isinstance(st, bytes):
            raise ValueError("String must be of type string or bytes, not %s" % type(st))
        return etree.fromstring(st)
