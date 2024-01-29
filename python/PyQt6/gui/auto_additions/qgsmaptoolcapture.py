# The following has been generated automatically from src/gui/qgsmaptoolcapture.h
QgsMapToolCapture.CaptureNone = QgsMapToolCapture.CaptureMode.CaptureNone
QgsMapToolCapture.CapturePoint = QgsMapToolCapture.CaptureMode.CapturePoint
QgsMapToolCapture.CaptureLine = QgsMapToolCapture.CaptureMode.CaptureLine
QgsMapToolCapture.CapturePolygon = QgsMapToolCapture.CaptureMode.CapturePolygon
QgsMapToolCapture.NoCapabilities = QgsMapToolCapture.Capability.NoCapabilities
QgsMapToolCapture.SupportsCurves = QgsMapToolCapture.Capability.SupportsCurves
QgsMapToolCapture.ValidateGeometries = QgsMapToolCapture.Capability.ValidateGeometries
QgsMapToolCapture.Capabilities = lambda flags=0: QgsMapToolCapture.Capability(flags)
def _force_int(v): return v if isinstance(v, int) else int(v.value)


QgsMapToolCapture.Capability.__bool__ = lambda flag: bool(_force_int(flag))
QgsMapToolCapture.Capability.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsMapToolCapture.Capability.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsMapToolCapture.Capability.__or__ = lambda flag1, flag2: QgsMapToolCapture.Capability(_force_int(flag1) | _force_int(flag2))
