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
/* $Id$ */

#include "qgscomposerpicture.h"
#include "qgsproject.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <QImageWriter>
#include <QSettings>
#include <QPolygonF>
#include <QAbstractGraphicsShapeItem>
#include <QGraphicsScene>

#include <cmath>
#include <iostream>

#define PI 3.14159265358979323846

QgsComposerPicture::QgsComposerPicture ( QgsComposition *composition, 
					int id, QString file ) 
    : QWidget(composition),
      QAbstractGraphicsShapeItem(0),
      mPicturePath ( file ),
      mPictureValid(false),
      mCX(-10), mCY(-10),
      mWidth(0), mHeight(0), mAngle(0),
      mFrame(false),
      mAreaPoints(4),
      mBoundingRect(0,0,0,0)
{
    setupUi(this);

#ifdef QGISDEBUG
    std::cout << "QgsComposerPicture::QgsComposerPicture()" << std::endl;
#endif

    mComposition = composition;
    mId  = id;

    init();
    loadPicture();

    // Add to scene
    mComposition->canvas()->addItem(this);

    QAbstractGraphicsShapeItem::show();
    QAbstractGraphicsShapeItem::update();
     
    writeSettings();
}

QgsComposerPicture::QgsComposerPicture ( QgsComposition *composition, int id ) :
    QWidget(),
    QAbstractGraphicsShapeItem(0),
    mFrame(false),
    mAreaPoints(4),
    mBoundingRect(0,0,0,0)
{
#ifdef QGISDEBUG
    std::cout << "QgsComposerPicture::QgsComposerPicture()" << std::endl;
#endif

    setupUi(this);

    mComposition = composition;
    mId  = id;

    init();

    readSettings();

    loadPicture();
    adjustPictureSize();

    // Add to scene
    mComposition->canvas()->addItem(this);

    recalculate();

    QAbstractGraphicsShapeItem::show();
    QAbstractGraphicsShapeItem::update();
}

void QgsComposerPicture::init ( void ) 
{
  mSelected = false;
  for ( int i = 0; i < 4; i++ ) 
  {
    mAreaPoints[i] = QPoint( 0, 0 );
  }

  QAbstractGraphicsShapeItem::setZValue(60);

}

void QgsComposerPicture::loadPicture ( void ) 
{
#ifdef QGISDEBUG
    std::cerr << "QgsComposerPicture::loadPicture() mPicturePath = " << mPicturePath.toLocal8Bit().data() << std::endl;
#endif

  mPicture = QPicture(); 
  mPictureValid = false;

  if ( !mPicturePath.isNull() ) 
  {
	if ( mPicturePath.toLower().right(3) == "svg" )
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
    QPen pen(QColor(0,0,0));
    pen.setWidthF(3.0);
    p.setPen( pen );
    p.setBrush( QBrush( QColor( 150, 150, 150) ) );

    double w, h; 
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
	
	p.drawRect (QRectF(0, 0, w, h)); 
	p.drawLine (QLineF(0, 0, w-1, h-1));
	p.drawLine (QLineF(w-1, 0, 0, h-1));

	p.end();	

 	mPicture.setBoundingRect ( QRect( 0, 0, (int)w, (int)h ) ); 
  }//END if(!mPictureValid)
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
    QGraphicsItem::hide();
}

void QgsComposerPicture::paint ( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget )
{
#ifdef QGISDEBUG
    std::cerr << "QgsComposerPicture::paint()" << std::endl;
#endif

    QRectF box = mPicture.boundingRect();
    double scale = 1. * mWidth / box.width();

    if(plotStyle() == QgsComposition::Postscript)
    {
      scale *= (96.0 / mComposition->resolution());
    }
    painter->save();

    painter->translate(-mWidth/2, -mHeight/2);

    painter->scale ( scale, scale );
    painter->rotate ( -mAngle );
    
//    painter->drawPicture (QPointF(-box.width()/2, -box.height()/2), mPicture ); //this doesn't work right...

    painter->drawPicture (0, 0, mPicture );

    painter->restore();

    if ( mFrame ) {
	  // TODO: rect is not correct, +/- 1 pixel - Qt3?
  	  painter->setPen( QPen(QColor(0,0,0), .3) );
	  painter->setBrush( QBrush( Qt::NoBrush ) );

	  painter->save();
      painter->rotate ( -mAngle );
      
      painter->drawRect (QRectF(-mWidth/2, -mHeight/2, mWidth, mHeight));

  	  painter->restore();
    }

    // Show selected / Highlight
    if ( mSelected && plotStyle() == QgsComposition::Preview ) {
      painter->setPen( mComposition->selectionPen() );
      painter->setBrush( mComposition->selectionBrush() );
  
      double s = mComposition->selectionBoxSize();

	  for ( int i = 0; i < 4; i++ ) 
	  {
	    painter->save();
	    painter->translate ( mAreaPoints[i].x(), mAreaPoints[i].y() );
	    painter->rotate ( -mAngle + i * 90 );
	    painter->drawRect(QRectF(0, 0, s, s));
	    painter->restore();
	  }
    }//END of drawing selected highlight

}

void QgsComposerPicture::setSize(double width, double height )
{
    mWidth = width;
    mHeight = height;
    adjustPictureSize(); 

    recalculate();
}

void QgsComposerPicture::recalculate()
{
#ifdef QGISDEBUG
    std::cout << "QgsComposerPicture::recalculate" << std::endl;
#endif
    
    QAbstractGraphicsShapeItem::prepareGeometryChange();

    QRect box = mPicture.boundingRect(); //size of the image, in pixels

    double angle = PI * mAngle / 180; //convert angle to radians
    
    // Angle between vertical in picture space and the vector 
    // from center to upper left corner of the picture
    double anglePicture = atan2 ( (double)box.width(), (double)box.height() );

    // Angle (clockwise) between horizontal in paper space
    // and the vector from center to upper left corner of the picture
    double anglePaper = PI / 2 - anglePicture - angle;

    // Distance from center to upper left corner in canvas units
    double r = sqrt ((double)(mWidth*mWidth/4 + mHeight*mHeight/4) );

    // Position of upper left corner in map
    int dx = (int) ( r * cos ( anglePaper ) );
    int dy = (int) ( r * sin ( anglePaper ) );


    mAreaPoints[0] = QPointF( -dx, -dy ); //add the top-left point to the polygon
    mAreaPoints[2] = QPointF( dx, dy ); //bottom-right

    anglePaper = PI / 2 - anglePicture + angle;
    dx = (int) ( r * cos ( anglePaper ) );
    dy = (int) ( r * sin ( anglePaper ) );

    mAreaPoints[1] =  QPointF( dx, -dy ); //top right
    mAreaPoints[3] = QPointF( -dx, dy ); //bottom left

    mBoundingRect = mAreaPoints.boundingRect();

    QAbstractGraphicsShapeItem::update();
    QAbstractGraphicsShapeItem::scene()->update();
}

QRectF QgsComposerPicture::boundingRect ( void ) const
{
#ifdef QGISDEBUG
    std::cout << "QgsComposerPicture::boundingRect" << std::endl;
#endif
    return mBoundingRect;
}

QPolygonF QgsComposerPicture::areaPoints() const
{
#ifdef QGISDEBUG
    std::cout << "QgsComposerPicture::areaPoints" << std::endl;
#endif

    return mAreaPoints;
}


void QgsComposerPicture::on_mFrameCheckBox_stateChanged ( int )
{
#ifdef QGISDEBUG
    std::cout << "QgsComposerPicture::on_mFrameCheckBox_stateChanged" << std::endl;
#endif

    mFrame = mFrameCheckBox->isChecked();

    QAbstractGraphicsShapeItem::update();
    QAbstractGraphicsShapeItem::scene()->update();

    writeSettings();
}

void QgsComposerPicture::on_mAngleLineEdit_editingFinished ( )
{
#ifdef QGISDEBUG
    std::cout << "QgsComposerPicture::on_mAngleLineEdit_editingFinished()" << std::endl;
#endif
    mAngle = mAngleLineEdit->text().toDouble();

    recalculate();

    writeSettings();

}

void QgsComposerPicture::on_mWidthLineEdit_editingFinished ( )
{
#ifdef QGISDEBUG
    std::cout << "QgsComposerPicture::on_mWidthLineEdit_editingFinished()" << std::endl;
#endif

    mWidth = mComposition->fromMM ( mWidthLineEdit->text().toDouble() );

    QRect box = mPicture.boundingRect();
    mHeight = mWidth*box.height()/box.width();
    setOptions();

    recalculate();

    writeSettings();
}

void QgsComposerPicture::on_mPictureBrowseButton_clicked ( )
{
#ifdef QGISDEBUG
    std::cout << "QgsComposerPicture::on_mPictureBrowseButton_clicked()" << std::endl;
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

    loadPicture();

    if ( !mPictureValid ) {
        QMessageBox::warning( 0, tr("Warning"),
                        tr("Cannot load picture.") );
    }
    else
    {
        adjustPictureSize();
        setOptions();
        recalculate();
    }
}

void QgsComposerPicture::on_mPictureLineEdit_editingFinished ( )
{
  pictureChanged();
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
    mWidthLineEdit->setText ( QString("%1").arg( mComposition->toMM((int)mWidth), 0,'g') );
    mHeightLineEdit->setText ( QString("%1").arg( mComposition->toMM((int)mHeight), 0,'g') );
    mAngleLineEdit->setText ( QString::number ( mAngle ) );
    mFrameCheckBox->setChecked ( mFrame );
}

void QgsComposerPicture::setSelected (  bool s ) 
{
    mSelected = s;
    QAbstractGraphicsShapeItem::update(); // show highlight
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
    QString filters = tr("Pictures ("); //+ " ( *.svg *.SVG ";
    QList<QByteArray> formats = QImageWriter::supportedImageFormats();

    for ( int i = 0; i < formats.count(); i++ )
    {
        QString frmt = formats.at( i );
        QString fltr = " *." + frmt.toLower() + " *." + frmt.toUpper();
        filters += fltr;
    }
    filters += ");;All other files (*.*)";

    // Retrieve the last used directory
    QSettings settings;
    QString lastUsedDir = settings.value("/UI/lastComposerPictureDir", ".").toString();

    QString file = QFileDialog::getOpenFileName(
                    0,
                    tr("Choose a file"),
                    lastUsedDir,
                    filters );
    if (file.length() != 0)
    {
      QFileInfo myFile(file);
      QString myPath = myFile.path();
      settings.setValue("/UI/lastComposerPictureDir", myPath);
    }

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

    QgsProject::instance()->writeEntry( "Compositions", path+"x", mComposition->toMM((int)QGraphicsItem::pos().x()) );
    QgsProject::instance()->writeEntry( "Compositions", path+"y", mComposition->toMM((int)QGraphicsItem::pos().y()) );
    QgsProject::instance()->writeEntry( "Compositions", path+"width", mComposition->toMM((int)mWidth) );
    QgsProject::instance()->writeEntry( "Compositions", path+"height", mComposition->toMM((int)mHeight) );

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

    double x = mComposition->fromMM(QgsProject::instance()->readDoubleEntry( "Compositions", path+"x", 0, &ok));
    double y = mComposition->fromMM(QgsProject::instance()->readDoubleEntry( "Compositions", path+"y", 0, &ok));
    setPos(x, y);

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
