
# Follows the 4 aspects of service metadata

class IServiceIdentificationMetadata:
    """OO-interface to service identification metadata.
    """

    type = property("""Service name (string): "WMS", "WFS", "WCS", or "SOS".""")
    version = property("""Version of service protocol (string).""")
    title = property("""Human-targeted title of service (string).""")
    abstract = property("""Text describing the service (string).""")
    keywords = property("""Keyword list (list).""")
    accessconstraints = property("""Explanation of access constraints associated with service (string).""")
    fees = property("""Explanation of fees associated with service (string).""")
    

class IServiceProviderMetadata:
    """OO-interface to service provider metadata.
    """

    name = property("""Provider's name (string).""")
    url = property("""URL for provider's web site (string).""")
    contact = property("""How to contact the service provider (string).""")


class IServiceOperations:
    """OO-interface to service operations metadata.
    """

    operations = property("""List of operation descriptors (list). These must implement IOperationMetadata (below).""")
    exceptions = property("""List of exception formats (list).""")


class IServiceContents:
    """OO-interface to service contents metadata.
    """

    contents = property("""List of content descriptors (list). These must implement IServiceContent (below).""")


# IServiceMetadata aggregates the 4 aspects above

class IServiceMetadata(IServiceOperations, IServiceContents):
    """OWS Metadata.

    operations and contents properties are inherited.
    """

    identification = property("""Object that implements IServiceIdentificationMetadata.""")
    provider = property("""Object that implements IServiceProviderMetadata.""")


# A Service has an online resource URL as well as metadata collections

class IService(IServiceMetadata):
    """The OGC Web Service interface.
    """

    url = property("""Online resource URL (string)""")


# 3 specific service types are described below: WMS, WFS, and WCS

class IWebMapService(IService):
    """Abstraction for an OGC Web Map Service (WMS).
    """

    def getcapabilities():
        """Make a request to the WMS, returns an XML document wrapped in a 
        Python file object."""

    def getmap(**kw):
        """Make a request to the WMS, returns an image wrapped in a Python
        file object."""

    def getfeatureinfo(**kw):
        """Make a request to the WMS, returns data."""


class IWebFeatureService(IService):
    """Abstraction for an OGC Web Feature Service (WFS).
    """

    def getcapabilities():
        """Make a request to the WFS, returns an XML document wrapped in a 
        Python file object."""

    def getfeature(**kw):
        """Make a request to the WFS, returns an XML document wrapped in a
        Python file object."""

    def describefeaturetype(**kw):
        """Make a request to the WFS, returns data."""


class IWebCoverageService(IService):
    # TODO
    pass

class ISensorObservationService(IService):
    """Abstraction for an OGC Sensor Observation Service (SOS).
    """

    def getcapabilities():
        """Make a request to the SOS, returns an XML document wrapped in a 
        Python file object."""

    def describesensor():
        """Make a request to the SOS, returns an XML document wrapped in a 
        Python file object."""

    def getobservation():
        """Make a request to the SOS, returns an XML document wrapped in a 
        Python file object."""

# Second level metadata interfaces

class IOperationMetadata:
    """OO-interface to operation metadata.
    """

    name = property("""Operation name (string): GetCapabilities", for example.""")
    formatOptions = property("""List of content types (list).""")
    methods = property("""Mapping of method descriptors, keyed to HTTP verbs. Items must implement IMethodMetadata (below).""")


class IMethodMetadata:
    """OO-interface to method metadata.
    """

    url = property("""Method endpoint URL (string).""")
    # TODO: constraint

class IContentMetadata:
    """OO-interface to content metadata.
    """

    id = property("""Unique identifier (string).""")
    title = property("""Human-targeted title (string).""")
    boundingBox = property("""Four bounding values and a coordinate reference system identifier (tuple).""")
    boundingBoxWGS84 = property("""Four bounding values in WGS coordinates.""")
    crsOptions = property("""List of available coordinate/spatial reference systems (list).""")
    styles = property("""List of style dicts (list).""")
    timepositions=property("""List of times for which data is available""")
    defaulttimeposition=property("""Default time position""")

class iSensorObservationServiceContentMetadata(IContentMetadata):
    """Extension class for SOS specifics"""
    pass

# XXX: needed?

class IContactMetadata:
    """OO-interface to OWS metadata.

    Properties
    ----------
    name : string
    organization : string
    address : string
    city : string
    region : string
    postcode : string
    country : string
    email : string
    hoursofservice: string
    role: string
    """
