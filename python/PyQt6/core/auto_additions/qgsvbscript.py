# The following has been generated automatically from src/core/providers/arcgis/qgsvbscript.h
# monkey patching scoped based enum
QgsVbScriptNode.NodeType.BlockStatement.__doc__ = "Block statement (a list of statements grouped together in a block)"
QgsVbScriptNode.NodeType.VariableDeclarationStatement.__doc__ = "Variable declaration statement"
QgsVbScriptNode.NodeType.AssignmentStatement.__doc__ = "Variable assignment statement"
QgsVbScriptNode.NodeType.FunctionDeclarationStatement.__doc__ = "Function declaration"
QgsVbScriptNode.NodeType.ExpressionStatement.__doc__ = "Expression"
QgsVbScriptNode.NodeType.LiteralExpression.__doc__ = "Literal"
QgsVbScriptNode.NodeType.VariableReferenceExpression.__doc__ = "Reference to a standard variable"
QgsVbScriptNode.NodeType.MemberAccessExpression.__doc__ = "Member access via bracket or notation (eg ``$feature.ATTRIBUTE`` or ``$feature[\"ATTRIBUTE\"]``)"
QgsVbScriptNode.NodeType.BinaryOperatorExpression.__doc__ = "Binary Operator"
QgsVbScriptNode.NodeType.UnaryOperatorExpression.__doc__ = "Operator"
QgsVbScriptNode.NodeType.FunctionCallExpression.__doc__ = "Function call"
QgsVbScriptNode.NodeType.IndexExpression.__doc__ = "Index operator"
QgsVbScriptNode.NodeType.ArrayLiteralExpression.__doc__ = "A literal array, where the element values may be expressions themselves (eg ``[1, 2, 3/4]``)"
QgsVbScriptNode.NodeType.IfStatement.__doc__ = "If statement"
QgsVbScriptNode.NodeType.ForStatement.__doc__ = "For loop"
QgsVbScriptNode.NodeType.ForEachStatement.__doc__ = "For each loop"
QgsVbScriptNode.NodeType.WhileStatement.__doc__ = "While loop"
QgsVbScriptNode.NodeType.ExitStatement.__doc__ = "Exit loop statement"
QgsVbScriptNode.NodeType.FieldReferenceExpression.__doc__ = "Special ESRI specific reference to a layer's field (eg ``[FIELD_NAME]``)"
QgsVbScriptNode.NodeType.__doc__ = """Node type identifier.

* ``BlockStatement``: Block statement (a list of statements grouped together in a block)
* ``VariableDeclarationStatement``: Variable declaration statement
* ``AssignmentStatement``: Variable assignment statement
* ``FunctionDeclarationStatement``: Function declaration
* ``ExpressionStatement``: Expression
* ``LiteralExpression``: Literal
* ``VariableReferenceExpression``: Reference to a standard variable
* ``MemberAccessExpression``: Member access via bracket or notation (eg ``$feature.ATTRIBUTE`` or ``$feature[\"ATTRIBUTE\"]``)
* ``BinaryOperatorExpression``: Binary Operator
* ``UnaryOperatorExpression``: Operator
* ``FunctionCallExpression``: Function call
* ``IndexExpression``: Index operator
* ``ArrayLiteralExpression``: A literal array, where the element values may be expressions themselves (eg ``[1, 2, 3/4]``)
* ``IfStatement``: If statement
* ``ForStatement``: For loop
* ``ForEachStatement``: For each loop
* ``WhileStatement``: While loop
* ``ExitStatement``: Exit loop statement
* ``FieldReferenceExpression``: Special ESRI specific reference to a layer's field (eg ``[FIELD_NAME]``)

"""
# --
# monkey patching scoped based enum
QgsVbScriptBinaryOperatorExpression.BinaryOperator.Plus.__doc__ = "Addition +"
QgsVbScriptBinaryOperatorExpression.BinaryOperator.Minus.__doc__ = "Subtraction -"
QgsVbScriptBinaryOperatorExpression.BinaryOperator.Multiply.__doc__ = "Multiplication *"
QgsVbScriptBinaryOperatorExpression.BinaryOperator.Divide.__doc__ = "Division /"
QgsVbScriptBinaryOperatorExpression.BinaryOperator.IntegerDivide.__doc__ = "Integer division"
QgsVbScriptBinaryOperatorExpression.BinaryOperator.Modulo.__doc__ = "Modulo %"
QgsVbScriptBinaryOperatorExpression.BinaryOperator.Exponent.__doc__ = "Exponential ^"
QgsVbScriptBinaryOperatorExpression.BinaryOperator.Concat.__doc__ = "String concatenation &"
QgsVbScriptBinaryOperatorExpression.BinaryOperator.Equal.__doc__ = "Equality test =="
QgsVbScriptBinaryOperatorExpression.BinaryOperator.NotEqual.__doc__ = "Not equal test !="
QgsVbScriptBinaryOperatorExpression.BinaryOperator.LessThan.__doc__ = "Less than <"
QgsVbScriptBinaryOperatorExpression.BinaryOperator.GreaterThan.__doc__ = "Greater than >"
QgsVbScriptBinaryOperatorExpression.BinaryOperator.LessThanOrEqual.__doc__ = "Less than or equal to <="
QgsVbScriptBinaryOperatorExpression.BinaryOperator.GreaterThanOrEqual.__doc__ = "Greater than or equal to >="
QgsVbScriptBinaryOperatorExpression.BinaryOperator.Is.__doc__ = "Is comparison"
QgsVbScriptBinaryOperatorExpression.BinaryOperator.LogicalAnd.__doc__ = "Logical and"
QgsVbScriptBinaryOperatorExpression.BinaryOperator.LogicalOr.__doc__ = "Logical or"
QgsVbScriptBinaryOperatorExpression.BinaryOperator.LogicalXor.__doc__ = "Logical xor"
QgsVbScriptBinaryOperatorExpression.BinaryOperator.LogicalImp.__doc__ = "Logical imp"
QgsVbScriptBinaryOperatorExpression.BinaryOperator.LogicalEqv.__doc__ = "Logical eqv"
QgsVbScriptBinaryOperatorExpression.BinaryOperator.__doc__ = """Binary operator enumeration.

* ``Plus``: Addition +
* ``Minus``: Subtraction -
* ``Multiply``: Multiplication *
* ``Divide``: Division /
* ``IntegerDivide``: Integer division
* ``Modulo``: Modulo %
* ``Exponent``: Exponential ^
* ``Concat``: String concatenation &
* ``Equal``: Equality test ==
* ``NotEqual``: Not equal test !=
* ``LessThan``: Less than <
* ``GreaterThan``: Greater than >
* ``LessThanOrEqual``: Less than or equal to <=
* ``GreaterThanOrEqual``: Greater than or equal to >=
* ``Is``: Is comparison
* ``LogicalAnd``: Logical and
* ``LogicalOr``: Logical or
* ``LogicalXor``: Logical xor
* ``LogicalImp``: Logical imp
* ``LogicalEqv``: Logical eqv

"""
# --
# monkey patching scoped based enum
QgsVbScriptUnaryOperatorExpression.UnaryOperator.Not.__doc__ = "Logical Not"
QgsVbScriptUnaryOperatorExpression.UnaryOperator.Minus.__doc__ = "Minus operator, -"
QgsVbScriptUnaryOperatorExpression.UnaryOperator.Plus.__doc__ = "Plus operator, +"
QgsVbScriptUnaryOperatorExpression.UnaryOperator.__doc__ = """Unary operator enumeration.

* ``Not``: Logical Not
* ``Minus``: Minus operator, -
* ``Plus``: Plus operator, +

"""
# --
# monkey patching scoped based enum
QgsVbScriptWhileStatement.LoopType.WhileWend.__doc__ = "While ... Wend loop"
QgsVbScriptWhileStatement.LoopType.DoWhile.__doc__ = "Do While ... Loop"
QgsVbScriptWhileStatement.LoopType.DoUntil.__doc__ = "Do Until ... Loop"
QgsVbScriptWhileStatement.LoopType.DoLoopWhile.__doc__ = "Do ... Loop While"
QgsVbScriptWhileStatement.LoopType.DoLoopUntil.__doc__ = "Do ... Loop Until"
QgsVbScriptWhileStatement.LoopType.__doc__ = """Type of while or do loop.

* ``WhileWend``: While ... Wend loop
* ``DoWhile``: Do While ... Loop
* ``DoUntil``: Do Until ... Loop
* ``DoLoopWhile``: Do ... Loop While
* ``DoLoopUntil``: Do ... Loop Until

"""
# --
# monkey patching scoped based enum
QgsVbScriptExitStatement.ExitType.ExitDo.__doc__ = "Exit Do"
QgsVbScriptExitStatement.ExitType.ExitFor.__doc__ = "Exit For"
QgsVbScriptExitStatement.ExitType.ExitFunction.__doc__ = "Exit Function"
QgsVbScriptExitStatement.ExitType.ExitSub.__doc__ = "Exit Sub"
QgsVbScriptExitStatement.ExitType.__doc__ = """Object to exit from.

* ``ExitDo``: Exit Do
* ``ExitFor``: Exit For
* ``ExitFunction``: Exit Function
* ``ExitSub``: Exit Sub

"""
# --
try:
    QgsVbScript.quotedString = staticmethod(QgsVbScript.quotedString)
    QgsVbScript.quotedFieldReference = staticmethod(QgsVbScript.quotedFieldReference)
    QgsVbScript.extractFieldReference = staticmethod(QgsVbScript.extractFieldReference)
    QgsVbScript.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsVbScriptNode.__abstract_methods__ = ['nodeType', 'dump', 'clone', 'accept']
    QgsVbScriptNode.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsVbScriptVisitor.__abstract_methods__ = ['visit']
    QgsVbScriptVisitor.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsVbScriptBlockStatement.__overridden_methods__ = ['nodeType', 'dump', 'clone', 'accept']
    QgsVbScriptBlockStatement.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsVbScriptVariableDeclarationStatement.__overridden_methods__ = ['nodeType', 'dump', 'clone', 'accept']
    QgsVbScriptVariableDeclarationStatement.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsVbScriptAssignmentStatement.__overridden_methods__ = ['nodeType', 'dump', 'clone', 'accept']
    QgsVbScriptAssignmentStatement.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsVbScriptFunctionDeclarationStatement.__overridden_methods__ = ['nodeType', 'dump', 'clone', 'accept']
    QgsVbScriptFunctionDeclarationStatement.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsVbScriptExpressionStatement.__overridden_methods__ = ['nodeType', 'dump', 'clone', 'accept']
    QgsVbScriptExpressionStatement.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsVbScriptLiteralExpression.__overridden_methods__ = ['nodeType', 'dump', 'clone', 'accept']
    QgsVbScriptLiteralExpression.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsVbScriptVariableReferenceExpression.__overridden_methods__ = ['nodeType', 'dump', 'clone', 'accept']
    QgsVbScriptVariableReferenceExpression.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsVbScriptMemberAccessExpression.__overridden_methods__ = ['nodeType', 'dump', 'clone', 'accept']
    QgsVbScriptMemberAccessExpression.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsVbScriptBinaryOperatorExpression.__overridden_methods__ = ['nodeType', 'dump', 'clone', 'accept']
    QgsVbScriptBinaryOperatorExpression.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsVbScriptUnaryOperatorExpression.__overridden_methods__ = ['nodeType', 'dump', 'clone', 'accept']
    QgsVbScriptUnaryOperatorExpression.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsVbScriptFunctionCallExpression.__overridden_methods__ = ['nodeType', 'dump', 'clone', 'accept']
    QgsVbScriptFunctionCallExpression.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsVbScriptIndexExpression.__overridden_methods__ = ['nodeType', 'dump', 'clone', 'accept']
    QgsVbScriptIndexExpression.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsVbScriptArrayLiteralExpression.__overridden_methods__ = ['nodeType', 'dump', 'clone', 'accept']
    QgsVbScriptArrayLiteralExpression.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsVbScriptIfStatement.__overridden_methods__ = ['nodeType', 'dump', 'clone', 'accept']
    QgsVbScriptIfStatement.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsVbScriptForStatement.__overridden_methods__ = ['nodeType', 'dump', 'clone', 'accept']
    QgsVbScriptForStatement.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsVbScriptForEachStatement.__overridden_methods__ = ['nodeType', 'dump', 'clone', 'accept']
    QgsVbScriptForEachStatement.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsVbScriptWhileStatement.__overridden_methods__ = ['nodeType', 'dump', 'clone', 'accept']
    QgsVbScriptWhileStatement.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsVbScriptExitStatement.__overridden_methods__ = ['nodeType', 'dump', 'clone', 'accept']
    QgsVbScriptExitStatement.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsVbScriptFieldReferenceExpression.__overridden_methods__ = ['nodeType', 'dump', 'clone', 'accept']
    QgsVbScriptFieldReferenceExpression.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsVbScriptRecursiveVisitor.__overridden_methods__ = ['visit']
    QgsVbScriptRecursiveVisitor.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsVbScriptStatement.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsVbScriptExpression.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
