# The following has been generated automatically from src/core/providers/qgsprovidermetadata.h
QgsMeshDriverMetadata.MeshDriverCapability.baseClass = QgsMeshDriverMetadata
QgsMeshDriverMetadata.MeshDriverCapabilities.baseClass = QgsMeshDriverMetadata
MeshDriverCapabilities = QgsMeshDriverMetadata  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
QgsProviderMetadata.FilterType.FilterVector.__doc__ = "Vector layers"
QgsProviderMetadata.FilterType.FilterRaster.__doc__ = "Raster layers"
QgsProviderMetadata.FilterType.FilterMesh.__doc__ = "Mesh layers"
QgsProviderMetadata.FilterType.FilterMeshDataset.__doc__ = "Mesh datasets"
QgsProviderMetadata.FilterType.FilterPointCloud.__doc__ = "Point clouds (since QGIS 3.18)"
QgsProviderMetadata.FilterType.__doc__ = 'Type of file filters\n\n.. versionadded:: 3.10\n\n' + '* ``FilterVector``: ' + QgsProviderMetadata.FilterType.FilterVector.__doc__ + '\n' + '* ``FilterRaster``: ' + QgsProviderMetadata.FilterType.FilterRaster.__doc__ + '\n' + '* ``FilterMesh``: ' + QgsProviderMetadata.FilterType.FilterMesh.__doc__ + '\n' + '* ``FilterMeshDataset``: ' + QgsProviderMetadata.FilterType.FilterMeshDataset.__doc__ + '\n' + '* ``FilterPointCloud``: ' + QgsProviderMetadata.FilterType.FilterPointCloud.__doc__
# --
