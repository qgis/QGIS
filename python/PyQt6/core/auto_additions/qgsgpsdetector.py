# The following has been generated automatically from src/core/gps/qgsgpsdetector.h
try:
    QgsGpsDetector.__attribute_docs__ = {'connectionDetected': 'Emitted when a GPS connection is successfully detected.\n\nCall :py:func:`~QgsGpsDetector.takeConnection` to take ownership of the detected connection.\n\n.. versionadded:: 3.38\n', 'detected': 'Emitted when the GPS connection has been detected. A single connection must listen for this signal and\nimmediately take ownership of the ``connection`` object.\n\n.. deprecated:: 3.40\n\n   This signal is dangerous and extremely unsafe! It is recommended to instead set the ``useUnsafeSignals`` parameter to ``False`` in the QgsGpsDetector constructor and use the safe :py:func:`~QgsGpsDetector.connectionDetected` signal instead.\n', 'detectionFailed': 'Emitted when the detector could not find a valid GPS connection.\n'}
    QgsGpsDetector.availablePorts = staticmethod(QgsGpsDetector.availablePorts)
    QgsGpsDetector.__group__ = ['gps']
except (NameError, AttributeError):
    pass
