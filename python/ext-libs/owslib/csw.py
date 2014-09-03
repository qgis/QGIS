# -*- coding: ISO-8859-15 -*-
# =============================================================================
# Copyright (c) 2009 Tom Kralidis
#
# Authors : Tom Kralidis <tomkralidis@gmail.com>
#
# Contact email: tomkralidis@gmail.com
# =============================================================================

""" CSW request and response processor """

import warnings
import StringIO
import random
from urllib import urlencode
from urllib2 import urlopen

from owslib.util import OrderedDict

from owslib.etree import etree
from owslib import fes
from owslib import util
from owslib import ows
from owslib.iso import MD_Metadata
from owslib.fgdc import Metadata
from owslib.dif import DIF
from owslib.namespaces import Namespaces
from owslib.util import cleanup_namespaces, bind_url, add_namespaces

# default variables
outputformat = 'application/xml'

def get_namespaces():
    n = Namespaces()
    return n.get_namespaces()
namespaces = get_namespaces()
schema = 'http://schemas.opengis.net/csw/2.0.2/CSW-discovery.xsd'
schema_location = '%s %s' % (namespaces['csw'], schema)

class CatalogueServiceWeb:
    """ csw request class """
    def __init__(self, url, lang='en-US', version='2.0.2', timeout=10, skip_caps=False):
        """

        Construct and process a GetCapabilities request

        Parameters
        ----------

        - url: the URL of the CSW
        - lang: the language (default is 'en-US')
        - version: version (default is '2.0.2')
        - timeout: timeout in seconds
        - skip_caps: whether to skip GetCapabilities processing on init (default is False)

        """

        self.url = url
        self.lang = lang
        self.version = version
        self.timeout = timeout
        self.service = 'CSW'
        self.exceptionreport = None
        self.owscommon = ows.OwsCommon('1.0.0')

        if not skip_caps:  # process GetCapabilities
            # construct request

            data = {'service': self.service, 'version': self.version, 'request': 'GetCapabilities'}

            self.request = '%s%s' % (bind_url(self.url), urlencode(data))
    
            self._invoke()
    
            if self.exceptionreport is None:
                # ServiceIdentification
                val = self._exml.find(util.nspath_eval('ows:ServiceIdentification', namespaces))
                self.identification=ows.ServiceIdentification(val,self.owscommon.namespace)
                # ServiceProvider
                val = self._exml.find(util.nspath_eval('ows:ServiceProvider', namespaces))
                self.provider=ows.ServiceProvider(val,self.owscommon.namespace)
                # ServiceOperations metadata 
                self.operations=[]
                for elem in self._exml.findall(util.nspath_eval('ows:OperationsMetadata/ows:Operation', namespaces)):
                    self.operations.append(ows.OperationsMetadata(elem, self.owscommon.namespace))
        
                # FilterCapabilities
                val = self._exml.find(util.nspath_eval('ogc:Filter_Capabilities', namespaces))
                self.filters=fes.FilterCapabilities(val)
 
    def describerecord(self, typename='csw:Record', format=outputformat):
        """

        Construct and process DescribeRecord request

        Parameters
        ----------

        - typename: the typename to describe (default is 'csw:Record')
        - format: the outputFormat (default is 'application/xml')
 
        """

        # construct request
        node0 = self._setrootelement('csw:DescribeRecord')
        node0.set('service', self.service)
        node0.set('version', self.version)
        node0.set('outputFormat', format)
        node0.set('schemaLanguage', namespaces['xs2'])
        node0.set(util.nspath_eval('xsi:schemaLocation', namespaces), schema_location)
        etree.SubElement(node0, util.nspath_eval('csw:TypeName', namespaces)).text = typename

        self.request = node0

        self._invoke()

        # parse result
        # TODO: process the XML Schema (you're on your own for now with self.response)

    def getdomain(self, dname, dtype='parameter'):
        """

        Construct and process a GetDomain request

        Parameters
        ----------

        - dname: the value of the Parameter or Property to query
        - dtype: whether to query a parameter (parameter) or property (property)

        """

        # construct request
        dtypename = 'ParameterName'
        node0 = self._setrootelement('csw:GetDomain')
        node0.set('service', self.service)
        node0.set('version', self.version)
        node0.set(util.nspath_eval('xsi:schemaLocation', namespaces), schema_location)
        if dtype == 'property':
            dtypename = 'PropertyName'
        etree.SubElement(node0, util.nspath_eval('csw:%s' % dtypename, namespaces)).text = dname

        self.request = node0

        self._invoke()

        if self.exceptionreport is None:
            self.results = {}

            val = self._exml.find(util.nspath_eval('csw:DomainValues', namespaces)).attrib.get('type')
            self.results['type'] = util.testXMLValue(val, True)

            val = self._exml.find(util.nspath_eval('csw:DomainValues/csw:%s' % dtypename, namespaces))
            self.results[dtype] = util.testXMLValue(val)

            # get the list of values associated with the Domain
            self.results['values'] = []

            for f in self._exml.findall(util.nspath_eval('csw:DomainValues/csw:ListOfValues/csw:Value', namespaces)):
                self.results['values'].append(util.testXMLValue(f))

    def getrecords(self, qtype=None, keywords=[], typenames='csw:Record', propertyname='csw:AnyText', bbox=None, esn='summary', sortby=None, outputschema=namespaces['csw'], format=outputformat, startposition=0, maxrecords=10, cql=None, xml=None, resulttype='results'):
        """

        Construct and process a  GetRecords request

        Parameters
        ----------

        - qtype: type of resource to query (i.e. service, dataset)
        - keywords: list of keywords
        - typenames: the typeNames to query against (default is csw:Record)
        - propertyname: the PropertyName to Filter against 
        - bbox: the bounding box of the spatial query in the form [minx,miny,maxx,maxy]
        - esn: the ElementSetName 'full', 'brief' or 'summary' (default is 'summary')
        - sortby: property to sort results on
        - outputschema: the outputSchema (default is 'http://www.opengis.net/cat/csw/2.0.2')
        - format: the outputFormat (default is 'application/xml')
        - startposition: requests a slice of the result set, starting at this position (default is 0)
        - maxrecords: the maximum number of records to return. No records are returned if 0 (default is 10)
        - cql: common query language text.  Note this overrides bbox, qtype, keywords
        - xml: raw XML request.  Note this overrides all other options
        - resulttype: the resultType 'hits', 'results', 'validate' (default is 'results')

        """

        warnings.warn("""Please use the updated 'getrecords2' method instead of 'getrecords'.  
        The 'getrecords' method will be upgraded to use the 'getrecords2' parameters
        in a future version of OWSLib.""")

        if xml is not None:
            self.request = etree.fromstring(xml)
            val = self.request.find(util.nspath_eval('csw:Query/csw:ElementSetName', namespaces))
            if val is not None:
                esn = util.testXMLValue(val)
        else:
            # construct request
            node0 = self._setrootelement('csw:GetRecords')
            if etree.__name__ != 'lxml.etree':  # apply nsmap manually
                node0.set('xmlns:ows', namespaces['ows'])
                node0.set('xmlns:gmd', namespaces['gmd'])
                node0.set('xmlns:dif', namespaces['dif'])
                node0.set('xmlns:fgdc', namespaces['fgdc'])
            node0.set('outputSchema', outputschema)
            node0.set('outputFormat', format)
            node0.set('version', self.version)
            node0.set('resultType', resulttype)
            node0.set('service', self.service)
            if startposition > 0:
                node0.set('startPosition', str(startposition))
            node0.set('maxRecords', str(maxrecords))
            node0.set(util.nspath_eval('xsi:schemaLocation', namespaces), schema_location)
    
            node1 = etree.SubElement(node0, util.nspath_eval('csw:Query', namespaces))
            node1.set('typeNames', typenames)
        
            etree.SubElement(node1, util.nspath_eval('csw:ElementSetName', namespaces)).text = esn
    
            self._setconstraint(node1, qtype, propertyname, keywords, bbox, cql, None)
    
            if sortby is not None:
                fes.setsortby(node1, sortby)
    
            self.request = node0

        self._invoke()
 
        if self.exceptionreport is None:
            self.results = {}
    
            # process search results attributes
            val = self._exml.find(util.nspath_eval('csw:SearchResults', namespaces)).attrib.get('numberOfRecordsMatched')
            self.results['matches'] = int(util.testXMLValue(val, True))
            val = self._exml.find(util.nspath_eval('csw:SearchResults', namespaces)).attrib.get('numberOfRecordsReturned')
            self.results['returned'] = int(util.testXMLValue(val, True))
            val = self._exml.find(util.nspath_eval('csw:SearchResults', namespaces)).attrib.get('nextRecord')
            self.results['nextrecord'] = int(util.testXMLValue(val, True))
    
            # process list of matching records
            self.records = OrderedDict()

            self._parserecords(outputschema, esn)

    def getrecordbyid(self, id=[], esn='full', outputschema=namespaces['csw'], format=outputformat):
        """

        Construct and process a GetRecordById request

        Parameters
        ----------

        - id: the list of Ids
        - esn: the ElementSetName 'full', 'brief' or 'summary' (default is 'full')
        - outputschema: the outputSchema (default is 'http://www.opengis.net/cat/csw/2.0.2')
        - format: the outputFormat (default is 'application/xml')

        """

        # construct request
        data = {
            'service': self.service,
            'version': self.version,
            'request': 'GetRecordById',
            'outputFormat': format,
            'outputSchema': outputschema,
            'elementsetname': esn,
            'id': ','.join(id),
        }

        self.request = '%s%s' % (bind_url(self.url), urlencode(data))

        self._invoke()

        if self.exceptionreport is None:
            self.results = {}
            self.records = OrderedDict()
            self._parserecords(outputschema, esn)

    def getrecords2(self, constraints=[], sortby=None, typenames='csw:Record', esn='summary', outputschema=namespaces['csw'], format=outputformat, startposition=0, maxrecords=10, cql=None, xml=None, resulttype='results'):
        """

        Construct and process a  GetRecords request

        Parameters
        ----------

        - constraints: the list of constraints (OgcExpression from owslib.fes module)
        - sortby: an OGC SortBy object (SortBy from owslib.fes module)
        - typenames: the typeNames to query against (default is csw:Record)
        - esn: the ElementSetName 'full', 'brief' or 'summary' (default is 'summary')        
        - outputschema: the outputSchema (default is 'http://www.opengis.net/cat/csw/2.0.2')
        - format: the outputFormat (default is 'application/xml')
        - startposition: requests a slice of the result set, starting at this position (default is 0)
        - maxrecords: the maximum number of records to return. No records are returned if 0 (default is 10)
        - cql: common query language text.  Note this overrides bbox, qtype, keywords
        - xml: raw XML request.  Note this overrides all other options
        - resulttype: the resultType 'hits', 'results', 'validate' (default is 'results')

        """

        if xml is not None:
            self.request = etree.fromstring(xml)
            val = self.request.find(util.nspath_eval('csw:Query/csw:ElementSetName', namespaces))
            if val is not None:
                esn = util.testXMLValue(val)
        else:
            # construct request
            node0 = self._setrootelement('csw:GetRecords')
            if etree.__name__ != 'lxml.etree':  # apply nsmap manually
                node0.set('xmlns:ows', namespaces['ows'])
                node0.set('xmlns:gmd', namespaces['gmd'])
                node0.set('xmlns:dif', namespaces['dif'])
                node0.set('xmlns:fgdc', namespaces['fgdc'])
            node0.set('outputSchema', outputschema)
            node0.set('outputFormat', format)
            node0.set('version', self.version)
            node0.set('service', self.service)
            node0.set('resultType', resulttype)
            if startposition > 0:
                node0.set('startPosition', str(startposition))
            node0.set('maxRecords', str(maxrecords))        
            node0.set(util.nspath_eval('xsi:schemaLocation', namespaces), schema_location)

            node1 = etree.SubElement(node0, util.nspath_eval('csw:Query', namespaces))
            node1.set('typeNames', typenames)
        
            etree.SubElement(node1, util.nspath_eval('csw:ElementSetName', namespaces)).text = esn

            if any([len(constraints) > 0, cql is not None]): 
                node2 = etree.SubElement(node1, util.nspath_eval('csw:Constraint', namespaces))
                node2.set('version', '1.1.0')
                flt = fes.FilterRequest()
                if len(constraints) > 0:
                    node2.append(flt.setConstraintList(constraints))
                # Now add a CQL filter if passed in
                elif cql is not None:
                    etree.SubElement(node2, util.nspath_eval('csw:CqlText', namespaces)).text = cql
                
            if sortby is not None and isinstance(sortby, fes.SortBy):
                node1.append(sortby.toXML())

            self.request = node0

        self._invoke()
 
        if self.exceptionreport is None:
            self.results = {}
    
            # process search results attributes
            val = self._exml.find(util.nspath_eval('csw:SearchResults', namespaces)).attrib.get('numberOfRecordsMatched')
            self.results['matches'] = int(util.testXMLValue(val, True))
            val = self._exml.find(util.nspath_eval('csw:SearchResults', namespaces)).attrib.get('numberOfRecordsReturned')
            self.results['returned'] = int(util.testXMLValue(val, True))
            val = self._exml.find(util.nspath_eval('csw:SearchResults', namespaces)).attrib.get('nextRecord')
            if val is not None:
                 self.results['nextrecord'] = int(util.testXMLValue(val, True))
            else:
                warnings.warn("""CSW Server did not supply a nextRecord value (it is optional), so the client
                should page through the results in another way.""")
                # For more info, see:
                # https://github.com/geopython/OWSLib/issues/100
                self.results['nextrecord'] = None

            # process list of matching records
            self.records = OrderedDict()

            self._parserecords(outputschema, esn)

    def transaction(self, ttype=None, typename='csw:Record', record=None, propertyname=None, propertyvalue=None, bbox=None, keywords=[], cql=None, identifier=None):
        """

        Construct and process a Transaction request

        Parameters
        ----------

        - ttype: the type of transaction 'insert, 'update', 'delete'
        - typename: the typename to describe (default is 'csw:Record')
        - record: the XML record to insert
        - propertyname: the RecordProperty/PropertyName to Filter against
        - propertyvalue: the RecordProperty Value to Filter against (for updates)
        - bbox: the bounding box of the spatial query in the form [minx,miny,maxx,maxy]
        - keywords: list of keywords
        - cql: common query language text.  Note this overrides bbox, qtype, keywords
        - identifier: record identifier.  Note this overrides bbox, qtype, keywords, cql

        """

        # construct request
        node0 = self._setrootelement('csw:Transaction')
        node0.set('version', self.version)
        node0.set('service', self.service)
        node0.set(util.nspath_eval('xsi:schemaLocation', namespaces), schema_location)

        validtransactions = ['insert', 'update', 'delete']

        if ttype not in validtransactions:  # invalid transaction
            raise RuntimeError, 'Invalid transaction \'%s\'.' % ttype

        node1 = etree.SubElement(node0, util.nspath_eval('csw:%s' % ttype.capitalize(), namespaces))

        if ttype != 'update':  
            node1.set('typeName', typename)

        if ttype == 'insert':
            if record is None:
                raise RuntimeError, 'Nothing to insert.'
            node1.append(etree.fromstring(record))
 
        if ttype == 'update':
            if record is not None:
                node1.append(etree.fromstring(record))
            else:
                if propertyname is not None and propertyvalue is not None:
                    node2 = etree.SubElement(node1, util.nspath_eval('csw:RecordProperty', namespaces))
                    etree.SubElement(node2, util.nspath_eval('csw:Name', namespaces)).text = propertyname
                    etree.SubElement(node2, util.nspath_eval('csw:Value', namespaces)).text = propertyvalue
                    self._setconstraint(node1, qtype, propertyname, keywords, bbox, cql, identifier)

        if ttype == 'delete':
            self._setconstraint(node1, None, propertyname, keywords, bbox, cql, identifier)

        self.request = node0

        self._invoke()
        self.results = {}

        if self.exceptionreport is None:
            self._parsetransactionsummary()
            self._parseinsertresult()

    def harvest(self, source, resourcetype, resourceformat=None, harvestinterval=None, responsehandler=None):
        """

        Construct and process a Harvest request

        Parameters
        ----------

        - source: a URI to harvest
        - resourcetype: namespace identifying the type of resource
        - resourceformat: MIME type of the resource
        - harvestinterval: frequency of harvesting, in ISO8601
        - responsehandler: endpoint that CSW should responsd to with response

        """

        # construct request
        node0 = self._setrootelement('csw:Harvest')
        node0.set('version', self.version)
        node0.set('service', self.service)
        node0.set(util.nspath_eval('xsi:schemaLocation', namespaces), schema_location)
        etree.SubElement(node0, util.nspath_eval('csw:Source', namespaces)).text = source
        etree.SubElement(node0, util.nspath_eval('csw:ResourceType', namespaces)).text = resourcetype
        if resourceformat is not None:
            etree.SubElement(node0, util.nspath_eval('csw:ResourceFormat', namespaces)).text = resourceformat
        if harvestinterval is not None:
            etree.SubElement(node0, util.nspath_eval('csw:HarvestInterval', namespaces)).text = harvestinterval
        if responsehandler is not None:
            etree.SubElement(node0, util.nspath_eval('csw:ResponseHandler', namespaces)).text = responsehandler
       
        self.request = node0

        self._invoke()
        self.results = {}

        if self.exceptionreport is None:
            val = self._exml.find(util.nspath_eval('csw:Acknowledgement', namespaces))
            if util.testXMLValue(val) is not None:
                ts = val.attrib.get('timeStamp')
                self.timestamp = util.testXMLValue(ts, True)
                id = val.find(util.nspath_eval('csw:RequestId', namespaces))
                self.id = util.testXMLValue(id) 
            else:
                self._parsetransactionsummary()
                self._parseinsertresult()

    def get_operation_by_name(self, name):
        """Return a named operation"""
        for item in self.operations:
            if item.name.lower() == name.lower():
                return item
        raise KeyError, "No operation named %s" % name

    def getService_urls(self, service_string=None):
        """

        Return easily identifiable URLs for all service types

        Parameters
        ----------

        - service_string: a URI to lookup

        """
        
        urls=[]
        for key,rec in self.records.iteritems():
            #create a generator object, and iterate through it until the match is found
            #if not found, gets the default value (here "none")
            url = next((d['url'] for d in rec.references if d['scheme'] == service_string), None)
            if url is not None:
                urls.append(url)
        return urls

    def _parseinsertresult(self):
        self.results['insertresults'] = []
        for i in self._exml.findall(util.nspath_eval('csw:InsertResult', namespaces)):
            for j in i.findall(util.nspath_eval('csw:BriefRecord/dc:identifier', namespaces)):
                self.results['insertresults'].append(util.testXMLValue(j))

    def _parserecords(self, outputschema, esn):
        if outputschema == namespaces['gmd']: # iso 19139
            for i in self._exml.findall('.//'+util.nspath_eval('gmd:MD_Metadata', namespaces)):
                val = i.find(util.nspath_eval('gmd:fileIdentifier/gco:CharacterString', namespaces))
                identifier = self._setidentifierkey(util.testXMLValue(val))
                self.records[identifier] = MD_Metadata(i)
        elif outputschema == namespaces['fgdc']: # fgdc csdgm
            for i in self._exml.findall('.//metadata'):
                val = i.find('idinfo/datasetid')
                identifier = self._setidentifierkey(util.testXMLValue(val))
                self.records[identifier] = Metadata(i)
        elif outputschema == namespaces['dif']: # nasa dif
            for i in self._exml.findall('.//'+util.nspath_eval('dif:DIF', namespaces)):
                val = i.find(util.nspath_eval('dif:Entry_ID', namespaces))
                identifier = self._setidentifierkey(util.testXMLValue(val))
                self.records[identifier] = DIF(i)
        else: # process default
            for i in self._exml.findall('.//'+util.nspath_eval('csw:%s' % self._setesnel(esn), namespaces)):
                val = i.find(util.nspath_eval('dc:identifier', namespaces))
                identifier = self._setidentifierkey(util.testXMLValue(val))
                self.records[identifier] = CswRecord(i)

    def _parsetransactionsummary(self):
        val = self._exml.find(util.nspath_eval('csw:TransactionSummary', namespaces))
        if val is not None:
            rid = val.attrib.get('requestId')
            self.results['requestid'] = util.testXMLValue(rid, True)
            ts = val.find(util.nspath_eval('csw:totalInserted', namespaces))
            self.results['inserted'] = int(util.testXMLValue(ts))
            ts = val.find(util.nspath_eval('csw:totalUpdated', namespaces))
            self.results['updated'] = int(util.testXMLValue(ts))
            ts = val.find(util.nspath_eval('csw:totalDeleted', namespaces))
            self.results['deleted'] = int(util.testXMLValue(ts))

    def _setesnel(self, esn):
        """ Set the element name to parse depending on the ElementSetName requested """
        el = 'Record'
        if esn == 'brief':
            el = 'BriefRecord'
        if esn == 'summary':
            el = 'SummaryRecord'
        return el

    def _setidentifierkey(self, el):
        if el is None: 
            return 'owslib_random_%i' % random.randint(1,65536)
        else:
            return el

    def _setrootelement(self, el):
        if etree.__name__ == 'lxml.etree':  # apply nsmap
            return etree.Element(util.nspath_eval(el, namespaces), nsmap=namespaces)
        else:
            return etree.Element(util.nspath_eval(el, namespaces))

    def _setconstraint(self, parent, qtype=None, propertyname='csw:AnyText', keywords=[], bbox=None, cql=None, identifier=None):
        if keywords or bbox is not None or qtype is not None or cql is not None or identifier is not None:
            node0 = etree.SubElement(parent, util.nspath_eval('csw:Constraint', namespaces))
            node0.set('version', '1.1.0')

            if identifier is not None:  # set identifier filter, overrides all other parameters
                flt = fes.FilterRequest()
                node0.append(flt.set(identifier=identifier))
            elif cql is not None:  # send raw CQL query
                # CQL passed, overrides all other parameters
                node1 = etree.SubElement(node0, util.nspath_eval('csw:CqlText', namespaces))
                node1.text = cql
            else:  # construct a Filter request
                flt = fes.FilterRequest()
                node0.append(flt.set(qtype=qtype, keywords=keywords, propertyname=propertyname,bbox=bbox))
    
    def _invoke(self):
        # do HTTP request

        if isinstance(self.request, basestring):  # GET KVP
            self.response = urlopen(self.request, timeout=self.timeout).read()
        else:
            xml_post_url = self.url
            # Get correct POST URL based on Operation list.
            # If skip_caps=True, then self.operations has not been set, so use
            # default URL.
            if hasattr(self, 'operations'):
                for op in self.operations:
                    post_verbs = filter(lambda x: x.get('type').lower() == 'post', op.methods)
                    if len(post_verbs) > 1:
                        # Filter by constraints.  We must match a PostEncoding of "XML"
                        try:
                            xml_post_url = next(x for x in filter(list, ([pv.get('url') for const in pv.get('constraints') if const.name.lower() == "postencoding" and 'xml' in map(lambda x: x.lower(), const.values)] for pv in post_verbs)))[0]
                        except StopIteration:
                            # Well, just use the first one.
                            xml_post_url = post_verbs[0].get('url')
                    elif len(post_verbs) == 1:
                        xml_post_url = post_verbs[0].get('url')

            self.request = cleanup_namespaces(self.request)
            # Add any namespaces used in the "typeNames" attribute of the
            # csw:Query element to the query's xml namespaces.
            for query in self.request.findall(util.nspath_eval('csw:Query', namespaces)):
                ns = query.get("typeNames", None)
                if ns is not None:
                    # Pull out "gmd" from something like "gmd:MD_Metadata" from the list
                    # of typenames
                    ns_keys = [x.split(':')[0] for x in ns.split(' ')]
                    self.request = add_namespaces(self.request, ns_keys)

            self.request = util.element_to_string(self.request, encoding='utf-8')

            self.response = util.http_post(xml_post_url, self.request, self.lang, self.timeout)

        # parse result see if it's XML
        self._exml = etree.parse(StringIO.StringIO(self.response))

        # it's XML.  Attempt to decipher whether the XML response is CSW-ish """
        valid_xpaths = [
            util.nspath_eval('ows:ExceptionReport', namespaces),
            util.nspath_eval('csw:Capabilities', namespaces),
            util.nspath_eval('csw:DescribeRecordResponse', namespaces),
            util.nspath_eval('csw:GetDomainResponse', namespaces),
            util.nspath_eval('csw:GetRecordsResponse', namespaces),
            util.nspath_eval('csw:GetRecordByIdResponse', namespaces),
            util.nspath_eval('csw:HarvestResponse', namespaces),
            util.nspath_eval('csw:TransactionResponse', namespaces)
        ]

        if self._exml.getroot().tag not in valid_xpaths:
            raise RuntimeError, 'Document is XML, but not CSW-ish'

        # check if it's an OGC Exception
        val = self._exml.find(util.nspath_eval('ows:Exception', namespaces))
        if val is not None:
            raise ows.ExceptionReport(self._exml, self.owscommon.namespace)
        else:
            self.exceptionreport = None

class CswRecord(object):
    """ Process csw:Record, csw:BriefRecord, csw:SummaryRecord """
    def __init__(self, record):

        if hasattr(record, 'getroot'):  # standalone document
            self.xml = etree.tostring(record.getroot())
        else:  # part of a larger document
            self.xml = etree.tostring(record)

        # check to see if Dublin Core record comes from
        # rdf:RDF/rdf:Description container
        # (child content model is identical)
        self.rdf = False
        rdf = record.find(util.nspath_eval('rdf:Description', namespaces))
        if rdf is not None:
            self.rdf = True
            record = rdf

        # some CSWs return records with multiple identifiers based on 
        # different schemes.  Use the first dc:identifier value to set
        # self.identifier, and set self.identifiers as a list of dicts
        val = record.find(util.nspath_eval('dc:identifier', namespaces))
        self.identifier = util.testXMLValue(val)

        self.identifiers = []
        for i in record.findall(util.nspath_eval('dc:identifier', namespaces)):
            d = {}
            d['scheme'] = i.attrib.get('scheme')
            d['identifier'] = i.text
            self.identifiers.append(d)

        val = record.find(util.nspath_eval('dc:type', namespaces))
        self.type = util.testXMLValue(val)

        val = record.find(util.nspath_eval('dc:title', namespaces))
        self.title = util.testXMLValue(val)

        val = record.find(util.nspath_eval('dct:alternative', namespaces))
        self.alternative = util.testXMLValue(val)

        val = record.find(util.nspath_eval('dct:isPartOf', namespaces))
        self.ispartof = util.testXMLValue(val)

        val = record.find(util.nspath_eval('dct:abstract', namespaces))
        self.abstract = util.testXMLValue(val)

        val = record.find(util.nspath_eval('dc:date', namespaces))
        self.date = util.testXMLValue(val)

        val = record.find(util.nspath_eval('dct:created', namespaces))
        self.created = util.testXMLValue(val)

        val = record.find(util.nspath_eval('dct:issued', namespaces))
        self.issued = util.testXMLValue(val)

        val = record.find(util.nspath_eval('dc:relation', namespaces))
        self.relation = util.testXMLValue(val)

        val = record.find(util.nspath_eval('dct:temporal', namespaces))
        self.temporal = util.testXMLValue(val)

        self.uris = []  # list of dicts
        for i in record.findall(util.nspath_eval('dc:URI', namespaces)):
            uri = {}
            uri['protocol'] = util.testXMLValue(i.attrib.get('protocol'), True)
            uri['name'] = util.testXMLValue(i.attrib.get('name'), True)
            uri['description'] = util.testXMLValue(i.attrib.get('description'), True)
            uri['url'] = util.testXMLValue(i)

            self.uris.append(uri)

        self.references = []  # list of dicts
        for i in record.findall(util.nspath_eval('dct:references', namespaces)):
            ref = {}
            ref['scheme'] = util.testXMLValue(i.attrib.get('scheme'), True)
            ref['url'] = util.testXMLValue(i)

            self.references.append(ref)

        val = record.find(util.nspath_eval('dct:modified', namespaces))
        self.modified = util.testXMLValue(val)

        val = record.find(util.nspath_eval('dc:creator', namespaces))
        self.creator = util.testXMLValue(val)

        val = record.find(util.nspath_eval('dc:publisher', namespaces))
        self.publisher = util.testXMLValue(val)

        val = record.find(util.nspath_eval('dc:coverage', namespaces))
        self.coverage = util.testXMLValue(val)

        val = record.find(util.nspath_eval('dc:contributor', namespaces))
        self.contributor = util.testXMLValue(val)

        val = record.find(util.nspath_eval('dc:language', namespaces))
        self.language = util.testXMLValue(val)

        val = record.find(util.nspath_eval('dc:source', namespaces))
        self.source = util.testXMLValue(val)

        val = record.find(util.nspath_eval('dct:rightsHolder', namespaces))
        self.rightsholder = util.testXMLValue(val)

        val = record.find(util.nspath_eval('dct:accessRights', namespaces))
        self.accessrights = util.testXMLValue(val)

        val = record.find(util.nspath_eval('dct:license', namespaces))
        self.license = util.testXMLValue(val)

        val = record.find(util.nspath_eval('dc:format', namespaces))
        self.format = util.testXMLValue(val)

        self.subjects = []
        for i in record.findall(util.nspath_eval('dc:subject', namespaces)):
            self.subjects.append(util.testXMLValue(i))

        self.rights = []
        for i in record.findall(util.nspath_eval('dc:rights', namespaces)):
            self.rights.append(util.testXMLValue(i))

        val = record.find(util.nspath_eval('dct:spatial', namespaces))
        self.spatial = util.testXMLValue(val)

        val = record.find(util.nspath_eval('ows:BoundingBox', namespaces))
        if val is not None:
            self.bbox = ows.BoundingBox(val, namespaces['ows'])
        else:
            self.bbox = None

        val = record.find(util.nspath_eval('ows:WGS84BoundingBox', namespaces))
        if val is not None:
            self.bbox_wgs84 = ows.WGS84BoundingBox(val, namespaces['ows'])
        else:
            self.bbox_wgs84 = None
