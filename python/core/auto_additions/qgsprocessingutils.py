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
QgsProcessingUtils.LayerHint.__doc__ = 'Layer type hints.\n\n.. versionadded:: 3.4\n\n' + '* ``UnknownType``: ' + QgsProcessingUtils.LayerHint.UnknownType.__doc__ + '\n' + '* ``Vector``: ' + QgsProcessingUtils.LayerHint.Vector.__doc__ + '\n' + '* ``Raster``: ' + QgsProcessingUtils.LayerHint.Raster.__doc__ + '\n' + '* ``Mesh``: ' + QgsProcessingUtils.LayerHint.Mesh.__doc__ + '\n' + '* ``PointCloud``: ' + QgsProcessingUtils.LayerHint.PointCloud.__doc__ + '\n' + '* ``Annotation``: ' + QgsProcessingUtils.LayerHint.Annotation.__doc__
# --
