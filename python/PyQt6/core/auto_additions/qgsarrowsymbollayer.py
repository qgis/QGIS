# The following has been generated automatically from src/core/symbology/qgsarrowsymbollayer.h
QgsArrowSymbolLayer.HeadSingle = QgsArrowSymbolLayer.HeadType.HeadSingle
QgsArrowSymbolLayer.HeadReversed = QgsArrowSymbolLayer.HeadType.HeadReversed
QgsArrowSymbolLayer.HeadDouble = QgsArrowSymbolLayer.HeadType.HeadDouble
QgsArrowSymbolLayer.ArrowPlain = QgsArrowSymbolLayer.ArrowType.ArrowPlain
QgsArrowSymbolLayer.ArrowLeftHalf = QgsArrowSymbolLayer.ArrowType.ArrowLeftHalf
QgsArrowSymbolLayer.ArrowRightHalf = QgsArrowSymbolLayer.ArrowType.ArrowRightHalf
try:
    QgsArrowSymbolLayer.create = staticmethod(QgsArrowSymbolLayer.create)
    QgsArrowSymbolLayer.__overridden_methods__ = ['clone', 'subSymbol', 'setSubSymbol', 'usedAttributes', 'hasDataDefinedProperties', 'usesMapUnits', 'setOutputUnit', 'properties', 'layerType', 'startRender', 'stopRender', 'startFeatureRender', 'stopFeatureRender', 'renderPolyline', 'setColor', 'color', 'canCauseArtifactsBetweenAdjacentTiles']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsArrowSymbolLayer_setSubSymbol = QgsArrowSymbolLayer.setSubSymbol
    def __QgsArrowSymbolLayer_setSubSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsArrowSymbolLayer_setSubSymbol(self, arg)
    QgsArrowSymbolLayer.setSubSymbol = _functools.update_wrapper(__QgsArrowSymbolLayer_setSubSymbol_wrapper, QgsArrowSymbolLayer.setSubSymbol)

    QgsArrowSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
