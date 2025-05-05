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
    QgsRendererAbstractMetadata.__virtual_methods__ = ['compatibleLayerTypes', 'createRendererWidget', 'createRendererFromSld']
    QgsRendererAbstractMetadata.__abstract_methods__ = ['createRenderer']
    QgsRendererAbstractMetadata.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsRendererMetadata.__overridden_methods__ = ['createRenderer', 'createRendererWidget', 'createRendererFromSld', 'compatibleLayerTypes']
    QgsRendererMetadata.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsRendererRegistry_addRenderer = QgsRendererRegistry.addRenderer
    def __QgsRendererRegistry_addRenderer_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsRendererRegistry_addRenderer(self, arg)
    QgsRendererRegistry.addRenderer = _functools.update_wrapper(__QgsRendererRegistry_addRenderer_wrapper, QgsRendererRegistry.addRenderer)

    QgsRendererRegistry.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
