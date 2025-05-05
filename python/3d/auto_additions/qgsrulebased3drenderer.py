# The following has been generated automatically from src/3d/qgsrulebased3drenderer.h
try:
    QgsRuleBased3DRenderer.Rule.create = staticmethod(QgsRuleBased3DRenderer.Rule.create)
    import functools as _functools
    __wrapped_QgsRuleBased3DRenderer.Rule_setSymbol = QgsRuleBased3DRenderer.Rule.setSymbol
    def __QgsRuleBased3DRenderer.Rule_setSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsRuleBased3DRenderer.Rule_setSymbol(self, arg)
    QgsRuleBased3DRenderer.Rule.setSymbol = _functools.update_wrapper(__QgsRuleBased3DRenderer.Rule_setSymbol_wrapper, QgsRuleBased3DRenderer.Rule.setSymbol)

    import functools as _functools
    __wrapped_QgsRuleBased3DRenderer.Rule_appendChild = QgsRuleBased3DRenderer.Rule.appendChild
    def __QgsRuleBased3DRenderer.Rule_appendChild_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsRuleBased3DRenderer.Rule_appendChild(self, arg)
    QgsRuleBased3DRenderer.Rule.appendChild = _functools.update_wrapper(__QgsRuleBased3DRenderer.Rule_appendChild_wrapper, QgsRuleBased3DRenderer.Rule.appendChild)

except (NameError, AttributeError):
    pass
try:
    QgsRuleBased3DRendererMetadata.__overridden_methods__ = ['createRenderer']
except (NameError, AttributeError):
    pass
try:
    QgsRuleBased3DRenderer.__overridden_methods__ = ['type', 'clone', 'writeXml', 'readXml']
    import functools as _functools
    __wrapped_QgsRuleBased3DRenderer_QgsRuleBased3DRenderer = QgsRuleBased3DRenderer.QgsRuleBased3DRenderer
    def __QgsRuleBased3DRenderer_QgsRuleBased3DRenderer_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsRuleBased3DRenderer_QgsRuleBased3DRenderer(self, arg)
    QgsRuleBased3DRenderer.QgsRuleBased3DRenderer = _functools.update_wrapper(__QgsRuleBased3DRenderer_QgsRuleBased3DRenderer_wrapper, QgsRuleBased3DRenderer.QgsRuleBased3DRenderer)

except (NameError, AttributeError):
    pass
