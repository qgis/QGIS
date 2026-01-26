# The following has been generated automatically from src/core/symbology/qgsvectorfieldsymbollayer.h
QgsVectorFieldSymbolLayer.Cartesian = QgsVectorFieldSymbolLayer.VectorFieldType.Cartesian
QgsVectorFieldSymbolLayer.Polar = QgsVectorFieldSymbolLayer.VectorFieldType.Polar
QgsVectorFieldSymbolLayer.Height = QgsVectorFieldSymbolLayer.VectorFieldType.Height
QgsVectorFieldSymbolLayer.ClockwiseFromNorth = QgsVectorFieldSymbolLayer.AngleOrientation.ClockwiseFromNorth
QgsVectorFieldSymbolLayer.CounterclockwiseFromEast = QgsVectorFieldSymbolLayer.AngleOrientation.CounterclockwiseFromEast
QgsVectorFieldSymbolLayer.Degrees = QgsVectorFieldSymbolLayer.AngleUnits.Degrees
QgsVectorFieldSymbolLayer.Radians = QgsVectorFieldSymbolLayer.AngleUnits.Radians
try:
    QgsVectorFieldSymbolLayer.create = staticmethod(QgsVectorFieldSymbolLayer.create)
    QgsVectorFieldSymbolLayer.createFromSld = staticmethod(QgsVectorFieldSymbolLayer.createFromSld)
    QgsVectorFieldSymbolLayer.__overridden_methods__ = ['layerType', 'setSubSymbol', 'subSymbol', 'setColor', 'color', 'renderPoint', 'startRender', 'stopRender', 'clone', 'properties', 'usesMapUnits', 'toSld', 'drawPreviewIcon', 'usedAttributes', 'hasDataDefinedProperties', 'setOutputUnit', 'outputUnit', 'setMapUnitScale', 'mapUnitScale', 'bounds']
    QgsVectorFieldSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
