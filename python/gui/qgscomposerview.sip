/** \ingroup MapComposer
 * \ingroup gui
 * Widget to display the composer items. Manages the composer tools and the
 * mouse/key events.
 * Creates the composer items according to the current map tools and keeps track
 * of the rubber band item.
 */
class QgsComposerView : QGraphicsView
{
%TypeHeaderCode
#include "qgscomposerview.h"
%End

%ConvertToSubClassCode
  if (sipCpp->inherits("QgsComposerView"))
    sipClass = sipClass_QgsComposerView;
  else
    sipClass = NULL;
%End

  public:

    /**Current tool*/
    enum Tool
    {
      Select,          // Select/Move item
      AddArrow,        // add arrow
      AddHtml,
      AddMap,          // add new map
      AddLegend,       // add vector legend
      AddLabel,        // add label
      AddScalebar,     // add scalebar
      AddPicture,      // add raster/vector picture
      AddRectangle,
      AddEllipse,
      AddTriangle,
      AddTable,        // add attribute table
      AddAttributeTable,
      MoveItemContent, // move content of item (e.g. content of map)
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

    QgsComposerView( QWidget* parent /TransferThis/ = 0, const char* name = 0, Qt::WindowFlags f = 0 );

    /**Add an item group containing the selected items*/
    void groupItems();

    /**Ungroups the selected items*/
    void ungroupItems();

    /**Cuts or copies the selected items*/
    void copyItems( ClipboardMode mode );

    /**Pastes items from clipboard*/
    void pasteItems( PasteMode mode );

    /**Deletes selected items*/
    void deleteSelectedItems();

    /**Selects all items*/
    void selectAll();

    /**Deselects all items*/
    void selectNone();

    /**Inverts current selection*/
    void selectInvert();

    QgsComposerView::Tool currentTool() const;
    void setCurrentTool( QgsComposerView::Tool t );

    /**Sets the composition for the view. If the composition is being set manually and not by a QgsComposer, then this must
     * be set BEFORE adding any items to the composition.
    */
    void setComposition( QgsComposition* c /KeepReference/ );

    /**Returns the composition or 0 in case of error*/
    QgsComposition* composition();

    /**Returns the composer main window*/
    QMainWindow* composerWindow();

    void setPaintingEnabled( bool enabled );
    bool paintingEnabled() const;

    /**Update rulers with current scene rect*/
    void updateRulers();

    void setHorizontalRuler( QgsComposerRuler* r );
    void setVerticalRuler( QgsComposerRuler* r );

    /**Set zoom level, where a zoom level of 1.0 corresponds to 100%*/
    void setZoomLevel( double zoomLevel );

    /**Sets whether a preview effect should be used to alter the view's appearance
     * @param enabled Set to true to enable the preview effect on the view
     * @note added in 2.3
     * @see setPreviewMode
    */
    void setPreviewModeEnabled( bool enabled );
    /**Sets the preview mode which should be used to modify the view's appearance. Preview modes are only used
     * if setPreviewMode is set to true.
     * @param mode PreviewMode to be used to draw the view
     * @note added in 2.3
     * @see setPreviewModeEnabled
    */
    void setPreviewMode( QgsPreviewEffect::PreviewMode mode );

  protected:
    void mousePressEvent( QMouseEvent* );
    void mouseReleaseEvent( QMouseEvent* );
    void mouseMoveEvent( QMouseEvent* );
    void mouseDoubleClickEvent( QMouseEvent* e );

    void keyPressEvent( QKeyEvent * e );
    void keyReleaseEvent( QKeyEvent * e );

    void wheelEvent( QWheelEvent* event );

    void paintEvent( QPaintEvent* event );

    void hideEvent( QHideEvent* e );
    void showEvent( QShowEvent* e );

    void resizeEvent( QResizeEvent* event );
    void scrollContentsBy( int dx, int dy );
  signals:
    /**Is emitted when selected item changed. If 0, no item is selected*/
    void selectedItemChanged( QgsComposerItem* selected );
    /**Is emitted when a composer item has been removed from the scene*/
    void itemRemoved( QgsComposerItem* );
    /**Current action (e.g. adding composer map) has been finished. The purpose of this signal is that
     QgsComposer may set the selection tool again*/
    void actionFinished();
    /**Is emitted when mouse cursor coordinates change*/
    void cursorPosChanged( QPointF );
    /**Is emitted when the view zoom changes*/
    void zoomLevelChanged();

    /**Emitted before composerview is shown*/
    void composerViewShow( QgsComposerView* );
    /**Emitted before composerview is hidden*/
    void composerViewHide( QgsComposerView* );

    /**Emitted when the composition is set for the view*/
    void compositionSet( QgsComposition* );
};
