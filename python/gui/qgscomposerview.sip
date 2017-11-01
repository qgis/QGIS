/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgscomposerview.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsComposerView: QGraphicsView
{
%Docstring
 Widget to display the composer items. Manages the composer tools and the
 mouse/key events.
 Creates the composer items according to the current map tools and keeps track
 of the rubber band item.
%End

%TypeHeaderCode
#include "qgscomposerview.h"
%End
%ConvertToSubClassCode
    if ( sipCpp->inherits( "QgsComposerView" ) )
      sipType = sipType_QgsComposerView;
    else
      sipType = NULL;
%End
  public:

    enum Tool
    {
      Select,
      AddArrow,
      AddHtml,
      AddMap,
      AddLegend,
      AddLabel,
      AddScalebar,
      AddPicture,
      AddRectangle,
      AddEllipse,
      AddPolygon,
      AddPolyline,
      AddTriangle,
      AddTable,
      AddAttributeTable,
      MoveItemContent,
      EditNodesItem,
      Pan,
      Zoom
    };

    enum ClipboardMode
    {
      ClipboardModeCut,
      ClipboardModeCopy
    };

    enum PasteMode
    {
      PasteModeCursor,
      PasteModeCenter,
      PasteModeInPlace
    };

    enum ToolStatus
    {
      Inactive,
      Active,
      ActiveUntilMouseRelease
    };

    QgsComposerView( QWidget *parent /TransferThis/ = 0, const char *name = 0, Qt::WindowFlags f = 0 );

    void groupItems();
%Docstring
Add an item group containing the selected items
%End

    void ungroupItems();
%Docstring
Ungroups the selected items
%End

    void copyItems( ClipboardMode mode );
%Docstring
Cuts or copies the selected items
%End

    void pasteItems( PasteMode mode );
%Docstring
Pastes items from clipboard
%End

    void deleteSelectedItems();
%Docstring
Deletes selected items
%End

    void selectAll();
%Docstring
Selects all items
%End

    void selectNone();
%Docstring
Deselects all items
%End

    void selectInvert();
%Docstring
Inverts current selection
%End

    QgsComposerView::Tool currentTool() const;
%Docstring
 :rtype: QgsComposerView.Tool
%End
    void setCurrentTool( QgsComposerView::Tool t );

    void setComposition( QgsComposition *c /KeepReference/ );
%Docstring
 Sets the composition for the view. If the composition is being set manually and not by a QgsComposer, then this must
 be set BEFORE adding any items to the composition.
%End

    QgsComposition *composition();
%Docstring
Returns the composition or 0 in case of error
 :rtype: QgsComposition
%End

    QMainWindow *composerWindow();
%Docstring
Returns the composer main window
 :rtype: QMainWindow
%End

    void setPaintingEnabled( bool enabled );
    bool paintingEnabled() const;
%Docstring
 :rtype: bool
%End

    void updateRulers();
%Docstring
Update rulers with current scene rect
%End

    void setHorizontalRuler( QgsComposerRuler *r );
    void setVerticalRuler( QgsComposerRuler *r );

    void setZoomLevel( double zoomLevel );
%Docstring
Set zoom level, where a zoom level of 1.0 corresponds to 100%
%End

    void scaleSafe( double scale );
%Docstring
 Scales the view in a safe way, by limiting the acceptable range
 of the scale applied.
 \param scale factor to scale view by
.. versionadded:: 2.16
%End

    void setPreviewModeEnabled( bool enabled );
%Docstring
 Sets whether a preview effect should be used to alter the view's appearance
 \param enabled Set to true to enable the preview effect on the view
.. versionadded:: 2.3
.. seealso:: setPreviewMode
%End

    void setPreviewMode( QgsPreviewEffect::PreviewMode mode );
%Docstring
 Sets the preview mode which should be used to modify the view's appearance. Preview modes are only used
 if setPreviewMode is set to true.
 \param mode PreviewMode to be used to draw the view
.. versionadded:: 2.3
.. seealso:: setPreviewModeEnabled
%End

    void setMapCanvas( QgsMapCanvas *canvas );
%Docstring
 Sets the map canvas associated with the view. This allows the
 view to retrieve map settings from the canvas.
.. versionadded:: 3.0
.. seealso:: mapCanvas()
%End

    QgsMapCanvas *mapCanvas() const;
%Docstring
 Returns the map canvas associated with the view.
.. seealso:: setMapCanvas()
.. versionadded:: 3.0
 :rtype: QgsMapCanvas
%End

  protected:
    virtual void mousePressEvent( QMouseEvent * );

    virtual void mouseReleaseEvent( QMouseEvent * );

    virtual void mouseMoveEvent( QMouseEvent * );

    virtual void mouseDoubleClickEvent( QMouseEvent *e );


    virtual void keyPressEvent( QKeyEvent *e );

    virtual void keyReleaseEvent( QKeyEvent *e );


    virtual void wheelEvent( QWheelEvent *event );


    virtual void paintEvent( QPaintEvent *event );


    virtual void hideEvent( QHideEvent *e );

    virtual void showEvent( QShowEvent *e );


    virtual void resizeEvent( QResizeEvent *event );

    virtual void scrollContentsBy( int dx, int dy );


  signals:
    void selectedItemChanged( QgsComposerItem *selected );
%Docstring
Is emitted when selected item changed. If 0, no item is selected
%End
    void itemRemoved( QgsComposerItem * );
%Docstring
Is emitted when a composer item has been removed from the scene
%End

    void actionFinished();
%Docstring
 Current action (e.g. adding composer map) has been finished. The purpose of this signal is that
QgsComposer may set the selection tool again*
%End
    void cursorPosChanged( QPointF );
%Docstring
Is emitted when mouse cursor coordinates change
%End
    void zoomLevelChanged();
%Docstring
Is emitted when the view zoom changes
%End

    void composerViewShow( QgsComposerView * );
%Docstring
Emitted before composerview is shown
%End
    void composerViewHide( QgsComposerView * );
%Docstring
Emitted before composerview is hidden
%End

    void compositionSet( QgsComposition * );
%Docstring
Emitted when the composition is set for the view
%End
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgscomposerview.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
