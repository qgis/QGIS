/***************************************************************************
                           qgscomposerscalebar.cpp
                             -------------------
    begin                : March 2005
    copyright            : (C) 2005 by Radim Blazek
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
#include <math.h>
#include <iostream>
#include <typeinfo>
#include <map>
#include <vector>

#include <qwidget.h>
#include <qrect.h>
#include <qcombobox.h>
#include <qdom.h>
#include <qcanvas.h>
#include <qpainter.h>
#include <qstring.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qlineedit.h>
#include <qpointarray.h>
#include <qfont.h>
#include <qfontmetrics.h>
#include <qfontdialog.h>
#include <qpen.h>
#include <qrect.h>
#include <qlistview.h>
#include <qpopupmenu.h>
#include <qlabel.h>
#include <qpointarray.h>
#include <qrect.h>
#include <qspinbox.h>

#include "qgsrect.h"
#include "qgsmaptopixel.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsvectorlayer.h"
#include "qgsdlgvectorlayerproperties.h"
#include "qgscomposition.h"
#include "qgscomposermap.h"
#include "qgscomposerscalebar.h"

#include "qgssymbol.h"

#include "qgsrenderer.h"
#include "qgsrenderitem.h"
#include "qgsrangerenderitem.h"

#include "qgscontinuouscolrenderer.h"
#include "qgsgraduatedmarenderer.h"
#include "qgsgraduatedsymrenderer.h"
#include "qgssimarenderer.h"
#include "qgssinglesymrenderer.h"
#include "qgsuniquevalrenderer.h"
#include "qgsuvalmarenderer.h"
#include "qgssvgcache.h"

QgsComposerScalebar::QgsComposerScalebar ( QgsComposition *composition, int id, 
	                                            int x, int y )
    : QCanvasPolygonalItem(0),
    mComposition(composition),
    mMap(0),
    mBrush(QColor(150,150,150))
{
    std::cout << "QgsComposerScalebar::QgsComposerScalebar()" << std::endl;
    mId = id;
    mSelected = false;

    mMapCanvas = mComposition->mapCanvas();

    QCanvasPolygonalItem::setX(x);
    QCanvasPolygonalItem::setY(y);

    init();

    // Set map to the first available if any
    std::vector<QgsComposerMap*> maps = mComposition->maps();
    if ( maps.size() > 0 ) {
	mMap = maps[0]->id();
    }

    // Set default according to the map
    QgsComposerMap *map = mComposition->map ( mMap );
    if ( map ) {
	mMapUnitsPerUnit = 1.;
	mUnitLabel = "m";

	// make one segment cca 1/10 of map width and it will be 1xxx, 2xxx or 5xxx
	double mapwidth = 1. * map->QCanvasRectangle::width() / map->scale();

	mSegmentLength = mapwidth / 10;
	
	int powerOf10 = int(pow(10.0, int(log(mSegmentLength) / log(10.0)))); // from scalebar plugin

	int isize = (int) ceil ( mSegmentLength / powerOf10 ); 

	if ( isize == 3 ) isize = 2;
	else if ( isize == 4 ) isize = 5;
	else if ( isize > 5  && isize < 8 ) isize = 5;
	else if ( isize > 7  ) isize = 10;
	
	mSegmentLength = isize * powerOf10 ;
	
	// the scale bar will take cca 1/4 of the map width
	mNumSegments = (int) ( mapwidth / 4 / mSegmentLength ); 
    
	int segsize = (int) ( mSegmentLength * map->scale() );
	mFont.setPointSize ( (int) ( segsize/10) );
    } 
    else 
    {
	mMapUnitsPerUnit = 1.;
	mUnitLabel = "m";
	mSegmentLength = 1000.;
	mNumSegments = 5;
	mFont.setPointSize ( 8  );
    }
    
    // Calc size
    recalculate();

    // Add to canvas
    setCanvas(mComposition->canvas());

    QCanvasPolygonalItem::show();
    QCanvasPolygonalItem::update();
     
    writeSettings();
}

QgsComposerScalebar::QgsComposerScalebar ( QgsComposition *composition, int id ) 
    : QCanvasPolygonalItem(0),
    mComposition(composition),
    mMap(0),
    mBrush(QColor(150,150,150))
{
    std::cout << "QgsComposerScalebar::QgsComposerScalebar()" << std::endl;
    mId = id;
    mSelected = false;

    mMapCanvas = mComposition->mapCanvas();

    init();

    readSettings();

    // Calc size
    recalculate();

    // Add to canvas
    setCanvas(mComposition->canvas());

    QCanvasPolygonalItem::show();
    QCanvasPolygonalItem::update();
}

void QgsComposerScalebar::init ( void ) 
{
    mUnitLabel = "m";

    // Rectangle
    QCanvasPolygonalItem::setZ(50);
    setActive(true);

    // Plot style
    setPlotStyle ( QgsComposition::Preview );
    
    connect ( mComposition, SIGNAL(mapChanged(int)), this, SLOT(mapChanged(int)) ); 
}

QgsComposerScalebar::~QgsComposerScalebar()
{
    std::cerr << "QgsComposerScalebar::~QgsComposerScalebar()" << std::endl;
    QCanvasItem::hide();
}

QRect QgsComposerScalebar::render ( QPainter *p )
{
    std::cout << "QgsComposerScalebar::render p = " << p << std::endl;

    // Painter can be 0, create dummy to avoid many if below
    QPainter *painter;
    QPixmap *pixmap;
    if ( p ) {
	painter = p;
    } else {
	pixmap = new QPixmap(1,1);
	painter = new QPainter( pixmap );
    }

    std::cout << "mComposition->scale() = " << mComposition->scale() << std::endl;

    // Draw background rectangle
    painter->setPen( QPen(QColor(255,255,255), 1) );
    painter->setBrush( QBrush( QColor(255,255,255), Qt::SolidPattern) );

    painter->drawRect ( mBoundingRect.x(), mBoundingRect.y(), 
	               mBoundingRect.width()+1, mBoundingRect.height()+1 ); // is it right?
    

    // Font size in canvas units
    int size = (int) ( 25.4 * mComposition->scale() * mFont.pointSize() / 72);

    // Metrics 
    QFont font ( mFont );
    font.setPointSize ( size );
    QFontMetrics metrics ( font );
    
    // Fonts for rendering

    // It seems that font pointSize is used in points in Postscript, that means it depends 
    // on resolution!
    if ( plotStyle() == QgsComposition::Print ) {
	size = (int) ( 72.0 * size / mComposition->resolution() );
    }
    font.setPointSize ( size );

    // Not sure about Style Strategy, QFont::PreferMatch?
    font.setStyleStrategy ( (QFont::StyleStrategy) (QFont::PreferOutline | QFont::PreferAntialias) );

    int xmin; // min x
    int xmax; // max x
    int ymin; // min y
    int ymax; // max y

    int cx = (int) QCanvasPolygonalItem::x();
    int cy = (int) QCanvasPolygonalItem::y();

    painter->setPen ( mPen );
    painter->setBrush ( mBrush );
    painter->setFont ( font );

    QgsComposerMap *map = mComposition->map ( mMap );
    if ( map ) {
	// width of the whole scalebar in canvas points
	int segwidth = (int) ( mSegmentLength * map->scale() );
	int width = (int) ( segwidth * mNumSegments );
	
	int barLx = (int) ( cx - width/2 );

	// fill odd
	for ( int i = 0; i < mNumSegments; i += 2 ) {
	    painter->drawRect( barLx+i*segwidth, cy, segwidth+1, mHeight );
	}

	// ticks
	int ticksize = (int ) (3./4*mHeight);
	for ( int i = 0; i <= mNumSegments; i++ ) {
	    painter->drawLine( barLx+i*segwidth, cy, barLx+i*segwidth, cy-ticksize );
	}

	painter->setBrush( Qt::NoBrush );

	painter->drawRect( barLx, cy, width+1, mHeight );
	
	// labels
	int h = metrics.height();
	int offset = (int ) (1./2*ticksize);
	for ( int i = 0; i <= mNumSegments; i++ ) {
	    int lab = (int) (1. * i * mSegmentLength / mMapUnitsPerUnit);
	    QString txt = QString::number(lab);
	    int w = metrics.width ( txt );
	    int shift = (int) w/2;
	    
	    if ( i == 0 ) { 
		xmin = (int) barLx - w/2; 
	    }

	    if ( i == mNumSegments ) { 
		txt.append ( " " + mUnitLabel );
		w = metrics.width ( txt );

		xmax = (int) barLx + width - shift + w; 
	    }
	    
	    int x = barLx+i*segwidth;

	    QRect r ( (int)x-shift, cy-ticksize-offset-h, w, h) ;
	    
	    painter->drawText( r, Qt::AlignCenter|Qt::SingleLine, txt );
	}
	
	ymin = cy - ticksize - offset - h;
	ymax = cy + mHeight;	
    } 
    else 
    {
	int width = 50 * mComposition->scale(); 

	int barLx = (int) ( cx - width/2 );
	painter->drawRect( (int)barLx, (int)(cy-mHeight/2), width, mHeight );

	xmin = barLx;
        xmax = barLx + width;
 	ymin = cy - mHeight;
	ymax = cy + mHeight;	
    }

    if ( !p ) {
	delete painter;
	delete pixmap;
    }

    return QRect ( xmin-mMargin, ymin-mMargin, xmax-xmin+2*mMargin, ymax-ymin+2*mMargin);
}

void QgsComposerScalebar::draw ( QPainter & painter )
{
    std::cout << "draw mPlotStyle = " << plotStyle() << std::endl;

    render( &painter );

    // Show selected / Highlight
    if ( mSelected && plotStyle() == QgsComposition::Preview ) {
        painter.setPen( mComposition->selectionPen() );
        painter.setBrush( mComposition->selectionBrush() );
	
	int s = mComposition->selectionBoxSize();
	QRect r = boundingRect();

	painter.drawRect ( r.x(), r.y(), s, s );
	painter.drawRect ( r.x()+r.width()-s, r.y(), s, s );
	painter.drawRect ( r.x()+r.width()-s, r.y()+r.height()-s, s, s );
	painter.drawRect ( r.x(), r.y()+r.height()-s, s, s );
    }
}

void QgsComposerScalebar::drawShape ( QPainter & painter )
{
    draw ( painter );
}

void QgsComposerScalebar::changeFont ( void ) 
{
    bool result;
    
    mFont = QFontDialog::getFont(&result, mFont, this );

    if ( result ) {
	recalculate();
	QCanvasPolygonalItem::update();
	QCanvasPolygonalItem::canvas()->update();
	writeSettings();
    }
}

void QgsComposerScalebar::unitLabelChanged (  )
{
    mUnitLabel = mUnitLabelLineEdit->text();
    recalculate();
    QCanvasPolygonalItem::update();
    QCanvasPolygonalItem::canvas()->update();
    writeSettings();
}

void QgsComposerScalebar::mapSelectionChanged ( int i )
{
    mMap = mMaps[i];
    recalculate();
    QCanvasPolygonalItem::update();
    QCanvasPolygonalItem::canvas()->update();
    writeSettings();
}

void QgsComposerScalebar::mapChanged ( int id )
{
    if ( id != mMap ) return;
    recalculate();
    QCanvasPolygonalItem::update();
    QCanvasPolygonalItem::canvas()->update();
}

void QgsComposerScalebar::sizeChanged ( )
{
    mSegmentLength = mSegmentLengthLineEdit->text().toDouble();
    mNumSegments = mNumSegmentsLineEdit->text().toInt();
    mPen.setWidth ( mLineWidthSpinBox->value() );
    mMapUnitsPerUnit = mMapUnitsPerUnitLineEdit->text().toInt();
    recalculate();
    QCanvasPolygonalItem::update();
    QCanvasPolygonalItem::canvas()->update();
    writeSettings();
}

void QgsComposerScalebar::moveBy(double x, double y )
{
    std::cout << "QgsComposerScalebar::move" << std::endl;
    QCanvasItem::moveBy ( x, y );

    recalculate();
    //writeSettings(); // not necessary called by composition
}

void QgsComposerScalebar::recalculate ( void ) 
{
    std::cout << "QgsComposerScalebar::recalculate" << std::endl;
    
    mHeight = (int) ( 25.4 * mComposition->scale() * mFont.pointSize() / 72);
    mMargin = (int) (3.*mHeight/2);
    
    // !!! invalidate() MUST BE called before the value returned by areaPoints() changes
    QCanvasPolygonalItem::invalidate();
    
    mBoundingRect = render(0);
    
    QCanvasItem::update();
}

QRect QgsComposerScalebar::boundingRect ( void ) const
{
    std::cout << "QgsComposerScalebar::boundingRect" << std::endl;
    return mBoundingRect;
}

QPointArray QgsComposerScalebar::areaPoints() const
{
    std::cout << "QgsComposerScalebar::areaPoints" << std::endl;

    QRect r = boundingRect();
    QPointArray pa(4);
    pa[0] = QPoint( r.x(), r.y() );
    pa[1] = QPoint( r.x()+r.width(), r.y() );
    pa[2] = QPoint( r.x()+r.width(), r.y()+r.height() );
    pa[3] = QPoint( r.x(), r.y()+r.height() );
    return pa ;
}

void QgsComposerScalebar::setOptions ( void )
{ 
    mSegmentLengthLineEdit->setText( QString::number(mSegmentLength) );
    mNumSegmentsLineEdit->setText( QString::number( mNumSegments ) );
    mUnitLabelLineEdit->setText( mUnitLabel );
    mMapUnitsPerUnitLineEdit->setText( QString::number(mMapUnitsPerUnit ) );

    mLineWidthSpinBox->setValue ( mPen.width() );
    
    // Maps
    mMapComboBox->clear();
    std::vector<QgsComposerMap*> maps = mComposition->maps();

    mMaps.clear();
    
    bool found = false;
    mMapComboBox->insertItem ( "", 0 );
    mMaps.push_back ( 0 );
    for ( int i = 0; i < maps.size(); i++ ) {
	mMapComboBox->insertItem ( maps[i]->name(), i+1 );
	mMaps.push_back ( maps[i]->id() );

	if ( maps[i]->id() == mMap ) {
	    found = true;
	    mMapComboBox->setCurrentItem ( i+1 );
	}
    }

    if ( ! found ) {
	mMap = 0;
	mMapComboBox->setCurrentItem ( 0 );
    }
}

void QgsComposerScalebar::setSelected (  bool s ) 
{
    mSelected = s;
    QCanvasPolygonalItem::update(); // show highlight
}    

bool QgsComposerScalebar::selected( void )
{
    return mSelected;
}

QWidget *QgsComposerScalebar::options ( void )
{
    setOptions ();
    return ( dynamic_cast <QWidget *> (this) ); 
}

bool QgsComposerScalebar::writeSettings ( void )  
{
    std::cout << "QgsComposerScalebar::writeSettings" << std::endl;
    QString path;
    path.sprintf("/composition_%d/scalebar_%d/", mComposition->id(), mId ); 

    QgsProject::instance()->writeEntry( "Compositions", path+"x", mComposition->toMM((int)QCanvasPolygonalItem::x()) );
    QgsProject::instance()->writeEntry( "Compositions", path+"y", mComposition->toMM((int)QCanvasPolygonalItem::y()) );

    QgsProject::instance()->writeEntry( "Compositions", path+"map", mMap );

    QgsProject::instance()->writeEntry( "Compositions", path+"unit/label", mUnitLabel  );
    QgsProject::instance()->writeEntry( "Compositions", path+"unit/mapunits", mMapUnitsPerUnit );

    QgsProject::instance()->writeEntry( "Compositions", path+"segmentsize", mSegmentLength );
    QgsProject::instance()->writeEntry( "Compositions", path+"numsegments", mNumSegments );

    QgsProject::instance()->writeEntry( "Compositions", path+"font/size", mFont.pointSize() );
    QgsProject::instance()->writeEntry( "Compositions", path+"font/family", mFont.family() );
    QgsProject::instance()->writeEntry( "Compositions", path+"font/weight", mFont.weight() );
    QgsProject::instance()->writeEntry( "Compositions", path+"font/underline", mFont.underline() );
    QgsProject::instance()->writeEntry( "Compositions", path+"font/strikeout", mFont.strikeOut() );
    
    QgsProject::instance()->writeEntry( "Compositions", path+"pen/width", (int)mPen.width() );

    return true; 
}

bool QgsComposerScalebar::readSettings ( void )
{
    bool ok;
    QString path;
    path.sprintf("/composition_%d/scalebar_%d/", mComposition->id(), mId );

    QCanvasPolygonalItem::setX( mComposition->fromMM(QgsProject::instance()->readDoubleEntry( "Compositions", path+"x", 0, &ok)) );
    QCanvasPolygonalItem::setY( mComposition->fromMM(QgsProject::instance()->readDoubleEntry( "Compositions", path+"y", 0, &ok)) );
    
    mMap = QgsProject::instance()->readNumEntry("Compositions", path+"map", 0, &ok);
    mUnitLabel = QgsProject::instance()->readEntry("Compositions", path+"unit/label", "???", &ok);
    mMapUnitsPerUnit = QgsProject::instance()->readDoubleEntry("Compositions", path+"unit/mapunits", 1., &ok);

    mSegmentLength = QgsProject::instance()->readDoubleEntry("Compositions", path+"segmentsize", 1000., &ok);
    mNumSegments = QgsProject::instance()->readNumEntry("Compositions", path+"numsegments", 5, &ok);
     
    mFont.setFamily ( QgsProject::instance()->readEntry("Compositions", path+"font/family", "", &ok) );
    mFont.setPointSize ( QgsProject::instance()->readNumEntry("Compositions", path+"font/size", 10, &ok) );
    mFont.setWeight(  QgsProject::instance()->readNumEntry("Compositions", path+"font/weight", (int)QFont::Normal, &ok) );
    mFont.setUnderline(  QgsProject::instance()->readBoolEntry("Compositions", path+"font/underline", false, &ok) );
    mFont.setStrikeOut(  QgsProject::instance()->readBoolEntry("Compositions", path+"font/strikeout", false, &ok) );

    mPen.setWidth(  QgsProject::instance()->readNumEntry("Compositions", path+"pen/width", 1, &ok) );
    
    recalculate();
    
    return true;
}

bool QgsComposerScalebar::removeSettings( void )
{
    std::cerr << "QgsComposerScalebar::deleteSettings" << std::endl;

    QString path;
    path.sprintf("/composition_%d/scalebar_%d", mComposition->id(), mId ); 
    return QgsProject::instance()->removeEntry ( "Compositions", path );
}

bool QgsComposerScalebar::writeXML( QDomNode & node, QDomDocument & document, bool temp )
{
    return true;
}

bool QgsComposerScalebar::readXML( QDomNode & node )
{
    return true;
}

