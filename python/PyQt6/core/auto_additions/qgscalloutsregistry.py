# The following has been generated automatically from src/core/callouts/qgscalloutsregistry.h
try:
    QgsCalloutRegistry.defaultCallout = staticmethod(QgsCalloutRegistry.defaultCallout)
    import functools as _functools
    __wrapped_QgsCalloutRegistry_addCalloutType = QgsCalloutRegistry.addCalloutType
    def __QgsCalloutRegistry_addCalloutType_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsCalloutRegistry_addCalloutType(self, arg)
    QgsCalloutRegistry.addCalloutType = _functools.update_wrapper(__QgsCalloutRegistry_addCalloutType_wrapper, QgsCalloutRegistry.addCalloutType)

    QgsCalloutRegistry.__group__ = ['callouts']
except (NameError, AttributeError):
    pass
try:
    QgsCalloutAbstractMetadata.__virtual_methods__ = ['createCalloutWidget']
    QgsCalloutAbstractMetadata.__abstract_methods__ = ['createCallout']
    QgsCalloutAbstractMetadata.__group__ = ['callouts']
except (NameError, AttributeError):
    pass
try:
    QgsCalloutMetadata.__overridden_methods__ = ['createCallout', 'createCalloutWidget']
    QgsCalloutMetadata.__group__ = ['callouts']
except (NameError, AttributeError):
    pass
