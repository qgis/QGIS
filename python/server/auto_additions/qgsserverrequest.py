# The following has been generated automatically from src/server/qgsserverrequest.h
QgsServerRequest.Method.baseClass = QgsServerRequest
QgsServerRequest.RequestHeader.baseClass = QgsServerRequest
try:
    QgsServerRequest.methodToString = staticmethod(QgsServerRequest.methodToString)
    QgsServerRequest.__virtual_methods__ = ['setParameter', 'removeParameter', 'header', 'data', 'setUrl']
except (NameError, AttributeError):
    pass
