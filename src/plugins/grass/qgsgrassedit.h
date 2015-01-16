/***************************************************************************
    qgsgrassedit.h  -    GRASS Edit
                             -------------------
    begin                : March, 2004
    copyright            : (C) 2004 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGRASSEDIT_H
#define QGSGRASSEDIT_H

#include <vector>

#include <QMainWindow>

class QAction;
class QPainter;
class QPen;
class QPixmap;
class QString;
class QCloseEvent;


#include "qgspoint.h"

class QgisInterface;
class QgsMapCanvas;
class QgsMapLayer;
class QgsMapToPixel;
class QgsRubberBand;
class QgsVertexMarker;
class QgsVectorLayer;
class QgsMapTool;
class QgsGrassEditLayer;
class QgsGrassAttributes;

class QgsGrassProvider;
#include "ui_qgsgrasseditbase.h"
#include "qgsgrassselect.h"

// forward declaration of edit tools
class QgsGrassEditTool;
class QgsGrassEditNewPoint;
class QgsGrassEditNewLine;
class QgsGrassEditMoveVertex;
class QgsGrassEditAddVertex;
class QgsGrassEditDeleteVertex;
class QgsGrassEditMoveLine;
class QgsGrassEditDeleteLine;
class QgsGrassEditSplitLine;
class QgsGrassEditAttributes;

typedef struct
{
  int field;
  int maxCat;
} MaxCat;

/*! \class QgsGrassEdit
 *  \brief GRASS vector edit.
 *
 */
class QgsGrassEdit: public QMainWindow, private Ui::QgsGrassEditBase
{
    Q_OBJECT

  public:
    //! Tools
    enum EditTools
    {
      NONE,
      NEW_POINT,
      NEW_LINE,
      NEW_BOUNDARY,
      NEW_CENTROID,
      MOVE_VERTEX,
      ADD_VERTEX,
      DELETE_VERTEX,
      SPLIT_LINE,
      MOVE_LINE,
      DELETE_LINE,
      EDIT_CATS,
      EDIT_ATTRIBUTES
    };

    // Symbology codes */
    enum SymbCode
    {
      SYMB_BACKGROUND,
      SYMB_HIGHLIGHT,
      SYMB_DYNAMIC,
      SYMB_POINT,
      SYMB_LINE,
      SYMB_BOUNDARY_0, /* No areas */
      SYMB_BOUNDARY_1, /* 1 area */
      SYMB_BOUNDARY_2, /* 2 areas */
      SYMB_CENTROID_IN,    /* Centroid in area */
      SYMB_CENTROID_OUT,   /* Centroid outside area */
      SYMB_CENTROID_DUPL,  /* Duplicate centroid in area */
      SYMB_NODE_0,     /* Node without lines (points or centroids) */
      SYMB_NODE_1,     /* Node with 1 line */
      SYMB_NODE_2,     /* Node with 2 lines */
      SYMB_COUNT       /* MUST BE LAST, number of symbology layers */
    };

    enum CatMode
    {
      CAT_MODE_NEXT = 0,
      CAT_MODE_MANUAL,
      CAT_MODE_NOCAT
    };

    //! Constructor
    QgsGrassEdit( QgisInterface *iface,
                  QgsMapLayer *layer, bool newMap,
                  QWidget * parent = 0, Qt::WindowFlags f = 0 );

    // Shared by constructors
    void init();

    //! Destructor
    ~QgsGrassEdit();

    //! Close editing
    bool isValid( void );

    //! Close editing
    static bool isRunning( void );

    //! Set new tool and close old one if any
    void startTool( int );

    //! Add category to selected line
    void addCat( int line );

    //! Delete category from selected line
    void deleteCat( int line, int field, int cat );

    //! Add attributes to current mAttributes
    void addAttributes( int field, int cat );

    //! Increase max cat
    void increaseMaxCat( void );

    //! Check orphan database records
    void checkOrphan( int field, int cat );

    //! pointer to layer
    QgsVectorLayer *layer() { return mLayer; }

  public slots:
    // TODO: once available in QGIS, use only one reciver for all signals

    //! Called when rendering is finished
    void postRender( QPainter * );

    // Slots to start tools (is it possible to pass tool to startTool() directly from GUI ?)
    void newPoint( void );      // digitize new point
    void newLine( void );       // digitize new line
    void newBoundary( void );   // digitize new boundary
    void newCentroid( void );   // digitize new centroid
    void moveVertex( void );    // move vertex of existing line/boundary
    void addVertex( void );     // add new vertex to existing line/boundary
    void deleteVertex( void );  // delete vertex of existing line/boundary
    void splitLine( void );     // split existing line/boundary
    void moveLine( void );      // move existing line/boundary
    void deleteLine( void );    // delete existing line/boundary
    void editCats( void );      // edit element categories
    void editAttributes( void ); // edit element attributes

    //! Category mode was changed
    void on_mCatModeBox_activated() { catModeChanged(); }
    void catModeChanged();

    //! Field was changed
    void on_mFieldBox_activated() { fieldChanged(); }
    void fieldChanged();

    // Change attribute table
    void on_mTableField_activated() { attributeTableFieldChanged(); }
    void attributeTableFieldChanged();

    // Add column
    void on_mAddColumnButton_clicked() { addColumn(); }
    void addColumn();

    // Alter table
    void on_mAlterTableButton_clicked() { alterTable(); }
    void alterTable();

    //! Close editing
    void closeEdit();

    void changeSymbology( QTreeWidgetItem * item, int col );

    //! width/size changed
    void lineWidthChanged();
    void markerSizeChanged();
    void on_mLineWidthSpinBox_valueChanged() { lineWidthChanged(); }
    void on_mMarkerSizeSpinBox_valueChanged() { markerSizeChanged(); }

    // The type of column was changed
    void columnTypeChanged( int row, int col );

    // ! Close event
    void closeEvent( QCloseEvent *e ) override;

    static bool isEditable( QgsMapLayer *layer );

    //! Window with attributes closed
    void attributesClosed();

    //! Receive key press from different widget
    void keyPress( QKeyEvent *e );

  signals:
    void finished();

  private:
    //! Editing is already running
    static bool mRunning;

    //! Pointer to edited layer
    QgsVectorLayer *mLayer;

    //! Pointer to toolbar
    QToolBar *mToolBar;

    //! Point / node size (later make editable array of Sizes)
    int mSize;

    //! Transform from layer coordinates to canvas including reprojection
    QgsPoint transformLayerToCanvas( QgsPoint point );

    //! Transform from layer coordinates to current projection
    QgsPoint transformLayerToMap( QgsPoint point );

    //! Display all lines and nodes
    void displayMap();

    /**
     *  Display icon
     *  @param x x coordinate in map units
     *  @param y y coordinate in map units
     *  @param type ICON_CROSS, ICON_X
     *  @param size size in pixels, should be odd number
     */
    void displayIcon( double x, double y, const QPen & pen, int type, int size, QPainter *painter = 0 );

    /**
     *  Display dynamic drawing (XOR)
     *  Old drawing (in mLastDynamicPoints) is deleted first, and new one is drawn and
     *  mLastDynamicPoints is set to Points.
     *  SYMB_DYNAMIC color is used
     */
    void displayDynamic( struct line_pnts *Points );

    /** Display dynamic icon */
    void displayDynamic( double x, double y, int type, int size );

    /* Display dynamic points + icon */
    void displayDynamic( struct line_pnts *Points, double x, double y, int type, int size );

    /** Erase dynamic */
    void eraseDynamic( void );

    /**
     *  Display map element (lines and points)
     *  @param line line number
     */
    void displayElement( int line, const QPen & pen, int size, QPainter *painter = 0 );

    /**
     *  Erase element and its nodes using SYMB_BACKGROUD
     *  @param line line number
     */
    void eraseElement( int line );

    /**
     *  Display one node
     *  @param painter pointer to painter or 0
     *  @param node node number
     *  @param size size in pixels, should be odd number
     */
    void displayNode( int node, const QPen & pen, int size, QPainter *painter = 0 );

    //! Status: true - active vector was successfully opened for editing
    bool mValid;

    //! Initialization complete
    bool mInited;

    //! Pointer to the QGIS interface object
    QgisInterface *mIface;

    //! Pointer to canvas
    QgsMapCanvas *mCanvas;

    //! Pointer to vector provider
    QgsGrassProvider *mProvider;

    //! Current tool (EditTools)
    int mTool;

    //! If tool is not closed and another QGIS tool is selected, suspend is set to true.
    //  GRASS Edit will continue if the same tool is selected later.
    bool mSuspend;

    //! Currently digitized line points,
    //  points of currently mooving line (original coordinates)
    struct line_pnts *mEditPoints;

    //! Working structure for line points
    struct line_pnts *mPoints;

    //! Working category structure
    struct line_cats *mCats;

    //! Vector of maximum used category values for each field
    std::vector<MaxCat> mMaxCats;

    //! Canvas pixmap
    QPixmap *mPixmap;

    //! Copy of background from canvas pixmap before any draw
    //QPixmap *mBackgroundPixmap;

    //! Transformation
    const QgsMapToPixel* mTransform;

    //! Last point where user clicked (map units)
    QgsPoint mLastPoint;

    //! Selected line or 0
    int mSelectedLine;

    //! Selected segment or vertex (first is 0) (MOVE_VERTEX, ADD_VERTEX, DELETE_VERTEX)
    int mSelectedPart;

    //! Appand a new vertex at the end (not insert into existing segment)
    bool mAddVertexEnd;

    //! Vector of symbology codes for lines, indexes of lines start at 1, always resize nlines+1
    std::vector<int> mLineSymb;

    //! Vector of symbology codes for nodes, indexes of nodes start at 1, always resize nnodes+1
    std::vector<int> mNodeSymb;

    //! Vector of pens for symbology codes
    std::vector<QPen> mSymb;

    //! Display this type
    std::vector<bool> mSymbDisplay;

    //! Symbology name
    std::vector<QString> mSymbName;

    //! Line width
    int mLineWidth;

    //! Marker size
    int mMarkerSize;

    /**
     *  Read line symbology from map
     *  @return symbology code
     */
    int lineSymbFromMap( int line );

    /**
     *  Read node symbology from map
     *  @return symbology code
     */
    int nodeSymbFromMap( int node );

    /**
     *  Set symbology of updated lines and nodes from map
     */
    void updateSymb( void );

    /**
     *  Display updated lines and nodes
     */
    void displayUpdated( void );

    /** Write new element. Current field category is taken.
     * return line number
     */
    int writeLine( int type, struct line_pnts *Points );

    /** Get Current threshold in map units */
    double threshold( void );

    /** Snap to nearest node in current threshold */
    void snap( QgsPoint & point );
    void snap( double *x, double *y );
    /** Snap point line  considering line starting point */
    void snap( QgsPoint & point, double startX, double startY );

    /** Attributes */
    QgsGrassAttributes *mAttributes;

    void restorePosition( void );

    void saveWindowLocation( void );

    // Set attribute table
    void setAttributeTable( int field );

    // Pront which should be displayed in status bar when mouse is in canvas
    QString mCanvasPrompt;

    // Set prompt for mouse buttons
    void setCanvasPrompt( QString left, QString mid, QString right );

    // New map, add new layers on exit
    bool mNewMap;

    // Actions
    QAction *mNewPointAction;
    QAction *mNewLineAction;
    QAction *mNewBoundaryAction;
    QAction *mNewCentroidAction;
    QAction *mMoveVertexAction;
    QAction *mAddVertexAction;
    QAction *mDeleteVertexAction;
    QAction *mMoveLineAction;
    QAction *mSplitLineAction;
    QAction *mDeleteLineAction;
    QAction *mEditAttributesAction;
    QAction *mCloseEditAction;

    // Current map tool
    QgsMapTool *mMapTool;

    // Is projection enabled?
    bool mProjectionEnabled;

    // Canvas items
    QgsGrassEditLayer* mCanvasEdit;
    QgsRubberBand *mRubberBandLine;
    QgsVertexMarker *mRubberBandIcon;

    // edit tools are friend classes so they can
    // access proteced/private members of QgsGrassEdit
    friend class QgsGrassEditTool;
    friend class QgsGrassEditNewPoint;
    friend class QgsGrassEditNewLine;
    friend class QgsGrassEditMoveVertex;
    friend class QgsGrassEditAddVertex;
    friend class QgsGrassEditDeleteVertex;
    friend class QgsGrassEditMoveLine;
    friend class QgsGrassEditDeleteLine;
    friend class QgsGrassEditSplitLine;
    friend class QgsGrassEditAttributes;
};

#endif // QGSGRASSEDIT_H
