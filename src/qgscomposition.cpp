/***************************************************************************
                              qgscomposition.cpp
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
#include <iostream>

#include <qwidget.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qcanvas.h>
#include <qevent.h>
#include <qpoint.h>
#include <qrect.h>
#include <qwmatrix.h>
#include <qobjectlist.h>
#include <qdom.h>
#include <qstringlist.h>

#include "qgsmapcanvas.h"
#include "qgsproject.h"

#include "qgscomposer.h"
#include "qgscomposeritem.h"
#include "qgscomposerview.h"
#include "qgscomposition.h"
#include "qgscomposermap.h"
#include "qgscomposervectorlegend.h"
#include "qgscomposerlabel.h"

QgsComposition::QgsComposition( QgsComposer *c, int id ) 
{
    #ifdef QGISDEBUG
    std::cerr << "QgsComposition::QgsComposition()" << std::endl;
    #endif

    mId = id;
    mNextItemId = 1;
    mCanvas = 0;
    mPaperItem = 0;

    mComposer = c;
    mMapCanvas = c->mapCanvas();
    mView = c->view();
    mSelectedItem = 0;
    mPlotStyle = Preview;
    // Note: scale 10 make it inacceptable slow: QgsComposerMap 900x900mm on paper 1500x1000
    //          cannot be smoothly moved even if mPreviewMode == Rectangle and no zoom in
    //       scale 2 results in minimum line width 0.5 mmm which is too much
    //       scale 3 is compromise
    mScale = 3;

    // Potemkin's Village
    mPaperSizeComboBox->insertItem( "Custom" );
    mPaperUnitsComboBox->insertItem( "mm" );
    mPaperOrientationComboBox->insertItem( "Landscape" );

    // Map - A4 land for now, if future read from template
    mPaperWidth = 297;
    mPaperHeight = 210;
    
    mResolution = 72;

    setOptions();

    createCanvas();

    // Tool
    mRectangleItem = 0;
    mNewCanvasItem = 0;
    mTool = Select;
    mToolStep = 0;
}

void QgsComposition::createDefault(void) 
{
    // Map - A4 land for now, if future read from template
    mPaperWidth = 297;
    mPaperHeight = 210;
    // TODO: The resolution should be at least 300, but point layer is rescaled
    //       in Postscript if != 72
    //mResolution = 300;
    mResolution = 72;

    setOptions();

    resizeCanvas();

    // Add the map to coposition
    QgsComposerMap *m = new QgsComposerMap ( this, mNextItemId++, 
	                          mScale*15, mScale*15, mScale*180, mScale*180 );
    m->setUserExtent( mMapCanvas->extent());
    mItems.push_back(m);

    // Add vector legend
    QgsComposerVectorLegend *vl = new QgsComposerVectorLegend ( this, mNextItemId++, 
	                       mScale*210, mScale*100, mScale*5 );
    mItems.push_back(vl);

    // Title
    QgsComposerLabel *tit = new QgsComposerLabel ( this, mNextItemId++, 
	                                           mScale*238, mScale*40, "Map", mScale*7 );
    mItems.push_back(tit);

    // Tool
    mRectangleItem = 0;
    mNewCanvasItem = 0;
    mTool = Select;
    mToolStep = 0;
    
    writeSettings();

}

void QgsComposition::createCanvas(void) 
{
    if ( mCanvas ) delete mCanvas;
    
    mCanvas = new QCanvas ( (int) mPaperWidth * mScale, (int) mPaperHeight * mScale);
    mCanvas->setBackgroundColor( QColor(180,180,180) );
    // mCanvas->setDoubleBuffering( false );  // makes the move very unpleasant and doesn't make it faster

    // Paper
    if ( mPaperItem ) delete mPaperItem;
    mPaperItem = new QCanvasRectangle( 0, 0, (int) mPaperWidth * mScale, 
	                                     (int) mPaperHeight * mScale, mCanvas );
    mPaperItem->setBrush( QColor(255,255,255) );
    mPaperItem->setPen( QPen(QColor(0,0,0), 1) );
    mPaperItem->setZ(0);
    mPaperItem->setActive(false);
    mPaperItem->show();
}

void QgsComposition::resizeCanvas(void) 
{
    mCanvas->resize ( (int) mPaperWidth * mScale, (int) mPaperHeight * mScale );
    
    mPaperItem->setSize ( (int) mPaperWidth * mScale, (int) mPaperHeight * mScale );
}

QgsComposition::~QgsComposition()
{
    // TODO: Delete all objects!!!!
}

QgsMapCanvas *QgsComposition::mapCanvas(void) { return mMapCanvas; }

void QgsComposition::setActive (  bool active )
{
    if ( active ) {
	mView->setCanvas ( mCanvas );
	mComposer->showCompositionOptions ( this );
    } else {
	// TODO
    }
}

void QgsComposition::contentsMousePressEvent(QMouseEvent* e)
{
    std::cerr << "QgsComposition::contentsMousePressEvent() mTool = " << mTool << " mToolStep = "
                  << mToolStep << std::endl;

    QPoint p = mView->inverseWorldMatrix().map(e->pos());

    switch ( mTool ) {
	case Select:
	    {
		QCanvasItemList l = mCanvas->collisions(p);

		double x,y;
		mView->inverseWorldMatrix().map( e->pos().x(), e->pos().y(), &x, &y );

		QCanvasItem * newItem = 0;

		for (QCanvasItemList::Iterator it=l.begin(); it!=l.end(); ++it) {
		    if (! (*it)->isActive() ) continue;
		    newItem = *it;
		}

		if ( newItem ) { // found
		    if ( newItem != mSelectedItem ) { // Show options
			if ( mSelectedItem ) {
			    QgsComposerItem *coi = dynamic_cast <QgsComposerItem *> (mSelectedItem);
			    coi->setSelected ( false );
			}

			QgsComposerItem *coi = dynamic_cast <QgsComposerItem *> (newItem);
			coi->setSelected ( true );
			
			QWidget *w = dynamic_cast <QWidget *> (coi);
			mComposer->showItemOptions ( w );
			mSelectedItem = newItem;
		    } 
		    mLastX = x;
		    mLastY = y;
		} else { // not found
		    if ( mSelectedItem ) {
			QgsComposerItem *coi = dynamic_cast <QgsComposerItem *> (mSelectedItem);
			coi->setSelected ( false );
		    }
		    mSelectedItem = 0;
		    mComposer->showItemOptions ( (QWidget *) 0 ); // hide old options
		}
		mCanvas->update();
	    }
	    break;

	case AddMap:
	    std::cerr << "AddMap" << std::endl;
	    if ( mToolStep == 0 ) {
		mRectangleItem = new QCanvasRectangle( p.x(), p.y(), 0, 0, mCanvas );
		mRectangleItem->setBrush( Qt::NoBrush );
		mRectangleItem->setPen( QPen(QColor(0,0,0), 1) );
		mRectangleItem->setZ(100);
		mRectangleItem->setActive(false);
		mRectangleItem->show();
		mToolStep = 1;
	    }
	    std::cerr << "mToolStep = " << mToolStep << std::endl;
	    break;

	case AddVectorLegend:
	    {
	    mNewCanvasItem->setX( p.x() );
	    mNewCanvasItem->setY( p.y() );
	    QgsComposerVectorLegend *vl = dynamic_cast <QgsComposerVectorLegend*> (mNewCanvasItem);
            mItems.push_back(vl);
	    vl->writeSettings();
	    mNewCanvasItem = 0;
	    mComposer->selectItem(); // usually just one legend
	    
	    // Select and show options
	    vl->setSelected ( true );
	    QWidget *w = dynamic_cast <QWidget *> (vl);
	    mComposer->showItemOptions ( w );

	    mCanvas->update();
	    }
	    break;

	case AddLabel:
	    {
	    mNewCanvasItem->setX( p.x() );
	    mNewCanvasItem->setY( p.y() );
	    QgsComposerLabel *lab = dynamic_cast <QgsComposerLabel*> (mNewCanvasItem);
            mItems.push_back(lab);
	    lab->writeSettings();
	    mNewCanvasItem = 0;
	    mComposer->selectItem(); // usually just one ???
	    
	    // Select and show options
	    lab->setSelected ( true );
	    QWidget *w = dynamic_cast <QWidget *> (lab);
	    mComposer->showItemOptions ( w );

	    mCanvas->update();
	    }
	    break;
    }
}

void QgsComposition::contentsMouseMoveEvent(QMouseEvent* e)
{
    std::cerr << "QgsComposition::contentsMouseMoveEvent() mTool = " << mTool << " mToolStep = "
                  << mToolStep << std::endl;

    QPoint p = mView->inverseWorldMatrix().map(e->pos());

    switch ( mTool ) {
	case Select:
	    if ( mSelectedItem ) {
		double x,y;
		mView->inverseWorldMatrix().map( e->pos().x(), e->pos().y(), &x, &y );
		std::cout << "move: " << x << ", " << y << std::endl;
		
		mSelectedItem->setX( mSelectedItem->x() + x - mLastX );
		mSelectedItem->setY( mSelectedItem->y() + y - mLastY );

		QgsComposerItem *ci = dynamic_cast <QgsComposerItem *> (mSelectedItem);
		ci->writeSettings();

		mLastX = x;
		mLastY = y;
		mCanvas->update();
	    }
	    break;

	case AddMap:
	    if ( mToolStep == 1 ) { // draw rectangle
		double x, y;
		int w, h;

		x = p.x() < mRectangleItem->x() ? p.x() : mRectangleItem->x();
		y = p.y() < mRectangleItem->y() ? p.y() : mRectangleItem->y();

		w = abs ( p.x() - (int)mRectangleItem->x() );
		h = abs ( p.y() - (int)mRectangleItem->y() );

		mRectangleItem->setX(x);
		mRectangleItem->setY(y);
		
		mRectangleItem->setSize(w,h);
		
		mCanvas->update();
    
		std::cerr << "x = " << x << " y = " << y << " w = " << w << " h = " << h << std::endl;
	    }
	    break;

	case AddVectorLegend:
	case AddLabel:
	    mNewCanvasItem->setX( p.x() );
	    mNewCanvasItem->setY( p.y() );
	    mCanvas->update();
	    break;
    }
}

void QgsComposition::contentsMouseReleaseEvent(QMouseEvent* e)
{
    std::cerr << "QgsComposition::contentsMouseReleaseEvent() mTool = " << mTool 
	      << " mToolStep = " << mToolStep << std::endl;

    QPoint p = mView->inverseWorldMatrix().map(e->pos());

    switch ( mTool ) {
	case AddMap: // mToolStep should be always 1 but rectangle can be 0 size
	    {
		int x = (int) mRectangleItem->x();
		int y = (int) mRectangleItem->y();
		int w = mRectangleItem->width();
		int h = mRectangleItem->height();
		delete mRectangleItem;
		mRectangleItem = 0;
		
		if ( w > 0 && h > 0 ) {
		    
		    QgsComposerMap *m = new QgsComposerMap ( this, mNextItemId++, x, y, w, h );
		    
		    m->setUserExtent( mMapCanvas->extent());
		    mItems.push_back(m);
		    m->setSelected ( true );

		    if ( mSelectedItem ) {
			QgsComposerItem *coi = dynamic_cast <QgsComposerItem *> (mSelectedItem);
			coi->setSelected ( false );
		    }

		    mSelectedItem = dynamic_cast <QCanvasItem *> (m);
		 
		    QWidget *w = dynamic_cast <QWidget *> (m);
		    mComposer->showItemOptions ( w );

		}
		mComposer->selectItem(); // usually just one map
		    
		mCanvas->setChanged ( QRect( x, y, w, h) ); // Should not be necessary
		mCanvas->update();
	    }
	    break;
    }
}

void QgsComposition::keyPressEvent ( QKeyEvent * e )
{
    std::cout << "QgsComposition::keyPressEvent() key = " << e->key() << std::endl;

    if ( e->key() == Qt::Key_Delete && mSelectedItem ) { // delete
	
	QgsComposerItem *coi = dynamic_cast <QgsComposerItem *> (mSelectedItem);
	//coi->setItemSelected ( false );
	coi->setSelected ( false );
	for (std::list < QgsComposerItem * >::iterator it = mItems.begin(); 
		                         it != mItems.end(); ++it) 
	{
	    if ( (*it) == coi ) {
		mItems.erase ( it );
		break;
	    }
	}
        std::cout << "mItems.size() = " << mItems.size() << std::endl;
	delete (mSelectedItem);
	mSelectedItem = 0;
	mCanvas->update();
    }
}

void QgsComposition::paperSizeChanged ( void )
{
    std::cout << "QgsComposition::paperSizeChanged" << std::endl;
    mPaperWidth = mPaperWidthLineEdit->text().toDouble();
    mPaperHeight = mPaperHeightLineEdit->text().toDouble();
    
    std::cout << "mPaperWidth = " << mPaperWidth << " mPaperHeight = " << mPaperHeight << std::endl;
    
    mCanvas->resize( (int) (mPaperWidth * mScale), (int) (mPaperHeight * mScale) );
    
    std::cout << "mCanvas width = " << mCanvas->width() << " height = " << mCanvas->height() << std::endl;
    
    mPaperItem->setSize((int) (mPaperWidth * mScale), (int) (mPaperHeight * mScale) );

    mComposer->zoomFull();
    mView->repaint(); // Does not repaint the view !
    mView->update();  // Does not repaint the view !
    mView->QWidget::repaint();
    writeSettings();
}

void QgsComposition::resolutionChanged ( void )
{
    mResolution = mResolutionLineEdit->text().toInt();
    writeSettings();
}

void QgsComposition::setOptions ( void )
{
    mPaperWidthLineEdit->setText ( QString("%1").arg(mPaperWidth,0,'g') );
    mPaperHeightLineEdit->setText ( QString("%1").arg(mPaperHeight,0,'g') );
    mResolutionLineEdit->setText ( QString("%1").arg(mResolution) );
}

void QgsComposition::setPlotStyle (  PlotStyle p )
{
    mPlotStyle = p;

    // Set all items
    for (std::list < QgsComposerItem * >::iterator it = mItems.begin(); it != mItems.end(); ++it) {
	(*it)->setPlotStyle( p ) ;
    }
}

double QgsComposition::viewScale ( void ) 
{
    double scale = mView->worldMatrix().m11();
    std::cout << "viewScale = " << scale << std::endl;
    return scale; 
}

int QgsComposition::id ( void ) { return mId; }

QgsComposer *QgsComposition::composer(void) { return mComposer; }

QCanvas *QgsComposition::canvas(void) { return mCanvas; }

double QgsComposition::paperWidth ( void ) { return mPaperWidth; }

double QgsComposition::paperHeight ( void ) { return mPaperHeight; }

int QgsComposition::resolution ( void ) { return mResolution; }

int QgsComposition::scale( void ) { return mScale; }

double QgsComposition::toMM ( int v ) { return v/mScale ; }

int QgsComposition::fromMM ( double v ) { return (int) (v * mScale); }

void QgsComposition::setTool ( Tool tool )
{
    // Stop old in progress
    mView->viewport()->setMouseTracking ( false ); // stop mouse tracking
    if ( mSelectedItem ) {
        QgsComposerItem *coi = dynamic_cast <QgsComposerItem *> (mSelectedItem);
	coi->setSelected ( false );
	mCanvas->update();
    }
    mSelectedItem = 0;
    mComposer->showItemOptions ( (QWidget *) 0 );
    
    if ( mNewCanvasItem ) {
	mNewCanvasItem->setX( -1000 );
	mNewCanvasItem->setY( -1000 );
	mCanvas->update();

	delete mNewCanvasItem;
	mNewCanvasItem = 0;
    }

    if ( mRectangleItem ) {
	delete mRectangleItem;
	mRectangleItem = 0;
    }
    
    // Start new
    if ( tool == AddVectorLegend ) { // Create temporary object
	if ( mNewCanvasItem ) delete mNewCanvasItem;
	
	// Create new object outside the visible area
	QgsComposerVectorLegend *vl = new QgsComposerVectorLegend ( this, mNextItemId++, 
		                                (-1000)*mScale, (-1000)*mScale, (int) (mScale*mPaperHeight/50) );
        mNewCanvasItem = dynamic_cast <QCanvasItem *> (vl);

	QWidget *w = dynamic_cast <QWidget *> (vl);
	mComposer->showItemOptions ( w );

	mView->viewport()->setMouseTracking ( true ); // to recieve mouse move
    } else if ( tool == AddLabel ) {
	if ( mNewCanvasItem ) delete mNewCanvasItem;
	
	// Create new object outside the visible area
	QgsComposerLabel *lab = new QgsComposerLabel ( this, mNextItemId++, 
	                                (-1000)*mScale, (-1000)*mScale, "Label", (int) (mScale*mPaperHeight/40) );
        mNewCanvasItem = dynamic_cast <QCanvasItem *> (lab);

	QWidget *w = dynamic_cast <QWidget *> (lab);
	mComposer->showItemOptions ( w );

	mView->viewport()->setMouseTracking ( true ); // to recieve mouse move
    }
    
    mTool = tool;
    mToolStep = 0;
}

int QgsComposition::selectionBoxSize ( void )
{
    // Scale rectangle, keep rectangle of fixed size in screen points
    return (int) (7/viewScale());
}

QPen QgsComposition::selectionPen ( void ) 
{
    return QPen( QColor(0,0,255), 0) ;
}

QBrush QgsComposition::selectionBrush ( void )
{
    return QBrush ( QBrush(QColor(0,0,255), Qt::SolidPattern) );
}

bool QgsComposition::writeSettings ( void )
{
    QString path;
    path.sprintf("/composition_%d/width", mId );
    QgsProject::instance()->writeEntry( "Compositions", path, mPaperWidth );
    path.sprintf("/composition_%d/height", mId );
    QgsProject::instance()->writeEntry( "Compositions", path, mPaperHeight );
    
    path.sprintf("/composition_%d/resolution", mId );
    QgsProject::instance()->writeEntry( "Compositions", path, mResolution );
    
    return true;
}

bool QgsComposition::readSettings ( void )
{
    std::cout << "QgsComposition::readSettings" << std::endl;

    bool ok;
    
    QString path;
    path.sprintf("/composition_%d/width", mId );
    mPaperWidth = QgsProject::instance()->readDoubleEntry( "Compositions", path, 297, &ok);
    path.sprintf("/composition_%d/height", mId );
    mPaperHeight = QgsProject::instance()->readDoubleEntry( "Compositions", path, 210, &ok);
    
    path.sprintf("/composition_%d/resolution", mId );
    mResolution = QgsProject::instance()->readNumEntry( "Compositions", path, 300, &ok);
    
    resizeCanvas();

    // TODO: read all objects
    // BUG in readListEntry
    /*
    path.sprintf("/composition_%d", mId );
    QStringList el = QgsProject::instance()->readListEntry ( "Compositions", path, &ok );

    for ( QStringList::iterator it = el.begin(); it != el.end(); ++it ) {
	std::cout << "entry: " << (*it).ascii() << std::endl;

    }
    */
    
    setOptions();
    mCanvas->update();

    return true;
}
