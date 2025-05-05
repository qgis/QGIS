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
    import functools as _functools
    __wrapped_QgsSQLStatement_NodeList_append = QgsSQLStatement.NodeList.append
    def __QgsSQLStatement_NodeList_append_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsSQLStatement_NodeList_append(self, arg)
    QgsSQLStatement.NodeList.append = _functools.update_wrapper(__QgsSQLStatement_NodeList_append_wrapper, QgsSQLStatement.NodeList.append)

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
    import functools as _functools
    __wrapped_QgsSQLStatement_NodeSelectedColumn_NodeSelectedColumn = QgsSQLStatement.NodeSelectedColumn.NodeSelectedColumn
    def __QgsSQLStatement_NodeSelectedColumn_NodeSelectedColumn_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsSQLStatement_NodeSelectedColumn_NodeSelectedColumn(self, arg)
    QgsSQLStatement.NodeSelectedColumn.NodeSelectedColumn = _functools.update_wrapper(__QgsSQLStatement_NodeSelectedColumn_NodeSelectedColumn_wrapper, QgsSQLStatement.NodeSelectedColumn.NodeSelectedColumn)

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
    import functools as _functools
    __wrapped_QgsSQLStatement_NodeSelect_setJoins = QgsSQLStatement.NodeSelect.setJoins
    def __QgsSQLStatement_NodeSelect_setJoins_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsSQLStatement_NodeSelect_setJoins(self, arg)
    QgsSQLStatement.NodeSelect.setJoins = _functools.update_wrapper(__QgsSQLStatement_NodeSelect_setJoins_wrapper, QgsSQLStatement.NodeSelect.setJoins)

    import functools as _functools
    __wrapped_QgsSQLStatement_NodeSelect_appendJoin = QgsSQLStatement.NodeSelect.appendJoin
    def __QgsSQLStatement_NodeSelect_appendJoin_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsSQLStatement_NodeSelect_appendJoin(self, arg)
    QgsSQLStatement.NodeSelect.appendJoin = _functools.update_wrapper(__QgsSQLStatement_NodeSelect_appendJoin_wrapper, QgsSQLStatement.NodeSelect.appendJoin)

    import functools as _functools
    __wrapped_QgsSQLStatement_NodeSelect_setWhere = QgsSQLStatement.NodeSelect.setWhere
    def __QgsSQLStatement_NodeSelect_setWhere_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsSQLStatement_NodeSelect_setWhere(self, arg)
    QgsSQLStatement.NodeSelect.setWhere = _functools.update_wrapper(__QgsSQLStatement_NodeSelect_setWhere_wrapper, QgsSQLStatement.NodeSelect.setWhere)

    import functools as _functools
    __wrapped_QgsSQLStatement_NodeSelect_setOrderBy = QgsSQLStatement.NodeSelect.setOrderBy
    def __QgsSQLStatement_NodeSelect_setOrderBy_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsSQLStatement_NodeSelect_setOrderBy(self, arg)
    QgsSQLStatement.NodeSelect.setOrderBy = _functools.update_wrapper(__QgsSQLStatement_NodeSelect_setOrderBy_wrapper, QgsSQLStatement.NodeSelect.setOrderBy)

except (NameError, AttributeError):
    pass
try:
    QgsSQLStatement.RecursiveVisitor.__overridden_methods__ = ['visit']
except (NameError, AttributeError):
    pass
