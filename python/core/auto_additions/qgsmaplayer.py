# The following has been generated automatically from src/core/qgsmaplayer.h
QgsMapLayer.LayerType = QgsMapLayerType
# monkey patching scoped based enum
QgsMapLayer.VectorLayer = QgsMapLayerType.VectorLayer
QgsMapLayer.VectorLayer.__doc__ = ""
QgsMapLayer.RasterLayer = QgsMapLayerType.RasterLayer
QgsMapLayer.RasterLayer.__doc__ = ""
QgsMapLayer.PluginLayer = QgsMapLayerType.PluginLayer
QgsMapLayer.PluginLayer.__doc__ = ""
QgsMapLayer.MeshLayer = QgsMapLayerType.MeshLayer
QgsMapLayer.MeshLayer.__doc__ = "Added in 3.2"
QgsMapLayer.VectorTileLayer = QgsMapLayerType.VectorTileLayer
QgsMapLayer.VectorTileLayer.__doc__ = "Added in 3.14"
QgsMapLayer.AnnotationLayer = QgsMapLayerType.AnnotationLayer
QgsMapLayer.AnnotationLayer.__doc__ = "Contains freeform, georeferenced annotations. Added in QGIS 3.16"
QgsMapLayerType.__doc__ = 'Types of layers that can be added to a map\n\n.. versionadded:: 3.8\n\n' + '* ``VectorLayer``: ' + QgsMapLayerType.VectorLayer.__doc__ + '\n' + '* ``RasterLayer``: ' + QgsMapLayerType.RasterLayer.__doc__ + '\n' + '* ``PluginLayer``: ' + QgsMapLayerType.PluginLayer.__doc__ + '\n' + '* ``MeshLayer``: ' + QgsMapLayerType.MeshLayer.__doc__ + '\n' + '* ``VectorTileLayer``: ' + QgsMapLayerType.VectorTileLayer.__doc__ + '\n' + '* ``AnnotationLayer``: ' + QgsMapLayerType.AnnotationLayer.__doc__
# --
QgsMapLayer.LayerFlag.baseClass = QgsMapLayer
QgsMapLayer.LayerFlags.baseClass = QgsMapLayer
LayerFlags = QgsMapLayer  # dirty hack since SIP seems to introduce the flags in module
QgsMapLayer.StyleCategory.baseClass = QgsMapLayer
QgsMapLayer.StyleCategories.baseClass = QgsMapLayer
StyleCategories = QgsMapLayer  # dirty hack since SIP seems to introduce the flags in module
