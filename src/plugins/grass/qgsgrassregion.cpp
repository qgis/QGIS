/***************************************************************************
    qgsgrassregion.h  -  Edit region 
                             -------------------
    begin                : August, 2004
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
#include <iostream>
#include <qdir.h>
#include <qfile.h>
#include <qsettings.h>
#include <qpixmap.h>
#include <q3listbox.h>
#include <qstringlist.h>
#include <qlabel.h>
#include <QComboBox>
#include <qspinbox.h>
#include <qmessagebox.h>
#include <qinputdialog.h>
#include <qpainter.h>
#include <qpen.h>
#include <qpoint.h>
#include <qcursor.h>
#include <qnamespace.h>
#include <qsettings.h>
#include <qvalidator.h>
#include <q3button.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <q3buttongroup.h>
#include <qcolordialog.h>
#include <qspinbox.h>
#include <qglobal.h>

#include <QMouseEvent>
#include <QRubberBand>

#include <qgsrasterlayer.h>
#include "qgis.h"
#include "qgsmaplayer.h"
#include "qgsvectorlayer.h"
#include "qgisinterface.h"
#include "qgsmapcanvas.h"
#include "qgsmaptool.h"
#include "qgsmaptopixel.h"
#include "qgspoint.h"

extern "C" {
#include <grass/gis.h>
}

#include "qgsgrass.h"
#include "qgsgrassplugin.h"
#include "qgsgrassregion.h"

/** map tool which uses rubber band for changing grass region */
class QgsGrassRegionEdit : public QgsMapTool
{
public:
  QgsGrassRegionEdit(QgsGrassRegion* reg)
    : QgsMapTool(reg->mCanvas), mRegion(reg)
  {
    mDraw = false;
    mRubberBand = new QRubberBand ( QRubberBand::Rectangle, mCanvas );
  }

  ~QgsGrassRegionEdit()
  {
    delete mRubberBand;
  }

  //! mouse click in map canvas
  void canvasPressEvent(QMouseEvent * event)
  {
    QgsPoint point = toMapCoords(event->pos());
    double x = point.x();
    double y = point.y();

#ifdef QGISDEBUG
    std::cerr << "QgsGrassRegionEdit::canvasPressEvent()" << std::endl;
#endif

    if ( !mDraw ) { // first corner
      mRegion->mX = x;
      mRegion->mY = y;

      mRegion->draw ( x, y, x, y );
      mDraw = true; 
    } else { 
      mRegion->draw ( mRegion->mX, mRegion->mY, x, y );
      mDraw = false; 
    }
    mRubberBand->show();
  }

  //! mouse movement in map canvas
  void canvasMoveEvent(QMouseEvent * event)
  {
    QgsPoint point = toMapCoords(event->pos());

#ifdef QGISDEBUG
    std::cerr << "QgsGrassRegionEdit::canvasMoveEvent()" << std::endl;
#endif

    if ( !mDraw ) return;
    mRegion->draw ( mRegion->mX, mRegion->mY, point.x(), point.y() );
  }

  //! called when map tool is about to get inactive
  void deactivate()
  { 
    mRubberBand->hide();

    QgsMapTool::deactivate();
  }

  void setRegion(const QgsPoint& ul, const QgsPoint& lr)
  {
    QPoint qul = toCanvasCoords(ul);
    QPoint qlr = toCanvasCoords(lr);    
    mRubberBand->setGeometry ( QRect(qul,qlr));
  }


private:
  //! Rubber band for selecting grass region
  QRubberBand* mRubberBand;

  //! Status of input from canvas 
  bool mDraw;

  QgsGrassRegion* mRegion;
};


QgsGrassRegion::QgsGrassRegion ( QgsGrassPlugin *plugin,  QgisInterface *iface,
        QWidget * parent, Qt::WFlags f ) 
        :QDialog(parent, f), QgsGrassRegionBase ( )
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassRegion()" << std::endl;
#endif

  setupUi(this);

  mPlugin = plugin;
  mInterface = iface;
  mCanvas = mInterface->getMapCanvas();
  restorePosition();
  mUpdatingGui = false;
  mDisplayed = false;

  // Set input validators
  QDoubleValidator *dv = new QDoubleValidator(0);
  QIntValidator *iv = new QIntValidator(0);

  mNorth->setValidator ( dv );
  mSouth->setValidator ( dv );
  mEast->setValidator ( dv );
  mWest->setValidator ( dv );
  mNSRes->setValidator ( dv );
  mEWRes->setValidator ( dv );
  mRows->setValidator ( iv );
  mCols->setValidator ( iv );

  // Group radio buttons
  mNSRadioGroup = new Q3ButtonGroup();
  mEWRadioGroup = new Q3ButtonGroup();
  mNSRadioGroup->insert ( mNSResRadio );
  mNSRadioGroup->insert ( mRowsRadio );
  mEWRadioGroup->insert ( mEWResRadio );
  mEWRadioGroup->insert ( mColsRadio );
  mNSResRadio->setChecked ( true );
  mEWResRadio->setChecked ( true );
  mRows->setEnabled(false);
  mCols->setEnabled(false);
  connect( mNSRadioGroup, SIGNAL(clicked(int)), this, SLOT(radioChanged()));
  connect( mEWRadioGroup, SIGNAL(clicked(int)), this, SLOT(radioChanged()));

  // Set values to current region
  QString gisdbase = QgsGrass::getDefaultGisdbase();
  QString location = QgsGrass::getDefaultLocation();
  QString mapset   = QgsGrass::getDefaultMapset();

  if ( gisdbase.isEmpty() || location.isEmpty() || mapset.isEmpty() ) {
    QMessageBox::warning( 0, tr("Warning"), tr("GISDBASE, LOCATION_NAME or MAPSET is not set, "
      "cannot display current region." ) );
  }

  QgsGrass::setLocation ( gisdbase, location );
  char *err = G__get_window ( &mWindow, (char *)"", (char *)"WIND", (char *) mapset.latin1() );

  if ( err ) {
    QMessageBox::warning( 0, tr("Warning"), tr("Cannot read current region: ") + QString(err) );
    return;
  }

  setGuiValues();

  connect( mCanvas, SIGNAL(renderComplete(QPainter *)), this, SLOT(postRender(QPainter *)));

  // Connect entries
  connect( mNorth, SIGNAL(textChanged(const QString &)), this, SLOT(northChanged(const QString &)));
  connect( mSouth, SIGNAL(textChanged(const QString &)), this, SLOT(southChanged(const QString &)));
  connect( mEast, SIGNAL(textChanged(const QString &)), this, SLOT(eastChanged(const QString &)));
  connect( mWest, SIGNAL(textChanged(const QString &)), this, SLOT(westChanged(const QString &)));
  connect( mNSRes, SIGNAL(textChanged(const QString &)), this, SLOT(NSResChanged(const QString &)));
  connect( mEWRes, SIGNAL(textChanged(const QString &)), this, SLOT(EWResChanged(const QString &)));
  connect( mRows, SIGNAL(textChanged(const QString &)), this, SLOT(rowsChanged(const QString &)));
  connect( mCols, SIGNAL(textChanged(const QString &)), this, SLOT(colsChanged(const QString &)));

  // Symbology
  QPen pen = mPlugin->regionPen();
  mColorButton->setColor( pen.color() );
  connect( mColorButton, SIGNAL(clicked()), this, SLOT(changeColor()));

  mWidthSpinBox->setValue( pen.width() );
  connect( mWidthSpinBox, SIGNAL(valueChanged(int)), this, SLOT(changeWidth()));

  mRegionEdit = new QgsGrassRegionEdit(this); // will be deleted by map canvas
  mCanvas->setMapTool(mRegionEdit);

  setAttribute ( Qt::WA_DeleteOnClose );
  displayRegion();
}

void QgsGrassRegion::changeColor ( void )
{
  QPen pen = mPlugin->regionPen();
  QColor color = QColorDialog::getColor ( pen.color(), this );
  if (color.isValid())
  {
    mColorButton->setColor( color );

    pen.setColor(color);
    mPlugin->setRegionPen(pen);
  }
}

void QgsGrassRegion::changeWidth ( void )
{
  QPen pen = mPlugin->regionPen();

  pen.setWidth( mWidthSpinBox->value() );
  mPlugin->setRegionPen(pen);
}

QString QgsGrassRegion::formatEdge ( double v )
{
  // Not sure about formating 
  if ( v > 999999 ) 
  {
    return  QString("%1").arg( v, 0, 'f', 0); // to avoid e format for large numbers
  } 
  return QString("%1").arg(v, 0, 'g');
}

void QgsGrassRegion::setGuiValues( bool north, bool south, bool east, bool west,
	                           bool nsres, bool ewres, bool rows, bool cols )
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassRegion::setGuiValues()" << std::endl;
#endif

  mUpdatingGui = true;

  if ( north ) mNorth->setText ( QString("%1").arg(mWindow.north, 0, 'g', 15) );
  if ( south ) mSouth->setText ( QString("%1").arg(mWindow.south, 0, 'g', 15) );
  if ( east )  mEast->setText  ( QString("%1").arg(mWindow.east, 0, 'g', 15) );
  if ( west )  mWest->setText  ( QString("%1").arg(mWindow.west, 0, 'g', 15) );
  if ( nsres ) mNSRes->setText ( QString("%1").arg(mWindow.ns_res,0,'g') );
  if ( ewres ) mEWRes->setText ( QString("%1").arg(mWindow.ew_res,0,'g') );
  if ( rows )  mRows->setText  ( QString("%1").arg(mWindow.rows) );
  if ( cols )  mCols->setText  ( QString("%1").arg(mWindow.cols) );

  mUpdatingGui = false;
}

QgsGrassRegion::~QgsGrassRegion ()
{
  delete mRegionEdit;
}

void QgsGrassRegion::northChanged(const QString &str)
{
  if ( mUpdatingGui ) return;
  mWindow.north = mNorth->text().toDouble();
  adjust();
  setGuiValues ( false );
  displayRegion();
}

void QgsGrassRegion::southChanged(const QString &str)
{
  if ( mUpdatingGui ) return;
  mWindow.south = mSouth->text().toDouble();
  adjust();
  setGuiValues ( true, false );
  displayRegion();
}

void QgsGrassRegion::eastChanged(const QString &str)
{
  if ( mUpdatingGui ) return;
  mWindow.east = mEast->text().toDouble();
  adjust();
  setGuiValues ( true, true, false );
  displayRegion();
}

void QgsGrassRegion::westChanged(const QString &str)
{
  if ( mUpdatingGui ) return;
  mWindow.west = mWest->text().toDouble();
  adjust();
  setGuiValues ( true, true, true, false );
  displayRegion();
}

void QgsGrassRegion::NSResChanged(const QString &str)
{
  if ( mUpdatingGui ) return;
  mWindow.ns_res = mNSRes->text().toDouble();
  adjust();
  setGuiValues ( true, true, true, true, false );
  displayRegion();
}

void QgsGrassRegion::EWResChanged(const QString &str)
{
  if ( mUpdatingGui ) return;
  mWindow.ew_res = mEWRes->text().toDouble();
  adjust();
  setGuiValues ( true, true, true, true, true, false );
  displayRegion();
}

void QgsGrassRegion::rowsChanged(const QString &str)
{
  if ( mUpdatingGui ) return;
  mWindow.rows = mRows->text().toInt();
  adjust();
  setGuiValues ( true, true, true, true, true, true, false );
  displayRegion();
}

void QgsGrassRegion::colsChanged(const QString &str)
{
  if ( mUpdatingGui ) return;
  mWindow.cols = mCols->text().toInt();
  adjust();
  setGuiValues ( true, true, true, true, true, true, true, false );
  displayRegion();
}

void QgsGrassRegion::adjust()
{
  int r, c;
  if ( mRowsRadio->isChecked() ) r = 1; else r = 0;
  if ( mColsRadio->isChecked() ) c = 1; else c = 0;
  G_adjust_Cell_head ( &mWindow, r, c );
}

void QgsGrassRegion::radioChanged()
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassRegion::radioChanged()" << std::endl;
#endif

  if ( mRowsRadio->isChecked() ) {
    mNSRes->setEnabled(false);
    mRows->setEnabled(true);
  } else { 
    mNSRes->setEnabled(true);
    mRows->setEnabled(false);
  }
  if ( mColsRadio->isChecked() ) {
    mEWRes->setEnabled(false);
    mCols->setEnabled(true);
  } else { 
    mEWRes->setEnabled(true);
    mCols->setEnabled(false);
  }
}

void QgsGrassRegion::draw ( double x1, double y1, double x2, double y2 ) 
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassRegion::draw()" << std::endl;
#endif

  if ( x1 < x2 ) {
    mWindow.west = x1;
    mWindow.east = x2; 
  } else {
    mWindow.west = x2; 
    mWindow.east = x1;
  }
  if ( y1 < y2 ) {
    mWindow.south = y1;
    mWindow.north = y2; 
  } else {
    mWindow.south = y2; 
    mWindow.north = y1;
  }

  adjust();
  setGuiValues();
  displayRegion();
}

void QgsGrassRegion::displayRegion()
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassRegion::displayRegion()" << std::endl;
#endif

  QgsPoint ul(mWindow.west, mWindow.north);
  QgsPoint lr(mWindow.east, mWindow.south);

  mRegionEdit->setRegion(ul, lr);

  mDisplayed = true;
}

void QgsGrassRegion::postRender(QPainter *painter)
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassRegion::postRender" << std::endl;
#endif

  mDisplayed = false;
  displayRegion();
}

void QgsGrassRegion::accept()
{
  // TODO: better repaint region
  QSettings settings;

  bool on = settings.readBoolEntry ("/GRASS/region/on", true );

  if ( on ) {
    mPlugin->switchRegion(false); // delete
  }

  QgsGrass::setLocation ( QgsGrass::getDefaultGisdbase(), QgsGrass::getDefaultLocation() );
  G__setenv( (char *)"MAPSET", (char *) QgsGrass::getDefaultMapset().latin1() );

  if ( G_put_window(&mWindow) == -1 ) {
    QMessageBox::warning( 0, tr("Warning"), tr("Cannot write region") );
    return;
  }

  if ( on ) {
    mPlugin->switchRegion(on);  // draw new
  }

  saveWindowLocation();
  mCanvas->setMapTool(NULL);
  delete this;
}

void QgsGrassRegion::reject()
{
  saveWindowLocation();
  mCanvas->setMapTool(NULL);
  delete this;
}

void QgsGrassRegion::restorePosition()
{
  QSettings settings;
  restoreGeometry(settings.value("/GRASS/windows/region/geometry").toByteArray());
}

void QgsGrassRegion::saveWindowLocation()
{
  QSettings settings;
  settings.setValue("/GRASS/windows/region/geometry", saveGeometry());
} 
