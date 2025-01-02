# The following has been generated automatically from src/server/qgsserverrequest.h
QgsServerRequest.HeadMethod = QgsServerRequest.Method.HeadMethod
QgsServerRequest.PutMethod = QgsServerRequest.Method.PutMethod
QgsServerRequest.GetMethod = QgsServerRequest.Method.GetMethod
QgsServerRequest.PostMethod = QgsServerRequest.Method.PostMethod
QgsServerRequest.DeleteMethod = QgsServerRequest.Method.DeleteMethod
QgsServerRequest.PatchMethod = QgsServerRequest.Method.PatchMethod
QgsServerRequest.Method.baseClass = QgsServerRequest
QgsServerRequest.HOST = QgsServerRequest.RequestHeader.HOST
QgsServerRequest.FORWARDED = QgsServerRequest.RequestHeader.FORWARDED
QgsServerRequest.X_FORWARDED_FOR = QgsServerRequest.RequestHeader.X_FORWARDED_FOR
QgsServerRequest.X_FORWARDED_HOST = QgsServerRequest.RequestHeader.X_FORWARDED_HOST
QgsServerRequest.X_FORWARDED_PROTO = QgsServerRequest.RequestHeader.X_FORWARDED_PROTO
QgsServerRequest.X_QGIS_SERVICE_URL = QgsServerRequest.RequestHeader.X_QGIS_SERVICE_URL
QgsServerRequest.X_QGIS_WMS_SERVICE_URL = QgsServerRequest.RequestHeader.X_QGIS_WMS_SERVICE_URL
QgsServerRequest.X_QGIS_WFS_SERVICE_URL = QgsServerRequest.RequestHeader.X_QGIS_WFS_SERVICE_URL
QgsServerRequest.X_QGIS_WCS_SERVICE_URL = QgsServerRequest.RequestHeader.X_QGIS_WCS_SERVICE_URL
QgsServerRequest.X_QGIS_WMTS_SERVICE_URL = QgsServerRequest.RequestHeader.X_QGIS_WMTS_SERVICE_URL
QgsServerRequest.ACCEPT = QgsServerRequest.RequestHeader.ACCEPT
QgsServerRequest.USER_AGENT = QgsServerRequest.RequestHeader.USER_AGENT
QgsServerRequest.AUTHORIZATION = QgsServerRequest.RequestHeader.AUTHORIZATION
QgsServerRequest.RequestHeader.baseClass = QgsServerRequest
try:
    QgsServerRequest.methodToString = staticmethod(QgsServerRequest.methodToString)
except (NameError, AttributeError):
    pass
