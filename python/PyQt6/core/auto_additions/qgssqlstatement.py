# The following has been generated automatically from src/core/qgssqlstatement.h
QgsSQLStatement.uoNot = QgsSQLStatement.UnaryOperator.uoNot
QgsSQLStatement.uoMinus = QgsSQLStatement.UnaryOperator.uoMinus
QgsSQLStatement.boOr = QgsSQLStatement.BinaryOperator.boOr
QgsSQLStatement.boAnd = QgsSQLStatement.BinaryOperator.boAnd
QgsSQLStatement.boEQ = QgsSQLStatement.BinaryOperator.boEQ
QgsSQLStatement.boNE = QgsSQLStatement.BinaryOperator.boNE
QgsSQLStatement.boLE = QgsSQLStatement.BinaryOperator.boLE
QgsSQLStatement.boGE = QgsSQLStatement.BinaryOperator.boGE
QgsSQLStatement.boLT = QgsSQLStatement.BinaryOperator.boLT
QgsSQLStatement.boGT = QgsSQLStatement.BinaryOperator.boGT
QgsSQLStatement.boLike = QgsSQLStatement.BinaryOperator.boLike
QgsSQLStatement.boNotLike = QgsSQLStatement.BinaryOperator.boNotLike
QgsSQLStatement.boILike = QgsSQLStatement.BinaryOperator.boILike
QgsSQLStatement.boNotILike = QgsSQLStatement.BinaryOperator.boNotILike
QgsSQLStatement.boIs = QgsSQLStatement.BinaryOperator.boIs
QgsSQLStatement.boIsNot = QgsSQLStatement.BinaryOperator.boIsNot
QgsSQLStatement.boPlus = QgsSQLStatement.BinaryOperator.boPlus
QgsSQLStatement.boMinus = QgsSQLStatement.BinaryOperator.boMinus
QgsSQLStatement.boMul = QgsSQLStatement.BinaryOperator.boMul
QgsSQLStatement.boDiv = QgsSQLStatement.BinaryOperator.boDiv
QgsSQLStatement.boIntDiv = QgsSQLStatement.BinaryOperator.boIntDiv
QgsSQLStatement.boMod = QgsSQLStatement.BinaryOperator.boMod
QgsSQLStatement.boPow = QgsSQLStatement.BinaryOperator.boPow
QgsSQLStatement.boConcat = QgsSQLStatement.BinaryOperator.boConcat
QgsSQLStatement.jtDefault = QgsSQLStatement.JoinType.jtDefault
QgsSQLStatement.jtLeft = QgsSQLStatement.JoinType.jtLeft
QgsSQLStatement.jtLeftOuter = QgsSQLStatement.JoinType.jtLeftOuter
QgsSQLStatement.jtRight = QgsSQLStatement.JoinType.jtRight
QgsSQLStatement.jtRightOuter = QgsSQLStatement.JoinType.jtRightOuter
QgsSQLStatement.jtCross = QgsSQLStatement.JoinType.jtCross
QgsSQLStatement.jtInner = QgsSQLStatement.JoinType.jtInner
QgsSQLStatement.jtFull = QgsSQLStatement.JoinType.jtFull
QgsSQLStatement.ntUnaryOperator = QgsSQLStatement.NodeType.ntUnaryOperator
QgsSQLStatement.ntBinaryOperator = QgsSQLStatement.NodeType.ntBinaryOperator
QgsSQLStatement.ntInOperator = QgsSQLStatement.NodeType.ntInOperator
QgsSQLStatement.ntBetweenOperator = QgsSQLStatement.NodeType.ntBetweenOperator
QgsSQLStatement.ntFunction = QgsSQLStatement.NodeType.ntFunction
QgsSQLStatement.ntLiteral = QgsSQLStatement.NodeType.ntLiteral
QgsSQLStatement.ntColumnRef = QgsSQLStatement.NodeType.ntColumnRef
QgsSQLStatement.ntSelectedColumn = QgsSQLStatement.NodeType.ntSelectedColumn
QgsSQLStatement.ntSelect = QgsSQLStatement.NodeType.ntSelect
QgsSQLStatement.ntTableDef = QgsSQLStatement.NodeType.ntTableDef
QgsSQLStatement.ntJoin = QgsSQLStatement.NodeType.ntJoin
QgsSQLStatement.ntColumnSorted = QgsSQLStatement.NodeType.ntColumnSorted
QgsSQLStatement.ntCast = QgsSQLStatement.NodeType.ntCast
try:
    QgsSQLStatement.quotedIdentifier = staticmethod(QgsSQLStatement.quotedIdentifier)
    QgsSQLStatement.quotedIdentifierIfNeeded = staticmethod(QgsSQLStatement.quotedIdentifierIfNeeded)
    QgsSQLStatement.stripQuotedIdentifier = staticmethod(QgsSQLStatement.stripQuotedIdentifier)
    QgsSQLStatement.stripMsQuotedIdentifier = staticmethod(QgsSQLStatement.stripMsQuotedIdentifier)
    QgsSQLStatement.quotedString = staticmethod(QgsSQLStatement.quotedString)
except (NameError, AttributeError):
    pass
try:
    QgsSQLStatement.NodeList.__virtual_methods__ = ['dump']
except (NameError, AttributeError):
    pass
try:
    QgsSQLStatement.Node.__abstract_methods__ = ['nodeType', 'dump', 'clone', 'accept']
except (NameError, AttributeError):
    pass
try:
    QgsSQLStatement.Visitor.__abstract_methods__ = ['visit']
except (NameError, AttributeError):
    pass
try:
    QgsSQLStatement.NodeUnaryOperator.__overridden_methods__ = ['nodeType', 'dump', 'accept', 'clone']
except (NameError, AttributeError):
    pass
try:
    QgsSQLStatement.NodeBinaryOperator.__overridden_methods__ = ['nodeType', 'dump', 'accept', 'clone']
except (NameError, AttributeError):
    pass
try:
    QgsSQLStatement.NodeInOperator.__overridden_methods__ = ['nodeType', 'dump', 'accept', 'clone']
except (NameError, AttributeError):
    pass
try:
    QgsSQLStatement.NodeBetweenOperator.__overridden_methods__ = ['nodeType', 'dump', 'accept', 'clone']
except (NameError, AttributeError):
    pass
try:
    QgsSQLStatement.NodeFunction.__overridden_methods__ = ['nodeType', 'dump', 'accept', 'clone']
except (NameError, AttributeError):
    pass
try:
    QgsSQLStatement.NodeLiteral.__overridden_methods__ = ['nodeType', 'dump', 'accept', 'clone']
except (NameError, AttributeError):
    pass
try:
    QgsSQLStatement.NodeColumnRef.__overridden_methods__ = ['nodeType', 'dump', 'accept', 'clone']
except (NameError, AttributeError):
    pass
try:
    QgsSQLStatement.NodeSelectedColumn.__overridden_methods__ = ['nodeType', 'dump', 'accept', 'clone']
except (NameError, AttributeError):
    pass
try:
    QgsSQLStatement.NodeCast.__overridden_methods__ = ['nodeType', 'dump', 'accept', 'clone']
except (NameError, AttributeError):
    pass
try:
    QgsSQLStatement.NodeTableDef.__overridden_methods__ = ['nodeType', 'dump', 'accept', 'clone']
except (NameError, AttributeError):
    pass
try:
    QgsSQLStatement.NodeJoin.__overridden_methods__ = ['nodeType', 'dump', 'accept', 'clone']
except (NameError, AttributeError):
    pass
try:
    QgsSQLStatement.NodeColumnSorted.__overridden_methods__ = ['nodeType', 'dump', 'accept', 'clone']
except (NameError, AttributeError):
    pass
try:
    QgsSQLStatement.NodeSelect.__overridden_methods__ = ['nodeType', 'dump', 'accept', 'clone']
except (NameError, AttributeError):
    pass
try:
    QgsSQLStatement.RecursiveVisitor.__overridden_methods__ = ['visit']
except (NameError, AttributeError):
    pass
