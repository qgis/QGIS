# The following has been generated automatically from src/gui/maptools/qgsmaptoolpan.h
try:
    QgsMapToolPan.__attribute_docs__ = {'panDistanceBearingChanged': 'Emitted whenever the distance or bearing of an in-progress panning\noperation is changed.\n\nThis signal will be emitted during a pan operation as the user moves the map,\ngiving the total distance and bearing between the map position at the\nstart of the pan and the current pan position.\n\n.. versionadded:: 3.12\n'}
    QgsMapToolPan.__signal_arguments__ = {'panDistanceBearingChanged': ['distance: float', 'unit: Qgis.DistanceUnit', 'bearing: float']}
    QgsMapToolPan.__group__ = ['maptools']
except (NameError, AttributeError):
    pass
