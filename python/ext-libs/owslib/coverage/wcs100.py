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

from owslib.coverage.wcsBase import WCSBase, WCSCapabilitiesReader, ServiceException
from urllib import urlencode
from owslib.util import openURL, testXMLValue
from owslib.etree import etree
from owslib.crs import Crs
import os, errno

import logging
from owslib.util import log

#  function to save writing out WCS namespace in full each time
def ns(tag):
    return '{http://www.opengis.net/wcs}'+tag

class WebCoverageService_1_0_0(WCSBase):
    """Abstraction for OGC Web Coverage Service (WCS), version 1.0.0
    Implements IWebCoverageService.
    """
    def __getitem__(self,name):
        ''' check contents dictionary to allow dict like access to service layers'''
        if name in self.__getattribute__('contents').keys():
            return self.__getattribute__('contents')[name]
        else:
            raise KeyError, "No content named %s" % name
    
    def __init__(self,url,xml, cookies):
        self.version='1.0.0'
        self.url = url   
        self.cookies=cookies
        # initialize from saved capability document or access the server
        reader = WCSCapabilitiesReader(self.version, self.cookies)
        if xml:
            self._capabilities = reader.readString(xml)
        else:
            self._capabilities = reader.read(self.url)

        # check for exceptions
        se = self._capabilities.find('ServiceException')

        if se is not None:
            err_message = str(se.text).strip()  
            raise ServiceException(err_message, xml) 

        #serviceIdentification metadata
        subelem=self._capabilities.find(ns('Service'))
        self.identification=ServiceIdentification(subelem)                               
                   
        #serviceProvider metadata
        subelem=self._capabilities.find(ns('Service/')+ns('responsibleParty'))
        self.provider=ServiceProvider(subelem)   
        
        #serviceOperations metadata
        self.operations=[]
        for elem in self._capabilities.find(ns('Capability/')+ns('Request'))[:]:
            self.operations.append(OperationMetadata(elem))
          
        #serviceContents metadata
        self.contents={}
        for elem in self._capabilities.findall(ns('ContentMetadata/')+ns('CoverageOfferingBrief')): 
            cm=ContentMetadata(elem, self)
            self.contents[cm.id]=cm
        
        #Some WCS servers (wrongly) advertise 'Content' OfferingBrief instead.
        if self.contents=={}:
            for elem in self._capabilities.findall(ns('ContentMetadata/')+ns('ContentOfferingBrief')): 
                cm=ContentMetadata(elem, self)
                self.contents[cm.id]=cm
        
        #exceptions
        self.exceptions = [f.text for f \
                in self._capabilities.findall('Capability/Exception/Format')]
    
    
    def items(self):
        '''supports dict-like items() access'''
        items=[]
        for item in self.contents:
            items.append((item,self.contents[item]))
        return items
    
    def __makeString(self,value):
        #using repr unconditionally breaks things in some circumstances if a value is already a string
        if type(value) is not str:
            sval=repr(value)
        else:
            sval = value
        return sval
  
    def getCoverage(self, identifier=None, bbox=None, time=None, format = None,  crs=None, width=None, height=None, resx=None, resy=None, resz=None,parameter=None,method='Get',**kwargs):
        """Request and return a coverage from the WCS as a file-like object
        note: additional **kwargs helps with multi-version implementation
        core keyword arguments should be supported cross version
        example:
        cvg=wcs.getCoverage(identifier=['TuMYrRQ4'], timeSequence=['2792-06-01T00:00:00.0'], bbox=(-112,36,-106,41),format='cf-netcdf')

        is equivalent to:
        http://myhost/mywcs?SERVICE=WCS&REQUEST=GetCoverage&IDENTIFIER=TuMYrRQ4&VERSION=1.1.0&BOUNDINGBOX=-180,-90,180,90&TIME=2792-06-01T00:00:00.0&FORMAT=cf-netcdf
           
        """
        if log.isEnabledFor(logging.DEBUG):
            log.debug('WCS 1.0.0 DEBUG: Parameters passed to GetCoverage: identifier=%s, bbox=%s, time=%s, format=%s, crs=%s, width=%s, height=%s, resx=%s, resy=%s, resz=%s, parameter=%s, method=%s, other_arguments=%s'%(identifier, bbox, time, format, crs, width, height, resx, resy, resz, parameter, method, str(kwargs)))
                
        try:
            base_url = next((m.get('url') for m in self.getOperationByName('GetCoverage').methods if m.get('type').lower() == method.lower()))
        except StopIteration:
            base_url = self.url
        
        if log.isEnabledFor(logging.DEBUG):
            log.debug('WCS 1.0.0 DEBUG: base url of server: %s'%base_url)
        
        #process kwargs
        request = {'version': self.version, 'request': 'GetCoverage', 'service':'WCS'}
        assert len(identifier) > 0
        request['Coverage']=identifier
        #request['identifier'] = ','.join(identifier)
        if bbox:
            request['BBox']=','.join([self.__makeString(x) for x in bbox])
        else:
            request['BBox']=None
        if time:
            request['time']=','.join(time)
        if crs:
            request['crs']=crs
        request['format']=format
        if width:
            request['width']=width
        if height:
            request['height']=height
        if resx:
            request['resx']=resx
        if resy:
            request['resy']=resy
        if resz:
            request['resz']=resz
        
        #anything else e.g. vendor specific parameters must go through kwargs
        if kwargs:
            for kw in kwargs:
                request[kw]=kwargs[kw]
        
        #encode and request
        data = urlencode(request)
        if log.isEnabledFor(logging.DEBUG):
            log.debug('WCS 1.0.0 DEBUG: Second part of URL: %s'%data)
        
        
        u=openURL(base_url, data, method, self.cookies)

        return u
    

               
    def getOperationByName(self, name):
        """Return a named operation item."""
        for item in self.operations:
            if item.name == name:
                return item
        raise KeyError, "No operation named %s" % name


class OperationMetadata(object):
    """Abstraction for WCS metadata.   
    Implements IMetadata.
    """
    def __init__(self, elem):
        """."""
        self.name = elem.tag.split('}')[1]          
        
        #self.formatOptions = [f.text for f in elem.findall('{http://www.opengis.net/wcs/1.1/ows}Parameter/{http://www.opengis.net/wcs/1.1/ows}AllowedValues/{http://www.opengis.net/wcs/1.1/ows}Value')]
        self.methods = []
        for resource in elem.findall(ns('DCPType/')+ns('HTTP/')+ns('Get/')+ns('OnlineResource')):
            url = resource.attrib['{http://www.w3.org/1999/xlink}href']
            self.methods.append({'type': 'Get', 'url': url})
        for resource in elem.findall(ns('DCPType/')+ns('HTTP/')+ns('Post/')+ns('OnlineResource')):
            url = resource.attrib['{http://www.w3.org/1999/xlink}href']
            self.methods.append({'type': 'Post', 'url': url})


class ServiceIdentification(object):
    """ Abstraction for ServiceIdentification metadata """
    def __init__(self,elem):
        # properties              
        self.type='OGC:WCS'
        self.version='1.0.0'
        self.service = testXMLValue(elem.find(ns('name')))
        self.abstract = testXMLValue(elem.find(ns('description')))
        self.title = testXMLValue(elem.find(ns('label')))     
        self.keywords = [f.text for f in elem.findall(ns('keywords')+'/'+ns('keyword'))]
        #note: differs from 'rights' in interface
        self.fees=elem.find(ns('fees')).text
        self.accessConstraints=elem.find(ns('accessConstraints')).text
       
class ServiceProvider(object):
    """ Abstraction for WCS ResponsibleParty 
    Implements IServiceProvider"""
    def __init__(self,elem):
        #it's not uncommon for the service provider info to be missing
        #so handle case where None is passed in
        if elem is None:
            self.name=None
            self.url=None
            self.contact = None
        else:
            self.name=testXMLValue(elem.find(ns('organisationName')))
            self.url=self.name #there is no definitive place for url  WCS, repeat organisationName
            self.contact=ContactMetadata(elem)

class ContactMetadata(object):
    ''' implements IContactMetadata'''
    def __init__(self, elem):
        try:
            self.name = elem.find(ns('individualName')).text
        except AttributeError:
            self.name = None
        try:
            self.organization=elem.find(ns('organisationName')).text 
        except AttributeError:
            self.organization = None
        try:
            self.address = elem.find(ns('contactInfo')+'/'+ns('address')+'/'+ns('deliveryPoint')).text
        except AttributeError:
            self.address = None
        try:
            self.city= elem.find(ns('contactInfo')+'/'+ns('address')+'/'+ns('city')).text
        except AttributeError:
            self.city = None
        try:
            self.region=elem.find(ns('contactInfo')+'/'+ns('address')+'/'+ns('administrativeArea')).text
        except AttributeError:
            self.region = None
        try:
            self.postcode=elem.find(ns('contactInfo')+'/'+ns('address')+'/'+ns('postalCode')).text
        except AttributeError:
            self.postcode=None
        try:
            self.country=elem.find(ns('contactInfo')+'/'+ns('address')+'/'+ns('country')).text
        except AttributeError:
            self.country = None
        try:
            self.email=elem.find(ns('contactInfo')+'/'+ns('address')+'/'+ns('electronicMailAddress')).text
        except AttributeError:
            self.email = None

class ContentMetadata(object):
    """
    Implements IContentMetadata
    """
    def __init__(self, elem, service):
        """Initialize. service is required so that describeCoverage requests may be made"""
        #TODO - examine the parent for bounding box info.
        
        #self._parent=parent
        self._elem=elem
        self._service=service
        self.id=elem.find(ns('name')).text
        self.title = testXMLValue(elem.find(ns('label')))
        self.abstract= testXMLValue(elem.find(ns('description')))
        self.keywords = [f.text for f in elem.findall(ns('keywords')+'/'+ns('keyword'))]        
        self.boundingBox=None #needed for iContentMetadata harmonisation
        self.boundingBoxWGS84 = None        
        b = elem.find(ns('lonLatEnvelope')) 
        if b is not None:
            gmlpositions=b.findall('{http://www.opengis.net/gml}pos')
            lc=gmlpositions[0].text
            uc=gmlpositions[1].text
            self.boundingBoxWGS84 = (
                    float(lc.split()[0]),float(lc.split()[1]),
                    float(uc.split()[0]), float(uc.split()[1]),
                    )
        #others not used but needed for iContentMetadata harmonisation
        self.styles=None
        self.crsOptions=None
        self.defaulttimeposition=None

    #grid is either a gml:Grid or a gml:RectifiedGrid if supplied as part of the DescribeCoverage response.
    def _getGrid(self):
        if not hasattr(self, 'descCov'):
                self.descCov=self._service.getDescribeCoverage(self.id)
        gridelem= self.descCov.find(ns('CoverageOffering/')+ns('domainSet/')+ns('spatialDomain/')+'{http://www.opengis.net/gml}RectifiedGrid')
        if gridelem is not None:
            grid=RectifiedGrid(gridelem)
        else:
            gridelem=self.descCov.find(ns('CoverageOffering/')+ns('domainSet/')+ns('spatialDomain/')+'{http://www.opengis.net/gml}Grid')
            grid=Grid(gridelem)
        return grid
    grid=property(_getGrid, None)
        
     #timelimits are the start/end times, timepositions are all timepoints. WCS servers can declare one or both or neither of these.
    def _getTimeLimits(self):
        timepoints, timelimits=[],[]
        b=self._elem.find(ns('lonLatEnvelope'))
        if b is not None:
            timepoints=b.findall('{http://www.opengis.net/gml}timePosition')
        else:
            #have to make a describeCoverage request...
            if not hasattr(self, 'descCov'):
                self.descCov=self._service.getDescribeCoverage(self.id)
            for pos in self.descCov.findall(ns('CoverageOffering/')+ns('domainSet/')+ns('temporalDomain/')+'{http://www.opengis.net/gml}timePosition'):
                timepoints.append(pos)
        if timepoints:
                timelimits=[timepoints[0].text,timepoints[1].text]
        return timelimits
    timelimits=property(_getTimeLimits, None)   
    
    def _getTimePositions(self):
        timepositions=[]
        if not hasattr(self, 'descCov'):
            self.descCov=self._service.getDescribeCoverage(self.id)
        for pos in self.descCov.findall(ns('CoverageOffering/')+ns('domainSet/')+ns('temporalDomain/')+'{http://www.opengis.net/gml}timePosition'):
                timepositions.append(pos.text)
        return timepositions
    timepositions=property(_getTimePositions, None)
           
            
    def _getOtherBoundingBoxes(self):
        ''' incomplete, should return other bounding boxes not in WGS84
            #TODO: find any other bounding boxes. Need to check for gml:EnvelopeWithTimePeriod.'''

        bboxes=[]

        if not hasattr(self, 'descCov'):
            self.descCov=self._service.getDescribeCoverage(self.id)

        for envelope in self.descCov.findall(ns('CoverageOffering/')+ns('domainSet/')+ns('spatialDomain/')+'{http://www.opengis.net/gml}Envelope'):
            bbox = {}
            bbox['nativeSrs'] = envelope.attrib['srsName']
            gmlpositions = envelope.findall('{http://www.opengis.net/gml}pos')
            lc=gmlpositions[0].text.split()
            uc=gmlpositions[1].text.split()
            bbox['bbox'] = (
                float(lc[0]),float(lc[1]),
                float(uc[0]), float(uc[1])
            )
            bboxes.append(bbox)

        return bboxes        
    boundingboxes=property(_getOtherBoundingBoxes,None)
    
    def _getSupportedCRSProperty(self):
        # gets supported crs info
        crss=[]
        for elem in self._service.getDescribeCoverage(self.id).findall(ns('CoverageOffering/')+ns('supportedCRSs/')+ns('responseCRSs')):
            for crs in elem.text.split(' '):
                crss.append(Crs(crs))
        for elem in self._service.getDescribeCoverage(self.id).findall(ns('CoverageOffering/')+ns('supportedCRSs/')+ns('requestResponseCRSs')):
            for crs in elem.text.split(' '):
                crss.append(Crs(crs))
        for elem in self._service.getDescribeCoverage(self.id).findall(ns('CoverageOffering/')+ns('supportedCRSs/')+ns('nativeCRSs')):
            for crs in elem.text.split(' '):
                crss.append(Crs(crs))
        return crss
    supportedCRS=property(_getSupportedCRSProperty, None)
       
       
    def _getSupportedFormatsProperty(self):
        # gets supported formats info
        frmts =[]
        for elem in self._service.getDescribeCoverage(self.id).findall(ns('CoverageOffering/')+ns('supportedFormats/')+ns('formats')):
            frmts.append(elem.text)
        return frmts
    supportedFormats=property(_getSupportedFormatsProperty, None)
    
    def _getAxisDescriptionsProperty(self):
        #gets any axis descriptions contained in the rangeset (requires a DescribeCoverage call to server).
        axisDescs =[]
        for elem in self._service.getDescribeCoverage(self.id).findall(ns('CoverageOffering/')+ns('rangeSet/')+ns('RangeSet/')+ns('axisDescription/')+ns('AxisDescription')):
            axisDescs.append(AxisDescription(elem)) #create a 'AxisDescription' object.
        return axisDescs
    axisDescriptions=property(_getAxisDescriptionsProperty, None)
        
        
          
#Adding classes to represent gml:grid and gml:rectifiedgrid. One of these is used for the cvg.grid property
#(where cvg is a member of the contents dictionary)     
#There is no simple way to convert the offset values in a rectifiedgrid grid to real values without CRS understanding, therefore this is beyond the current scope of owslib, so the representation here is purely to provide access to the information in the GML.
   
class Grid(object):
    ''' Simple grid class to provide axis and value information for a gml grid '''
    def __init__(self, grid):
        self.axislabels = []
        self.dimension=None
        self.lowlimits=[]
        self.highlimits=[]
        if grid is not None:
            self.dimension=int(grid.get('dimension'))
            self.lowlimits= grid.find('{http://www.opengis.net/gml}limits/{http://www.opengis.net/gml}GridEnvelope/{http://www.opengis.net/gml}low').text.split(' ')
            self.highlimits = grid.find('{http://www.opengis.net/gml}limits/{http://www.opengis.net/gml}GridEnvelope/{http://www.opengis.net/gml}high').text.split(' ')
            for axis in grid.findall('{http://www.opengis.net/gml}axisName'):
                self.axislabels.append(axis.text)
      

class RectifiedGrid(Grid):
    ''' RectifiedGrid class, extends Grid with additional offset vector information '''
    def __init__(self, rectifiedgrid):
        super(RectifiedGrid,self).__init__(rectifiedgrid)
        self.origin=rectifiedgrid.find('{http://www.opengis.net/gml}origin/{http://www.opengis.net/gml}pos').text.split()
        self.offsetvectors=[]
        for offset in rectifiedgrid.findall('{http://www.opengis.net/gml}offsetVector'):
            self.offsetvectors.append(offset.text.split())
        
class AxisDescription(object):
    ''' Class to represent the AxisDescription element optionally found as part of the RangeSet and used to 
    define ordinates of additional dimensions such as wavelength bands or pressure levels'''
    def __init__(self, axisdescElem):
        self.name=self.label=None
        self.values=[]
        for elem in axisdescElem.getchildren():
            if elem.tag == ns('name'):
                self.name = elem.text
            elif elem.tag == ns('label'):
                self.label = elem.text
            elif elem.tag == ns('values'):
                for child in elem.getchildren():
                    self.values.append(child.text)     
