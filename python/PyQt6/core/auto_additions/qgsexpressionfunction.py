# The following has been generated automatically from src/core/expression/qgsexpressionfunction.h
try:
    QgsExpressionFunction.allParamsStatic = staticmethod(QgsExpressionFunction.allParamsStatic)
    QgsExpressionFunction.__virtual_methods__ = ['usesGeometry', 'aliases', 'isStatic', 'prepare', 'referencedColumns', 'isDeprecated', 'run', 'handlesNull']
    QgsExpressionFunction.__abstract_methods__ = ['func']
    QgsExpressionFunction.__group__ = ['expression']
except (NameError, AttributeError):
    pass
try:
    QgsExpressionFunction.Parameter.__group__ = ['expression']
except (NameError, AttributeError):
    pass
