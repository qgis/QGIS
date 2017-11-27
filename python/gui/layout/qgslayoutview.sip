/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layout/qgslayoutview.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsLayoutView: QGraphicsView
{
%Docstring
 A graphical widget to display and interact with QgsLayouts.

 QgsLayoutView manages the layout interaction tools and mouse/key events.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutview.h"
%End
  public:

    QgsLayoutView( QWidget *parent /TransferThis/ = 0 );
%Docstring
 Constructor for QgsLayoutView.
%End

    QgsLayout *currentLayout();
%Docstring
 Returns the current layout associated with the view.
.. seealso:: setCurrentLayout()
.. seealso:: layoutSet()
 :rtype: QgsLayout
%End


    void setCurrentLayout( QgsLayout *layout /KeepReference/ );
%Docstring
 Sets the current ``layout`` to edit in the view.
.. seealso:: currentLayout()
.. seealso:: layoutSet()
%End

    QgsLayoutViewTool *tool();
%Docstring
 Returns the currently active tool for the view.
.. seealso:: setTool()
 :rtype: QgsLayoutViewTool
%End

    void setTool( QgsLayoutViewTool *tool );
%Docstring
 Sets the ``tool`` currently being used in the view.
.. seealso:: unsetTool()
.. seealso:: tool()
%End

    void unsetTool( QgsLayoutViewTool *tool );
%Docstring
 Unsets the current view tool, if it matches the specified ``tool``.

 This is called from destructor of view tools to make sure
 that the tool won't be used any more.
 You don't have to call it manually, QgsLayoutViewTool takes care of it.
%End

    void setPreviewModeEnabled( bool enabled );
%Docstring
 Sets whether a preview effect should be used to alter the view's appearance.
 \param enabled Set to true to enable the preview effect on the view.
.. seealso:: setPreviewMode()
%End

    bool previewModeEnabled() const;
%Docstring
 Returns true if a preview effect is being used to alter the view's appearance.
.. seealso:: setPreviewModeEnabled()
 :rtype: bool
%End

    void setPreviewMode( QgsPreviewEffect::PreviewMode mode );
%Docstring
 Sets the preview ``mode`` which should be used to modify the view's appearance. Preview modes are only used
 if previewModeEnabled() is true.
.. seealso:: setPreviewModeEnabled()
.. seealso:: previewMode()
%End

    QgsPreviewEffect::PreviewMode previewMode() const;
%Docstring
 Returns the preview mode which may be used to modify the view's appearance. Preview modes are only used
 if previewModeEnabled() is true.
.. seealso:: setPreviewMode()
.. seealso:: previewModeEnabled()
 :rtype: QgsPreviewEffect.PreviewMode
%End

    void scaleSafe( double scale );
%Docstring
 Scales the view in a safe way, by limiting the acceptable range
 of the scale applied. The ``scale`` parameter specifies the zoom factor to scale the view by.
%End

    void setZoomLevel( double level );
%Docstring
 Sets the zoom ``level`` for the view, where a zoom level of 1.0 corresponds to 100%.
%End

    void setHorizontalRuler( QgsLayoutRuler *ruler );
%Docstring
 Sets a horizontal ``ruler`` to synchronize with the view state.
.. seealso:: setVerticalRuler()
%End

    void setVerticalRuler( QgsLayoutRuler *ruler );
%Docstring
 Sets a vertical ``ruler`` to synchronize with the view state.
.. seealso:: setHorizontalRuler()
%End

    void setMenuProvider( QgsLayoutViewMenuProvider *provider /Transfer/ );
%Docstring
 Sets a ``provider`` for context menus. Ownership of the provider is transferred to the view.
.. seealso:: menuProvider()
%End

    QgsLayoutViewMenuProvider *menuProvider() const;
%Docstring
 Returns the provider for context menus. Returned value may be None if no provider is set.
.. seealso:: setMenuProvider()
 :rtype: QgsLayoutViewMenuProvider
%End

    int currentPage() const;
%Docstring
 Returns the page visible in the view. This method
 considers the page at the center of the view as the current visible
 page.
.. seealso:: pageChanged()
 :rtype: int
%End

    QList< QgsLayoutItemPage * > visiblePages() const;
%Docstring
 Returns a list of page items which are currently visible in the view.
.. seealso:: visiblePageNumbers()
 :rtype: list of QgsLayoutItemPage
%End

    QList< int > visiblePageNumbers() const;
%Docstring
 Returns a list of page numbers for pages which are currently visible in the view.
.. seealso:: visiblePages()
 :rtype: list of int
%End

    void alignSelectedItems( QgsLayoutAligner::Alignment alignment );
%Docstring
 Aligns all selected items using the specified ``alignment``.
.. seealso:: distributeSelectedItems()
.. seealso:: resizeSelectedItems()
%End

    void distributeSelectedItems( QgsLayoutAligner::Distribution distribution );
%Docstring
 Distributes all selected items using the specified ``distribution``.
.. seealso:: alignSelectedItems()
.. seealso:: resizeSelectedItems()
%End

    void resizeSelectedItems( QgsLayoutAligner::Resize resize );
%Docstring
 Resizes all selected items using the specified ``resize`` mode.
.. seealso:: alignSelectedItems()
.. seealso:: distributeSelectedItems()
%End

    QPointF deltaForKeyEvent( QKeyEvent *event );
%Docstring
 Returns the delta (in layout coordinates) by which to move items
 for the given key ``event``.
 :rtype: QPointF
%End

  public slots:

    void zoomFull();
%Docstring
 Zooms the view to the full extent of the layout.
.. seealso:: zoomIn()
.. seealso:: zoomOut()
.. seealso:: zoomActual()
%End

    void zoomWidth();
%Docstring
 Zooms the view to the full width of the layout.
.. seealso:: zoomIn()
.. seealso:: zoomOut()
.. seealso:: zoomActual()
%End

    void zoomIn();
%Docstring
 Zooms in to the view by a preset amount.
.. seealso:: zoomFull()
.. seealso:: zoomOut()
.. seealso:: zoomActual()
%End

    void zoomOut();
%Docstring
 Zooms out of the view by a preset amount.
.. seealso:: zoomFull()
.. seealso:: zoomIn()
.. seealso:: zoomActual()
%End

    void zoomActual();
%Docstring
 Zooms to the actual size of the layout.
.. seealso:: zoomFull()
.. seealso:: zoomIn()
.. seealso:: zoomOut()
%End

    void emitZoomLevelChanged();


    void selectAll();
%Docstring
 Selects all items in the view.
.. seealso:: deselectAll()
.. seealso:: invertSelection()
.. seealso:: selectNextItemAbove()
.. seealso:: selectNextItemBelow()
%End

    void deselectAll();
%Docstring
 Deselects all items in the view.
.. seealso:: selectAll()
.. seealso:: invertSelection()
%End

    void invertSelection();
%Docstring
 Inverts the current selection, selecting deselected items
 and deselecting and selected items.
.. seealso:: selectAll()
.. seealso:: deselectAll()
%End

    void selectNextItemAbove();
%Docstring
 Selects the next item above the existing selection, by item z order.
.. seealso:: selectNextItemBelow()
.. seealso:: selectAll()
.. seealso:: deselectAll()
%End

    void selectNextItemBelow();
%Docstring
 Selects the next item below the existing selection, by item z order.
.. seealso:: selectNextItemAbove()
.. seealso:: selectAll()
.. seealso:: deselectAll()
%End

    void raiseSelectedItems();
%Docstring
 Raises the selected items up the z-order.
.. seealso:: lowerSelectedItems()
.. seealso:: moveSelectedItemsToTop()
.. seealso:: moveSelectedItemsToBottom()
%End

    void lowerSelectedItems();
%Docstring
 Lowers the selected items down the z-order.
.. seealso:: raiseSelectedItems()
.. seealso:: moveSelectedItemsToTop()
.. seealso:: moveSelectedItemsToBottom()
%End

    void moveSelectedItemsToTop();
%Docstring
 Raises the selected items to the top of the z-order.
.. seealso:: raiseSelectedItems()
.. seealso:: lowerSelectedItems()
.. seealso:: moveSelectedItemsToBottom()
%End

    void moveSelectedItemsToBottom();
%Docstring
 Lowers the selected items to the bottom of the z-order.
.. seealso:: raiseSelectedItems()
.. seealso:: lowerSelectedItems()
.. seealso:: moveSelectedItemsToTop()
%End

    void lockSelectedItems();
%Docstring
 Locks any selected items, preventing them from being interacted with
 by mouse interactions.
.. seealso:: unlockAllItems()
%End

    void unlockAllItems();
%Docstring
 Unlocks all locked items in the layout.
.. seealso:: lockSelectedItems()
%End

    void deleteSelectedItems();
%Docstring
 Deletes all selected items.
%End

    void groupSelectedItems();
%Docstring
 Groups all selected items.
.. seealso:: ungroupSelectedItems()
%End

    void ungroupSelectedItems();
%Docstring
 Ungroups all selected items.
.. seealso:: groupSelectedItems()
%End

    void viewChanged();
%Docstring
 Updates associated rulers and other widgets after view extent or zoom has changed.
 This should be called after calling any of the QGraphicsView
 base class methods which alter the view's zoom level or extent,
 i.e. QGraphicsView.fitInView().
%End

    void pushStatusMessage( const QString &message );
%Docstring
 Pushes a new status bar ``message`` to the view. This causes statusMessage()
 to be emitted, which should cause the message to appear in the status bar
 for the parent window.
.. seealso:: statusMessage()
%End

  signals:

    void layoutSet( QgsLayout *layout );
%Docstring
 Emitted when a ``layout`` is set for the view.
.. seealso:: currentLayout()
.. seealso:: setCurrentLayout()
%End

    void toolSet( QgsLayoutViewTool *tool );
%Docstring
 Emitted when the current ``tool`` is changed.
.. seealso:: setTool()
%End

    void zoomLevelChanged();
%Docstring
 Is emitted whenever the zoom level of the view is changed.
%End

    void cursorPosChanged( QPointF layoutPoint );
%Docstring
 Is emitted when the mouse cursor coordinates change within the view.
 The ``layoutPoint`` argument indicates the cursor position within
 the layout coordinate system.
%End

    void pageChanged( int page );
%Docstring
 Emitted when the page visible in the view is changed. This signal
 considers the page at the center of the view as the current visible
 page.
.. seealso:: currentPage()
%End

    void statusMessage( const QString &message );
%Docstring
 Emitted when the view has a ``message`` for display in a parent window's
 status bar.
.. seealso:: pushStatusMessage()
%End

    void itemFocused( QgsLayoutItem *item );
%Docstring
 Emitted when an ``item`` is "focused" in the view, i.e. it becomes the active
 item and should have its properties displayed in any designer windows.
%End

  protected:
    virtual void mousePressEvent( QMouseEvent *event );

    virtual void mouseReleaseEvent( QMouseEvent *event );

    virtual void mouseMoveEvent( QMouseEvent *event );

    virtual void mouseDoubleClickEvent( QMouseEvent *event );

    virtual void wheelEvent( QWheelEvent *event );

    virtual void keyPressEvent( QKeyEvent *event );

    virtual void keyReleaseEvent( QKeyEvent *event );

    virtual void resizeEvent( QResizeEvent *event );

    virtual void scrollContentsBy( int dx, int dy );


};


class QgsLayoutViewMenuProvider
{
%Docstring

 Interface for a QgsLayoutView context menu.

 Implementations of this interface can be made to allow QgsLayoutView
 instances to provide custom context menus (opened upon right-click).

.. seealso:: QgsLayoutView
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutview.h"
%End
  public:
    virtual ~QgsLayoutViewMenuProvider();

    virtual QMenu *createContextMenu( QWidget *parent /Transfer/, QgsLayout *layout, QPointF layoutPoint ) const = 0 /Factory/;
%Docstring
Return a newly created menu instance (or null pointer on error)
 :rtype: QMenu
%End
};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layout/qgslayoutview.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
