# The following has been generated automatically from src/server/qgsserverogcapi.h
QgsServerOgcApi.alternate = QgsServerOgcApi.Rel.alternate
QgsServerOgcApi.describedBy = QgsServerOgcApi.Rel.describedBy
QgsServerOgcApi.collection = QgsServerOgcApi.Rel.collection
QgsServerOgcApi.item = QgsServerOgcApi.Rel.item
QgsServerOgcApi.self = QgsServerOgcApi.Rel.self
QgsServerOgcApi.service_desc = QgsServerOgcApi.Rel.service_desc
QgsServerOgcApi.service_doc = QgsServerOgcApi.Rel.service_doc
QgsServerOgcApi.prev = QgsServerOgcApi.Rel.prev
QgsServerOgcApi.next = QgsServerOgcApi.Rel.next
QgsServerOgcApi.license = QgsServerOgcApi.Rel.license
QgsServerOgcApi.items = QgsServerOgcApi.Rel.items
QgsServerOgcApi.conformance = QgsServerOgcApi.Rel.conformance
QgsServerOgcApi.data = QgsServerOgcApi.Rel.data
QgsServerOgcApi.schema = QgsServerOgcApi.Rel.schema
QgsServerOgcApi.profile = QgsServerOgcApi.Rel.profile
QgsServerOgcApi.Rel.baseClass = QgsServerOgcApi
QgsServerOgcApi.GEOJSON = QgsServerOgcApi.ContentType.GEOJSON
QgsServerOgcApi.OPENAPI3 = QgsServerOgcApi.ContentType.OPENAPI3
QgsServerOgcApi.JSON = QgsServerOgcApi.ContentType.JSON
QgsServerOgcApi.SCHEMA_JSON = QgsServerOgcApi.ContentType.SCHEMA_JSON
QgsServerOgcApi.HTML = QgsServerOgcApi.ContentType.HTML
QgsServerOgcApi.XML = QgsServerOgcApi.ContentType.XML
QgsServerOgcApi.FLATGEOBUF = QgsServerOgcApi.ContentType.FLATGEOBUF
QgsServerOgcApi.ContentType.baseClass = QgsServerOgcApi
# monkey patching scoped based enum
QgsServerOgcApi.Profile.NONE.__doc__ = "No profile"
QgsServerOgcApi.Profile.RFC7946.__doc__ = "GeoJSON profile according to RFC7946"
QgsServerOgcApi.Profile.REL_AS_LINK.__doc__ = "JSON responses that include links for referenced resources http://www.opengis.net/def/profile/ogc/0/rel-as-link"
QgsServerOgcApi.Profile.REL_AS_KEY.__doc__ = "JSON responses that include key for referenced resources http://www.opengis.net/def/profile/ogc/0/rel-as-key"
QgsServerOgcApi.Profile.REL_AS_URI.__doc__ = "JSON responses that include URI for referenced resources http://www.opengis.net/def/profile/ogc/0/rel-as-uri"
QgsServerOgcApi.Profile.__doc__ = """JSON profile

* ``NONE``: No profile
* ``RFC7946``: GeoJSON profile according to RFC7946
* ``REL_AS_LINK``: JSON responses that include links for referenced resources http://www.opengis.net/def/profile/ogc/0/rel-as-link
* ``REL_AS_KEY``: JSON responses that include key for referenced resources http://www.opengis.net/def/profile/ogc/0/rel-as-key
* ``REL_AS_URI``: JSON responses that include URI for referenced resources http://www.opengis.net/def/profile/ogc/0/rel-as-uri

"""
# --
QgsServerOgcApi.Profile.baseClass = QgsServerOgcApi
try:
    QgsServerOgcApi.sanitizeUrl = staticmethod(QgsServerOgcApi.sanitizeUrl)
    QgsServerOgcApi.relToString = staticmethod(QgsServerOgcApi.relToString)
    QgsServerOgcApi.contentTypeToString = staticmethod(QgsServerOgcApi.contentTypeToString)
    QgsServerOgcApi.contentTypeToStdString = staticmethod(QgsServerOgcApi.contentTypeToStdString)
    QgsServerOgcApi.contentTypeToExtension = staticmethod(QgsServerOgcApi.contentTypeToExtension)
    QgsServerOgcApi.contenTypeFromExtension = staticmethod(QgsServerOgcApi.contenTypeFromExtension)
    QgsServerOgcApi.contentTypeFromExtension = staticmethod(QgsServerOgcApi.contentTypeFromExtension)
    QgsServerOgcApi.mimeType = staticmethod(QgsServerOgcApi.mimeType)
    QgsServerOgcApi.__overridden_methods__ = ['name', 'description', 'version', 'rootPath', 'executeRequest']
except (NameError, AttributeError):
    pass
