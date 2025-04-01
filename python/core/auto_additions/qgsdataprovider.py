# The following has been generated automatically from src/core/providers/qgsdataprovider.h
try:
    QgsDataProvider.ProviderOptions.__attribute_docs__ = {'transformContext': 'Coordinate transform context'}
    QgsDataProvider.ProviderOptions.__annotations__ = {'transformContext': 'QgsCoordinateTransformContext'}
    QgsDataProvider.ProviderOptions.__doc__ = """Setting options for creating vector data providers.

.. note::

   coordinateTransformContext was added in QGIS 3.8

.. versionadded:: 3.2"""
    QgsDataProvider.ProviderOptions.__group__ = ['providers']
except (NameError, AttributeError):
    pass
try:
    QgsDataProvider.__attribute_docs__ = {'fullExtentCalculated': 'Emitted whenever a deferred extent calculation is completed by the\nprovider.\n\nLayers should connect to this signal and update their cached extents\nwhenever it is emitted.\n', 'dataChanged': "Emitted whenever a change is made to the data provider which may have\ncaused changes in the provider's data OUTSIDE of QGIS.\n\nWhen emitted from a :py:class:`QgsVectorDataProvider`, any cached\ninformation such as feature ids should be invalidated.\n\n.. warning::\n\n   This signal is NOT emitted when changes are made to a provider\n   from INSIDE QGIS -- e.g. when adding features to a vector layer, deleting features\n   or modifying existing features. Instead, the specific :py:class:`QgsVectorLayer` signals\n   should be used to detect these operations.\n", 'notify': 'Emitted when the datasource issues a notification.\n\n.. seealso:: :py:func:`setListening`\n'}
    QgsDataProvider.sublayerSeparator = staticmethod(QgsDataProvider.sublayerSeparator)
    QgsDataProvider.__virtual_methods__ = ['setDataSourceUri', 'dataSourceUri', 'dataComment', 'htmlMetadata', 'flags', 'temporalCapabilities', 'elevationProperties', 'extent3D', 'updateExtents', 'subsetStringDialect', 'subsetStringHelpUrl', 'setSubsetString', 'supportsSubsetString', 'subsetString', 'subLayers', 'subLayerStyles', 'subLayerCount', 'setLayerOrder', 'setSubLayerVisibility', 'fileVectorFilters', 'fileRasterFilters', 'reloadData', 'timestamp', 'dataTimestamp', 'error', 'invalidateConnections', 'enterUpdateMode', 'leaveUpdateMode', 'setListening', 'layerMetadata', 'writeLayerMetadata', 'styleStorageCapabilities']
    QgsDataProvider.__abstract_methods__ = ['crs', 'extent', 'isValid', 'name', 'description']
    QgsDataProvider.__signal_arguments__ = {'notify': ['msg: str']}
    QgsDataProvider.__group__ = ['providers']
except (NameError, AttributeError):
    pass
