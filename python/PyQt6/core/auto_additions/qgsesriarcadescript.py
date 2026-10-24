# The following has been generated automatically from src/core/providers/arcgis/qgsesriarcadescript.h
# monkey patching scoped based enum
QgsEsriArcadeNode.NodeType.BlockStatement.__doc__ = "Block statement (a list of statements grouped together in a block)"
QgsEsriArcadeNode.NodeType.VariableDeclarationStatement.__doc__ = "Variable declaration statement"
QgsEsriArcadeNode.NodeType.ReturnStatement.__doc__ = "Return statement"
QgsEsriArcadeNode.NodeType.AssignmentStatement.__doc__ = "Variable assignment statement"
QgsEsriArcadeNode.NodeType.FunctionDeclarationStatement.__doc__ = "Function declaration"
QgsEsriArcadeNode.NodeType.ExpressionStatement.__doc__ = "Expression"
QgsEsriArcadeNode.NodeType.LiteralExpression.__doc__ = "Literal"
QgsEsriArcadeNode.NodeType.VariableReferenceExpression.__doc__ = "Reference to a standard variable"
QgsEsriArcadeNode.NodeType.ProfileVariableReferenceExpression.__doc__ = "Reference to a profile variable (eg ``$feature``)"
QgsEsriArcadeNode.NodeType.MemberAccessExpression.__doc__ = "Member access via bracket or notation (eg ``$feature.ATTRIBUTE`` or ``$feature[\"ATTRIBUTE\"]``)"
QgsEsriArcadeNode.NodeType.BinaryOperatorExpression.__doc__ = "Binary Operator"
QgsEsriArcadeNode.NodeType.UnaryOperatorExpression.__doc__ = "Operator"
QgsEsriArcadeNode.NodeType.FunctionCallExpression.__doc__ = "Function call"
QgsEsriArcadeNode.NodeType.IndexExpression.__doc__ = "Index operator"
QgsEsriArcadeNode.NodeType.ArrayLiteralExpression.__doc__ = "A literal array, where the element values may be expressions themselves (eg ``[1, 2, 3/4]``)"
QgsEsriArcadeNode.NodeType.DictionaryLiteralExpression.__doc__ = "A literal dictionary, eg ``{key: 1, other_key: 2}``"
QgsEsriArcadeNode.NodeType.IfStatement.__doc__ = "If statement"
QgsEsriArcadeNode.NodeType.ForStatement.__doc__ = "For loop"
QgsEsriArcadeNode.NodeType.ForInStatement.__doc__ = "For x in y loop"
QgsEsriArcadeNode.NodeType.WhileStatement.__doc__ = "While loop"
QgsEsriArcadeNode.NodeType.BreakStatement.__doc__ = "Break statement"
QgsEsriArcadeNode.NodeType.ContinueStatement.__doc__ = "Continue statement"
QgsEsriArcadeNode.NodeType.__doc__ = """Node type identifier.

* ``BlockStatement``: Block statement (a list of statements grouped together in a block)
* ``VariableDeclarationStatement``: Variable declaration statement
* ``ReturnStatement``: Return statement
* ``AssignmentStatement``: Variable assignment statement
* ``FunctionDeclarationStatement``: Function declaration
* ``ExpressionStatement``: Expression
* ``LiteralExpression``: Literal
* ``VariableReferenceExpression``: Reference to a standard variable
* ``ProfileVariableReferenceExpression``: Reference to a profile variable (eg ``$feature``)
* ``MemberAccessExpression``: Member access via bracket or notation (eg ``$feature.ATTRIBUTE`` or ``$feature[\"ATTRIBUTE\"]``)
* ``BinaryOperatorExpression``: Binary Operator
* ``UnaryOperatorExpression``: Operator
* ``FunctionCallExpression``: Function call
* ``IndexExpression``: Index operator
* ``ArrayLiteralExpression``: A literal array, where the element values may be expressions themselves (eg ``[1, 2, 3/4]``)
* ``DictionaryLiteralExpression``: A literal dictionary, eg ``{key: 1, other_key: 2}``
* ``IfStatement``: If statement
* ``ForStatement``: For loop
* ``ForInStatement``: For x in y loop
* ``WhileStatement``: While loop
* ``BreakStatement``: Break statement
* ``ContinueStatement``: Continue statement

"""
# --
# monkey patching scoped based enum
QgsEsriArcadeAssignmentStatement.AssignmentOperator.Assign.__doc__ = "Standard assignment ``a = 10``"
QgsEsriArcadeAssignmentStatement.AssignmentOperator.PlusAssign.__doc__ = "Add and assign ``a += 10``"
QgsEsriArcadeAssignmentStatement.AssignmentOperator.MinusAssign.__doc__ = "Subtract and assign ``a -= 10``"
QgsEsriArcadeAssignmentStatement.AssignmentOperator.MultiplyAssign.__doc__ = "Multiply and assign ``a *= 10``"
QgsEsriArcadeAssignmentStatement.AssignmentOperator.DivideAssign.__doc__ = "Divide and assign ``a /= 10``"
QgsEsriArcadeAssignmentStatement.AssignmentOperator.__doc__ = """Assignment operator types.

* ``Assign``: Standard assignment ``a = 10``
* ``PlusAssign``: Add and assign ``a += 10``
* ``MinusAssign``: Subtract and assign ``a -= 10``
* ``MultiplyAssign``: Multiply and assign ``a *= 10``
* ``DivideAssign``: Divide and assign ``a /= 10``

"""
# --
# monkey patching scoped based enum
QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.Plus.__doc__ = "Addition +"
QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.Minus.__doc__ = "Subtraction -"
QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.Multiply.__doc__ = "Multiplication *"
QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.Divide.__doc__ = "Division /"
QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.Modulo.__doc__ = "Modulo %"
QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.Equal.__doc__ = "Equality test =="
QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.NotEqual.__doc__ = "Not equal test !="
QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.LessThan.__doc__ = "Less than <"
QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.GreaterThan.__doc__ = "Greater than >"
QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.LessThanOrEqual.__doc__ = "Less than or equal to <="
QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.GreaterThanOrEqual.__doc__ = "Greater than or equal to >="
QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.LogicalAnd.__doc__ = "Logical and &&"
QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.LogicalOr.__doc__ = "Logical or ||"
QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.BitwiseAnd.__doc__ = "Bitwise and, eg \"7 & 14\""
QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.BitwiseOr.__doc__ = "Bitwise or, eg \"7 | 14\""
QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.BitwiseXor.__doc__ = "Bitwise xor, eg \"7 ^ 14\""
QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.BitwiseLeftShift.__doc__ = "Bitwise left shift, eg \"7 << 14\""
QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.BitwiseSignedRightShift.__doc__ = "Bitwise right shift, eg \"7 >> 14\""
QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.BitwiseUnsignedRightShift.__doc__ = "Bitwise unsigned right shift, eg \"7 >>> 14\""
QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.__doc__ = """Binary operator enumeration.

* ``Plus``: Addition +
* ``Minus``: Subtraction -
* ``Multiply``: Multiplication *
* ``Divide``: Division /
* ``Modulo``: Modulo %
* ``Equal``: Equality test ==
* ``NotEqual``: Not equal test !=
* ``LessThan``: Less than <
* ``GreaterThan``: Greater than >
* ``LessThanOrEqual``: Less than or equal to <=
* ``GreaterThanOrEqual``: Greater than or equal to >=
* ``LogicalAnd``: Logical and &&
* ``LogicalOr``: Logical or ||
* ``BitwiseAnd``: Bitwise and, eg \"7 & 14\"
* ``BitwiseOr``: Bitwise or, eg \"7 | 14\"
* ``BitwiseXor``: Bitwise xor, eg \"7 ^ 14\"
* ``BitwiseLeftShift``: Bitwise left shift, eg \"7 << 14\"
* ``BitwiseSignedRightShift``: Bitwise right shift, eg \"7 >> 14\"
* ``BitwiseUnsignedRightShift``: Bitwise unsigned right shift, eg \"7 >>> 14\"

"""
# --
# monkey patching scoped based enum
QgsEsriArcadeUnaryOperatorExpression.UnaryOperator.Not.__doc__ = "Not operator, !"
QgsEsriArcadeUnaryOperatorExpression.UnaryOperator.Minus.__doc__ = "Minus operator, -"
QgsEsriArcadeUnaryOperatorExpression.UnaryOperator.Plus.__doc__ = "Plus operator, +"
QgsEsriArcadeUnaryOperatorExpression.UnaryOperator.BitwiseNot.__doc__ = "Bitwise not, eg \"~7\""
QgsEsriArcadeUnaryOperatorExpression.UnaryOperator.PreIncrement.__doc__ = "Preincrement operator, eg \"++x\""
QgsEsriArcadeUnaryOperatorExpression.UnaryOperator.PreDecrement.__doc__ = "Prederement operator, eg \"--x\""
QgsEsriArcadeUnaryOperatorExpression.UnaryOperator.PostIncrement.__doc__ = "Postincrement operator, eg \"x++\""
QgsEsriArcadeUnaryOperatorExpression.UnaryOperator.PostDecrement.__doc__ = "Postdecrement operator, eg \"x--\""
QgsEsriArcadeUnaryOperatorExpression.UnaryOperator.__doc__ = """Unary operator enumeration.

* ``Not``: Not operator, !
* ``Minus``: Minus operator, -
* ``Plus``: Plus operator, +
* ``BitwiseNot``: Bitwise not, eg \"~7\"
* ``PreIncrement``: Preincrement operator, eg \"++x\"
* ``PreDecrement``: Prederement operator, eg \"--x\"
* ``PostIncrement``: Postincrement operator, eg \"x++\"
* ``PostDecrement``: Postdecrement operator, eg \"x--\"

"""
# --
# monkey patching scoped based enum
QgsEsriArcadeForInStatement.IterationType.ForIn.__doc__ = "For x in y loop"
QgsEsriArcadeForInStatement.IterationType.ForOf.__doc__ = "For x of y loop"
QgsEsriArcadeForInStatement.IterationType.__doc__ = """Type of iteration loop.

* ``ForIn``: For x in y loop
* ``ForOf``: For x of y loop

"""
# --
try:
    QgsEsriArcadeScript.quotedString = staticmethod(QgsEsriArcadeScript.quotedString)
    QgsEsriArcadeScript.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsEsriArcadeNode.__abstract_methods__ = ['nodeType', 'dump', 'clone', 'accept']
    QgsEsriArcadeNode.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsEsriArcadeVisitor.__abstract_methods__ = ['visit']
    QgsEsriArcadeVisitor.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsEsriArcadeBlockStatement.__overridden_methods__ = ['nodeType', 'dump', 'clone', 'accept']
    QgsEsriArcadeBlockStatement.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsEsriArcadeVariableDeclarationStatement.__overridden_methods__ = ['nodeType', 'dump', 'clone', 'accept']
    QgsEsriArcadeVariableDeclarationStatement.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsEsriArcadeAssignmentStatement.__overridden_methods__ = ['nodeType', 'dump', 'clone', 'accept']
    QgsEsriArcadeAssignmentStatement.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsEsriArcadeReturnStatement.__overridden_methods__ = ['nodeType', 'dump', 'clone', 'accept']
    QgsEsriArcadeReturnStatement.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsEsriArcadeFunctionDeclarationStatement.__overridden_methods__ = ['nodeType', 'dump', 'clone', 'accept']
    QgsEsriArcadeFunctionDeclarationStatement.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsEsriArcadeExpressionStatement.__overridden_methods__ = ['nodeType', 'dump', 'clone', 'accept']
    QgsEsriArcadeExpressionStatement.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsEsriArcadeLiteralExpression.__overridden_methods__ = ['nodeType', 'dump', 'clone', 'accept']
    QgsEsriArcadeLiteralExpression.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsEsriArcadeVariableReferenceExpression.__overridden_methods__ = ['nodeType', 'dump', 'clone', 'accept']
    QgsEsriArcadeVariableReferenceExpression.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsEsriArcadeProfileVariableReferenceExpression.__overridden_methods__ = ['nodeType', 'dump', 'clone', 'accept']
    QgsEsriArcadeProfileVariableReferenceExpression.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsEsriArcadeMemberAccessExpression.__overridden_methods__ = ['nodeType', 'dump', 'clone', 'accept']
    QgsEsriArcadeMemberAccessExpression.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsEsriArcadeBinaryOperatorExpression.__overridden_methods__ = ['nodeType', 'dump', 'clone', 'accept']
    QgsEsriArcadeBinaryOperatorExpression.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsEsriArcadeUnaryOperatorExpression.__overridden_methods__ = ['nodeType', 'dump', 'clone', 'accept']
    QgsEsriArcadeUnaryOperatorExpression.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsEsriArcadeFunctionCallExpression.__overridden_methods__ = ['nodeType', 'dump', 'clone', 'accept']
    QgsEsriArcadeFunctionCallExpression.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsEsriArcadeIndexExpression.__overridden_methods__ = ['nodeType', 'dump', 'clone', 'accept']
    QgsEsriArcadeIndexExpression.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsEsriArcadeArrayLiteralExpression.__overridden_methods__ = ['nodeType', 'dump', 'clone', 'accept']
    QgsEsriArcadeArrayLiteralExpression.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsEsriArcadeDictionaryLiteralExpression.__overridden_methods__ = ['nodeType', 'dump', 'clone', 'accept']
    QgsEsriArcadeDictionaryLiteralExpression.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsEsriArcadeIfStatement.__overridden_methods__ = ['nodeType', 'dump', 'clone', 'accept']
    QgsEsriArcadeIfStatement.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsEsriArcadeForStatement.__overridden_methods__ = ['nodeType', 'dump', 'clone', 'accept']
    QgsEsriArcadeForStatement.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsEsriArcadeForInStatement.__overridden_methods__ = ['nodeType', 'dump', 'clone', 'accept']
    QgsEsriArcadeForInStatement.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsEsriArcadeWhileStatement.__overridden_methods__ = ['nodeType', 'dump', 'clone', 'accept']
    QgsEsriArcadeWhileStatement.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsEsriArcadeBreakStatement.__overridden_methods__ = ['nodeType', 'dump', 'clone', 'accept']
    QgsEsriArcadeBreakStatement.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsEsriArcadeContinueStatement.__overridden_methods__ = ['nodeType', 'dump', 'clone', 'accept']
    QgsEsriArcadeContinueStatement.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsEsriArcadeRecursiveVisitor.__overridden_methods__ = ['visit']
    QgsEsriArcadeRecursiveVisitor.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsEsriArcadeStatement.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsEsriArcadeExpression.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsEsriArcadeDictionaryElement.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
