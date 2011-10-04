/**********************************************************************
                        qgsgrassmapcalc.cpp
                       -------------------
    begin                : September, 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : radim.blazek@gmail.com
**********************************************************************/
/**********************************************************************
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 *************************************************************************/

#include "qgsgrassmapcalc.h"
#include "qgsgrassselect.h"
#include "qgsgrass.h"

#include "qgisinterface.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsgrassplugin.h"

#include <cmath>
#include <typeinfo>

#include <QDir>
#include <QDomDocument>
#include <QInputDialog>
#include <QMessageBox>
#include <QMouseEvent>
#include <QTextStream>
#include <QToolBar>


QgsGrassMapcalc::QgsGrassMapcalc(
  QgsGrassTools *tools, QgsGrassModule *module,
  QgisInterface *iface,
  QWidget * parent, Qt::WFlags f )
    : QMainWindow( 0, Qt::Dialog )
    , QgsGrassMapcalcBase( )
    , QgsGrassModuleOptions( tools, module, iface )
    , mTool( -1 )
    , mObject( 0 )
    , mConnector( 0 )
{
  Q_UNUSED( parent );
  Q_UNUSED( f );
  QgsDebugMsg( "QgsGrassMapcalc()" );

  setupUi( this );

  mStartMoveConnectorPoints.resize( 2 );
  mNextId = 0;

  // Set frame fixed (but for current desktop font/theme)
  mInputFrame->setMinimumHeight( mInputFrame->height() );
  mInputFrame->setMaximumHeight( mInputFrame->height() );

  mView = new QgsGrassMapcalcView( this, mViewFrame );
  QGridLayout *layout = new QGridLayout( mViewFrame );
  layout->addWidget( mView, 0, 0 );

  mCanvas = new QGraphicsScene( 0, 0, 400, 300 );
  mCanvas->setBackgroundBrush( QColor( 180, 180, 180 ) );

  mPaper = new QGraphicsRectItem();
  mCanvas->addItem( mPaper );
  mPaper->setBrush( QBrush( QColor( 255, 255, 255 ) ) );
  mPaper->show();

  resizeCanvas( 400, 300 );

  mView->setScene( mCanvas );


  QActionGroup *ag = new QActionGroup( this );
  QToolBar *tb = addToolBar( tr( "Mapcalc tools" ) );

  mActionAddMap = new QAction( QgsGrassPlugin::getThemeIcon( "mapcalc_add_map.png" ),
                               tr( "Add map" ), this );
  mActionAddMap->setCheckable( true );
  ag->addAction( mActionAddMap );
  tb->addAction( mActionAddMap );
  connect( mActionAddMap, SIGNAL( triggered() ), this, SLOT( addMap() ) );

  mActionAddConstant = new QAction( QgsGrassPlugin::getThemeIcon( "mapcalc_add_constant.png" ),
                                    tr( "Add constant value" ), this );
  mActionAddConstant->setCheckable( true );
  ag->addAction( mActionAddConstant );
  tb->addAction( mActionAddConstant );
  connect( mActionAddConstant, SIGNAL( triggered() ), this, SLOT( addConstant() ) );

  mActionAddFunction = new QAction( QgsGrassPlugin::getThemeIcon( "mapcalc_add_function.png" ),
                                    tr( "Add operator or function" ), this );
  mActionAddFunction->setCheckable( true );
  ag->addAction( mActionAddFunction );
  tb->addAction( mActionAddFunction );
  connect( mActionAddFunction, SIGNAL( triggered() ), this, SLOT( addFunction() ) );

  mActionAddConnection = new QAction( QgsGrassPlugin::getThemeIcon( "mapcalc_add_connection.png" ),
                                      tr( "Add connection" ), this );
  mActionAddConnection->setCheckable( true );
  ag->addAction( mActionAddConnection );
  tb->addAction( mActionAddConnection );
  connect( mActionAddConnection, SIGNAL( triggered() ), this, SLOT( addConnection() ) );

  mActionSelectItem = new QAction( QgsGrassPlugin::getThemeIcon( "mapcalc_select.png" ),
                                   tr( "Select item" ), this );
  mActionSelectItem->setCheckable( true );
  ag->addAction( mActionSelectItem );
  tb->addAction( mActionSelectItem );
  connect( mActionSelectItem, SIGNAL( triggered() ), this, SLOT( selectItem() ) );

  mActionDeleteItem = new QAction( QgsGrassPlugin::getThemeIcon( "mapcalc_delete.png" ),
                                   tr( "Delete selected item" ), this );
  mActionDeleteItem->setCheckable( true );
  mActionDeleteItem->setEnabled( false );
  ag->addAction( mActionDeleteItem );
  tb->addAction( mActionDeleteItem );
  connect( mActionDeleteItem, SIGNAL( triggered() ), this, SLOT( deleteItem() ) );

  mActionAddMap->setChecked( true );

  mActionLoad = new QAction( QgsGrassPlugin::getThemeIcon( "mapcalc_open.png" ),
                             tr( "Open" ), this );
  tb->addAction( mActionLoad );
  connect( mActionLoad, SIGNAL( triggered() ), this, SLOT( load() ) );

  mActionSave = new QAction( QgsGrassPlugin::getThemeIcon( "mapcalc_save.png" ),
                             tr( "Save" ), this );
  tb->addAction( mActionSave );
  connect( mActionSave, SIGNAL( triggered() ), this, SLOT( save() ) );
  mActionSave->setEnabled( false );

  mActionSaveAs = new QAction( QgsGrassPlugin::getThemeIcon( "mapcalc_save_as.png" ),
                               tr( "Save as" ), this );
  tb->addAction( mActionSaveAs );
  connect( mActionSaveAs, SIGNAL( triggered() ), this, SLOT( saveAs() ) );

  /* Create functions */
  int t = QgsGrassMapcalcFunction::Operator;
  //mFunctions.push_back(QgsGrassMapcalcFunction("-",2, "Odcitani", "in1,in2" ));
  // Arithmetical
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "+", 2, tr( "Addition" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "-", 2, tr( "Subtraction" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "*", 2, tr( "Multiplication" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "/", 2, tr( "Division" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "%", 2, tr( "Modulus" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "^", 2, tr( "Exponentiation" ) ) );

  // Logical
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "==", 2, tr( "Equal" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "!=", 2, tr( "Not equal" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, ">",  2, tr( "Greater than" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, ">=", 2, tr( "Greater than or equal" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "<",  2, tr( "Less than" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "<=", 2, tr( "Less than or equal" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "&&", 2, tr( "And" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "||", 2, tr( "Or" ) ) );

  t = QgsGrassMapcalcFunction::Function;
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "abs",  1, tr( "Absolute value of x" ), "abs(x)" ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "atan", 1, tr( "Inverse tangent of x (result is in degrees)" ), "atan(x)" ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "atan", 2, tr( "Inverse tangent of y/x (result is in degrees)" ), "atan(x,y)" ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "col", 0, tr( "Current column of moving window (starts with 1)" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "cos",  1, tr( "Cosine of x (x is in degrees)" ), "cos(x)" ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "double", 1, tr( "Convert x to double-precision floating point" ), "double(x)" ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "ewres", 0, tr( "Current east-west resolution" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "exp", 1, tr( "Exponential function of x" ), "exp(x)" ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "exp", 2, tr( "x to the power y" ), "exp(x,y)" ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "float", 2, tr( "Convert x to single-precision floating point" ), "float(x)" ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "if", 1, tr( "Decision: 1 if x not zero, 0 otherwise" ), "if(x)" ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "if", 2, tr( "Decision: a if x not zero, 0 otherwise" ), "if(x,a)" ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "if", 3, tr( "Decision: a if x not zero, b otherwise" ), "if(x,a,b)", "if,then,else", false ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "if", 4, tr( "Decision: a if x > 0, b if x is zero, c if x < 0" ), "if(x,a,b,c)" ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "int", 1, tr( "Convert x to integer [ truncates ]" ), "int(x)" ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "isnull", 1, tr( "Check if x = NULL" ), "isnull(x)" ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "log", 1, tr( "Natural log of x" ), "log(x)" ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "log", 2, tr( "Log of x base b" ), "log(x,b)" ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "max", 2, tr( "Largest value" ), "max(a,b)" ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "max", 3, tr( "Largest value" ), "max(a,b,c)" ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "median", 2, tr( "Median value" ), "median(a,b)" ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "median", 3, tr( "Median value" ), "median(a,b,c)" ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "min", 2, tr( "Smallest value" ), "min(a,b)" ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "min", 3, tr( "Smallest value" ), "min(a,b,c)" ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "mode", 2, tr( "Mode value" ), "mode(a,b)" ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "mode", 3, tr( "Mode value" ), "mode(a,b,c)" ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "not", 1, tr( "1 if x is zero, 0 otherwise" ), "not(x)" ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "nsres", 0, tr( "Current north-south resolution" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "null", 0, tr( "NULL value" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "rand", 2, tr( "Random value between a and b" ), "rand(a,b)" ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "round", 1, tr( "Round x to nearest integer" ), "round(x)" ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "row", 0, tr( "Current row of moving window (Starts with 1)" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "sin", 1, tr( "Sine of x (x is in degrees)", "sin(x)" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "sqrt", 1, tr( "Square root of x", "sqrt(x)" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "tan", 1, tr( "Tangent of x (x is in degrees)", "tan(x)" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "x", 0, tr( "Current x-coordinate of moving window" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, "y", 0, tr( "Current y-coordinate of moving window" ) ) );

  for ( unsigned int i = 0; i < mFunctions.size(); i++ )
  {
    mFunctionComboBox->addItem( mFunctions[i].label()
                                + "  " + mFunctions[i].description() );
  }

  // Add output object
  mOutput = new QgsGrassMapcalcObject( QgsGrassMapcalcObject::Output );
  mOutput->setId( nextId() );
  mOutput->setValue( tr( "Output" ) );
  mCanvas->addItem( mOutput );
  mOutput->setCenter(( int )( mCanvas->width() - mOutput->rect().width() ), ( int )( mCanvas->height() / 2 ) );
  mCanvas->update();
  mOutput->QGraphicsRectItem::show();

  // Set default tool
  updateMaps();
  if ( mMaps.size() > 0 )
  {
    setTool( AddMap );
  }
  else
  {
    setTool( AddConstant );
  }
}

void QgsGrassMapcalc::mousePressEvent( QMouseEvent* e )
{
  QgsDebugMsg( QString( "mTool = %1 mToolStep = %2" ).arg( mTool ).arg( mToolStep ) );

  QPoint p = mView->mapToScene( e->pos() ).toPoint();
  limit( &p );

  switch ( mTool )
  {
    case AddMap:
    case AddConstant:
    case AddFunction:
      mObject->setCenter( p.x(), p.y() );
      mObject = 0;
      //addMap(); // restart
      setTool( mTool );  // restart
      break;

    case AddConnector:
      if ( mToolStep == 0 )
      {
        mConnector->setPoint( 0, p );
        mConnector->setPoint( 1, p );
        // Try to connect
        mConnector->tryConnectEnd( 0 );
        mToolStep = 1;
      }
      break;

    case Select:
      // Cleare previous
      if ( mObject )
      {
        mObject->setSelected( false );
        mObject = 0;
      }
      if ( mConnector )
      {
        mConnector->setSelected( false );
        mConnector = 0;
      }
      showOptions( Select );

      QRectF r( p.x() - 5, p.y() - 5, 10, 10 );
      QList<QGraphicsItem *> l = mCanvas->items( r );

      // Connector precedence (reverse order - connectors are under objects)
      QList<QGraphicsItem *>::const_iterator it = l.constEnd();
      while ( it != l.constBegin() )
      {
        --it;

        if ( typeid( **it ) == typeid( QgsGrassMapcalcConnector ) )
        {
          mConnector = dynamic_cast<QgsGrassMapcalcConnector *>( *it );
          mConnector->setSelected( true );
          mConnector->selectEnd( p );
          mStartMoveConnectorPoints[0] = mConnector->point( 0 );
          mStartMoveConnectorPoints[1] = mConnector->point( 1 );

          break;
        }
        else if ( typeid( **it ) == typeid( QgsGrassMapcalcObject ) )
        {
          mObject = dynamic_cast<QgsGrassMapcalcObject *>( *it );
          mObject->setSelected( true );

          int tool = Select;
          if ( mObject->type() == QgsGrassMapcalcObject::Map )
            tool = AddMap;
          else if ( mObject->type() == QgsGrassMapcalcObject::Constant )
            tool = AddConstant;
          else if ( mObject->type() == QgsGrassMapcalcObject::Function )
            tool = AddFunction;

          showOptions( tool );

          break;
        }
      }

      if (( mConnector && mConnector->selectedEnd() == -1 ) || mObject )
      {
        mView->setCursor( QCursor( Qt::SizeAllCursor ) );
      }
      else if ( mConnector )
      {
        mView->setCursor( QCursor( Qt::CrossCursor ) );
      }

      if ( mConnector ||
           ( mObject && mObject->type() != QgsGrassMapcalcObject::Output ) )
      {
        mActionDeleteItem->setEnabled( true );
      }
      else
      {
        mActionDeleteItem->setEnabled( false );
      }

      setOption();
      break;
  }
  mCanvas->update();
  mLastPoint = p;
  mStartMovePoint = p;
}

void QgsGrassMapcalc::mouseMoveEvent( QMouseEvent* e )
{
  // QgsDebugMsg(QString("mTool = %1 mToolStep = %2").arg(mTool).arg(mToolStep));

  QPoint p = mView->mapToScene( e->pos() ).toPoint();
  limit( &p );

  switch ( mTool )
  {
    case AddMap:
    case AddConstant:
    case AddFunction:
      mObject->setCenter( p.x(), p.y() );
      break;

    case AddConnector:
      if ( mToolStep == 1 )
      {
        mConnector->setPoint( 1, p );
        mConnector->setSocket( 1 );   // disconnect
        mConnector->tryConnectEnd( 1 );  // try to connect
      }
      break;

    case Select:
      if ( mObject )
      {
        int dx = p.x() - mLastPoint.x();
        int dy = p.y() - mLastPoint.y();
        QPoint c = mObject->center();
        mObject->setCenter( c.x() + dx, c.y() + dy );
      }
      if ( mConnector )
      {
        int end = mConnector->selectedEnd();
        int dx = p.x() - mStartMovePoint.x();
        int dy = p.y() - mStartMovePoint.y();

        if ( end == -1 )
        {
          for ( int i = 0; i < 2; i++ )
          {
            //QPoint pe = mConnector->point( i );
            mConnector->setSocket( i );   // disconnect
            mConnector->setPoint( i, QPoint(
                                    mStartMoveConnectorPoints[i].x() + dx,
                                    mStartMoveConnectorPoints[i].y() + dy ) );
            mConnector->tryConnectEnd( i );  // try to connect
          }
        }
        else
        {
          mConnector->setSocket( end );   // disconnect
          mConnector->setPoint( end, QPoint( p.x(), p.y() ) );
          mConnector->tryConnectEnd( end );  // try to connect
        }
      }
      break;
  }

  mCanvas->update();
  mLastPoint = p;
}

void QgsGrassMapcalc::mouseReleaseEvent( QMouseEvent* e )
{
  QgsDebugMsg( QString( "mTool = %1 mToolStep = %2" ).arg( mTool ).arg( mToolStep ) );

  QPoint p = mView->mapToScene( e->pos() ).toPoint();
  limit( &p );

  switch ( mTool )
  {
    case AddConnector:
      if ( mToolStep == 1 )
      {
        QPoint p0 = mConnector->point( 0 );
        double d = sqrt( pow(( double )( p.x() - p0.x() ), 2.0 )
                         + pow(( double )( p.y() - p0.y() ), 2.0 ) );
        QgsDebugMsg( QString( "d = %1" ).arg( d ) );
        if ( d <  5 ) // filter 'single' clicks
        {
          mConnector->setSocket( 0 );   // disconnect
          delete mConnector;
        }
        mConnector = 0;
        setTool( mTool );  // restart
      }
      break;

    case Select:
      mView->setCursor( QCursor( Qt::ArrowCursor ) );
      break;
  }
  autoGrow();
  mCanvas->update();
  mLastPoint = p;
}

QStringList QgsGrassMapcalc::arguments()
{
  QString cmd = "";
  // Attention with quotes and spaces!
  //cmd.append("\"");

  cmd.append( mOutputLineEdit->text() );
  cmd.append( " = " );
  cmd.append( mOutput->expression() );
  //cmd.append("\"");

  return QStringList( cmd );
}

QStringList QgsGrassMapcalc::checkOutput()
{
  QgsDebugMsg( "entered." );
  QStringList list;

  QString value = mOutputLineEdit->text().trimmed();

  if ( value.length() == 0 )
    return QStringList();

  QString path = QgsGrass::getDefaultGisdbase() + "/"
                 + QgsGrass::getDefaultLocation() + "/"
                 + QgsGrass::getDefaultMapset()
                 + "/cell/" + value;

  QFileInfo fi( path );

  if ( fi.exists() )
  {
    return ( QStringList( value ) );
  }

  return QStringList();
}

QStringList QgsGrassMapcalc::checkRegion()
{
  QgsDebugMsg( "entered." );
  QStringList list;

  QList<QGraphicsItem *> l = mCanvas->items();

  struct Cell_head currentWindow;
  if ( !QgsGrass::region( QgsGrass::getDefaultGisdbase(),
                          QgsGrass::getDefaultLocation(),
                          QgsGrass::getDefaultMapset(), &currentWindow ) )
  {
    QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot get current region" ) );
    return list;
  }

  QList<QGraphicsItem *>::const_iterator it = l.constEnd();
  while ( it != l.constBegin() )
  {
    --it;

    QgsGrassMapcalcObject *obj = dynamic_cast<QgsGrassMapcalcObject *>( *it );
    if ( !obj )
      continue;

    if ( obj->type() != QgsGrassMapcalcObject::Map )
      continue;

    struct Cell_head window;

    QStringList mm = obj->value().split( "@" );
    if ( mm.size() < 1 )
      continue;

    QString map = mm.at( 0 );
    QString mapset = QgsGrass::getDefaultMapset();
    if ( mm.size() > 1 )
      mapset = mm.at( 1 );

    if ( !QgsGrass::mapRegion( QgsGrass::Raster,
                               QgsGrass::getDefaultGisdbase(),
                               QgsGrass::getDefaultLocation(), mapset, map,
                               &window ) )
    {
      QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot check region of map %1" ).arg( obj->value() ) );
      continue;
    }

    if ( G_window_overlap( &currentWindow,
                           window.north, window.south, window.east, window.west ) == 0 )
    {
      list.append( obj->value() );
    }
  }
  return list;
}

bool QgsGrassMapcalc::inputRegion( struct Cell_head *window, bool all )
{
  Q_UNUSED( all );
  QgsDebugMsg( "entered." );

  if ( !QgsGrass::region( QgsGrass::getDefaultGisdbase(),
                          QgsGrass::getDefaultLocation(),
                          QgsGrass::getDefaultMapset(), window ) )
  {
    QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot get current region" ) );
    return false;
  }

  QList<QGraphicsItem *> l = mCanvas->items();

  int count = 0;
  QList<QGraphicsItem *>::const_iterator it = l.constEnd();
  while ( it != l.constBegin() )
  {
    --it;

    QgsGrassMapcalcObject *obj = dynamic_cast<QgsGrassMapcalcObject *>( *it );
    if ( !obj )
      continue;

    if ( obj->type() != QgsGrassMapcalcObject::Map )
      continue;

    struct Cell_head mapWindow;

    QStringList mm = obj->value().split( "@" );
    if ( mm.size() < 1 )
      continue;

    QString map = mm.at( 0 );
    QString mapset = QgsGrass::getDefaultMapset();
    if ( mm.size() > 1 )
      mapset = mm.at( 1 );

    if ( !QgsGrass::mapRegion( QgsGrass::Raster,
                               QgsGrass::getDefaultGisdbase(),
                               QgsGrass::getDefaultLocation(), mapset, map,
                               &mapWindow ) )
    {
      QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot get region of map %1" ).arg( obj->value() ) );
      return false;
    }

    // TODO: best way to set resolution ?
    if ( count == 0 )
    {
      QgsGrass::copyRegionExtent( &mapWindow, window );
      QgsGrass::copyRegionResolution( &mapWindow, window );
    }
    else
    {
      QgsGrass::extendRegion( &mapWindow, window );
    }
    count++;
  }

  return true;
}

QStringList QgsGrassMapcalc::output( int type )
{
  QgsDebugMsg( "entered." );
  QStringList list;
  if ( type == QgsGrassModuleOption::Raster )
  {
    list.append( mOutputLineEdit->text() );
  }
  return list;
}

QgsGrassMapcalc::~QgsGrassMapcalc()
{
}

void QgsGrassMapcalc::showOptions( int tool )
{
  QgsDebugMsg( QString( "tool = %1" ).arg( tool ) );

  // Hide widgets
  mMapComboBox->hide();
  mConstantLineEdit->hide();
  mFunctionComboBox->hide();

  switch ( tool )
  {
    case AddMap:
      mMapComboBox->show();
      break;

    case AddConstant:
      mConstantLineEdit->show();
      break;

    case AddFunction:
      mFunctionComboBox->show();
      break;
  }
}

void QgsGrassMapcalc::setOption()
{
  QgsDebugMsg( "entered." );

  if ( mTool != Select )
    return;
  if ( !mObject )
    return;

  switch ( mObject->type() )
  {
    case QgsGrassMapcalcObject::Map :
    {
      bool found = false;
      for ( unsigned int i = 0 ; i < mMaps.size(); i++ )
      {
        if ( mMapComboBox->itemText( i ) == mObject->label()
             && mMaps[i] == mObject->value() )
        {
          mMapComboBox->setCurrentIndex( i ) ;
          found = true;
        }
      }
      if ( !found )
      {
        mMaps.push_back( mObject->value() );
        mMapComboBox->addItem( mObject->label() );
        mMapComboBox->setCurrentIndex( mMapComboBox->count() - 1 );
      }
      break;
    }

    case QgsGrassMapcalcObject::Constant :
      mConstantLineEdit->setText( mObject->value() );
      break;

    case QgsGrassMapcalcObject::Function :
      for ( unsigned int i = 0; i < mFunctions.size(); i++ )
      {
        if ( mFunctions[i].name() != mObject->function().name() )
          continue;
        if ( mFunctions[i].inputCount() != mObject->function().inputCount() )
          continue;

        mFunctionComboBox->setCurrentIndex( i );
        break;
      }
      // TODO: if not found

      break;
  }

}

void QgsGrassMapcalc::setTool( int tool )
{
  // Clear old
  if ( mTool == Select )
  {
    if ( mObject )
      mObject->setSelected( false );
    if ( mConnector )
      mConnector->setSelected( false );
  }
  else
  {
    if ( mObject )
      delete mObject;
    if ( mConnector )
      delete mConnector;
    mCanvas->update();
  }
  mObject = 0;
  mConnector = 0;

  mTool = tool;
  mToolStep = 0;

  mView->viewport()->setMouseTracking( false );

  switch ( mTool )
  {
    case AddMap:
      mObject = new QgsGrassMapcalcObject( QgsGrassMapcalcObject::Map );
      mObject->setId( nextId() );

      // TODO check if there are maps
      mObject->setValue( mMaps[mMapComboBox->currentIndex()],
                         mMapComboBox->currentText() );

      mObject->setCenter( mLastPoint.x(), mLastPoint.y() );
      mCanvas->addItem( mObject );
      mObject->QGraphicsRectItem::show();
      mActionAddMap->setChecked( true );
      mView->viewport()->setMouseTracking( true );
      mView->setCursor( QCursor( Qt::SizeAllCursor ) );
      break;

    case AddConstant:
      mObject = new QgsGrassMapcalcObject( QgsGrassMapcalcObject::Constant );
      mObject->setId( nextId() );
      mObject->setValue( mConstantLineEdit->text() );
      mObject->setCenter( mLastPoint.x(), mLastPoint.y() );
      mCanvas->addItem( mObject );
      mObject->QGraphicsRectItem::show();
      mActionAddConstant->setChecked( true );
      mView->viewport()->setMouseTracking( true );
      mView->setCursor( QCursor( Qt::SizeAllCursor ) );
      break;

    case AddFunction:
      mObject = new QgsGrassMapcalcObject( QgsGrassMapcalcObject::Function );
      mObject->setId( nextId() );
      //mObject->setValue ( mFunctionComboBox->currentText() );
      mObject->setFunction( mFunctions[ mFunctionComboBox->currentIndex()] );
      mObject->setCenter( mLastPoint.x(), mLastPoint.y() );
      mCanvas->addItem( mObject );
      mObject->QGraphicsRectItem::show();
      mActionAddFunction->setChecked( true );
      mView->viewport()->setMouseTracking( true );
      mView->setCursor( QCursor( Qt::SizeAllCursor ) );
      break;

    case AddConnector:
      mConnector = new QgsGrassMapcalcConnector( mCanvas );
      mConnector->setId( nextId() );
      mCanvas->addItem( mConnector );
      mConnector->QGraphicsLineItem::show();
      mActionAddConnection->setChecked( true );
      mView->setCursor( QCursor( Qt::CrossCursor ) );
      break;
  }

  showOptions( mTool );
  setToolActionsOff();
  mActionDeleteItem->setEnabled( false );
  mCanvas->update();
}

void QgsGrassMapcalc::addMap()
{
  updateMaps();
  if ( mMaps.size() == 0 )
  {
    QMessageBox::warning( 0, tr( "Warning" ), tr( "No GRASS raster maps currently in QGIS" ) );

    setTool( AddConstant );
    return;
  }

  setTool( AddMap );
}

void QgsGrassMapcalc::addConstant()
{
  setTool( AddConstant );
}

void QgsGrassMapcalc::addFunction()
{
  setTool( AddFunction );
}

void QgsGrassMapcalc::addConnection()
{
  setTool( AddConnector );
}

void QgsGrassMapcalc::selectItem()
{
  setTool( Select );
  mActionSelectItem->setChecked( true );
  mView->setCursor( QCursor( Qt::ArrowCursor ) );
}

void QgsGrassMapcalc::deleteItem()
{
  if ( mConnector )
  {
    delete mConnector;
    mConnector = 0;
  }
  if ( mObject && mObject->type() != QgsGrassMapcalcObject::Output )
  {
    delete mObject;
    mObject = 0;
  }
  mActionDeleteItem->setEnabled( false );
  mCanvas->update();
}

void QgsGrassMapcalc::keyPressEvent( QKeyEvent * e )
{
  if ( e->key() == Qt::Key_Delete )
  {
    deleteItem();
  }
}

void QgsGrassMapcalc::setToolActionsOff()
{
  mActionAddMap->setChecked( false );
  mActionAddConstant->setChecked( false );
  mActionAddFunction->setChecked( false );
  mActionAddConnection->setChecked( false );
  mActionSelectItem->setChecked( false );
  mActionDeleteItem->setChecked( false );
}

void QgsGrassMapcalc::updateMaps()
{
  // TODO: this copy and paste from QgsGrassModuleInput, do it better
  QgsDebugMsg( "entered." );
  QString current = mMapComboBox->currentText();
  mMapComboBox->clear();
  mMaps.clear();

  QgsMapCanvas *canvas = mIface->mapCanvas();

  int nlayers = canvas->layerCount();
  QgsDebugMsg( QString( "nlayers = %1" ).arg( nlayers ) );
  for ( int i = 0; i < nlayers; i++ )
  {
    QgsMapLayer *layer = canvas->layer( i );

    if ( layer->type() != QgsMapLayer::RasterLayer )
      continue;

    // Check if it is GRASS raster
    QString source = QDir::cleanPath( layer->source() );

    // Note: QDir::cleanPath is using '/' also on Windows
    //QChar sep = QDir::separator();
    QChar sep = '/';

    if ( source.contains( "cellhd" ) == 0 )
      continue;

    // Most probably GRASS layer, check GISBASE and LOCATION
    QStringList split = source.split( sep, QString::SkipEmptyParts );

    if ( split.size() < 4 )
      continue;

    QString map = split.last();
    split.pop_back(); // map
    if ( split.last() != "cellhd" )
      continue;
    split.pop_back(); // cellhd

    QString mapset = split.last();
    split.pop_back(); // mapset

    //QDir locDir ( sep + split.join ( QString(sep) ) ) ;
    //QString loc = locDir.canonicalPath();

    QString loc =  source.remove( QRegExp( "/[^/]+/[^/]+/[^/]+$" ) );
    loc = QDir( loc ).canonicalPath();

    QDir curlocDir( QgsGrass::getDefaultGisdbase() + sep + QgsGrass::getDefaultLocation() );
    QString curloc = curlocDir.canonicalPath();

    if ( loc != curloc )
      continue;

#if 0
    if ( mUpdate && mapset != QgsGrass::getDefaultMapset() )
      continue;
#endif

    mMapComboBox->addItem( layer->name() );
    //if ( layer->name() == current )
    //  mMapComboBox->setItemText( mMapComboBox->currentIndex(), current );
    mMaps.push_back( map + "@" + mapset );
  }
}

void QgsGrassMapcalc::mapChanged()
{
  QgsDebugMsg( "entered." );

  if (( mTool != AddMap && mTool != Select )  || !mObject )
    return;
  if ( mObject->type() != QgsGrassMapcalcObject::Map )
    return;

  mObject->setValue( mMaps[mMapComboBox->currentIndex()],
                     mMapComboBox->currentText() );
  mCanvas->update();
}

void QgsGrassMapcalc::constantChanged()
{
  QgsDebugMsg( "entered." );

  if (( mTool != AddConstant && mTool != Select ) || !mObject )
    return;
  if ( mObject->type() != QgsGrassMapcalcObject::Constant )
    return;

  mObject->setValue( mConstantLineEdit->text() );
  mCanvas->update();
}

void QgsGrassMapcalc::functionChanged()
{
  QgsDebugMsg( "entered." );

  if (( mTool != AddFunction && mTool != Select ) || !mObject )
    return;
  if ( mObject->type() != QgsGrassMapcalcObject::Function )
    return;

  mObject->setFunction( mFunctions[ mFunctionComboBox->currentIndex()] );
  mCanvas->update();
}

void QgsGrassMapcalc::limit( QPoint *point )
{
  if ( point->x() < 0 )
    point->setX( 0 );
  if ( point->y() < 0 )
    point->setY( 0 );
  if ( point->x() > mCanvas->width() )
    point->setX( mCanvas->width() );
  if ( point->y() > mCanvas->height() )
    point->setY( mCanvas->height() );
}

void QgsGrassMapcalc::resizeCanvas( int width, int height )
{
  mCanvas->setSceneRect( 0, 0, width, height );
  mPaper->setRect( 0, 0, width, height );
  mCanvas->update();
}

void QgsGrassMapcalc::growCanvas( int left, int right, int top, int bottom )
{
  QgsDebugMsg( "entered." );
  QgsDebugMsg( QString( "left = %1 right = %2 top = %3 bottom = %4" ).arg( left ).arg( right ).arg( top ).arg( bottom ) );

  int width = mCanvas->width() + left + right;
  int height = mCanvas->height() + top + bottom;
  resizeCanvas( width, height );

  QList<QGraphicsItem *> l = mCanvas->items();

  QList<QGraphicsItem *>::const_iterator it = l.constEnd();
  while ( it != l.constBegin() )
  {
    --it;

    if ( typeid( **it ) == typeid( QgsGrassMapcalcObject ) )
    {
      QgsGrassMapcalcObject *obj = dynamic_cast<QgsGrassMapcalcObject *>( *it );

      QPoint p = obj->center();
      obj->setCenter( p.x() + left, p.y() + top );
    }
    else if ( typeid( **it ) == typeid( QgsGrassMapcalcConnector ) )
    {
      QgsGrassMapcalcConnector *con = dynamic_cast<QgsGrassMapcalcConnector *>( *it );

      for ( int i = 0; i < 2; i++ )
      {
        QPoint p = con->point( i );
        p.setX( p.x() + left );
        p.setY( p.y() + top );
        con->setPoint( i,  p );
      }
    }
  }

  mCanvas->update();
}

void QgsGrassMapcalc::autoGrow()
{
  QgsDebugMsg( "entered." );

  int thresh = 15;

  int left = 0;
  int right = mCanvas->width();
  int top = 0;
  int bottom = mCanvas->height();
  QgsDebugMsg( QString( "left = %1 right = %2 top = %3 bottom = %4" ).arg( left ).arg( right ).arg( top ).arg( bottom ) );

  QList<QGraphicsItem *> l = mCanvas->items();

  QList<QGraphicsItem *>::const_iterator it = l.constEnd();
  while ( it != l.constBegin() )
  {
    --it;
    if ( !dynamic_cast<QgsGrassMapcalcItem *>( *it ) )
      continue;

    // Exclude current
    if (( mTool != Select ) && ( *it == mObject || *it == mConnector ) )
      continue;

    QRectF r = ( *it )->boundingRect().translated(( *it )->pos() );

    QgsDebugMsg( QString( "r.left = %1 r.right = %2 r.top = %3 bottom = %4" ).arg( r.left() ).arg( r.right() ).arg( r.top() ).arg( r.bottom() ) );

    if ( r.left() - thresh < left )
      left   = r.left() - thresh;
    if ( r.right() + thresh > right )
      right  = r.right() + thresh;
    if ( r.top() - thresh < top )
      top    = r.top() - thresh;
    if ( r.bottom() + thresh > bottom )
      bottom = r.bottom() + thresh;

    QgsDebugMsg( QString( "left = %1 right = %2 top = %3 bottom = %4" ).arg( left ).arg( right ).arg( top ).arg( bottom ) );
  }
  left = -left;
  right = right - mCanvas->width();
  top = -top;
  bottom = bottom - mCanvas->height();

  growCanvas( left, right, top, bottom );
}

void QgsGrassMapcalc::saveAs()
{
  QgsDebugMsg( "entered." );

  // Check/create 'mapcalc' directory in current mapset
  QString ms = QgsGrass::getDefaultGisdbase() + "/"
               + QgsGrass::getDefaultLocation() + "/"
               + QgsGrass::getDefaultMapset();

  QString mc = ms + "/mapcalc";

  if ( !QFile::exists( mc ) )
  {
    QDir d( ms );

    if ( !d.mkdir( "mapcalc" ) )
    {
      QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot create 'mapcalc' directory in current mapset." ) );
      return;
    }
  }

  // Ask for file name
  QString name;
  while ( 1 )
  {
    bool ok;
    name = QInputDialog::getText( this, tr( "New mapcalc" ),
                                  tr( "Enter new mapcalc name:" ), QLineEdit::Normal, mFileName, &ok );
    if ( !ok )
      return;
    name = name.trimmed();

    if ( name.isEmpty() )
    {
      QMessageBox::warning( 0, tr( "Warning" ), tr( "Enter vector name" ) );
      continue;
    }

    // check if exists
    if ( QFile::exists( mc + "/" + name ) )
    {
      QMessageBox::StandardButton ret = QMessageBox::question( 0, tr( "Warning" ),
                                        tr( "The file already exists. Overwrite?" ),
                                        QMessageBox::Ok | QMessageBox::Cancel );

      if ( ret == QMessageBox::Cancel )
        continue;
    }
    break;
  }

  mFileName = name;
  mActionSave->setEnabled( true );
  save();
}

void QgsGrassMapcalc::save()
{
  QgsDebugMsg( "entered." );
  if ( mFileName.isEmpty() ) // Should not happen
  {
    QMessageBox::warning( this, tr( "Save mapcalc" ), tr( "File name empty" ) );
    return;
  }
  // TODO!!!: 'escape' < > & ...

  // Erase objects of Add* tools
  int tool = mTool;
  setTool( Select );

  // Open file
  QString path = QgsGrass::getDefaultGisdbase() + "/"
                 + QgsGrass::getDefaultLocation() + "/"
                 + QgsGrass::getDefaultMapset()
                 + "/mapcalc/" + mFileName;

  QFile out( path );
  if ( !out.open( QIODevice::WriteOnly ) )
  {
    QMessageBox::warning( this, tr( "Save mapcalc" ),
                          tr( "Cannot open mapcalc file" ) );
    return;
  }

  QTextStream stream( &out );

  stream << "<mapcalc>\n";
  stream << "  <canvas width=\"" + QString::number( mCanvas->width() )
  + "\" height=\"" + QString::number( mCanvas->height() )
  + "\"/>\n";

  QList<QGraphicsItem *> l = mCanvas->items();

  QList<QGraphicsItem *>::const_iterator it = l.constEnd();
  while ( it != l.constBegin() )
  {
    --it;

    if ( typeid( **it ) == typeid( QgsGrassMapcalcObject ) )
    {
      QgsGrassMapcalcObject *obj = dynamic_cast<QgsGrassMapcalcObject *>( *it );

      QString type;
      if ( obj->type() == QgsGrassMapcalcObject::Map )
      {
        type = "map";
      }
      else if ( obj->type() == QgsGrassMapcalcObject::Constant )
      {
        type = "constant";
      }
      else if ( obj->type() == QgsGrassMapcalcObject::Function )
      {
        if ( obj->function().type() == QgsGrassMapcalcFunction::Operator )
          type = "operator";
        else
          type = "function";
      }
      else if ( obj->type() == QgsGrassMapcalcObject::Output )
      {
        type = "output";
      }

      QString val = obj->value();
      if ( obj->type() == QgsGrassMapcalcObject::Function )
      {
        val.replace( "&", "&amp;" );
        val.replace( "<", "&lt;" );
        val.replace( ">", "&gt;" );
      }

      stream << "  <object id=\"" + QString::number( obj->id() )
      + "\" x=\"" + QString::number( obj->center().x() )
      + "\" y=\"" + QString::number( obj->center().y() )
      + "\" type=\"" + type
      + "\" value=\"" + val + "\"";

      if ( obj->type() == QgsGrassMapcalcObject::Function )
      {
        stream << "  inputCount=\""
        + QString::number( obj->function().inputCount() ) + "\"";
      }
      if ( obj->type() == QgsGrassMapcalcObject::Map )
      {
        stream << "  label=\"" + obj->label() + "\"";
      }
      stream <<  "/>\n";
    }
    else if ( typeid( **it ) == typeid( QgsGrassMapcalcConnector ) )
    {
      QgsGrassMapcalcConnector *con = dynamic_cast<QgsGrassMapcalcConnector *>( *it );

      stream << "  <connector id=\"" + QString::number( con->id() )
      + "\">\n";

      for ( int i = 0; i < 2; i++ )
      {
        stream << "    <end x=\"" + QString::number( con->point( i ).x() )
        + "\" y=\"" + QString::number( con->point( i ).y() )
        + "\"";
        if ( con->object( i ) )
        {
          stream << " object=\""
          + QString::number( con->object( i )->id() )
          + "\" socketType=\"";

          if ( con->socketDirection( i ) == QgsGrassMapcalcObject::In )
          {
            stream << "in";
          }
          else
          {
            stream << "out";
          }

          stream << "\" socket=\""
          + QString::number( con->socket( i ) )
          + "\"";
        }
        stream << "/>\n";

      }
      stream << "  </connector>\n";
    }
  }

  stream << "</mapcalc>";
  out.close();
  setTool( tool );
}

void QgsGrassMapcalc::load()
{
  QgsDebugMsg( "entered." );

  QgsGrassSelect *sel = new QgsGrassSelect( QgsGrassSelect::MAPCALC );
  if ( sel->exec() == QDialog::Rejected )
    return;

  // Open file
  QString path = sel->gisdbase + "/" + sel->location + "/"
                 + sel->mapset + "/mapcalc/" + sel->map;

  QFile file( path );

  if ( !file.exists() ) // should not happen
  {
    QMessageBox::warning( 0, tr( "Warning" ), tr( "The mapcalc schema (%1) not found." ).arg( path ) );
    return;
  }

  if ( ! file.open( QIODevice::ReadOnly ) )
  {
    QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot open mapcalc schema (%1)" ).arg( path ) );

    return;
  }

  QDomDocument doc( "mapcalc" );
  QString err;
  int line, column;
  int parsed = doc.setContent( &file,  &err, &line, &column );
  file.close();
  if ( !parsed )
  {
    QString errmsg = tr( "Cannot read mapcalc schema (%1):" ).arg( path )
                     + tr( "\n%1\nat line %2 column %3" ).arg( err ).arg( line ).arg( column );
    QMessageBox::warning( 0, tr( "Warning" ), errmsg );
    return;
  }

  clear();
  QDomElement docElem = doc.documentElement();

  // Set canvas
  QDomNodeList canvasNodes = docElem.elementsByTagName( "canvas" );
  QDomElement canvasElement = canvasNodes.item( 0 ).toElement();
  int width = canvasElement.attribute( "width", "300" ).toInt();
  int height = canvasElement.attribute( "height", "200" ).toInt();
  resizeCanvas( width, height );

  // Add objects
  std::vector<QgsGrassMapcalcObject *> objects;
  QDomNodeList objectNodes = docElem.elementsByTagName( "object" );
  QgsDebugMsg( QString( "objectNodes.count() = %1" ).arg( objectNodes.count() ) );
  for ( int n = 0; n < objectNodes.count(); n++ )
  {
    QDomNode node = objectNodes.item( n );
    QDomElement e = node.toElement();
    if ( e.isNull() )
      continue;

    QgsDebugMsg( QString( "id = %1" ).arg( e.attribute( "id", "?" ).toLocal8Bit().constData() ) );
    unsigned int id = e.attribute( "id", "0" ).toInt();
    int x = e.attribute( "x", "0" ).toInt();
    int y = e.attribute( "y", "0" ).toInt();
    QString typeName = e.attribute( "type", "constant" );
    QString value = e.attribute( "value", "???" );

    if ( id >= mNextId )
      mNextId = id + 1;
    if ( id >= objects.size() )
    {
      objects.resize( id + 1 );
    }

    int type = -1;

    if ( typeName == "map" )
      type = QgsGrassMapcalcObject::Map;
    else if ( typeName == "constant" )
      type = QgsGrassMapcalcObject::Constant;
    else if ( typeName == "operator" )
      type = QgsGrassMapcalcObject::Function;
    else if ( typeName == "function" )
      type = QgsGrassMapcalcObject::Function;
    else if ( typeName == "output" )
      type = QgsGrassMapcalcObject::Output;

    if ( type == -1 )
      continue;

    QgsGrassMapcalcObject *obj = new QgsGrassMapcalcObject( type );
    objects[id] = obj;

    obj->setId( id );
    obj->setValue( value );
    obj->setCenter( x, y );
    mCanvas->addItem( obj );
    obj->show();

    switch ( type )
    {
      case QgsGrassMapcalcObject::Map:
      {
        QString label = QApplication::translate( "grasslabel", e.attribute( "label", "???" ).toUtf8() );
        obj->setValue( value, label );
        break;
      }

      case QgsGrassMapcalcObject::Output :
        obj->setValue( tr( "Output" ) );
        mOutput = obj;
        break;

      case QgsGrassMapcalcObject::Function :
        int inputCount = e.attribute( "inputCount", "1" ).toInt();
        // Find function
        int fn = -1;
        for ( unsigned int i = 0; i < mFunctions.size(); i++ )
        {
          if ( mFunctions[i].name() != value )
            continue;
          if ( mFunctions[i].inputCount() != inputCount )
            continue;
          fn = i;
        }

        if ( fn >= 0 )
        {
          obj->setFunction( mFunctions[fn] );
        }
        // TODO: if not found

        break;
    }
  }

  // Add connectors
  QDomNodeList connectorNodes = docElem.elementsByTagName( "connector" );
  for ( int n = 0; n < connectorNodes.count(); n++ )
  {
    QDomNode node = connectorNodes.item( n );
    QDomElement e = node.toElement();
    if ( e.isNull() )
      continue;

    QgsDebugMsg( QString( "id = %1" ).arg( e.attribute( "id", "?" ).toLocal8Bit().constData() ) );
    unsigned int id = e.attribute( "id", "0" ).toInt();
    if ( id >= mNextId )
      mNextId = id + 1;

    QgsGrassMapcalcConnector *con = new QgsGrassMapcalcConnector( mCanvas );

    con->setId( id );
    mCanvas->addItem( con );
    con->show();

    QDomNodeList endNodes = e.elementsByTagName( "end" );
    QgsDebugMsg( QString( "endNodes.count = %1" ).arg( endNodes.count() ) );
    for ( int n2 = 0; n2 < endNodes.count() && n2 < 2; n2++ )
    {
      QDomNode node2 = endNodes.item( n2 );
      QDomElement e2 = node2.toElement();
      if ( e2.isNull() )
        continue;

      int x = e2.attribute( "x", "0" ).toInt();
      int y = e2.attribute( "y", "0" ).toInt();
      con->setPoint( n2, QPoint( x, y ) );

      QgsDebugMsg( QString( "x = %1 y = %2" ).arg( x ).arg( y ) );

      int objId = e2.attribute( "object", "-1" ).toInt();
      QgsDebugMsg( QString( "objId = %1" ).arg( objId ) );
      if ( objId < 0 )
        continue; // not connected

      if ( static_cast<uint>( objId ) < objects.size() && objects[objId] )
      {
        QString socketTypeName = e2.attribute( "socketType", "out" );
        int socketType;
        if ( socketTypeName == "in" )
          socketType = QgsGrassMapcalcObject::In;
        else
          socketType = QgsGrassMapcalcObject::Out;

        int socket = e2.attribute( "socket", "0" ).toInt();

        QgsDebugMsg( QString( "end =  %1 objId = %2 socketType = %3 socket = %4" ).arg( n2 ).arg( objId ).arg( socketType ).arg( socket ) );

        con->setSocket( n2, objects[objId], socketType, socket );

        objects[objId]->setConnector( socketType, socket, con, n2 );
      }
    }

  }

  mFileName = sel->map;
  mActionSave->setEnabled( true );
  mCanvas->update();
}

void QgsGrassMapcalc::clear()
{
  QgsDebugMsg( "entered." );

  setTool( Select );

  QList<QGraphicsItem *> l = mCanvas->items();

  QList<QGraphicsItem *>::const_iterator it = l.constEnd();
  while ( it != l.constBegin() )
  {
    --it;
    if ( !dynamic_cast<QgsGrassMapcalcItem *>( *it ) )
      continue;

    delete *it;
  }
  mNextId = 0;
}

/******************** CANVAS ITEMS ******************************/
QgsGrassMapcalcItem::QgsGrassMapcalcItem(): mSelected( false )
{
  QgsDebugMsg( "entered." );
}

QgsGrassMapcalcItem::~QgsGrassMapcalcItem()
{
}

void QgsGrassMapcalcItem::setSelected( bool s )
{
  mSelected = s;
}

bool QgsGrassMapcalcItem::selected()
{
  return mSelected;
}

/**************************** OBJECT ************************/
QgsGrassMapcalcObject::QgsGrassMapcalcObject( int type )
    : QGraphicsRectItem( -1000, -1000, 50, 20, 0 ), QgsGrassMapcalcItem(),
    mType( type ), mCenter( -1000, -1000 ), mSelectionBoxSize( 5 ),
    mOutputConnector( 0 )
{
  QgsDebugMsg( "entered." );

  QGraphicsRectItem::setZValue( 20 );

  mInputCount = 0;
  mOutputCount = 1;

  if ( mType == Function )
    mInputCount = 2;

  if ( mType == Output )
  {
    mInputCount = 1;
    mOutputCount = 0;
  }

  mInputConnectors.resize( mInputCount );
  mInputConnectorsEnd.resize( mInputCount );
}

QgsGrassMapcalcObject::~QgsGrassMapcalcObject()
{
  QgsDebugMsg( "entered." );
  // Delete connections
  for ( int i = 0; i < mInputCount; i++ )
  {
    if ( mInputConnectors[i] )
    {
      QgsGrassMapcalcConnector *con = mInputConnectors[i];
      mInputConnectors[i]->setSocket( mInputConnectorsEnd[i] );
      con->repaint();
    }
  }
  if ( mOutputConnector )
  {
    QgsGrassMapcalcConnector *con = mOutputConnector;
    mOutputConnector->setSocket( mOutputConnectorEnd );
    con->repaint();
  }
  QgsDebugMsg( "exited." );
}

int QgsGrassMapcalcObject::type()
{
  return mType;
}

void QgsGrassMapcalcObject::paint( QPainter * painter,
                                   const QStyleOptionGraphicsItem * option, QWidget * widget )
{
  Q_UNUSED( option );
  Q_UNUSED( widget );
  //QGraphicsRectItem::paint(painter, option, widget);

  painter->setPen( QPen( QColor( 0, 0, 0 ) ) );
  painter->setBrush( QBrush( QColor( 255, 255, 255 ) ) );
  int xRound = ( int )( 100 * mRound / mRect.width() );
  int yRound = ( int )( 100 * mRound / mRect.height() );

  painter->drawRoundRect( mRect, xRound, yRound );

  // Input sockets
  for ( int i = 0; i < mInputCount; i++ )
  {
    if ( mInputConnectors[i] )
      painter->setBrush( QBrush( QColor( 180, 180, 180 ) ) );
    else
      painter->setBrush( QBrush( QColor( 255, 0, 0 ) ) );

    painter->drawEllipse( mInputPoints[i].x() - mSocketHalf,
                          mInputPoints[i].y() - mSocketHalf,
                          2*mSocketHalf + 1, 2*mSocketHalf + 1 );
  }

  // Output socket
  if ( mOutputCount > 0 )
  {
    if ( mOutputConnector )
      painter->setBrush( QBrush( QColor( 180, 180, 180 ) ) );
    else
      painter->setBrush( QBrush( QColor( 255, 0, 0 ) ) );

    painter->drawEllipse( mOutputPoint.x() - mSocketHalf,
                          mOutputPoint.y() - mSocketHalf,
                          2*mSocketHalf + 1, 2*mSocketHalf + 1 );
  }

  // Input labels
  if ( mType == Function && mInputTextWidth > 0 )
  {
    painter->setFont( mFont );
    QFontMetrics metrics( mFont );
    for ( int i = 0; i < mFunction.inputLabels().size(); i++ )
    {
      /*
      QStringList::Iterator it = mFunction.inputLabels().at(i);
      QString l = *it;
      */
      QString l = mFunction.inputLabels().at( i );


      int lx = mRect.x() + mSpace;
      int ly = mRect.y() + mSpace + i * ( mTextHeight + mSpace );
      QRect lr( lx, ly, metrics.width( l ), mTextHeight ) ;

      painter->drawText( lr, Qt::AlignCenter | Qt::TextSingleLine, l );
    }
  }

  // Label
  if ( mType != Function || mFunction.drawlabel() )
  {
    painter->drawText( mLabelRect, Qt::AlignCenter | Qt::TextSingleLine, mLabel );
  }

  // Selection
  if ( mSelected )
  {
    painter->setPen( QColor( 0, 255, 255 ) );
    painter->setBrush( QColor( 0, 255, 255 ) );

    int s = mSelectionBoxSize;

    painter->drawRect( mRect.x(), mRect.y(), s, s );
    painter->drawRect( mRect.x() + mRect.width() - s, mRect.y(), s, s );
    painter->drawRect( mRect.x() + mRect.width() - s,
                       mRect.y() + mRect.height() - s, s, s );
    painter->drawRect( mRect.x(), mRect.y() + mRect.height() - s, s, s );
  }
}

void QgsGrassMapcalcObject::setCenter( int x, int y )
{
  // QgsDebugMsg(QString("x = %1 y = %2").arg(x).arg(y));
  mCenter.setX( x );
  mCenter.setY( y );
  setPos( x - mRect.width() / 2 - mMargin, y - mRect.height() / 2 - mMargin );
}

void QgsGrassMapcalcObject::resetSize()
{
  QFontMetrics metrics( mFont );
  mTextHeight = metrics.height();

  mSocketHalf = ( int )( mFont.pointSize() / 3 + 1 );
  mSpace = ( int )( 1.0 * mFont.pointSize() );
  mRound = ( int )( 1.0 * mTextHeight );
  mMargin = 2 * mSocketHalf + 1;

  mInputTextWidth = 0;
  if ( mType == Function )
  {
    for ( int i = 0; i < mFunction.inputLabels().size(); i++ )
    {
      /*
      QStringList::Iterator it = mFunction.inputLabels().at(i);
      QString l = *it;
      */
      QString l = mFunction.inputLabels().at( i );
      int len = metrics.width( l );
      if ( len > mInputTextWidth )
        mInputTextWidth = len;
    }
  }

  int labelTextWidth = metrics.width( mLabel );
  if ( mType == Function && !mFunction.drawlabel() )
  {
    labelTextWidth = 0;
  }

  // Drawn rectangle
  int width = mSpace + mInputTextWidth + labelTextWidth;
  if ( mInputTextWidth > 0 && !mLabel.isEmpty() )
  {
    width += mSpace;
  }
  if ( labelTextWidth > 0 )
  {
    width += mSpace;
  }
  int height;
  if ( mInputCount > 0 )
  {
    height = mInputCount * ( mTextHeight + mSpace ) + mSpace;
  }
  else // Label only
  {
    height = 2 * mSpace + mTextHeight;
  }

  mRect.setX( mMargin );
  mRect.setY( mMargin );
  mRect.setSize( QSize( width, height ) );

  QGraphicsRectItem::setRect( 0, 0, width + 2*mMargin, height + 2*mMargin );

  // Label rectangle
  int lx = mRect.x() + mSpace;
  if ( mInputTextWidth > 0 )
  {
    lx += mInputTextWidth + mSpace;
  }
  int ly = mRect.y() + mSpace;
  if ( mInputCount > 1 )
  {
    ly += ( int )(( mInputCount * mTextHeight +
                    ( mInputCount - 1 ) * mSpace ) / 2 - mTextHeight / 2 );
  }
  mLabelRect.setX( lx );
  mLabelRect.setY( ly );
  mLabelRect.setSize( QSize( labelTextWidth, mTextHeight ) );

  // Input sockets
  mInputPoints.resize( mInputCount );

  for ( int i = 0; i < mInputCount; i++ )
  {
    mInputPoints[i] = QPoint( mRect.x() - mSocketHalf - 1,
                              ( int )( mRect.y() + ( i + 1 ) * ( mSpace + mTextHeight ) - mTextHeight / 2 ) );
  }

  // Output socket
  mOutputPoint.setX( mRect.right() + mSocketHalf + 1 );
  mOutputPoint.setY(( int )( mRect.y() + mRect.height() / 2 ) );

  // Update all connected connectors
  for ( int i = 0; i < mInputCount; i++ )
  {
    if ( mInputConnectors[i] )
    {
      mInputConnectors[i]->repaint();
    }
  }
  if ( mOutputConnector )
  {
    mOutputConnector->repaint();
  }

  QGraphicsRectItem::update();
}

void QgsGrassMapcalcObject::setValue( QString value, QString lab )
{
  mValue = value;
  if ( lab.isEmpty() )
  {
    mLabel = mValue;
  }
  else
  {
    mLabel = lab;
  }

  resetSize();
}

void QgsGrassMapcalcObject::setFunction( QgsGrassMapcalcFunction f )
{
  mValue = f.name();
  //mLabel = f.label();
  mLabel = f.name();
  mFunction = f;

  mInputCount = f.inputCount();
  mOutputCount = 1;

  mInputConnectors.resize( mInputCount );
  mInputConnectorsEnd.resize( mInputCount );

  resetSize();
}

void QgsGrassMapcalcObject::setSelected( bool s )
{
  mSelected = s;
  QGraphicsRectItem::update();
}

bool QgsGrassMapcalcObject::tryConnect( QgsGrassMapcalcConnector *connector,
                                        int end )
{
  QgsDebugMsg( "entered." );

  QPoint p = connector->point( end );

  // Input
  if ( !connector->connected( In ) )
  {
    for ( int i = 0; i < mInputCount; i++ )
    {
      if ( mInputConnectors[i] )
        continue; // used

      double d = sqrt( pow(( double )( mInputPoints[i].x() + pos().x() - p.x() ), 2.0 )
                       + pow(( double )( mInputPoints[i].y() + pos().y() - p.y() ), 2.0 ) );

      if ( d <= mSocketHalf )
      {
        QgsDebugMsg( QString( "Object: connector connected to input %1" ).arg( i ) );
        connector->setSocket( end, this, In, i );
        mInputConnectors[i] = connector;
        return true;
      }
    }
  }

  // Output
  if ( !connector->connected( Out ) && !mOutputConnector )
  {
    double d = sqrt( pow(( double )( mOutputPoint.x() + pos().x() - p.x() ), 2.0 )
                     + pow(( double )( mOutputPoint.y() + pos().y() - p.y() ), 2.0 ) );

    if ( d <= mSocketHalf )
    {
      QgsDebugMsg( "Object: connector connected to output " );
      connector->setSocket( end, this, Out );
      mOutputConnector = connector;
      return true;
    }
  }

  return false;
}

void QgsGrassMapcalcObject::setConnector( int direction, int socket,
    QgsGrassMapcalcConnector *connector, int end )
{
  QgsDebugMsg( "entered." );

  if ( direction == In )
  {
    mInputConnectors[socket] = connector;
    mInputConnectorsEnd[socket] = end;
  }
  else
  {
    mOutputConnector = connector;
    mOutputConnectorEnd = end;
  }

  QGraphicsRectItem::update();
}

QPoint QgsGrassMapcalcObject::socketPoint( int direction, int socket )
{
  // QgsDebugMsg("entered.");

  if ( direction == In )
  {
    return mInputPoints[socket] + pos().toPoint();
  }

  return mOutputPoint + pos().toPoint();
}

QString QgsGrassMapcalcObject::expression()
{
  QgsDebugMsg( "entered." );
  QgsDebugMsg( QString( "mType = %1" ).arg( mType ) );

  if ( mType == Map || mType == Constant )
  {
    return mValue;
  }

  if ( mType == Output )
  {
    if ( mInputConnectors[0] )
      //return mInputConnectors[0]->expression();
      return "(" + mInputConnectors[0]->expression() + ")";
    else
      return "null()";
  }

  // Functions and operators
  QString exp;

  if ( mFunction.type() == QgsGrassMapcalcFunction::Function )
    exp.append( mFunction.name() );

  exp.append( "(" );

  for ( int i = 0; i < mInputCount; i++ )
  {
    if ( i > 0 )
    {
      if ( mFunction.type() == QgsGrassMapcalcFunction::Function )
        exp.append( "," );
      else
        exp.append( mFunction.name() );
    }

    if ( mInputConnectors[i] )
      exp.append( mInputConnectors[i]->expression() );
    else
      exp.append( "null()" );

  }

  exp.append( ")" );

  QgsDebugMsg( QString( "exp = %1" ).arg( exp.toLocal8Bit().constData() ) );
  return exp;
}

/************************* CONNECTOR **********************************/
QgsGrassMapcalcConnector::QgsGrassMapcalcConnector( QGraphicsScene *canvas )
    : QGraphicsLineItem(), QgsGrassMapcalcItem()
{
  QgsDebugMsg( "entered." );

  canvas->addItem( this );

  QGraphicsLineItem::setZValue( 10 );

  mPoints.resize( 2 );
  mPoints[0] = QPoint( -1000, -1000 );
  mPoints[1] = QPoint( -1000, -1000 );

  mSocketObjects.resize( 2 );
  mSocketObjects[0] = 0;
  mSocketObjects[1] = 0;
  mSocketDir.resize( 2 );
  mSocket.resize( 2 );
}

QgsGrassMapcalcConnector::~QgsGrassMapcalcConnector()
{
  // Disconnect
  setSocket( 0 );
  setSocket( 1 );
}

void QgsGrassMapcalcConnector::paint( QPainter * painter,
                                      const QStyleOptionGraphicsItem * option, QWidget * widget )
{
  Q_UNUSED( option );
  Q_UNUSED( widget );
  for ( int i = 0; i < 2; i++ )
  {
    if ( mSocketObjects[i] )
    {
      mPoints[i] = mSocketObjects[i]->socketPoint( mSocketDir[i],
                   mSocket[i] );
    }
  }

  if ( !mSocketObjects[0] || !mSocketObjects[1] )
  {
    painter->setPen( QPen( QColor( 255, 0, 0 ) ) );
  }
  else
  {
    painter->setPen( QPen( QColor( 0, 0, 0 ) ) );
  }

  painter->drawLine( mPoints[0], mPoints[1] );

  if ( mSelected )
  {
    painter->setPen( QPen( QColor( 0, 255, 255 ), 0, Qt::DotLine ) );
  }
  painter->drawLine( mPoints[0], mPoints[1] );
}

void QgsGrassMapcalcConnector::repaint()
{
  setPoint( 0, point( 0 ) );
  //QCanvasLine::setX(QCanvasLine::x());
  QGraphicsLineItem::update();
}

void QgsGrassMapcalcConnector::setPoint( int index, QPoint point )
{
  // QgsDebugMsg(QString("index = %1").arg(index));

  mPoints[index] = point;
  QGraphicsLineItem::setLine( mPoints[0].x(), mPoints[0].y(),
                              mPoints[1].x(), mPoints[1].y() );
  QGraphicsLineItem::update();
}

QPoint QgsGrassMapcalcConnector::point( int index )
{
  return ( mPoints[index] );
}

void QgsGrassMapcalcConnector::setSelected( bool s )
{
  mSelected = s;
  QGraphicsLineItem::update();
}

void QgsGrassMapcalcConnector::selectEnd( QPoint point )
{
  QgsDebugMsg( "entered." );
  mSelectedEnd = -1;

  double d0 = sqrt( pow(( double )( point.x() - mPoints[0].x() ), 2.0 )
                    + pow(( double )( point.y() - mPoints[0].y() ), 2.0 ) );

  double d1 = sqrt( pow(( double )( point.x() - mPoints[1].x() ), 2.0 )
                    + pow(( double )( point.y() - mPoints[1].y() ), 2.0 ) );


  if ( d0 < 15 || d1 < 15 )
  {
    if ( d0 < d1 )
    {
      mSelectedEnd = 0;
    }
    else
    {
      mSelectedEnd = 1;
    }
  }
  QgsDebugMsg( QString( "mSelectedEnd = %1" ).arg( mSelectedEnd ) );
}

int QgsGrassMapcalcConnector::selectedEnd()
{
  return mSelectedEnd;
}

bool QgsGrassMapcalcConnector::tryConnectEnd( int end )
{
  QgsDebugMsg( "entered." );

  QList<QGraphicsItem *> l = scene()->items( mPoints[end] );
  QgsGrassMapcalcObject *object = 0;
  QList<QGraphicsItem *>::const_iterator it = l.constEnd();
  while ( it != l.constBegin() )
  {
    --it;

    if ( typeid( **it ) == typeid( QgsGrassMapcalcObject ) )
    {
      object = dynamic_cast<QgsGrassMapcalcObject *>( *it );
      break;
    }
  }

  // try to connect
  return object && object->tryConnect( this, end );
}

void QgsGrassMapcalcConnector::setSocket( int end,
    QgsGrassMapcalcObject *object, int direction, int socket )
{
  QgsDebugMsg( "entered." );

  // Remove old connection from object
  if ( mSocketObjects[end] )
  {
    mSocketObjects[end]->setConnector( mSocketDir[end],
                                       mSocket[end] );

    mSocketObjects[end] = 0;
  }

  // Create new connection
  mSocketObjects[end] = object;
  mSocketDir[end] = direction;
  mSocket[end] = socket;

  if ( !object )
    return; // disconnect only

  mSocketObjects[end]->setConnector( mSocketDir[end],
                                     mSocket[end], this, end );
}

bool QgsGrassMapcalcConnector::connected( int direction )
{
  for ( int i = 0; i < 2; i++ )
  {
    if ( mSocketObjects[i] )
    {
      if ( mSocketDir[i] == direction )
      {
        return true;
      }
    }
  }
  return false;
}

QString QgsGrassMapcalcConnector::expression()
{
  QgsDebugMsg( "entered." );
  for ( int i = 0; i < 2; i++ )
  {
    if ( !mSocketObjects[i] )
      continue;
    if ( mSocketDir[i] != QgsGrassMapcalcObject::Out )
      continue;
    return mSocketObjects[i]->expression();
  }

  return "null()";
}

QgsGrassMapcalcObject *QgsGrassMapcalcConnector::object( int end )
{
  return mSocketObjects[end];
}

/************************* FUNCTION *****************************/
QgsGrassMapcalcFunction::QgsGrassMapcalcFunction( int type, QString name,
    int count, QString description, QString label, QString labels,
    bool drawLabel ) :
    mName( name ), mType( type ), mInputCount( count ),
    mLabel( label ), mDescription( description ),
    mDrawLabel( drawLabel )
{
  if ( mLabel.isEmpty() )
    mLabel = mName;

  if ( !labels.isEmpty() )
  {
    mInputLabels = labels.split( ",", QString::SkipEmptyParts );
  }
}

/******************** CANVAS VIEW ******************************/

QgsGrassMapcalcView::QgsGrassMapcalcView( QgsGrassMapcalc * mapcalc,
    QWidget * parent, Qt::WFlags f ) :
    QGraphicsView( parent )
{
  Q_UNUSED( f );
  setAttribute( Qt::WA_StaticContents );
  mMapcalc = mapcalc;

  // TODO: nothing does work -> necessary to call setFocus ()
  setEnabled( true );
  setFocusPolicy( Qt::StrongFocus );
  setFocusProxy( 0 );
}

void QgsGrassMapcalcView::mousePressEvent( QMouseEvent * e )
{
  // TODO: find how to get focus without setFocus
  setFocus();
  mMapcalc->mousePressEvent( e );
}

void QgsGrassMapcalcView::mouseReleaseEvent( QMouseEvent * e )
{
  mMapcalc->mouseReleaseEvent( e );
}

void QgsGrassMapcalcView::mouseMoveEvent( QMouseEvent * e )
{
  mMapcalc->mouseMoveEvent( e );
}

void QgsGrassMapcalcView::keyPressEvent( QKeyEvent * e )
{
  mMapcalc->keyPressEvent( e );
}
