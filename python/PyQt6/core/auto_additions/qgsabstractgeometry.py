# The following has been generated automatically from src/core/geometry/qgsabstractgeometry.h
QgsAbstractGeometry.MaximumAngle = QgsAbstractGeometry.SegmentationToleranceType.MaximumAngle
QgsAbstractGeometry.MaximumDifference = QgsAbstractGeometry.SegmentationToleranceType.MaximumDifference
QgsAbstractGeometry.SegmentationToleranceType.baseClass = QgsAbstractGeometry
QgsAbstractGeometry.XY = QgsAbstractGeometry.AxisOrder.XY
QgsAbstractGeometry.YX = QgsAbstractGeometry.AxisOrder.YX
QgsAbstractGeometry.FlagExportTrianglesAsPolygons = QgsAbstractGeometry.WkbFlag.FlagExportTrianglesAsPolygons
QgsAbstractGeometry.FlagExportNanAsDoubleMin = QgsAbstractGeometry.WkbFlag.FlagExportNanAsDoubleMin
QgsAbstractGeometry.WkbFlags = lambda flags=0: QgsAbstractGeometry.WkbFlag(flags)
from enum import Enum


def _force_int(v): return int(v.value) if isinstance(v, Enum) else v


QgsAbstractGeometry.WkbFlag.__bool__ = lambda flag: bool(_force_int(flag))
QgsAbstractGeometry.WkbFlag.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsAbstractGeometry.WkbFlag.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsAbstractGeometry.WkbFlag.__or__ = lambda flag1, flag2: QgsAbstractGeometry.WkbFlag(_force_int(flag1) | _force_int(flag2))
try:
    QgsAbstractGeometry.__group__ = ['geometry']
except (NameError, AttributeError):
    pass
try:
    QgsVertexIterator.__group__ = ['geometry']
except (NameError, AttributeError):
    pass
try:
    QgsGeometryPartIterator.__group__ = ['geometry']
except (NameError, AttributeError):
    pass
try:
    QgsGeometryConstPartIterator.__group__ = ['geometry']
except (NameError, AttributeError):
    pass
