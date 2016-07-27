# -*- coding: ISO-8859-15 -*-
# =============================================================================
# Copyright (c) 2004, 2006 Sean C. Gillies
# Copyright (c) 2007 STFC <http://www.stfc.ac.uk>
#
# Authors : 
#          Dominic Lowe <d.lowe@rl.ac.uk>
#
# Contact email: d.lowe@rl.ac.uk
# =============================================================================

##########NOTE: Does not conform to new interfaces yet #################

from __future__ import (absolute_import, division, print_function)

from .wcsBase import WCSBase, WCSCapabilitiesReader, ServiceException
from owslib.util import openURL, testXMLValue
try:
    from urllib import urlencode
except ImportError:
    from urllib.parse import urlencode
from owslib.etree import etree
import os, errno
from owslib.coverage import wcsdecoder
from owslib.crs import Crs

import logging
from owslib.util import log

class Namespaces_1_1_0():

    def WCS(self, tag):
        return '{http://www.opengis.net/wcs/1.1}'+tag
    
    def WCS_OWS(self, tag):
        return '{http://www.opengis.net/wcs/1.1/ows}'+tag
    
    def OWS(self, tag):
        return '{http://www.opengis.net/ows/1.1}'+tag
    

class WebCoverageService_1_1_0(WCSBase):
    """Abstraction for OGC Web Coverage Service (WCS), version 1.1.0
    Implements IWebCoverageService.
    """
    
    version='1.1.0'
    ns = Namespaces_1_1_0()
    
    def __getitem__(self, name):
        ''' check contents dictionary to allow dict like access to service layers'''
        if name in self.__getattribute__('contents').keys():
            return self.__getattribute__('contents')[name]
        else:
            raise KeyError("No content named %s" % name)
    
    def __init__(self,url,xml, cookies):
        
        self.url = url   
        self.cookies=cookies
        # initialize from saved capability document or access the server
        reader = WCSCapabilitiesReader(self.version)
        if xml:
            self._capabilities = reader.readString(xml)
        else:
            self._capabilities = reader.read(self.url)

        # check for exceptions
        se = self._capabilities.find(self.ns.OWS('Exception'))

        if se is not None:
            err_message = str(se.text).strip()
            raise ServiceException(err_message, xml)

        #build metadata objects:
        
        #serviceIdentification metadata
        elem=self._capabilities.find(self.ns.WCS_OWS('ServiceIdentification'))
        if elem is None:
            elem=self._capabilities.find(self.ns.OWS('ServiceIdentification'))
        self.identification=ServiceIdentification(elem, self.ns)
        
        #serviceProvider
        elem=self._capabilities.find(self.ns.OWS('ServiceProvider')) or self._capabilities.find(self.ns.OWS('ServiceProvider'))
        self.provider=ServiceProvider(elem, self.ns)
                
        #serviceOperations
        self.operations = []
        for elem in self._capabilities.findall(self.ns.WCS_OWS('OperationsMetadata') + '/' + self.ns.WCS_OWS('Operation') + '/'):
            self.operations.append(Operation(elem, self.ns))
        
        # exceptions - ***********TO DO *************
            self.exceptions = [f.text for f \
                in self._capabilities.findall('Capability/Exception/Format')]
              
        # serviceContents: our assumption is that services use a top-level layer
        # as a metadata organizer, nothing more.
        self.contents = {}
        top = self._capabilities.find(self.ns.WCS('Contents') + '/' + self.ns.WCS('CoverageSummary'))
        for elem in self._capabilities.findall(self.ns.WCS('Contents') + '/' + self.ns.WCS('CoverageSummary') + '/' + self.ns.WCS('CoverageSummary')):                    
            cm=ContentMetadata(elem, top, self, self.ns)
            self.contents[cm.id]=cm
            
        if self.contents=={}:
            #non-hierarchical.
            top=None
            for elem in self._capabilities.findall(self.ns.WCS('Contents') + '/' + self.ns.WCS('CoverageSummary')):     
                cm=ContentMetadata(elem, top, self, self.ns)
                #make the describeCoverage requests to populate the supported formats/crs attributes
                self.contents[cm.id]=cm

    def items(self):
        '''supports dict-like items() access'''
        items=[]
        for item in self.contents:
            items.append((item,self.contents[item]))
        return items
          
    #TO DECIDE: Offer repackaging of coverageXML/Multipart MIME output?
    #def getData(self, directory='outputdir', outputfile='coverage.nc',  **kwargs):
        #u=self.getCoverageRequest(**kwargs)
        ##create the directory if it doesn't exist:
        #try:
            #os.mkdir(directory)
        #except OSError, e:
            ## Ignore directory exists error
            #if e.errno <> errno.EEXIST:
                #raise          
        ##elif wcs.version=='1.1.0':
        ##Could be multipart mime or XML Coverages document, need to use the decoder...
        #decoder=wcsdecoder.WCSDecoder(u)
        #x=decoder.getCoverages()
        #if type(x) is wcsdecoder.MpartMime:
            #filenames=x.unpackToDir(directory)
            ##print 'Files from 1.1.0 service written to %s directory'%(directory)
        #else:
            #filenames=x
        #return filenames
    
    #TO DO: Handle rest of the  WCS 1.1.0 keyword parameters e.g. GridCRS etc. 
    def getCoverage(self, identifier=None, bbox=None, time=None, format = None, store=False, rangesubset=None, gridbaseCRS=None, gridtype=None, gridCS=None, gridorigin=None, gridoffsets=None, method='Get',**kwargs):
        """Request and return a coverage from the WCS as a file-like object
        note: additional **kwargs helps with multi-version implementation
        core keyword arguments should be supported cross version
        example:
        cvg=wcs.getCoverageRequest(identifier=['TuMYrRQ4'], time=['2792-06-01T00:00:00.0'], bbox=(-112,36,-106,41),format='application/netcdf', store='true')

        is equivalent to:
        http://myhost/mywcs?SERVICE=WCS&REQUEST=GetCoverage&IDENTIFIER=TuMYrRQ4&VERSION=1.1.0&BOUNDINGBOX=-180,-90,180,90&TIMESEQUENCE=2792-06-01T00:00:00.0&FORMAT=application/netcdf
        
        if store = true, returns a coverages XML file
        if store = false, returns a multipart mime
        """
        if log.isEnabledFor(logging.DEBUG):
            log.debug('WCS 1.1.0 DEBUG: Parameters passed to GetCoverage: identifier=%s, bbox=%s, time=%s, format=%s, rangesubset=%s, gridbaseCRS=%s, gridtype=%s, gridCS=%s, gridorigin=%s, gridoffsets=%s, method=%s, other_arguments=%s'%(identifier, bbox, time, format, rangesubset, gridbaseCRS, gridtype, gridCS, gridorigin, gridoffsets, method, str(kwargs)))       
        
        if method == 'Get':
            method=self.ns.WCS_OWS('Get')
        try:
            base_url = next((m.get('url') for m in self.getOperationByName('GetCoverage').methods if m.get('type').lower() == method.lower()))
        except StopIteration:
            base_url = self.url


        #process kwargs
        request = {'version': self.version, 'request': 'GetCoverage', 'service':'WCS'}
        assert len(identifier) > 0
        request['identifier']=identifier
        #request['identifier'] = ','.join(identifier)
        if bbox:
            request['boundingbox']=','.join([repr(x) for x in bbox])
        if time:
            request['timesequence']=','.join(time)
        request['format']=format
        request['store']=store
        
        #rangesubset: untested - require a server implementation
        if rangesubset:
            request['RangeSubset']=rangesubset
        
        #GridCRS structure: untested - require a server implementation
        if gridbaseCRS:
            request['gridbaseCRS']=gridbaseCRS
        if gridtype:
            request['gridtype']=gridtype
        if gridCS:
            request['gridCS']=gridCS
        if gridorigin:
            request['gridorigin']=gridorigin
        if gridoffsets:
            request['gridoffsets']=gridoffsets
       
       #anything else e.g. vendor specific parameters must go through kwargs
        if kwargs:
            for kw in kwargs:
                request[kw]=kwargs[kw]
        
        #encode and request
        data = urlencode(request)
        
        u=openURL(base_url, data, method, self.cookies)
        return u
        
        
    def getOperationByName(self, name):
        """Return a named operation item."""
        for item in self.operations:
            if item.name == name:
                return item
        raise KeyError("No operation named %s" % name)
        
class Operation(object):
    """Abstraction for operation metadata    
    Implements IOperationMetadata.
    """
    ns = Namespaces_1_1_0()
        
    def __init__(self, elem, nmSpc):
        
        self.name = elem.get('name')       
        self.formatOptions = [f.text for f in elem.findall(nmSpc.WCS_OWS('Parameter') + '/' + nmSpc.WCS_OWS('AllowedValues') + '/' + nmSpc.WCS_OWS('Value'))]
        methods = []
        for verb in elem.findall(nmSpc.WCS_OWS('DCP') + '/' + nmSpc.WCS_OWS('HTTP/*')):
            url = verb.attrib['{http://www.w3.org/1999/xlink}href']
            methods.append((verb.tag, {'url': url}))
        self.methods = dict(methods)

class ServiceIdentification(object):
    """ Abstraction for ServiceIdentification Metadata 
    implements IServiceIdentificationMetadata"""
            
    def __init__(self, elem, nmSpc):        
        self.service="WCS"

        self.title=testXMLValue(elem.find(nmSpc.OWS('Title')))
        if self.title is None:  #may have used the wcs ows namespace:
            self.title=testXMLValue(elem.find(nmSpc.WCS_OWS('Title')))
        if self.title is None:  #may have used the other wcs ows namespace:
            self.title=testXMLValue(elem.find(nmSpc.OWS('Title')))
        
        self.abstract=testXMLValue(elem.find(nmSpc.OWS('Abstract')))
        if self.abstract is None:#may have used the wcs ows namespace:
            self.abstract=testXMLValue(elem.find(nmSpc.WCS_OWS('Abstract')))
        if self.title is None:  #may have used the other wcs ows namespace:
            self.title=testXMLValue(elem.find(nmSpc.OWS('Abstract')))
        if elem.find(nmSpc.OWS('Abstract')) is not None:
            self.abstract=elem.find(nmSpc.OWS('Abstract')).text
        else:
            self.abstract = None
        self.keywords = [f.text for f in elem.findall(nmSpc.OWS('Keywords') + '/' + nmSpc.OWS('Keyword'))]
        #self.link = elem.find(nmSpc.WCS('Service') + '/' + nmSpc.WCS('OnlineResource')).attrib.get('{http://www.w3.org/1999/xlink}href', '')
               
        if elem.find(nmSpc.WCS_OWS('Fees')) is not None:            
            self.fees=elem.find(nmSpc.WCS_OWS('Fees')).text
        else:
            self.fees=None
        
        if  elem.find(nmSpc.WCS_OWS('AccessConstraints')) is not None:
            self.accessConstraints=elem.find(nmSpc.WCS_OWS('AccessConstraints')).text
        else:
            self.accessConstraints=None       
       
class ServiceProvider(object):
    """ Abstraction for ServiceProvider metadata 
    implements IServiceProviderMetadata """
    
    def __init__(self, elem, nmSpc):
        name=elem.find(nmSpc.OWS('ProviderName'))

        if name is not None:
            self.name=name.text
        else:
            self.name=None

        #self.contact=ServiceContact(elem.find(nmSpc.OWS('ServiceContact')))
        self.contact =ContactMetadata(elem, nmSpc)
        self.url=self.name # no obvious definitive place for url in wcs, repeat provider name?

class ContactMetadata(object):
    ''' implements IContactMetadata'''
    
    def __init__(self, elem, nmSpc):
        try:
            self.name = elem.find(nmSpc.OWS('ServiceContact') + '/' + nmSpc.OWS('IndividualName')).text
        except AttributeError:
            self.name = None
        
        try:
            self.organization=elem.find(nmSpc.OWS('ProviderName')).text 
        except AttributeError:
            self.organization = None
        
        try:
            self.address = elem.find(nmSpc.OWS('ServiceContact') + '/' + nmSpc.OWS('ContactInfo') + '/' + nmSpc.OWS('Address') + '/' + nmSpc.OWS('DeliveryPoint')).text
        except AttributeError:
            self.address = None
        try:
            self.city=  elem.find(nmSpc.OWS('ServiceContact') + '/' + nmSpc.OWS('ContactInfo') + '/' + nmSpc.OWS('Address') + '/' + nmSpc.OWS('City')).text
        except AttributeError:
            self.city = None
        
        try:
            self.region= elem.find(nmSpc.OWS('ServiceContact') + '/' + nmSpc.OWS('ContactInfo') + '/' + nmSpc.OWS('Address') + '/' + nmSpc.OWS('AdministrativeArea')).text
        except AttributeError:
            self.region = None
        
        try:
            self.postcode= elem.find(nmSpc.OWS('ServiceContact') + '/' + nmSpc.OWS('ContactInfo') + '/' + nmSpc.OWS('Address') + '/' + nmSpc.OWS('PostalCode')).text
        except AttributeError:
            self.postcode = None
        
        try:
            self.country= elem.find(nmSpc.OWS('ServiceContact') + '/' + nmSpc.OWS('ContactInfo') + '/' + nmSpc.OWS('Address') + '/' + nmSpc.OWS('Country')).text
        except AttributeError:
            self.country = None
        
        try:
            self.email =            elem.find(nmSpc.OWS('ServiceContact') + '/' + nmSpc.OWS('ContactInfo') + '/' + nmSpc.OWS('Address') + '/' + nmSpc.OWS('ElectronicMailAddress')).text
        except AttributeError:
            self.email = None

class ContentMetadata(object):
    """Abstraction for WCS ContentMetadata
    Implements IContentMetadata
    """
    
    def __init__(self, elem, parent, service, nmSpc):
        """Initialize."""
        #TODO - examine the parent for bounding box info.

        self._service=service
        self._elem=elem
        self._parent=parent
        self.id=self._checkChildAndParent(nmSpc.WCS('Identifier'))
        self.description =self._checkChildAndParent(nmSpc.WCS('Description'))           
        self.title =self._checkChildAndParent(nmSpc.OWS('Title'))
        self.abstract =self._checkChildAndParent(nmSpc.OWS('Abstract'))
        
        #keywords.
        self.keywords=[]
        for kw in elem.findall(nmSpc.OWS('Keywords') + '/' + nmSpc.OWS('Keyword')):
            if kw is not None:
                self.keywords.append(kw.text)
        
        #also inherit any keywords from parent coverage summary (if there is one)
        if parent is not None:
            for kw in parent.findall(nmSpc.OWS('Keywords') + '/' + nmSpc.OWS('Keyword')):
                if kw is not None:
                    self.keywords.append(kw.text)
            
        self.boundingBox=None #needed for iContentMetadata harmonisation
        self.boundingBoxWGS84 = None
        b = elem.find(nmSpc.OWS('WGS84BoundingBox'))
        if b is not None:
            lc=b.find(nmSpc.OWS('LowerCorner')).text
            uc=b.find(nmSpc.OWS('UpperCorner')).text
            self.boundingBoxWGS84 = (
                    float(lc.split()[0]),float(lc.split()[1]),
                    float(uc.split()[0]), float(uc.split()[1]),
                    )
                
        # bboxes - other CRS 
        self.boundingboxes = []
        for bbox in elem.findall(nmSpc.OWS('BoundingBox')):
            if bbox is not None:
                try:
                    lc=b.find(nmSpc.OWS('LowerCorner')).text
                    uc=b.find(nmSpc.OWS('UpperCorner')).text
                    boundingBox =  (
                            float(lc.split()[0]),float(lc.split()[1]),
                            float(uc.split()[0]), float(uc.split()[1]),
                            b.attrib['crs'])
                    self.boundingboxes.append(boundingBox)
                except:
                     pass

        #others not used but needed for iContentMetadata harmonisation
        self.styles=None
        self.crsOptions=None
                
        #SupportedCRS
        self.supportedCRS=[]
        for crs in elem.findall(nmSpc.WCS('SupportedCRS')):
            self.supportedCRS.append(Crs(crs.text))
            
            
        #SupportedFormats         
        self.supportedFormats=[]
        for format in elem.findall(nmSpc.WCS('SupportedFormat')):
            self.supportedFormats.append(format.text)
            
    #grid is either a gml:Grid or a gml:RectifiedGrid if supplied as part of the DescribeCoverage response.
    def _getGrid(self):
        grid=None
        #TODO- convert this to 1.1 from 1.0
        #if not hasattr(self, 'descCov'):
                #self.descCov=self._service.getDescribeCoverage(self.id)
        #gridelem= self.descCov.find(ns('CoverageOffering/')+ns('domainSet/')+ns('spatialDomain/')+'{http://www.opengis.net/gml}RectifiedGrid')
        #if gridelem is not None:
            #grid=RectifiedGrid(gridelem)
        #else:
            #gridelem=self.descCov.find(ns('CoverageOffering/')+ns('domainSet/')+ns('spatialDomain/')+'{http://www.opengis.net/gml}Grid')
            #grid=Grid(gridelem)
        return grid
    grid=property(_getGrid, None)
        
        
        
    #time limits/postions require a describeCoverage request therefore only resolve when requested
    def _getTimeLimits(self):
         timelimits=[]
         for elem in self._service.getDescribeCoverage(self.id).findall(ns('CoverageDescription/')+ns('Domain/')+ns('TemporalDomain/')+ns('TimePeriod/')):
             subelems=elem.getchildren()
             timelimits=[subelems[0].text,subelems[1].text]
         return timelimits
    timelimits=property(_getTimeLimits, None)
    
    #TODO timepositions property
    def _getTimePositions(self):
        return []
    timepositions=property(_getTimePositions, None)
    
    def _checkChildAndParent(self, path):
        ''' checks child coverage  summary, and if item not found checks higher level coverage summary'''
        try:
            value = self._elem.find(path).text
        except:
            try:
                value = self._parent.find(path).text
            except:
                value = None
        return value  

