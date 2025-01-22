# The following has been generated automatically from src/core/pointcloud/qgspointclouddataprovider.h
QgsPointCloudDataProvider.NoCapabilities = QgsPointCloudDataProvider.Capability.NoCapabilities
QgsPointCloudDataProvider.ReadLayerMetadata = QgsPointCloudDataProvider.Capability.ReadLayerMetadata
QgsPointCloudDataProvider.WriteLayerMetadata = QgsPointCloudDataProvider.Capability.WriteLayerMetadata
QgsPointCloudDataProvider.CreateRenderer = QgsPointCloudDataProvider.Capability.CreateRenderer
QgsPointCloudDataProvider.ContainSubIndexes = QgsPointCloudDataProvider.Capability.ContainSubIndexes
QgsPointCloudDataProvider.ChangeAttributeValues = QgsPointCloudDataProvider.Capability.ChangeAttributeValues
QgsPointCloudDataProvider.Capabilities = lambda flags=0: QgsPointCloudDataProvider.Capability(flags)
QgsPointCloudDataProvider.NotIndexed = QgsPointCloudDataProvider.PointCloudIndexGenerationState.NotIndexed
QgsPointCloudDataProvider.Indexing = QgsPointCloudDataProvider.PointCloudIndexGenerationState.Indexing
QgsPointCloudDataProvider.Indexed = QgsPointCloudDataProvider.PointCloudIndexGenerationState.Indexed
try:
    QgsPointCloudDataProvider.__attribute_docs__ = {'indexGenerationStateChanged': 'Emitted when point cloud generation state is changed\n'}
    QgsPointCloudDataProvider.lasClassificationCodes = staticmethod(QgsPointCloudDataProvider.lasClassificationCodes)
    QgsPointCloudDataProvider.translatedLasClassificationCodes = staticmethod(QgsPointCloudDataProvider.translatedLasClassificationCodes)
    QgsPointCloudDataProvider.dataFormatIds = staticmethod(QgsPointCloudDataProvider.dataFormatIds)
    QgsPointCloudDataProvider.translatedDataFormatIds = staticmethod(QgsPointCloudDataProvider.translatedDataFormatIds)
    QgsPointCloudDataProvider.__signal_arguments__ = {'indexGenerationStateChanged': ['state: QgsPointCloudDataProvider.PointCloudIndexGenerationState']}
    QgsPointCloudDataProvider.__group__ = ['pointcloud']
except (NameError, AttributeError):
    pass
