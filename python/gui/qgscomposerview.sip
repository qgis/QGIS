/** \ingroup MapComposer
 * \ingroup gui
 * Widget to display the composer items. Manages the composer tools and the
 * mouse/key events.
 * Creates the composer items according to the current map tools and keeps track
 * of the rubber band item.
 */
class QgsComposerView: QGraphicsView
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
      Select = 0,      // Select/Move item
      AddArrow,         //add arrow
      AddHtml,
      AddMap,          // add new map
      AddLegend, // add vector legend
      AddLabel,        // add label
      AddScalebar,     // add scalebar
      AddPicture,       // add raster/vector picture
      AddRectangle,
      AddEllipse,
      AddTriangle,
      AddTable, //add attribute table
      MoveItemContent //move content of item (e.g. content of map)
    };

    QgsComposerView( QWidget* parent = 0, const char* name = 0, Qt::WFlags f = 0 );

    /**Add an item group containing the selected items*/
    void groupItems();

    /**Ungroups the selected items*/
    void ungroupItems();

    QgsComposerView::Tool currentTool() const;
    void setCurrentTool( QgsComposerView::Tool t );

    /**Sets composition (derived from QGraphicsScene)*/
    void setComposition( QgsComposition* c );
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

  protected:
    void mousePressEvent( QMouseEvent* );
    void mouseReleaseEvent( QMouseEvent* );
    void mouseMoveEvent( QMouseEvent* );
    void mouseDoubleClickEvent( QMouseEvent* e );

    void keyPressEvent( QKeyEvent * e );

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

    /**Emitted before composerview is shown*/
    void composerViewShow( QgsComposerView* );
    /**Emitted before composerview is hidden*/
    void composerViewHide( QgsComposerView* );
};
