# The following has been generated automatically from src/core/stac/qgsstacconnection.h
try:
    QgsStacConnection.Data.__attribute_docs__ = {'url': 'Catalog URL', 'authCfg': 'Authentication configuration id', 'username': 'HTTP Basic username', 'password': 'HTTP Basic password', 'httpHeaders': 'HTTP headers'}
    QgsStacConnection.Data.__annotations__ = {'url': str, 'authCfg': str, 'username': str, 'password': str, 'httpHeaders': 'QgsHttpHeaders'}
    QgsStacConnection.Data.__doc__ = """Represents decoded data of a connection"""
    QgsStacConnection.Data.__group__ = ['stac']
except (NameError, AttributeError):
    pass
try:
    QgsStacConnection.encodedUri = staticmethod(QgsStacConnection.encodedUri)
    QgsStacConnection.decodedUri = staticmethod(QgsStacConnection.decodedUri)
    QgsStacConnection.connectionList = staticmethod(QgsStacConnection.connectionList)
    QgsStacConnection.connection = staticmethod(QgsStacConnection.connection)
    QgsStacConnection.deleteConnection = staticmethod(QgsStacConnection.deleteConnection)
    QgsStacConnection.addConnection = staticmethod(QgsStacConnection.addConnection)
    QgsStacConnection.selectedConnection = staticmethod(QgsStacConnection.selectedConnection)
    QgsStacConnection.setSelectedConnection = staticmethod(QgsStacConnection.setSelectedConnection)
    QgsStacConnection.__overridden_methods__ = ['store', 'remove']
    QgsStacConnection.__group__ = ['stac']
except (NameError, AttributeError):
    pass
