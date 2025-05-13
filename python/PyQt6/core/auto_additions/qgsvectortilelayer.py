# The following has been generated automatically from src/core/vectortile/qgsvectortilelayer.h
try:
    QgsVectorTileLayer.LayerOptions.__attribute_docs__ = {'transformContext': 'Coordinate transform context'}
    QgsVectorTileLayer.LayerOptions.__annotations__ = {'transformContext': 'QgsCoordinateTransformContext'}
    QgsVectorTileLayer.LayerOptions.__doc__ = """Setting options for loading vector tile layers.

.. versionadded:: 3.22"""
    QgsVectorTileLayer.LayerOptions.__group__ = ['vectortile']
except (NameError, AttributeError):
    pass
try:
    QgsVectorTileLayer.__attribute_docs__ = {'selectionChanged': 'Emitted whenever the selected features in the layer are changed.\n\n.. versionadded:: 3.28\n'}
    QgsVectorTileLayer.__virtual_methods__ = ['encodedSource', 'decodedSource']
    QgsVectorTileLayer.__overridden_methods__ = ['clone', 'dataProvider', 'createMapRenderer', 'readXml', 'writeXml', 'readSymbology', 'writeSymbology', 'setTransformContext', 'loadDefaultStyle', 'properties', 'loadDefaultMetadata', 'htmlMetadata']
    QgsVectorTileLayer.__group__ = ['vectortile']
except (NameError, AttributeError):
    pass
