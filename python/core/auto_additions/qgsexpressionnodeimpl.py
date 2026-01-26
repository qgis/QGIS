# The following has been generated automatically from src/core/expression/qgsexpressionnodeimpl.h
try:
    QgsExpressionNodeFunction.validateParams = staticmethod(QgsExpressionNodeFunction.validateParams)
    QgsExpressionNodeFunction.__overridden_methods__ = ['nodeType', 'prepareNode', 'evalNode', 'dump', 'referencedColumns', 'referencedVariables', 'referencedFunctions', 'clone', 'isStatic']
    QgsExpressionNodeFunction.__group__ = ['expression']
except (NameError, AttributeError):
    pass
try:
    QgsExpressionNodeUnaryOperator.__overridden_methods__ = ['nodeType', 'prepareNode', 'evalNode', 'dump', 'referencedColumns', 'referencedVariables', 'referencedFunctions', 'clone', 'isStatic']
    QgsExpressionNodeUnaryOperator.__group__ = ['expression']
except (NameError, AttributeError):
    pass
try:
    QgsExpressionNodeBinaryOperator.__overridden_methods__ = ['nodeType', 'prepareNode', 'evalNode', 'dump', 'referencedColumns', 'referencedVariables', 'referencedFunctions', 'needsGeometry', 'clone', 'isStatic']
    QgsExpressionNodeBinaryOperator.__group__ = ['expression']
except (NameError, AttributeError):
    pass
try:
    QgsExpressionNodeIndexOperator.__overridden_methods__ = ['nodeType', 'prepareNode', 'evalNode', 'dump', 'referencedColumns', 'referencedVariables', 'referencedFunctions', 'needsGeometry', 'clone', 'isStatic']
    QgsExpressionNodeIndexOperator.__group__ = ['expression']
except (NameError, AttributeError):
    pass
try:
    QgsExpressionNodeBetweenOperator.__overridden_methods__ = ['nodeType', 'prepareNode', 'evalNode', 'dump', 'referencedColumns', 'referencedVariables', 'referencedFunctions', 'clone', 'isStatic']
    QgsExpressionNodeBetweenOperator.__group__ = ['expression']
except (NameError, AttributeError):
    pass
try:
    QgsExpressionNodeInOperator.__overridden_methods__ = ['nodeType', 'prepareNode', 'evalNode', 'dump', 'referencedColumns', 'referencedVariables', 'referencedFunctions', 'clone', 'isStatic']
    QgsExpressionNodeInOperator.__group__ = ['expression']
except (NameError, AttributeError):
    pass
try:
    QgsExpressionNodeLiteral.__overridden_methods__ = ['nodeType', 'prepareNode', 'evalNode', 'dump', 'referencedColumns', 'referencedVariables', 'referencedFunctions', 'clone', 'isStatic']
    QgsExpressionNodeLiteral.__group__ = ['expression']
except (NameError, AttributeError):
    pass
try:
    QgsExpressionNodeColumnRef.__overridden_methods__ = ['nodeType', 'prepareNode', 'evalNode', 'dump', 'referencedColumns', 'referencedVariables', 'referencedFunctions', 'needsGeometry', 'clone', 'isStatic']
    QgsExpressionNodeColumnRef.__group__ = ['expression']
except (NameError, AttributeError):
    pass
try:
    QgsExpressionNodeCondition.__overridden_methods__ = ['nodeType', 'evalNode', 'prepareNode', 'dump', 'referencedColumns', 'referencedVariables', 'referencedFunctions', 'needsGeometry', 'clone', 'isStatic']
    QgsExpressionNodeCondition.__group__ = ['expression']
except (NameError, AttributeError):
    pass
try:
    QgsExpressionNodeCondition.WhenThen.__group__ = ['expression']
except (NameError, AttributeError):
    pass
