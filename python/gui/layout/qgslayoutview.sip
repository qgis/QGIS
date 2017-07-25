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

    void viewChanged();
%Docstring
 Updates associated rulers and other widgets after view extent or zoom has changed.
 This should be called after calling any of the QGraphicsView
 base class methods which alter the view's zoom level or extent,
 i.e. QGraphicsView.fitInView().
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
