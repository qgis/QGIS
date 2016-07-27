#
#
# Author: Luca Cinquini
#
#

"""

Abstract
--------
The wps module of the OWSlib package provides client-side functionality for executing invocations to a remote Web Processing Server.


Disclaimer
----------
PLEASE NOTE: the owslib wps module should be considered in beta state: it has been tested versus only a handful of WPS services (deployed by the USGS, BADC and PML).
More extensive testing is needed and feedback is appreciated.


Usage
-----

The module can be used to execute three types of requests versus a remote WPS endpoint:

a) "GetCapabilities"
    - use the method wps.getcapabilities(xml=None)
    - the optional keyword argument "xml" may be used to avoid a real live request, and instead read the WPS capabilities document from a cached XML file

b) "DescribeProcess"
    - use the method wps.describeprocess(identifier, xml=None)
    - identifier is the process identifier, retrieved from the list obtained from a previous "GetCapabilities" invocation
    - the optional keyword argument "xml" may be used to avoid a real live request, and instead read the WPS process description document from a cached XML file

c) "Execute"
    - use the method wps.execute(identifier, inputs, output=None, request=None, response=None),
      which submits the job to the remote WPS server and returns a WPSExecution object that can be used to periodically check the job status until completion
      (or error)

    - the optional keyword argument "request" may be used to avoid re-building the request XML from input arguments, and instead submit a request from a
      pre-made XML file

    - alternatively, an "Execute" request can be built from input arguments by supplying the "identifier", "inputs" and "output" arguments to the execute() method.
        - "identifier" is the mandatory process identifier
        - "inputs" is a dictionary of (key,value) pairs where:
            - key is a named input parameter
            - value is either a string, or any python object that supports a getXml() method
              In particular, a few classes are included in the package to support a FeatuteCollection input:
                  - "WFSFeatureCollection" can be used in conjunction with "WFSQuery" to define a FEATURE_COLLECTION retrieved from a live WFS server.
                  - "GMLMultiPolygonFeatureCollection" can be used to define one or more polygons of (latitude, longitude) points.
          - "output" is an optional output identifier to be included in the ResponseForm section of the request.

    - the optional keyword argument "response" mey be used to avoid submitting a real live request, and instead reading the WPS execution response document
      from a cached XML file (for debugging or testing purposes)
    - the convenience module function monitorExecution() can be used to periodically check the status of a remote running job, and eventually download the output
      either to a named file, or to a file specified by the server.


Examples
--------

The files examples/wps-usgs-script.py, examples/wps-pml-script-1.py and examples/wps-pml-script-2.py contain real-world usage examples
that submits a "GetCapabilities", "DescribeProcess" and "Execute" requests to the live USGS and PML servers. To run:
    cd examples
    python wps-usgs-script.py
    python wps-pml-script-1.py
    python wps-pml-script-2.py

The file wps-client.py contains a command-line client that can be used to submit a "GetCapabilities", "DescribeProcess" or "Execute"
request to an arbitratry WPS server. For example, you can run it as follows:
    cd examples
    To prints out usage and example invocations: wps-client -help
    To execute a (fake) WPS invocation:
        wps-client.py -v -u http://cida.usgs.gov/climate/gdp/process/WebProcessingService -r GetCapabilities -x ../tests/USGSCapabilities.xml

The directory tests/ includes several doctest-style files wps_*.txt that show how to interactively submit a
"GetCapabilities", "DescribeProcess" or "Execute" request, without making a live request but rather parsing the response of cached XML response documents. To run:
    cd tests
    python -m doctest wps_*.txt
    (or python -m doctest -v wps_*.txt for verbose output)

Also, the directory tests/ contains several examples of well-formed "Execute" requests:
    - The files wps_USGSExecuteRequest*.xml contain requests that can be submitted to the live USGS WPS service.
    - The files PMLExecuteRequest*.xml contain requests that can be submitted to the live PML WPS service.

"""

from __future__ import (absolute_import, division, print_function)

from owslib.etree import etree
from owslib.ows import DEFAULT_OWS_NAMESPACE, ServiceIdentification, ServiceProvider, OperationsMetadata, BoundingBox
from time import sleep
from owslib.util import (testXMLValue, build_get_url, dump, getTypedValue,
                         getNamespace, element_to_string, nspath, openURL, nspath_eval, log)
from xml.dom.minidom import parseString
from owslib.namespaces import Namespaces
try:                    # Python 3
    from urllib.parse import urlparse
except ImportError:     # Python 2
    from urlparse import urlparse

# namespace definition
n = Namespaces()

# These static namespaces are DEPRECIATED.  Please don't use them.
# No great way of printing a message since there are at the file level
WPS_DEFAULT_NAMESPACE = n.get_namespace("wps")
WFS_NAMESPACE = n.get_namespace("wfs")
OGC_NAMESPACE = n.get_namespace("ogc")
GML_NAMESPACE = n.get_namespace("gml")
DRAW_NAMESPACE = n.get_namespace("draw")

GML_SCHEMA_LOCATION = "http://schemas.opengis.net/gml/3.1.1/base/feature.xsd"
DRAW_SCHEMA_LOCATION = 'http://cida.usgs.gov/climate/derivative/xsd/draw.xsd'
WFS_SCHEMA_LOCATION = 'http://schemas.opengis.net/wfs/1.1.0/wfs.xsd'
WPS_DEFAULT_SCHEMA_LOCATION = 'http://schemas.opengis.net/wps/1.0.0/wpsExecute_request.xsd'
WPS_DEFAULT_VERSION = '1.0.0'


def get_namespaces():
    ns = n.get_namespaces(["ogc", "wfs", "wps", "gml", "xsi", "xlink"])
    ns[None] = n.get_namespace("wps")
    ns["ows"] = DEFAULT_OWS_NAMESPACE
    return ns
namespaces = get_namespaces()


def is_reference(val):
    """
    Checks if the provided value is a reference (URL).
    """
    try:
        parsed = urlparse(val)
        is_ref = parsed.scheme != ''
    except:
        is_ref = False
    return is_ref


def is_literaldata(val):
    """
    Checks if the provided value is a string (includes unicode).
    """
    is_str = isinstance(val, str)
    if not is_str:
        # on python 2.x we need to check unicode
        try:
            is_str = isinstance(val, unicode)
        except:
            # unicode is not available on python 3.x
            is_str = False
    return is_str


def is_complexdata(val):
    """
    Checks if the provided value is an implementation of IComplexData.
    """
    return hasattr(val, 'getXml')


class IComplexDataInput(object):

    """
    Abstract interface representing complex input object for a WPS request.
    """

    def getXml(self):
        """
        Method that returns the object data as an XML snippet,
        to be inserted into the WPS request document sent to the server.
        """
        raise NotImplementedError


class WebProcessingService(object):

    """
    Class that contains client-side functionality for invoking an OGC Web Processing Service (WPS).

    Implements IWebProcessingService.
    """

    def __init__(self, url, version=WPS_DEFAULT_VERSION, username=None, password=None, verbose=False, skip_caps=False):
        """
        Initialization method resets the object status.
        By default it will execute a GetCapabilities invocation to the remote service,
        which can be skipped by using skip_caps=True.
        """

        # fields passed in from object initializer
        self.url = url
        self.username = username
        self.password = password
        self.version = version
        self.verbose = verbose

        # fields populated by method invocations
        self._capabilities = None
        self.identification = None
        self.provider = None
        self.operations = []
        self.processes = []

        if not skip_caps:
            self.getcapabilities()

    def getcapabilities(self, xml=None):
        """
        Method that requests a capabilities document from the remote WPS server and populates this object's metadata.
        keyword argument xml: local XML GetCapabilities document, prevents actual HTTP invocation.
        """

        # read capabilities document
        reader = WPSCapabilitiesReader(
            version=self.version, verbose=self.verbose)
        if xml:
            # read from stored XML file
            self._capabilities = reader.readFromString(xml)
        else:
            self._capabilities = reader.readFromUrl(
                self.url, username=self.username, password=self.password)

        log.debug(element_to_string(self._capabilities))

        # populate the capabilities metadata obects from the XML tree
        self._parseCapabilitiesMetadata(self._capabilities)

    def describeprocess(self, identifier, xml=None):
        """
        Requests a process document from a WPS service and populates the process metadata.
        Returns the process object.
        """

        # read capabilities document
        reader = WPSDescribeProcessReader(
            version=self.version, verbose=self.verbose)
        if xml:
            # read from stored XML file
            rootElement = reader.readFromString(xml)
        else:
            # read from server
            rootElement = reader.readFromUrl(self.url, identifier)

        log.info(element_to_string(rootElement))

        # build metadata objects
        return self._parseProcessMetadata(rootElement)

    def execute(self, identifier, inputs, output=None, request=None, response=None):
        """
        Submits a WPS process execution request.
        Returns a WPSExecution object, which can be used to monitor the status of the job, and ultimately retrieve the result.

        identifier: the requested process identifier
        inputs: list of process inputs as (key, value) tuples (where value is either a string for LiteralData, or an object for ComplexData)
        output: optional identifier for process output reference (if not provided, output will be embedded in the response)
        request: optional pre-built XML request document, prevents building of request from other arguments
        response: optional pre-built XML response document, prevents submission of request to live WPS server
        """

        # instantiate a WPSExecution object
        log.info('Executing WPS request...')
        execution = WPSExecution(version=self.version, url=self.url,
                                 username=self.username, password=self.password, verbose=self.verbose)

        # build XML request from parameters
        if request is None:
            requestElement = execution.buildRequest(identifier, inputs, output)
            request = etree.tostring(requestElement)
            execution.request = request
        log.debug(request)

        # submit the request to the live server
        if response is None:
            response = execution.submitRequest(request)
        else:
            response = etree.fromstring(response)

        log.debug(etree.tostring(response))

        # parse response
        execution.parseResponse(response)

        return execution

    def _parseProcessMetadata(self, rootElement):
        """
        Method to parse a <ProcessDescriptions> XML element and returned the constructed Process object
        """

        processDescriptionElement = rootElement.find('ProcessDescription')
        process = Process(processDescriptionElement, verbose=self.verbose)

        # override existing processes in object metadata, if existing already
        found = False
        for n, p in enumerate(self.processes):
            if p.identifier == process.identifier:
                self.processes[n] = process
                found = True
        # otherwise add it
        if not found:
            self.processes.append(process)

        return process

    def _parseCapabilitiesMetadata(self, root):
        ''' Sets up capabilities metadata objects '''

        # use the WPS namespace defined in the document root
        wpsns = getNamespace(root)

        # loop over children WITHOUT requiring a specific namespace
        for element in root:

            # thie element's namespace
            ns = getNamespace(element)

            # <ows:ServiceIdentification> metadata
            if element.tag.endswith('ServiceIdentification'):
                self.identification = ServiceIdentification(
                    element, namespace=ns)
                if self.verbose == True:
                    dump(self.identification)

            # <ows:ServiceProvider> metadata
            elif element.tag.endswith('ServiceProvider'):
                self.provider = ServiceProvider(element, namespace=ns)
                if self.verbose == True:
                    dump(self.provider)

            # <ns0:OperationsMetadata xmlns:ns0="http://www.opengeospatial.net/ows">
            #   <ns0:Operation name="GetCapabilities">
            #     <ns0:DCP>
            #       <ns0:HTTP>
            #         <ns0:Get xlink:href="http://ceda-wps2.badc.rl.ac.uk/wps?" xmlns:xlink="http://www.w3.org/1999/xlink" />
            #       </ns0:HTTP>
            #    </ns0:DCP>
            #  </ns0:Operation>
            #  ........
            # </ns0:OperationsMetadata>
            elif element.tag.endswith('OperationsMetadata'):
                for child in element.findall(nspath('Operation', ns=ns)):
                    self.operations.append(
                        OperationsMetadata(child, namespace=ns))
                    if self.verbose == True:
                        dump(self.operations[-1])

            # <wps:ProcessOfferings>
            #   <wps:Process ns0:processVersion="1.0.0">
            #     <ows:Identifier xmlns:ows="http://www.opengis.net/ows/1.1">gov.usgs.cida.gdp.wps.algorithm.filemanagement.ReceiveFiles</ows:Identifier>
            #     <ows:Title xmlns:ows="http://www.opengis.net/ows/1.1">gov.usgs.cida.gdp.wps.algorithm.filemanagement.ReceiveFiles</ows:Title>
            #   </wps:Process>
            #   ......
            # </wps:ProcessOfferings>
            elif element.tag.endswith('ProcessOfferings'):
                for child in element.findall(nspath('Process', ns=ns)):
                    p = Process(child, verbose=self.verbose)
                    self.processes.append(p)
                    if self.verbose == True:
                        dump(self.processes[-1])


class WPSReader(object):

    """
    Superclass for reading a WPS document into a lxml.etree infoset.
    """

    def __init__(self, version=WPS_DEFAULT_VERSION, verbose=False):
        self.version = version
        self.verbose = verbose

    def _readFromUrl(self, url, data, method='Get', username=None, password=None):
        """
        Method to get and parse a WPS document, returning an elementtree instance.
        url: WPS service base url.
        data: GET: dictionary of HTTP (key, value) parameter pairs, POST: XML document to post
        username, password: optional user credentials
        """

        if method == 'Get':
            # full HTTP request url
            request_url = build_get_url(url, data)
            log.debug(request_url)

            # split URL into base url and query string to use utility function
            spliturl = request_url.split('?')
            u = openURL(spliturl[0], spliturl[
                        1], method='Get', username=username, password=password)
            return etree.fromstring(u.read())

        elif method == 'Post':
            u = openURL(url, data, method='Post',
                        username=username, password=password)
            return etree.fromstring(u.read())

        else:
            raise Exception("Unrecognized HTTP method: %s" % method)

    def readFromString(self, string):
        """
        Method to read a WPS GetCapabilities document from an XML string.
        """

        if not isinstance(string, str) and not isinstance(string, bytes):
            raise ValueError(
                "Input must be of type string, not %s" % type(string))
        return etree.fromstring(string)


class WPSCapabilitiesReader(WPSReader):

    """
    Utility class that reads and parses a WPS GetCapabilities document into a lxml.etree infoset.
    """

    def __init__(self, version=WPS_DEFAULT_VERSION, verbose=False):
        # superclass initializer
        super(WPSCapabilitiesReader, self).__init__(
            version=version, verbose=verbose)

    def readFromUrl(self, url, username=None, password=None):
        """
        Method to get and parse a WPS capabilities document, returning an elementtree instance.
        url: WPS service base url, to which is appended the HTTP parameters: service, version, and request.
        username, password: optional user credentials
        """
        return self._readFromUrl(url,
                                 {'service': 'WPS', 'request':
                                     'GetCapabilities', 'version': self.version},
                                 username=username, password=password)


class WPSDescribeProcessReader(WPSReader):

    """
    Class that reads and parses a WPS DescribeProcess document into a etree infoset
    """

    def __init__(self, version=WPS_DEFAULT_VERSION, verbose=False):
        # superclass initializer
        super(WPSDescribeProcessReader, self).__init__(
            version=version, verbose=verbose)

    def readFromUrl(self, url, identifier, username=None, password=None):
        """
        Reads a WPS DescribeProcess document from a remote service and returns the XML etree object
        url: WPS service base url, to which is appended the HTTP parameters: 'service', 'version', and 'request', and 'identifier'.
        """

        return self._readFromUrl(url,
                                 {'service': 'WPS', 'request': 'DescribeProcess',
                                     'version': self.version, 'identifier': identifier},
                                 username=username, password=password)


class WPSExecuteReader(WPSReader):

    """
    Class that reads and parses a WPS Execute response document into a etree infoset
    """

    def __init__(self, verbose=False):
        # superclass initializer
        super(WPSExecuteReader, self).__init__(verbose=verbose)

    def readFromUrl(self, url, data={}, method='Get', username=None, password=None):
        """
        Reads a WPS status document from a remote service and returns the XML etree object.
        url: the URL to submit the GET/POST request to.
        """

        return self._readFromUrl(url, data, method, username=username, password=password)


class WPSExecution():

    """
    Class that represents a single WPS process executed on a remote WPS service.
    """

    def __init__(self, version=WPS_DEFAULT_VERSION, url=None, username=None, password=None, verbose=False):

        # initialize fields
        self.url = url
        self.version = version
        self.username = username
        self.password = password
        self.verbose = verbose

        # request document
        self.request = None

        # last response document
        self.response = None

        # status fields retrieved from the response documents
        self.process = None
        self.serviceInstance = None
        self.status = None
        self.percentCompleted = 0
        self.statusMessage = None
        self.errors = []
        self.statusLocation = None
        self.dataInputs = []
        self.processOutputs = []

    def buildRequest(self, identifier, inputs=[], output=None):
        """
        Method to build a WPS process request.
        identifier: the requested process identifier
        inputs: array of input arguments for the process.
            - LiteralData inputs are expressed as simple (key,value) tuples where key is the input identifier, value is the value
            - ComplexData inputs are expressed as (key, object) tuples, where key is the input identifier,
              and the object must contain a 'getXml()' method that returns an XML infoset to be included in the WPS request
        output: optional identifier if process output is to be returned as a hyperlink reference
        """

        #<wps:Execute xmlns:wps="http://www.opengis.net/wps/1.0.0"
        #             xmlns:ows="http://www.opengis.net/ows/1.1"
        #             xmlns:xlink="http://www.w3.org/1999/xlink"
        #             xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
        #             service="WPS"
        #             version="1.0.0"
        # xsi:schemaLocation="http://www.opengis.net/wps/1.0.0
        # http://schemas.opengis.net/wps/1.0.0/wpsExecute_request.xsd">
        root = etree.Element(nspath_eval('wps:Execute', namespaces))
        root.set('service', 'WPS')
        root.set('version', WPS_DEFAULT_VERSION)
        root.set(nspath_eval('xsi:schemaLocation', namespaces), '%s %s' %
                 (namespaces['wps'], WPS_DEFAULT_SCHEMA_LOCATION))

        # <ows:Identifier>gov.usgs.cida.gdp.wps.algorithm.FeatureWeightedGridStatisticsAlgorithm</ows:Identifier>
        identifierElement = etree.SubElement(
            root, nspath_eval('ows:Identifier', namespaces))
        identifierElement.text = identifier

        # <wps:DataInputs>
        dataInputsElement = etree.SubElement(
            root, nspath_eval('wps:DataInputs', namespaces))

        for (key, val) in inputs:

            inputElement = etree.SubElement(
                dataInputsElement, nspath_eval('wps:Input', namespaces))
            identifierElement = etree.SubElement(
                inputElement, nspath_eval('ows:Identifier', namespaces))
            identifierElement.text = key

            # Literal data
            # <wps:Input>
            #   <ows:Identifier>DATASET_URI</ows:Identifier>
            #   <wps:Data>
            #     <wps:LiteralData>dods://igsarm-cida-thredds1.er.usgs.gov:8080/thredds/dodsC/dcp/conus_grid.w_meta.ncml</wps:LiteralData>
            #   </wps:Data>
            # </wps:Input>
            if is_literaldata(val):
                log.debug("literaldata %s", key)
                dataElement = etree.SubElement(
                    inputElement, nspath_eval('wps:Data', namespaces))
                literalDataElement = etree.SubElement(
                    dataElement, nspath_eval('wps:LiteralData', namespaces))
                literalDataElement.text = val

            # Complex data
            # <wps:Input>
            #   <ows:Identifier>FEATURE_COLLECTION</ows:Identifier>
            #   <wps:Reference xlink:href="http://igsarm-cida-gdp2.er.usgs.gov:8082/geoserver/wfs">
            #      <wps:Body>
            #        <wfs:GetFeature xmlns:wfs="http://www.opengis.net/wfs" xmlns:ogc="http://www.opengis.net/ogc" xmlns:gml="http://www.opengis.net/gml" service="WFS" version="1.1.0" outputFormat="text/xml; subtype=gml/3.1.1" xsi:schemaLocation="http://www.opengis.net/wfs ../wfs/1.1.0/WFS.xsd">
            #            <wfs:Query typeName="sample:CONUS_States">
            #                <wfs:PropertyName>the_geom</wfs:PropertyName>
            #                <wfs:PropertyName>STATE</wfs:PropertyName>
            #                <ogc:Filter>
            #                    <ogc:GmlObjectId gml:id="CONUS_States.508"/>
            #                </ogc:Filter>
            #            </wfs:Query>
            #        </wfs:GetFeature>
            #      </wps:Body>
            #   </wps:Reference>
            # </wps:Input>
            elif is_complexdata(val):
                log.debug("complexdata %s", key)
                inputElement.append(val.getXml())
            else:
                raise Exception(
                    'input type of "%s" parameter is unknown' % key)

        # <wps:ResponseForm>
        #   <wps:ResponseDocument storeExecuteResponse="true" status="true">
        #     <wps:Output asReference="true">
        #       <ows:Identifier>OUTPUT</ows:Identifier>
        #     </wps:Output>
        #   </wps:ResponseDocument>
        # </wps:ResponseForm>
        if output is not None:
            responseFormElement = etree.SubElement(
                root, nspath_eval('wps:ResponseForm', namespaces))
            responseDocumentElement = etree.SubElement(
                responseFormElement, nspath_eval(
                    'wps:ResponseDocument', namespaces),
                                                       attrib={'storeExecuteResponse': 'true', 'status': 'true'})
            if isinstance(output, str):
                self._add_output(
                    responseDocumentElement, output, asReference=True)
            elif isinstance(output, list):
                for (identifier, as_reference) in output:
                    self._add_output(
                        responseDocumentElement, identifier, asReference=as_reference)
            else:
                raise Exception(
                    'output parameter is neither string nor list. output=%s' % output)
        return root

    def _add_output(self, element, identifier, asReference=False):
        outputElement = etree.SubElement(
            element, nspath_eval('wps:Output', namespaces),
                                                       attrib={'asReference': str(asReference).lower()})
        outputIdentifierElement = etree.SubElement(
            outputElement, nspath_eval('ows:Identifier', namespaces)).text = identifier

    # wait for 60 seconds by default
    def checkStatus(self, url=None, response=None, sleepSecs=60):
        """
        Method to check the status of a job execution.
        In the process, this method will upadte the object 'response' attribute.

        url: optional 'statusLocation' URL retrieved from a previous WPS Execute response document.
             If not provided, the current 'statusLocation' URL will be used.
        sleepSecs: number of seconds to sleep before returning control to the caller.
        """

        reader = WPSExecuteReader(verbose=self.verbose)
        if response is None:
            # override status location
            if url is not None:
                self.statusLocation = url
            log.info('\nChecking execution status... (location=%s)' %
                     self.statusLocation)
            response = reader.readFromUrl(
                self.statusLocation, username=self.username, password=self.password)
        else:
            response = reader.readFromString(response)

        # store latest response
        self.response = etree.tostring(response)
        log.debug(self.response)

        self.parseResponse(response)

        # sleep given number of seconds
        if self.isComplete() == False:
            log.info('Sleeping %d seconds...' % sleepSecs)
            sleep(sleepSecs)

    def getStatus(self):
        return self.status

    def isComplete(self):
        if (self.status == 'ProcessSucceeded' or self.status == 'ProcessFailed' or self.status == 'Exception'):
            return True
        elif (self.status == 'ProcessStarted'):
            return False
        elif (self.status == 'ProcessAccepted' or self.status == 'ProcessPaused'):
            return False
        else:
            raise Exception(
                'Unknown process execution status: %s' % self.status)

    def isSucceded(self):
        if self.status == 'ProcessSucceeded':
            return True
        else:
            return False

    def isNotComplete(self):
        return not self.isComplete()

    def getOutput(self, filepath=None):
        """
        Method to write the outputs of a WPS process to a file:
        either retrieves the referenced files from the server, or writes out the content of response embedded output.

        filepath: optional path to the output file, otherwise a file will be created in the local directory with the name assigned by the server,
                  or default name 'wps.out' for embedded output.
        """

        if self.isSucceded():
            content = ''
            for output in self.processOutputs:

                output_content = output.retrieveData(
                    self.username, self.password)

                # ExecuteResponse contains reference to server-side output
                if output_content is not "":
                    content = content + output_content
                    if filepath is None:
                        filepath = output.fileName

                # ExecuteResponse contain embedded output
                if len(output.data) > 0:
                    if filepath is None:
                        filepath = 'wps.out'
                    for data in output.data:
                        content = content + data

            # write out content
            if content is not '':
                out = open(filepath, 'wb')
                out.write(content)
                out.close()
                log.info('Output written to file: %s' % filepath)

        else:
            raise Exception(
                "Execution not successfully completed: status=%s" % self.status)

    def submitRequest(self, request):
        """
        Submits a WPS Execute document to a remote service, returns the XML response document from the server.
        This method will save the request document and the first returned response document.

        request: the XML request document to be submitted as POST to the server.
        """

        self.request = request
        reader = WPSExecuteReader(verbose=self.verbose)
        response = reader.readFromUrl(
            self.url, request, method='Post', username=self.username, password=self.password)
        self.response = response
        return response

        '''
        if response is None:
            # override status location
            if url is not None:
                self.statusLocation = url

        else:
            response = reader.readFromString(response)


        '''

    def parseResponse(self, response):
        """
        Method to parse a WPS response document
        """

        rootTag = response.tag.split('}')[1]
        # <ns0:ExecuteResponse>
        if rootTag == 'ExecuteResponse':
            self._parseExecuteResponse(response)

        # <ows:ExceptionReport>
        elif rootTag == 'ExceptionReport':
            self._parseExceptionReport(response)

        else:
            log.debug('Unknown Response')

        # log status, errors
        log.info('Execution status=%s' % self.status)
        log.info('Percent completed=%s' % self.percentCompleted)
        log.info('Status message=%s' % self.statusMessage)
        for error in self.errors:
            dump(error)

    def _parseExceptionReport(self, root):
        """
        Method to parse a WPS ExceptionReport document and populate this object's metadata.
        """
        # set exception status, unless set already
        if self.status is None:
            self.status = "Exception"

        for exceptionEl in root.findall(nspath('Exception', ns=namespaces['ows'])):
            self.errors.append(WPSException(exceptionEl))

    def _parseExecuteResponse(self, root):
        """
        Method to parse a WPS ExecuteResponse response document and populate this object's metadata.
        """

        # retrieve WPS namespace directly from root element
        wpsns = getNamespace(root)

        self.serviceInstance = root.get('serviceInstance')
        if self.statusLocation is None:
            self.statusLocation = root.get('statusLocation')

        # <ns0:Status creationTime="2011-11-09T14:19:50Z">
        #  <ns0:ProcessSucceeded>PyWPS Process v.net.path successfully calculated</ns0:ProcessSucceeded>
        # </ns0:Status>
        # OR
        # <ns0:Status creationTime="2011-11-07T08:26:44.359-06:00">
        #  <ns0:ProcessFailed>
        #   <ows:ExceptionReport xmlns:ows="http://www.opengis.net/ows/1.1">
        #    <ows:Exception>
        #     <ows:ExceptionText>Attribute null not found in feature collection</ows:ExceptionText>
        #    </ows:Exception>
        #   </ows:ExceptionReport>
        #  </ns0:ProcessFailed>
        # </ns0:Status>
        statusEl = root.find(nspath('Status/*', ns=wpsns))
        self.status = statusEl.tag.split('}')[1]
        # get progress info
        try:
            percentCompleted = int(statusEl.get('percentCompleted'))
            self.percentCompleted = percentCompleted
        except:
            pass
        # get status message
        self.statusMessage = statusEl.text
        # exceptions ?
        for element in statusEl:
            if element.tag.endswith('ExceptionReport'):
                self._parseExceptionReport(element)

        self.process = Process(
            root.find(nspath('Process', ns=wpsns)), verbose=self.verbose)

        #<wps:DataInputs xmlns:wps="http://www.opengis.net/wps/1.0.0"
        # xmlns:ows="http://www.opengis.net/ows/1.1"
        # xmlns:xlink="http://www.w3.org/1999/xlink">
        for inputElement in root.findall(nspath('DataInputs/Input', ns=wpsns)):
            self.dataInputs.append(Input(inputElement))
            if self.verbose == True:
                dump(self.dataInputs[-1])

        # <ns:ProcessOutputs>
        # xmlns:ns="http://www.opengis.net/wps/1.0.0"
        for outputElement in root.findall(nspath('ProcessOutputs/Output', ns=wpsns)):
            self.processOutputs.append(Output(outputElement))
            if self.verbose == True:
                dump(self.processOutputs[-1])


class ComplexData(object):

    """
    Class that represents a ComplexData element in a WPS document
    """

    def __init__(self, mimeType=None, encoding=None, schema=None):
        self.mimeType = mimeType
        self.encoding = encoding
        self.schema = schema


class InputOutput(object):

    """
    Superclass of a WPS input or output data object.
    """

    def __init__(self, element):

        self.abstract = None

        # loop over sub-elements without requiring a specific namespace
        for subElement in element:

            # <ows:Identifier xmlns:ows="http://www.opengis.net/ows/1.1">SUMMARIZE_TIMESTEP</ows:Identifier>
            if subElement.tag.endswith('Identifier'):
                self.identifier = testXMLValue(subElement)

            # <ows:Title xmlns:ows="http://www.opengis.net/ows/1.1">Summarize Timestep</ows:Title>
            elif subElement.tag.endswith('Title'):
                self.title = testXMLValue(subElement)

            # <ows:Abstract xmlns:ows="http://www.opengis.net/ows/1.1">If selected, processing output will include columns with summarized statistics for all feature attribute values for each timestep</ows:Abstract>
            elif subElement.tag.endswith('Abstract'):
                self.abstract = testXMLValue(subElement)

        self.allowedValues = []
        self.supportedValues = []
        self.defaultValue = None
        self.dataType = None
        self.anyValue = False

    def _parseData(self, element):
        """
        Method to parse a "Data" element
        """

        # <ns0:Data>
        #        <ns0:ComplexData mimeType="text/plain">
        #             7504912.93758151 -764109.175074507,7750849.82379226 -22141.8611641468,8561828.42371234 -897195.923493867,7724946.16844165 -602984.014261927
        #        </ns0:ComplexData>
        # </ns0:Data>
        # nspath('Data', ns=WPS_NAMESPACE)
        complex_data_element = element.find(
            nspath('ComplexData', ns=getNamespace(element)))
        if complex_data_element is not None:
            self.dataType = "ComplexData"

    def _parseLiteralData(self, element, literalElementName):
        """
        Method to parse the LiteralData element.
        """

        # <LiteralData>
        #    <ows:DataType ows:reference="xs:string" xmlns:ows="http://www.opengis.net/ows/1.1" />
        #    <ows:AllowedValues xmlns:ows="http://www.opengis.net/ows/1.1">
        #        <ows:Value>COMMA</ows:Value>
        #        <ows:Value>TAB</ows:Value>
        #        <ows:Value>SPACE</ows:Value>
        #    </ows:AllowedValues>
        #    <DefaultValue>COMMA</DefaultValue>
        # </LiteralData>

        # <LiteralData>
        #     <ows:DataType ows:reference="xs:anyURI" xmlns:ows="http://www.opengis.net/ows/1.1" />
        #     <ows:AnyValue xmlns:ows="http://www.opengis.net/ows/1.1" />
        # </LiteralData>

        literal_data_element = element.find(literalElementName)

        if literal_data_element is not None:
            self.dataType = 'LiteralData'
            for sub_element in literal_data_element:
                subns = getNamespace(sub_element)
                if sub_element.tag.endswith('DataType'):
                    self.dataType = sub_element.get(
                        nspath("reference", ns=subns)).split(':')[-1]

            for sub_element in literal_data_element:

                subns = getNamespace(sub_element)

                if sub_element.tag.endswith('DefaultValue'):
                    self.defaultValue = getTypedValue(
                        self.dataType, sub_element.text)

                if sub_element.tag.endswith('AllowedValues'):
                    for value in sub_element.findall(nspath('Value', ns=subns)):
                        self.allowedValues.append(
                            getTypedValue(self.dataType, value.text))
                elif sub_element.tag.endswith('AnyValue'):
                    self.anyValue = True

    def _parseComplexData(self, element, complexDataElementName):
        """
        Method to parse a ComplexData or ComplexOutput element.
        """

        # <ComplexData>
        #     <Default>
        #         <Format>
        #            <MimeType>text/xml</MimeType>
        #            <Encoding>UTF-8</Encoding>
        #            <Schema>http://schemas.opengis.net/gml/2.0.0/feature.xsd</Schema>
        #        </Format>
        #    </Default>
        #    <Supported>
        #        <Format>
        #            <MimeType>text/xml</MimeType>
        #            <Encoding>UTF-8</Encoding>
        #            <Schema>http://schemas.opengis.net/gml/2.0.0/feature.xsd</Schema>
        #        </Format>
        #        <Format>
        #            <MimeType>text/xml</MimeType>
        #            <Encoding>UTF-8</Encoding>
        #            <Schema>http://schemas.opengis.net/gml/2.1.1/feature.xsd</Schema>
        #        </Format>
        #    </Supported>
        # </ComplexData>
        # OR
        # <ComplexOutput defaultEncoding="UTF-8" defaultFormat="text/XML" defaultSchema="NONE">
        #     <SupportedComplexData>
        #         <Format>text/XML</Format>
        #         <Encoding>UTF-8</Encoding>
        #         <Schema>NONE</Schema>
        #     </SupportedComplexData>
        # </ComplexOutput>

        complex_data_element = element.find(complexDataElementName)
        if complex_data_element is not None:
            self.dataType = "ComplexData"

            for supported_comlexdata_element in\
                    complex_data_element.findall('SupportedComplexData'):
                self.supportedValues.append(
                    ComplexData(
                        mimeType=testXMLValue(
                            supported_comlexdata_element.find('Format')),
                        encoding=testXMLValue(
                            supported_comlexdata_element.find('Encoding')),
                        schema=testXMLValue(
                            supported_comlexdata_element.find('Schema'))
                    )
                )

            for format_element in\
                    complex_data_element.findall('Supported/Format'):
                self.supportedValues.append(
                    ComplexData(
                        mimeType=testXMLValue(format_element.find('MimeType')),
                        encoding=testXMLValue(format_element.find('Encoding')),
                        schema=testXMLValue(format_element.find('Schema'))
                    )
                )

            default_format_element = complex_data_element.find('Default/Format')
            if default_format_element is not None:
                self.defaultValue = ComplexData(
                    mimeType=testXMLValue(
                        default_format_element.find('MimeType')),
                    encoding=testXMLValue(
                        default_format_element.find('Encoding')),
                    schema=testXMLValue(default_format_element.find('Schema'))
                )

    def _parseBoundingBoxData(self, element, bboxElementName):
        """
        Method to parse the BoundingBoxData element.
        """

        # <BoundingBoxData>
        #   <Default>
        #     <CRS>epsg:4326</CRS>
        #   </Default>
        #   <Supported>
        #     <CRS>epsg:4326</CRS>
        #   </Supported>
        # </BoundingBoxData>
        #
        # OR
        #
        # <BoundingBoxOutput>
        #   <Default>
        #     <CRS>epsg:4326</CRS>
        #   </Default>
        #   <Supported>
        #     <CRS>epsg:4326</CRS>
        #   </Supported>
        # </BoundingBoxOutput>

        bbox_data_element = element.find(bboxElementName)
        if bbox_data_element is not None:
            self.dataType = 'BoundingBoxData'

            for bbox_element in bbox_data_element.findall('Supported/CRS'):
                self.supportedValues.append(bbox_element.text)

            default_bbox_element = bbox_data_element.find('Default/CRS')
            if default_bbox_element is not None:
                self.defaultValue = default_bbox_element.text


class Input(InputOutput):
    """
    Class that represents a WPS process input.
    """

    def __init__(self, inputElement):

        # superclass initializer
        super(Input, self).__init__(inputElement)

        # <Input maxOccurs="1" minOccurs="0">
        # OR
        # <MinimumOccurs>1</MinimumOccurs>
        self.minOccurs = -1
        if inputElement.get("minOccurs") is not None:
            self.minOccurs = int(inputElement.get("minOccurs"))
        if inputElement.find('MinimumOccurs') is not None:
            self.minOccurs = int(
                testXMLValue(inputElement.find('MinimumOccurs')))
        self.maxOccurs = -1
        if inputElement.get("maxOccurs") is not None:
            self.maxOccurs = int(inputElement.get("maxOccurs"))
        if inputElement.find('MaximumOccurs') is not None:
            self.maxOccurs = int(
                testXMLValue(inputElement.find('MaximumOccurs')))

        # <LiteralData>
        self._parseLiteralData(inputElement, 'LiteralData')

        # <ComplexData>
        self._parseComplexData(inputElement, 'ComplexData')

        # <BoundingBoxData>
        self._parseBoundingBoxData(inputElement, 'BoundingBoxData')


class Output(InputOutput):

    """
    Class that represents a WPS process output.
    """

    def __init__(self, outputElement):

        # superclass initializer
        super(Output, self).__init__(outputElement)

        self.reference = None
        self.mimeType = None
        self.data = []
        self.fileName = None
        self.filePath = None

        # extract wps namespace from outputElement itself
        wpsns = getNamespace(outputElement)

        # <ns:Reference encoding="UTF-8" mimeType="text/csv"
        # href="http://cida.usgs.gov/climate/gdp/process/RetrieveResultServlet?id=1318528582026OUTPUT.601bb3d0-547f-4eab-8642-7c7d2834459e"
        # />
        referenceElement = outputElement.find(nspath('Reference', ns=wpsns))
        if referenceElement is not None:
            self.reference = referenceElement.get('href')
            self.mimeType = referenceElement.get('mimeType')

        # <LiteralOutput>
        self._parseLiteralData(outputElement, 'LiteralOutput')

        # <ComplexData> or <ComplexOutput>
        self._parseComplexData(outputElement, 'ComplexOutput')

        # <BoundingBoxData>
        self._parseBoundingBoxData(outputElement, 'BoundingBoxOutput')

        # <Data>
        # <ns0:Data>
        #        <ns0:ComplexData mimeType="text/plain">
        #             7504912.93758151 -764109.175074507,7750849.82379226 -22141.8611641468,8561828.42371234 -897195.923493867,7724946.16844165 -602984.014261927
        #        </ns0:ComplexData>
        # </ns0:Data>
        # OR:
        # <ns0:Data>
        #     <ns0:ComplexData encoding="UTF-8" mimeType="text/xml" schema="http://schemas.opengis.net/gml/2.1.2/feature.xsd">
        #         <ns3:FeatureCollection xsi:schemaLocation="http://ogr.maptools.org/ output_0n7ij9D.xsd" xmlns:ns3="http://ogr.maptools.org/">
        #             <gml:boundedBy xmlns:gml="http://www.opengis.net/gml">
        #                 <gml:Box>
        #                     <gml:coord><gml:X>-960123.1421801626</gml:X><gml:Y>4665723.56559387</gml:Y></gml:coord>
        #                     <gml:coord><gml:X>-101288.6510608822</gml:X><gml:Y>5108200.011823481</gml:Y></gml:coord>
        #                 </gml:Box>
        #            </gml:boundedBy>
        #            <gml:featureMember xmlns:gml="http://www.opengis.net/gml">
        #                <ns3:output fid="F0">
        #                    <ns3:geometryProperty><gml:LineString><gml:coordinates>-960123.142180162365548,4665723.565593870356679,0 -960123.142180162365548,4665723.565593870356679,0 -960123.142180162598379,4665723.565593870356679,0 -960123.142180162598379,4665723.565593870356679,0 -711230.141176006174646,4710278.48552671354264,0 -711230.141176006174646,4710278.48552671354264,0 -623656.677859728806652,4848552.374973464757204,0 -623656.677859728806652,4848552.374973464757204,0 -410100.337491964863148,4923834.82589447684586,0 -410100.337491964863148,4923834.82589447684586,0 -101288.651060882242746,5108200.011823480948806,0 -101288.651060882242746,5108200.011823480948806,0 -101288.651060882257298,5108200.011823480948806,0 -101288.651060882257298,5108200.011823480948806,0</gml:coordinates></gml:LineString></ns3:geometryProperty>
        #                    <ns3:cat>1</ns3:cat>
        #                    <ns3:id>1</ns3:id>
        #                    <ns3:fcat>0</ns3:fcat>
        #                    <ns3:tcat>0</ns3:tcat>
        #                    <ns3:sp>0</ns3:sp>
        #                    <ns3:cost>1002619.181</ns3:cost>
        #                    <ns3:fdist>0</ns3:fdist>
        #                    <ns3:tdist>0</ns3:tdist>
        #                </ns3:output>
        #            </gml:featureMember>
        #        </ns3:FeatureCollection>
        #     </ns0:ComplexData>
        # </ns0:Data>
        #
        #
        # OWS BoundingBox:
        #
        # <wps:Data>
        #   <wps:BoundingBoxData crs="EPSG:4326" dimensions="2">
        #     <ows:LowerCorner>0.0 -90.0</ows:LowerCorner>
        #     <ows:UpperCorner>180.0 90.0</ows:UpperCorner>
        #   </wps:BoundingBoxData>
        # </wps:Data>
        #
        dataElement = outputElement.find(nspath('Data', ns=wpsns))
        if dataElement is not None:
            complexDataElement = dataElement.find(
                nspath('ComplexData', ns=wpsns))
            if complexDataElement is not None:
                self.dataType = "ComplexData"
                self.mimeType = complexDataElement.get('mimeType')
                if complexDataElement.text is not None and complexDataElement.text.strip() is not '':
                    self.data.append(complexDataElement.text.strip())
                for child in complexDataElement:
                    self.data.append(etree.tostring(child))
            literalDataElement = dataElement.find(
                nspath('LiteralData', ns=wpsns))
            if literalDataElement is not None:
                self.dataType = literalDataElement.get('dataType')
                if literalDataElement.text is not None and literalDataElement.text.strip() is not '':
                    self.data.append(literalDataElement.text.strip())
            bboxDataElement = dataElement.find(
                nspath('BoundingBoxData', ns=wpsns))
            if bboxDataElement is not None:
                self.dataType = "BoundingBoxData"
                bbox = BoundingBox(bboxDataElement)
                if bbox is not None and bbox.minx is not None:
                    bbox_value = None
                    if bbox.crs is not None and bbox.crs.axisorder == 'yx':
                        bbox_value = "{0},{1},{2},{3}".format(bbox.miny, bbox.minx, bbox.maxy, bbox.maxx)
                    else:
                        bbox_value = "{0},{1},{2},{3}".format(bbox.minx, bbox.miny, bbox.maxx, bbox.maxy)
                    log.debug("bbox=%s", bbox_value)
                    self.data.append(bbox_value)

    def retrieveData(self, username=None, password=None):
        """
        Method to retrieve data from server-side reference:
        returns "" if the reference is not known.

        username, password: credentials to access the remote WPS server
        """

        url = self.reference
        if url is None:
            return ""

        # a) 'http://cida.usgs.gov/climate/gdp/process/RetrieveResultServlet?id=1318528582026OUTPUT.601bb3d0-547f-4eab-8642-7c7d2834459e'
        # b) 'http://rsg.pml.ac.uk/wps/wpsoutputs/outputImage-11294Bd6l2a.tif'
        log.info('Output URL=%s' % url)
        if '?' in url:
            spliturl = url.split('?')
            u = openURL(spliturl[0], spliturl[
                        1], method='Get', username=username, password=password)
            # extract output filepath from URL query string
            self.fileName = spliturl[1].split('=')[1]
        else:
            u = openURL(
                url, '', method='Get', username=username, password=password)
            # extract output filepath from base URL
            self.fileName = url.split('/')[-1]

        return u.read()

    def writeToDisk(self, path=None, username=None, password=None):
        """
        Method to write an output of a WPS process to disk:
        it either retrieves the referenced file from the server, or write out the content of response embedded output.

        filepath: optional path to the output file, otherwise a file will be created in the local directory with the name assigned by the server,
        username, password: credentials to access the remote WPS server
        """

        # Check if ExecuteResponse contains reference to server-side output
        content = self.retrieveData(username, password)

        # ExecuteResponse contain embedded output
        if content is "" and len(self.data) > 0:
            self.fileName = self.identifier
            for data in self.data:
                content = content + data

        # write out content
        if content is not "":
            if self.fileName == "":
                self.fileName = self.identifier
            self.filePath = path + self.fileName
            out = open(self.filePath, 'wb')
            out.write(content)
            out.close()
            log.info('Output written to file: %s' % self.filePath)


class WPSException:

    """
    Class representing an exception raised by a WPS.
    """

    def __init__(self, root):
        self.code = root.attrib.get("exceptionCode", None)
        self.locator = root.attrib.get("locator", None)
        textEl = root.find(nspath('ExceptionText', ns=getNamespace(root)))
        if textEl is not None:
            self.text = textEl.text
        else:
            self.text = ""


class Process(object):

    """
    Class that represents a WPS process.
    """

    def __init__(self, elem, verbose=False):
        """ Initialization method extracts all available metadata from an XML document (passed in as etree object) """

        # <ns0:ProcessDescriptions service="WPS" version="1.0.0"
        #                          xsi:schemaLocation="http://www.opengis.net/wps/1.0.0 http://schemas.opengis.net/wps/1.0.0/wpsDescribeProcess_response.xsd"
        #                          xml:lang="en-US" xmlns:ns0="http://www.opengis.net/wps/1.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
        # OR:
        # <ns0:Process ns0:processVersion="1.0.0">
        self._root = elem
        self.verbose = verbose

        wpsns = getNamespace(elem)

        # <ProcessDescription statusSupported="true" storeSupported="true" ns0:processVersion="1.0.0">
        self.processVersion = elem.get(nspath('processVersion', ns=wpsns))
        self.statusSupported = bool(elem.get("statusSupported"))
        self.storeSupported = bool(elem.get("storeSupported"))
        self.abstract = None

        for child in elem:

            # this element's namespace
            ns = getNamespace(child)

            # <ows:Identifier xmlns:ows="http://www.opengis.net/ows/1.1">gov.usgs.cida.gdp.wps.algorithm.FeatureWeightedGridStatisticsAlgorithm</ows:Identifier>
            if child.tag.endswith('Identifier'):
                self.identifier = testXMLValue(child)

            # <ows:Title xmlns:ows="http://www.opengis.net/ows/1.1">Feature Weighted Grid Statistics</ows:Title>
            elif child.tag.endswith('Title'):
                self.title = testXMLValue(child)

            # <ows:Abstract xmlns:ows="http://www.opengis.net/ows/1.1">This algorithm generates area weighted statistics of a gridded dataset for a set of vector polygon features. Using the bounding-box that encloses the feature data and the time range, if provided, a subset of the gridded dataset is requested from the remote gridded data server. Polygon representations are generated for cells in the retrieved grid. The polygon grid-cell representations are then projected to the feature data coordinate reference system. The grid-cells are used to calculate per grid-cell feature coverage fractions. Area-weighted statistics are then calculated for each feature using the grid values and fractions as weights. If the gridded dataset has a time range the last step is repeated for each time step within the time range or all time steps if a time range was not supplied.</ows:Abstract>
            elif child.tag.endswith('Abstract'):
                self.abstract = testXMLValue(child)

        if self.verbose == True:
            dump(self)

        # <DataInputs>
        self.dataInputs = []
        for inputElement in elem.findall('DataInputs/Input'):
            self.dataInputs.append(Input(inputElement))
            if self.verbose == True:
                dump(self.dataInputs[-1], prefix='\tInput: ')

        # <ProcessOutputs>
        self.processOutputs = []
        for outputElement in elem.findall('ProcessOutputs/Output'):
            self.processOutputs.append(Output(outputElement))
            if self.verbose == True:
                dump(self.processOutputs[-1],  prefix='\tOutput: ')


class ComplexDataInput(IComplexDataInput, ComplexData):

    def __init__(self, value, mimeType=None, encoding=None, schema=None):
        super(ComplexDataInput, self).__init__(
            mimeType=mimeType, encoding=encoding, schema=schema)
        self.value = value

    def getXml(self):
        if is_reference(self.value):
            return self.complexDataAsReference()
        else:
            return self.complexDataRaw()

    def complexDataAsReference(self):
        """
           <wps:Reference xlink:href="http://somewhere/test.xml"/>
        """
        refElement = etree.Element(nspath_eval('wps:Reference', namespaces),
                                   attrib={nspath_eval("xlink:href", namespaces): self.value})
        return refElement

    def complexDataRaw(self):
        '''
            <wps:Data>
                <wps:ComplexData mimeType="text/xml" encoding="UTF-8"
                    schema="http://schemas.opengis.net/gml/3.1.1/base/feature.xsd">
                </wps:ComplexData>
            </wps:Data>
        '''
        dataElement = etree.Element(nspath_eval('wps:Data', namespaces))

        attrib = dict()
        if self.encoding:
            attrib['encoding'] = self.encoding
        if self.schema:
            attrib['schema'] = self.schema
        if self.mimeType:
            attrib['mimeType'] = self.mimeType
        complexDataElement = etree.SubElement(
            dataElement, nspath_eval('wps:ComplexData', namespaces), attrib=attrib)
        complexDataElement.text = self.value
        return dataElement


class FeatureCollection(IComplexDataInput):

    '''
    Base class to represent a Feature Collection used as input to a WPS request.
    The method getXml() is invoked by the WPS execute() method to build the WPS request.
    All subclasses must implement the getXml() method to provide their specific XML.

    Implements IComplexDataInput.
    '''

    def __init__(self):
        pass

    def getXml(self):
        raise NotImplementedError


class WFSFeatureCollection(FeatureCollection):

    '''
    FeatureCollection specified by a WFS query.
    All subclasses must implement the getQuery() method to provide the specific query portion of the XML.
    '''
    def __init__(self, wfsUrl, wfsQuery, wfsMethod=None):
        '''
        wfsUrl: the WFS service URL
                example: wfsUrl = "http://igsarm-cida-gdp2.er.usgs.gov:8082/geoserver/wfs"
        wfsQuery : a WFS query instance
        '''
        self.url = wfsUrl
        self.query = wfsQuery
        self.method = wfsMethod
    #    <wps:Reference xlink:href="http://igsarm-cida-gdp2.er.usgs.gov:8082/geoserver/wfs">
    #      <wps:Body>
    #        <wfs:GetFeature xmlns:wfs="http://www.opengis.net/wfs" xmlns:ogc="http://www.opengis.net/ogc" xmlns:gml="http://www.opengis.net/gml" service="WFS" version="1.1.0" outputFormat="text/xml; subtype=gml/3.1.1" xsi:schemaLocation="http://www.opengis.net/wfs ../wfs/1.1.0/WFS.xsd">
    #            .......
    #        </wfs:GetFeature>
    #      </wps:Body>
    #   </wps:Reference>
    def getXml(self):
        root = etree.Element(nspath_eval('wps:Reference', namespaces),
                             attrib={nspath_eval("xlink:href", namespaces): self.url})
        if self.method:
            root.attrib['method'] = self.method
        bodyElement = etree.SubElement(
            root, nspath_eval('wps:Body', namespaces))
        getFeatureElement = etree.SubElement(
            bodyElement, nspath_eval('wfs:GetFeature', namespaces),
                                             attrib={"service": "WFS",
                                                     "version": "1.1.0",
                                                     "outputFormat": "text/xml; subtype=gml/3.1.1",
                                                     nspath_eval("xsi:schemaLocation", namespaces): "%s %s" % (namespaces['wfs'], WFS_SCHEMA_LOCATION)})

        #            <wfs:Query typeName="sample:CONUS_States">
        #                <wfs:PropertyName>the_geom</wfs:PropertyName>
        #                <wfs:PropertyName>STATE</wfs:PropertyName>
        #                <ogc:Filter>
        #                    <ogc:GmlObjectId gml:id="CONUS_States.508"/>
        #                </ogc:Filter>
        #            </wfs:Query>
        getFeatureElement.append(self.query.getXml())

        return root


class WFSQuery(IComplexDataInput):

    '''
    Class representing a WFS query, for insertion into a WFSFeatureCollection instance.

    Implements IComplexDataInput.
    '''

    def __init__(self, typeName, propertyNames=[], filters=[]):
        self.typeName = typeName
        self.propertyNames = propertyNames
        self.filters = filters

    def getXml(self):

        #            <wfs:Query typeName="sample:CONUS_States">
        #                <wfs:PropertyName>the_geom</wfs:PropertyName>
        #                <wfs:PropertyName>STATE</wfs:PropertyName>
        #                <ogc:Filter>
        #                    <ogc:GmlObjectId gml:id="CONUS_States.508"/>
        #                </ogc:Filter>
        #            </wfs:Query>

        queryElement = etree.Element(
            nspath_eval('wfs:Query', namespaces), attrib={"typeName": self.typeName})
        for propertyName in self.propertyNames:
            propertyNameElement = etree.SubElement(
                queryElement, nspath_eval('wfs:PropertyName', namespaces))
            propertyNameElement.text = propertyName
        if len(self.filters) > 0:
            filterElement = etree.SubElement(
                queryElement, nspath_eval('ogc:Filter', namespaces))
            for filter in self.filters:
                gmlObjectIdElement = etree.SubElement(
                    filterElement, nspath_eval('ogc:GmlObjectId', namespaces),
                                                      attrib={nspath_eval('gml:id', namespaces): filter})
        return queryElement


class GMLMultiPolygonFeatureCollection(FeatureCollection):

    '''
    Class that represents a FeatureCollection defined as a GML multi-polygon.
    '''

    def __init__(self, polygons):
        '''
        Initializer accepts an array of polygons, where each polygon is an array of (lat,lon) tuples.
        Example: polygons = [ [(-102.8184, 39.5273), (-102.8184, 37.418), (-101.2363, 37.418), (-101.2363, 39.5273), (-102.8184, 39.5273)],
                              [(-92.8184, 39.5273), (-92.8184, 37.418), (-91.2363, 37.418), (-91.2363, 39.5273), (-92.8184, 39.5273)] ]
        '''
        self.polygons = polygons

    def getXml(self):
        '''
            <wps:Data>
                <wps:ComplexData mimeType="text/xml" encoding="UTF-8"
                    schema="http://schemas.opengis.net/gml/3.1.1/base/feature.xsd">
                    <gml:featureMembers xmlns:ogc="http://www.opengis.net/ogc"
                        xmlns:draw="gov.usgs.cida.gdp.draw" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
                        xmlns:ows="http://www.opengis.net/ows" xmlns:gml="http://www.opengis.net/gml"
                        xmlns:xlink="http://www.w3.org/1999/xlink"
                        xsi:schemaLocation="gov.usgs.cida.gdp.draw http://cida.usgs.gov/climate/derivative/xsd/draw.xsd">
                        <gml:box gml:id="box.1">
                            <gml:the_geom>
                                <gml:MultiPolygon srsDimension="2"
                                    srsName="http://www.opengis.net/gml/srs/epsg.xml#4326">
                                    <gml:polygonMember>
                                        <gml:Polygon>
                                            <gml:exterior>
                                                <gml:LinearRing>
                                                    <gml:posList>-102.8184 39.5273 -102.8184 37.418 -101.2363 37.418 -101.2363 39.5273 -102.8184 39.5273</gml:posList>
                                                </gml:LinearRing>
                                            </gml:exterior>
                                        </gml:Polygon>
                                    </gml:polygonMember>
                                </gml:MultiPolygon>
                            </gml:the_geom>
                            <gml:ID>0</gml:ID>
                        </gml:box>
                    </gml:featureMembers>
                </wps:ComplexData>
            </wps:Data>
        '''
        dataElement = etree.Element(nspath_eval('wps:Data', namespaces))
        complexDataElement = etree.SubElement(
            dataElement, nspath_eval('wps:ComplexData', namespaces),
                                              attrib={"mimeType": "text/xml", "encoding": "UTF-8", "schema": GML_SCHEMA_LOCATION})
        featureMembersElement = etree.SubElement(
            complexDataElement, nspath_eval('gml:featureMembers', namespaces),
                                                 attrib={nspath_eval("xsi:schemaLocation", namespaces): "%s %s" % (DRAW_NAMESPACE, DRAW_SCHEMA_LOCATION)})
        boxElement = etree.SubElement(featureMembersElement, nspath_eval(
            'gml:box', namespaces), attrib={nspath_eval("gml:id", namespaces): "box.1"})
        geomElement = etree.SubElement(
            boxElement, nspath_eval('gml:the_geom', namespaces))
        multiPolygonElement = etree.SubElement(
            geomElement, nspath_eval('gml:MultiPolygon', namespaces),
                                               attrib={"srsDimension": "2", "srsName": "http://www.opengis.net/gml/srs/epsg.xml#4326"})
        for polygon in self.polygons:
            polygonMemberElement = etree.SubElement(
                multiPolygonElement, nspath_eval('gml:polygonMember', namespaces))
            polygonElement = etree.SubElement(
                polygonMemberElement, nspath_eval('gml:Polygon', namespaces))
            exteriorElement = etree.SubElement(
                polygonElement, nspath_eval('gml:exterior', namespaces))
            linearRingElement = etree.SubElement(
                exteriorElement, nspath_eval('gml:LinearRing', namespaces))
            posListElement = etree.SubElement(
                linearRingElement, nspath_eval('gml:posList', namespaces))
            posListElement.text = ' '.join(
                ["%s %s" % (x, y) for x, y in polygon[:]])

        idElement = etree.SubElement(
            boxElement, nspath_eval('gml:ID', namespaces))
        idElement.text = "0"
        return dataElement


def monitorExecution(execution, sleepSecs=3, download=False, filepath=None):
    '''
    Convenience method to monitor the status of a WPS execution till it completes (succesfully or not),
    and write the output to file after a succesfull job completion.

    execution: WPSExecution instance
    sleepSecs: number of seconds to sleep in between check status invocations
    download: True to download the output when the process terminates, False otherwise
    filepath: optional path to output file (if downloaded=True), otherwise filepath will be inferred from response document

    '''

    while execution.isComplete() == False:
        execution.checkStatus(sleepSecs=sleepSecs)
        log.info('Execution status: %s' % execution.status)

    if execution.isSucceded():
        if download:
            execution.getOutput(filepath=filepath)
        else:
            for output in execution.processOutputs:
                if output.reference is not None:
                    log.info('Output URL=%s' % output.reference)
    else:
        for ex in execution.errors:
            log.error('Error: code=%s, locator=%s, text=%s' %
                      (ex.code, ex.locator, ex.text))


def printValue(value):
    '''
    Utility method to format a value for printing.
    '''

    # ComplexData type
    if isinstance(value, ComplexData):
        return "mimeType=%s, encoding=%s, schema=%s" % (value.mimeType, value.encoding, value.schema)
    # other type
    else:
        return value


def printInputOutput(value, indent=''):
    '''
    Utility method to inspect an input/output element.
    '''

    # InputOutput fields
    print('%s identifier=%s, title=%s, abstract=%s, data type=%s' %
          (indent, value.identifier, value.title, value.abstract, value.dataType))
    for val in value.allowedValues:
        print('%s Allowed Value: %s' % (indent, printValue(val)))
    if value.anyValue:
        print(' Any value allowed')
    for val in value.supportedValues:
        print('%s Supported Value: %s' % (indent, printValue(val)))
    print('%s Default Value: %s ' % (indent, printValue(value.defaultValue)))

    # Input fields
    if isinstance(value, Input):
        print('%s minOccurs=%d, maxOccurs=%d' %
              (indent, value.minOccurs, value.maxOccurs))

    # Output fields
    if isinstance(value, Output):
        print('%s reference=%s, mimeType=%s' %
              (indent, value.reference, value.mimeType))
        for datum in value.data:
            print('%s Data Value: %s' % (indent, printValue(datum)))
