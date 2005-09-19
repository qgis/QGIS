/***************************************************************************
                           qgscomposerpicture.cpp
                             -------------------
    begin                : September 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : radim.blazek@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <math.h>
#include <iostream>
#include <typeinfo>

#include <qwidget.h>
#include <qrect.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qdom.h>
#include <qcanvas.h>
#include <qpainter.h>
#include <qstring.h>
#include <qpixmap.h>
#include <qpicture.h>
#include <qimage.h>
#include <qlineedit.h>
#include <qpointarray.h>
#include <qpen.h>
#include <qrect.h>
#include <qlabel.h>
#include <qfiledialog.h>
#include <qmessagebox.h>

#include "qgsproject.h"
#include "qgscomposition.h"
#include "qgscomposerpicture.h"

#define PI 3.14159265358979323846

QgsComposerPicture::QgsComposerPicture ( QgsComposition *composition, 
					int id, QString file ) 
    : QCanvasPolygonalItem(0),
      mPicturePath ( file ), mPictureValid(false),
      mCX(-10), mCY(-10), mWidth(0), mHeight(0), mAngle(0),
      mBoundingRect(0,0,0,0), mAreaPoints(4),
      mFrame(false)
{
#ifdef QGISDEBUG
    std::cout << "QgsComposerPicture::QgsComposerPicture()" << std::endl;
#endif

    mComposition = composition;
    mId  = id;

    init();
    loadPicture();

    // Add to canvas
    setCanvas(mComposition->canvas());

    QCanvasPolygonalItem::show();
    QCanvasPolygonalItem::update();
     
    writeSettings();
}

QgsComposerPicture::QgsComposerPicture ( QgsComposition *composition, int id ) 
    : QCanvasPolygonalItem(0), mBoundingRect(0,0,0,0), mAreaPoints(4),
      mFrame(false)
{
#ifdef QGISDEBUG
    std::cout << "QgsComposerPicture::QgsComposerPicture()" << std::endl;
#endif

    mComposition = composition;
    mId  = id;

    init();

    readSettings();

    loadPicture();
    adjustPictureSize();

    // Add to canvas
    setCanvas(mComposition->canvas());

    recalculate();

    QCanvasPolygonalItem::show();
    QCanvasPolygonalItem::update();
}

void QgsComposerPicture::init ( void ) 
{
    mSelected = false;
    for ( int i = 0; i < 4; i++ ) 
    {
	mAreaPoints[i] = QPoint( 0, 0 );
    }

    // Rectangle
    QCanvasPolygonalItem::setZ(60);
    setActive(true);
}

void QgsComposerPicture::loadPicture ( void ) 
{
#ifdef QGISDEBUG
    std::cerr << "QgsComposerPicture::loadPicture() mPicturePath = " << mPicturePath.local8Bit() << std::endl;
#endif
    mPicture = QPicture(); 
    mPictureValid = false;

    if ( !mPicturePath.isNull() ) 
    {
	if ( mPicturePath.lower().right(3) == "svg" )
	{
	    if ( !mPicture.load ( mPicturePath, "svg" ) )
	    {
		std::cerr << "Cannot load svg" << std::endl;
	    }	
	    else
	    {
		mPictureValid = true;
	    }
	}
	else
	{
	    QImage image;
	    if ( !image.load(mPicturePath) )
	    {
		std::cerr << "Cannot load raster" << std::endl;
	    }
	    else
	    {	
		QPainter  p;
		p.begin( &mPicture );
		p.drawImage ( 0, 0, image ); 
		p.end();	
		mPictureValid = true;
	    }
	}
    }

    if ( !mPictureValid ) 
    {
        // Dummy picture
        QPainter  p;
	p.begin( &mPicture );
  	p.setPen( QPen(QColor(0,0,0), 1) );
	p.setBrush( QBrush( QColor( 150, 150, 150) ) );

        int w, h; 
        if ( mWidth > 0 && mHeight > 0 
             && mWidth/mHeight > 0.001 && mWidth/mHeight < 1000 ) 
	{
	    w = mWidth;
	    h = mHeight;
        }
	else
 	{
	    w = 100;
	    h = 100;
	}
	
	p.drawRect ( 0, 0, w, h ); 
	p.drawLine ( 0, 0, w-1, h-1 );
	p.drawLine ( w-1, 0, 0, h-1 );

	p.end();	

 	mPicture.setBoundingRect ( QRect ( 0, 0, w, h ) ); 
    }
}

bool QgsComposerPicture::pictureValid ( void )
{
    return mPictureValid;
}

QgsComposerPicture::~QgsComposerPicture()
{
#ifdef QGISDEBUG
    std::cerr << "QgsComposerPicture::~QgsComposerPicture()" << std::endl;
#endif
    QCanvasItem::hide();
}

void QgsComposerPicture::draw ( QPainter & painter )
{
#ifdef QGISDEBUG
    std::cerr << "QgsComposerPicture::draw()" << std::endl;
#endif

    QRect box = mPicture.boundingRect();
    double scale = 1. * mWidth / box.width(); 
    
    painter.save();

    painter.translate ( mX, mY );
    painter.scale ( scale, scale );
    painter.rotate ( -mAngle );
    
    painter.drawPicture ( -box.x(), -box.y(), mPicture );
    
    painter.restore();

    if ( mFrame ) {
	// TODO: rect is not correct, +/- 1 pixle - Qt3?
  	painter.setPen( QPen(QColor(0,0,0), 1) );
	painter.setBrush( QBrush( Qt::NoBrush ) );

	painter.save();
        painter.translate ( mX, mY );
        painter.rotate ( -mAngle );
      
	painter.drawRect ( 0, 0, mWidth, mHeight ); 
  	painter.restore();
    }

    // Show selected / Highlight
    if ( mSelected && plotStyle() == QgsComposition::Preview ) {
        painter.setPen( mComposition->selectionPen() );
        painter.setBrush( mComposition->selectionBrush() );
  
      	int s = mComposition->selectionBoxSize();

	for ( int i = 0; i < 4; i++ ) 
	{
	    painter.save();
	    painter.translate ( mAreaPoints.point(i).x(), mAreaPoints.point(i).y() );
	    painter.rotate ( -mAngle + i * 90 );
	  
	    painter.drawRect ( 0, 0, s, s );
	    painter.restore();
	}
    }
}

void QgsComposerPicture::drawShape( QPainter & painter )
{
#ifdef QGISDEBUG
    std::cout << "QgsComposerPicture::drawShape" << std::endl;
#endif
    draw ( painter );
}

void QgsComposerPicture::setBox ( int x1, int y1, int x2, int y2 )
{
    int tmp;

    if ( x1 > x2 ) { tmp = x1; x1 = x2; x2 = tmp; }
    if ( y1 > y2 ) { tmp = y1; y1 = y2; y2 = tmp; }
   
    // Center
    mCX = (x1 + x2) / 2;
    mCY = (y1 + y2) / 2;

    QRect box = mPicture.boundingRect();
    std::cout << "box.width() = " << box.width() << " box.height() = " << box.height() << std::endl;

    mWidth = x2-x1;
    mHeight = y2-y1;
    adjustPictureSize(); 

    recalculate();
}

void QgsComposerPicture::moveBy( double x, double y )
{
#ifdef QGISDEBUG
    std::cout << "QgsComposerPicture::moveBy()" << std::endl;
#endif

    mCX += (int) x; 
    mCY += (int) y; 
    recalculate();
}

void QgsComposerPicture::recalculate()
{
#ifdef QGISDEBUG
    std::cout << "QgsComposerPicture::recalculate" << std::endl;
#endif
    
    QCanvasPolygonalItem::invalidate();

    QRect box = mPicture.boundingRect();

    double angle = PI * mAngle / 180;
    
    // Angle between vertical in picture space and the vector 
    // from center to upper left corner of the picture
    double anglePicture = atan2 ( box.width(), box.height() );

    // Angle (clockwise) between horizontal in paper space
    // and the vector from center to upper left corner of the picture
    double anglePaper = PI / 2 - anglePicture - angle;

    // Distance from center to upper left corner in canvas units
    double r = sqrt ( mWidth*mWidth/4 + mHeight*mHeight/4 );

    // Position of upper left corner in map
    int dx = (int) ( r * cos ( anglePaper ) );
    int dy = (int) ( r * sin ( anglePaper ) );

    mX = mCX - dx;
    mY = mCY - dy;
    
    // Area points
    mAreaPoints[0] = QPoint( mCX-dx, mCY-dy );
    mAreaPoints[2] = QPoint( mCX+dx, mCY+dy );

    anglePaper = angle + PI / 2 - anglePicture;
    dx = (int) ( r * cos ( anglePaper ) );
    dy = (int) ( r * sin ( anglePaper ) );
    mAreaPoints[1] = QPoint( mCX+dx, mCY-dy );
    mAreaPoints[3] = QPoint( mCX-dx, mCY+dy );

    mBoundingRect = mAreaPoints.boundingRect();
    
    QCanvasPolygonalItem::canvas()->setChanged(mBoundingRect);
    QCanvasPolygonalItem::update();
    QCanvasPolygonalItem::canvas()->update();
}

QRect QgsComposerPicture::boundingRect ( void ) const
{
#ifdef QGISDEBUG
    std::cout << "QgsComposerPicture::boundingRect" << std::endl;
#endif
    return mBoundingRect;
}

QPointArray QgsComposerPicture::areaPoints() const
{
#ifdef QGISDEBUG
    std::cout << "QgsComposerPicture::areaPoints" << std::endl;
#endif

    return mAreaPoints;
}


void QgsComposerPicture::frameChanged ( )
{
    mFrame = mFrameCheckBox->isChecked();

    QCanvasPolygonalItem::update();
    QCanvasPolygonalItem::canvas()->update();

    writeSettings();
}

void QgsComposerPicture::angleChanged ( )
{
#ifdef QGISDEBUG
    std::cout << "QgsComposerPicture::angleChanged()" << std::endl;
#endif

    mAngle = mAngleLineEdit->text().toDouble();

    recalculate();

    writeSettings();
}

void QgsComposerPicture::widthChanged ( )
{
#ifdef QGISDEBUG
    std::cout << "QgsComposerPicture::widthChanged()" << std::endl;
#endif

    mWidth = mComposition->fromMM ( mWidthLineEdit->text().toDouble() );

    QRect box = mPicture.boundingRect();
    mHeight = mWidth*box.height()/box.width();
    setOptions();

    recalculate();

    writeSettings();
}

void QgsComposerPicture::browsePicture ( )
{
#ifdef QGISDEBUG
    std::cout << "QgsComposerPicture::browsePicture()" << std::endl;
#endif
 
    QString file = QgsComposerPicture::pictureDialog();

    if ( file.isNull() ) return;
    
    mPicturePath = file;
    mPictureLineEdit->setText ( mPicturePath );

    pictureChanged();
}

void QgsComposerPicture::pictureChanged ( )
{
#ifdef QGISDEBUG
    std::cout << "QgsComposerPicture::pictureChanged()" << std::endl;
#endif

    mPicturePath = mPictureLineEdit->text();

    int w = mWidth;
    int h = mHeight;

    loadPicture();

    if ( !mPictureValid ) {
        QMessageBox::warning( 0, "Warning",
                        "Cannot load picture." );
    }
    else
    {
        adjustPictureSize();
        setOptions();
        recalculate();
    }
}

void QgsComposerPicture::adjustPictureSize ( )
{
    // Addjust to original size
    QRect box = mPicture.boundingRect();

    if ( box.width() == 0 || box.height() == 0
	 || mWidth == 0 || mHeight == 0 )
    {
	mWidth = 0;
	mHeight = 0;	
        return;
    }

    if ( 1.*box.width()/box.height() > 1.*mWidth/mHeight )
    {
	mHeight = mWidth*box.height()/box.width();
    }
    else
    {
	mWidth = mHeight*box.width()/box.height();
    }
}

void QgsComposerPicture::setOptions ( void )
{ 
    mPictureLineEdit->setText ( mPicturePath );
    mWidthLineEdit->setText ( QString("%1").arg( mComposition->toMM(mWidth), 0,'g') );
    mHeightLineEdit->setText ( QString("%1").arg( mComposition->toMM(mHeight), 0,'g') );
    mAngleLineEdit->setText ( QString::number ( mAngle ) );
    mFrameCheckBox->setChecked ( mFrame );
}

void QgsComposerPicture::setSelected (  bool s ) 
{
    mSelected = s;
    QCanvasPolygonalItem::update(); // show highlight
}    

bool QgsComposerPicture::selected( void )
{
    return mSelected;
}

QWidget *QgsComposerPicture::options ( void )
{
    setOptions ();
    return ( dynamic_cast <QWidget *> (this) ); 
}

QString QgsComposerPicture::pictureDialog ( void )
{
    QString filters = "Pictures ( *.svg *.SVG ";
    QStrList formats = QImageIO::outputFormats();

    for ( int i = 0; i < formats.count(); i++ )
    {
        QString frmt = QImageIO::outputFormats().at( i );
        QString fltr = " *." + frmt.lower() + " *." + frmt.upper();
        filters += fltr;
    }
    filters += " )";

    QString file = QFileDialog::getOpenFileName(
                    ".",
                    filters,
                    0,
                    "open picture dialog",
                    "Choose a file" );

    return file; 
}

bool QgsComposerPicture::writeSettings ( void )  
{
#ifdef QGISDEBUG
    std::cout << "QgsComposerPicture::writeSettings" << std::endl;
#endif

    QString path;
    path.sprintf("/composition_%d/picture_%d/", mComposition->id(), mId ); 

    QgsProject::instance()->writeEntry( "Compositions", path+"picture", mPicturePath );

    QgsProject::instance()->writeEntry( "Compositions", path+"x", mComposition->toMM(mCX) );
    QgsProject::instance()->writeEntry( "Compositions", path+"y", mComposition->toMM(mCY) );
    QgsProject::instance()->writeEntry( "Compositions", path+"width", mComposition->toMM(mWidth) );
    QgsProject::instance()->writeEntry( "Compositions", path+"height", mComposition->toMM(mHeight) );

    QgsProject::instance()->writeEntry( "Compositions", path+"angle", mAngle );

    QgsProject::instance()->writeEntry( "Compositions", path+"frame", mFrame );

    return true; 
}

bool QgsComposerPicture::readSettings ( void )
{
    bool ok;
    QString path;
    path.sprintf("/composition_%d/picture_%d/", mComposition->id(), mId );

    mPicturePath = QgsProject::instance()->readEntry( "Compositions", path+"picture", "", &ok) ;

    mCX = mComposition->fromMM(QgsProject::instance()->readDoubleEntry( "Compositions", path+"x", 0, &ok));
    mCY = mComposition->fromMM(QgsProject::instance()->readDoubleEntry( "Compositions", path+"y", 0, &ok));
    mWidth = mComposition->fromMM(QgsProject::instance()->readDoubleEntry( "Compositions", path+"width", 0, &ok));
    mHeight = mComposition->fromMM(QgsProject::instance()->readDoubleEntry( "Compositions", path+"height", 0, &ok));

    mAngle = mComposition->fromMM(QgsProject::instance()->readDoubleEntry( "Compositions", path+"angle", 0, &ok));

    mFrame = QgsProject::instance()->readBoolEntry("Compositions", path+"frame", true, &ok);

    return true;
}

bool QgsComposerPicture::removeSettings( void )
{
#ifdef QGISDEBUG
    std::cerr << "QgsComposerPicture::deleteSettings" << std::endl;
#endif

    QString path;
    path.sprintf("/composition_%d/picture_%d", mComposition->id(), mId ); 
    return QgsProject::instance()->removeEntry ( "Compositions", path );
}

bool QgsComposerPicture::writeXML( QDomNode & node, QDomDocument & document, bool temp )
{
    return true;
}

bool QgsComposerPicture::readXML( QDomNode & node )
{
    return true;
}

