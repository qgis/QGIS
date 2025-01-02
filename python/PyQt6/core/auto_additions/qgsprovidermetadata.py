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
    QgsProviderMetadata.__attribute_docs__ = {'connectionCreated': 'Emitted when a connection with the specified ``name`` is created.\n\n.. note::\n\n   Only providers which implement the connection handling API will emit this signal.\n\n.. versionadded:: 3.14\n', 'connectionDeleted': 'Emitted when the connection with the specified ``name`` was deleted.\n\n.. note::\n\n   Only providers which implement the connection handling API will emit this signal.\n\n.. versionadded:: 3.14\n', 'connectionChanged': 'Emitted when the connection with the specified ``name`` is changed, e.g. the settings\nrelating to the connection have been updated.\n\n.. note::\n\n   Only providers which implement the connection handling API will emit this signal.\n\n.. versionadded:: 3.14\n'}
    QgsProviderMetadata.setBoolParameter = staticmethod(QgsProviderMetadata.setBoolParameter)
    QgsProviderMetadata.boolParameter = staticmethod(QgsProviderMetadata.boolParameter)
    QgsProviderMetadata.__signal_arguments__ = {'connectionCreated': ['name: str'], 'connectionDeleted': ['name: str'], 'connectionChanged': ['name: str']}
    QgsProviderMetadata.__group__ = ['providers']
except (NameError, AttributeError):
    pass
try:
    QgsMeshDriverMetadata.__group__ = ['providers']
except (NameError, AttributeError):
    pass
