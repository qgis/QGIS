# The following has been generated automatically from src/core/classification/qgsclassificationmethodregistry.h
try:
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsClassificationMethodRegistry_addMethod = QgsClassificationMethodRegistry.addMethod
    def __QgsClassificationMethodRegistry_addMethod_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsClassificationMethodRegistry_addMethod(self, arg)
    QgsClassificationMethodRegistry.addMethod = _functools.update_wrapper(__QgsClassificationMethodRegistry_addMethod_wrapper, QgsClassificationMethodRegistry.addMethod)

    QgsClassificationMethodRegistry.__group__ = ['classification']
except (NameError, AttributeError):
    pass
