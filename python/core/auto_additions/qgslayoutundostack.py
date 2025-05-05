# The following has been generated automatically from src/core/layout/qgslayoutundostack.h
try:
    QgsLayoutUndoStack.__attribute_docs__ = {'undoRedoOccurredForItems': 'Emitted when an undo or redo action has occurred, which affected a set\nof layout ``itemUuids``.\n'}
    QgsLayoutUndoStack.__signal_arguments__ = {'undoRedoOccurredForItems': ['itemUuids: Set[str]']}
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsLayoutUndoStack_push = QgsLayoutUndoStack.push
    def __QgsLayoutUndoStack_push_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsLayoutUndoStack_push(self, arg)
    QgsLayoutUndoStack.push = _functools.update_wrapper(__QgsLayoutUndoStack_push_wrapper, QgsLayoutUndoStack.push)

    QgsLayoutUndoStack.__group__ = ['layout']
except (NameError, AttributeError):
    pass
