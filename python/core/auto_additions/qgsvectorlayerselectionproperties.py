# The following has been generated automatically from src/core/vector/qgsvectorlayerselectionproperties.h
try:
    QgsVectorLayerSelectionProperties.__overridden_methods__ = ['writeXml', 'readXml', 'clone']
    import functools as _functools
    __wrapped_QgsVectorLayerSelectionProperties_setSelectionSymbol = QgsVectorLayerSelectionProperties.setSelectionSymbol
    def __QgsVectorLayerSelectionProperties_setSelectionSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsVectorLayerSelectionProperties_setSelectionSymbol(self, arg)
    QgsVectorLayerSelectionProperties.setSelectionSymbol = _functools.update_wrapper(__QgsVectorLayerSelectionProperties_setSelectionSymbol_wrapper, QgsVectorLayerSelectionProperties.setSelectionSymbol)

    QgsVectorLayerSelectionProperties.__group__ = ['vector']
except (NameError, AttributeError):
    pass
