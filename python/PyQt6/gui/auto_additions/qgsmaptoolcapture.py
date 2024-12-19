# The following has been generated automatically from src/gui/maptools/qgsmaptoolcapture.h
QgsMapToolCapture.CaptureNone = QgsMapToolCapture.CaptureMode.CaptureNone
QgsMapToolCapture.CapturePoint = QgsMapToolCapture.CaptureMode.CapturePoint
QgsMapToolCapture.CaptureLine = QgsMapToolCapture.CaptureMode.CaptureLine
QgsMapToolCapture.CapturePolygon = QgsMapToolCapture.CaptureMode.CapturePolygon
QgsMapToolCapture.NoCapabilities = QgsMapToolCapture.Capability.NoCapabilities
QgsMapToolCapture.SupportsCurves = QgsMapToolCapture.Capability.SupportsCurves
QgsMapToolCapture.ValidateGeometries = QgsMapToolCapture.Capability.ValidateGeometries
QgsMapToolCapture.Capabilities = lambda flags=0: QgsMapToolCapture.Capability(flags)
from enum import Enum


def _force_int(v): return int(v.value) if isinstance(v, Enum) else v


QgsMapToolCapture.Capability.__bool__ = lambda flag: bool(_force_int(flag))
QgsMapToolCapture.Capability.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsMapToolCapture.Capability.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsMapToolCapture.Capability.__or__ = lambda flag1, flag2: QgsMapToolCapture.Capability(_force_int(flag1) | _force_int(flag2))
try:
    QgsMapToolCapture.__group__ = ['maptools']
except (NameError, AttributeError):
    pass
