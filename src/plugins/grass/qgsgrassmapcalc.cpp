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
  QWidget *parent, Qt::WindowFlags f )
  : QMainWindow( iface->mainWindow(), Qt::Dialog )
  , QgsGrassMapcalcBase()
  , QgsGrassModuleOptions( tools, module, iface, false )
  , mTool( -1 )
{
  Q_UNUSED( parent )
  Q_UNUSED( f )
  QgsDebugMsgLevel( "QgsGrassMapcalc()", 4 );

  setupUi( this );
  connect( mConstantLineEdit, &QLineEdit::textChanged, this, &QgsGrassMapcalc::mConstantLineEdit_textChanged );
  connect( mFunctionComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::activated ), this, &QgsGrassMapcalc::mFunctionComboBox_activated );

  mStartMoveConnectorPoints.resize( 2 );
  mNextId = 0;

  // Set frame fixed (but for current desktop font/theme)
  mInputFrame->setMinimumHeight( mInputFrame->height() );
  mInputFrame->setMaximumHeight( mInputFrame->height() );

  mView = new QgsGrassMapcalcView( this, mViewFrame );
  QGridLayout *layout = new QGridLayout( mViewFrame );
  layout->addWidget( mView, 0, 0 );

  mCanvasScene = new QGraphicsScene( 0, 0, 400, 300 );
  mCanvasScene->setBackgroundBrush( QColor( 180, 180, 180 ) );

  mPaper = new QGraphicsRectItem();
  mCanvasScene->addItem( mPaper );
  mPaper->setBrush( QBrush( QColor( 255, 255, 255 ) ) );
  mPaper->show();

  resizeCanvas( 400, 300 );

  mView->setScene( mCanvasScene );


  QActionGroup *ag = new QActionGroup( this );
  QToolBar *tb = addToolBar( tr( "Mapcalc tools" ) );

  mActionAddMap = new QAction( QgsGrassPlugin::getThemeIcon( QStringLiteral( "mapcalc_add_map.png" ) ),
                               tr( "Add map" ), this );
  mActionAddMap->setCheckable( true );
  ag->addAction( mActionAddMap );
  tb->addAction( mActionAddMap );
  connect( mActionAddMap, &QAction::triggered, this, &QgsGrassMapcalc::addMap );

  mActionAddConstant = new QAction( QgsGrassPlugin::getThemeIcon( QStringLiteral( "mapcalc_add_constant.png" ) ),
                                    tr( "Add constant value" ), this );
  mActionAddConstant->setCheckable( true );
  ag->addAction( mActionAddConstant );
  tb->addAction( mActionAddConstant );
  connect( mActionAddConstant, &QAction::triggered, this, &QgsGrassMapcalc::addConstant );

  mActionAddFunction = new QAction( QgsGrassPlugin::getThemeIcon( QStringLiteral( "mapcalc_add_function.png" ) ),
                                    tr( "Add operator or function" ), this );
  mActionAddFunction->setCheckable( true );
  ag->addAction( mActionAddFunction );
  tb->addAction( mActionAddFunction );
  connect( mActionAddFunction, &QAction::triggered, this, &QgsGrassMapcalc::addFunction );

  mActionAddConnection = new QAction( QgsGrassPlugin::getThemeIcon( QStringLiteral( "mapcalc_add_connection.png" ) ),
                                      tr( "Add connection" ), this );
  mActionAddConnection->setCheckable( true );
  ag->addAction( mActionAddConnection );
  tb->addAction( mActionAddConnection );
  connect( mActionAddConnection, &QAction::triggered, this, &QgsGrassMapcalc::addConnection );

  mActionSelectItem = new QAction( QgsGrassPlugin::getThemeIcon( QStringLiteral( "mapcalc_select.png" ) ),
                                   tr( "Select item" ), this );
  mActionSelectItem->setCheckable( true );
  ag->addAction( mActionSelectItem );
  tb->addAction( mActionSelectItem );
  connect( mActionSelectItem, &QAction::triggered, this, &QgsGrassMapcalc::selectItem );

  mActionDeleteItem = new QAction( QgsGrassPlugin::getThemeIcon( QStringLiteral( "mapcalc_delete.png" ) ),
                                   tr( "Delete selected item" ), this );
  mActionDeleteItem->setCheckable( true );
  mActionDeleteItem->setEnabled( false );
  ag->addAction( mActionDeleteItem );
  tb->addAction( mActionDeleteItem );
  connect( mActionDeleteItem, &QAction::triggered, this, &QgsGrassMapcalc::deleteItem );

  mActionAddMap->setChecked( true );

  mActionLoad = new QAction( QgsGrassPlugin::getThemeIcon( QStringLiteral( "mapcalc_open.png" ) ),
                             tr( "Open" ), this );
  tb->addAction( mActionLoad );
  connect( mActionLoad, &QAction::triggered, this, &QgsGrassMapcalc::load );

  mActionSave = new QAction( QgsGrassPlugin::getThemeIcon( QStringLiteral( "mapcalc_save.png" ) ),
                             tr( "Save" ), this );
  tb->addAction( mActionSave );
  connect( mActionSave, &QAction::triggered, this, &QgsGrassMapcalc::save );
  mActionSave->setEnabled( false );

  mActionSaveAs = new QAction( QgsGrassPlugin::getThemeIcon( QStringLiteral( "mapcalc_save_as.png" ) ),
                               tr( "Save as" ), this );
  tb->addAction( mActionSaveAs );
  connect( mActionSaveAs, &QAction::triggered, this, &QgsGrassMapcalc::saveAs );

  // Map input
  mMapComboBox = new QgsGrassModuleInputComboBox( QgsGrassObject::Raster, this );
  mMapComboBox->setSizePolicy( QSizePolicy::Expanding, QSizePolicy:: Preferred );
  // QComboBox does not emit activated() when item is selected in completer popup
  connect( mMapComboBox, static_cast<void ( QComboBox::* )( const QString & )>( &QComboBox::activated ), this, &QgsGrassMapcalc::mapChanged );
  connect( mMapComboBox->completer(), static_cast<void ( QCompleter::* )( const QString & )>( &QCompleter::activated ), this, &QgsGrassMapcalc::mapChanged );
  connect( mMapComboBox, &QComboBox::editTextChanged, this, &QgsGrassMapcalc::mapChanged );
  bool firstSet = mMapComboBox->setFirst();
  Q_UNUSED( firstSet )
  mInputFrame->layout()->addWidget( mMapComboBox );

  /* Create functions */
  int t = QgsGrassMapcalcFunction::Operator;
  // Arithmetical
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "+" ), 2, tr( "Addition" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "-" ), 2, tr( "Subtraction" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "*" ), 2, tr( "Multiplication" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "/" ), 2, tr( "Division" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "%" ), 2, tr( "Modulus" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "^" ), 2, tr( "Exponentiation" ) ) );

  // Logical
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "==" ), 2, tr( "Equal" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "!=" ), 2, tr( "Not equal" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( ">" ),  2, tr( "Greater than" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( ">=" ), 2, tr( "Greater than or equal" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "<" ),  2, tr( "Less than" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "<=" ), 2, tr( "Less than or equal" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "&&" ), 2, tr( "And" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "||" ), 2, tr( "Or" ) ) );

  t = QgsGrassMapcalcFunction::Function;
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "abs" ),  1, tr( "Absolute value of x" ), QStringLiteral( "abs(x)" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "atan" ), 1, tr( "Inverse tangent of x (result is in degrees)" ), QStringLiteral( "atan(x)" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "atan" ), 2, tr( "Inverse tangent of y/x (result is in degrees)" ), QStringLiteral( "atan(x,y)" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "col" ), 0, tr( "Current column of moving window (starts with 1)" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "cos" ),  1, tr( "Cosine of x (x is in degrees)" ), QStringLiteral( "cos(x)" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "double" ), 1, tr( "Convert x to double-precision floating point" ), QStringLiteral( "double(x)" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "ewres" ), 0, tr( "Current east-west resolution" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "exp" ), 1, tr( "Exponential function of x" ), QStringLiteral( "exp(x)" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "exp" ), 2, tr( "x to the power y" ), QStringLiteral( "exp(x,y)" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "float" ), 1, tr( "Convert x to single-precision floating point" ), QStringLiteral( "float(x)" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "if" ), 1, tr( "Decision: 1 if x not zero, 0 otherwise" ), QStringLiteral( "if(x)" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "if" ), 2, tr( "Decision: a if x not zero, 0 otherwise" ), QStringLiteral( "if(x,a)" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "if" ), 3, tr( "Decision: a if x not zero, b otherwise" ), QStringLiteral( "if(x,a,b)" ), QStringLiteral( "if,then,else" ), false ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "if" ), 4, tr( "Decision: a if x > 0, b if x is zero, c if x < 0" ), QStringLiteral( "if(x,a,b,c)" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "int" ), 1, tr( "Convert x to integer [ truncates ]" ), QStringLiteral( "int(x)" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "isnull" ), 1, tr( "Check if x = NULL" ), QStringLiteral( "isnull(x)" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "log" ), 1, tr( "Natural log of x" ), QStringLiteral( "log(x)" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "log" ), 2, tr( "Log of x base b" ), QStringLiteral( "log(x,b)" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "max" ), 2, tr( "Largest value" ), QStringLiteral( "max(a,b)" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "max" ), 3, tr( "Largest value" ), QStringLiteral( "max(a,b,c)" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "median" ), 2, tr( "Median value" ), QStringLiteral( "median(a,b)" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "median" ), 3, tr( "Median value" ), QStringLiteral( "median(a,b,c)" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "min" ), 2, tr( "Smallest value" ), QStringLiteral( "min(a,b)" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "min" ), 3, tr( "Smallest value" ), QStringLiteral( "min(a,b,c)" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "mode" ), 2, tr( "Mode value" ), QStringLiteral( "mode(a,b)" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "mode" ), 3, tr( "Mode value" ), QStringLiteral( "mode(a,b,c)" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "not" ), 1, tr( "1 if x is zero, 0 otherwise" ), QStringLiteral( "not(x)" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "nsres" ), 0, tr( "Current north-south resolution" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "null" ), 0, tr( "NULL value" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "rand" ), 2, tr( "Random value between a and b" ), QStringLiteral( "rand(a,b)" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "round" ), 1, tr( "Round x to nearest integer" ), QStringLiteral( "round(x)" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "row" ), 0, tr( "Current row of moving window (Starts with 1)" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "sin" ), 1, tr( "Sine of x (x is in degrees)", "sin(x)" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "sqrt" ), 1, tr( "Square root of x", "sqrt(x)" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "tan" ), 1, tr( "Tangent of x (x is in degrees)", "tan(x)" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "x" ), 0, tr( "Current x-coordinate of moving window" ) ) );
  mFunctions.push_back( QgsGrassMapcalcFunction( t, QStringLiteral( "y" ), 0, tr( "Current y-coordinate of moving window" ) ) );

  for ( unsigned int i = 0; i < mFunctions.size(); i++ )
  {
    mFunctionComboBox->addItem( mFunctions[i].label()
                                + "  " + mFunctions[i].description() );
  }

  // Add output object
  mOutput = new QgsGrassMapcalcObject( QgsGrassMapcalcObject::Output );
  mOutput->setId( nextId() );
  mOutput->setValue( tr( "Output" ) );
  mCanvasScene->addItem( mOutput );
  mOutput->setCenter( ( int )( mCanvasScene->width() - mOutput->rect().width() ), ( int )( mCanvasScene->height() / 2 ) );
  mCanvasScene->update();
  mOutput->QGraphicsRectItem::show();

  // Set default tool
  if ( mMapComboBox->count() > 0 )
  {
    setTool( AddMap );
  }
  else
  {
    setTool( AddConstant );
  }
}

void QgsGrassMapcalc::mousePressEvent( QMouseEvent *e )
{
  QgsDebugMsgLevel( QString( "mTool = %1 mToolStep = %2" ).arg( mTool ).arg( mToolStep ), 4 );

  QPoint p = mView->mapToScene( e->pos() ).toPoint();
  limit( &p );

  switch ( mTool )
  {
    case AddMap:
    case AddConstant:
    case AddFunction:
      mObject->setCenter( p.x(), p.y() );
      mObject = nullptr;
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
      // Cleared previous
      if ( mObject )
      {
        mObject->setSelected( false );
        mObject = nullptr;
      }
      if ( mConnector )
      {
        mConnector->setSelected( false );
        mConnector = nullptr;
      }
      showOptions( Select );

      QRectF r( p.x() - 5, p.y() - 5, 10, 10 );
      QList<QGraphicsItem *> l = mCanvasScene->items( r );

      // Connector precedence (reverse order - connectors are under objects)
      QList<QGraphicsItem *>::const_iterator it = l.constEnd();
      while ( it != l.constBegin() )
      {
        --it;

        if ( QgsGrassMapcalcConnector *con = dynamic_cast<QgsGrassMapcalcConnector *>( *it ) )
        {
          mConnector = con;
          mConnector->setSelected( true );
          mConnector->selectEnd( p );
          mStartMoveConnectorPoints[0] = mConnector->point( 0 );
          mStartMoveConnectorPoints[1] = mConnector->point( 1 );

          break;
        }
        else if ( QgsGrassMapcalcObject *obj = dynamic_cast<QgsGrassMapcalcObject *>( *it ) )
        {
          mObject = obj;
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

      if ( ( mConnector && mConnector->selectedEnd() == -1 ) || mObject )
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
  mCanvasScene->update();
  mLastPoint = p;
  mStartMovePoint = p;
}

void QgsGrassMapcalc::mouseMoveEvent( QMouseEvent *e )
{
  // QgsDebugMsgLevel(QString("mTool = %1 mToolStep = %2").arg(mTool).arg(mToolStep), 4);

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

  mCanvasScene->update();
  mLastPoint = p;
}

void QgsGrassMapcalc::mouseReleaseEvent( QMouseEvent *e )
{
  QgsDebugMsgLevel( QString( "mTool = %1 mToolStep = %2" ).arg( mTool ).arg( mToolStep ), 4 );

  QPoint p = mView->mapToScene( e->pos() ).toPoint();
  limit( &p );

  switch ( mTool )
  {
    case AddConnector:
      if ( mToolStep == 1 )
      {
        QPoint p0 = mConnector->point( 0 );
        double d = std::sqrt( std::pow( ( double )( p.x() - p0.x() ), 2.0 )
                              + std::pow( ( double )( p.y() - p0.y() ), 2.0 ) );
        QgsDebugMsgLevel( QString( "d = %1" ).arg( d ), 4 );
        if ( d <  5 ) // filter 'single' clicks
        {
          mConnector->setSocket( 0 );   // disconnect
          delete mConnector;
        }
        mConnector = nullptr;
        setTool( mTool );  // restart
      }
      break;

    case Select:
      mView->setCursor( QCursor( Qt::ArrowCursor ) );
      break;
  }
  autoGrow();
  mCanvasScene->update();
  mLastPoint = p;
}

QStringList QgsGrassMapcalc::arguments()
{
  QString cmd;
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
  QStringList list;

  QList<QGraphicsItem *> l = mCanvasScene->items();

  struct Cell_head currentWindow;
  try
  {
    QgsGrass::region( &currentWindow );
  }
  catch ( QgsGrass::Exception &e )
  {
    QgsGrass::warning( e );
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

    QStringList mm = obj->value().split( '@' );
    if ( mm.size() < 1 )
      continue;

    QString map = mm.at( 0 );
    QString mapset = QgsGrass::getDefaultMapset();
    if ( mm.size() > 1 )
      mapset = mm.at( 1 );

    if ( !QgsGrass::mapRegion( QgsGrassObject::Raster,
                               QgsGrass::getDefaultGisdbase(),
                               QgsGrass::getDefaultLocation(), mapset, map,
                               &window ) )
    {
      QMessageBox::warning( nullptr, tr( "Warning" ), tr( "Cannot check region of map %1" ).arg( obj->value() ) );
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

bool QgsGrassMapcalc::inputRegion( struct Cell_head *window, QgsCoordinateReferenceSystem &crs, bool all )
{
  Q_UNUSED( crs )
  Q_UNUSED( all )

  try
  {
    QgsGrass::region( window );
  }
  catch ( QgsGrass::Exception &e )
  {
    QgsGrass::warning( e );
    return false;
  }

  QList<QGraphicsItem *> l = mCanvasScene->items();

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

    QStringList mm = obj->value().split( '@' );
    if ( mm.size() < 1 )
      continue;

    QString map = mm.at( 0 );
    QString mapset = QgsGrass::getDefaultMapset();
    if ( mm.size() > 1 )
      mapset = mm.at( 1 );

    if ( !QgsGrass::mapRegion( QgsGrassObject::Raster,
                               QgsGrass::getDefaultGisdbase(),
                               QgsGrass::getDefaultLocation(), mapset, map,
                               &mapWindow ) )
    {
      QMessageBox::warning( nullptr, tr( "Warning" ), tr( "Cannot get region of map %1" ).arg( obj->value() ) );
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
  QStringList list;
  if ( type == QgsGrassModuleOption::Raster )
  {
    list.append( mOutputLineEdit->text() );
  }
  return list;
}

void QgsGrassMapcalc::showOptions( int tool )
{
  QgsDebugMsgLevel( QString( "tool = %1" ).arg( tool ), 4 );

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

  if ( mTool != Select )
    return;
  if ( !mObject )
    return;

  switch ( mObject->type() )
  {
    case QgsGrassMapcalcObject::Map :
    {
      QStringList mapMapset = mObject->value().split( '@' );
      if ( !mMapComboBox->setCurrent( mapMapset.value( 0 ), mapMapset.value( 1 ) ) )
      {
        mMapComboBox->setEditText( mObject->value() );
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
    mCanvasScene->update();
  }
  mObject = nullptr;
  mConnector = nullptr;

  mTool = tool;
  mToolStep = 0;

  mView->viewport()->setMouseTracking( false );

  switch ( mTool )
  {
    case AddMap:
      mObject = new QgsGrassMapcalcObject( QgsGrassMapcalcObject::Map );
      mObject->setId( nextId() );
      mObject->setValue( mMapComboBox->currentText() );
      mObject->setCenter( mLastPoint.x(), mLastPoint.y() );
      mCanvasScene->addItem( mObject );
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
      mCanvasScene->addItem( mObject );
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
      mCanvasScene->addItem( mObject );
      mObject->QGraphicsRectItem::show();
      mActionAddFunction->setChecked( true );
      mView->viewport()->setMouseTracking( true );
      mView->setCursor( QCursor( Qt::SizeAllCursor ) );
      break;

    case AddConnector:
      mConnector = new QgsGrassMapcalcConnector( mCanvasScene );
      mConnector->setId( nextId() );
      mCanvasScene->addItem( mConnector );
      mConnector->QGraphicsLineItem::show();
      mActionAddConnection->setChecked( true );
      mView->setCursor( QCursor( Qt::CrossCursor ) );
      break;
  }

  showOptions( mTool );
  setToolActionsOff();
  mActionDeleteItem->setEnabled( false );
  mCanvasScene->update();
}

void QgsGrassMapcalc::addMap()
{
  if ( mMapComboBox->count() == 0 )
  {
    QMessageBox::warning( nullptr, tr( "Warning" ), tr( "No GRASS raster maps available" ) );

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
    mConnector = nullptr;
  }
  if ( mObject && mObject->type() != QgsGrassMapcalcObject::Output )
  {
    delete mObject;
    mObject = nullptr;
  }
  mActionDeleteItem->setEnabled( false );
  mCanvasScene->update();
}

void QgsGrassMapcalc::keyPressEvent( QKeyEvent *e )
{
  if ( e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete )
  {
    deleteItem();

    // Override default shortcut management in MapCanvas
    e->ignore();
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

void QgsGrassMapcalc::mapChanged( const QString &text )
{

  if ( ( mTool != AddMap && mTool != Select )  || !mObject )
    return;
  if ( mObject->type() != QgsGrassMapcalcObject::Map )
    return;

  mObject->setValue( text );
  mCanvasScene->update();
}

void QgsGrassMapcalc::constantChanged()
{

  if ( ( mTool != AddConstant && mTool != Select ) || !mObject )
    return;
  if ( mObject->type() != QgsGrassMapcalcObject::Constant )
    return;

  mObject->setValue( mConstantLineEdit->text() );
  mCanvasScene->update();
}

void QgsGrassMapcalc::functionChanged()
{

  if ( ( mTool != AddFunction && mTool != Select ) || !mObject )
    return;
  if ( mObject->type() != QgsGrassMapcalcObject::Function )
    return;

  mObject->setFunction( mFunctions[ mFunctionComboBox->currentIndex()] );
  mCanvasScene->update();
}

void QgsGrassMapcalc::limit( QPoint *point )
{
  if ( point->x() < 0 )
    point->setX( 0 );
  if ( point->y() < 0 )
    point->setY( 0 );
  if ( point->x() > mCanvasScene->width() )
    point->setX( mCanvasScene->width() );
  if ( point->y() > mCanvasScene->height() )
    point->setY( mCanvasScene->height() );
}

void QgsGrassMapcalc::resizeCanvas( int width, int height )
{
  mCanvasScene->setSceneRect( 0, 0, width, height );
  mPaper->setRect( 0, 0, width, height );
  mCanvasScene->update();
}

void QgsGrassMapcalc::growCanvas( int left, int right, int top, int bottom )
{
  QgsDebugMsgLevel( QString( "left = %1 right = %2 top = %3 bottom = %4" ).arg( left ).arg( right ).arg( top ).arg( bottom ), 4 );

  int width = mCanvasScene->width() + left + right;
  int height = mCanvasScene->height() + top + bottom;
  resizeCanvas( width, height );

  QList<QGraphicsItem *> l = mCanvasScene->items();

  QList<QGraphicsItem *>::const_iterator it = l.constEnd();
  while ( it != l.constBegin() )
  {
    --it;

    if ( QgsGrassMapcalcObject *obj = dynamic_cast<QgsGrassMapcalcObject *>( *it ) )
    {
      QPoint p = obj->center();
      obj->setCenter( p.x() + left, p.y() + top );
    }
    else if ( QgsGrassMapcalcConnector *con = dynamic_cast<QgsGrassMapcalcConnector *>( *it ) )
    {
      for ( int i = 0; i < 2; i++ )
      {
        QPoint p = con->point( i );
        p.setX( p.x() + left );
        p.setY( p.y() + top );
        con->setPoint( i,  p );
      }
    }
  }

  mCanvasScene->update();
}

void QgsGrassMapcalc::autoGrow()
{

  int thresh = 15;

  int left = 0;
  int right = mCanvasScene->width();
  int top = 0;
  int bottom = mCanvasScene->height();
  QgsDebugMsgLevel( QString( "left = %1 right = %2 top = %3 bottom = %4" ).arg( left ).arg( right ).arg( top ).arg( bottom ), 4 );

  QList<QGraphicsItem *> l = mCanvasScene->items();

  QList<QGraphicsItem *>::const_iterator it = l.constEnd();
  while ( it != l.constBegin() )
  {
    --it;
    if ( !dynamic_cast<QgsGrassMapcalcItem *>( *it ) )
      continue;

    // Exclude current
    if ( ( mTool != Select ) && ( *it == mObject || *it == mConnector ) )
      continue;

    QRectF r = ( *it )->boundingRect().translated( ( *it )->pos() );

    QgsDebugMsgLevel( QString( "r.left = %1 r.right = %2 r.top = %3 bottom = %4" ).arg( r.left() ).arg( r.right() ).arg( r.top() ).arg( r.bottom() ), 4 );

    if ( r.left() - thresh < left )
      left   = r.left() - thresh;
    if ( r.right() + thresh > right )
      right  = r.right() + thresh;
    if ( r.top() - thresh < top )
      top    = r.top() - thresh;
    if ( r.bottom() + thresh > bottom )
      bottom = r.bottom() + thresh;

    QgsDebugMsgLevel( QString( "left = %1 right = %2 top = %3 bottom = %4" ).arg( left ).arg( right ).arg( top ).arg( bottom ), 4 );
  }
  left = -left;
  right = right - mCanvasScene->width();
  top = -top;
  bottom = bottom - mCanvasScene->height();

  growCanvas( left, right, top, bottom );
}

void QgsGrassMapcalc::saveAs()
{

  // Check/create 'mapcalc' directory in current mapset
  QString ms = QgsGrass::getDefaultGisdbase() + "/"
               + QgsGrass::getDefaultLocation() + "/"
               + QgsGrass::getDefaultMapset();

  QString mc = ms + "/mapcalc";

  if ( !QFile::exists( mc ) )
  {
    QDir d( ms );

    if ( !d.mkdir( QStringLiteral( "mapcalc" ) ) )
    {
      QMessageBox::warning( nullptr, tr( "Warning" ), tr( "Cannot create 'mapcalc' directory in current mapset." ) );
      return;
    }
  }

  // Ask for file name
  QString name;
  for ( ;; )
  {
    bool ok;
    name = QInputDialog::getText( this, tr( "New mapcalc" ),
                                  tr( "Enter new mapcalc name:" ), QLineEdit::Normal, mFileName, &ok );
    if ( !ok )
      return;
    name = name.trimmed();

    if ( name.isEmpty() )
    {
      QMessageBox::warning( nullptr, tr( "Warning" ), tr( "Enter vector name" ) );
      continue;
    }

    // check if exists
    if ( QFile::exists( mc + "/" + name ) )
    {
      QMessageBox::StandardButton ret = QMessageBox::question( nullptr, tr( "Warning" ),
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
  if ( !out.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
  {
    QMessageBox::warning( this, tr( "Save mapcalc" ),
                          tr( "Cannot open mapcalc file" ) );
    return;
  }

  QTextStream stream( &out );

  stream << "<mapcalc>\n";
  stream << "  <canvas width=\"" + QString::number( mCanvasScene->width() )
         + "\" height=\"" + QString::number( mCanvasScene->height() )
         + "\"/>\n";

  QList<QGraphicsItem *> l = mCanvasScene->items();

  QList<QGraphicsItem *>::const_iterator it = l.constEnd();
  while ( it != l.constBegin() )
  {
    --it;

    if ( QgsGrassMapcalcObject *obj = dynamic_cast<QgsGrassMapcalcObject *>( *it ) )
    {
      QString type;
      if ( obj->type() == QgsGrassMapcalcObject::Map )
      {
        type = QStringLiteral( "map" );
      }
      else if ( obj->type() == QgsGrassMapcalcObject::Constant )
      {
        type = QStringLiteral( "constant" );
      }
      else if ( obj->type() == QgsGrassMapcalcObject::Function )
      {
        if ( obj->function().type() == QgsGrassMapcalcFunction::Operator )
          type = QStringLiteral( "operator" );
        else
          type = QStringLiteral( "function" );
      }
      else if ( obj->type() == QgsGrassMapcalcObject::Output )
      {
        type = QStringLiteral( "output" );
      }

      QString val = obj->value();
      if ( obj->type() == QgsGrassMapcalcObject::Function )
      {
        val.replace( QLatin1String( "&" ), QLatin1String( "&amp;" ) );
        val.replace( QLatin1String( "<" ), QLatin1String( "&lt;" ) );
        val.replace( QLatin1String( ">" ), QLatin1String( "&gt;" ) );
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
    else if ( QgsGrassMapcalcConnector *con = dynamic_cast<QgsGrassMapcalcConnector *>( *it ) )
    {
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

  QgsGrassSelect *sel = new QgsGrassSelect( this, QgsGrassSelect::MapCalc );
  if ( sel->exec() == QDialog::Rejected )
    return;

  // Open file
  QString path = sel->gisdbase + "/" + sel->location + "/"
                 + sel->mapset + "/mapcalc/" + sel->map;

  QFile file( path );

  if ( !file.exists() ) // should not happen
  {
    QMessageBox::warning( nullptr, tr( "Warning" ), tr( "The mapcalc schema (%1) not found." ).arg( path ) );
    return;
  }

  if ( ! file.open( QIODevice::ReadOnly ) )
  {
    QMessageBox::warning( nullptr, tr( "Warning" ), tr( "Cannot open mapcalc schema (%1)" ).arg( path ) );

    return;
  }

  QDomDocument doc( QStringLiteral( "mapcalc" ) );
  QString err;
  int line, column;
  int parsed = doc.setContent( &file,  &err, &line, &column );
  file.close();
  if ( !parsed )
  {
    QString errmsg = tr( "Cannot read mapcalc schema (%1):" ).arg( path )
                     + tr( "\n%1\nat line %2 column %3" ).arg( err ).arg( line ).arg( column );
    QMessageBox::warning( nullptr, tr( "Warning" ), errmsg );
    return;
  }

  clear();
  QDomElement docElem = doc.documentElement();

  // Set canvas
  QDomNodeList canvasNodes = docElem.elementsByTagName( QStringLiteral( "canvas" ) );
  QDomElement canvasElement = canvasNodes.item( 0 ).toElement();
  int width = canvasElement.attribute( QStringLiteral( "width" ), QStringLiteral( "300" ) ).toInt();
  int height = canvasElement.attribute( QStringLiteral( "height" ), QStringLiteral( "200" ) ).toInt();
  resizeCanvas( width, height );

  // Add objects
  std::vector<QgsGrassMapcalcObject *> objects;
  QDomNodeList objectNodes = docElem.elementsByTagName( QStringLiteral( "object" ) );
  QgsDebugMsgLevel( QString( "objectNodes.count() = %1" ).arg( objectNodes.count() ), 2 );
  for ( int n = 0; n < objectNodes.count(); n++ )
  {
    QDomNode node = objectNodes.item( n );
    QDomElement e = node.toElement();
    if ( e.isNull() )
      continue;

    QgsDebugMsgLevel( QString( "id = %1" ).arg( e.attribute( "id", "?" ).toLocal8Bit().constData() ), 2 );
    unsigned int id = e.attribute( QStringLiteral( "id" ), QStringLiteral( "0" ) ).toInt();
    int x = e.attribute( QStringLiteral( "x" ), QStringLiteral( "0" ) ).toInt();
    int y = e.attribute( QStringLiteral( "y" ), QStringLiteral( "0" ) ).toInt();
    QString typeName = e.attribute( QStringLiteral( "type" ), QStringLiteral( "constant" ) );
    QString value = e.attribute( QStringLiteral( "value" ), QStringLiteral( "???" ) );

    if ( id >= mNextId )
      mNextId = id + 1;
    if ( id >= objects.size() )
    {
      objects.resize( id + 1 );
    }

    int type = -1;

    if ( typeName == QLatin1String( "map" ) )
      type = QgsGrassMapcalcObject::Map;
    else if ( typeName == QLatin1String( "constant" ) )
      type = QgsGrassMapcalcObject::Constant;
    else if ( typeName == QLatin1String( "operator" ) )
      type = QgsGrassMapcalcObject::Function;
    else if ( typeName == QLatin1String( "function" ) )
      type = QgsGrassMapcalcObject::Function;
    else if ( typeName == QLatin1String( "output" ) )
      type = QgsGrassMapcalcObject::Output;

    if ( type == -1 )
      continue;

    QgsGrassMapcalcObject *obj = new QgsGrassMapcalcObject( type );
    objects[id] = obj;

    obj->setId( id );
    obj->setValue( value );
    obj->setCenter( x, y );
    mCanvasScene->addItem( obj );
    obj->show();

    switch ( type )
    {
      case QgsGrassMapcalcObject::Map:
      {
        QString label = QApplication::translate( "grasslabel", e.attribute( QStringLiteral( "label" ), QStringLiteral( "???" ) ).toUtf8() );
        obj->setValue( value, label );
        break;
      }

      case QgsGrassMapcalcObject::Output :
        obj->setValue( tr( "Output" ) );
        mOutput = obj;
        break;

      case QgsGrassMapcalcObject::Function :
        int inputCount = e.attribute( QStringLiteral( "inputCount" ), QStringLiteral( "1" ) ).toInt();
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
  QDomNodeList connectorNodes = docElem.elementsByTagName( QStringLiteral( "connector" ) );
  for ( int n = 0; n < connectorNodes.count(); n++ )
  {
    QDomNode node = connectorNodes.item( n );
    QDomElement e = node.toElement();
    if ( e.isNull() )
      continue;

    QgsDebugMsgLevel( QString( "id = %1" ).arg( e.attribute( "id", "?" ).toLocal8Bit().constData() ), 2 );
    unsigned int id = e.attribute( QStringLiteral( "id" ), QStringLiteral( "0" ) ).toInt();
    if ( id >= mNextId )
      mNextId = id + 1;

    QgsGrassMapcalcConnector *con = new QgsGrassMapcalcConnector( mCanvasScene );

    con->setId( id );
    mCanvasScene->addItem( con );
    con->show();

    QDomNodeList endNodes = e.elementsByTagName( QStringLiteral( "end" ) );
    QgsDebugMsgLevel( QString( "endNodes.count = %1" ).arg( endNodes.count() ), 2 );
    for ( int n2 = 0; n2 < endNodes.count() && n2 < 2; n2++ )
    {
      QDomNode node2 = endNodes.item( n2 );
      QDomElement e2 = node2.toElement();
      if ( e2.isNull() )
        continue;

      int x = e2.attribute( QStringLiteral( "x" ), QStringLiteral( "0" ) ).toInt();
      int y = e2.attribute( QStringLiteral( "y" ), QStringLiteral( "0" ) ).toInt();
      con->setPoint( n2, QPoint( x, y ) );

      QgsDebugMsgLevel( QString( "x = %1 y = %2" ).arg( x ).arg( y ), 2 );

      int objId = e2.attribute( QStringLiteral( "object" ), QStringLiteral( "-1" ) ).toInt();
      QgsDebugMsgLevel( QString( "objId = %1" ).arg( objId ), 2 );
      if ( objId < 0 )
        continue; // not connected

      if ( static_cast<uint>( objId ) < objects.size() && objects[objId] )
      {
        QString socketTypeName = e2.attribute( QStringLiteral( "socketType" ), QStringLiteral( "out" ) );
        int socketType;
        if ( socketTypeName == QLatin1String( "in" ) )
          socketType = QgsGrassMapcalcObject::In;
        else
          socketType = QgsGrassMapcalcObject::Out;

        int socket = e2.attribute( QStringLiteral( "socket" ), QStringLiteral( "0" ) ).toInt();

        QgsDebugMsgLevel( QString( "end = %1 objId = %2 socketType = %3 socket = %4" ).arg( n2 ).arg( objId ).arg( socketType ).arg( socket ), 2 );

        con->setSocket( n2, objects[objId], socketType, socket );

        objects[objId]->setConnector( socketType, socket, con, n2 );
      }
    }

  }

  mFileName = sel->map;
  mActionSave->setEnabled( true );
  mCanvasScene->update();
}

void QgsGrassMapcalc::clear()
{

  setTool( Select );

  QList<QGraphicsItem *> l = mCanvasScene->items();

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

/**************************** OBJECT ************************/
QgsGrassMapcalcObject::QgsGrassMapcalcObject( int type )
  : QGraphicsRectItem( -1000, -1000, 50, 20, nullptr )
  , mType( type )
  , mRound( 0. )
  , mCenter( -1000, -1000 )
  , mSocketHalf( 0. )
  , mMargin( 0. )
  , mSpace( 0. )
  , mTextHeight( 0 )
  , mInputTextWidth( 0 )
  , mSelectionBoxSize( 5 )
  , mOutputConnectorEnd( 0 )
{

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
  QgsDebugMsgLevel( "exited.", 4 );
}

int QgsGrassMapcalcObject::type() const
{
  return mType;
}

void QgsGrassMapcalcObject::paint( QPainter *painter,
                                   const QStyleOptionGraphicsItem *option, QWidget *widget )
{
  Q_UNUSED( option )
  Q_UNUSED( widget )
  //QGraphicsRectItem::paint(painter, option, widget);

  painter->setPen( QPen( QColor( 0, 0, 0 ) ) );
  painter->setBrush( QBrush( QColor( 255, 255, 255 ) ) );
  int xRound = ( int )( 100 * mRound / mRect.width() );
  int yRound = ( int )( 100 * mRound / mRect.height() );

  painter->drawRoundedRect( mRect, xRound, yRound );

  // Input sockets
  for ( int i = 0; i < mInputCount; i++ )
  {
    if ( mInputConnectors[i] )
      painter->setBrush( QBrush( QColor( 180, 180, 180 ) ) );
    else
      painter->setBrush( QBrush( QColor( 255, 0, 0 ) ) );

    painter->drawEllipse( mInputPoints[i].x() - mSocketHalf,
                          mInputPoints[i].y() - mSocketHalf,
                          2 * mSocketHalf + 1, 2 * mSocketHalf + 1 );
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
                          2 * mSocketHalf + 1, 2 * mSocketHalf + 1 );
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
      QRect lr( lx, ly, metrics.horizontalAdvance( l ), mTextHeight );

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
  // QgsDebugMsgLevel(QString("x = %1 y = %2").arg(x).arg(y), 2);
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
      QString l = mFunction.inputLabels().at( i );
      int len = metrics.horizontalAdvance( l );
      if ( len > mInputTextWidth )
        mInputTextWidth = len;
    }
  }

  int labelTextWidth = metrics.horizontalAdvance( mLabel );
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

  QGraphicsRectItem::setRect( 0, 0, width + 2 * mMargin, height + 2 * mMargin );

  // Label rectangle
  int lx = mRect.x() + mSpace;
  if ( mInputTextWidth > 0 )
  {
    lx += mInputTextWidth + mSpace;
  }
  int ly = mRect.y() + mSpace;
  if ( mInputCount > 1 )
  {
    ly += ( int )( ( mInputCount * mTextHeight +
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
  mOutputPoint.setY( ( int )( mRect.y() + mRect.height() / 2 ) );

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

  QPoint p = connector->point( end );

  // Input
  if ( !connector->connected( In ) )
  {
    for ( int i = 0; i < mInputCount; i++ )
    {
      if ( mInputConnectors[i] )
        continue; // used

      double d = std::sqrt( std::pow( ( double )( mInputPoints[i].x() + pos().x() - p.x() ), 2.0 )
                            + std::pow( ( double )( mInputPoints[i].y() + pos().y() - p.y() ), 2.0 ) );

      if ( d <= mSocketHalf )
      {
        QgsDebugMsgLevel( QString( "Object: connector connected to input %1" ).arg( i ), 2 );
        connector->setSocket( end, this, In, i );
        mInputConnectors[i] = connector;
        return true;
      }
    }
  }

  // Output
  if ( !connector->connected( Out ) && !mOutputConnector )
  {
    double d = std::sqrt( std::pow( ( double )( mOutputPoint.x() + pos().x() - p.x() ), 2.0 )
                          + std::pow( ( double )( mOutputPoint.y() + pos().y() - p.y() ), 2.0 ) );

    if ( d <= mSocketHalf )
    {
      QgsDebugMsgLevel( "Object: connector connected to output ", 2 );
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

  if ( direction == In )
  {
    return mInputPoints[socket] + pos().toPoint();
  }

  return mOutputPoint + pos().toPoint();
}

QString QgsGrassMapcalcObject::expression()
{
  QgsDebugMsgLevel( QString( "mType = %1" ).arg( mType ), 2 );

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
      return QStringLiteral( "null()" );
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

  QgsDebugMsgLevel( QString( "exp = %1" ).arg( exp.toLocal8Bit().constData() ), 2 );
  return exp;
}

/************************* CONNECTOR **********************************/
QgsGrassMapcalcConnector::QgsGrassMapcalcConnector( QGraphicsScene *canvas )
{
  canvas->addItem( this );

  QGraphicsLineItem::setZValue( 10 );

  mPoints.resize( 2 );
  mPoints[0] = QPoint( -1000, -1000 );
  mPoints[1] = QPoint( -1000, -1000 );

  mSocketObjects.resize( 2 );
  mSocketObjects[0] = nullptr;
  mSocketObjects[1] = nullptr;
  mSocketDir.resize( 2 );
  mSocket.resize( 2 );
}

QgsGrassMapcalcConnector::~QgsGrassMapcalcConnector()
{
  // Disconnect
  setSocket( 0 );
  setSocket( 1 );
}

void QgsGrassMapcalcConnector::paint( QPainter *painter,
                                      const QStyleOptionGraphicsItem *option, QWidget *widget )
{
  Q_UNUSED( option )
  Q_UNUSED( widget )
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
  // QgsDebugMsgLevel(QString("index = %1").arg(index), 2);

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
  mSelectedEnd = -1;

  double d0 = std::sqrt( std::pow( ( double )( point.x() - mPoints[0].x() ), 2.0 )
                         + std::pow( ( double )( point.y() - mPoints[0].y() ), 2.0 ) );

  double d1 = std::sqrt( std::pow( ( double )( point.x() - mPoints[1].x() ), 2.0 )
                         + std::pow( ( double )( point.y() - mPoints[1].y() ), 2.0 ) );


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
  QgsDebugMsgLevel( QString( "mSelectedEnd = %1" ).arg( mSelectedEnd ), 2 );
}

int QgsGrassMapcalcConnector::selectedEnd()
{
  return mSelectedEnd;
}

bool QgsGrassMapcalcConnector::tryConnectEnd( int end )
{

  QList<QGraphicsItem *> l = scene()->items( mPoints[end] );
  QgsGrassMapcalcObject *object = nullptr;
  QList<QGraphicsItem *>::const_iterator it = l.constEnd();
  while ( it != l.constBegin() )
  {
    --it;

    if ( ( object = dynamic_cast<QgsGrassMapcalcObject *>( *it ) ) )
      break;
  }

  // try to connect
  return object && object->tryConnect( this, end );
}

void QgsGrassMapcalcConnector::setSocket( int end,
    QgsGrassMapcalcObject *object, int direction, int socket )
{

  // Remove old connection from object
  if ( mSocketObjects[end] )
  {
    mSocketObjects[end]->setConnector( mSocketDir[end],
                                       mSocket[end] );

    mSocketObjects[end] = nullptr;
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
  for ( int i = 0; i < 2; i++ )
  {
    if ( !mSocketObjects[i] )
      continue;
    if ( mSocketDir[i] != QgsGrassMapcalcObject::Out )
      continue;
    return mSocketObjects[i]->expression();
  }

  return QStringLiteral( "null()" );
}

QgsGrassMapcalcObject *QgsGrassMapcalcConnector::object( int end )
{
  return mSocketObjects[end];
}

/************************* FUNCTION *****************************/
QgsGrassMapcalcFunction::QgsGrassMapcalcFunction( int type, QString name,
    int count, QString description, QString label, QString labels,
    bool drawLabel )
  : mName( name )
  , mType( type )
  , mInputCount( count )
  , mLabel( label )
  , mDescription( description )
  , mDrawLabel( drawLabel )
{
  if ( mLabel.isEmpty() )
    mLabel = mName;

  if ( !labels.isEmpty() )
  {
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    mInputLabels = labels.split( QStringLiteral( "," ), QString::SkipEmptyParts );
#else
    mInputLabels = labels.split( QStringLiteral( "," ), Qt::SkipEmptyParts );
#endif
  }
}

/******************** CANVAS VIEW ******************************/

QgsGrassMapcalcView::QgsGrassMapcalcView( QgsGrassMapcalc *mapcalc,
    QWidget *parent, Qt::WindowFlags f )
  : QGraphicsView( parent )
{
  Q_UNUSED( f )
  setAttribute( Qt::WA_StaticContents );
  mMapcalc = mapcalc;

  // TODO: nothing does work -> necessary to call setFocus ()
  setEnabled( true );
  setFocusPolicy( Qt::StrongFocus );
  setFocusProxy( nullptr );
}

void QgsGrassMapcalcView::mousePressEvent( QMouseEvent *e )
{
  // TODO: find how to get focus without setFocus
  setFocus();
  mMapcalc->mousePressEvent( e );
}

void QgsGrassMapcalcView::mouseReleaseEvent( QMouseEvent *e )
{
  mMapcalc->mouseReleaseEvent( e );
}

void QgsGrassMapcalcView::mouseMoveEvent( QMouseEvent *e )
{
  mMapcalc->mouseMoveEvent( e );
}

void QgsGrassMapcalcView::keyPressEvent( QKeyEvent *e )
{
  mMapcalc->keyPressEvent( e );
}
