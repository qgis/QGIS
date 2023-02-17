/**********************************************************************
                           qgsgrassmapcalc.h
                             -------------------
    begin                : September, 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : radim.blazek@gmail.com
 **********************************************************************/
/************************************************************************
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *************************************************************************/
#ifndef QGSGRASSMAPCALC_H
#define QGSGRASSMAPCALC_H

#include "qgsgrassmoduleinput.h"
#include "ui_qgsgrassmapcalcbase.h"
#include "qgsgrassmodule.h"

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsView>

class QgsGrassMapcalcConnector;
class QgsGrassMapcalcFunction;
class QgsGrassMapcalcObject;
class QgsGrassMapcalcView;

/*!
 *  \class QgsGrassMapcalc
 *  \brief Interface for r.mapcalc
 */
class QgsGrassMapcalc: public QMainWindow, private Ui::QgsGrassMapcalcBase,
  public QgsGrassModuleOptions
{
    Q_OBJECT

  public:
    //! Constructor
    QgsGrassMapcalc(
      QgsGrassTools *tools, QgsGrassModule *module,
      QgisInterface *iface,
      QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );

    // Current tool
    enum Tool
    {
      AddMap = 0,
      AddConstant,
      AddFunction,
      AddConnector,
      Select
    };

    //! Gets module options as list of arguments for QProcess
    QStringList arguments() override;

    // Reimplemented methods
    QStringList checkOutput() override;
    QStringList ready() override { return QStringList() ; }
    bool requestsRegion() override { return false; }
    bool usesRegion() override { return true; }
    QStringList checkRegion() override;
    bool inputRegion( struct Cell_head *window, QgsCoordinateReferenceSystem &crs, bool all ) override;
    QStringList output( int type ) override;
    bool hasOutput( int type ) override
    { Q_UNUSED( type ) return true; }

    //! \brief receives contentsMousePressEvent from view
    void mousePressEvent( QMouseEvent * ) override;

    //! \brief receives contentsMouseReleaseEvent from view
    void mouseReleaseEvent( QMouseEvent * ) override;

    //! \brief receives contentsMouseMoveEvent from view
    void mouseMoveEvent( QMouseEvent * ) override;

    void keyPressEvent( QKeyEvent *e ) override;

    //! Cut coordinates by current canvas extent
    void limit( QPoint * );

    //! Grow canvas and move items
    void growCanvas( int left, int right, int top, int bottom );

    //! Grow automatically if an item is near border
    void autoGrow();

    void resizeCanvas( int width, int height );

    //! Show/hide options for tool
    void showOptions( int tool );

    //! Sets option for selected object
    void setOption( void );

  public slots:
    //! Add new map
    void addMap( void );

    //! Add constant
    void addConstant( void );

    //! Add function
    void addFunction( void );

    //! Add connection
    void addConnection( void );

    //! Select item
    void selectItem( void );

    //! Delete selected item
    void deleteItem( void );

    //! Reset tool actions togles
    void setToolActionsOff( void );

    //! Sets current tool and toggle menu
    void setTool( int );

    //! Map selection changed
    void mapChanged( const QString &text );

    //! Constant changed
    void mConstantLineEdit_textChanged() { constantChanged(); }
    void constantChanged();

    //! Function selection changed
    void mFunctionComboBox_activated() { functionChanged(); }
    void functionChanged();

    //! Save current state to file
    void save();
    void saveAs();

    //! Load from file
    void load();

    //! Remove all items including output
    void clear();

    // Get next item id and increase counter
    int nextId() { return mNextId++; }

  private:
    // Canvas view
    QgsGrassMapcalcView *mView = nullptr;

    // Canvas
    QGraphicsScene *mCanvasScene = nullptr;

    // Tool
    int mTool;
    int mToolStep;

    // Pointer to current object
    QgsGrassMapcalcObject *mObject = nullptr;

    // Pointer to current connector
    QgsGrassMapcalcConnector *mConnector = nullptr;

    QgsGrassModuleInputComboBox *mMapComboBox = nullptr;

    //! Last point position
    QPoint mLastPoint;

    //! Start point with move
    QPoint mStartMovePoint;

    //! Start end points of connector with move
    std::vector<QPoint> mStartMoveConnectorPoints;

    //! Available functions
    std::vector<QgsGrassMapcalcFunction> mFunctions;

    //! Output object
    QgsGrassMapcalcObject *mOutput = nullptr;

    //! Current file name, empty if no file is loaded/saved
    QString mFileName;

    //! Item id
    unsigned int mNextId;

    //! Background
    QGraphicsRectItem *mPaper = nullptr;

    // Actions
    QAction *mActionAddMap = nullptr;
    QAction *mActionAddConstant = nullptr;
    QAction *mActionAddFunction = nullptr;
    QAction *mActionAddConnection = nullptr;
    QAction *mActionSelectItem = nullptr;
    QAction *mActionDeleteItem = nullptr;

    QAction *mActionLoad = nullptr;
    QAction *mActionSave = nullptr;
    QAction *mActionSaveAs = nullptr;

    QgsGrassMapcalc( const QgsGrassMapcalc & ) = delete;
    QgsGrassMapcalc &operator = ( const QgsGrassMapcalc & ) = delete;
};

/*
 * Function. Represents function or operator data.
 */
class QgsGrassMapcalcFunction
{
  public:
    enum Type
    {
      Operator = 0,
      Function
    };

    QgsGrassMapcalcFunction() = default;
    QgsGrassMapcalcFunction( int type, QString name, int count = 2,
                             QString description = "", QString label = "",
                             QString labels = "", bool drawLabel = true );
    ~QgsGrassMapcalcFunction() = default;

    QString name() const { return mName; }
    int     type() const { return mType; }
    int     inputCount() const { return mInputCount; }
    QString label() const { return mLabel; }
    QString description() const { return mDescription; }
    QStringList inputLabels() const { return mInputLabels; }
    bool drawlabel() const { return mDrawLabel; }

  private:
    /* Value used in expression, e.g. 'if' */
    QString mName;

    int mType = 0;

    /* Number of inputs */
    int mInputCount = 0;

    /* Identification name, e.g., 'if(x,a,b)' */
    //QString mName;

    /* Label used in combobox and objects */
    QString mLabel;

    /* More detailed description */
    QString mDescription;

    /* Input labels */
    QStringList mInputLabels;

    // Draw main label in box
    bool mDrawLabel = false;
};

/******************** CANVAS ITEMS *****************************/
/*
 * Base class inherited by QgsGrassMapcalcObject and
 * QgsGrassMapcalcConnector
 */
class QgsGrassMapcalcItem
{
  public:
    QgsGrassMapcalcItem() = default;
    virtual ~QgsGrassMapcalcItem() = default;

    virtual void setSelected( bool s ) { mSelected = s; }
    bool selected( void ) const { return mSelected; }
//    virtual void paint ( QPainter * painter,
//      const QStyleOptionGraphicsItem * option, QWidget * widget );
//
    int id() const { return mId; }
    void setId( int id ) { mId = id; }

  protected:
    bool mSelected = false;

    int mId = -1;
};

/*
 * QgsGrassMapcalcObject represents map, constant or function
 *
 * All coordinates are calculated from mCenter using font size,
 * number of input and labels.
 *
 *      QGraphicsRectItem.x()
 *      |
 *      | mRect.x()
 *      | |           mCenter.x()
 *      | |           |
 *      +----------------------------+------QCanvasRectangle.y()
 *      | +------------------------+ |---mRect.y()
 *      | | +---------+            | |
 *      |o| | Input 1 |            | |
 *      | | +---------+ +--------+ | |----------------+
 *  +---| |             | Label  | |o|--mCenter.y()   |
 *  |   | | +---------+ +--------+ | |----------------+
 *  |   |o| | Input 2 |            | |                |
 *  |   | | +---------+            | |                mTextHeight
 *  +---| +------------------------+ |
 *  |   +----------------------------+
 *  |       |         | |          | |
 *  |       |         | |          +-+---mMargin = 2*mSocketHalf+1
 *  |       +---------+-- mInputTextWidth
 *  |                   |
 *  mInputHeight        mLabelX
 */
class QgsGrassMapcalcObject: public QGraphicsRectItem, public QgsGrassMapcalcItem
{
  public:
    enum Type
    {
      Map = 0,      // raster map
      Constant,
      Function,
      Output
    };

    enum Dir
    {
      In = 0,
      Out,
      None
    };

    explicit QgsGrassMapcalcObject( int type );
    ~QgsGrassMapcalcObject() override;

    // Set map name, constant value or function/operator
    void setValue( QString val, QString lab = "" );

    // Set function
    void setFunction( QgsGrassMapcalcFunction f );

    void paint( QPainter *painter,
                const QStyleOptionGraphicsItem *option, QWidget *widget ) override;

    // Set object center
    void setCenter( int, int );

    // Get center point
    QPoint center() { return mCenter; }

    // Recalculate size
    void resetSize();

    void setSelected( bool s ) override;

    // Try to connect connector end
    bool tryConnect( QgsGrassMapcalcConnector *, int );

    // Get socket coordinates
    QPoint socketPoint( int direction, int socket );

    // Set socket's connector
    void setConnector( int direction, int socket,
                       QgsGrassMapcalcConnector *connector = nullptr, int end = 0 );

    // Object type
    int type() const override;

    // Value
    QString value() { return mValue; }

    // label
    QString label() { return mLabel; }

    //! Function
    QgsGrassMapcalcFunction function() { return mFunction; }

    // Expression
    QString expression();

  private:
//    bool mSelected;

    // Object type: Map,Constant,Function
    int mType;

    // Map name, constant or function/operator
    QString mValue;

    // Label
    QString mLabel;

    // Number of inputs
    int mInputCount;

    // Number of outputs (0 or 1)
    int mOutputCount;

    // Function
    QgsGrassMapcalcFunction mFunction;

    // Label font
    QFont mFont;

    // Drawn rectangle
    QRect mRect;

    // Rounding of box
    int mRound;

    // Center of object
    QPoint mCenter;

    // Half size of socket symbol
    int mSocketHalf;

    // Margin between mRect and QCanvasRectangle.rect()
    int mMargin;

    // Space between text boxes
    int mSpace;

    // Height of text box
    int mTextHeight;

    // Maximum width of input labels
    int mInputTextWidth;

    // Label box
    QRect mLabelRect;

    // Coordinates of input sockets
    std::vector<QPoint> mInputPoints;

    // Coordinates of output
    QPoint mOutputPoint;

    // Selection box size
    int mSelectionBoxSize;

    // Input connectors
    std::vector<QgsGrassMapcalcConnector *> mInputConnectors;
    std::vector<int> mInputConnectorsEnd;

    // Output connector
    QgsGrassMapcalcConnector *mOutputConnector = nullptr;
    int mOutputConnectorEnd;

};

/*
 * Connector.
 * End are stored in vectors with indexes 0,1
 */
class QgsGrassMapcalcConnector: public QGraphicsLineItem, public QgsGrassMapcalcItem
{
  public:
    explicit QgsGrassMapcalcConnector( QGraphicsScene * );
    ~QgsGrassMapcalcConnector() override;

    void paint( QPainter *painter,
                const QStyleOptionGraphicsItem *option, QWidget *widget ) override;

    // Set connector end point coordinates
    void setPoint( int, QPoint );

    QPoint point( int );

    // Recalculate size
    //void resetSize();

    void setSelected( bool s ) override;

    // Select end
    void selectEnd( QPoint );

    // Which end is selected
    int selectedEnd();

    // Try to connect specified end to an object
    bool tryConnectEnd( int end );

    // Register end as connected to object/input
    // If this end of connector was connected to an object
    // the connection is also deleted from object
    // If object is NULL the old connection is deleted.
    void setSocket( int end, QgsGrassMapcalcObject *object = nullptr,
                    int direction = QgsGrassMapcalcObject::None,
                    int socket = 0
                  );

    // Return pointer to object on end
    QgsGrassMapcalcObject *object( int end );

    // End object direction
    int socketDirection( int end ) { return mSocketDir[end]; }

    // End object socket number
    int socket( int end ) { return mSocket[end]; }

    // Refresh/repaint
    void repaint();

    // Is it connected to a socket of given direction
    bool connected( int direction );

    // Expression
    QString expression();

  private:
    // Coordinates of ends
    std::vector<QPoint> mPoints;

    // Selected end, -1 for whole connector
    int mSelectedEnd = -1;

    // Connected objects
    std::vector<QgsGrassMapcalcObject *> mSocketObjects;
    std::vector<int> mSocketDir;
    std::vector<int> mSocket;
};

/******************** CANVAS VIEW ******************************/
class QgsGrassMapcalcView: public QGraphicsView
{
    Q_OBJECT

  public:
    QgsGrassMapcalcView( QgsGrassMapcalc *mapcalc, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );

  protected:
    void mousePressEvent( QMouseEvent *e ) override;
    void mouseReleaseEvent( QMouseEvent *e ) override;
    void mouseMoveEvent( QMouseEvent *e ) override;
    void keyPressEvent( QKeyEvent *e ) override;

  private:
    QgsGrassMapcalc *mMapcalc = nullptr;

};

#endif // QGSGRASSMAPCALC_H
