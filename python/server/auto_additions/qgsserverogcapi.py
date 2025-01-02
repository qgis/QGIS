# The following has been generated automatically from src/server/qgsserverogcapi.h
QgsServerOgcApi.Rel.baseClass = QgsServerOgcApi
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
