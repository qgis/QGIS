# The following has been generated automatically from src/server/qgsserverresponse.h
try:
    QgsServerResponse.__virtual_methods__ = ['write', 'finish', 'flush', 'feedback']
    QgsServerResponse.__abstract_methods__ = ['setHeader', 'addHeader', 'removeHeader', 'header', 'fullHeader', 'headers', 'fullHeaders', 'headersSent', 'setStatusCode', 'statusCode', 'sendError', 'io', 'clear', 'data', 'truncate']
except (NameError, AttributeError):
    pass
