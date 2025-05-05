# The following has been generated automatically from src/core/qgsproperty.h
try:
    QgsProperty.propertyMapToVariantMap = staticmethod(QgsProperty.propertyMapToVariantMap)
    QgsProperty.variantMapToPropertyMap = staticmethod(QgsProperty.variantMapToPropertyMap)
    QgsProperty.fromExpression = staticmethod(QgsProperty.fromExpression)
    QgsProperty.fromField = staticmethod(QgsProperty.fromField)
    QgsProperty.fromValue = staticmethod(QgsProperty.fromValue)
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsProperty_setTransformer = QgsProperty.setTransformer
    def __QgsProperty_setTransformer_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsProperty_setTransformer(self, arg)
    QgsProperty.setTransformer = _functools.update_wrapper(__QgsProperty_setTransformer_wrapper, QgsProperty.setTransformer)

except (NameError, AttributeError):
    pass
