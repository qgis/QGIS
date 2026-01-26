# The following has been generated automatically from src/core/qgspropertytransformer.h
try:
    QgsPropertyTransformer.create = staticmethod(QgsPropertyTransformer.create)
    QgsPropertyTransformer.fromExpression = staticmethod(QgsPropertyTransformer.fromExpression)
    QgsPropertyTransformer.__virtual_methods__ = ['loadVariant', 'toVariant']
    QgsPropertyTransformer.__abstract_methods__ = ['transformerType', 'clone', 'transform', 'toExpression']
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
except (NameError, AttributeError):
    pass
