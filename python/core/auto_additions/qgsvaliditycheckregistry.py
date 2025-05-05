# The following has been generated automatically from src/core/validity/qgsvaliditycheckregistry.h
try:
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsValidityCheckRegistry_addCheck = QgsValidityCheckRegistry.addCheck
    def __QgsValidityCheckRegistry_addCheck_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsValidityCheckRegistry_addCheck(self, arg)
    QgsValidityCheckRegistry.addCheck = _functools.update_wrapper(__QgsValidityCheckRegistry_addCheck_wrapper, QgsValidityCheckRegistry.addCheck)

    QgsValidityCheckRegistry.__group__ = ['validity']
except (NameError, AttributeError):
    pass
