# The following has been generated automatically from src/core/./3d/qgs3drendererregistry.h
try:
    Qgs3DRendererAbstractMetadata.__abstract_methods__ = ['createRenderer']
    Qgs3DRendererAbstractMetadata.__group__ = ['3d']
except (NameError, AttributeError):
    pass
try:
    import functools as _functools
    __wrapped_Qgs3DRendererRegistry_addRenderer = Qgs3DRendererRegistry.addRenderer
    def __Qgs3DRendererRegistry_addRenderer_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_Qgs3DRendererRegistry_addRenderer(self, arg)
    Qgs3DRendererRegistry.addRenderer = _functools.update_wrapper(__Qgs3DRendererRegistry_addRenderer_wrapper, Qgs3DRendererRegistry.addRenderer)

    Qgs3DRendererRegistry.__group__ = ['3d']
except (NameError, AttributeError):
    pass
