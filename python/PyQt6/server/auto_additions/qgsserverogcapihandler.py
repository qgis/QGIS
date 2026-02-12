# The following has been generated automatically from src/server/qgsserverogcapihandler.h
try:
    QgsServerOgcApiHandler.parentLink = staticmethod(QgsServerOgcApiHandler.parentLink)
    QgsServerOgcApiHandler.layerFromCollectionId = staticmethod(QgsServerOgcApiHandler.layerFromCollectionId)
    QgsServerOgcApiHandler.__virtual_methods__ = ['parameters', 'tags', 'defaultContentType', 'handleRequest', 'values']
    QgsServerOgcApiHandler.__abstract_methods__ = ['path', 'operationId', 'summary', 'description', 'linkTitle', 'linkType']
except (NameError, AttributeError):
    pass
