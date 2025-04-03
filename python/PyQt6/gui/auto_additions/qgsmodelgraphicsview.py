# The following has been generated automatically from src/gui/processing/models/qgsmodelgraphicsview.h
QgsModelGraphicsView.ClipboardCut = QgsModelGraphicsView.ClipboardOperation.ClipboardCut
QgsModelGraphicsView.ClipboardCopy = QgsModelGraphicsView.ClipboardOperation.ClipboardCopy
QgsModelGraphicsView.PasteModeCursor = QgsModelGraphicsView.PasteMode.PasteModeCursor
QgsModelGraphicsView.PasteModeCenter = QgsModelGraphicsView.PasteMode.PasteModeCenter
QgsModelGraphicsView.PasteModeInPlace = QgsModelGraphicsView.PasteMode.PasteModeInPlace
try:
    QgsModelGraphicsView.__attribute_docs__ = {'algorithmDropped': 'Emitted when an algorithm is dropped onto the view.\n', 'inputDropped': 'Emitted when an input parameter is dropped onto the view.\n', 'itemFocused': 'Emitted when an ``item`` is "focused" in the view, i.e. it becomes the\nactive item and should have its properties displayed in any designer\nwindows.\n', 'willBeDeleted': 'Emitted in the destructor when the view is about to be deleted, but is\nstill in a perfectly valid state.\n', 'macroCommandStarted': 'Emitted when a macro command containing a group of interactions is\nstarted in the view.\n', 'macroCommandEnded': 'Emitted when a macro command containing a group of interactions in the\nview has ended.\n', 'commandBegun': 'Emitted when an undo command is started in the view.\n', 'commandEnded': 'Emitted when an undo command in the view has ended.\n', 'deleteSelectedItems': 'Emitted when the selected items should be deleted;\n'}
    QgsModelGraphicsView.__overridden_methods__ = ['dragEnterEvent', 'dropEvent', 'dragMoveEvent', 'wheelEvent', 'mousePressEvent', 'mouseReleaseEvent', 'mouseMoveEvent', 'mouseDoubleClickEvent', 'keyPressEvent', 'keyReleaseEvent']
    QgsModelGraphicsView.__signal_arguments__ = {'algorithmDropped': ['algorithmId: str', 'pos: QPointF'], 'inputDropped': ['inputId: str', 'pos: QPointF'], 'itemFocused': ['item: QgsModelComponentGraphicItem'], 'macroCommandStarted': ['text: str'], 'commandBegun': ['text: str']}
    QgsModelGraphicsView.__group__ = ['processing', 'models']
except (NameError, AttributeError):
    pass
try:
    QgsModelViewSnapMarker.__overridden_methods__ = ['paint']
    QgsModelViewSnapMarker.__group__ = ['processing', 'models']
except (NameError, AttributeError):
    pass
