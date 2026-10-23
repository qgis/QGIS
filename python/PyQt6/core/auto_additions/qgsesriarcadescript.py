# The following has been generated automatically from src/core/providers/arcgis/qgsesriarcadescript.h
# monkey patching scoped based enum
QgsEsriArcadeNode.NodeType.ntBlockStatement.__doc__ = "Block statement (a list of statements grouped together in a block)"
QgsEsriArcadeNode.NodeType.ntVariableDeclarationStatement.__doc__ = "Variable declaration statement"
QgsEsriArcadeNode.NodeType.ntReturnStatement.__doc__ = "Return statement"
QgsEsriArcadeNode.NodeType.ntAssignmentStatement.__doc__ = "Variable assignment statement"
QgsEsriArcadeNode.NodeType.ntFunctionDeclarationStatement.__doc__ = "Function declaration"
QgsEsriArcadeNode.NodeType.ntExpressionStatement.__doc__ = "Expression"
QgsEsriArcadeNode.NodeType.ntLiteralExpression.__doc__ = "Literal"
QgsEsriArcadeNode.NodeType.ntVariableReferenceExpression.__doc__ = "Reference to a standard variable"
QgsEsriArcadeNode.NodeType.ntProfileVariableReferenceExpression.__doc__ = "Reference to a profile variable (eg $feature.ATTRIBUTE)"
QgsEsriArcadeNode.NodeType.ntMemberAccessExpression.__doc__ = "Member access via bracket notation (eg $feature[\"ATTRIBUTE\"])"
QgsEsriArcadeNode.NodeType.ntBinaryOperatorExpression.__doc__ = "Binary Operator"
QgsEsriArcadeNode.NodeType.ntUnaryOperatorExpression.__doc__ = "Operator"
QgsEsriArcadeNode.NodeType.ntFunctionCallExpression.__doc__ = "Function call"
QgsEsriArcadeNode.NodeType.ntIndexExpression.__doc__ = "Index operator"
QgsEsriArcadeNode.NodeType.ntArrayLiteralExpression.__doc__ = "A literal array, where the element values may be expressions themselves (eg [1, 2, 3/4])"
QgsEsriArcadeNode.NodeType.ntDictionaryLiteralExpression.__doc__ = "A literal dictionary, eg {key: 1, other_key: 2}"
QgsEsriArcadeNode.NodeType.__doc__ = """Node type identifier.

* ``ntBlockStatement``: Block statement (a list of statements grouped together in a block)
* ``ntVariableDeclarationStatement``: Variable declaration statement
* ``ntReturnStatement``: Return statement
* ``ntAssignmentStatement``: Variable assignment statement
* ``ntFunctionDeclarationStatement``: Function declaration
* ``ntExpressionStatement``: Expression
* ``ntLiteralExpression``: Literal
* ``ntVariableReferenceExpression``: Reference to a standard variable
* ``ntProfileVariableReferenceExpression``: Reference to a profile variable (eg $feature.ATTRIBUTE)
* ``ntMemberAccessExpression``: Member access via bracket notation (eg $feature[\"ATTRIBUTE\"])
* ``ntBinaryOperatorExpression``: Binary Operator
* ``ntUnaryOperatorExpression``: Operator
* ``ntFunctionCallExpression``: Function call
* ``ntIndexExpression``: Index operator
* ``ntArrayLiteralExpression``: A literal array, where the element values may be expressions themselves (eg [1, 2, 3/4])
* ``ntDictionaryLiteralExpression``: A literal dictionary, eg {key: 1, other_key: 2}

"""
# --
# monkey patching scoped based enum
QgsEsriArcadeAssignmentStatement.AssignmentOperator.aoAssign.__doc__ = "Standard assignment \"a = 10\""
QgsEsriArcadeAssignmentStatement.AssignmentOperator.aoPlusAssign.__doc__ = "Add and assign \"a += 10\""
QgsEsriArcadeAssignmentStatement.AssignmentOperator.aoMinusAssign.__doc__ = "Subtract and assign \"a -= 10\""
QgsEsriArcadeAssignmentStatement.AssignmentOperator.aoMultiplyAssign.__doc__ = "Multiply and assign \"a *= 10\""
QgsEsriArcadeAssignmentStatement.AssignmentOperator.aoDivideAssign.__doc__ = "Divide and assign \"a /= 10\""
QgsEsriArcadeAssignmentStatement.AssignmentOperator.__doc__ = """Assignment operator types.

* ``aoAssign``: Standard assignment \"a = 10\"
* ``aoPlusAssign``: Add and assign \"a += 10\"
* ``aoMinusAssign``: Subtract and assign \"a -= 10\"
* ``aoMultiplyAssign``: Multiply and assign \"a *= 10\"
* ``aoDivideAssign``: Divide and assign \"a /= 10\"

"""
# --
# monkey patching scoped based enum
QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.boPlus.__doc__ = ""
QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.boMinus.__doc__ = ""
QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.boMultiply.__doc__ = ""
QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.boDivide.__doc__ = ""
QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.boModulo.__doc__ = ""
QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.boEqual.__doc__ = ""
QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.boNotEqual.__doc__ = ""
QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.boLessThan.__doc__ = ""
QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.boGreaterThan.__doc__ = ""
QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.boLessThanOrEqual.__doc__ = ""
QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.boGreaterThanOrEqual.__doc__ = ""
QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.boLogicalAnd.__doc__ = ""
QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.boLogicalOr.__doc__ = ""
QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.boBitwiseAnd.__doc__ = "Bitwise and, eg \"7 & 14\""
QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.boBitwiseOr.__doc__ = "Bitwise or, eg \"7 | 14\""
QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.boBitwiseXor.__doc__ = "Bitwise xor, eg \"7 ^ 14\""
QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.boBitwiseLeftShift.__doc__ = "Bitwise left shift, eg \"7 << 14\""
QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.boBitwiseSignedRightShift.__doc__ = "Bitwise right shift, eg \"7 >> 14\""
QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.boBitwiseUnsignedRightShift.__doc__ = "Bitwise unsigned right shift, eg \"7 >>> 14\""
QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.__doc__ = """Binary operator enumeration.

* ``boPlus``: 
* ``boMinus``: 
* ``boMultiply``: 
* ``boDivide``: 
* ``boModulo``: 
* ``boEqual``: 
* ``boNotEqual``: 
* ``boLessThan``: 
* ``boGreaterThan``: 
* ``boLessThanOrEqual``: 
* ``boGreaterThanOrEqual``: 
* ``boLogicalAnd``: 
* ``boLogicalOr``: 
* ``boBitwiseAnd``: Bitwise and, eg \"7 & 14\"
* ``boBitwiseOr``: Bitwise or, eg \"7 | 14\"
* ``boBitwiseXor``: Bitwise xor, eg \"7 ^ 14\"
* ``boBitwiseLeftShift``: Bitwise left shift, eg \"7 << 14\"
* ``boBitwiseSignedRightShift``: Bitwise right shift, eg \"7 >> 14\"
* ``boBitwiseUnsignedRightShift``: Bitwise unsigned right shift, eg \"7 >>> 14\"

"""
# --
# monkey patching scoped based enum
QgsEsriArcadeUnaryOperatorExpression.UnaryOperator.uoNot.__doc__ = ""
QgsEsriArcadeUnaryOperatorExpression.UnaryOperator.uoMinus.__doc__ = ""
QgsEsriArcadeUnaryOperatorExpression.UnaryOperator.uoPlus.__doc__ = ""
QgsEsriArcadeUnaryOperatorExpression.UnaryOperator.uoBitwiseNot.__doc__ = "Bitwise not, eg \"~7\""
QgsEsriArcadeUnaryOperatorExpression.UnaryOperator.uoPreIncrement.__doc__ = "Preincrement operator, eg \"++x\""
QgsEsriArcadeUnaryOperatorExpression.UnaryOperator.uoPreDecrement.__doc__ = "Prederement operator, eg \"--x\""
QgsEsriArcadeUnaryOperatorExpression.UnaryOperator.uoPostIncrement.__doc__ = "Postincrement operator, eg \"x++\""
QgsEsriArcadeUnaryOperatorExpression.UnaryOperator.uoPostDecrement.__doc__ = "Postdecrement operator, eg \"x--\""
QgsEsriArcadeUnaryOperatorExpression.UnaryOperator.__doc__ = """Unary operator enumeration.

* ``uoNot``: 
* ``uoMinus``: 
* ``uoPlus``: 
* ``uoBitwiseNot``: Bitwise not, eg \"~7\"
* ``uoPreIncrement``: Preincrement operator, eg \"++x\"
* ``uoPreDecrement``: Prederement operator, eg \"--x\"
* ``uoPostIncrement``: Postincrement operator, eg \"x++\"
* ``uoPostDecrement``: Postdecrement operator, eg \"x--\"

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
