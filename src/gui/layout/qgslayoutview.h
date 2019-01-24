/***************************************************************************
                             qgslayoutview.h
                             ---------------
    Date                 : July 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLAYOUTVIEW_H
#define QGSLAYOUTVIEW_H

#include "qgis_sip.h"
#include "qgsprevieweffect.h" // for QgsPreviewEffect::PreviewMode
#include "qgis_gui.h"
#include "qgslayoutitempage.h"
#include "qgslayoutaligner.h"
#include <QPointer>
#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <memory>

class QMenu;
class QgsLayout;
class QgsLayoutViewTool;
class QgsLayoutViewToolTemporaryKeyPan;
class QgsLayoutViewToolTemporaryKeyZoom;
class QgsLayoutViewToolTemporaryMousePan;
class QgsLayoutRuler;
class QgsLayoutViewMenuProvider;
class QgsLayoutViewSnapMarker;
class QgsLayoutReportSectionLabel;

/**
 * \ingroup gui
 * A graphical widget to display and interact with QgsLayouts.
 *
 * QgsLayoutView manages the layout interaction tools and mouse/key events.
 *
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsLayoutView: public QGraphicsView
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( qobject_cast<QgsLayoutView *>( sipCpp ) )
      sipType = sipType_QgsLayoutView;
    else
      sipType = NULL;
    SIP_END
#endif

    Q_OBJECT

    Q_PROPERTY( QgsLayout *currentLayout READ currentLayout WRITE setCurrentLayout NOTIFY layoutSet )
    Q_PROPERTY( QgsLayoutViewTool *tool READ tool WRITE setTool NOTIFY toolSet )

  public:

    //! Clipboard operations
    enum ClipboardOperation
    {
      ClipboardCut, //!< Cut items
      ClipboardCopy, //!< Copy items
    };

    //! Paste modes
    enum PasteMode
    {
      PasteModeCursor, //!< Paste items at cursor position
      PasteModeCenter, //!< Paste items in center of view
      PasteModeInPlace, //!< Paste items in place
    };

    /**
     * Constructor for QgsLayoutView.
     */
    QgsLayoutView( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    ~QgsLayoutView() override;

    /**
     * Returns the current layout associated with the view.
     * \see setCurrentLayout()
     * \see layoutSet()
     */
    QgsLayout *currentLayout();

    /**
     * Returns the current layout associated with the view.
     * \see setCurrentLayout()
     * \see layoutSet()
     */
    SIP_SKIP const QgsLayout *currentLayout() const;

    /**
     * Sets the current \a layout to edit in the view.
     * \see currentLayout()
     * \see layoutSet()
     */
    void setCurrentLayout( QgsLayout *layout SIP_KEEPREFERENCE );

    /**
     * Returns the currently active tool for the view.
     * \see setTool()
     */
    QgsLayoutViewTool *tool();

    /**
     * Sets the \a tool currently being used in the view.
     * \see unsetTool()
     * \see tool()
     */
    void setTool( QgsLayoutViewTool *tool );

    /**
     * Unsets the current view tool, if it matches the specified \a tool.
     *
     * This is called from destructor of view tools to make sure
     * that the tool won't be used any more.
     * You don't have to call it manually, QgsLayoutViewTool takes care of it.
     */
    void unsetTool( QgsLayoutViewTool *tool );

    /**
     * Sets whether a preview effect should be used to alter the view's appearance.
     * \param enabled Set to true to enable the preview effect on the view.
     * \see setPreviewMode()
     */
    void setPreviewModeEnabled( bool enabled );

    /**
     * Returns true if a preview effect is being used to alter the view's appearance.
     * \see setPreviewModeEnabled()
     */
    bool previewModeEnabled() const;

    /**
     * Sets the preview \a mode which should be used to modify the view's appearance. Preview modes are only used
     * if previewModeEnabled() is true.
     * \see setPreviewModeEnabled()
     * \see previewMode()
     */
    void setPreviewMode( QgsPreviewEffect::PreviewMode mode );

    /**
     * Returns the preview mode which may be used to modify the view's appearance. Preview modes are only used
     * if previewModeEnabled() is true.
     * \see setPreviewMode()
     * \see previewModeEnabled()
     */
    QgsPreviewEffect::PreviewMode previewMode() const;

    /**
     * Scales the view in a safe way, by limiting the acceptable range
     * of the scale applied. The \a scale parameter specifies the zoom factor to scale the view by.
     */
    void scaleSafe( double scale );

    /**
     * Sets the zoom \a level for the view, where a zoom level of 1.0 corresponds to 100%.
     */
    void setZoomLevel( double level );

    /**
     * Sets a horizontal \a ruler to synchronize with the view state.
     * \see setVerticalRuler()
     */
    void setHorizontalRuler( QgsLayoutRuler *ruler );

    /**
     * Sets a vertical \a ruler to synchronize with the view state.
     * \see setHorizontalRuler()
     */
    void setVerticalRuler( QgsLayoutRuler *ruler );

    /**
     * Sets a \a provider for context menus. Ownership of the provider is transferred to the view.
     * \see menuProvider()
     */
    void setMenuProvider( QgsLayoutViewMenuProvider *provider SIP_TRANSFER );

    /**
    * Returns the provider for context menus. Returned value may be nullptr if no provider is set.
    * \see setMenuProvider()
    */
    QgsLayoutViewMenuProvider *menuProvider() const;

    /**
     * Returns the page visible in the view. This method
     * considers the page at the center of the view as the current visible
     * page.
     * \see pageChanged()
     */
    int currentPage() const { return mCurrentPage; }

    /**
     * Returns a list of page items which are currently visible in the view.
     * \see visiblePageNumbers()
     */
    QList< QgsLayoutItemPage * > visiblePages() const;

    /**
     * Returns a list of page numbers for pages which are currently visible in the view.
     * \see visiblePages()
     */
    QList< int > visiblePageNumbers() const;

    /**
     * Aligns all selected items using the specified \a alignment.
     * \see distributeSelectedItems()
     * \see resizeSelectedItems()
     */
    void alignSelectedItems( QgsLayoutAligner::Alignment alignment );

    /**
     * Distributes all selected items using the specified \a distribution.
     * \see alignSelectedItems()
     * \see resizeSelectedItems()
     */
    void distributeSelectedItems( QgsLayoutAligner::Distribution distribution );

    /**
     * Resizes all selected items using the specified \a resize mode.
     * \see alignSelectedItems()
     * \see distributeSelectedItems()
     */
    void resizeSelectedItems( QgsLayoutAligner::Resize resize );

    /**
     * Cuts or copies the selected items, respecting the specified \a operation.
     * \see copyItems()
     * \see pasteItems()
     */
    void copySelectedItems( ClipboardOperation operation );

    /**
     * Cuts or copies the a list of \a items, respecting the specified \a operation.
     * \see copySelectedItems()
     * \see pasteItems()
     */
    void copyItems( const QList< QgsLayoutItem * > &items, ClipboardOperation operation );

    /**
     * Pastes items from clipboard, using the specified \a mode.
     *
     * A list of pasted items is returned.
     *
     * \see copySelectedItems()
     * \see hasItemsInClipboard()
     */
    QList< QgsLayoutItem * > pasteItems( PasteMode mode );

    /**
     * Pastes items from clipboard, at the specified \a layoutPoint,
     * in layout units.
     *
     * A list of pasted items is returned.
     *
     * \see copySelectedItems()
     * \see hasItemsInClipboard()
     */
    QList< QgsLayoutItem * > pasteItems( QPointF layoutPoint );

    /**
     * Returns true if the current clipboard contains layout items.
     * \see pasteItems()
     */
    bool hasItemsInClipboard() const;

    /**
     * Returns the delta (in layout coordinates) by which to move items
     * for the given key \a event.
     */
    QPointF deltaForKeyEvent( QKeyEvent *event );

    /**
     * Sets whether widget repainting should be allowed for the view. This is
     * used to temporarily halt painting while exporting layouts.
     * \note Not available in Python bindings.
     */
    void setPaintingEnabled( bool enabled ); SIP_SKIP

    /**
     * Sets a section \a label, to display above the first page shown in the
     * view.
     */
    void setSectionLabel( const QString &label );

  public slots:

    /**
     * Zooms the view to the full extent of the layout.
     * \see zoomIn()
     * \see zoomOut()
     * \see zoomActual()
     */
    void zoomFull();

    /**
     * Zooms the view to the full width of the layout.
     * \see zoomIn()
     * \see zoomOut()
     * \see zoomActual()
     */
    void zoomWidth();

    /**
     * Zooms in to the view by a preset amount.
     * \see zoomFull()
     * \see zoomOut()
     * \see zoomActual()
     */
    void zoomIn();

    /**
     * Zooms out of the view by a preset amount.
     * \see zoomFull()
     * \see zoomIn()
     * \see zoomActual()
     */
    void zoomOut();

    /**
     * Zooms to the actual size of the layout.
     * \see zoomFull()
     * \see zoomIn()
     * \see zoomOut()
     */
    void zoomActual();

    /**
     * Emits the zoomLevelChanged() signal. This should be called after
     * calling any of the QGraphicsView base class methods which alter
     * the view's zoom level, i.e. QGraphicsView::fitInView().
     */
    // NOTE - I realize these emitXXX methods are gross, but there's no clean
    // alternative here. We can't override the non-virtual Qt QGraphicsView
    // methods, and adding our own renamed methods which call the base class
    // methods also adds noise to the API.
    void emitZoomLevelChanged();

    // Why are these select methods in the view and not in the scene (QgsLayout)?
    // Well, in my opinion selections are purely a GUI concept. Ideally
    // NONE of the selection handling would be done in core, but we're restrained
    // by the QGraphicsScene API here.

    /**
     * Selects all items in the view.
     * \see deselectAll()
     * \see invertSelection()
     * \see selectNextItemAbove()
     * \see selectNextItemBelow()
     */
    void selectAll();

    /**
     * Deselects all items in the view.
     * \see selectAll()
     * \see invertSelection()
     */
    void deselectAll();

    /**
     * Inverts the current selection, selecting deselected items
     * and deselecting and selected items.
     * \see selectAll()
     * \see deselectAll()
     */
    void invertSelection();

    /**
     * Selects the next item above the existing selection, by item z order.
     * \see selectNextItemBelow()
     * \see selectAll()
     * \see deselectAll()
     */
    void selectNextItemAbove();

    /**
     * Selects the next item below the existing selection, by item z order.
     * \see selectNextItemAbove()
     * \see selectAll()
     * \see deselectAll()
     */
    void selectNextItemBelow();

    /**
     * Raises the selected items up the z-order.
     * \see lowerSelectedItems()
     * \see moveSelectedItemsToTop()
     * \see moveSelectedItemsToBottom()
     */
    void raiseSelectedItems();

    /**
     * Lowers the selected items down the z-order.
     * \see raiseSelectedItems()
     * \see moveSelectedItemsToTop()
     * \see moveSelectedItemsToBottom()
     */
    void lowerSelectedItems();

    /**
     * Raises the selected items to the top of the z-order.
     * \see raiseSelectedItems()
     * \see lowerSelectedItems()
     * \see moveSelectedItemsToBottom()
     */
    void moveSelectedItemsToTop();

    /**
     * Lowers the selected items to the bottom of the z-order.
     * \see raiseSelectedItems()
     * \see lowerSelectedItems()
     * \see moveSelectedItemsToTop()
     */
    void moveSelectedItemsToBottom();

    /**
     * Locks any selected items, preventing them from being interacted with
     * by mouse interactions.
     * \see unlockAllItems()
     */
    void lockSelectedItems();

    /**
     * Unlocks all locked items in the layout.
     * \see lockSelectedItems()
     */
    void unlockAllItems();

    /**
     * Deletes all selected items.
     * \see deleteItems()
     */
    void deleteSelectedItems();

    /**
     * Delete the specified \a items.
     * \see deleteSelectedItems()
     */
    void deleteItems( const QList< QgsLayoutItem * > &items );

    /**
     * Groups all selected items.
     * \see ungroupSelectedItems()
     */
    void groupSelectedItems();

    /**
     * Ungroups all selected items.
     * \see groupSelectedItems()
     */
    void ungroupSelectedItems();

    /**
     * Updates associated rulers and other widgets after view extent or zoom has changed.
     * This should be called after calling any of the QGraphicsView
     * base class methods which alter the view's zoom level or extent,
     * i.e. QGraphicsView::fitInView().
     */
    void viewChanged();

    /**
     * Pushes a new status bar \a message to the view. This causes statusMessage()
     * to be emitted, which should cause the message to appear in the status bar
     * for the parent window.
     * \see statusMessage()
     */
    void pushStatusMessage( const QString &message );

  signals:

    /**
     * Emitted when a \a layout is set for the view.
     * \see currentLayout()
     * \see setCurrentLayout()
     */
    void layoutSet( QgsLayout *layout );

    /**
     * Emitted when the current \a tool is changed.
     * \see setTool()
     */
    void toolSet( QgsLayoutViewTool *tool );

    /**
     * Is emitted whenever the zoom level of the view is changed.
     */
    void zoomLevelChanged();

    /**
     * Is emitted when the mouse cursor coordinates change within the view.
     * The \a layoutPoint argument indicates the cursor position within
     * the layout coordinate system.
     */
    void cursorPosChanged( QPointF layoutPoint );

    /**
     * Emitted when the page visible in the view is changed. This signal
     * considers the page at the center of the view as the current visible
     * page.
     * \see currentPage()
     */
    void pageChanged( int page );

    /**
     * Emitted when the view has a \a message for display in a parent window's
     * status bar.
     * \see pushStatusMessage()
     */
    void statusMessage( const QString &message );

    /**
     * Emitted when an \a item is "focused" in the view, i.e. it becomes the active
     * item and should have its properties displayed in any designer windows.
     */
    void itemFocused( QgsLayoutItem *item );

    /**
     * Emitted in the destructor when the view is about to be deleted,
     * but is still in a perfectly valid state.
     */
    void willBeDeleted();

  protected:
    void mousePressEvent( QMouseEvent *event ) override;
    void mouseReleaseEvent( QMouseEvent *event ) override;
    void mouseMoveEvent( QMouseEvent *event ) override;
    void mouseDoubleClickEvent( QMouseEvent *event ) override;
    void wheelEvent( QWheelEvent *event ) override;
    void keyPressEvent( QKeyEvent *event ) override;
    void keyReleaseEvent( QKeyEvent *event ) override;
    void resizeEvent( QResizeEvent *event ) override;
    void scrollContentsBy( int dx, int dy ) override;
    void dragEnterEvent( QDragEnterEvent *e ) override;
    void paintEvent( QPaintEvent *event ) override;

  private slots:

    void invalidateCachedRenders();

  private:

    //! Zoom layout from a mouse wheel event
    void wheelZoom( QWheelEvent *event );

    QPointer< QgsLayoutViewTool > mTool;

    QgsLayoutViewToolTemporaryKeyPan *mSpacePanTool = nullptr;
    QgsLayoutViewToolTemporaryMousePan *mMidMouseButtonPanTool = nullptr;
    QgsLayoutViewToolTemporaryKeyZoom *mSpaceZoomTool = nullptr;

    QPoint mMouseCurrentXY;

    QgsLayoutRuler *mHorizontalRuler = nullptr;
    QgsLayoutRuler *mVerticalRuler = nullptr;
    std::unique_ptr< QgsLayoutViewMenuProvider > mMenuProvider;

    QgsLayoutViewSnapMarker *mSnapMarker = nullptr;
    QgsLayoutReportSectionLabel *mSectionLabel = nullptr;

    QGraphicsLineItem *mHorizontalSnapLine = nullptr;
    QGraphicsLineItem *mVerticalSnapLine = nullptr;

    int mCurrentPage = 0;

    QgsPreviewEffect *mPreviewEffect = nullptr;

    bool mPaintingEnabled = true;

    friend class TestQgsLayoutView;
    friend class QgsLayoutMouseHandles;

    QGraphicsLineItem *createSnapLine() const;
};


/**
 * \ingroup gui
 *
 * Interface for a QgsLayoutView context menu.
 *
 * Implementations of this interface can be made to allow QgsLayoutView
 * instances to provide custom context menus (opened upon right-click).
 *
 * \see QgsLayoutView
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsLayoutViewMenuProvider
{
  public:
    virtual ~QgsLayoutViewMenuProvider() = default;

    //! Returns a newly created menu instance (or null pointer on error)
    virtual QMenu *createContextMenu( QWidget *parent SIP_TRANSFER, QgsLayout *layout, QPointF layoutPoint ) const = 0 SIP_FACTORY;
};


#ifndef SIP_RUN
///@cond PRIVATE


/**
 * \ingroup gui
 * A simple graphics item rendered as an 'x'.
 */
class GUI_EXPORT QgsLayoutViewSnapMarker : public QGraphicsRectItem
{
  public:

    QgsLayoutViewSnapMarker();

    void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr ) override;

  private:

    int mSize = 0;

};

///@endcond
#endif

#endif // QGSLAYOUTVIEW_H
