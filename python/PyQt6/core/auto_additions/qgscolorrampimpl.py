# The following has been generated automatically from src/core/qgscolorrampimpl.h
try:
    QgsGradientStop.__attribute_docs__ = {'offset': 'Relative positional offset, between 0 and 1', 'color': 'Gradient color at stop'}
    QgsGradientStop.__annotations__ = {'offset': float, 'color': 'QColor'}
except (NameError, AttributeError):
    pass
try:
    QgsGradientColorRamp.create = staticmethod(QgsGradientColorRamp.create)
    QgsGradientColorRamp.typeString = staticmethod(QgsGradientColorRamp.typeString)
    QgsGradientColorRamp.__overridden_methods__ = ['count', 'value', 'color', 'type', 'invert', 'clone', 'properties']
except (NameError, AttributeError):
    pass
try:
    QgsLimitedRandomColorRamp.create = staticmethod(QgsLimitedRandomColorRamp.create)
    QgsLimitedRandomColorRamp.typeString = staticmethod(QgsLimitedRandomColorRamp.typeString)
    QgsLimitedRandomColorRamp.randomColors = staticmethod(QgsLimitedRandomColorRamp.randomColors)
    QgsLimitedRandomColorRamp.__overridden_methods__ = ['value', 'color', 'type', 'clone', 'properties', 'count']
except (NameError, AttributeError):
    pass
try:
    QgsRandomColorRamp.typeString = staticmethod(QgsRandomColorRamp.typeString)
    QgsRandomColorRamp.__virtual_methods__ = ['setTotalColorCount']
    QgsRandomColorRamp.__overridden_methods__ = ['count', 'value', 'color', 'type', 'clone', 'properties']
except (NameError, AttributeError):
    pass
try:
    QgsPresetSchemeColorRamp.create = staticmethod(QgsPresetSchemeColorRamp.create)
    QgsPresetSchemeColorRamp.typeString = staticmethod(QgsPresetSchemeColorRamp.typeString)
    QgsPresetSchemeColorRamp.__overridden_methods__ = ['setColors', 'value', 'color', 'type', 'invert', 'clone', 'properties', 'count', 'schemeName', 'fetchColors', 'isEditable']
except (NameError, AttributeError):
    pass
try:
    QgsColorBrewerColorRamp.create = staticmethod(QgsColorBrewerColorRamp.create)
    QgsColorBrewerColorRamp.typeString = staticmethod(QgsColorBrewerColorRamp.typeString)
    QgsColorBrewerColorRamp.listSchemeNames = staticmethod(QgsColorBrewerColorRamp.listSchemeNames)
    QgsColorBrewerColorRamp.listSchemeVariants = staticmethod(QgsColorBrewerColorRamp.listSchemeVariants)
    QgsColorBrewerColorRamp.__overridden_methods__ = ['value', 'color', 'type', 'invert', 'clone', 'properties', 'count']
except (NameError, AttributeError):
    pass
try:
    QgsCptCityColorRamp.create = staticmethod(QgsCptCityColorRamp.create)
    QgsCptCityColorRamp.typeString = staticmethod(QgsCptCityColorRamp.typeString)
    QgsCptCityColorRamp.__overridden_methods__ = ['type', 'invert', 'clone', 'properties']
except (NameError, AttributeError):
    pass
