# The following has been generated automatically from src/core/symbology/qgsrenderer.h
# monkey patching scoped based enum
QgsFeatureRenderer.Property.HeatmapRadius.__doc__ = "Heatmap renderer radius"
QgsFeatureRenderer.Property.HeatmapMaximum.__doc__ = "Heatmap maximum value"
QgsFeatureRenderer.Property.__doc__ = """Data definable properties for renderers.

.. versionadded:: 3.38

* ``HeatmapRadius``: Heatmap renderer radius
* ``HeatmapMaximum``: Heatmap maximum value

"""
# --
try:
    QgsFeatureRenderer.defaultRenderer = staticmethod(QgsFeatureRenderer.defaultRenderer)
    QgsFeatureRenderer.load = staticmethod(QgsFeatureRenderer.load)
    QgsFeatureRenderer.loadSld = staticmethod(QgsFeatureRenderer.loadSld)
    QgsFeatureRenderer._getPoint = staticmethod(QgsFeatureRenderer._getPoint)
    QgsFeatureRenderer.convertSymbolSizeScale = staticmethod(QgsFeatureRenderer.convertSymbolSizeScale)
    QgsFeatureRenderer.convertSymbolRotation = staticmethod(QgsFeatureRenderer.convertSymbolRotation)
    QgsFeatureRenderer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsSymbolLevelItem.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
