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
#include <qfiledialog.h> 
#include <qsettings.h>
#include <qpixmap.h>
#include <qlistbox.h>
#include <qstringlist.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qmessagebox.h>
#include <qinputdialog.h>
#include <qpainter.h>
#include <qpen.h>
#include <qpointarray.h>
#include <qcursor.h>
#include <qnamespace.h>
#include <qsettings.h>
#include <qvalidator.h>
#include <qbutton.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qpalette.h>
#include <qcolordialog.h>
#include <qspinbox.h>

#include "../../src/qgis.h"
#include "../../src/qgisapp.h"
#include "../../src/qgsmaplayer.h"
#include "../../src/qgsrasterlayer.h"
#include "../../src/qgsvectorlayer.h"
#include "../../src/qgisiface.h"
#include "../../src/qgsmapcanvas.h"
#include "../../src/qgscoordinatetransform.h"
#include "../../src/qgspoint.h"

extern "C" {
#include <gis.h>
}

#include "../../providers/grass/qgsgrass.h"
#include "qgsgrassplugin.h"
#include "qgsgrassregion.h"

bool QgsGrassRegion::mRunning = false;

QgsGrassRegion::QgsGrassRegion ( QgsGrassPlugin *plugin,  QgisApp *qgisApp, QgisIface *interface,
        QWidget * parent, const char * name, WFlags f ) :QgsGrassRegionBase ( parent, name, f )
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassRegion()" << std::endl;
    #endif

    mRunning = true;
    mPlugin = plugin;
    mQgisApp = qgisApp;
    mInterface = interface;
    mCanvas = mInterface->getMapCanvas();
    restorePosition();
    mDraw = false;
    mUpdatingGui = false;
    mDisplayed = false;
    mPointArray.resize(5);

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
    mNSRadioGroup = new QButtonGroup();
    mEWRadioGroup = new QButtonGroup();
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
	QMessageBox::warning( 0, "Warning", "GISDBASE, LOCATION_NAME or MAPSET is not set, "
		                 "cannot display current region." );
    }

    QgsGrass::setLocation ( gisdbase, location );
    char *err = G__get_window ( &mWindow, "", "WIND", (char *) mapset.latin1() );

    if ( err ) {
	QMessageBox::warning( 0, "Warning", "Cannot read current region: " + QString(err) );
	return;
    }
	
    setGuiValues();

    connect( mCanvas, SIGNAL(xyClickCoordinates(QgsPoint &)), this, SLOT(mouseEventReceiverClick(QgsPoint &)));
    connect( mCanvas, SIGNAL(xyCoordinates(QgsPoint &)), this, SLOT(mouseEventReceiverMove(QgsPoint &)));
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

    mCanvas->setMapTool ( QGis::CapturePoint );
    mCanvas->setCursor (  Qt::CrossCursor );

    // Symbology
    QPen pen = mPlugin->regionPen();
    QPalette palette = mColorButton->palette();
    palette.setColor( QColorGroup::Button, pen.color() );
    mColorButton->setPalette( palette );
    connect( mColorButton, SIGNAL(clicked()), this, SLOT(changeColor()));

    mWidthSpinBox->setValue( pen.width() );
    connect( mWidthSpinBox, SIGNAL(valueChanged(int)), this, SLOT(changeWidth()));

    displayRegion();
}

void QgsGrassRegion::changeColor ( void ) {
    QPen pen = mPlugin->regionPen();
    QColor color = QColorDialog::getColor ( pen.color(), this );

    QPalette palette = mColorButton->palette();
    palette.setColor( QColorGroup::Button, pen.color() );
    mColorButton->setPalette( palette );

    pen.setColor(color);
    mPlugin->setRegionPen(pen);
}

void QgsGrassRegion::changeWidth ( void ) {
    QPen pen = mPlugin->regionPen();

    pen.setWidth( mWidthSpinBox->value() );
    mPlugin->setRegionPen(pen);
}

void QgsGrassRegion::setGuiValues( bool north, bool south, bool east, bool west,
	                           bool nsres, bool ewres, bool rows, bool cols )
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassRegion::setGuiValues()" << std::endl;
    #endif
    
    mUpdatingGui = true;
    
    if ( north ) mNorth->setText ( QString("%1").arg(mWindow.north, 0, 'f') );
    if ( south ) mSouth->setText ( QString("%1").arg(mWindow.south, 0, 'f') );
    if ( east )  mEast->setText  ( QString("%1").arg(mWindow.east, 0, 'f') );
    if ( west )  mWest->setText  ( QString("%1").arg(mWindow.west, 0, 'f') );
    if ( nsres ) mNSRes->setText ( QString("%1").arg(mWindow.ns_res,0,'g') );
    if ( ewres ) mEWRes->setText ( QString("%1").arg(mWindow.ew_res,0,'g') );
    if ( rows )  mRows->setText  ( QString("%1").arg(mWindow.rows) );
    if ( cols )  mCols->setText  ( QString("%1").arg(mWindow.cols) );

    mUpdatingGui = false;
}

QgsGrassRegion::~QgsGrassRegion ()
{
    mRunning = false;
}

bool QgsGrassRegion::isRunning(void)
{
    return mRunning;
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
    if ( mRowsRadio->state() == QButton::On ) r = 1; else r = 0;
    if ( mColsRadio->state() == QButton::On ) c = 1; else c = 0;
    G_adjust_Cell_head ( &mWindow, r, c );
}

void QgsGrassRegion::radioChanged()
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassRegion::radioChanged()" << std::endl;
    #endif
    
    if ( mRowsRadio->state() == QButton::On ) {
        mNSRes->setEnabled(false);
        mRows->setEnabled(true);
    } else { 
        mNSRes->setEnabled(true);
        mRows->setEnabled(false);
    }
    if ( mColsRadio->state() == QButton::On ) {
        mEWRes->setEnabled(false);
        mCols->setEnabled(true);
    } else { 
        mEWRes->setEnabled(true);
        mCols->setEnabled(false);
    }
}
void QgsGrassRegion::mouseEventReceiverClick( QgsPoint & point )
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassRegion::mouseEventReceiverClick()" << std::endl;
    #endif

    if ( !mDraw ) { // first corner
        mX = point.x();
	mY = point.y();
        
	draw ( mX, mY, mX, mY );
	mDraw = true; 
    } else { 
        draw ( mX, mY, point.x(), point.y() );
	mDraw = false; 
    }
}

void QgsGrassRegion::mouseEventReceiverMove( QgsPoint & point )
{
    if ( !mDraw ) return;
    draw ( mX, mY, point.x(), point.y() );
}

void QgsGrassRegion::draw ( double x1, double y1, double x2, double y2 ) 
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassRegion::draw()" << std::endl;
    #endif

    double n, s, e, w;

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

    QPainter *painter = new QPainter();
    QPixmap *pixmap = mCanvas->canvasPixmap();
    painter->begin(pixmap);
    painter->setRasterOp(Qt::XorROP);
    painter->setPen ( QColor(125,125,125) );

    if ( mDisplayed ) { // delete old
        painter->drawPolyline ( mPointArray );
    }

    std::vector<QgsPoint> points;
    points.resize(5);
    
    points[0].setX(mWindow.west); points[0].setY(mWindow.south);
    points[1].setX(mWindow.east); points[1].setY(mWindow.south);
    points[2].setX(mWindow.east); points[2].setY(mWindow.north);
    points[3].setX(mWindow.west); points[3].setY(mWindow.north);
    points[4].setX(mWindow.west); points[4].setY(mWindow.south);
    
    QgsCoordinateTransform *transform = mCanvas->getCoordinateTransform();

    for ( int i = 0; i < 5; i++ ) {
        transform->transform( &(points[i]) );
        mPointArray.setPoint( i, points[i].xToInt(), points[i].yToInt() );
    }
    
    painter->drawPolyline ( mPointArray );

    painter->end();
    mCanvas->repaint(false);
    delete painter;

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
    QgsGrass::setLocation ( QgsGrass::getDefaultGisdbase(), QgsGrass::getDefaultLocation() );
    G__setenv( "MAPSET", (char *) QgsGrass::getDefaultMapset().latin1() );
    
    if ( G_put_window(&mWindow) == -1 ) {
	QMessageBox::warning( 0, "Warning", "Cannot write region" );
        return;
    }

    saveWindowLocation();
    close();
    delete this;
}

void QgsGrassRegion::reject()
{
    saveWindowLocation();
    close();
    delete this;
}

void QgsGrassRegion::restorePosition()
{
  QSettings settings;
  int ww = settings.readNumEntry("/qgis/grass/windows/region/w", 250);
  int wh = settings.readNumEntry("/qgis/grass/windows/region/h", 350);
  int wx = settings.readNumEntry("/qgis/grass/windows/region/x", 100);
  int wy = settings.readNumEntry("/qgis/grass/windows/region/y", 100);
  resize(ww,wh);
  move(wx,wy);
}

void QgsGrassRegion::saveWindowLocation()
{
  QSettings settings;
  QPoint p = this->pos();
  QSize s = this->size();
  settings.writeEntry("/qgis/grass/windows/region/x", p.x());
  settings.writeEntry("/qgis/grass/windows/region/y", p.y());
  settings.writeEntry("/qgis/grass/windows/region/w", s.width());
  settings.writeEntry("/qgis/grass/windows/region/h", s.height());
} 

