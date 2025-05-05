# The following has been generated automatically from src/core/symbology/qgspointdistancerenderer.h
try:
    QgsPointDistanceRenderer.GroupedFeature.__attribute_docs__ = {'feature': 'Feature', 'isSelected': 'True if feature is selected and should be rendered in a selected state', 'label': 'Optional label text'}
    QgsPointDistanceRenderer.GroupedFeature.__annotations__ = {'feature': 'QgsFeature', 'isSelected': bool, 'label': str}
    QgsPointDistanceRenderer.GroupedFeature.__doc__ = """Contains properties for a feature within a clustered group."""
    QgsPointDistanceRenderer.GroupedFeature.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsPointDistanceRenderer.__abstract_methods__ = ['drawGroup']
    QgsPointDistanceRenderer.__overridden_methods__ = ['toSld', 'renderFeature', 'usedAttributes', 'filterNeedsGeometry', 'capabilities', 'symbols', 'symbolForFeature', 'originalSymbolForFeature', 'symbolsForFeature', 'originalSymbolsForFeature', 'legendKeysForFeature', 'legendKeyToExpression', 'willRenderFeature', 'startRender', 'stopRender', 'legendSymbolItems', 'setEmbeddedRenderer', 'embeddedRenderer', 'setLegendSymbolItem', 'legendSymbolItemsCheckable', 'legendSymbolItemChecked', 'checkLegendSymbolItem', 'filter', 'accept']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsPointDistanceRenderer_embeddedRenderer = QgsPointDistanceRenderer.embeddedRenderer
    def __QgsPointDistanceRenderer_embeddedRenderer_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsPointDistanceRenderer_embeddedRenderer(self, arg)
    QgsPointDistanceRenderer.embeddedRenderer = _functools.update_wrapper(__QgsPointDistanceRenderer_embeddedRenderer_wrapper, QgsPointDistanceRenderer.embeddedRenderer)

    QgsPointDistanceRenderer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
