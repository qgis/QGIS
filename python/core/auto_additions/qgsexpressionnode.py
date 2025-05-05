# The following has been generated automatically from src/core/expression/qgsexpressionnode.h
try:
    QgsExpressionNode.NamedNode.__attribute_docs__ = {'name': 'Node name', 'node': 'Node'}
    QgsExpressionNode.NamedNode.__annotations__ = {'name': str, 'node': 'QgsExpressionNode'}
    QgsExpressionNode.NamedNode.__doc__ = """Named node"""
    QgsExpressionNode.NamedNode.__group__ = ['expression']
except (NameError, AttributeError):
    pass
try:
    QgsExpressionNode.__attribute_docs__ = {'parserFirstLine': 'First line in the parser this node was found.\n\n.. note::\n\n   This might not be complete for all nodes. Currently\n   only :py:class:`QgsExpressionNode` has this complete', 'parserFirstColumn': 'First column in the parser this node was found.\n\n.. note::\n\n   This might not be complete for all nodes. Currently\n   only :py:class:`QgsExpressionNode` has this complete', 'parserLastLine': 'Last line in the parser this node was found.\n\n.. note::\n\n   This might not be complete for all nodes. Currently\n   only :py:class:`QgsExpressionNode` has this complete', 'parserLastColumn': 'Last column in the parser this node was found.\n\n.. note::\n\n   This might not be complete for all nodes. Currently\n   only :py:class:`QgsExpressionNode` has this complete'}
    QgsExpressionNode.__annotations__ = {'parserFirstLine': int, 'parserFirstColumn': int, 'parserLastLine': int, 'parserLastColumn': int}
    QgsExpressionNode.__abstract_methods__ = ['nodeType', 'dump', 'clone', 'referencedColumns', 'referencedVariables', 'referencedFunctions', 'needsGeometry', 'isStatic']
    QgsExpressionNode.__group__ = ['expression']
except (NameError, AttributeError):
    pass
try:
    QgsExpressionNode.NodeList.__virtual_methods__ = ['dump']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsExpressionNode_NodeList_append = QgsExpressionNode.NodeList.append
    def __QgsExpressionNode_NodeList_append_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsExpressionNode_NodeList_append(self, arg)
    QgsExpressionNode.NodeList.append = _functools.update_wrapper(__QgsExpressionNode_NodeList_append_wrapper, QgsExpressionNode.NodeList.append)

    QgsExpressionNode.NodeList.__group__ = ['expression']
except (NameError, AttributeError):
    pass
