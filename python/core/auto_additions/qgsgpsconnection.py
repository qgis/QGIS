# The following has been generated automatically from src/core/gps/qgsgpsconnection.h
try:
    QgsGpsConnection.__attribute_docs__ = {'stateChanged': 'Emitted whenever the GPS state is changed.\n', 'nmeaSentenceReceived': 'Emitted whenever the GPS device receives a raw NMEA sentence.\n', 'fixStatusChanged': 'Emitted when the GPS device fix status is changed.\n\n.. versionadded:: 3.30\n', 'positionChanged': 'Emitted when the GPS position changes.\n\nThis signal is only emitted when the new GPS location is considered\nvalid (see :py:func:`QgsGpsInformation.isValid()`).\n\n.. versionadded:: 3.30\n'}
    QgsGpsConnection.__abstract_methods__ = ['parseData']
    QgsGpsConnection.__signal_arguments__ = {'stateChanged': ['info: QgsGpsInformation'], 'nmeaSentenceReceived': ['substring: str'], 'fixStatusChanged': ['status: Qgis.GpsFixStatus'], 'positionChanged': ['point: QgsPoint']}
    import functools as _functools
    __wrapped_QgsGpsConnection_QgsGpsConnection = QgsGpsConnection.QgsGpsConnection
    def __QgsGpsConnection_QgsGpsConnection_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsGpsConnection_QgsGpsConnection(self, arg)
    QgsGpsConnection.QgsGpsConnection = _functools.update_wrapper(__QgsGpsConnection_QgsGpsConnection_wrapper, QgsGpsConnection.QgsGpsConnection)

    import functools as _functools
    __wrapped_QgsGpsConnection_setSource = QgsGpsConnection.setSource
    def __QgsGpsConnection_setSource_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsGpsConnection_setSource(self, arg)
    QgsGpsConnection.setSource = _functools.update_wrapper(__QgsGpsConnection_setSource_wrapper, QgsGpsConnection.setSource)

    QgsGpsConnection.__group__ = ['gps']
except (NameError, AttributeError):
    pass
