/***************************************************************************
                         qgscomposermap.cpp
                             -------------------
    begin                : January 2005
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
#include <qlabel.h>
#include <qcheckbox.h>

#include "qgsproject.h"
#include "qgsrect.h"
#include "qgsmaptopixel.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsvectorlayer.h"
#include "qgscomposition.h"
#include "qgscomposermap.h"
#include "qgsscalecalculator.h"

QgsComposerMap::QgsComposerMap ( QgsComposition *composition, int id, int x, int y, int width, int height )
    : QCanvasRectangle(x,y,width,height,0)
{
    mComposition = composition;
    mId = id;
    mMapCanvas = mComposition->mapCanvas();
    mName.sprintf ( tr("Map %d"), mId );

    init();
    recalculate();

    // Add to canvas
    setCanvas(mComposition->canvas());
    QCanvasRectangle::show();
    QCanvasRectangle::update(); // ?

    writeSettings();
}

QgsComposerMap::QgsComposerMap ( QgsComposition *composition, int id )
    : QCanvasRectangle(0,0,10,10,0)
{
    mComposition = composition;
    mId = id;
    mMapCanvas = mComposition->mapCanvas();
    mName.sprintf ( tr("Map %d"), mId );

    init();
    readSettings();
    recalculate();

    // Add to canvas
    setCanvas(mComposition->canvas());
    QCanvasRectangle::show();
    QCanvasRectangle::update(); // ?
}

void QgsComposerMap::init ()
{
    mNumCachedLayers = 0;
    mSelected = false;
    mUserExtent = mMapCanvas->extent();
    mDrawing = false;

    // Cache
    mCacheUpdated = false;

    // Potemkin
    mCalculateComboBox->insertItem( "Scale" );

    setPlotStyle ( QgsComposition::Preview );
    
    // Preview style
    mPreviewMode = Cache;
    mPreviewModeComboBox->insertItem ( "Cache", Cache );
    mPreviewModeComboBox->insertItem ( "Render", Render );
    mPreviewModeComboBox->insertItem ( "Rectangle", Rectangle );
    mPreviewModeComboBox->setCurrentItem ( Cache );

    mWidthScale = 1.0 / mComposition->scale();
    mSymbolScale = 1.0;
    mFontScale = 1.0;

    mFrame = true;

    QCanvasRectangle::setZ(20);
    setActive(true);
}

QgsComposerMap::~QgsComposerMap()
{
     std::cerr << "QgsComposerMap::~QgsComposerMap" << std::endl;
}

void QgsComposerMap::draw ( QPainter *painter, QgsRect *extent, QgsMapToPixel *transform, QPaintDevice *device )
{
    mMapCanvas->freeze(true);  // necessary ?
    int nlayers = mMapCanvas->layerCount();

    for ( int i = 0; i < nlayers; i++ ) {
	QgsMapLayer *layer = mMapCanvas->getZpos(i);

	if ( !layer->visible() ) continue;

	if ( layer->type() == QgsMapLayer::VECTOR ) {
	    QgsVectorLayer *vector = dynamic_cast <QgsVectorLayer*> (layer);

	    double widthScale = mWidthScale * mComposition->scale();
	    if ( plotStyle() == QgsComposition::Preview && mPreviewMode == Render ) {
		widthScale *= mComposition->viewScale();
	    }
	    double symbolScale = mSymbolScale * mComposition->scale();
	    vector->draw( painter, extent, transform, device, widthScale, symbolScale );

	    if ( vector->labelOn() ) {
	        double fontScale = 25.4 * mFontScale * mComposition->scale() / 72;
		if ( plotStyle() == QgsComposition::Print ) {
		    fontScale *= 72.0 / mComposition->resolution();
		}
		vector->drawLabels (  painter, extent, transform, device, fontScale );
	    }
	} else {
	    layer->draw( painter, extent, transform, device );
	}
    }
    mMapCanvas->freeze(false);
}

void QgsComposerMap::setUserExtent ( QgsRect const & rect )
{
    mUserExtent = rect;
    recalculate();
    
    QCanvasRectangle::update();
    QCanvasRectangle::canvas()->update();
}

void QgsComposerMap::cache ( void )
{
    // Create preview on some reasonable size. It was slow with cca 1500x1500 points on 2x1.5GHz 
    // Note: The resolution should also respect the line widths, it means that 
    //       1 pixel in cache should have ia similar size as 1 pixel in canvas
    //       but it can result in big cache -> limit

    int w = QCanvasRectangle::width() < 1000 ? QCanvasRectangle::width() : 1000;
    int h = (int) ( mExtent.height() * w / mExtent.width() );
    // It can happen that extent is not initialised well -> check 
    if ( h < 1 || h > 10000 ) h = w; 
    
    std::cout << "extent = " << mExtent.width() <<  " x " << mExtent.height() << std::endl;
    std::cout << "cache = " << w <<  " x " << h << std::endl;

    mCacheExtent = QgsRect ( mExtent );
    double scale = mExtent.width() / w;
    mCacheExtent.setXmax ( mCacheExtent.xMin() + w * scale );
    mCacheExtent.setYmax ( mCacheExtent.yMin() + h * scale );
	    
    mCachePixmap.resize( w, h );

    // WARNING: ymax in QgsMapToPixel is device height!!!
    QgsMapToPixel transform(scale, h, mCacheExtent.yMin(), mCacheExtent.xMin() );

    std::cout << "transform = " << transform.showParameters() << std::endl;
    
    mCachePixmap.fill(QColor(255,255,255));

    QPainter p(&mCachePixmap);
    
    draw( &p, &mCacheExtent, &transform, &mCachePixmap );
    p.end();

    mNumCachedLayers = mMapCanvas->layerCount();
    mCacheUpdated = true;
}

void QgsComposerMap::draw ( QPainter & painter )
{
    if ( mDrawing ) return; 
    mDrawing = true;

    std::cout << "draw mPlotStyle = " << plotStyle() 
	      << " mPreviewMode = " << mPreviewMode << std::endl;
    
    if ( plotStyle() == QgsComposition::Preview &&  mPreviewMode == Cache ) { // Draw from cache
        std::cout << "use cache" << std::endl;

	if ( !mCacheUpdated || mMapCanvas->layerCount() != mNumCachedLayers ) {
	    cache();
	}
	
	// Scale so that the cache fills the map rectangle
	double scale = 1.0 * QCanvasRectangle::width() / mCachePixmap.width();
	
	
	painter.save();

	painter.translate ( QCanvasRectangle::x(), QCanvasRectangle::y() );
	painter.scale(scale,scale);
	std::cout << "scale = " << scale << std::endl;
        std::cout << "translate: " << QCanvasRectangle::x() << ", " << QCanvasRectangle::y() << std::endl;
	// Note: drawing only a visible part of the pixmap doesn't make it much faster
	painter.drawPixmap(0,0, mCachePixmap);

	painter.restore();

    } else if ( (plotStyle() == QgsComposition::Preview && mPreviewMode == Render) || 
	         plotStyle() == QgsComposition::Print ) 
    {
        std::cout << "render" << std::endl;
	
	double scale = mExtent.width() / QCanvasRectangle::width();
	QgsMapToPixel transform(scale, QCanvasRectangle::height(), mExtent.yMin(), mExtent.xMin() );
	
	painter.save();
	painter.translate ( QCanvasRectangle::x(), QCanvasRectangle::y() );
	   
	// Note: CoordDevice doesn't work well
	painter.setClipRect ( 0, 0, QCanvasRectangle::width(), QCanvasRectangle::height(), QPainter::CoordPainter );
	
	draw( &painter, &mExtent, &transform, painter.device() );
	painter.restore();
    } 

    // Draw frame around
    if ( mFrame ) {
	painter.setPen( QPen(QColor(0,0,0), 1) );
	painter.setBrush( Qt::NoBrush );
        painter.save();
	painter.translate ( QCanvasRectangle::x(), QCanvasRectangle::y() );
	painter.drawRect ( 0, 0, QCanvasRectangle::width()+1, QCanvasRectangle::height()+1 ); // is it right?
        painter.restore();
    }

    // Show selected / Highlight
    std::cout << "mSelected = " << mSelected << std::endl;
    if ( mSelected && plotStyle() == QgsComposition::Preview ) {
	std::cout << "highlight" << std::endl;
	painter.setPen( mComposition->selectionPen() );
	painter.setBrush( mComposition->selectionBrush() );
	int x = (int) QCanvasRectangle::x();
	int y = (int) QCanvasRectangle::y();
	int s = mComposition->selectionBoxSize();

	painter.drawRect ( x, y, s, s );
	x += QCanvasRectangle::width();
	painter.drawRect ( x-s, y, s, s );
	y += QCanvasRectangle::height();
	painter.drawRect ( x-s, y-s, s, s );
	x -= QCanvasRectangle::width();
	painter.drawRect ( x, y-s, s, s );
    }
    
    mDrawing = false;
}

void QgsComposerMap::sizeChanged ( void ) 
{
    int w, h;
    w = mComposition->fromMM ( mWidthLineEdit->text().toDouble() );
    h = mComposition->fromMM ( mHeightLineEdit->text().toDouble() );

    QCanvasRectangle::setSize ( w, h);
    recalculate();

    QCanvasRectangle::update();
    QCanvasRectangle::canvas()->update();
    
    writeSettings();
}

void QgsComposerMap::scaleChanged ( void ) 
{
    mWidthScale = mWidthScaleLineEdit->text().toDouble();
    mSymbolScale = mSymbolScaleLineEdit->text().toDouble();
    mFontScale = mFontScaleLineEdit->text().toDouble();

    mCacheUpdated = false;
    QCanvasRectangle::update();
    QCanvasRectangle::canvas()->update();
    
    writeSettings();
    mComposition->emitMapChanged ( mId );
}

void QgsComposerMap::previewModeChanged ( int i )
{
    mPreviewMode = (PreviewMode) i;
    writeSettings();
}

void QgsComposerMap::recalculate ( void ) 
{
    // Currently only QgsComposition::Scale is supported

    // Calculate scale from extent and rectangle
    double xscale = QCanvasRectangle::width() / mUserExtent.width();
    double yscale = QCanvasRectangle::height() / mUserExtent.height();

    mExtent = mUserExtent;

    if ( xscale < yscale ) {
	mScale = xscale;
	// extend y
	double d = ( QCanvasRectangle::height() / mScale - mUserExtent.height() ) / 2 ;
	mExtent.setYmin ( mExtent.yMin() - d );
	mExtent.setYmax ( mExtent.yMax() + d );
    } else {
	mScale = yscale;
	// extend x
	double d = ( QCanvasRectangle::width() / mScale - mUserExtent.width() ) / 2 ;
	mExtent.setXmin ( mExtent.xMin() - d );
	mExtent.setXmax ( mExtent.xMax() + d );
    }

    std::cout << "mUserExtent = " << mUserExtent.stringRep() << std::endl;
    std::cout << "mScale = " << mScale << std::endl;
    std::cout << "mExtent = " << mExtent.stringRep() << std::endl;

    setOptions();
    mCacheUpdated = false;
}

void QgsComposerMap::frameChanged ( )
{
    mFrame = mFrameCheckBox->isChecked();

    QCanvasRectangle::update();
    QCanvasRectangle::canvas()->update();

    writeSettings();
}


void QgsComposerMap::setOptions ( void )
{ 
    mNameLabel->setText ( mName );
    
    mWidthLineEdit->setText ( QString("%1").arg( mComposition->toMM(QCanvasRectangle::width()), 0,'g') );
    mHeightLineEdit->setText ( QString("%1").arg( mComposition->toMM(QCanvasRectangle::height()),0,'g') );
    
    // Scale
    double scale;
    switch ( QgsProject::instance()->mapUnits() ) {
	case QgsScaleCalculator::METERS :
	    scale = 1000. * mComposition->scale() / mScale; 
            mScaleLineEdit->setText ( QString("%1").arg((int)scale) );
	    break;
	case QgsScaleCalculator::FEET :
	    scale = 304.8 * mComposition->scale() / mScale; 
            mScaleLineEdit->setText ( QString("%1").arg((int)scale) );
	    break;
	case QgsScaleCalculator::DEGREES :
	    scale = mComposition->scale() / mScale;
            mScaleLineEdit->setText ( QString("%1").arg(scale,0,'f') );
	    break;
    }
    
    mWidthScaleLineEdit->setText ( QString("%1").arg(mWidthScale,0,'g',2) );
    mSymbolScaleLineEdit->setText ( QString("%1").arg(mSymbolScale,0,'g',2) );
    mFontScaleLineEdit->setText ( QString("%1").arg(mFontScale,0,'g',2) );

    mFrameCheckBox->setChecked ( mFrame );
}

void QgsComposerMap::setCurrentExtent ( void )
{ 
    mUserExtent = mMapCanvas->extent();
    recalculate();
    QCanvasRectangle::update();
    QCanvasRectangle::canvas()->update();
    writeSettings();
}

void QgsComposerMap::setSelected (  bool s ) 
{
    mSelected = s;
    QCanvasRectangle::update(); // show highlight
}    

bool QgsComposerMap::selected( void )
{
    return mSelected;
}

double QgsComposerMap::scale ( void ) { return mScale; }

QWidget *QgsComposerMap::options ( void )
{
    setOptions ();
    return ( dynamic_cast <QWidget *> (this) );
}

QString QgsComposerMap::name ( void ) 
{
    return mName;
}

double QgsComposerMap::widthScale (void ) { return mWidthScale ; }
double QgsComposerMap::symbolScale (void ) { return mSymbolScale ; }
double QgsComposerMap::fontScale (void ) { return mFontScale ; }

bool QgsComposerMap::writeSettings ( void )  
{
    QString path;
    path.sprintf("/composition_%d/map_%d/", mComposition->id(), mId ); 
    QgsProject::instance()->writeEntry( "Compositions", path+"x", mComposition->toMM((int)QCanvasRectangle::x()) );
    QgsProject::instance()->writeEntry( "Compositions", path+"y", mComposition->toMM((int)QCanvasRectangle::y()) );
    QgsProject::instance()->writeEntry( "Compositions", path+"width", mComposition->toMM(QCanvasRectangle::width()) );
    QgsProject::instance()->writeEntry( "Compositions", path+"height", mComposition->toMM(QCanvasRectangle::height()) );
    QgsProject::instance()->writeEntry( "Compositions", path+"north", mUserExtent.yMax() );
    QgsProject::instance()->writeEntry( "Compositions", path+"south", mUserExtent.yMin() );
    QgsProject::instance()->writeEntry( "Compositions", path+"east", mUserExtent.xMax() );
    QgsProject::instance()->writeEntry( "Compositions", path+"west", mUserExtent.xMin() );

    QgsProject::instance()->writeEntry( "Compositions", path+"widthscale", mWidthScale );
    QgsProject::instance()->writeEntry( "Compositions", path+"symbolscale", mSymbolScale );
    QgsProject::instance()->writeEntry( "Compositions", path+"fontscale", mFontScale );

    QgsProject::instance()->writeEntry( "Compositions", path+"frame", mFrame );

    QgsProject::instance()->writeEntry( "Compositions", path+"previewmode", mPreviewMode );

    return true; 
}

bool QgsComposerMap::readSettings ( void )
{
    bool ok;
    QString path;
    path.sprintf("/composition_%d/map_%d/", mComposition->id(), mId );

    
    QCanvasRectangle::setX( mComposition->fromMM(QgsProject::instance()->readDoubleEntry( "Compositions", path+"x", 0, &ok)) );
    QCanvasRectangle::setY( mComposition->fromMM(QgsProject::instance()->readDoubleEntry( "Compositions", path+"y", 0, &ok)) );
    int w = mComposition->fromMM(QgsProject::instance()->readDoubleEntry( "Compositions", path+"width", 100, &ok)) ;
    int h = mComposition->fromMM(QgsProject::instance()->readDoubleEntry( "Compositions", path+"height", 100, &ok)) ;
    QCanvasRectangle::setSize(w,h);

    mUserExtent.setYmax ( QgsProject::instance()->readDoubleEntry( "Compositions", path+"north", 100, &ok) );
    mUserExtent.setYmin ( QgsProject::instance()->readDoubleEntry( "Compositions", path+"south", 0, &ok) );
    mUserExtent.setXmax ( QgsProject::instance()->readDoubleEntry( "Compositions", path+"east", 100, &ok) );
    mUserExtent.setXmin ( QgsProject::instance()->readDoubleEntry( "Compositions", path+"west", 0, &ok) );

    mWidthScale = QgsProject::instance()->readDoubleEntry("Compositions", path+"widthscale", 1., &ok);
    mSymbolScale = QgsProject::instance()->readDoubleEntry("Compositions", path+"symbolscale", 1., &ok);
    mFontScale = QgsProject::instance()->readDoubleEntry("Compositions", path+"fontscale", 1., &ok);
    
    mFrame = QgsProject::instance()->readBoolEntry("Compositions", path+"frame", true, &ok);
    
    mPreviewMode = (PreviewMode) QgsProject::instance()->readNumEntry("Compositions", path+"previewmode", Cache, &ok);
    
    recalculate();

    return true;
}

bool QgsComposerMap::removeSettings ( void )
{
    QString path;
    path.sprintf("/composition_%d/map_%d", mComposition->id(), mId );
    return QgsProject::instance()->removeEntry ( "Compositions", path );
}
    
bool QgsComposerMap::writeXML( QDomNode & node, QDomDocument & document, bool temp )
{
    return true;
}

bool QgsComposerMap::readXML( QDomNode & node )
{
    return true;
}
