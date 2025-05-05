# The following has been generated automatically from src/core/gps/qgsnmeaconnection.h
try:
    QgsNmeaConnection.__overridden_methods__ = ['parseData']
    import functools as _functools
    __wrapped_QgsNmeaConnection_QgsNmeaConnection = QgsNmeaConnection.QgsNmeaConnection
    def __QgsNmeaConnection_QgsNmeaConnection_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsNmeaConnection_QgsNmeaConnection(self, arg)
    QgsNmeaConnection.QgsNmeaConnection = _functools.update_wrapper(__QgsNmeaConnection_QgsNmeaConnection_wrapper, QgsNmeaConnection.QgsNmeaConnection)

    QgsNmeaConnection.__group__ = ['gps']
except (NameError, AttributeError):
    pass
