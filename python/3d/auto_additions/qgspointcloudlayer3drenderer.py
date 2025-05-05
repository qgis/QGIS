# The following has been generated automatically from src/3d/qgspointcloudlayer3drenderer.h
try:
    QgsPointCloudLayer3DRenderer.__overridden_methods__ = ['type', 'clone', 'writeXml', 'readXml', 'resolveReferences', 'convertFrom2DRenderer']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsPointCloudLayer3DRenderer_setSymbol = QgsPointCloudLayer3DRenderer.setSymbol
    def __QgsPointCloudLayer3DRenderer_setSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsPointCloudLayer3DRenderer_setSymbol(self, arg)
    QgsPointCloudLayer3DRenderer.setSymbol = _functools.update_wrapper(__QgsPointCloudLayer3DRenderer_setSymbol_wrapper, QgsPointCloudLayer3DRenderer.setSymbol)

except (NameError, AttributeError):
    pass
