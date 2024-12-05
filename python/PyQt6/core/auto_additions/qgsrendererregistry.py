# The following has been generated automatically from src/core/symbology/qgsrendererregistry.h
QgsRendererAbstractMetadata.PointLayer = QgsRendererAbstractMetadata.LayerType.PointLayer
QgsRendererAbstractMetadata.LineLayer = QgsRendererAbstractMetadata.LayerType.LineLayer
QgsRendererAbstractMetadata.PolygonLayer = QgsRendererAbstractMetadata.LayerType.PolygonLayer
QgsRendererAbstractMetadata.All = QgsRendererAbstractMetadata.LayerType.All
QgsRendererAbstractMetadata.LayerTypes = lambda flags=0: QgsRendererAbstractMetadata.LayerType(flags)
from enum import Enum


def _force_int(v): return int(v.value) if isinstance(v, Enum) else v


QgsRendererAbstractMetadata.LayerType.__bool__ = lambda flag: bool(_force_int(flag))
QgsRendererAbstractMetadata.LayerType.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsRendererAbstractMetadata.LayerType.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsRendererAbstractMetadata.LayerType.__or__ = lambda flag1, flag2: QgsRendererAbstractMetadata.LayerType(_force_int(flag1) | _force_int(flag2))
try:
    QgsRendererAbstractMetadata.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsRendererMetadata.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsRendererRegistry.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
