# The following has been generated automatically from src/core/providers/qgsprovidermetadata.h
QgsMeshDriverMetadata.CanWriteFaceDatasets = QgsMeshDriverMetadata.MeshDriverCapability.CanWriteFaceDatasets
QgsMeshDriverMetadata.CanWriteVertexDatasets = QgsMeshDriverMetadata.MeshDriverCapability.CanWriteVertexDatasets
QgsMeshDriverMetadata.CanWriteEdgeDatasets = QgsMeshDriverMetadata.MeshDriverCapability.CanWriteEdgeDatasets
QgsMeshDriverMetadata.CanWriteMeshData = QgsMeshDriverMetadata.MeshDriverCapability.CanWriteMeshData
QgsMeshDriverMetadata.MeshDriverCapability.baseClass = QgsMeshDriverMetadata
QgsMeshDriverMetadata.MeshDriverCapabilities = lambda flags=0: QgsMeshDriverMetadata.MeshDriverCapability(flags)
QgsMeshDriverMetadata.MeshDriverCapabilities.baseClass = QgsMeshDriverMetadata
MeshDriverCapabilities = QgsMeshDriverMetadata  # dirty hack since SIP seems to introduce the flags in module
from enum import Enum


def _force_int(v): return int(v.value) if isinstance(v, Enum) else v


QgsMeshDriverMetadata.MeshDriverCapability.__bool__ = lambda flag: bool(_force_int(flag))
QgsMeshDriverMetadata.MeshDriverCapability.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsMeshDriverMetadata.MeshDriverCapability.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsMeshDriverMetadata.MeshDriverCapability.__or__ = lambda flag1, flag2: QgsMeshDriverMetadata.MeshDriverCapability(_force_int(flag1) | _force_int(flag2))
QgsProviderMetadata.PriorityForUri = QgsProviderMetadata.ProviderMetadataCapability.PriorityForUri
QgsProviderMetadata.LayerTypesForUri = QgsProviderMetadata.ProviderMetadataCapability.LayerTypesForUri
QgsProviderMetadata.QuerySublayers = QgsProviderMetadata.ProviderMetadataCapability.QuerySublayers
QgsProviderMetadata.CreateDatabase = QgsProviderMetadata.ProviderMetadataCapability.CreateDatabase
QgsProviderMetadata.UrisReferToSame = QgsProviderMetadata.ProviderMetadataCapability.UrisReferToSame
QgsProviderMetadata.ProviderMetadataCapabilities = lambda flags=0: QgsProviderMetadata.ProviderMetadataCapability(flags)
QgsProviderMetadata.FileBasedUris = QgsProviderMetadata.ProviderCapability.FileBasedUris
QgsProviderMetadata.SaveLayerMetadata = QgsProviderMetadata.ProviderCapability.SaveLayerMetadata
QgsProviderMetadata.ParallelCreateProvider = QgsProviderMetadata.ProviderCapability.ParallelCreateProvider
QgsProviderMetadata.ProviderCapabilities = lambda flags=0: QgsProviderMetadata.ProviderCapability(flags)
QgsProviderMetadata.ProviderMetadataCapability.__bool__ = lambda flag: bool(_force_int(flag))
QgsProviderMetadata.ProviderMetadataCapability.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsProviderMetadata.ProviderMetadataCapability.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsProviderMetadata.ProviderMetadataCapability.__or__ = lambda flag1, flag2: QgsProviderMetadata.ProviderMetadataCapability(_force_int(flag1) | _force_int(flag2))
QgsProviderMetadata.ProviderCapability.__bool__ = lambda flag: bool(_force_int(flag))
QgsProviderMetadata.ProviderCapability.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsProviderMetadata.ProviderCapability.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsProviderMetadata.ProviderCapability.__or__ = lambda flag1, flag2: QgsProviderMetadata.ProviderCapability(_force_int(flag1) | _force_int(flag2))
try:
    QgsProviderMetadata.setBoolParameter = staticmethod(QgsProviderMetadata.setBoolParameter)
    QgsProviderMetadata.boolParameter = staticmethod(QgsProviderMetadata.boolParameter)
    QgsProviderMetadata.__virtual_methods__ = ['icon', 'capabilities', 'providerCapabilities', 'initProvider', 'cleanupProvider', 'filters', 'meshDriversMetadata', 'priorityForUri', 'validLayerTypesForUri', 'uriIsBlocklisted', 'sidecarFilesForUri', 'querySublayers', 'suggestGroupNameForUri', 'createProvider', 'createDatabase', 'createRasterDataProvider', 'createMeshData', 'pyramidResamplingMethods', 'decodeUri', 'encodeUri', 'absoluteToRelativeUri', 'relativeToAbsoluteUri', 'cleanUri', 'urisReferToSame', 'dataItemProviders', 'listStyles', 'styleExists', 'getStyleById', 'deleteStyleById', 'saveStyle', 'loadStyle', 'loadStoredStyle', 'saveLayerMetadata', 'createDb', 'createTransaction', 'connections', 'createConnection', 'deleteConnection', 'saveConnection']
    QgsProviderMetadata.__group__ = ['providers']
except (NameError, AttributeError):
    pass
try:
    QgsMeshDriverMetadata.__group__ = ['providers']
except (NameError, AttributeError):
    pass
