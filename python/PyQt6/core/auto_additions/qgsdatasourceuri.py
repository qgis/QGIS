# The following has been generated automatically from src/core/qgsdatasourceuri.h
QgsDataSourceUri.SslPrefer = QgsDataSourceUri.SslMode.SslPrefer
QgsDataSourceUri.SslDisable = QgsDataSourceUri.SslMode.SslDisable
QgsDataSourceUri.SslAllow = QgsDataSourceUri.SslMode.SslAllow
QgsDataSourceUri.SslRequire = QgsDataSourceUri.SslMode.SslRequire
QgsDataSourceUri.SslVerifyCa = QgsDataSourceUri.SslMode.SslVerifyCa
QgsDataSourceUri.SslVerifyFull = QgsDataSourceUri.SslMode.SslVerifyFull
QgsDataSourceUri.SslMode.baseClass = QgsDataSourceUri
try:
    QgsDataSourceUri.removePassword = staticmethod(QgsDataSourceUri.removePassword)
    QgsDataSourceUri.decodeSslMode = staticmethod(QgsDataSourceUri.decodeSslMode)
    QgsDataSourceUri.encodeSslMode = staticmethod(QgsDataSourceUri.encodeSslMode)
except (NameError, AttributeError):
    pass
