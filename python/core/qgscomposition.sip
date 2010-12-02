/** \ingroup MapComposer
 * Graphics scene for map printing. The class manages the paper item which always
 * is the item in the back (z-value 0). It maintains the z-Values of the items and stores
 * them in a list in ascending z-Order. This list can be changed to lower/raise items one position
 * or to bring them to front/back.
 * */
class QgsComposition: QGraphicsScene
{
%TypeHeaderCode
#include <qgscomposition.h>
%End
  public:

    /** \brief Plot type */
    enum PlotStyle
    {
      Preview = 0, // Use cache etc
      Print,       // Render well
      Postscript   // Fonts need different scaling!
    };

    /**Style to draw the snapping grid*/
    enum GridStyle
    {
      Solid,
      Dots,
      Crosses
    };

    QgsComposition( QgsMapRenderer* mapRenderer );
    ~QgsComposition();

    /**Changes size of paper item*/
    void setPaperSize( double width, double height );

    /**Returns height of paper item*/
    double paperHeight() const;

    /**Returns width of paper item*/
    double paperWidth() const;

    void setSnapToGridEnabled( bool b );
    bool snapToGridEnabled() const;

    void setSnapGridResolution( double r );
    double snapGridResolution() const;

    void setSnapGridOffsetX( double offset );
    double snapGridOffsetX() const;

    void setSnapGridOffsetY( double offset );
    double snapGridOffsetY() const;

    void setGridPen( const QPen& p );
    const QPen& gridPen() const;

    void setGridStyle( GridStyle s );
    GridStyle gridStyle() const;

    /**Returns pointer to undo/redo command storage*/
    QUndoStack* undoStack();

    /**Returns the topmose composer item. Ignores mPaperItem*/
    QgsComposerItem* composerItemAt( const QPointF & position );

    QList<QgsComposerItem*> selectedComposerItems();

    /**Returns pointers to all composer maps in the scene*/
    //todo: needs a new mapping for QList<const T*> ?
    //QList<const QgsComposerMap*> composerMapItems() const;

    /**Returns the composer map with specified id
     @return id or 0 pointer if the composer map item does not exist*/
    const QgsComposerMap* getComposerMapById( int id ) const;

    int printResolution() const;
    void setPrintResolution( int dpi );

    bool printAsRaster() const;
    void setPrintAsRaster(bool enabled);

    /**Returns pointer to map renderer of qgis map canvas*/
    QgsMapRenderer* mapRenderer();

    QgsComposition::PlotStyle plotStyle();
    void setPlotStyle( QgsComposition::PlotStyle style );

    /**Returns the pixel font size for a font that has point size set.
     The result depends on the resolution (dpi) and of the preview mode. Each item that sets
    a font should call this function before drawing text*/
    int pixelFontSize( double pointSize ) const;

    /**Does the inverse calculation and returns points for pixels (equals to mm in QgsComposition)*/
    double pointFontSize( int pixelSize ) const;

    /**Writes settings to xml (paper dimension)*/
    bool writeXML( QDomElement& composerElem, QDomDocument& doc );

    /**Reads settings from xml file*/
    bool readXML( const QDomElement& compositionElem, const QDomDocument& doc );

    /**Adds item to z list. Usually called from constructor of QgsComposerItem*/
    void addItemToZList( QgsComposerItem* item );
    /**Removes item from z list. Usually called from destructor of QgsComposerItem*/
    void removeItemFromZList( QgsComposerItem* item );

    //functions to move selected items in hierarchy
    void raiseSelectedItems();
    void raiseItem( QgsComposerItem* item );
    void lowerSelectedItems();
    void lowerItem( QgsComposerItem* item );
    void moveSelectedItemsToTop();
    void moveItemToTop( QgsComposerItem* item );
    void moveSelectedItemsToBottom();
    void moveItemToBottom( QgsComposerItem* item );

    //functions to align selected items
    void alignSelectedItemsLeft();
    void alignSelectedItemsHCenter();
    void alignSelectedItemsRight();
    void alignSelectedItemsTop();
    void alignSelectedItemsVCenter();
    void alignSelectedItemsBottom();

    /**Sorts the zList. The only time where this function needs to be called is from QgsComposer
     after reading all the items from xml file*/
    void sortZList();

    /**Snaps a scene coordinate point to grid*/
    QPointF snapPointToGrid( const QPointF& scenePoint ) const;

    /**Allocates new item command and saves initial state in it
      @param item target item
      @param commandText descriptive command text
      @param c context for merge commands (unknown for non-mergeable commands)*/
    void beginCommand( QgsComposerItem* item, const QString& commandText, QgsComposerMergeCommand::Context c = QgsComposerMergeCommand::Unknown );

    /**Saves end state of item and pushes command to the undo history*/
    void endCommand();
    /**Deletes current command*/
    void cancelCommand();
};
