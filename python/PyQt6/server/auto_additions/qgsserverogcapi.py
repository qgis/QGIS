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
QgsServerOgcApi.Rel.baseClass = QgsServerOgcApi
QgsServerOgcApi.GEOJSON = QgsServerOgcApi.ContentType.GEOJSON
QgsServerOgcApi.OPENAPI3 = QgsServerOgcApi.ContentType.OPENAPI3
QgsServerOgcApi.JSON = QgsServerOgcApi.ContentType.JSON
QgsServerOgcApi.HTML = QgsServerOgcApi.ContentType.HTML
QgsServerOgcApi.XML = QgsServerOgcApi.ContentType.XML
QgsServerOgcApi.ContentType.baseClass = QgsServerOgcApi
try:
    QgsServerOgcApi.sanitizeUrl = staticmethod(QgsServerOgcApi.sanitizeUrl)
    QgsServerOgcApi.relToString = staticmethod(QgsServerOgcApi.relToString)
    QgsServerOgcApi.contentTypeToString = staticmethod(QgsServerOgcApi.contentTypeToString)
    QgsServerOgcApi.contentTypeToStdString = staticmethod(QgsServerOgcApi.contentTypeToStdString)
    QgsServerOgcApi.contentTypeToExtension = staticmethod(QgsServerOgcApi.contentTypeToExtension)
    QgsServerOgcApi.contenTypeFromExtension = staticmethod(QgsServerOgcApi.contenTypeFromExtension)
    QgsServerOgcApi.contentTypeFromExtension = staticmethod(QgsServerOgcApi.contentTypeFromExtension)
    QgsServerOgcApi.mimeType = staticmethod(QgsServerOgcApi.mimeType)
except (NameError, AttributeError):
    pass
