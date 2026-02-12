# The following has been generated automatically from src/core/qgssimplifymethod.h
QgsSimplifyMethod.NoSimplification = QgsSimplifyMethod.MethodType.NoSimplification
QgsSimplifyMethod.OptimizeForRendering = QgsSimplifyMethod.MethodType.OptimizeForRendering
QgsSimplifyMethod.PreserveTopology = QgsSimplifyMethod.MethodType.PreserveTopology
try:
    QgsSimplifyMethod.createGeometrySimplifier = staticmethod(QgsSimplifyMethod.createGeometrySimplifier)
except (NameError, AttributeError):
    pass
