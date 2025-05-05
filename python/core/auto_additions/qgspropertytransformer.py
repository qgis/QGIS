# The following has been generated automatically from src/core/qgspropertytransformer.h
try:
    QgsPropertyTransformer.create = staticmethod(QgsPropertyTransformer.create)
    QgsPropertyTransformer.fromExpression = staticmethod(QgsPropertyTransformer.fromExpression)
    QgsPropertyTransformer.__virtual_methods__ = ['loadVariant', 'toVariant']
    QgsPropertyTransformer.__abstract_methods__ = ['transformerType', 'clone', 'transform', 'toExpression']
    import functools as _functools
    __wrapped_QgsPropertyTransformer_setCurveTransform = QgsPropertyTransformer.setCurveTransform
    def __QgsPropertyTransformer_setCurveTransform_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsPropertyTransformer_setCurveTransform(self, arg)
    QgsPropertyTransformer.setCurveTransform = _functools.update_wrapper(__QgsPropertyTransformer_setCurveTransform_wrapper, QgsPropertyTransformer.setCurveTransform)

except (NameError, AttributeError):
    pass
try:
    QgsGenericNumericTransformer.fromExpression = staticmethod(QgsGenericNumericTransformer.fromExpression)
    QgsGenericNumericTransformer.__overridden_methods__ = ['transformerType', 'clone', 'toVariant', 'loadVariant', 'transform', 'toExpression']
except (NameError, AttributeError):
    pass
try:
    QgsSizeScaleTransformer.fromExpression = staticmethod(QgsSizeScaleTransformer.fromExpression)
    QgsSizeScaleTransformer.__overridden_methods__ = ['transformerType', 'clone', 'toVariant', 'loadVariant', 'transform', 'toExpression']
except (NameError, AttributeError):
    pass
try:
    QgsColorRampTransformer.__overridden_methods__ = ['transformerType', 'clone', 'toVariant', 'loadVariant', 'transform', 'toExpression']
    import functools as _functools
    __wrapped_QgsColorRampTransformer_QgsColorRampTransformer = QgsColorRampTransformer.QgsColorRampTransformer
    def __QgsColorRampTransformer_QgsColorRampTransformer_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsColorRampTransformer_QgsColorRampTransformer(self, arg)
    QgsColorRampTransformer.QgsColorRampTransformer = _functools.update_wrapper(__QgsColorRampTransformer_QgsColorRampTransformer_wrapper, QgsColorRampTransformer.QgsColorRampTransformer)

    import functools as _functools
    __wrapped_QgsColorRampTransformer_setColorRamp = QgsColorRampTransformer.setColorRamp
    def __QgsColorRampTransformer_setColorRamp_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsColorRampTransformer_setColorRamp(self, arg)
    QgsColorRampTransformer.setColorRamp = _functools.update_wrapper(__QgsColorRampTransformer_setColorRamp_wrapper, QgsColorRampTransformer.setColorRamp)

except (NameError, AttributeError):
    pass
