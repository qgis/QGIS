# The following has been generated automatically from src/core/symbology/qgsmergedfeaturerenderer.h
QgsMergedFeatureRenderer.Merge = QgsMergedFeatureRenderer.GeometryOperation.Merge
QgsMergedFeatureRenderer.InvertOnly = QgsMergedFeatureRenderer.GeometryOperation.InvertOnly
QgsMergedFeatureRenderer.MergeAndInvert = QgsMergedFeatureRenderer.GeometryOperation.MergeAndInvert
try:
    QgsMergedFeatureRenderer.create = staticmethod(QgsMergedFeatureRenderer.create)
    QgsMergedFeatureRenderer.convertFromRenderer = staticmethod(QgsMergedFeatureRenderer.convertFromRenderer)
    QgsMergedFeatureRenderer.__overridden_methods__ = ['clone', 'startRender', 'flags', 'renderFeature', 'stopRender', 'dump', 'usedAttributes', 'filterNeedsGeometry', 'capabilities', 'symbols', 'symbolForFeature', 'originalSymbolForFeature', 'symbolsForFeature', 'originalSymbolsForFeature', 'legendKeysForFeature', 'legendKeyToExpression', 'legendSymbolItems', 'willRenderFeature', 'save', 'setEmbeddedRenderer', 'embeddedRenderer', 'setLegendSymbolItem', 'legendSymbolItemsCheckable', 'legendSymbolItemChecked', 'checkLegendSymbolItem', 'accept']
    QgsMergedFeatureRenderer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
