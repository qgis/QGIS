# The following has been generated automatically from src/core/expression/qgsexpressionnodeimpl.h
QgsExpressionNodeUnaryOperator.uoNot = QgsExpressionNodeUnaryOperator.UnaryOperator.uoNot
QgsExpressionNodeUnaryOperator.uoMinus = QgsExpressionNodeUnaryOperator.UnaryOperator.uoMinus
QgsExpressionNodeBinaryOperator.boOr = QgsExpressionNodeBinaryOperator.BinaryOperator.boOr
QgsExpressionNodeBinaryOperator.boAnd = QgsExpressionNodeBinaryOperator.BinaryOperator.boAnd
QgsExpressionNodeBinaryOperator.boEQ = QgsExpressionNodeBinaryOperator.BinaryOperator.boEQ
QgsExpressionNodeBinaryOperator.boNE = QgsExpressionNodeBinaryOperator.BinaryOperator.boNE
QgsExpressionNodeBinaryOperator.boLE = QgsExpressionNodeBinaryOperator.BinaryOperator.boLE
QgsExpressionNodeBinaryOperator.boGE = QgsExpressionNodeBinaryOperator.BinaryOperator.boGE
QgsExpressionNodeBinaryOperator.boLT = QgsExpressionNodeBinaryOperator.BinaryOperator.boLT
QgsExpressionNodeBinaryOperator.boGT = QgsExpressionNodeBinaryOperator.BinaryOperator.boGT
QgsExpressionNodeBinaryOperator.boRegexp = QgsExpressionNodeBinaryOperator.BinaryOperator.boRegexp
QgsExpressionNodeBinaryOperator.boLike = QgsExpressionNodeBinaryOperator.BinaryOperator.boLike
QgsExpressionNodeBinaryOperator.boNotLike = QgsExpressionNodeBinaryOperator.BinaryOperator.boNotLike
QgsExpressionNodeBinaryOperator.boILike = QgsExpressionNodeBinaryOperator.BinaryOperator.boILike
QgsExpressionNodeBinaryOperator.boNotILike = QgsExpressionNodeBinaryOperator.BinaryOperator.boNotILike
QgsExpressionNodeBinaryOperator.boIs = QgsExpressionNodeBinaryOperator.BinaryOperator.boIs
QgsExpressionNodeBinaryOperator.boIsNot = QgsExpressionNodeBinaryOperator.BinaryOperator.boIsNot
QgsExpressionNodeBinaryOperator.boPlus = QgsExpressionNodeBinaryOperator.BinaryOperator.boPlus
QgsExpressionNodeBinaryOperator.boMinus = QgsExpressionNodeBinaryOperator.BinaryOperator.boMinus
QgsExpressionNodeBinaryOperator.boMul = QgsExpressionNodeBinaryOperator.BinaryOperator.boMul
QgsExpressionNodeBinaryOperator.boDiv = QgsExpressionNodeBinaryOperator.BinaryOperator.boDiv
QgsExpressionNodeBinaryOperator.boIntDiv = QgsExpressionNodeBinaryOperator.BinaryOperator.boIntDiv
QgsExpressionNodeBinaryOperator.boMod = QgsExpressionNodeBinaryOperator.BinaryOperator.boMod
QgsExpressionNodeBinaryOperator.boPow = QgsExpressionNodeBinaryOperator.BinaryOperator.boPow
QgsExpressionNodeBinaryOperator.boConcat = QgsExpressionNodeBinaryOperator.BinaryOperator.boConcat
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
