# The following has been generated automatically from src/core/qgsmaplayer.h
# monkey patching scoped based enum
QgsMapLayer.VectorLayer.__doc__ = ""
QgsMapLayer.VectorLayer = QgsMapLayerType.VectorLayer
QgsMapLayer.RasterLayer.__doc__ = ""
QgsMapLayer.RasterLayer = QgsMapLayerType.RasterLayer
QgsMapLayer.PluginLayer.__doc__ = ""
QgsMapLayer.PluginLayer = QgsMapLayerType.PluginLayer
QgsMapLayer.MeshLayer.__doc__ = "Added in 3.2"
QgsMapLayer.MeshLayer = QgsMapLayerType.MeshLayer
.QgsMapLayerType.__doc__ = 'Types of layers that can be added to a map\n\n' + '* VectorLayer: ' + .QgsMapLayerType.VectorLayer.__doc__ + '\n' + '* RasterLayer: ' + .QgsMapLayerType.RasterLayer.__doc__ + '\n' + '* PluginLayer: ' + .QgsMapLayerType.PluginLayer.__doc__ + '\n' + '* MeshLayer: ' + .QgsMapLayerType.MeshLayer.__doc__
# --
QgsMapLayer.LayerFlag.baseClass = QgsMapLayer
QgsMapLayer.LayerFlags.baseClass = QgsMapLayer
LayerFlags = QgsMapLayer  # dirty hack since SIP seems to introduce the flags in module
QgsMapLayer.StyleCategory.baseClass = QgsMapLayer
QgsMapLayer.StyleCategories.baseClass = QgsMapLayer
StyleCategories = QgsMapLayer  # dirty hack since SIP seems to introduce the flags in module
