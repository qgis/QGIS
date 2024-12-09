# The following has been generated automatically from src/core/qgspropertytransformer.h
QgsPropertyTransformer.GenericNumericTransformer = QgsPropertyTransformer.Type.GenericNumericTransformer
QgsPropertyTransformer.SizeScaleTransformer = QgsPropertyTransformer.Type.SizeScaleTransformer
QgsPropertyTransformer.ColorRampTransformer = QgsPropertyTransformer.Type.ColorRampTransformer
QgsSizeScaleTransformer.Linear = QgsSizeScaleTransformer.ScaleType.Linear
QgsSizeScaleTransformer.Area = QgsSizeScaleTransformer.ScaleType.Area
QgsSizeScaleTransformer.Flannery = QgsSizeScaleTransformer.ScaleType.Flannery
QgsSizeScaleTransformer.Exponential = QgsSizeScaleTransformer.ScaleType.Exponential
try:
    QgsPropertyTransformer.create = staticmethod(QgsPropertyTransformer.create)
    QgsPropertyTransformer.fromExpression = staticmethod(QgsPropertyTransformer.fromExpression)
except (NameError, AttributeError):
    pass
try:
    QgsGenericNumericTransformer.fromExpression = staticmethod(QgsGenericNumericTransformer.fromExpression)
except (NameError, AttributeError):
    pass
try:
    QgsSizeScaleTransformer.fromExpression = staticmethod(QgsSizeScaleTransformer.fromExpression)
except (NameError, AttributeError):
    pass
