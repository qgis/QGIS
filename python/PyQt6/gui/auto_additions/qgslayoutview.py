# The following has been generated automatically from src/gui/layout/qgslayoutview.h
QgsLayoutView.ClipboardCut = QgsLayoutView.ClipboardOperation.ClipboardCut
QgsLayoutView.ClipboardCopy = QgsLayoutView.ClipboardOperation.ClipboardCopy
QgsLayoutView.PasteModeCursor = QgsLayoutView.PasteMode.PasteModeCursor
QgsLayoutView.PasteModeCenter = QgsLayoutView.PasteMode.PasteModeCenter
QgsLayoutView.PasteModeInPlace = QgsLayoutView.PasteMode.PasteModeInPlace
try:
    QgsLayoutView.__attribute_docs__ = {'layoutSet': 'Emitted when a ``layout`` is set for the view.\n\n.. seealso:: :py:func:`currentLayout`\n\n.. seealso:: :py:func:`setCurrentLayout`\n', 'toolSet': 'Emitted when the current ``tool`` is changed.\n\n.. seealso:: :py:func:`setTool`\n', 'zoomLevelChanged': 'Emitted whenever the zoom level of the view is changed.\n', 'cursorPosChanged': 'Emitted when the mouse cursor coordinates change within the view.\nThe ``layoutPoint`` argument indicates the cursor position within\nthe layout coordinate system.\n', 'pageChanged': 'Emitted when the page visible in the view is changed. This signal\nconsiders the page at the center of the view as the current visible\npage.\n\n.. seealso:: :py:func:`currentPage`\n', 'statusMessage': "Emitted when the view has a ``message`` for display in a parent window's\nstatus bar.\n\n.. seealso:: :py:func:`pushStatusMessage`\n", 'itemFocused': 'Emitted when an ``item`` is "focused" in the view, i.e. it becomes the active\nitem and should have its properties displayed in any designer windows.\n', 'willBeDeleted': 'Emitted in the destructor when the view is about to be deleted,\nbut is still in a perfectly valid state.\n'}
    QgsLayoutView.__signal_arguments__ = {'layoutSet': ['layout: QgsLayout'], 'toolSet': ['tool: QgsLayoutViewTool'], 'cursorPosChanged': ['layoutPoint: QPointF'], 'pageChanged': ['page: int'], 'statusMessage': ['message: str'], 'itemFocused': ['item: QgsLayoutItem']}
    QgsLayoutView.__group__ = ['layout']
except (NameError, AttributeError):
    pass
try:
    QgsLayoutViewMenuProvider.__group__ = ['layout']
except (NameError, AttributeError):
    pass
