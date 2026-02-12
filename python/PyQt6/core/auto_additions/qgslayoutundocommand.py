# The following has been generated automatically from src/core/layout/qgslayoutundocommand.h
try:
    QgsAbstractLayoutUndoCommand.__virtual_methods__ = ['containsChange']
    QgsAbstractLayoutUndoCommand.__abstract_methods__ = ['saveState', 'restoreState']
    QgsAbstractLayoutUndoCommand.__overridden_methods__ = ['undo', 'redo', 'id']
    QgsAbstractLayoutUndoCommand.__group__ = ['layout']
except (NameError, AttributeError):
    pass
try:
    QgsLayoutUndoObjectInterface.__abstract_methods__ = ['createCommand']
    QgsLayoutUndoObjectInterface.__group__ = ['layout']
except (NameError, AttributeError):
    pass
