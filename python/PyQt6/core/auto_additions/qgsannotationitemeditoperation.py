# The following has been generated automatically from src/core/annotations/qgsannotationitemeditoperation.h
# monkey patching scoped based enum
QgsAbstractAnnotationItemEditOperation.Type.MoveNode.__doc__ = "Move a node"
QgsAbstractAnnotationItemEditOperation.Type.DeleteNode.__doc__ = "Delete a node"
QgsAbstractAnnotationItemEditOperation.Type.AddNode.__doc__ = "Add a node"
QgsAbstractAnnotationItemEditOperation.Type.TranslateItem.__doc__ = "Translate (move) an item"
QgsAbstractAnnotationItemEditOperation.Type.RotateItem.__doc__ = "Rotate an item \n.. versionadded:: 4.0"
QgsAbstractAnnotationItemEditOperation.Type.__doc__ = """Operation type

* ``MoveNode``: Move a node
* ``DeleteNode``: Delete a node
* ``AddNode``: Add a node
* ``TranslateItem``: Translate (move) an item
* ``RotateItem``: Rotate an item

  .. versionadded:: 4.0


"""
# --
try:
    QgsAbstractAnnotationItemEditOperation.__abstract_methods__ = ['type']
    QgsAbstractAnnotationItemEditOperation.__group__ = ['annotations']
except (NameError, AttributeError):
    pass
try:
    QgsAnnotationItemEditOperationMoveNode.__overridden_methods__ = ['type']
    QgsAnnotationItemEditOperationMoveNode.__group__ = ['annotations']
except (NameError, AttributeError):
    pass
try:
    QgsAnnotationItemEditOperationDeleteNode.__overridden_methods__ = ['type']
    QgsAnnotationItemEditOperationDeleteNode.__group__ = ['annotations']
except (NameError, AttributeError):
    pass
try:
    QgsAnnotationItemEditOperationAddNode.__overridden_methods__ = ['type']
    QgsAnnotationItemEditOperationAddNode.__group__ = ['annotations']
except (NameError, AttributeError):
    pass
try:
    QgsAnnotationItemEditOperationTranslateItem.__overridden_methods__ = ['type']
    QgsAnnotationItemEditOperationTranslateItem.__group__ = ['annotations']
except (NameError, AttributeError):
    pass
try:
    QgsAnnotationItemEditOperationRotateItem.__overridden_methods__ = ['type']
    QgsAnnotationItemEditOperationRotateItem.__group__ = ['annotations']
except (NameError, AttributeError):
    pass
try:
    QgsAnnotationItemEditContext.__group__ = ['annotations']
except (NameError, AttributeError):
    pass
try:
    QgsAnnotationItemEditOperationTransientResults.__group__ = ['annotations']
except (NameError, AttributeError):
    pass
