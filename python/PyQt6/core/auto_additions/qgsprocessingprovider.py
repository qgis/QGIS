# The following has been generated automatically from src/core/processing/qgsprocessingprovider.h
try:
    QgsProcessingProvider.__attribute_docs__ = {'algorithmsLoaded': 'Emitted when the provider has loaded (or refreshed) its list of\navailable algorithms.\n\n.. seealso:: :py:func:`refreshAlgorithms`\n'}
    QgsProcessingProvider.__virtual_methods__ = ['icon', 'svgIconPath', 'flags', 'helpId', 'longName', 'versionInfo', 'canBeActivated', 'warningMessage', 'isActive', 'supportedOutputRasterLayerExtensions', 'supportedOutputVectorLayerExtensions', 'supportedOutputPointCloudLayerExtensions', 'supportedOutputVectorTileLayerExtensions', 'supportedOutputTableExtensions', 'isSupportedOutputValue', 'defaultVectorFileExtension', 'defaultRasterFileExtension', 'defaultPointCloudFileExtension', 'defaultVectorTileFileExtension', 'supportsNonFileBasedOutput', 'load', 'unload']
    QgsProcessingProvider.__abstract_methods__ = ['id', 'name', 'loadAlgorithms']
    QgsProcessingProvider.__group__ = ['processing']
except (NameError, AttributeError):
    pass
