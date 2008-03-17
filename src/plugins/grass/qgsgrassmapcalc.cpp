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
#include <iostream>

#include <qapplication.h>
#include <qstringlist.h>
#include <qlabel.h>
#include <QComboBox>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qpen.h>
#include <q3pointarray.h>
#include <qcursor.h>
#include <qevent.h>
#include <qpoint.h>
#include <qsize.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <q3groupbox.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <q3picture.h>
#include <q3canvas.h>
#include <qstatusbar.h>
#include <qinputdialog.h>
#include <qdom.h>

#include <QMouseEvent>
#include <QKeyEvent>
#include <QGridLayout>
#include <QTextStream>
#include <QActionGroup>
#include <QToolBar>

#include "qgis.h"
#include "qgisinterface.h"
#include "qgsapplication.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsvectorlayer.h"
#include "qgsdataprovider.h"
#include "qgsfield.h"

extern "C" {
#include <grass/gis.h>
#include <grass/Vect.h>
}

#include "../../src/providers/grass/qgsgrass.h"
#include "../../src/providers/grass/qgsgrassprovider.h"
#include "qgsgrassattributes.h"
#include "qgsgrassmodule.h"
#include "qgsgrasstools.h"
#include "qgsgrassmapcalc.h"
#include "qgsgrassselect.h"

QgsGrassMapcalc::QgsGrassMapcalc ( 
           QgsGrassTools *tools, QgsGrassModule *module,
           QgisInterface *iface, 
           QWidget * parent, const char * name, Qt::WFlags f )
       : QMainWindow(0,Qt::WType_Dialog),
         QgsGrassMapcalcBase ( ),
         QgsGrassModuleOptions( tools, module, iface),
	 mTool(-1), mObject(0), mConnector(0)
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassMapcalc()" << std::endl;
#endif

  setupUi(this);

  mStartMoveConnectorPoints.resize(2);
  mNextId = 0;

  // Set frame fixed (but for current desktop font/theme)
  mInputFrame->setMinimumHeight( mInputFrame->height() );
  mInputFrame->setMaximumHeight( mInputFrame->height() );

  mView = new QgsGrassMapcalcView ( this, mViewFrame);
  QGridLayout *layout = new QGridLayout( mViewFrame, 1, 1 );
  layout->addWidget( mView, 0, 0 );

  mCanvas = new Q3Canvas ( 400, 300 );
  mCanvas->setBackgroundColor( QColor(180,180,180) );

  mPaper = new Q3CanvasRectangle ( mCanvas );
  mPaper->setBrush ( QBrush(QColor(255,255,255)) );
  mPaper->setActive(false);
  mPaper->show();

  resizeCanvas( 400, 300 );

  mView->setCanvas ( mCanvas );

  QString myIconPath = QgsApplication::themePath() + "/grass/";

  QActionGroup *ag = new QActionGroup ( this );
  QToolBar *tb = addToolBar(tr("Mapcalc tools"));

  mActionAddMap = new QAction( QIcon(myIconPath+"mapcalc_add_map.png"), 
    tr("Add map"), this);
  mActionAddMap->setCheckable ( true );
  ag->addAction ( mActionAddMap );
  tb->addAction ( mActionAddMap );
  connect ( mActionAddMap, SIGNAL(triggered()), this, SLOT(addMap()) );

  mActionAddConstant = new QAction( QIcon(myIconPath+"mapcalc_add_constant.png"), 
    tr("Add constant value"), this);
  mActionAddConstant->setCheckable ( true );
  ag->addAction ( mActionAddConstant );
  tb->addAction ( mActionAddConstant );
  connect ( mActionAddConstant, SIGNAL(triggered()), this, SLOT(addConstant()) );

  mActionAddFunction = new QAction( QIcon(myIconPath+"mapcalc_add_function.png"), 
    tr("Add operator or function"), this);
  mActionAddFunction->setCheckable ( true );
  ag->addAction ( mActionAddFunction );
  tb->addAction ( mActionAddFunction );
  connect ( mActionAddFunction, SIGNAL(triggered()), this, SLOT(addFunction()) );

  mActionAddConnection = new QAction( QIcon(myIconPath+"mapcalc_add_connection.png"), 
    tr("Add connection"), this);
  mActionAddConnection->setCheckable ( true );
  ag->addAction ( mActionAddConnection );
  tb->addAction ( mActionAddConnection );
  connect ( mActionAddConnection, SIGNAL(triggered()), this, SLOT(addConnection()) );

  mActionSelectItem = new QAction( QIcon(myIconPath+"mapcalc_select.png"), 
    tr("Select item"), this);
  mActionSelectItem->setCheckable ( true );
  ag->addAction ( mActionSelectItem );
  tb->addAction ( mActionSelectItem );
  connect ( mActionSelectItem, SIGNAL(triggered()), this, SLOT(selectItem()) );

  mActionDeleteItem = new QAction( QIcon(myIconPath+"mapcalc_delete.png"), 
    tr("Delete selected item"), this);
  mActionDeleteItem->setCheckable ( true );
  mActionDeleteItem->setEnabled ( false );
  ag->addAction ( mActionDeleteItem );
  tb->addAction ( mActionDeleteItem );
  connect ( mActionDeleteItem, SIGNAL(triggered()), this, SLOT(deleteItem()) );

  mActionAddMap->setOn(true);

  mActionLoad = new QAction( QIcon(myIconPath+"mapcalc_open.png"), 
    tr("Open"), this);
  tb->addAction ( mActionLoad );
  connect ( mActionLoad, SIGNAL(triggered()), this, SLOT(load()) );

  mActionSave = new QAction( QIcon(myIconPath+"mapcalc_save.png"), 
    tr("Save"), this);
  tb->addAction ( mActionSave );
  connect ( mActionSave, SIGNAL(triggered()), this, SLOT(save()) );
  mActionSave->setEnabled(false);

  mActionSaveAs = new QAction( QIcon(myIconPath+"mapcalc_save_as.png"), 
    tr("Save as"), this);
  tb->addAction ( mActionSaveAs );
  connect ( mActionSaveAs, SIGNAL(triggered()), this, SLOT(saveAs()) );

  /* Create functions */
  int t = QgsGrassMapcalcFunction::Operator;
  //mFunctions.push_back(QgsGrassMapcalcFunction("-",2, "Odcitani", "in1,in2" ));
  // Arithmetical
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "+", 2, tr("Addition" )));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "-", 2, tr("Subtraction")));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "*", 2, tr("Multiplication" )));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "/", 2, tr("Division" )));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "%", 2, tr("Modulus" )));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "^", 2, tr("Exponentiation" )));

  // Logical
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "==", 2, tr("Equal" )));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "!=", 2, tr("Not equal" )));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, ">",  2, tr("Greater than" )));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, ">=", 2, tr("Greater than or equal" )));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "<",  2, tr("Less than" )));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "<=", 2, tr("Less than or equal" )));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "&&", 2, tr("And" )));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "||", 2, tr("Or" )));

  t = QgsGrassMapcalcFunction::Function;
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "abs",  1, tr("Absolute value of x"), "abs(x)" ));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "atan", 1, tr("Inverse tangent of x (result is in degrees)"), "atan(x)" ));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "atan", 2, tr("Inverse tangent of y/x (result is in degrees)"), "atan(x,y)" ));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "col", 0, tr("Current column of moving window (starts with 1)") ));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "cos",  1, tr("Cosine of x (x is in degrees)"), "cos(x)" ));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "double", 1, tr("Convert x to double-precision floating point"), "double(x)" ));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "ewres", 0, tr("Current east-west resolution" )));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "exp", 1, tr("Exponential function of x"), "exp(x)" ));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "exp", 2, tr("x to the power y"), "exp(x,y)" ));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "float", 2, tr("Convert x to single-precision floating point"), "float(x)" ));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "if", 1, tr("Decision: 1 if x not zero, 0 otherwise"), "if(x)" ));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "if", 2, tr("Decision: a if x not zero, 0 otherwise"), "if(x,a)" ));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "if", 3, tr("Decision: a if x not zero, b otherwise"), "if(x,a,b)", "if,then,else", false ));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "if", 4, tr("Decision: a if x > 0, b if x is zero, c if x < 0"), "if(x,a,b,c)" ));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "int", 1, tr("Convert x to integer [ truncates ]"), "int(x)" ));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "isnull", 1, tr("Check if x = NULL"), "isnull(x)" ));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "log", 1, tr("Natural log of x"), "log(x)" ));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "log", 2, tr("Log of x base b"), "log(x,b)" ));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "max", 2, tr("Largest value"), "max(a,b)" ));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "max", 3, tr("Largest value"), "max(a,b,c)" ));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "median", 2, tr("Median value"), "median(a,b)" ));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "median", 3, tr("Median value"), "median(a,b,c)" ));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "min", 2, tr("Smallest value"), "min(a,b)" ));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "min", 3, tr("Smallest value"), "min(a,b,c)" ));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "mode", 2, tr("Mode value"), "mode(a,b)" ));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "mode", 3, tr("Mode value"), "mode(a,b,c)" ));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "not", 1, tr("1 if x is zero, 0 otherwise"), "not(x)" ));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "nsres", 0, tr("Current north-south resolution" )));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "null", 0, tr("NULL value" )));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "rand", 2, tr("Random value between a and b"), "rand(a,b)" ));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "round", 1, tr("Round x to nearest integer"), "round(x)" ));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "row", 0, tr("Current row of moving window (Starts with 1)" )));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "sin", 1, tr("Sine of x (x is in degrees)", "sin(x)" )));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "sqrt", 1, tr("Square root of x", "sqrt(x)" )));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "tan", 1, tr("Tangent of x (x is in degrees)", "tan(x)" )));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "x", 0, tr("Current x-coordinate of moving window" )));
  mFunctions.push_back(QgsGrassMapcalcFunction( t, "y", 0, tr("Current y-coordinate of moving window" )));

  for ( unsigned int i =0; i < mFunctions.size(); i++ )
  {
    mFunctionComboBox->insertItem( mFunctions[i].label() 
      + "  " + mFunctions[i].description() );
  }

  // Add output object
  mOutput = new QgsGrassMapcalcObject( QgsGrassMapcalcObject::Output);
  mOutput->setId ( nextId() );
  mOutput->setValue ( tr("Output") );
  mOutput->setCanvas(mCanvas);
  mOutput->setCenter ( (int)(mCanvas->width()-mOutput->width()), (int)(mCanvas->height()/2) ), 
    mCanvas->update();
  mOutput->Q3CanvasRectangle::show();

  // Set default tool
  updateMaps();
  if ( mMaps.size() > 0 )
  {
    setTool ( AddMap ); 
  }
  else
  {
    setTool ( AddConstant );
  }
}

void QgsGrassMapcalc::contentsMousePressEvent(QMouseEvent* e)
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassMapcalc::contentsMousePressEvent mTool = " 
    << mTool << " mToolStep = " << mToolStep << std::endl;
#endif

  QPoint p = mView->inverseWorldMatrix().map(e->pos());
  limit(&p);

  switch ( mTool ) 
  {
  case AddMap:
  case AddConstant:
  case AddFunction:
    mObject->setCenter ( p.x(), p.y() ); 
    mObject = 0;
    //addMap(); // restart
    setTool ( mTool ); // restart
    break;

  case AddConnector:
    if ( mToolStep == 0 )
    { 
      mConnector->setPoint ( 0, p ); 
      mConnector->setPoint ( 1, p ); 
      // Try to connect 
      mConnector->tryConnectEnd ( 0 );
      mToolStep = 1;
    }
    break;

  case Select:
    // Cleare previous
    if ( mObject ) 
    {
      mObject->setSelected ( false );
      mObject = 0;
    }
    if ( mConnector ) 
    {
      mConnector->setSelected ( false );
      mConnector = 0;
    }
    showOptions(Select);

    QRect r ( p.x()-5, p.y()-5, 10, 10 );
    Q3CanvasItemList l = mCanvas->collisions(r);

    // Connector precedence (reverse order - connectors are under objects)
    for ( Q3CanvasItemList::Iterator it=l.fromLast(); it!=l.end(); --it) {
      if (! (*it)->isActive() ) continue;

      if ( typeid (**it) == typeid (QgsGrassMapcalcConnector) )
      {
        mConnector = dynamic_cast <QgsGrassMapcalcConnector *> (*it);
        mConnector->setSelected ( true );
        mConnector->selectEnd ( p );
        mStartMoveConnectorPoints[0] = mConnector->point(0);
        mStartMoveConnectorPoints[1] = mConnector->point(1);

        break;
      }
      else if ( typeid (**it) == typeid (QgsGrassMapcalcObject) )
      {
        mObject = dynamic_cast <QgsGrassMapcalcObject *> (*it);
        mObject->setSelected ( true );

        int tool = Select;
        if ( mObject->type() == QgsGrassMapcalcObject::Map )
          tool = AddMap;
        else if ( mObject->type() == QgsGrassMapcalcObject::Constant )
          tool = AddConstant;
        else if ( mObject->type() == QgsGrassMapcalcObject::Function )
          tool = AddFunction;

        showOptions(tool);

        break;
      } 
    }

    if ( (mConnector && mConnector->selectedEnd() == -1) || mObject )
    {
      mView->setCursor ( QCursor(Qt::SizeAllCursor) ); 
    } 
    else if ( mConnector )
    {
      mView->setCursor ( QCursor(Qt::CrossCursor) ); 
    }

    if ( mConnector || 
      ( mObject && mObject->type() != QgsGrassMapcalcObject::Output ) )
    {
      mActionDeleteItem->setEnabled(true);
    }
    else
    {
      mActionDeleteItem->setEnabled(false);
    }

    setOption();
    break;
  }
  mCanvas->update();
  mLastPoint = p;
  mStartMovePoint = p;
}

void QgsGrassMapcalc::contentsMouseMoveEvent(QMouseEvent* e)
{
#ifdef QGISDEBUG
  //    std::cerr << "QgsGrassMapcalc::contentsMouseMoveEvent mTool = " 
  //            << mTool << " mToolStep = " << mToolStep << std::endl;
#endif

  QPoint p = mView->inverseWorldMatrix().map(e->pos());
  limit(&p);

  switch ( mTool ) 
  {
  case AddMap:
  case AddConstant:
  case AddFunction:
    mObject->setCenter ( p.x(), p.y() ); 
    break;

  case AddConnector:
    if ( mToolStep == 1 )
    { 
      mConnector->setPoint ( 1, p ); 
      mConnector->setSocket  ( 1 ); // disconnect
      mConnector->tryConnectEnd ( 1 ); // try to connect
    }
    break;

  case Select:
    if ( mObject )
    {
      int dx = p.x() - mLastPoint.x();
      int dy = p.y() - mLastPoint.y();
      QPoint c = mObject->center();
      mObject->setCenter ( c.x()+dx, c.y()+dy );
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
          mConnector->setSocket  ( i ); // disconnect
          mConnector->setPoint ( i, QPoint( 
            mStartMoveConnectorPoints[i].x()+dx, 
            mStartMoveConnectorPoints[i].y()+dy) );
          mConnector->tryConnectEnd ( i ); // try to connect
        }
      }
      else  
      {
        mConnector->setSocket  ( end ); // disconnect
        mConnector->setPoint ( end, QPoint(p.x(),p.y()) );
        mConnector->tryConnectEnd ( end ); // try to connect
      }
    }
    break;
  }

  mCanvas->update();
  mLastPoint = p;
}

void QgsGrassMapcalc::contentsMouseReleaseEvent(QMouseEvent* e)
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassMapcalc::contentsMouseReleaseEvent mTool = " 
    << mTool << " mToolStep = " << mToolStep << std::endl;
#endif

  QPoint p = mView->inverseWorldMatrix().map(e->pos());
  limit(&p);

  switch ( mTool ) 
  {
  case AddConnector:
    if ( mToolStep == 1 )
    {
      QPoint p0 = mConnector->point(0);
      double d = sqrt ( pow( (double)(p.x()-p0.x()), 2.0 )
        + pow( (double)(p.y()-p0.y()), 2.0 ) );
      std::cerr << "d = " << d << std::endl;
      if ( d <  5 ) // filter 'single' clicks
      {
        mConnector->setSocket  ( 0 ); // disconnect
        delete mConnector;
      }
      mConnector = 0;
      setTool ( mTool ); // restart
    }
    break;

  case Select:
    mView->setCursor ( QCursor(Qt::ArrowCursor) ); 
    break;
  }
  autoGrow();
  mCanvas->update();
  mLastPoint = p;
}

QStringList QgsGrassMapcalc::arguments()
{
  QString cmd = "";
  //cmd.append("'");

  cmd.append( mOutputLineEdit->text() );
  cmd.append("=");
  cmd.append ( mOutput->expression() );
  //cmd.append("'");

  //cmd = "\"pok=1\"";
  //cmd = mOutputLineEdit->text();

  return QStringList ( cmd );
}

QStringList QgsGrassMapcalc::checkOutput()
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassMapcalc::checkOutput()" << std::endl;
#endif
  QStringList list;

  QString value = mOutputLineEdit->text().trimmed();

  if ( value.length() == 0 ) return QStringList();

  QString path = QgsGrass::getDefaultGisdbase() + "/"
    + QgsGrass::getDefaultLocation() + "/"
    + QgsGrass::getDefaultMapset()
    + "/cell/" + value;

  QFileInfo fi(path);

  if ( fi.exists() )
  {
    return ( QStringList(value) );
  }

  return QStringList();
}

QStringList QgsGrassMapcalc::checkRegion()
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassMapcalc::checkRegion()" << std::endl;
#endif
  QStringList list;

  Q3CanvasItemList l = mCanvas->allItems();

  struct Cell_head currentWindow;
  if  ( !QgsGrass::region ( QgsGrass::getDefaultGisdbase(),
    QgsGrass::getDefaultLocation(),
    QgsGrass::getDefaultMapset(), &currentWindow ) )
  {
    QMessageBox::warning( 0, tr("Warning"), tr("Cannot get current region" ));
    return list;
  }

  for ( Q3CanvasItemList::Iterator it=l.fromLast(); it!=l.end(); --it) 
  {
    if (! (*it)->isActive() ) continue;

    if ( typeid (**it) != typeid (QgsGrassMapcalcObject) ) continue;

    QgsGrassMapcalcObject *obj = 
      dynamic_cast <QgsGrassMapcalcObject *> (*it);

    if ( obj->type() != QgsGrassMapcalcObject::Map ) continue;

    struct Cell_head window;

    QStringList mm = obj->value().split("@");
    if ( mm.size() < 1 ) continue;

    QString map = mm.at(0);
    QString mapset = QgsGrass::getDefaultMapset();
    if  ( mm.size() > 1 ) mapset = mm.at(1);

    if ( !QgsGrass::mapRegion ( QgsGrass::Raster,
      QgsGrass::getDefaultGisdbase(),
      QgsGrass::getDefaultLocation(), mapset, map,
      &window ) )
    {
      QMessageBox::warning( 0, tr("Warning"), tr("Cannot check region "
        "of map ") + obj->value() );
      continue;
    }

    if ( G_window_overlap ( &currentWindow ,
      window.north, window.south, window.east, window.west) == 0 )
    {
      list.append ( obj->value() );
    }
  }
  return list;
}

bool QgsGrassMapcalc::inputRegion ( struct Cell_head *window, bool all )
{
#ifdef QGISDEBUG
  std::cerr << "gsGrassMapcalc::inputRegion" << std::endl;
#endif

  if  ( !QgsGrass::region ( QgsGrass::getDefaultGisdbase(),
    QgsGrass::getDefaultLocation(),
    QgsGrass::getDefaultMapset(), window ) )
  {
    QMessageBox::warning( 0, tr("Warning"), tr("Cannot get current region" ));
    return false;
  }

  Q3CanvasItemList l = mCanvas->allItems();

  int count = 0;
  for ( Q3CanvasItemList::Iterator it=l.fromLast(); it!=l.end(); --it) 
  {
    if (! (*it)->isActive() ) continue;

    if ( typeid (**it) != typeid (QgsGrassMapcalcObject) ) continue;

    QgsGrassMapcalcObject *obj = 
      dynamic_cast <QgsGrassMapcalcObject *> (*it);

    if ( obj->type() != QgsGrassMapcalcObject::Map ) continue;

    struct Cell_head mapWindow;

    QStringList mm = obj->value().split("@");
    if ( mm.size() < 1 ) continue;

    QString map = mm.at(0);
    QString mapset = QgsGrass::getDefaultMapset();
    if  ( mm.size() > 1 ) mapset = mm.at(1);

    if ( !QgsGrass::mapRegion ( QgsGrass::Raster,
      QgsGrass::getDefaultGisdbase(),
      QgsGrass::getDefaultLocation(), mapset, map,
      &mapWindow ) )
    {
      QMessageBox::warning( 0, tr("Warning"), tr("Cannot get region "
        "of map ") + obj->value() );
      return false;
    }

    // TODO: best way to set resolution ?
    if ( count == 0)
    {
      QgsGrass::copyRegionExtent ( &mapWindow, window );
      QgsGrass::copyRegionResolution ( &mapWindow, window );
    }
    else
    {
      QgsGrass::extendRegion ( &mapWindow, window );
    }
    count++;
  }

  return true;
}

QStringList QgsGrassMapcalc::output ( int type )
{
#ifdef QGISDEBUG
  std::cerr << "gsGrassMapcalc::output" << std::endl;
#endif
  QStringList list;
  if ( type == QgsGrassModuleOption::Raster ) 
  {
    list.append ( mOutputLineEdit->text() );
  }
  return list;
}

QgsGrassMapcalc::~QgsGrassMapcalc()
{
}

void QgsGrassMapcalc::showOptions( int tool )
{
  std::cerr << "QgsGrassMapcalc::showOptions() tool = " << tool << std::endl;

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
  std::cerr << "QgsGrassMapcalc::setOption()" << std::endl;

  if ( mTool != Select ) return;
  if ( !mObject ) return;

  switch ( mObject->type() )
  {
  case QgsGrassMapcalcObject::Map :
    {
      bool found = false;
      for ( unsigned int i = 0 ; i < mMaps.size(); i++ ) 
      { 
        if ( mMapComboBox->text(i) == mObject->label()
          && mMaps[i] == mObject->value() )
        {
          mMapComboBox->setCurrentItem ( i ) ;
          found = true;
        }
      }
      if ( !found ) 
      {
        mMaps.push_back ( mObject->value() );
        mMapComboBox->insertItem ( mObject->label() );
        mMapComboBox->setCurrentItem ( mMapComboBox->count()-1 );
      }
      break;
    }

  case QgsGrassMapcalcObject::Constant :
    mConstantLineEdit->setText ( mObject->value() ); 
    break;

  case QgsGrassMapcalcObject::Function :
    for ( unsigned int i = 0; i < mFunctions.size(); i++ ) 
    {
      if ( mFunctions[i].name() != mObject->function().name() ) continue;
      if ( mFunctions[i].inputCount() != mObject->function().inputCount() ) continue;

      mFunctionComboBox->setCurrentItem ( i );
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
    if ( mObject ) mObject->setSelected ( false );
    if ( mConnector ) mConnector->setSelected ( false );
  }
  else 
  {
    if ( mObject ) delete mObject;
    if ( mConnector ) delete mConnector;
    mCanvas->update();
  }
  mObject = 0;
  mConnector = 0;

  mTool = tool;
  mToolStep = 0;

  mView->viewport()->setMouseTracking ( false );

  switch ( mTool )
  {
  case AddMap:
    mObject = new QgsGrassMapcalcObject(QgsGrassMapcalcObject::Map);
    mObject->setId ( nextId() );

    // TODO check if there are maps
    mObject->setValue ( mMaps[mMapComboBox->currentItem()],
      mMapComboBox->currentText() );

    mObject->setCanvas(mCanvas);
    mObject->Q3CanvasRectangle::show();
    mActionAddMap->setOn(true);
    mView->viewport()->setMouseTracking ( true );
    mView->setCursor ( QCursor(Qt::SizeAllCursor) ); 
    break;

  case AddConstant:
    mObject = new QgsGrassMapcalcObject(QgsGrassMapcalcObject::Constant);
    mObject->setId ( nextId() );
    mObject->setValue ( mConstantLineEdit->text() );
    mObject->setCanvas(mCanvas);
    mObject->Q3CanvasRectangle::show();
    mActionAddConstant->setOn(true);
    mView->viewport()->setMouseTracking ( true );
    mView->setCursor ( QCursor(Qt::SizeAllCursor) ); 
    break;

  case AddFunction:
    mObject = new QgsGrassMapcalcObject(QgsGrassMapcalcObject::Function);
    mObject->setId ( nextId() );
    //mObject->setValue ( mFunctionComboBox->currentText() );
    mObject->setFunction ( mFunctions[ mFunctionComboBox->currentItem() ] );
    mObject->setCanvas(mCanvas);
    mObject->Q3CanvasRectangle::show();
    mActionAddFunction->setOn(true);
    mView->viewport()->setMouseTracking ( true );
    mView->setCursor ( QCursor(Qt::SizeAllCursor) ); 
    break;

  case AddConnector:
    mConnector = new QgsGrassMapcalcConnector ( mCanvas );
    mConnector->setId ( nextId() );
    mConnector->setCanvas(mCanvas);
    mConnector->Q3CanvasLine::show();
    mActionAddConnection->setOn(true);
    mView->setCursor ( QCursor(Qt::CrossCursor) ); 
    break;
  }

  showOptions(mTool);
  setToolActionsOff(); 
  mActionDeleteItem->setEnabled( false );
  mCanvas->update();
}

void QgsGrassMapcalc::addMap()
{
  updateMaps();
  if ( mMaps.size() == 0 )
  {
    QMessageBox::warning( 0, tr("Warning"), tr("No GRASS raster maps"
      " currently in QGIS" ));

    setTool ( AddConstant);
    return;
  }

  setTool(AddMap);
}

void QgsGrassMapcalc::addConstant()
{
  setTool(AddConstant);
}

void QgsGrassMapcalc::addFunction()
{
  setTool(AddFunction);
}

void QgsGrassMapcalc::addConnection()
{
  setTool(AddConnector);
}

void QgsGrassMapcalc::selectItem()
{
  setTool(Select);
  mActionSelectItem->setOn(true);
  mView->setCursor ( QCursor(Qt::ArrowCursor) ); 
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
  mActionDeleteItem->setEnabled(false);
  mCanvas->update();
}

void QgsGrassMapcalc::keyPressEvent ( QKeyEvent * e )
{
  if ( e->key() == Qt::Key_Delete ) 
  {
    deleteItem();
  }
}

void QgsGrassMapcalc::setToolActionsOff()
{
  mActionAddMap->setOn(false);
  mActionAddConstant->setOn(false);
  mActionAddFunction->setOn(false);
  mActionAddConnection->setOn(false);
  mActionSelectItem->setOn(false);
  mActionDeleteItem->setOn(false);
}

void QgsGrassMapcalc::updateMaps()
{
  // TODO: this copy and paste from QgsGrassModuleInput, do it better
#ifdef QGISDEBUG
  std::cerr << "QgsGrassMapcalc::updateMaps" << std::endl;
#endif
  QString current = mMapComboBox->currentText ();
  mMapComboBox->clear();
  mMaps.resize(0);

  QgsMapCanvas *canvas = mIface->getMapCanvas();

  int nlayers = canvas->layerCount();
  std::cerr << "nlayers = " << nlayers << std::endl;
  for ( int i = 0; i < nlayers; i++ ) {
    QgsMapLayer *layer = canvas->getZpos(i);

    if ( layer->type() != QgsMapLayer::RASTER ) continue;

    // Check if it is GRASS raster
    QString source = QDir::cleanDirPath ( layer->source() ); 

    // Note: QDir::cleanPath is using '/' also on Windows
    //QChar sep = QDir::separator();
    QChar sep = '/';

    if ( source.contains( "cellhd" ) == 0 ) continue;

    // Most probably GRASS layer, check GISBASE and LOCATION
    QStringList split = QStringList::split ( sep, source );

    if ( split.size() < 4 ) continue;

    QString map = split.last();
    split.pop_back(); // map
    if ( split.last() != "cellhd" ) continue;
    split.pop_back(); // cellhd

    QString mapset = split.last();
    split.pop_back(); // mapset

    //QDir locDir ( sep + split.join ( QString(sep) ) ) ;
    //QString loc = locDir.canonicalPath();

    QString loc =  source.remove ( QRegExp("/[^/]+/[^/]+/[^/]+$") );
    loc = QDir(loc).canonicalPath();

    QDir curlocDir ( QgsGrass::getDefaultGisdbase() + sep + QgsGrass::getDefaultLocation() );
    QString curloc = curlocDir.canonicalPath();

    if ( loc != curloc ) continue;

    //if ( mUpdate && mapset != QgsGrass::getDefaultMapset() ) continue;

    mMapComboBox->insertItem( layer->name() );
    if ( layer->name() == current ) mMapComboBox->setCurrentText ( current );
    mMaps.push_back ( map + "@" + mapset );
  }
}

void QgsGrassMapcalc::mapChanged()
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassMapcalc::mapChanged" << std::endl;
#endif

  if ( (mTool != AddMap && mTool != Select)  || !mObject ) return;
  if ( mObject->type() != QgsGrassMapcalcObject::Map ) return;

  mObject->setValue ( mMaps[mMapComboBox->currentItem()],
    mMapComboBox->currentText() );
  mObject->resetSize();
  mCanvas->update();
}

void QgsGrassMapcalc::constantChanged()
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassMapcalc::constantChanged" << std::endl;
#endif

  if ( (mTool != AddConstant && mTool != Select) || !mObject ) return;
  if ( mObject->type() != QgsGrassMapcalcObject::Constant ) return;

  mObject->setValue ( mConstantLineEdit->text() );
  mObject->resetSize();
  mCanvas->update();
}

void QgsGrassMapcalc::functionChanged()
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassMapcalc::functionChanged" << std::endl;
#endif

  if ( (mTool != AddFunction && mTool != Select) || !mObject ) return;
  if ( mObject->type() != QgsGrassMapcalcObject::Function ) return;

  mObject->setFunction ( mFunctions[ mFunctionComboBox->currentItem() ] );
  mObject->resetSize();
  mCanvas->update();
}

void QgsGrassMapcalc::limit( QPoint *point)
{
  if ( point->x() < 0 ) point->setX(0);
  if ( point->y() < 0 ) point->setY(0);
  if ( point->x() > mCanvas->width() ) point->setX(mCanvas->width());
  if ( point->y() > mCanvas->height() ) point->setY(mCanvas->height());
}

void QgsGrassMapcalc::resizeCanvas( int width, int height )
{
  mCanvas->resize ( width, height );
  mPaper->setSize ( width, height );
  mCanvas->update();
}

void QgsGrassMapcalc::growCanvas(int left, int right, int top, int bottom)
{
  std::cerr << "QgsGrassMapcalc::growCanvas()" << std::endl;
  std::cerr << "left = " << left << " right = " << right
    << " top = " << top << " bottom = " << bottom << std::endl;

  int width = mCanvas->width() + left + right;
  int height = mCanvas->height() + top + bottom;
  resizeCanvas( width, height );

  Q3CanvasItemList l = mCanvas->allItems();

  for ( Q3CanvasItemList::Iterator it=l.fromLast(); it!=l.end(); --it) 
  {
    if (! (*it)->isActive() ) continue;

    if ( typeid (**it) == typeid (QgsGrassMapcalcObject) )
    {
      QgsGrassMapcalcObject *obj = 
        dynamic_cast <QgsGrassMapcalcObject *> (*it);

      QPoint p = obj->center();
      obj->setCenter ( p.x()+left, p.y()+top );
    } 
    else if ( typeid (**it) == typeid (QgsGrassMapcalcConnector) )
    {
      QgsGrassMapcalcConnector *con = 
        dynamic_cast <QgsGrassMapcalcConnector *> (*it);

      for ( int i = 0; i < 2; i++ )
      {
        QPoint p = con->point(i);
        p.setX ( p.x()+left );
        p.setY ( p.y()+top );
        con->setPoint ( i,  p );
      }
    }
  }

  mCanvas->update();
}

void QgsGrassMapcalc::autoGrow()
{
  std::cerr << "QgsGrassMapcalc::autoGrow()" << std::endl;

  int thresh = 15;

  int left = 0;
  int right = mCanvas->width();
  int top = 0;
  int bottom = mCanvas->height();
  std::cerr << "left = " << left << " right = " << right
    << " top = " << top << " bottom = " << bottom << std::endl;

  Q3CanvasItemList l = mCanvas->allItems();

  for ( Q3CanvasItemList::Iterator it=l.fromLast(); it!=l.end(); --it) 
  {
    if (! (*it)->isActive() ) continue;

    // Exclude current
    if ( (mTool != Select) && (*it == mObject || *it == mConnector) ) 
      continue;

    QRect r = (*it)->boundingRect(); 

    std::cerr << "r.left = " << r.left() << " r.right = " << r.right()
      << " r.top = " << r.top() << " bottom = " << r.bottom () << std::endl;

    if ( r.left() - thresh < left )     left   = r.left() - thresh;
    if ( r.right() + thresh > right )   right  = r.right() + thresh;
    if ( r.top() - thresh < top )       top    = r.top() - thresh;
    if ( r.bottom() + thresh > bottom ) bottom = r.bottom() + thresh;

    std::cerr << "left = " << left << " right = " << right
      << " top = " << top << " bottom = " << bottom << std::endl;
  }
  left = -left;
  right = right - mCanvas->width();
  top = -top;
  bottom = bottom - mCanvas->height();

  growCanvas ( left, right, top, bottom );
}

void QgsGrassMapcalc::saveAs()
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassMapcalc::saveAs()" << std::endl;
#endif

  // Check/create 'mapcalc' directory in current mapset
  QString ms = QgsGrass::getDefaultGisdbase() + "/" 
    + QgsGrass::getDefaultLocation() + "/"
    + QgsGrass::getDefaultMapset();

  QString mc = ms + "/mapcalc";

  if ( !QFile::exists(mc) )
  {
    QDir d(ms);

    if ( !d.mkdir("mapcalc" ) )
    {
      QMessageBox::warning( 0, tr("Warning"), tr("Cannot create 'mapcalc' "
        "directory in current mapset." ));
      return;
    }
  }

  // Ask for file name
  QString name;
  while ( 1 ) 
  {
    bool ok;
    name = QInputDialog::getText( tr("New mapcalc"), 
      tr("Enter new mapcalc name:"), QLineEdit::Normal, mFileName, &ok);
    if ( !ok ) return;
    name = name.stripWhiteSpace();

    if ( name.isEmpty() ) {
      QMessageBox::warning( 0, tr("Warning"), tr("Enter vector name" ));
      continue;
    }

    // check if exists
    if ( QFile::exists( mc+ "/" + name ) )
    {
      QMessageBox::StandardButton ret = QMessageBox::question ( 0, tr("Warning"), 
        tr("The file already exists. Overwrite? "),  
        QMessageBox::Ok | QMessageBox::Cancel );

      if ( ret == QMessageBox::Cancel ) continue;   
    }
    break;
  }

  mFileName = name;
  mActionSave->setEnabled(true);
  save();
}

void QgsGrassMapcalc::save()
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassMapcalc::save()" << std::endl;
#endif
  if ( mFileName.isEmpty() ) // Should not happen
  {
    QMessageBox::warning (this, tr("Save mapcalc"), tr("File name empty") );
    return;
  }
  // TODO!!!: 'escape' < > & ...

  // Erase objects of Add* tools
  int tool = mTool;
  setTool(Select);

  // Open file 
  QString path = QgsGrass::getDefaultGisdbase() + "/" 
    + QgsGrass::getDefaultLocation() + "/"
    + QgsGrass::getDefaultMapset() 
    + "/mapcalc/" + mFileName;

  QFile out ( path );
  if ( !out.open( QIODevice::WriteOnly ) )
  {
    QMessageBox::warning (this, tr("Save mapcalc"), 
      tr("Cannot open mapcalc file") );
    return;
  }

  QTextStream stream ( &out );		    

  stream << "<mapcalc>\n"; 
  stream << "  <canvas width=\"" + QString::number(mCanvas->width())
    + "\" height=\"" + QString::number(mCanvas->height())
    + "\"/>\n"; 

  Q3CanvasItemList l = mCanvas->allItems();

  for ( Q3CanvasItemList::Iterator it=l.fromLast(); it!=l.end(); --it) 
  {
    if (! (*it)->isActive() ) continue;

    if ( typeid (**it) == typeid (QgsGrassMapcalcObject) )
    {
      QgsGrassMapcalcObject *obj = 
        dynamic_cast <QgsGrassMapcalcObject *> (*it);

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
        val.replace ( "&", "&amp;" );
        val.replace ( "<", "&lt;" );
        val.replace ( ">", "&gt;" );
      }

      stream << "  <object id=\"" + QString::number(obj->id()) 
        + "\" x=\"" + QString::number(obj->center().x()) 
        + "\" y=\"" + QString::number(obj->center().y()) 
        + "\" type=\"" + type 
        + "\" value=\"" + val + "\"";

      if ( obj->type() == QgsGrassMapcalcObject::Function )
      {
        stream << "  inputCount=\"" 
          + QString::number(obj->function().inputCount()) + "\"";
      }
      if ( obj->type() == QgsGrassMapcalcObject::Map )
      {
        stream << "  label=\"" + obj->label() + "\"";
      }
      stream <<  "/>\n";
    } 
    else if ( typeid (**it) == typeid (QgsGrassMapcalcConnector) )
    {
      QgsGrassMapcalcConnector *con = 
        dynamic_cast <QgsGrassMapcalcConnector *> (*it);

      stream << "  <connector id=\"" + QString::number(con->id()) 
        + "\">\n";

      for ( int i = 0; i < 2; i++ ) 
      {
        stream << "    <end x=\"" + QString::number(con->point(i).x()) 
          + "\" y=\"" + QString::number(con->point(i).y())
          + "\"";
        if ( con->object(i) )
        {
          stream << " object=\"" 
            + QString::number(con->object(i)->id()) 
            + "\" socketType=\"";

          if ( con->socketDirection(i) == QgsGrassMapcalcObject::In )
          {
            stream << "in"; 
          }
          else
          {
            stream << "out"; 
          }

          stream << "\" socket=\"" 
            + QString::number(con->socket(i)) 
            + "\"";
        }
        stream << "/>\n";

      }
      stream << "  </connector>\n";
    }
  }

  stream << "</mapcalc>"; 
  out.close();
  setTool(tool);
}

void QgsGrassMapcalc::load()
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassMapcalc::load()" << std::endl;
#endif

  QgsGrassSelect *sel = new QgsGrassSelect(QgsGrassSelect::MAPCALC );
  if ( sel->exec() == QDialog::Rejected ) return;

  // Open file 
  QString path = sel->gisdbase + "/" + sel->location + "/" 
    + sel->mapset + "/mapcalc/" + sel->map;

  QFile file ( path );

  if ( !file.exists() ) // should not happen
  {
    QMessageBox::warning( 0, tr("Warning"), tr("The mapcalc schema (") 
      + path + tr(") not found." ));
    return;
  }

  if ( ! file.open( QIODevice::ReadOnly ) ) 
  {
    QMessageBox::warning( 0, tr("Warning"), tr("Cannot open mapcalc schema (")
      + path + ")" );

    return;
  }

  QDomDocument doc ( "mapcalc" );
  QString err;
  int line, column;
  int parsed = doc.setContent( &file,  &err, &line, &column );
  file.close();
  if ( !parsed ) {
    QString errmsg = tr("Cannot read mapcalc schema (") + path + "):\n" + err 
      + tr("\nat line ") + QString::number(line) 
      + tr(" column ") + QString::number(column);

    QMessageBox::warning( 0, tr("Warning"), errmsg );
    return;
  }

  clear();
  QDomElement docElem = doc.documentElement();

  // Set canvas
  QDomNodeList canvasNodes = docElem.elementsByTagName ( "canvas" );
  QDomElement canvasElement = canvasNodes.item(0).toElement();
  int width = canvasElement.attribute("width","300").toInt();
  int height = canvasElement.attribute("height","200").toInt();
  resizeCanvas(width, height );

  // Add objects
  std::vector<QgsGrassMapcalcObject *> objects;
  QDomNodeList objectNodes = docElem.elementsByTagName ( "object" );
  std::cerr << "objectNodes.count() = " << objectNodes.count() << std::endl;
  for ( int n = 0; n < objectNodes.count(); n++ ) 
  {
    QDomNode node = objectNodes.item(n);
    QDomElement e = node.toElement();
    if( e.isNull() ) continue;

    std::cerr << "id = " << e.attribute("id","?").local8Bit().data() << std::endl;
    unsigned int id = e.attribute("id","0").toInt();
    int x = e.attribute("x","0").toInt();
    int y = e.attribute("y","0").toInt();
    QString typeName = e.attribute("type","constant");
    QString value = e.attribute("value","???");

    if ( id >= mNextId ) mNextId = id+1;
    if ( id >= objects.size() )
    {
      objects.resize(id + 1);
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

    if ( type == -1 ) continue;

    QgsGrassMapcalcObject *obj = new QgsGrassMapcalcObject ( type );
    objects[id] = obj;

    obj->setId ( id );
    obj->setValue ( value );
    obj->setCenter ( x, y );
    obj->setCanvas ( mCanvas );
    obj->show();

    switch ( type )
    {
    case QgsGrassMapcalcObject::Map:
      {
        QString label = e.attribute("label","???");
        obj->setValue ( value, label );
        break;
      }

    case QgsGrassMapcalcObject::Output :
      obj->setValue ( tr("Output") );
      mOutput = obj;
      break;

    case QgsGrassMapcalcObject::Function :
      int inputCount = e.attribute("inputCount","1").toInt();
      // Find function
      int fn = -1;
      for ( unsigned int i = 0; i < mFunctions.size(); i++ ) 
      {
        if ( mFunctions[i].name() != value ) continue;
        if ( mFunctions[i].inputCount() != inputCount ) continue;
        fn = i;
      }

      if ( fn >= 0 )
      {
        obj->setFunction ( mFunctions[fn] );
      }
      // TODO: if not found

      break;
    }
  }

  // Add connectors
  QDomNodeList connectorNodes = docElem.elementsByTagName ( "connector" );
  for ( int n = 0; n < connectorNodes.count(); n++ ) 
  {
    QDomNode node = connectorNodes.item(n);
    QDomElement e = node.toElement();
    if( e.isNull() ) continue;

    std::cerr << "id = " << e.attribute("id","?").local8Bit().data() << std::endl;
    unsigned int id = e.attribute("id","0").toInt();
    if ( id >= mNextId ) mNextId = id+1;

    QgsGrassMapcalcConnector *con = new QgsGrassMapcalcConnector ( mCanvas );

    con->setId ( id );
    con->setCanvas ( mCanvas );
    con->show();

    QDomNodeList endNodes = e.elementsByTagName ( "end" );
    std::cerr << "endNodes.count = " <<  endNodes.count() << std::endl;
    for ( int n2 = 0; n2 < endNodes.count() && n2 < 2; n2++ ) 
    {
      QDomNode node2 = endNodes.item(n2);
      QDomElement e2 = node2.toElement();
      if( e2.isNull() ) continue;

      int x = e2.attribute("x","0").toInt();
      int y = e2.attribute("y","0").toInt();
      con->setPoint ( n2, QPoint(x,y) ); 

      std::cerr << "x = " << x << " y = " << y << std::endl;

      int objId = e2.attribute("object","-1").toInt();
      std::cerr << "objId = " << objId << std::endl;
      if ( objId < 0 ) continue; // not connected 

      if ( static_cast<uint>(objId) < objects.size() && objects[objId] )
      {
        QString socketTypeName = e2.attribute("socketType","out");
        int socketType;
        if ( socketTypeName == "in" )
          socketType = QgsGrassMapcalcObject::In;
        else
          socketType = QgsGrassMapcalcObject::Out;

        int socket = e2.attribute("socket","0").toInt();

        std::cerr << "end =  " << n2 << " objId = " << objId
          << " socketType = " << socketType 
          << " socket = " << socket  << std::endl;

        con->setSocket ( n2, objects[objId], socketType, socket );

        objects[objId]->setConnector ( socketType, socket, con, n2 );
      }
    }

  }

  mFileName = sel->map;
  mActionSave->setEnabled(true);
  mCanvas->update();
}

void QgsGrassMapcalc::clear()
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassMapcalc::clear()" << std::endl;
#endif

  setTool (Select);

  Q3CanvasItemList l = mCanvas->allItems();

  for ( Q3CanvasItemList::Iterator it=l.fromLast(); it!=l.end(); --it) 
  {
    if (! (*it)->isActive() ) continue;

    delete *it;
  }
  mNextId = 0;
}

/******************** CANVAS ITEMS ******************************/
QgsGrassMapcalcItem::QgsGrassMapcalcItem(): mSelected(false)
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassMapcalcItem::QgsGrassMapcalcItem()" << std::endl;
#endif
}

QgsGrassMapcalcItem::~QgsGrassMapcalcItem()
{
}

void QgsGrassMapcalcItem::setSelected(bool s)
{
  mSelected = s;
}

bool QgsGrassMapcalcItem::selected()
{
  return mSelected;
}

/**************************** OBJECT ************************/
QgsGrassMapcalcObject::QgsGrassMapcalcObject( int type )
       :Q3CanvasRectangle(-1000,-1000,50,20,0), QgsGrassMapcalcItem(),
       mType(type),mCenter(-1000,-1000), mSelectionBoxSize(5),
       mOutputConnector(0)
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassMapcalcObject::QgsGrassMapcalcObject()" << std::endl;
#endif

  Q3CanvasRectangle::setZ(20);
  setActive(true);

  mInputCount = 0;
  mOutputCount = 1;

  if ( mType == Function ) mInputCount = 2;

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
  std::cerr << "QgsGrassMapcalcObject::~QgsGrassMapcalcObject()" << std::endl;
  // Delete connections
  for ( int i = 0; i < mInputCount; i++ )
  { 
    if ( mInputConnectors[i] )
    {
      QgsGrassMapcalcConnector *con = mInputConnectors[i];
      mInputConnectors[i]->setSocket ( mInputConnectorsEnd[i] );
      con->repaint();
    }
  }
  if ( mOutputConnector )
  {
    QgsGrassMapcalcConnector *con = mOutputConnector;
    mOutputConnector->setSocket ( mOutputConnectorEnd );
    con->repaint();
  }
  std::cerr << "QgsGrassMapcalcObject::~QgsGrassMapcalcObject() end" << std::endl;
}

int QgsGrassMapcalcObject::type()
{
  return mType;
}

void QgsGrassMapcalcObject::draw( QPainter & painter )
{
  //QCanvasRectangle::draw(painter);

  painter.setPen ( QPen(QColor(0,0,0)) );
  painter.setBrush ( QBrush(QColor(255,255,255)) );
  int xRound = (int) ( 100 * mRound / mRect.width() );
  int yRound = (int) ( 100 * mRound / mRect.height() );

  painter.drawRoundRect ( mRect, xRound, yRound );

  // Input sockets
  for ( int i = 0; i < mInputCount; i++ )
  {
    if ( mInputConnectors[i] )
      painter.setBrush ( QBrush(QColor(180,180,180)) );
    else
      painter.setBrush ( QBrush(QColor(255,0,0)) );

    painter.drawEllipse ( mInputPoints[i].x()-mSocketHalf,
      mInputPoints[i].y()-mSocketHalf, 
      2*mSocketHalf+1, 2*mSocketHalf+1 );
  }

  // Output socket
  if ( mOutputCount > 0 ) 
  {
    if ( mOutputConnector )
      painter.setBrush ( QBrush(QColor(180,180,180)) );
    else
      painter.setBrush ( QBrush(QColor(255,0,0)) );

    painter.drawEllipse ( mOutputPoint.x()-mSocketHalf,
      mOutputPoint.y()-mSocketHalf, 
      2*mSocketHalf+1, 2*mSocketHalf+1 );
  }

  // Input labels
  if ( mType == Function && mInputTextWidth > 0 )
  {
    painter.setFont ( mFont );
    QFontMetrics metrics ( mFont );
    for ( int i = 0; i < mFunction.inputLabels().size(); i++ ) 
    {
      /*
      QStringList::Iterator it = mFunction.inputLabels().at(i); 
      QString l = *it;
      */
      QString l = mFunction.inputLabels().at(i); 


      int lx = mRect.x()+mSpace;
      int ly = mRect.y() +mSpace + i*(mTextHeight+mSpace);
      QRect lr ( lx, ly, metrics.width(l), mTextHeight ) ;

      painter.drawText ( lr, Qt::AlignCenter|Qt::TextSingleLine, l );
    }
  }

  // Label    
  if ( mType != Function || mFunction.drawlabel() )
  {
    painter.drawText ( mLabelRect, Qt::AlignCenter|Qt::TextSingleLine, mLabel );
  }

  // Selection
  if ( mSelected ) 
  {
    painter.setPen( QColor(0,255,255) );
    painter.setBrush( QColor(0,255,255) );

    int s = mSelectionBoxSize;

    painter.drawRect ( mRect.x(), mRect.y(), s, s );
    painter.drawRect ( mRect.x()+mRect.width()-s, mRect.y(), s, s );
    painter.drawRect ( mRect.x()+mRect.width()-s, 
      mRect.y()+mRect.height()-s, s, s );
    painter.drawRect ( mRect.x(), mRect.y()+mRect.height()-s, s, s );
  }
}

void QgsGrassMapcalcObject::setCenter( int x, int y )
{
  //    std::cerr << "QgsGrassMapcalcObject::setCenter() x = " << x << " y = " << y << std::endl;
  mCenter.setX(x);
  mCenter.setY(y);
  resetSize();
  //QCanvasRectangle::update();
}

void QgsGrassMapcalcObject::resetSize()
{
  mSocketHalf = (int) ( mFont.pointSize()/3 + 1 );
  mSpace = (int) ( 1.0*mFont.pointSize() );
  mRound = (int) ( 1.0*mTextHeight );
  mMargin = 2*mSocketHalf+1;

  QFontMetrics metrics ( mFont );
  mTextHeight = metrics.height();

  mInputTextWidth = 0;
  if ( mType == Function )
  {
    for ( int i = 0; i < mFunction.inputLabels().size(); i++ ) 
    {
      /*
      QStringList::Iterator it = mFunction.inputLabels().at(i); 
      QString l = *it;
      */
      QString l = mFunction.inputLabels().at(i); 
      int len = metrics.width ( l );
      if ( len > mInputTextWidth ) mInputTextWidth = len;
    }
  }

  int labelTextWidth = metrics.width ( mLabel );
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
    height = mInputCount*(mTextHeight+mSpace) + mSpace;
  }
  else // Label only
  {
    height = 2*mSpace + mTextHeight;
  }

  mRect.setX ( (int)(mCenter.x()-width/2) );
  mRect.setY ( (int)(mCenter.y()-height/2) );
  mRect.setSize ( QSize(width, height) );

  Q3CanvasRectangle::setX ( mRect.x()-mMargin );
  Q3CanvasRectangle::setY ( mRect.y()-mMargin );
  Q3CanvasRectangle::setSize ( width+2*mMargin, height+2*mMargin );

  // Label rectangle
  int lx = mRect.x()+mSpace;
  if ( mInputTextWidth > 0 )
  {
    lx += mInputTextWidth + mSpace;
  }
  int ly = mRect.y()+mSpace;
  if ( mInputCount > 1 )
  {
    ly += (int)( (mInputCount*mTextHeight + 
      (mInputCount-1)*mSpace)/2 - mTextHeight/2 );
  }
  mLabelRect.setX(lx);
  mLabelRect.setY(ly);
  mLabelRect.setSize ( QSize(labelTextWidth, mTextHeight) );

  // Input sockets
  mInputPoints.resize( mInputCount );

  for ( int i = 0; i < mInputCount; i++ )
  {
    mInputPoints[i] = QPoint ( mRect.x()-mSocketHalf-1, 
      (int)(mRect.y() + (i+1)*(mSpace+mTextHeight) - mTextHeight/2) );
  }

  // Output socket
  mOutputPoint.setX ( mRect.right()+mSocketHalf+1 );
  mOutputPoint.setY ( (int) ( mRect.y()+mRect.height()/2 ) );

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

  Q3CanvasRectangle::update();
}

void QgsGrassMapcalcObject::setValue ( QString value, QString lab )
{
  mValue = value;
  if ( lab.isEmpty() ) {
    mLabel = mValue;
  }
  else
  {
    mLabel = lab;
  }

  resetSize();
}

void QgsGrassMapcalcObject::setFunction ( QgsGrassMapcalcFunction f )
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

void QgsGrassMapcalcObject::setSelected(bool s)
{
  mSelected = s;
  Q3CanvasRectangle::update();
}

bool QgsGrassMapcalcObject::tryConnect( QgsGrassMapcalcConnector *connector,
                                        int end  )
{
  std::cerr << "QgsGrassMapcalcObject::connect" << std::endl;

  QPoint p = connector->point ( end );

  // Input
  if ( !connector->connected(In) ) 
  {
    for ( int i = 0; i < mInputCount; i++ )
    {
      if ( mInputConnectors[i] ) continue; // used

      double d = sqrt ( pow( (double)(mInputPoints[i].x() - p.x()), 2.0 )
        + pow( (double)(mInputPoints[i].y() - p.y()), 2.0 ) );

      if ( d <= mSocketHalf )
      {
        std::cerr << "Object: connector connected to input " << i << std::endl;
        connector->setSocket  ( end, this, In, i ); 
        mInputConnectors[i] = connector;
        return true;
      }
    }
  }

  // Output
  if ( !connector->connected(Out) && !mOutputConnector )
  {
    double d = sqrt ( pow( (double)(mOutputPoint.x() - p.x()), 2.0 )
      + pow( (double)(mOutputPoint.y() - p.y()), 2.0 ) );

    if ( d <= mSocketHalf )
    {
      std::cerr << "Object: connector connected to output " << std::endl;
      connector->setSocket  ( end, this, Out ); 
      mOutputConnector = connector;
      return true;
    }
  }

  return false;
}

void QgsGrassMapcalcObject::setConnector ( int direction, int socket,
                                           QgsGrassMapcalcConnector *connector, int end )
{
  std::cerr << "QgsGrassMapcalcObject::setConnector" << std::endl;

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

  Q3CanvasRectangle::update();
}

QPoint QgsGrassMapcalcObject::socketPoint ( int direction , int socket  )
{
  //    std::cerr << "QgsGrassMapcalcObject::socketPoint" << std::endl;

  if ( direction == In )
  {
    return mInputPoints[socket];
  }

  return mOutputPoint;
}

QString QgsGrassMapcalcObject::expression()
{
  std::cerr << "QgsGrassMapcalcObject::expression()" << std::endl;
  std::cerr << "mType = " << mType << std::endl;

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
    exp.append ( mFunction.name() );

  exp.append ( "(" );

  for ( int i = 0; i < mInputCount; i++ )
  { 
    if ( i > 0 )
    {
      if ( mFunction.type() == QgsGrassMapcalcFunction::Function )
        exp.append ( "," );
      else
        exp.append ( mFunction.name() );
    }

    if ( mInputConnectors[i] )
      exp.append ( mInputConnectors[i]->expression() );
    else
      exp.append ( "null()" );

  }

  exp.append ( ")" );

  std::cerr << "exp = " << exp.local8Bit().data() << std::endl;
  return exp;
}

/************************* CONNECTOR **********************************/
QgsGrassMapcalcConnector::QgsGrassMapcalcConnector( Q3Canvas *canvas )
       :Q3CanvasLine(canvas), QgsGrassMapcalcItem()
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassMapcalcConnector::QgsGrassMapcalcConnector()" << std::endl;
#endif

  Q3CanvasLine::setZ(10);
  setActive(true);

  mPoints.resize(2);
  mPoints[0] = QPoint ( -1000, -1000 );
  mPoints[1] = QPoint ( -1000, -1000 );

  mSocketObjects.resize(2);
  mSocketObjects[0] = 0;
  mSocketObjects[1] = 0;
  mSocketDir.resize(2);
  mSocket.resize(2);
}

QgsGrassMapcalcConnector::~QgsGrassMapcalcConnector()
{
  // Disconnect
  setSocket(0); 
  setSocket(1); 
}

void QgsGrassMapcalcConnector::draw( QPainter & painter )
{
  for ( int i = 0; i < 2; i++ ) 
  {
    if ( mSocketObjects[i] ) {
      mPoints[i] = mSocketObjects[i]->socketPoint ( mSocketDir[i], 
        mSocket[i] );
    }
  }

  if ( !mSocketObjects[0] || !mSocketObjects[1] )
  {
    painter.setPen ( QPen(QColor(255,0,0)) );
  }
  else
  {
    painter.setPen ( QPen(QColor(0,0,0)) );
  }

  painter.drawLine ( mPoints[0], mPoints[1] );

  if ( mSelected )
  {
    painter.setPen ( QPen(QColor(0,255,255), 0, Qt::DotLine ) );
  } 
  painter.drawLine ( mPoints[0], mPoints[1] );
}

void QgsGrassMapcalcConnector::repaint()
{
  setPoint ( 0, point (0) );
  //QCanvasLine::setX(QCanvasLine::x());
  Q3CanvasLine::update();
}

void QgsGrassMapcalcConnector::setPoint ( int index, QPoint point  )
{
  //    std::cerr << "QgsGrassMapcalcConnector::setPoint index = " << index << std::endl;

  mPoints[index] = point;
  Q3CanvasLine::setPoints ( mPoints[0].x(), mPoints[0].y(), 
    mPoints[1].x(), mPoints[1].y() );
  Q3CanvasLine::update();
}

QPoint QgsGrassMapcalcConnector::point ( int index )
{
  return ( mPoints[index] );
}

void QgsGrassMapcalcConnector::setSelected(bool s)
{
  mSelected = s;
  Q3CanvasLine::update();
}

void QgsGrassMapcalcConnector::selectEnd( QPoint point )
{
  std::cerr << "QgsGrassMapcalcConnector::selectEnd" << std::endl;
  mSelectedEnd = -1;

  double d0 = sqrt ( pow((double)(point.x()-mPoints[0].x()),2.0) 
    + pow((double)(point.y()-mPoints[0].y()),2.0) );

  double d1 = sqrt ( pow((double)(point.x()-mPoints[1].x()),2.0) 
    + pow((double)(point.y()-mPoints[1].y()),2.0) );


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
  std::cerr << "mSelectedEnd = " << mSelectedEnd << std::endl;
}

int QgsGrassMapcalcConnector::selectedEnd()
{
  return mSelectedEnd;
}

bool QgsGrassMapcalcConnector::tryConnectEnd( int end )
{
  std::cerr << "QgsGrassMapcalcConnector::tryConnect" << std::endl;

  Q3CanvasItemList l = canvas()->collisions( mPoints[end] );
  QgsGrassMapcalcObject *object = 0;
  for ( Q3CanvasItemList::Iterator it=l.fromLast(); it!=l.end(); --it) 
  {
    if (! (*it)->isActive() ) continue;

    if ( typeid (**it) == typeid (QgsGrassMapcalcObject) )
    {
      object = dynamic_cast <QgsGrassMapcalcObject *> (*it);
      break;
    }
  }

  if ( !object ) return false;

  // try to connect
  if ( !object->tryConnect ( this, end ) ) return false;

  return true;
}

void QgsGrassMapcalcConnector::setSocket( int end, 
                                          QgsGrassMapcalcObject *object, int direction, int socket )
{
  std::cerr << "QgsGrassMapcalcConnector::setSocket" << std::endl;

  // Remove old connection from object
  if ( mSocketObjects[end] )
  {
    mSocketObjects[end]->setConnector ( mSocketDir[end], 
      mSocket[end] );

    mSocketObjects[end] = 0;
  }

  // Create new connection 
  mSocketObjects[end] = object;
  mSocketDir[end] = direction;
  mSocket[end] = socket;

  if ( !object ) return; // disconnect only

  mSocketObjects[end]->setConnector ( mSocketDir[end], 
    mSocket[end], this, end );
}

bool QgsGrassMapcalcConnector::connected ( int direction )
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
  std::cerr << "QgsGrassMapcalcConnector::expression()" << std::endl;
  for ( int i = 0; i < 2; i++ )
  {
    if ( !mSocketObjects[i] ) continue;
    if ( mSocketDir[i] != QgsGrassMapcalcObject::Out ) continue;
    return mSocketObjects[i]->expression();
  }

  return "null()";
}

QgsGrassMapcalcObject *QgsGrassMapcalcConnector::object( int end )
{
  return mSocketObjects[end];
}

/************************* FUNCTION *****************************/
QgsGrassMapcalcFunction::QgsGrassMapcalcFunction ( int type, QString name, 
                                                   int count, QString description, QString label, QString labels,
                                                   bool drawLabel ) :
  mName(name), mType(type), mInputCount(count), 
  mLabel(label), mDescription(description),
  mDrawLabel(drawLabel)
{
  if ( mLabel.isEmpty() ) mLabel = mName;

  if ( !labels.isEmpty() )
  {
    mInputLabels = QStringList::split ( ",", labels );
  }
}

/******************** CANVAS VIEW ******************************/

QgsGrassMapcalcView::QgsGrassMapcalcView( QgsGrassMapcalc *mapcalc, 
      QWidget* parent, const char* name, Qt::WFlags f) :
      Q3CanvasView(parent,name,f|Qt::WNoAutoErase|Qt::WResizeNoErase|Qt::WStaticContents)
{
  mMapcalc = mapcalc;

  // TODO: nothing does work -> necessary to call setFocus ()
  setEnabled ( true );
  setFocusPolicy ( Qt::StrongFocus );
  setFocusProxy ( 0 );
}

void QgsGrassMapcalcView::contentsMousePressEvent(QMouseEvent* e)
{
  // TODO: find how to get focus without setFocus
  setFocus ();
  mMapcalc->contentsMousePressEvent(e);
}

void QgsGrassMapcalcView::contentsMouseReleaseEvent(QMouseEvent* e)
{
  mMapcalc->contentsMouseReleaseEvent(e);
}

void QgsGrassMapcalcView::contentsMouseMoveEvent(QMouseEvent* e)
{
  mMapcalc->contentsMouseMoveEvent(e);
}

void QgsGrassMapcalcView::keyPressEvent ( QKeyEvent * e )
{
  mMapcalc->keyPressEvent ( e );
}
