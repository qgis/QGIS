# The following has been generated automatically from src/core/processing/qgsprocessingutils.h
# monkey patching scoped based enum
QgsProcessingUtils.UnknownType = QgsProcessingUtils.LayerHint.UnknownType
QgsProcessingUtils.UnknownType.is_monkey_patched = True
QgsProcessingUtils.LayerHint.UnknownType.__doc__ = "Unknown layer type"
QgsProcessingUtils.Vector = QgsProcessingUtils.LayerHint.Vector
QgsProcessingUtils.Vector.is_monkey_patched = True
QgsProcessingUtils.LayerHint.Vector.__doc__ = "Vector layer type"
QgsProcessingUtils.Raster = QgsProcessingUtils.LayerHint.Raster
QgsProcessingUtils.Raster.is_monkey_patched = True
QgsProcessingUtils.LayerHint.Raster.__doc__ = "Raster layer type"
QgsProcessingUtils.Mesh = QgsProcessingUtils.LayerHint.Mesh
QgsProcessingUtils.Mesh.is_monkey_patched = True
QgsProcessingUtils.LayerHint.Mesh.__doc__ = "Mesh layer type, since QGIS 3.6"
QgsProcessingUtils.PointCloud = QgsProcessingUtils.LayerHint.PointCloud
QgsProcessingUtils.PointCloud.is_monkey_patched = True
QgsProcessingUtils.LayerHint.PointCloud.__doc__ = "Point cloud layer type, since QGIS 3.22"
QgsProcessingUtils.Annotation = QgsProcessingUtils.LayerHint.Annotation
QgsProcessingUtils.Annotation.is_monkey_patched = True
QgsProcessingUtils.LayerHint.Annotation.__doc__ = "Annotation layer type, since QGIS 3.22"
QgsProcessingUtils.VectorTile = QgsProcessingUtils.LayerHint.VectorTile
QgsProcessingUtils.VectorTile.is_monkey_patched = True
QgsProcessingUtils.LayerHint.VectorTile.__doc__ = "Vector tile layer type, since QGIS 3.32"
QgsProcessingUtils.TiledScene = QgsProcessingUtils.LayerHint.TiledScene
QgsProcessingUtils.TiledScene.is_monkey_patched = True
QgsProcessingUtils.LayerHint.TiledScene.__doc__ = "Tiled scene layer type, since QGIS 3.34"
QgsProcessingUtils.LayerHint.__doc__ = """Layer type hints.

.. versionadded:: 3.4

* ``UnknownType``: Unknown layer type
* ``Vector``: Vector layer type
* ``Raster``: Raster layer type
* ``Mesh``: Mesh layer type, since QGIS 3.6
* ``PointCloud``: Point cloud layer type, since QGIS 3.22
* ``Annotation``: Annotation layer type, since QGIS 3.22
* ``VectorTile``: Vector tile layer type, since QGIS 3.32
* ``TiledScene``: Tiled scene layer type, since QGIS 3.34

"""
# --
QgsProcessingUtils.LayerHint.baseClass = QgsProcessingUtils
try:
    QgsProcessingUtils.compatibleRasterLayers = staticmethod(QgsProcessingUtils.compatibleRasterLayers)
    QgsProcessingUtils.compatibleVectorLayers = staticmethod(QgsProcessingUtils.compatibleVectorLayers)
    QgsProcessingUtils.compatibleMeshLayers = staticmethod(QgsProcessingUtils.compatibleMeshLayers)
    QgsProcessingUtils.compatiblePluginLayers = staticmethod(QgsProcessingUtils.compatiblePluginLayers)
    QgsProcessingUtils.compatiblePointCloudLayers = staticmethod(QgsProcessingUtils.compatiblePointCloudLayers)
    QgsProcessingUtils.compatibleAnnotationLayers = staticmethod(QgsProcessingUtils.compatibleAnnotationLayers)
    QgsProcessingUtils.compatibleVectorTileLayers = staticmethod(QgsProcessingUtils.compatibleVectorTileLayers)
    QgsProcessingUtils.compatibleTiledSceneLayers = staticmethod(QgsProcessingUtils.compatibleTiledSceneLayers)
    QgsProcessingUtils.compatibleLayers = staticmethod(QgsProcessingUtils.compatibleLayers)
    QgsProcessingUtils.encodeProviderKeyAndUri = staticmethod(QgsProcessingUtils.encodeProviderKeyAndUri)
    QgsProcessingUtils.decodeProviderKeyAndUri = staticmethod(QgsProcessingUtils.decodeProviderKeyAndUri)
    QgsProcessingUtils.mapLayerFromString = staticmethod(QgsProcessingUtils.mapLayerFromString)
    QgsProcessingUtils.variantToSource = staticmethod(QgsProcessingUtils.variantToSource)
    QgsProcessingUtils.variantToCrs = staticmethod(QgsProcessingUtils.variantToCrs)
    QgsProcessingUtils.normalizeLayerSource = staticmethod(QgsProcessingUtils.normalizeLayerSource)
    QgsProcessingUtils.layerToStringIdentifier = staticmethod(QgsProcessingUtils.layerToStringIdentifier)
    QgsProcessingUtils.variantToPythonLiteral = staticmethod(QgsProcessingUtils.variantToPythonLiteral)
    QgsProcessingUtils.stringToPythonLiteral = staticmethod(QgsProcessingUtils.stringToPythonLiteral)
    QgsProcessingUtils.createFeatureSink = staticmethod(QgsProcessingUtils.createFeatureSink)
    QgsProcessingUtils.combineLayerExtents = staticmethod(QgsProcessingUtils.combineLayerExtents)
    QgsProcessingUtils.generateIteratingDestination = staticmethod(QgsProcessingUtils.generateIteratingDestination)
    QgsProcessingUtils.tempFolder = staticmethod(QgsProcessingUtils.tempFolder)
    QgsProcessingUtils.generateTempFilename = staticmethod(QgsProcessingUtils.generateTempFilename)
    QgsProcessingUtils.formatHelpMapAsHtml = staticmethod(QgsProcessingUtils.formatHelpMapAsHtml)
    QgsProcessingUtils.convertToCompatibleFormat = staticmethod(QgsProcessingUtils.convertToCompatibleFormat)
    QgsProcessingUtils.convertToCompatibleFormatAndLayerName = staticmethod(QgsProcessingUtils.convertToCompatibleFormatAndLayerName)
    QgsProcessingUtils.combineFields = staticmethod(QgsProcessingUtils.combineFields)
    QgsProcessingUtils.fieldNamesToIndices = staticmethod(QgsProcessingUtils.fieldNamesToIndices)
    QgsProcessingUtils.indicesToFields = staticmethod(QgsProcessingUtils.indicesToFields)
    QgsProcessingUtils.defaultVectorExtension = staticmethod(QgsProcessingUtils.defaultVectorExtension)
    QgsProcessingUtils.defaultRasterExtension = staticmethod(QgsProcessingUtils.defaultRasterExtension)
    QgsProcessingUtils.defaultPointCloudExtension = staticmethod(QgsProcessingUtils.defaultPointCloudExtension)
    QgsProcessingUtils.defaultVectorTileExtension = staticmethod(QgsProcessingUtils.defaultVectorTileExtension)
    QgsProcessingUtils.removePointerValuesFromMap = staticmethod(QgsProcessingUtils.removePointerValuesFromMap)
    QgsProcessingUtils.preprocessQgisProcessParameters = staticmethod(QgsProcessingUtils.preprocessQgisProcessParameters)
    QgsProcessingUtils.resolveDefaultEncoding = staticmethod(QgsProcessingUtils.resolveDefaultEncoding)
    QgsProcessingUtils.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingFeatureSource.__group__ = ['processing']
except (NameError, AttributeError):
    pass
