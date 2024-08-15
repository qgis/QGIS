# The following has been generated automatically from src/core/symbology/qgsrenderer.h
# monkey patching scoped based enum
QgsFeatureRenderer.Property.HeatmapRadius.__doc__ = "Heatmap renderer radius"
QgsFeatureRenderer.Property.HeatmapMaximum.__doc__ = "Heatmap maximum value"
QgsFeatureRenderer.Property.__doc__ = "Data definable properties for renderers.\n\n.. versionadded:: 3.38\n\n" + '* ``HeatmapRadius``: ' + QgsFeatureRenderer.Property.HeatmapRadius.__doc__ + '\n' + '* ``HeatmapMaximum``: ' + QgsFeatureRenderer.Property.HeatmapMaximum.__doc__
# --
QgsFeatureRenderer.defaultRenderer = staticmethod(QgsFeatureRenderer.defaultRenderer)
QgsFeatureRenderer.load = staticmethod(QgsFeatureRenderer.load)
QgsFeatureRenderer.loadSld = staticmethod(QgsFeatureRenderer.loadSld)
QgsFeatureRenderer._getPoint = staticmethod(QgsFeatureRenderer._getPoint)
QgsFeatureRenderer.convertSymbolSizeScale = staticmethod(QgsFeatureRenderer.convertSymbolSizeScale)
QgsFeatureRenderer.convertSymbolRotation = staticmethod(QgsFeatureRenderer.convertSymbolRotation)
try:
    QgsSymbolLevelItem.__group__ = ['symbology']
except NameError:
    pass
try:
    QgsFeatureRenderer.__group__ = ['symbology']
except NameError:
    pass
