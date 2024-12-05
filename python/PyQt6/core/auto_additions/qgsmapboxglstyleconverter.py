# The following has been generated automatically from src/core/vectortile/qgsmapboxglstyleconverter.h
QgsMapBoxGlStyleConverter.Success = QgsMapBoxGlStyleConverter.Result.Success
QgsMapBoxGlStyleConverter.NoLayerList = QgsMapBoxGlStyleConverter.Result.NoLayerList
# monkey patching scoped based enum
QgsMapBoxGlStyleConverter.PropertyType.Color.__doc__ = "Color property"
QgsMapBoxGlStyleConverter.PropertyType.Numeric.__doc__ = "Numeric property (e.g. line width, text size)"
QgsMapBoxGlStyleConverter.PropertyType.Opacity.__doc__ = "Opacity property"
QgsMapBoxGlStyleConverter.PropertyType.Point.__doc__ = "Point/offset property"
QgsMapBoxGlStyleConverter.PropertyType.NumericArray.__doc__ = "Numeric array for dash arrays or such"
QgsMapBoxGlStyleConverter.PropertyType.__doc__ = """Property types, for interpolated value conversion

.. warning::

   This is private API only, and may change in future QGIS versions

* ``Color``: Color property
* ``Numeric``: Numeric property (e.g. line width, text size)
* ``Opacity``: Opacity property
* ``Point``: Point/offset property
* ``NumericArray``: Numeric array for dash arrays or such

"""
# --
QgsMapBoxGlStyleConverter.PropertyType.baseClass = QgsMapBoxGlStyleConverter
try:
    QgsMapBoxGlStyleConverter.parseFillLayer = staticmethod(QgsMapBoxGlStyleConverter.parseFillLayer)
    QgsMapBoxGlStyleConverter.parseLineLayer = staticmethod(QgsMapBoxGlStyleConverter.parseLineLayer)
    QgsMapBoxGlStyleConverter.parseCircleLayer = staticmethod(QgsMapBoxGlStyleConverter.parseCircleLayer)
    QgsMapBoxGlStyleConverter.parseSymbolLayer = staticmethod(QgsMapBoxGlStyleConverter.parseSymbolLayer)
    QgsMapBoxGlStyleConverter.parseSymbolLayerAsRenderer = staticmethod(QgsMapBoxGlStyleConverter.parseSymbolLayerAsRenderer)
    QgsMapBoxGlStyleConverter.parseInterpolateColorByZoom = staticmethod(QgsMapBoxGlStyleConverter.parseInterpolateColorByZoom)
    QgsMapBoxGlStyleConverter.parseInterpolateByZoom = staticmethod(QgsMapBoxGlStyleConverter.parseInterpolateByZoom)
    QgsMapBoxGlStyleConverter.parseInterpolateOpacityByZoom = staticmethod(QgsMapBoxGlStyleConverter.parseInterpolateOpacityByZoom)
    QgsMapBoxGlStyleConverter.parseOpacityStops = staticmethod(QgsMapBoxGlStyleConverter.parseOpacityStops)
    QgsMapBoxGlStyleConverter.parseInterpolatePointByZoom = staticmethod(QgsMapBoxGlStyleConverter.parseInterpolatePointByZoom)
    QgsMapBoxGlStyleConverter.parseInterpolateStringByZoom = staticmethod(QgsMapBoxGlStyleConverter.parseInterpolateStringByZoom)
    QgsMapBoxGlStyleConverter.parsePointStops = staticmethod(QgsMapBoxGlStyleConverter.parsePointStops)
    QgsMapBoxGlStyleConverter.parseArrayStops = staticmethod(QgsMapBoxGlStyleConverter.parseArrayStops)
    QgsMapBoxGlStyleConverter.parseStops = staticmethod(QgsMapBoxGlStyleConverter.parseStops)
    QgsMapBoxGlStyleConverter.parseStringStops = staticmethod(QgsMapBoxGlStyleConverter.parseStringStops)
    QgsMapBoxGlStyleConverter.parseLabelStops = staticmethod(QgsMapBoxGlStyleConverter.parseLabelStops)
    QgsMapBoxGlStyleConverter.parseValueList = staticmethod(QgsMapBoxGlStyleConverter.parseValueList)
    QgsMapBoxGlStyleConverter.parseMatchList = staticmethod(QgsMapBoxGlStyleConverter.parseMatchList)
    QgsMapBoxGlStyleConverter.parseStepList = staticmethod(QgsMapBoxGlStyleConverter.parseStepList)
    QgsMapBoxGlStyleConverter.parseInterpolateListByZoom = staticmethod(QgsMapBoxGlStyleConverter.parseInterpolateListByZoom)
    QgsMapBoxGlStyleConverter.parseColorExpression = staticmethod(QgsMapBoxGlStyleConverter.parseColorExpression)
    QgsMapBoxGlStyleConverter.parseColor = staticmethod(QgsMapBoxGlStyleConverter.parseColor)
    QgsMapBoxGlStyleConverter.colorAsHslaComponents = staticmethod(QgsMapBoxGlStyleConverter.colorAsHslaComponents)
    QgsMapBoxGlStyleConverter.interpolateExpression = staticmethod(QgsMapBoxGlStyleConverter.interpolateExpression)
    QgsMapBoxGlStyleConverter.parseCapStyle = staticmethod(QgsMapBoxGlStyleConverter.parseCapStyle)
    QgsMapBoxGlStyleConverter.parseJoinStyle = staticmethod(QgsMapBoxGlStyleConverter.parseJoinStyle)
    QgsMapBoxGlStyleConverter.parseExpression = staticmethod(QgsMapBoxGlStyleConverter.parseExpression)
    QgsMapBoxGlStyleConverter.retrieveSprite = staticmethod(QgsMapBoxGlStyleConverter.retrieveSprite)
    QgsMapBoxGlStyleConverter.retrieveSpriteAsBase64 = staticmethod(QgsMapBoxGlStyleConverter.retrieveSpriteAsBase64)
    QgsMapBoxGlStyleConverter.retrieveSpriteAsBase64WithProperties = staticmethod(QgsMapBoxGlStyleConverter.retrieveSpriteAsBase64WithProperties)
    QgsMapBoxGlStyleConverter.__group__ = ['vectortile']
except (NameError, AttributeError):
    pass
try:
    QgsMapBoxGlStyleConversionContext.__group__ = ['vectortile']
except (NameError, AttributeError):
    pass
try:
    QgsMapBoxGlStyleAbstractSource.__group__ = ['vectortile']
except (NameError, AttributeError):
    pass
try:
    QgsMapBoxGlStyleRasterSource.__group__ = ['vectortile']
except (NameError, AttributeError):
    pass
try:
    QgsMapBoxGlStyleRasterSubLayer.__group__ = ['vectortile']
except (NameError, AttributeError):
    pass
