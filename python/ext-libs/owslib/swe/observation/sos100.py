import cgi
from owslib.etree import etree
from datetime import datetime
from urllib import urlencode
from owslib import ows
from owslib.crs import Crs
from owslib.fes import FilterCapabilities
from owslib.util import openURL, testXMLValue, nspath_eval, nspath, extract_time
from owslib.namespaces import Namespaces

def get_namespaces():
    n = Namespaces()
    ns = n.get_namespaces(["ogc","sml","gml","sos","swe","xlink"])
    ns["ows"] = n.get_namespace("ows110")
    return ns
namespaces = get_namespaces()

class SensorObservationService_1_0_0(object):
    """
        Abstraction for OGC Sensor Observation Service (SOS).

        Implements ISensorObservationService.
    """

    def __new__(self,url, version, xml=None, username=None, password=None):
        """overridden __new__ method"""
        obj=object.__new__(self)
        obj.__init__(url, version, xml, username, password)
        return obj

    def __getitem__(self,id):
        ''' check contents dictionary to allow dict like access to service observational offerings'''
        if name in self.__getattribute__('contents').keys():
            return self.__getattribute__('contents')[id]
        else:
            raise KeyError, "No Observational Offering with id: %s" % id

    def __init__(self, url, version='1.0.0', xml=None, username=None, password=None):
        """Initialize."""
        self.url = url
        self.username = username
        self.password = password
        self.version = version
        self._capabilities = None

        # Authentication handled by Reader
        reader = SosCapabilitiesReader(
                version=self.version, url=self.url, username=self.username, password=self.password
                )
        if xml:  # read from stored xml
            self._capabilities = reader.read_string(xml)
        else:  # read from server
            self._capabilities = reader.read(self.url)

        # Avoid building metadata if the response is an Exception
        if self._capabilities.tag == nspath_eval("ows:ExceptionReport", namespaces):
            raise ows.ExceptionReport(self._capabilities)

        # build metadata objects
        self._build_metadata()

    def getOperationByName(self, name):
        """Return a named content item."""
        for item in self.operations:
            if item.name == name:
                return item
        raise KeyError("No operation named %s" % name)

    def _build_metadata(self):
        """ 
            Set up capabilities metadata objects
        """
        # ows:ServiceIdentification metadata
        service_id_element = self._capabilities.find(nspath_eval('ows:ServiceIdentification', namespaces))
        self.identification = ows.ServiceIdentification(service_id_element)
        
        # ows:ServiceProvider metadata
        service_provider_element = self._capabilities.find(nspath_eval('ows:ServiceProvider', namespaces))
        self.provider = ows.ServiceProvider(service_provider_element)
            
        # ows:OperationsMetadata metadata
        self.operations=[]
        for elem in self._capabilities.findall(nspath_eval('ows:OperationsMetadata/ows:Operation', namespaces)):
            self.operations.append(ows.OperationsMetadata(elem))

        # sos:FilterCapabilities
        filters = self._capabilities.find(nspath_eval('sos:Filter_Capabilities', namespaces))
        if filters is not None:
            self.filters = FilterCapabilities(filters)
        else:
            self.filters = None

        # sos:Contents metadata
        self.contents = {}
        self.offerings = []
        for offering in self._capabilities.findall(nspath_eval('sos:Contents/sos:ObservationOfferingList/sos:ObservationOffering', namespaces)):
            off = SosObservationOffering(offering)
            self.contents[off.id] = off
            self.offerings.append(off)

    def describe_sensor(self,   outputFormat=None,
                                procedure=None,
                                method='Get',
                                **kwargs):

        try:
            base_url = next((m.get('url') for m in self.getOperationByName('DescribeSensor').methods if m.get('type').lower() == method.lower()))
        except StopIteration:
            base_url = self.url
        request = {'service': 'SOS', 'version': self.version, 'request': 'DescribeSensor'}

        # Required Fields
        assert isinstance(outputFormat, str)
        request['outputFormat'] = outputFormat

        assert isinstance(procedure, str)
        request['procedure'] = procedure

        # Optional Fields
        if kwargs:
            for kw in kwargs:
                request[kw]=kwargs[kw]
       
        data = urlencode(request)        

        response = openURL(base_url, data, method, username=self.username, password=self.password).read()
        tr = etree.fromstring(response)

        if tr.tag == nspath_eval("ows:ExceptionReport", namespaces):
            raise ows.ExceptionReport(tr)

        return response

    def get_observation(self,   responseFormat=None,
                                offerings=None,
                                observedProperties=None,
                                eventTime=None,
                                method='Get',
                                **kwargs):
        """
        Parameters
        ----------
        format : string
            Output format. Provide one that is available for all offerings
        method : string
            Optional. HTTP DCP method name: Get or Post.  Must
        **kwargs : extra arguments
            anything else e.g. vendor specific parameters
        """
        try:
            base_url = next((m.get('url') for m in self.getOperationByName('GetObservation').methods if m.get('type').lower() == method.lower()))
        except StopIteration:
            base_url = self.url

        request = {'service': 'SOS', 'version': self.version, 'request': 'GetObservation'}

        # Required Fields
        assert isinstance(offerings, list) and len(offerings) > 0
        request['offering'] = ','.join(offerings)

        assert isinstance(observedProperties, list) and len(observedProperties) > 0
        request['observedProperty'] = ','.join(observedProperties)

        assert isinstance(responseFormat, str)
        request['responseFormat'] = responseFormat


        # Optional Fields
        if eventTime is not None:
            request['eventTime'] = eventTime

        if kwargs:
            for kw in kwargs:
                request[kw]=kwargs[kw]

        data = urlencode(request)        

        response = openURL(base_url, data, method, username=self.username, password=self.password).read()
        try:
            tr = etree.fromstring(response)
            if tr.tag == nspath_eval("ows:ExceptionReport", namespaces):
                raise ows.ExceptionReport(tr)
            else:
                return response                
        except ows.ExceptionReport:
            raise
        except BaseException:
            return response

    def get_operation_by_name(self, name): 
        """
            Return a Operation item by name, case insensitive
        """
        for item in self.operations:
            if item.name.lower() == name.lower():
                return item
        raise KeyError, "No Operation named %s" % name

class SosObservationOffering(object):
    def __init__(self, element):
        self._root = element

        self.id = testXMLValue(self._root.attrib.get(nspath_eval('gml:id', namespaces)), True)
        self.description = testXMLValue(self._root.find(nspath_eval('gml:description', namespaces)))
        self.name = testXMLValue(self._root.find(nspath_eval('gml:name', namespaces)))
        val = testXMLValue(self._root.find(nspath_eval('gml:srsName', namespaces)))
        if val is not None:
            self.srs = Crs(val)

        # LOOK: Check on GML boundedBy to make sure we handle all of the cases
        # gml:boundedBy
        try:
            envelope = self._root.find(nspath_eval('gml:boundedBy/gml:Envelope', namespaces))
            lower_left_corner = testXMLValue(envelope.find(nspath_eval('gml:lowerCorner', namespaces))).split()
            upper_right_corner = testXMLValue(envelope.find(nspath_eval('gml:upperCorner', namespaces))).split()
            # (left, bottom, right, top) in self.bbox_srs units
            self.bbox = (float(lower_left_corner[1]), float(lower_left_corner[0]), float(upper_right_corner[1]), float(upper_right_corner[0]))
            self.bbox_srs = Crs(testXMLValue(envelope.attrib.get('srsName'), True))
        except Exception, err:
            self.bbox = None
            self.bbox_srs = None

        # LOOK: Support all gml:TimeGeometricPrimitivePropertyType
        # Right now we are just supporting gml:TimePeriod
        # sos:Time
        begin_position_element = self._root.find(nspath_eval('sos:time/gml:TimePeriod/gml:beginPosition', namespaces))
        self.begin_position = extract_time(begin_position_element)
        end_position_element = self._root.find(nspath_eval('sos:time/gml:TimePeriod/gml:endPosition', namespaces))
        self.end_position = extract_time(end_position_element)

        self.result_model = testXMLValue(self._root.find(nspath_eval('sos:resultModel', namespaces)))

        self.procedures = []
        for proc in self._root.findall(nspath_eval('sos:procedure', namespaces)):
            self.procedures.append(testXMLValue(proc.attrib.get(nspath_eval('xlink:href', namespaces)), True))

        # LOOK: Support swe:Phenomenon here
        # this includes compound properties
        self.observed_properties = []
        for op in self._root.findall(nspath_eval('sos:observedProperty', namespaces)):
            self.observed_properties.append(testXMLValue(op.attrib.get(nspath_eval('xlink:href', namespaces)), True))

        self.features_of_interest = []
        for fot in self._root.findall(nspath_eval('sos:featureOfInterest', namespaces)):
            self.features_of_interest.append(testXMLValue(fot.attrib.get(nspath_eval('xlink:href', namespaces)), True))

        self.response_formats = []
        for rf in self._root.findall(nspath_eval('sos:responseFormat', namespaces)):
            self.response_formats.append(testXMLValue(rf))

        self.response_modes = []
        for rm in self._root.findall(nspath_eval('sos:responseMode', namespaces)):
            self.response_modes.append(testXMLValue(rm))

    def __str__(self):
        return 'Offering id: %s, name: %s' % (self.id, self.name)
        
class SosCapabilitiesReader(object):
    def __init__(self, version="1.0.0", url=None, username=None, password=None):
        self.version = version
        self.url = url
        self.username = username
        self.password = password

    def capabilities_url(self, service_url):
        """
            Return a capabilities url
        """
        qs = []
        if service_url.find('?') != -1:
            qs = cgi.parse_qsl(service_url.split('?')[1])

        params = [x[0] for x in qs]

        if 'service' not in params:
            qs.append(('service', 'SOS'))
        if 'request' not in params:
            qs.append(('request', 'GetCapabilities'))
        if 'acceptVersions' not in params:
            qs.append(('acceptVersions', self.version))

        urlqs = urlencode(tuple(qs))
        return service_url.split('?')[0] + '?' + urlqs

    def read(self, service_url):
        """
            Get and parse a WMS capabilities document, returning an
            elementtree instance

            service_url is the base url, to which is appended the service,
            acceptVersions, and request parameters
        """
        getcaprequest = self.capabilities_url(service_url)
        spliturl=getcaprequest.split('?')
        u = openURL(spliturl[0], spliturl[1], method='Get', username=self.username, password=self.password)
        return etree.fromstring(u.read())

    def read_string(self, st):
        """
            Parse a SOS capabilities document, returning an elementtree instance

            st should be an XML capabilities document
        """
        if not isinstance(st, str):
            raise ValueError("String must be of type string, not %s" % type(st))
        return etree.fromstring(st)

