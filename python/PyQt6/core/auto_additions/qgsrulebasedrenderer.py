# The following has been generated automatically from src/core/symbology/qgsrulebasedrenderer.h
QgsRuleBasedRenderer.FeatIsSelected = QgsRuleBasedRenderer.FeatureFlags.FeatIsSelected
QgsRuleBasedRenderer.FeatDrawMarkers = QgsRuleBasedRenderer.FeatureFlags.FeatDrawMarkers
QgsRuleBasedRenderer.Rule.Filtered = QgsRuleBasedRenderer.Rule.RenderResult.Filtered
QgsRuleBasedRenderer.Rule.Inactive = QgsRuleBasedRenderer.Rule.RenderResult.Inactive
QgsRuleBasedRenderer.Rule.Rendered = QgsRuleBasedRenderer.Rule.RenderResult.Rendered
try:
    QgsRuleBasedRenderer.RenderJob.__attribute_docs__ = {'ftr': 'Feature to render', 'symbol': 'Symbol to render feature with (not owned by this object).'}
    QgsRuleBasedRenderer.RenderJob.__annotations__ = {'ftr': 'QgsRuleBasedRenderer.FeatureToRender', 'symbol': 'QgsSymbol'}
    QgsRuleBasedRenderer.RenderJob.__doc__ = """A QgsRuleBasedRenderer rendering job, consisting of a feature to be rendered with a particular symbol."""
    QgsRuleBasedRenderer.RenderJob.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsRuleBasedRenderer.RenderLevel.__attribute_docs__ = {'jobs': 'List of jobs to render, owned by this object.'}
    QgsRuleBasedRenderer.RenderLevel.__annotations__ = {'jobs': 'List[QgsRuleBasedRenderer.RenderJob]'}
    QgsRuleBasedRenderer.RenderLevel.__doc__ = """Render level: a list of jobs to be drawn at particular level for a QgsRuleBasedRenderer."""
    QgsRuleBasedRenderer.RenderLevel.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsRuleBasedRenderer.create = staticmethod(QgsRuleBasedRenderer.create)
    QgsRuleBasedRenderer.createFromSld = staticmethod(QgsRuleBasedRenderer.createFromSld)
    QgsRuleBasedRenderer.refineRuleCategories = staticmethod(QgsRuleBasedRenderer.refineRuleCategories)
    QgsRuleBasedRenderer.refineRuleRanges = staticmethod(QgsRuleBasedRenderer.refineRuleRanges)
    QgsRuleBasedRenderer.refineRuleScales = staticmethod(QgsRuleBasedRenderer.refineRuleScales)
    QgsRuleBasedRenderer.convertFromRenderer = staticmethod(QgsRuleBasedRenderer.convertFromRenderer)
    QgsRuleBasedRenderer.convertToDataDefinedSymbology = staticmethod(QgsRuleBasedRenderer.convertToDataDefinedSymbology)
    QgsRuleBasedRenderer.__overridden_methods__ = ['symbolForFeature', 'flags', 'renderFeature', 'startRender', 'canSkipRender', 'stopRender', 'filter', 'usedAttributes', 'filterNeedsGeometry', 'clone', 'toSld', 'symbols', 'save', 'legendSymbolItemsCheckable', 'legendSymbolItemChecked', 'checkLegendSymbolItem', 'legendKeyToExpression', 'setLegendSymbolItem', 'legendSymbolItems', 'dump', 'willRenderFeature', 'symbolsForFeature', 'originalSymbolsForFeature', 'legendKeysForFeature', 'capabilities', 'accept']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsRuleBasedRenderer_QgsRuleBasedRenderer = QgsRuleBasedRenderer.QgsRuleBasedRenderer
    def __QgsRuleBasedRenderer_QgsRuleBasedRenderer_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsRuleBasedRenderer_QgsRuleBasedRenderer(self, arg)
    QgsRuleBasedRenderer.QgsRuleBasedRenderer = _functools.update_wrapper(__QgsRuleBasedRenderer_QgsRuleBasedRenderer_wrapper, QgsRuleBasedRenderer.QgsRuleBasedRenderer)

    QgsRuleBasedRenderer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsRuleBasedRenderer.Rule.createFromSld = staticmethod(QgsRuleBasedRenderer.Rule.createFromSld)
    QgsRuleBasedRenderer.Rule.create = staticmethod(QgsRuleBasedRenderer.Rule.create)
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsRuleBasedRenderer_Rule_setSymbol = QgsRuleBasedRenderer.Rule.setSymbol
    def __QgsRuleBasedRenderer_Rule_setSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsRuleBasedRenderer_Rule_setSymbol(self, arg)
    QgsRuleBasedRenderer.Rule.setSymbol = _functools.update_wrapper(__QgsRuleBasedRenderer_Rule_setSymbol_wrapper, QgsRuleBasedRenderer.Rule.setSymbol)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsRuleBasedRenderer_Rule_appendChild = QgsRuleBasedRenderer.Rule.appendChild
    def __QgsRuleBasedRenderer_Rule_appendChild_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsRuleBasedRenderer_Rule_appendChild(self, arg)
    QgsRuleBasedRenderer.Rule.appendChild = _functools.update_wrapper(__QgsRuleBasedRenderer_Rule_appendChild_wrapper, QgsRuleBasedRenderer.Rule.appendChild)

    QgsRuleBasedRenderer.Rule.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsRuleBasedRenderer.FeatureToRender.__doc__ = """Feature for rendering by a QgsRuleBasedRenderer. Contains a QgsFeature and some flags."""
    QgsRuleBasedRenderer.FeatureToRender.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
