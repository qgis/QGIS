/***************************************************************************
                         qgscomposervectorlegend.cpp
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
#include <typeinfo>
#include <map>

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

#include "qgsrect.h"
#include "qgsmaptopixel.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsvectorlayer.h"
#include "qgsdlgvectorlayerproperties.h"
#include "qgscomposition.h"
#include "qgscomposermap.h"
#include "qgscomposervectorlegend.h"

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

QgsComposerVectorLegend::QgsComposerVectorLegend ( QgsComposition *composition, int id, 
	                                            int x, int y, int fontSize )
    : QCanvasRectangle(x,y,10,10,0)
{
    std::cout << "QgsComposerVectorLegend::QgsComposerVectorLegend()" << std::endl;

    mComposition = composition;
    mId  = id;
    mMapCanvas = mComposition->mapCanvas();
    mNumCachedLayers = 0;

    mTitle = "Legend";

    // Font and pen 
    mFont.setPointSize ( fontSize );

    // Plot style
    setPlotStyle ( QgsComposition::Preview );

    mSelected = false;
    
    // Preview style
    mPreviewMode = Render;
    mPreviewModeComboBox->insertItem ( "Cache", Cache );
    mPreviewModeComboBox->insertItem ( "Render", Render );
    mPreviewModeComboBox->insertItem ( "Rectangle", Rectangle );
    mPreviewModeComboBox->setCurrentItem ( mPreviewMode );

    // Cache
    mCachePixmap = new QPixmap();
    
    // Calc size and cache
    recalculate();
    cache();

    // Add to canvas
    setCanvas(mComposition->canvas());
    QCanvasRectangle::setZ(70);
    setActive(true);
    QCanvasRectangle::show();
    QCanvasRectangle::update(); // ?
    

    // Potemkin
    mMapComboBox->insertItem( "Map 1" );

    writeSettings();
}

QgsComposerVectorLegend::~QgsComposerVectorLegend()
{
}

QRect QgsComposerVectorLegend::render ( QPainter *painter )
{
    std::cout << "QgsComposerVectorLegend::render" << std::endl;

    // Font size in canvas units
    int titleSize = (int) ( 25.4 * mComposition->scale() * mTitleFont.pointSize() / 72);
    int sectionSize = (int) ( 25.4 * mComposition->scale() * mSectionFont.pointSize() / 72);
    int size = (int) ( 25.4 * mComposition->scale() * mFont.pointSize() / 72);

    
    std::cout << "font size in canvas units = " << size << std::endl;

    // Metrics 
    QFont titleFont ( mTitleFont );
    QFont sectionFont ( mSectionFont );
    QFont font ( mFont );

    titleFont.setPointSize ( titleSize );
    sectionFont.setPointSize ( sectionSize );
    font.setPointSize ( size );

    QFontMetrics titleMetrics ( titleFont );
    QFontMetrics sectionMetrics ( sectionFont );
    QFontMetrics metrics ( font );

    // Fonts for rendering

    // It seems that font pointSize is used in points in Postscript, that means it depends 
    // on resolution!
    if ( plotStyle() == QgsComposition::Print ) {
	titleSize = (int) ( 72.0 * titleSize / mComposition->resolution() );
	sectionSize = (int) ( 72.0 * sectionSize / mComposition->resolution() );
	size = (int) ( 72.0 * size / mComposition->resolution() );
    }
    
    titleFont.setPointSize ( titleSize );
    sectionFont.setPointSize ( sectionSize );
    font.setPointSize ( size );

    std::cout << "title font size in canvas units = " << titleSize << std::endl;
    std::cout << "font size in canvas units = " << size << std::endl;

    // Not sure about Style Strategy, QFont::PreferMatch?
    titleFont.setStyleStrategy ( (QFont::StyleStrategy) (QFont::PreferOutline | QFont::PreferAntialias) );
    sectionFont.setStyleStrategy ( (QFont::StyleStrategy) (QFont::PreferOutline | QFont::PreferAntialias) );
    font.setStyleStrategy ( (QFont::StyleStrategy) (QFont::PreferOutline | QFont::PreferAntialias) );

    int x, y;

    // Title
    y = mMargin + titleMetrics.height();
    if ( painter ) {
        painter->setPen ( mPen );
	painter->setFont ( titleFont );
        painter->drawText( mMargin, y, mTitle );
    }
    int width = 2 * mMargin + titleMetrics.width ( mTitle ); 
    int height = mMargin + mSymbolSpace + titleMetrics.height(); // mSymbolSpace?
    
    int nlayers = mMapCanvas->layerCount();
    for ( int i = 0; i < nlayers; i++ ) {
	QgsMapLayer *layer = mMapCanvas->getZpos(i);
	if ( !layer->visible() ) continue;
	if ( layer->type() != QgsMapLayer::VECTOR ) continue;

	QgsVectorLayer *vector = dynamic_cast <QgsVectorLayer*> (layer);
	QgsRenderer *renderer = vector->renderer();
	
	height += mSymbolSpace;

	// Symbol
        if ( painter ) {
	    if ( typeid (*renderer) == typeid(QgsSingleSymRenderer) ) {
		QgsSingleSymRenderer *r = dynamic_cast <QgsSingleSymRenderer *> (renderer);
		QgsRenderItem *ri = r->item();
		QgsSymbol *sym = ri->getSymbol();
		
		painter->setPen ( sym->pen() );
		painter->setBrush ( sym->brush() );
		
		if ( vector->vectorType() == QGis::Point ) {
		    painter->drawRect( mMargin+mSymbolWidth/2-2, height+mSymbolHeight/2-2, 5, 5 );
		} else if ( vector->vectorType() == QGis::Line ) {
		    painter->drawLine ( mMargin, height+mSymbolHeight/2, 
					mMargin+mSymbolWidth, height+mSymbolHeight/2 );
		} else if ( vector->vectorType() == QGis::Polygon ) {
		    painter->drawRect ( mMargin, height, mSymbolWidth, mSymbolHeight );
		}
	    }
	}
	
	// Label 
	x = (int) ( 2*mMargin + mSymbolWidth );
	y = (int) ( height + mSymbolHeight/2 + metrics.height()/2 );
        if ( painter ) {
            painter->setPen ( mPen );
	    painter->setFont ( font );
            painter->drawText( x, y, layer->name() );	
	}

	int w = 3*mMargin + mSymbolWidth + metrics.width(layer->name());
	if ( w > width ) width = w;
	height += mSymbolHeight;
    }

    height += mMargin;
    
    QCanvasRectangle::setSize (  width, height );

    return QRect ( 0, 0, width, height);
}

void QgsComposerVectorLegend::cache ( void )
{
    std::cout << "QgsComposerVectorLegend::cache()" << std::endl;

    delete mCachePixmap;
    mCachePixmap = new QPixmap ( QCanvasRectangle::width(), QCanvasRectangle::height(), -1, QPixmap::BestOptim ); 

    QPainter p(mCachePixmap);
    
    mCachePixmap->fill(QColor(255,255,255));
    render ( &p );
    p.end();

    mNumCachedLayers = mMapCanvas->layerCount();
}

void QgsComposerVectorLegend::draw ( QPainter & painter )
{
    std::cout << "draw mPlotStyle = " << plotStyle() 
	      << " mPreviewMode = " << mPreviewMode << std::endl;

    // Draw background rectangle
    painter.setPen( QPen(QColor(0,0,0), 1) );
    painter.setBrush( QBrush( QColor(255,255,255), Qt::SolidPattern) );

    painter.save();
	
    painter.translate ( QCanvasRectangle::x(), QCanvasRectangle::y() );
    painter.drawRect ( 0, 0, QCanvasRectangle::width()+1, QCanvasRectangle::height()+1 ); // is it right?
    painter.restore();
    
    if ( plotStyle() == QgsComposition::Preview &&  mPreviewMode == Cache ) { // Draw from cache
        std::cout << "use cache" << std::endl;
	
	if ( mMapCanvas->layerCount() != mNumCachedLayers ) {
	    cache();
	}
	
	painter.save();
	painter.translate ( QCanvasRectangle::x(), QCanvasRectangle::y() );
        std::cout << "translate: " << QCanvasRectangle::x() << ", " << QCanvasRectangle::y() << std::endl;
	painter.drawPixmap(0,0, *mCachePixmap);

	painter.restore();

    } else if ( (plotStyle() == QgsComposition::Preview && mPreviewMode == Render) || 
	         plotStyle() == QgsComposition::Print ) 
    {
        std::cout << "render" << std::endl;
	
	painter.save();
	painter.translate ( QCanvasRectangle::x(), QCanvasRectangle::y() );
	render( &painter );
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
}

void QgsComposerVectorLegend::changeFont ( void ) 
{
    bool result;

    mFont = QFontDialog::getFont(&result, mFont, this );

    if ( result ) {
	recalculate();
	QCanvasRectangle::update();
	QCanvasRectangle::canvas()->update();
        writeSettings();
    }
}

void QgsComposerVectorLegend::previewModeChanged ( int i )
{
    //mPreviewMode = (PreviewMode) mPreviewModeComboBox->currentItem();
    mPreviewMode = (PreviewMode) i;
    std::cout << "mPreviewMode = " << mPreviewMode << std::endl;
    writeSettings();
}

void QgsComposerVectorLegend::recalculate ( void ) 
{
    std::cout << "QgsComposerVectorLegend::recalculate" << std::endl;
    
    // Recalculate sizes according to current font size
    
    // Title and section font 
    mTitleFont = mFont;
    mTitleFont.setPointSize ( (int) (1.4 * mFont.pointSize()) );
    mSectionFont = mFont;
    mSectionFont.setPointSize ( (int) (1.2 * mFont.pointSize()) );
    
    std::cout << "font size = " << mFont.pointSize() << std::endl;
    std::cout << "title font size = " << mTitleFont.pointSize() << std::endl;

    // Font size in canvas units
    int size = (int) ( 25.4 * mComposition->scale() * mFont.pointSize() / 72);

    mMargin = (int) ( 0.9 * size );
    mSymbolHeight = (int) ( 1.3 * size );
    mSymbolWidth = (int) ( 3.5 * size );
    mSymbolSpace = (int) ( 0.4 * size );

    std::cout << "mMargin = " << mMargin << " mSymbolHeight = " << mSymbolHeight
              << "mSymbolWidth = " << mSymbolWidth << " mSymbolSpace = " << mSymbolSpace << std::endl;
     
    int nlayers = mMapCanvas->layerCount();

    QRect r = render(0);

    QCanvasRectangle::setSize ( r.width(), r.height() );
    
    setOptions();
    cache();
}

void QgsComposerVectorLegend::setOptions ( void )
{ 
}

void QgsComposerVectorLegend::setSelected (  bool s ) 
{
    mSelected = s;
    QCanvasRectangle::update(); // show highlight
}    

bool QgsComposerVectorLegend::selected( void )
{
    return mSelected;
}

bool QgsComposerVectorLegend::writeSettings ( void )  
{
    QString path;
    path.sprintf("/composition_%d/vectorlegend_%d/", mComposition->id(), mId ); 
    QgsProject::instance()->writeEntry( "Compositions", path+"x", (int)QCanvasRectangle::x() );
    QgsProject::instance()->writeEntry( "Compositions", path+"y", (int)QCanvasRectangle::y() );
    
    QgsProject::instance()->writeEntry( "Compositions", path+"font/size", mFont.pointSize() );
    QgsProject::instance()->writeEntry( "Compositions", path+"font/family", mFont.family() );
    
    return true; 
}

bool QgsComposerVectorLegend::readSettings ( void )
{
    bool ok;
    QString path;
    path.sprintf("/composition_%d/vectorlegend_%d/", mComposition->id(), mId );

    QCanvasRectangle::setX( mComposition->fromMM(QgsProject::instance()->readDoubleEntry( "Compositions", path+"x", 0, &ok)) );
    QCanvasRectangle::setY( mComposition->fromMM(QgsProject::instance()->readDoubleEntry( "Compositions", path+"y", 0, &ok)) );
     
    mFont.setFamily ( QgsProject::instance()->readEntry("Compositions", path+"font/family", "", &ok) );
    mFont.setPointSize ( QgsProject::instance()->readNumEntry("Compositions", path+"font/size", 10, &ok) );
    
    recalculate();
    
    return true;
}

bool QgsComposerVectorLegend::writeXML( QDomNode & node, QDomDocument & document, bool temp )
{
    return true;
}

bool QgsComposerVectorLegend::readXML( QDomNode & node )
{
    return true;
}
