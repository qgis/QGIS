# The following has been generated automatically from src/core/symbology/qgsembeddedsymbolrenderer.h
try:
    QgsEmbeddedSymbolRenderer.create = staticmethod(QgsEmbeddedSymbolRenderer.create)
    QgsEmbeddedSymbolRenderer.convertFromRenderer = staticmethod(QgsEmbeddedSymbolRenderer.convertFromRenderer)
    QgsEmbeddedSymbolRenderer.__overridden_methods__ = ['symbolForFeature', 'originalSymbolForFeature', 'startRender', 'renderFeature', 'stopRender', 'usedAttributes', 'usesEmbeddedSymbols', 'clone', 'capabilities', 'save', 'symbols']
    import functools as _functools
    __wrapped_QgsEmbeddedSymbolRenderer_QgsEmbeddedSymbolRenderer = QgsEmbeddedSymbolRenderer.QgsEmbeddedSymbolRenderer
    def __QgsEmbeddedSymbolRenderer_QgsEmbeddedSymbolRenderer_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsEmbeddedSymbolRenderer_QgsEmbeddedSymbolRenderer(self, arg)
    QgsEmbeddedSymbolRenderer.QgsEmbeddedSymbolRenderer = _functools.update_wrapper(__QgsEmbeddedSymbolRenderer_QgsEmbeddedSymbolRenderer_wrapper, QgsEmbeddedSymbolRenderer.QgsEmbeddedSymbolRenderer)

    import functools as _functools
    __wrapped_QgsEmbeddedSymbolRenderer_setDefaultSymbol = QgsEmbeddedSymbolRenderer.setDefaultSymbol
    def __QgsEmbeddedSymbolRenderer_setDefaultSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsEmbeddedSymbolRenderer_setDefaultSymbol(self, arg)
    QgsEmbeddedSymbolRenderer.setDefaultSymbol = _functools.update_wrapper(__QgsEmbeddedSymbolRenderer_setDefaultSymbol_wrapper, QgsEmbeddedSymbolRenderer.setDefaultSymbol)

    QgsEmbeddedSymbolRenderer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
