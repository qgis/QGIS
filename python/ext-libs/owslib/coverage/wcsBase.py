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

from __future__ import (absolute_import, division, print_function)

try:
    from urllib import urlencode
except ImportError:
    from urllib.parse import urlencode
from owslib.etree import etree
import cgi
from six.moves import cStringIO as StringIO
import six
from owslib.util import openURL


class ServiceException(Exception):
    """WCS ServiceException

    Attributes:
        message -- short error message
        xml  -- full xml error message from server
    """

    def __init__(self, message, xml):
        self.message = message
        self.xml = xml

    def __str__(self):
        return repr(self.message)

class WCSBase(object):
    """Base class to be subclassed by version dependent WCS classes. Provides 'high-level' version independent methods"""
    def __new__(self,url, xml, cookies):
        """ overridden __new__ method 
        
        @type url: string
        @param url: url of WCS capabilities document
        @type xml: string
        @param xml: elementtree object
        @return: inititalised WCSBase object
        """
        obj=object.__new__(self)
        obj.__init__(url, xml, cookies)
        self.cookies=cookies
        self._describeCoverage = {} #cache for DescribeCoverage responses
        return obj
    
    def __init__(self):
        pass    

    def getDescribeCoverage(self, identifier):
        ''' returns a describe coverage document - checks the internal cache to see if it has been fetched before '''
        if identifier not in self._describeCoverage.keys():
            reader = DescribeCoverageReader(self.version, identifier, self.cookies)
            self._describeCoverage[identifier] = reader.read(self.url)
        return self._describeCoverage[identifier]
        
        
class WCSCapabilitiesReader(object):
    """Read and parses WCS capabilities document into a lxml.etree infoset
    """

    def __init__(self, version=None, cookies = None):
        """Initialize
        @type version: string
        @param version: WCS Version parameter e.g '1.0.0'
        """
        self.version = version
        self._infoset = None
        self.cookies = cookies

    def capabilities_url(self, service_url):
        """Return a capabilities url
        @type service_url: string
        @param service_url: base url of WCS service
        @rtype: string
        @return: getCapabilities URL
        """
        qs = []
        if service_url.find('?') != -1:
            qs = cgi.parse_qsl(service_url.split('?')[1])

        params = [x[0] for x in qs]

        if 'service' not in params:
            qs.append(('service', 'WCS'))
        if 'request' not in params:
            qs.append(('request', 'GetCapabilities'))
        if ('version' not in params) and (self.version is not None):
            qs.append(('version', self.version))

        urlqs = urlencode(tuple(qs))
        return service_url.split('?')[0] + '?' + urlqs

    def read(self, service_url, timeout=30):
        """Get and parse a WCS capabilities document, returning an
        elementtree tree

        @type service_url: string
        @param service_url: The base url, to which is appended the service,
        version, and request parameters
        @rtype: elementtree tree
        @return: An elementtree tree representation of the capabilities document
        """
        request = self.capabilities_url(service_url)
        u = openURL(request, timeout=timeout, cookies=self.cookies)
        return etree.fromstring(u.read())

    def readString(self, st):
        """Parse a WCS capabilities document, returning an
        instance of WCSCapabilitiesInfoset
        string should be an XML capabilities document
        """
        return etree.fromstring(st)

class DescribeCoverageReader(object):
    """Read and parses WCS DescribeCoverage document into a lxml.etree infoset
    """

    def __init__(self, version, identifier, cookies):
        """Initialize
        @type version: string
        @param version: WCS Version parameter e.g '1.0.0'
        """
        self.version = version
        self._infoset = None
        self.identifier=identifier
        self.cookies = cookies

    def descCov_url(self, service_url):
        """Return a describe coverage url
        @type service_url: string
        @param service_url: base url of WCS service
        @rtype: string
        @return: getCapabilities URL
        """
        qs = []
        if service_url.find('?') != -1:
            qs = cgi.parse_qsl(service_url.split('?')[1])

        params = [x[0] for x in qs]

        if 'service' not in params:
            qs.append(('service', 'WCS'))
        if 'request' not in params:
            qs.append(('request', 'DescribeCoverage'))
        if 'version' not in params:
            qs.append(('version', self.version))
        if self.version == '1.0.0':
            if 'coverage' not in params:
                qs.append(('coverage', self.identifier))
        elif self.version == '1.1.0' or self.version == '1.1.1':
            #NOTE: WCS 1.1.0 is ambigous about whether it should be identifier
            #or identifiers (see tables 9, 10 of specification)  
            if 'identifiers' not in params:
                qs.append(('identifiers', self.identifier))
            if 'identifier' not in params:
                qs.append(('identifier', self.identifier))
                qs.append(('format', 'text/xml'))
        urlqs = urlencode(tuple(qs))
        return service_url.split('?')[0] + '?' + urlqs

    def read(self, service_url, timeout=30):
        """Get and parse a Describe Coverage document, returning an
        elementtree tree

        @type service_url: string
        @param service_url: The base url, to which is appended the service,
        version, and request parameters
        @rtype: elementtree tree
        @return: An elementtree tree representation of the capabilities document
        """
        request = self.descCov_url(service_url)
        u = openURL(request, cookies=self.cookies, timeout=timeout)
        return etree.fromstring(u.read())

