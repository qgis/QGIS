# The following has been generated automatically from src/core/annotations/qgsannotationitemeditoperation.h
# monkey patching scoped based enum
QgsAbstractAnnotationItemEditOperation.Type.MoveNode.__doc__ = "Move a node"
QgsAbstractAnnotationItemEditOperation.Type.DeleteNode.__doc__ = "Delete a node"
QgsAbstractAnnotationItemEditOperation.Type.AddNode.__doc__ = "Add a node"
QgsAbstractAnnotationItemEditOperation.Type.TranslateItem.__doc__ = "Translate (move) an item"
QgsAbstractAnnotationItemEditOperation.Type.__doc__ = """Operation type

* ``MoveNode``: Move a node
* ``DeleteNode``: Delete a node
* ``AddNode``: Add a node
* ``TranslateItem``: Translate (move) an item

"""
# --
try:
    QgsAnnotationItemEditContext.__group__ = ['annotations']
except (NameError, AttributeError):
    pass
try:
    QgsAbstractAnnotationItemEditOperation.__group__ = ['annotations']
except (NameError, AttributeError):
    pass
try:
    QgsAnnotationItemEditOperationMoveNode.__group__ = ['annotations']
except (NameError, AttributeError):
    pass
try:
    QgsAnnotationItemEditOperationDeleteNode.__group__ = ['annotations']
except (NameError, AttributeError):
    pass
try:
    QgsAnnotationItemEditOperationAddNode.__group__ = ['annotations']
except (NameError, AttributeError):
    pass
try:
    QgsAnnotationItemEditOperationTranslateItem.__group__ = ['annotations']
except (NameError, AttributeError):
    pass
try:
    QgsAnnotationItemEditOperationTransientResults.__group__ = ['annotations']
except (NameError, AttributeError):
    pass
