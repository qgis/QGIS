# The following has been generated automatically from src/core/processing/qgsprocessingutils.h
# monkey patching scoped based enum
QgsProcessingUtils.UnknownType = QgsProcessingUtils.LayerHint.UnknownType
QgsProcessingUtils.LayerHint.UnknownType.__doc__ = "Unknown layer type"
QgsProcessingUtils.Vector = QgsProcessingUtils.LayerHint.Vector
QgsProcessingUtils.LayerHint.Vector.__doc__ = "Vector layer type"
QgsProcessingUtils.Raster = QgsProcessingUtils.LayerHint.Raster
QgsProcessingUtils.LayerHint.Raster.__doc__ = "Raster layer type"
QgsProcessingUtils.Mesh = QgsProcessingUtils.LayerHint.Mesh
QgsProcessingUtils.LayerHint.Mesh.__doc__ = "Mesh layer type  \since QGIS 3.6"
QgsProcessingUtils.LayerHint.__doc__ = 'Layer type hints.\n\n.. versionadded:: 3.4\n\n' + '* ``UnknownType``: ' + QgsProcessingUtils.LayerHint.UnknownType.__doc__ + '\n' + '* ``Vector``: ' + QgsProcessingUtils.LayerHint.Vector.__doc__ + '\n' + '* ``Raster``: ' + QgsProcessingUtils.LayerHint.Raster.__doc__ + '\n' + '* ``Mesh``: ' + QgsProcessingUtils.LayerHint.Mesh.__doc__
# --
