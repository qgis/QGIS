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
#include <vector>

#include <qwidget.h>
#include <qrect.h>
#include <qcombobox.h>
#include <qdom.h>
#include <qcanvas.h>
#include <qpainter.h>
#include <qstring.h>
#include <qpixmap.h>
#include <qpicture.h>
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
#include "qgssvgcache.h"
#include "qgsmarkercatalogue.h"

QgsComposerVectorLegend::QgsComposerVectorLegend ( QgsComposition *composition, int id, 
	                                            int x, int y, int fontSize )
    : QCanvasRectangle(x,y,10,10,0)
{
    std::cout << "QgsComposerVectorLegend::QgsComposerVectorLegend()" << std::endl;

    mComposition = composition;
    mId  = id;
    mMapCanvas = mComposition->mapCanvas();

    init();

    // Font and pen 
    mFont.setPointSize ( fontSize );
    
    // Set map to the first available if any
    std::vector<QgsComposerMap*> maps = mComposition->maps();
    if ( maps.size() > 0 ) {
	mMap = maps[0]->id();
    }

    // Calc size and cache
    recalculate();

    // Add to canvas
    setCanvas(mComposition->canvas());

    QCanvasRectangle::show();
    QCanvasRectangle::update();

     
    writeSettings();
}

QgsComposerVectorLegend::QgsComposerVectorLegend ( QgsComposition *composition, int id ) 
    : QCanvasRectangle(0,0,10,10,0)
{
    std::cout << "QgsComposerVectorLegend::QgsComposerVectorLegend()" << std::endl;

    mComposition = composition;
    mId  = id;
    mMapCanvas = mComposition->mapCanvas();

    init();

    readSettings();

    // Calc size and cache
    recalculate();

    // Add to canvas
    setCanvas(mComposition->canvas());

    QCanvasRectangle::show();
    QCanvasRectangle::update();
}

void QgsComposerVectorLegend::init ( void ) 
{
    mSelected = false;
    mNumCachedLayers = 0;
    mTitle = "Legend";
    mMap = 0;
    mNextLayerGroup = 1;

    // Cache
    mCacheUpdated = false;

    // Rectangle
    QCanvasRectangle::setZ(50);
    setActive(true);

    // Layers list view
    mLayersListView->setColumnText(0,tr("Layers"));
    mLayersListView->addColumn(tr("Group"));
    mLayersListView->setSorting(-1);
    mLayersListView->setResizeMode(QListView::AllColumns);
    mLayersListView->setSelectionMode(QListView::Extended);

    mLayersPopupMenu = new QPopupMenu( );

    mLayersPopupMenu->insertItem( "Combine selected layers", this, SLOT(groupLayers()) );

    connect ( mLayersListView, SIGNAL(clicked(QListViewItem *)), 
	                       this, SLOT(layerChanged(QListViewItem *)));

    connect ( mLayersListView, SIGNAL(rightButtonClicked(QListViewItem *, const QPoint &, int)), 
		                this, SLOT( showLayersPopupMenu(QListViewItem *, const QPoint &, int)) );

    // Plot style
    setPlotStyle ( QgsComposition::Preview );
    
    // Preview style
    mPreviewMode = Render;
    mPreviewModeComboBox->insertItem ( "Cache", Cache );
    mPreviewModeComboBox->insertItem ( "Render", Render );
    mPreviewModeComboBox->insertItem ( "Rectangle", Rectangle );
    mPreviewModeComboBox->setCurrentItem ( mPreviewMode );

    connect ( mComposition, SIGNAL(mapChanged(int)), this, SLOT(mapChanged(int)) ); 
}

QgsComposerVectorLegend::~QgsComposerVectorLegend()
{
    std::cerr << "QgsComposerVectorLegend::~QgsComposerVectorLegend()" << std::endl;
}

QRect QgsComposerVectorLegend::render ( QPainter *p )
{
    std::cout << "QgsComposerVectorLegend::render p = " << p << std::endl;

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
    // Font size in canvas units
    int titleSize = (int) ( 25.4 * mComposition->scale() * mTitleFont.pointSize() / 72);
    int sectionSize = (int) ( 25.4 * mComposition->scale() * mSectionFont.pointSize() / 72);
    int size = (int) ( 25.4 * mComposition->scale() * mFont.pointSize() / 72);

    std::cout << "font sizes = " << titleSize << " " << sectionSize << " " << size << std::endl;

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

    // Not sure about Style Strategy, QFont::PreferMatch?
    titleFont.setStyleStrategy ( (QFont::StyleStrategy) (QFont::PreferOutline | QFont::PreferAntialias) );
    sectionFont.setStyleStrategy ( (QFont::StyleStrategy) (QFont::PreferOutline | QFont::PreferAntialias) );
    font.setStyleStrategy ( (QFont::StyleStrategy) (QFont::PreferOutline | QFont::PreferAntialias) );

    int x, y;

    // Title
    y = mMargin + titleMetrics.height();
    painter->setPen ( mPen );
    painter->setFont ( titleFont );
    painter->drawText( (int) (2*mMargin), y, mTitle );
    int width = 2 * mMargin + titleMetrics.width ( mTitle ); 
    int height = mMargin + mSymbolSpace + titleMetrics.height(); // mSymbolSpace?
    
    // Layers
    QgsComposerMap *map = mComposition->map ( mMap );
    if ( map ) {
	std::map<int,int> doneGroups;
	
	int nlayers = mMapCanvas->layerCount();
	for ( int i = nlayers - 1; i >= 0; i-- ) {
            std::cout << "layer = " << i << std::endl;
	    QgsMapLayer *layer = mMapCanvas->getZpos(i);
	    if ( !layer->visible() ) continue;
	    if ( layer->type() != QgsMapLayer::VECTOR ) continue;

	    QString layerId = layer->getLayerID();
	    if( ! layerOn(layerId) ) continue;
		    
	    int group = layerGroup ( layerId );
            std::cout << "group = " << group << std::endl;
	    if ( group > 0 ) { 
		//std::map<int,int>::iterator it= doneGroups.find();
	        if ( doneGroups.find(group) != doneGroups.end() ) {
                    std::cout << "already done" << std::endl;
		    continue; 
		} else {
                    std::cout << "do the group" << std::endl;
		    doneGroups.insert(std::make_pair(group,1));
		}
	    }

	    /* Make list of all layers in the group and count section items */
	    std::vector<int> groupLayers; // vector of layers
	    std::vector<int> itemHeights; // maximum item sizes
	    std::vector<QString> itemLabels; // item labels
	    int sectionItemsCount = 0;
	    QString sectionTitle;

	    for ( int j = nlayers - 1; j >= 0; j-- ) {
                std::cout << "layer = " << j << std::endl;
		QgsMapLayer *layer2 = mMapCanvas->getZpos(j);
		if ( !layer2->visible() ) continue;
		if ( layer2->type() != QgsMapLayer::VECTOR ) continue;
	    
		QString layerId2 = layer2->getLayerID();;
	        if( ! layerOn(layerId2) ) continue;
		
		int group2 = layerGroup ( layerId2 );
		
		QgsVectorLayer *vector = dynamic_cast <QgsVectorLayer*> (layer2);
		QgsRenderer *renderer = vector->renderer();

		// QgsContinuousColRenderer is not supported yet
		// QgsSiMaRenderer, QgsGraduatedMaRenderer, QgsUValMaRenderer no more
		if ( typeid (*renderer) == typeid(QgsContinuousColRenderer) ||
		     typeid (*renderer) == typeid(QgsSiMaRenderer) ||
		     typeid (*renderer) == typeid(QgsGraduatedMaRenderer) ||
		     typeid (*renderer) == typeid(QgsUValMaRenderer) )
		{ 
		    continue;
		}

		if ( (group > 0 && group2 == group) || ( group == 0 && j == i )  ) {
                    std::cout << "add to group" << std::endl;

		    groupLayers.push_back(j);

		    std::list<QgsRenderItem*> items = renderer->items();

		    if ( sectionTitle.length() == 0 ) {
		       sectionTitle = layer2->name();
		    }	       
			    
		    if ( items.size() > sectionItemsCount ) {
			sectionItemsCount = items.size();
		        itemHeights.resize(sectionItemsCount);
		        itemLabels.resize(sectionItemsCount);	
		    }
		
		    int icnt = 0;
		    for ( std::list<QgsRenderItem*>::iterator it = items.begin(); it != items.end(); ++it ) {
			double scale = map->symbolScale() * mComposition->scale();
		    
			QgsRenderItem *ri = (*it);
		        QgsSymbol *sym = ri->getSymbol();
			
			// height
			if ( itemHeights[icnt] < mSymbolHeight ) { // init first
			    itemHeights[icnt] = mSymbolHeight;
			}
			QPicture pic = sym->getPointSymbolAsPicture();
			QRect br = pic.boundingRect();

			int h = (int) ( scale * br.height() );
			if ( h > itemHeights[icnt] ) {
			    itemHeights[icnt] = h;
			}

			if ( itemLabels[icnt].length() == 0 ) {
			    if ( ri->label().length() > 0 ) {
				itemLabels[icnt] = ri->label();
			    } else {
				itemLabels[icnt] = ri->value();
			    }
			}
			    
			icnt++;
		    }
		}
	    }
            std::cout << "group size = " << groupLayers.size() << std::endl;
            std::cout << "sectionItemsCount = " << sectionItemsCount << std::endl;

	    // Section title 
	    if ( sectionItemsCount > 1 ) {
		height += mSymbolSpace;

		x = (int) ( 2*mMargin );
		y = (int) ( height + sectionMetrics.height() );
		painter->setPen ( mPen );
		painter->setFont ( sectionFont );
		painter->drawText( x, y, sectionTitle );	

		int w = 3*mMargin + sectionMetrics.width( sectionTitle );
		if ( w > width ) width = w;
		height += sectionMetrics.height();
		height += (int) (0.7*mSymbolSpace);
	    }

	    // Draw all layers in group 
	    int groupStartHeight = height;
	    for ( int j = groupLayers.size()-1; j >= 0; j-- ) {
		std::cout << "layer = " << groupLayers[j] << std::endl;

		int localHeight = groupStartHeight;
		
		layer = mMapCanvas->getZpos(groupLayers[j]);
		QgsVectorLayer *vector = dynamic_cast <QgsVectorLayer*> (layer);
		QgsRenderer *renderer = vector->renderer();
		
		// Symbol
		std::list<QgsRenderItem*> items = renderer->items();
		
		int icnt = 0;
		for ( std::list<QgsRenderItem*>::iterator it = items.begin(); it != items.end(); ++it ) {
		    localHeight += mSymbolSpace;

		    int symbolHeight = itemHeights[icnt];

		    QgsRenderItem *ri = (*it);

		    QgsSymbol *sym = ri->getSymbol();
		    
		    QPen pen = sym->pen();
		    double widthScale = map->widthScale() * mComposition->scale();
		    if ( plotStyle() == QgsComposition::Preview && mPreviewMode == Render ) {
			widthScale *= mComposition->viewScale();
		    }
		    pen.setWidth ( (int)  ( widthScale * pen.width() ) );
		    painter->setPen ( pen );
		    painter->setBrush ( sym->brush() );
		    
		    if ( vector->vectorType() == QGis::Point ) {
			double scale = map->symbolScale() * mComposition->scale();

			// Get the picture of appropriate size directly from catalogue
			QPicture pic = QgsMarkerCatalogue::instance()->marker (
					sym->pointSymbolName(), (int) (scale * sym->pointSize()),
					pen, sym->brush() );
					
			QRect br = pic.boundingRect();
			    
			painter->drawPicture ( static_cast<int>( (mMargin+mSymbolWidth/2)-br.width()/2),
					       static_cast<int>( (localHeight+symbolHeight/2)-br.height()/2),
					       pic );

		    } else if ( vector->vectorType() == QGis::Line ) {
			painter->drawLine ( mMargin, localHeight+mSymbolHeight/2, 
					    mMargin+mSymbolWidth, localHeight+mSymbolHeight/2 );
		    } else if ( vector->vectorType() == QGis::Polygon ) {
			painter->drawRect ( mMargin, localHeight, mSymbolWidth, mSymbolHeight );
		    }

		    // Label 
		    painter->setPen ( mPen );
		    painter->setFont ( font );
		    QString lab;
		    if ( sectionItemsCount == 1 ) {
			lab = sectionTitle;
		    } else { 
			lab = itemLabels[icnt];
		    }
		    
		    QRect br = metrics.boundingRect ( lab );
		    x = (int) ( 2*mMargin + mSymbolWidth );
		    y = (int) ( localHeight + symbolHeight/2 - br.height()/2 );
		    painter->drawText( x, y, br.width(), br.height(), Qt::AlignLeft|Qt::AlignVCenter, lab );	

		    int w = 3*mMargin + mSymbolWidth + metrics.width(lab);
		    if ( w > width ) width = w;

		    localHeight += symbolHeight;
		    icnt++;
		}
	    }
	    /* add height of section items */
	    height = groupStartHeight;
	    for ( int j = 0; j < itemHeights.size(); j++ ) {
		height += mSymbolSpace + itemHeights[j];
	    }
	    if ( sectionItemsCount > 1 ) { // add more space to separate section from next item
	        height += mSymbolSpace;
	    }	
	}
    }

    height += mMargin;
    
    QCanvasRectangle::setSize (  width, height );
    
    if ( !p ) {
	delete painter;
	delete pixmap;
    }

    return QRect ( 0, 0, width, height);
}

void QgsComposerVectorLegend::cache ( void )
{
    std::cout << "QgsComposerVectorLegend::cache()" << std::endl;

    mCachePixmap.resize ( QCanvasRectangle::width(), QCanvasRectangle::height() ); 

    QPainter p(&mCachePixmap);
    
    mCachePixmap.fill(QColor(255,255,255));
    render ( &p );
    p.end();

    mNumCachedLayers = mMapCanvas->layerCount();
    mCacheUpdated = true;    
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
	
	if ( !mCacheUpdated || mMapCanvas->layerCount() != mNumCachedLayers ) {
	    cache();
	}
	
	painter.save();
	painter.translate ( QCanvasRectangle::x(), QCanvasRectangle::y() );
        std::cout << "translate: " << QCanvasRectangle::x() << ", " << QCanvasRectangle::y() << std::endl;
	painter.drawPixmap(0,0, mCachePixmap);

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

void QgsComposerVectorLegend::titleChanged (  )
{
    mTitle = mTitleLineEdit->text();
    recalculate();
    QCanvasRectangle::update();
    QCanvasRectangle::canvas()->update();
    writeSettings();
}

void QgsComposerVectorLegend::previewModeChanged ( int i )
{
    mPreviewMode = (PreviewMode) i;
    std::cout << "mPreviewMode = " << mPreviewMode << std::endl;
    writeSettings();
}

void QgsComposerVectorLegend::mapSelectionChanged ( int i )
{
    mMap = mMaps[i];
    recalculate();
    QCanvasRectangle::update();
    QCanvasRectangle::canvas()->update();
    writeSettings();
}

void QgsComposerVectorLegend::mapChanged ( int id )
{
    if ( id != mMap ) return;

    recalculate();
    QCanvasRectangle::update();
    QCanvasRectangle::canvas()->update();
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
    
    mCacheUpdated = false;
}

void QgsComposerVectorLegend::setOptions ( void )
{ 
    mTitleLineEdit->setText( mTitle );
    
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
    
    // Layers
    mLayersListView->clear();

    if ( mMap != 0 ) {
	QgsComposerMap *map = mComposition->map ( mMap );

	if ( map ) {
	    int nlayers = mMapCanvas->layerCount();
	    for ( int i = 0; i < nlayers; i++ ) {
		QgsMapLayer *layer = mMapCanvas->getZpos(i);
		
		if ( !layer->visible() ) continue;
		//if ( layer->type() != QgsMapLayer::VECTOR ) continue;

		QCheckListItem *li = new QCheckListItem ( mLayersListView, layer->name(), QCheckListItem::CheckBox );

		QString id = layer->getLayerID();
		li->setText(2, id );

		li->setOn ( layerOn(id) );
		
		int group = layerGroup(id);
		if ( group > 0 ) {
		    li->setText(1, QString::number(group) );
		}

		mLayersListView->insertItem ( li );
	    }
	}
    }
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

void QgsComposerVectorLegend::showLayersPopupMenu ( QListViewItem * lvi, const QPoint & pt, int )
{
    std::cout << "QgsComposerVectorLegend::showLayersPopupMenu" << std::endl;

    mLayersPopupMenu->exec(pt);
}

bool QgsComposerVectorLegend::layerOn ( QString id ) 
{
    std::map<QString,bool>::iterator it = mLayersOn.find(id);

    if(it != mLayersOn.end() ) {
	return ( it->second );
    }

    return true;
}

void QgsComposerVectorLegend::setLayerOn ( QString id, bool on ) 
{
    std::map<QString,bool>::iterator it = mLayersOn.find(id);

    if(it != mLayersOn.end() ) {
	it->second = on;
    } else {
	mLayersOn.insert(std::make_pair(id,on));
    }
}

int QgsComposerVectorLegend::layerGroup ( QString id ) 
{
    std::map<QString,int>::iterator it = mLayersGroups.find(id);

    if(it != mLayersGroups.end() ) {
	return ( it->second );
    }

    return 0;
}

void QgsComposerVectorLegend::setLayerGroup ( QString id, int group ) 
{
    std::map<QString,int>::iterator it = mLayersGroups.find(id);

    if(it != mLayersGroups.end() ) {
	it->second = group;
    } else {
	mLayersGroups.insert(std::make_pair(id,group));
    }
}

void QgsComposerVectorLegend::layerChanged ( QListViewItem *lvi )
{
    std::cout << "QgsComposerVectorLegend::layerChanged" << std::endl;

    if ( lvi == 0 ) return; 
    
    QString id = lvi->text(2);
    QCheckListItem *cli = dynamic_cast <QCheckListItem *>(lvi);
    setLayerOn ( id, cli->isOn() );

    writeSettings();

    recalculate();
    QCanvasRectangle::update();
    QCanvasRectangle::canvas()->update();
}

void QgsComposerVectorLegend::groupLayers ( void ) 
{
    std::cout << "QgsComposerVectorLegend::groupLayers" << std::endl;

    QListViewItemIterator it( mLayersListView );
    int count = 0;
    QListViewItem *lastItem;
    QString id;
    while ( it.current() ) {
        if ( it.current()->isSelected() ) {
	    std::cout << "selected: " << it.current()->text(0) << " " << it.current()->text(2) << std::endl;

	    id = it.current()->text(2);
	    setLayerGroup ( id, mNextLayerGroup );
	    it.current()->setText(1,QString::number(mNextLayerGroup) );
	    lastItem = it.current();
	    count++;
	}
	++it;
    }
    if ( count == 1 ) { // one item only
        setLayerGroup ( id, 0 );
	lastItem->setText(1,"" );
    }
	
    std::cout << "Groups:" << std::endl;

    for ( std::map<QString,int>::iterator it3 = mLayersGroups.begin(); it3 != mLayersGroups.end(); ++it3 ) {
	std::cout << "layer: " << it3->first << " group: " << it3->second << std::endl;
    }
    
    mNextLayerGroup++;
    
    writeSettings();

    recalculate();
    QCanvasRectangle::update();
    QCanvasRectangle::canvas()->update();
}    

QWidget *QgsComposerVectorLegend::options ( void )
{
    setOptions ();
    return ( dynamic_cast <QWidget *> (this) ); 
}

bool QgsComposerVectorLegend::writeSettings ( void )  
{
    std::cout << "QgsComposerVectorLegend::writeSettings" << std::endl;
    QString path;
    path.sprintf("/composition_%d/vectorlegend_%d/", mComposition->id(), mId ); 

    QgsProject::instance()->writeEntry( "Compositions", path+"x", mComposition->toMM((int)QCanvasRectangle::x()) );
    QgsProject::instance()->writeEntry( "Compositions", path+"y", mComposition->toMM((int)QCanvasRectangle::y()) );

    QgsProject::instance()->writeEntry( "Compositions", path+"map", mMap );

    QgsProject::instance()->writeEntry( "Compositions", path+"title", mTitle );

    QgsProject::instance()->writeEntry( "Compositions", path+"font/size", mFont.pointSize() );
    QgsProject::instance()->writeEntry( "Compositions", path+"font/family", mFont.family() );
    QgsProject::instance()->writeEntry( "Compositions", path+"font/weight", mFont.weight() );
    QgsProject::instance()->writeEntry( "Compositions", path+"font/underline", mFont.underline() );
    QgsProject::instance()->writeEntry( "Compositions", path+"font/strikeout", mFont.strikeOut() );

    // Layers: remove all, write new
    path.sprintf("/composition_%d/vectorlegend_%d/layers/", mComposition->id(), mId ); 
    QgsProject::instance()->removeEntry ( "Compositions", path );
    
    if ( mMap != 0 ) {
	QgsComposerMap *map = mComposition->map ( mMap );

	if ( map ) {
	    int nlayers = mMapCanvas->layerCount();
	    for ( int i = 0; i < nlayers; i++ ) {
		QgsMapLayer *layer = mMapCanvas->getZpos(i);
		
		if ( !layer->visible() ) continue;

		QString id = layer->getLayerID();
                path.sprintf("/composition_%d/vectorlegend_%d/layers/layer_%s/", mComposition->id(), mId, id.ascii() ); 
		QgsProject::instance()->writeEntry( "Compositions", path+"on", layerOn(id) );
		QgsProject::instance()->writeEntry( "Compositions", path+"group", layerGroup(id) );
	    }
	}
    }

    QgsProject::instance()->writeEntry( "Compositions", path+"previewmode", mPreviewMode );
    
    return true; 
}

bool QgsComposerVectorLegend::readSettings ( void )
{
    bool ok;
    QString path;
    path.sprintf("/composition_%d/vectorlegend_%d/", mComposition->id(), mId );

    QCanvasRectangle::setX( mComposition->fromMM(QgsProject::instance()->readDoubleEntry( "Compositions", path+"x", 0, &ok)) );
    QCanvasRectangle::setY( mComposition->fromMM(QgsProject::instance()->readDoubleEntry( "Compositions", path+"y", 0, &ok)) );

    mMap = QgsProject::instance()->readNumEntry("Compositions", path+"map", 0, &ok);
    mTitle = QgsProject::instance()->readEntry("Compositions", path+"title", "???", &ok);
     
    mFont.setFamily ( QgsProject::instance()->readEntry("Compositions", path+"font/family", "", &ok) );
    mFont.setPointSize ( QgsProject::instance()->readNumEntry("Compositions", path+"font/size", 10, &ok) );
    mFont.setWeight(  QgsProject::instance()->readNumEntry("Compositions", path+"font/weight", (int)QFont::Normal, &ok) );
    mFont.setUnderline(  QgsProject::instance()->readBoolEntry("Compositions", path+"font/underline", false, &ok) );
    mFont.setStrikeOut(  QgsProject::instance()->readBoolEntry("Compositions", path+"font/strikeout", false, &ok) );

    // Layers
    path.sprintf("/composition_%d/vectorlegend_%d/layers/", mComposition->id(), mId );
    QStringList el = QgsProject::instance()->subkeyList ( "Compositions", path );
    
    for ( QStringList::iterator it = el.begin(); it != el.end(); ++it ) {
	int idx = (*it).find('_');

	QString id = (*it).right( (*it).length() - (idx+1) );
	
	path.sprintf("/composition_%d/vectorlegend_%d/layers/layer_%s/", mComposition->id(), mId, id.ascii() );
	bool on = QgsProject::instance()->readBoolEntry("Compositions", path+"on", true, &ok);
	int group = QgsProject::instance()->readNumEntry("Compositions", path+"group", 0, &ok);
	setLayerOn ( id , on );
	setLayerGroup ( id, group );

	if ( group >= mNextLayerGroup ) mNextLayerGroup = group+1;
    }
    
    // Preview mode
    mPreviewMode = (PreviewMode) QgsProject::instance()->readNumEntry("Compositions", path+"previewmode", Render, &ok);
    
    recalculate();
    
    return true;
}

bool QgsComposerVectorLegend::removeSettings( void )
{
    std::cerr << "QgsComposerVectorLegend::deleteSettings" << std::endl;

    QString path;
    path.sprintf("/composition_%d/vectorlegend_%d", mComposition->id(), mId ); 
    return QgsProject::instance()->removeEntry ( "Compositions", path );
}

bool QgsComposerVectorLegend::writeXML( QDomNode & node, QDomDocument & document, bool temp )
{
    return true;
}

bool QgsComposerVectorLegend::readXML( QDomNode & node )
{
    return true;
}
