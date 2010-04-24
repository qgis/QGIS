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
      AddMap,          // add new map
      AddLegend, // add vector legend
      AddLabel,        // add label
      AddScalebar,     // add scalebar
      AddPicture,       // add raster/vector picture
      AddShape, //add shape item (ellipse, rectangle, triangle)
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
    
    /**Adds label to the graphics scene and advices composer to create a widget for it (through signal)*/
    void addComposerLabel( QgsComposerLabel* label);
    /**Adds map to the graphics scene and advices composer to create a widget for it (through signal)*/
    void addComposerMap( QgsComposerMap* map );
    /**Adds scale bar to the graphics scene and advices composer to create a widget for it (through signal)*/
    void addComposerScaleBar( QgsComposerScaleBar* scaleBar );
    /**Adds legend to the graphics scene and advices composer to create a widget for it (through signal)*/
    void addComposerLegend( QgsComposerLegend* legend );
    /**Adds picture to the graphics scene and advices composer to create a widget for it (through signal)*/
    void addComposerPicture( QgsComposerPicture* picture );
    /**Adds a composer shape to the graphics scene and advices composer to create a widget for it (through signal)*/
    void addComposerShape( QgsComposerShape* shape );

    /**Returns the composer main window*/
    QMainWindow* composerWindow();

  protected:
    void mousePressEvent( QMouseEvent* );
    void mouseReleaseEvent( QMouseEvent* );
    void mouseMoveEvent( QMouseEvent* );

    void keyPressEvent( QKeyEvent * e );
    void keyReleaseEvent( QKeyEvent * e );

    void wheelEvent( QWheelEvent* event );
    

  public slots:
    /**For QgsComposerItemGroup to send its signals to QgsComposer (or other classes that keep track of input widgets)*/
    void sendItemRemovedSignal( QgsComposerItem* item );

  signals:
    /**Is emitted when selected item changed. If 0, no item is selected*/
    void selectedItemChanged( const QgsComposerItem* selected );
    /**Ist emittted when new composer label has been added to the view*/
    void composerLabelAdded( QgsComposerLabel* label );
    /**Is emitted when new composer map has been added to the view*/
    void composerMapAdded( QgsComposerMap* map );
    /**Is emitted when new composer scale bar has been added*/
    void composerScaleBarAdded( QgsComposerScaleBar* scalebar );
    /**Is emitted when a new composer legend has been added*/
    void composerLegendAdded( QgsComposerLegend* legend );
    /**Is emitted when a new composer picture has been added*/
    void composerPictureAdded( QgsComposerPicture* picture );
    /**Is emitted when a composer item has been removed from the scene*/
    void itemRemoved( QgsComposerItem* );
    /**Current action (e.g. adding composer map) has been finished. The purpose of this signal is that
     QgsComposer may set the selection tool again*/
    void actionFinished();
};
