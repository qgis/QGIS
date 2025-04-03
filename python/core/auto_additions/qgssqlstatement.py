# The following has been generated automatically from src/core/qgssqlstatement.h
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
