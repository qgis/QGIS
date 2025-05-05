# The following has been generated automatically from src/core/vector/qgsvectorlayerundocommand.h
try:
    QgsVectorLayerUndoCommand.__overridden_methods__ = ['id', 'mergeWith']
    import functools as _functools
    __wrapped_QgsVectorLayerUndoCommand_QgsVectorLayerUndoCommand = QgsVectorLayerUndoCommand.QgsVectorLayerUndoCommand
    def __QgsVectorLayerUndoCommand_QgsVectorLayerUndoCommand_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsVectorLayerUndoCommand_QgsVectorLayerUndoCommand(self, arg)
    QgsVectorLayerUndoCommand.QgsVectorLayerUndoCommand = _functools.update_wrapper(__QgsVectorLayerUndoCommand_QgsVectorLayerUndoCommand_wrapper, QgsVectorLayerUndoCommand.QgsVectorLayerUndoCommand)

    QgsVectorLayerUndoCommand.__group__ = ['vector']
except (NameError, AttributeError):
    pass
try:
    QgsVectorLayerUndoCommandAddFeature.__overridden_methods__ = ['undo', 'redo']
    QgsVectorLayerUndoCommandAddFeature.__group__ = ['vector']
except (NameError, AttributeError):
    pass
try:
    QgsVectorLayerUndoCommandDeleteFeature.__overridden_methods__ = ['undo', 'redo']
    QgsVectorLayerUndoCommandDeleteFeature.__group__ = ['vector']
except (NameError, AttributeError):
    pass
try:
    QgsVectorLayerUndoCommandChangeGeometry.__overridden_methods__ = ['undo', 'redo', 'id', 'mergeWith']
    QgsVectorLayerUndoCommandChangeGeometry.__group__ = ['vector']
except (NameError, AttributeError):
    pass
try:
    QgsVectorLayerUndoCommandChangeAttribute.__overridden_methods__ = ['undo', 'redo']
    QgsVectorLayerUndoCommandChangeAttribute.__group__ = ['vector']
except (NameError, AttributeError):
    pass
try:
    QgsVectorLayerUndoCommandAddAttribute.__overridden_methods__ = ['undo', 'redo']
    QgsVectorLayerUndoCommandAddAttribute.__group__ = ['vector']
except (NameError, AttributeError):
    pass
try:
    QgsVectorLayerUndoCommandDeleteAttribute.__overridden_methods__ = ['undo', 'redo']
    QgsVectorLayerUndoCommandDeleteAttribute.__group__ = ['vector']
except (NameError, AttributeError):
    pass
try:
    QgsVectorLayerUndoCommandRenameAttribute.__overridden_methods__ = ['undo', 'redo']
    QgsVectorLayerUndoCommandRenameAttribute.__group__ = ['vector']
except (NameError, AttributeError):
    pass
