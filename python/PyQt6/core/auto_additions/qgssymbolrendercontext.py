# The following has been generated automatically from src/core/symbology/qgssymbolrendercontext.h
try:
    import functools as _functools
    __wrapped_QgsSymbolRenderContext_setExpressionContextScope = QgsSymbolRenderContext.setExpressionContextScope
    def __QgsSymbolRenderContext_setExpressionContextScope_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsSymbolRenderContext_setExpressionContextScope(self, arg)
    QgsSymbolRenderContext.setExpressionContextScope = _functools.update_wrapper(__QgsSymbolRenderContext_setExpressionContextScope_wrapper, QgsSymbolRenderContext.setExpressionContextScope)

    QgsSymbolRenderContext.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
