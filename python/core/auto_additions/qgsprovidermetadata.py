# The following has been generated automatically from src/core/providers/qgsprovidermetadata.h
QgsMeshDriverMetadata.MeshDriverCapability.baseClass = QgsMeshDriverMetadata
QgsMeshDriverMetadata.MeshDriverCapabilities.baseClass = QgsMeshDriverMetadata
MeshDriverCapabilities = QgsMeshDriverMetadata  # dirty hack since SIP seems to introduce the flags in module
try:
    QgsProviderMetadata.__attribute_docs__ = {'connectionCreated': 'Emitted when a connection with the specified ``name`` is created.\n\n.. note::\n\n   Only providers which implement the connection handling API will emit this signal.\n\n.. versionadded:: 3.14\n', 'connectionDeleted': 'Emitted when the connection with the specified ``name`` was deleted.\n\n.. note::\n\n   Only providers which implement the connection handling API will emit this signal.\n\n.. versionadded:: 3.14\n', 'connectionChanged': 'Emitted when the connection with the specified ``name`` is changed, e.g.\nthe settings relating to the connection have been updated.\n\n.. note::\n\n   Only providers which implement the connection handling API will emit this signal.\n\n.. versionadded:: 3.14\n'}
    QgsProviderMetadata.setBoolParameter = staticmethod(QgsProviderMetadata.setBoolParameter)
    QgsProviderMetadata.boolParameter = staticmethod(QgsProviderMetadata.boolParameter)
    QgsProviderMetadata.__virtual_methods__ = ['icon', 'capabilities', 'providerCapabilities', 'initProvider', 'cleanupProvider', 'filters', 'meshDriversMetadata', 'priorityForUri', 'validLayerTypesForUri', 'uriIsBlocklisted', 'sidecarFilesForUri', 'querySublayers', 'suggestGroupNameForUri', 'createProvider', 'createDatabase', 'createRasterDataProvider', 'createMeshData', 'pyramidResamplingMethods', 'decodeUri', 'encodeUri', 'absoluteToRelativeUri', 'relativeToAbsoluteUri', 'cleanUri', 'dataItemProviders', 'listStyles', 'styleExists', 'getStyleById', 'deleteStyleById', 'saveStyle', 'loadStyle', 'loadStoredStyle', 'saveLayerMetadata', 'createDb', 'createTransaction', 'connections', 'createConnection', 'deleteConnection', 'saveConnection']
    QgsProviderMetadata.__signal_arguments__ = {'connectionCreated': ['name: str'], 'connectionDeleted': ['name: str'], 'connectionChanged': ['name: str']}
    QgsProviderMetadata.__group__ = ['providers']
except (NameError, AttributeError):
    pass
try:
    QgsMeshDriverMetadata.__group__ = ['providers']
except (NameError, AttributeError):
    pass
