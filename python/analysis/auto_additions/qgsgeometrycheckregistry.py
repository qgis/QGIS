# The following has been generated automatically from src/analysis/vector/geometry_checker/qgsgeometrycheckregistry.h
try:
    import functools as _functools
    __wrapped_QgsGeometryCheckRegistry_registerGeometryCheck = QgsGeometryCheckRegistry.registerGeometryCheck
    def __QgsGeometryCheckRegistry_registerGeometryCheck_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsGeometryCheckRegistry_registerGeometryCheck(self, arg)
    QgsGeometryCheckRegistry.registerGeometryCheck = _functools.update_wrapper(__QgsGeometryCheckRegistry_registerGeometryCheck_wrapper, QgsGeometryCheckRegistry.registerGeometryCheck)

    QgsGeometryCheckRegistry.__group__ = ['vector', 'geometry_checker']
except (NameError, AttributeError):
    pass
