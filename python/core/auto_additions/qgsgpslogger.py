# The following has been generated automatically from src/core/gps/qgsgpslogger.h
try:
    QgsGpsLogger.__attribute_docs__ = {'trackIsEmptyChanged': 'Emitted whenever the current track changes from being empty to non-empty or vice versa.\n', 'trackReset': 'Emitted whenever the current track is reset.\n', 'trackVertexAdded': 'Emitted whenever a new vertex is added to the track.\n\nThe ``vertex`` point will be in WGS84 coordinate reference system.\n', 'stateChanged': 'Emitted whenever the associated GPS device state is changed.\n', 'distanceAreaChanged': 'Emitted whenever the distance area used to calculate track distances is changed.\n'}
    QgsGpsLogger.__signal_arguments__ = {'trackIsEmptyChanged': ['isEmpty: bool'], 'trackVertexAdded': ['vertex: QgsPoint'], 'stateChanged': ['info: QgsGpsInformation']}
    QgsGpsLogger.__group__ = ['gps']
except (NameError, AttributeError):
    pass
