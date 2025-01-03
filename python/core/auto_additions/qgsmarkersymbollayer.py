# The following has been generated automatically from src/core/symbology/qgsmarkersymbollayer.h
# monkey patching scoped based enum
QgsFontMarkerSymbolLayer.VerticalAnchorMode.Bounds.__doc__ = "Calculate anchor points according to character bounds"
QgsFontMarkerSymbolLayer.VerticalAnchorMode.Baseline.__doc__ = "Calculate anchor points with fix baseline"
QgsFontMarkerSymbolLayer.VerticalAnchorMode.Legacy.__doc__ = "Calculate anchor points with different offsets"
QgsFontMarkerSymbolLayer.VerticalAnchorMode.__doc__ = """Vertical anchor modes

* ``Bounds``: Calculate anchor points according to character bounds
* ``Baseline``: Calculate anchor points with fix baseline
* ``Legacy``: Calculate anchor points with different offsets

"""
# --
try:
    QgsSimpleMarkerSymbolLayerBase.availableShapes = staticmethod(QgsSimpleMarkerSymbolLayerBase.availableShapes)
    QgsSimpleMarkerSymbolLayerBase.shapeIsFilled = staticmethod(QgsSimpleMarkerSymbolLayerBase.shapeIsFilled)
    QgsSimpleMarkerSymbolLayerBase.decodeShape = staticmethod(QgsSimpleMarkerSymbolLayerBase.decodeShape)
    QgsSimpleMarkerSymbolLayerBase.encodeShape = staticmethod(QgsSimpleMarkerSymbolLayerBase.encodeShape)
    QgsSimpleMarkerSymbolLayerBase.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsSimpleMarkerSymbolLayer.create = staticmethod(QgsSimpleMarkerSymbolLayer.create)
    QgsSimpleMarkerSymbolLayer.createFromSld = staticmethod(QgsSimpleMarkerSymbolLayer.createFromSld)
    QgsSimpleMarkerSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsFilledMarkerSymbolLayer.create = staticmethod(QgsFilledMarkerSymbolLayer.create)
    QgsFilledMarkerSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsSvgMarkerSymbolLayer.create = staticmethod(QgsSvgMarkerSymbolLayer.create)
    QgsSvgMarkerSymbolLayer.createFromSld = staticmethod(QgsSvgMarkerSymbolLayer.createFromSld)
    QgsSvgMarkerSymbolLayer.resolvePaths = staticmethod(QgsSvgMarkerSymbolLayer.resolvePaths)
    QgsSvgMarkerSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsRasterMarkerSymbolLayer.create = staticmethod(QgsRasterMarkerSymbolLayer.create)
    QgsRasterMarkerSymbolLayer.resolvePaths = staticmethod(QgsRasterMarkerSymbolLayer.resolvePaths)
    QgsRasterMarkerSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsFontMarkerSymbolLayer.create = staticmethod(QgsFontMarkerSymbolLayer.create)
    QgsFontMarkerSymbolLayer.createFromSld = staticmethod(QgsFontMarkerSymbolLayer.createFromSld)
    QgsFontMarkerSymbolLayer.resolveFonts = staticmethod(QgsFontMarkerSymbolLayer.resolveFonts)
    QgsFontMarkerSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsAnimatedMarkerSymbolLayer.create = staticmethod(QgsAnimatedMarkerSymbolLayer.create)
    QgsAnimatedMarkerSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
