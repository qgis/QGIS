# The following has been generated automatically from src/core/annotations/qgsannotationitemeditoperation.h
# monkey patching scoped based enum
QgsAbstractAnnotationItemEditOperation.Type.MoveNode.__doc__ = "Move a node"
QgsAbstractAnnotationItemEditOperation.Type.DeleteNode.__doc__ = "Delete a node"
QgsAbstractAnnotationItemEditOperation.Type.AddNode.__doc__ = "Add a node"
QgsAbstractAnnotationItemEditOperation.Type.TranslateItem.__doc__ = "Translate (move) an item"
QgsAbstractAnnotationItemEditOperation.Type.__doc__ = "Operation type\n\n" + '* ``MoveNode``: ' + QgsAbstractAnnotationItemEditOperation.Type.MoveNode.__doc__ + '\n' + '* ``DeleteNode``: ' + QgsAbstractAnnotationItemEditOperation.Type.DeleteNode.__doc__ + '\n' + '* ``AddNode``: ' + QgsAbstractAnnotationItemEditOperation.Type.AddNode.__doc__ + '\n' + '* ``TranslateItem``: ' + QgsAbstractAnnotationItemEditOperation.Type.TranslateItem.__doc__
# --
try:
    QgsAnnotationItemEditContext.__group__ = ['annotations']
except NameError:
    pass
try:
    QgsAbstractAnnotationItemEditOperation.__group__ = ['annotations']
except NameError:
    pass
try:
    QgsAnnotationItemEditOperationMoveNode.__group__ = ['annotations']
except NameError:
    pass
try:
    QgsAnnotationItemEditOperationDeleteNode.__group__ = ['annotations']
except NameError:
    pass
try:
    QgsAnnotationItemEditOperationAddNode.__group__ = ['annotations']
except NameError:
    pass
try:
    QgsAnnotationItemEditOperationTranslateItem.__group__ = ['annotations']
except NameError:
    pass
try:
    QgsAnnotationItemEditOperationTransientResults.__group__ = ['annotations']
except NameError:
    pass
