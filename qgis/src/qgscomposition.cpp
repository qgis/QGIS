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

QgsCompositionPaper::QgsCompositionPaper ( QString name, int w, int h, bool c)
{
        mName = name; mWidth = w; mHeight = h; mCustom = c;
}

QgsCompositionPaper::~QgsCompositionPaper ( )
{
}

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
    mScale = 5;

    // Add paper sizes and set default. 
    mPapers.push_back ( QgsCompositionPaper( tr("Custom"), 0, 0, 1 ) );
    mPapers.push_back ( QgsCompositionPaper( tr("A5 (148x210 mm)"), 148, 210 ) );
    mPapers.push_back ( QgsCompositionPaper( tr("A4 (210x297 mm)"), 210, 297 ) );
    mPapers.push_back ( QgsCompositionPaper( tr("A3 (297x420 mm)"), 297, 420 ) );
    mPapers.push_back ( QgsCompositionPaper( tr("A2 (420x594 mm)"), 420, 594 ) );
    mPapers.push_back ( QgsCompositionPaper( tr("A1 (594x841 mm)"), 594, 841 ) );
    mPapers.push_back ( QgsCompositionPaper( tr("A0 (841x1189 mm)"), 841, 1189 ) );
    mPapers.push_back ( QgsCompositionPaper( tr("B5 (176 x 250 mm)"), 176, 250 ) );
    mPapers.push_back ( QgsCompositionPaper( tr("B4 (250 x 353 mm)"), 250, 353 ) );
    mPapers.push_back ( QgsCompositionPaper( tr("B3 (353 x 500 mm)"), 353, 500 ) );
    mPapers.push_back ( QgsCompositionPaper( tr("B2 (500 x 707 mm)"), 500, 707 ) );
    mPapers.push_back ( QgsCompositionPaper( tr("B1 (707 x 1000 mm)"), 707, 1000 ) );
    mPapers.push_back ( QgsCompositionPaper( tr("B0 (1000 x 1414 mm)"), 1000, 1414 ) );
    mPapers.push_back ( QgsCompositionPaper( tr("Letter (8.5x11 inches)"),  216, 279 ) );
    mPapers.push_back ( QgsCompositionPaper( tr("Legal (8.5x14 inches)"), 216, 356 ) );

    mDefaultPaper = mCustomPaper = 0;
    for( int i = 0; i < mPapers.size(); i++ ) {
        mPaperSizeComboBox->insertItem( mPapers[i].mName );
        // Map - A4 land for now, if future read from template
        if ( mPapers[i].mWidth == 210 && mPapers[i].mHeight == 297 ){
	    mDefaultPaper = i;
	}
	if ( mPapers[i].mCustom ) mCustomPaper = i;
    }

    // Orientation
    mPaperOrientationComboBox->insertItem( tr("Portrait"), Portrait );
    mPaperOrientationComboBox->insertItem( tr("Landscape"), Landscape );

    mPaperUnitsComboBox->insertItem( "mm" );

    // Create canvas 
    mPaperWidth = 1;
    mPaperHeight = 1;
    createCanvas();
    
    // Tool
    mRectangleItem = 0;
    mNewCanvasItem = 0;
    mTool = Select;
    mToolStep = 0;
}

void QgsComposition::createDefault(void) 
{
    mPaperSizeComboBox->setCurrentItem(mDefaultPaper);
    mPaperOrientationComboBox->setCurrentItem(Landscape);

    mUserPaperWidth = mPapers[mDefaultPaper].mWidth;
    mUserPaperHeight = mPapers[mDefaultPaper].mHeight;

    recalculate();

    mResolution = 300;

    setOptions();

    // Add the map to coposition
    QgsComposerMap *m = new QgsComposerMap ( this, mNextItemId++, 
	                          mScale*15, mScale*15, mScale*180, mScale*180 );
    mItems.push_back(m);

    // Add vector legend
    QgsComposerVectorLegend *vl = new QgsComposerVectorLegend ( this, mNextItemId++, 
	                       mScale*210, mScale*100, 10 );
    mItems.push_back(vl);

    // Title
    QgsComposerLabel *tit = new QgsComposerLabel ( this, mNextItemId++, 
	                                           mScale*238, mScale*40, "Map", 24 );
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
    std::cout << "mCanvas width = " << mCanvas->width() << " height = " << mCanvas->height() << std::endl;
    mPaperItem->setSize ( (int) mPaperWidth * mScale, (int) mPaperHeight * mScale );
}

QgsComposition::~QgsComposition()
{
    std::cerr << "QgsComposition::~QgsComposition" << std::endl;
    mView->setCanvas ( 0 );
    
    if ( mPaperItem ) delete mPaperItem;

    /* TODO: For some strange reason, it crashes if QgsComposerItem (QgsComposerLabel,QgsComposerMap)
     *       is deleted. It crashes before the destructor is called. QgsComposerVectorLegend works.
     *       -> deleting canvas items 
     */
    for (std::list < QgsComposerItem * >::iterator it = mItems.begin(); 
				     it != mItems.end(); ++it) 
    {
	    //delete *it; // crashes on QgsComposerLabel and QgsComposerMap
	    QCanvasItem *ci = dynamic_cast<QCanvasItem*>(*it);
	    delete ci;
    }

    /*
    QCanvasItemList l = mCanvas->allItems();
    for ( QCanvasItemList::Iterator it=l.begin(); it!=l.end(); ++it) {
	delete *it;
    }
    */
    
    if ( mCanvas ) delete mCanvas;
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

		for ( QCanvasItemList::Iterator it=l.fromLast(); it!=l.end(); --it) {
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
			
			mComposer->showItemOptions ( coi->options() );
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
	    mNewCanvasItem = 0;
	    mComposer->selectItem(); // usually just one legend
	    
	    // Select and show options
	    vl->setSelected ( true );
	    mComposer->showItemOptions ( vl->options() );
	    mSelectedItem = dynamic_cast <QCanvasItem*> (vl);

	    mCanvas->update();
	    }
	    break;

	case AddLabel:
	    {
	    mNewCanvasItem->setX( p.x() );
	    mNewCanvasItem->setY( p.y() );
	    QgsComposerLabel *lab = dynamic_cast <QgsComposerLabel*> (mNewCanvasItem);
            mItems.push_back(lab);
	    mNewCanvasItem = 0;
	    mComposer->selectItem(); // usually just one ???
	    
	    // Select and show options
	    lab->setSelected ( true );
	    mComposer->showItemOptions ( lab->options() );
	    mSelectedItem = dynamic_cast <QCanvasItem*> (lab);

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

		    mComposer->selectItem(); // usually just one map
		 
		    m->setSelected ( true );
		    mComposer->showItemOptions ( m->options() );
		    mSelectedItem = dynamic_cast <QCanvasItem *> (m);
		} else {
    		    mToolStep = 0;
		}
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
	    
    mPaper = mPaperSizeComboBox->currentItem();
    mPaperOrientation = mPaperOrientationComboBox->currentItem();
    std::cout << "custom = " << mPapers[mPaper].mCustom << std::endl;
    std::cout << "orientation = " << mPaperOrientation << std::endl;
    if ( mPapers[mPaper].mCustom ) {
	mUserPaperWidth = mPaperWidthLineEdit->text().toDouble();
	mUserPaperHeight = mPaperHeightLineEdit->text().toDouble();
	mPaperWidthLineEdit->setEnabled( TRUE );
	mPaperHeightLineEdit->setEnabled( TRUE );
    } else {
	mUserPaperWidth = mPapers[mPaperSizeComboBox->currentItem()].mWidth;
	mUserPaperHeight = mPapers[mPaperSizeComboBox->currentItem()].mHeight;
	mPaperWidthLineEdit->setEnabled( FALSE );
	mPaperHeightLineEdit->setEnabled( FALSE );
	setOptions();
    }

    recalculate();
	
    mView->repaintContents();
    writeSettings();
}

void QgsComposition::recalculate ( void ) 
{
    if ( (mPaperOrientation == Portrait &&  mUserPaperWidth < mUserPaperHeight) ||
	 (mPaperOrientation == Landscape &&  mUserPaperWidth > mUserPaperHeight) ) 
    {
	mPaperWidth = mUserPaperWidth;
	mPaperHeight = mUserPaperHeight;
    } else {
	mPaperWidth = mUserPaperHeight;
	mPaperHeight = mUserPaperWidth;
    }
    std::cout << "mPaperWidth = " << mPaperWidth << " mPaperHeight = " << mPaperHeight << std::endl;
    resizeCanvas();
    mComposer->zoomFull();
}

void QgsComposition::resolutionChanged ( void )
{
    mResolution = mResolutionLineEdit->text().toInt();
    writeSettings();
}

void QgsComposition::setOptions ( void )
{
    mPaperSizeComboBox->setCurrentItem(mPaper);
    mPaperOrientationComboBox->setCurrentItem(mPaperOrientation);
    mPaperWidthLineEdit->setText ( QString("%1").arg(mUserPaperWidth,0,'g') );
    mPaperHeightLineEdit->setText ( QString("%1").arg(mUserPaperHeight,0,'g') );
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
    return scale; 
}

int QgsComposition::id ( void ) { return mId; }

QgsComposer *QgsComposition::composer(void) { return mComposer; }

QCanvas *QgsComposition::canvas(void) { return mCanvas; }

double QgsComposition::paperWidth ( void ) { return mPaperWidth; }

double QgsComposition::paperHeight ( void ) { return mPaperHeight; }

int QgsComposition::resolution ( void ) { return mResolution; }

int QgsComposition::scale( void ) { 
    std::cout << "QgsComposition::scale = " << mScale << std::endl;
    return mScale; 
}

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

	mComposer->showItemOptions ( vl->options() );

	mView->viewport()->setMouseTracking ( true ); // to recieve mouse move
    } else if ( tool == AddLabel ) {
	if ( mNewCanvasItem ) delete mNewCanvasItem;
	
	// Create new object outside the visible area
	QgsComposerLabel *lab = new QgsComposerLabel ( this, mNextItemId++, 
	                                (-1000)*mScale, (-1000)*mScale, "Label", (int) (mScale*mPaperHeight/40) );
        mNewCanvasItem = dynamic_cast <QCanvasItem *> (lab);
	mComposer->showItemOptions ( lab->options() );

	mView->viewport()->setMouseTracking ( true ); // to recieve mouse move
    }
    
    mTool = tool;
    mToolStep = 0;
}

std::vector<QgsComposerMap*> QgsComposition::maps(void) 
{
    std::vector<QgsComposerMap*> v;
    for (std::list < QgsComposerItem * >::iterator it = mItems.begin(); it != mItems.end(); ++it) {
	QgsComposerItem *ci = (*it);
	if (  typeid (*ci) == typeid(QgsComposerMap) ) {
	    v.push_back ( dynamic_cast<QgsComposerMap*>(ci) );
	}
    }
    
    return v;
}

QgsComposerMap* QgsComposition::map ( int id ) 
{ 
    for (std::list < QgsComposerItem * >::iterator it = mItems.begin(); it != mItems.end(); ++it) {
	QgsComposerItem *ci = (*it);
	if (  ci->id() == id ) {
	    return ( dynamic_cast<QgsComposerMap*>(ci) );
	}
    }
    return 0;
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

void QgsComposition::emitMapChanged ( int id )
{
    emit mapChanged ( id );
}

bool QgsComposition::writeSettings ( void )
{
    QString path, val;
    path.sprintf("/composition_%d/", mId );
    QgsProject::instance()->writeEntry( "Compositions", path+"width", mUserPaperWidth );
    QgsProject::instance()->writeEntry( "Compositions", path+"height", mUserPaperHeight );
    QgsProject::instance()->writeEntry( "Compositions", path+"resolution", mResolution );
    
    if ( mPaperOrientation == Landscape ) {
	val = "landscape";
    } else {
	val = "portrait";
    }
    QgsProject::instance()->writeEntry( "Compositions", path+"orientation", val );

    
    return true;
}

bool QgsComposition::readSettings ( void )
{
    std::cout << "QgsComposition::readSettings" << std::endl;

    bool ok;
    
    mPaper = mCustomPaper;
    
    QString path, val;
    path.sprintf("/composition_%d/", mId );
    mUserPaperWidth = QgsProject::instance()->readDoubleEntry( "Compositions", path+"width", 297, &ok);
    mUserPaperHeight = QgsProject::instance()->readDoubleEntry( "Compositions", path+"height", 210, &ok);
    mResolution = QgsProject::instance()->readNumEntry( "Compositions", path+"resolution", 300, &ok);
    
    val = QgsProject::instance()->readEntry( "Compositions", path+"orientation", "landscape", &ok);
    if ( val.compare("landscape") == 0 ) {
	mPaperOrientation = Landscape;
    } else {
	mPaperOrientation = Portrait;
    }
    
    recalculate();
    setOptions();

    // Create objects
    path.sprintf("/composition_%d", mId );
    QStringList el = QgsProject::instance()->subkeyList ( "Compositions", path );

    for ( QStringList::iterator it = el.begin(); it != el.end(); ++it ) {
	std::cout << "key: " << (*it).ascii() << std::endl;
	
	QStringList l = QStringList::split( '_', (*it) );
	if ( l.size() == 2 ) {
	    QString name = l.first();
	    QString ids = l.last();
	    int id = ids.toInt();
	    
	    if ( name.compare("map") == 0 ) {
	        QgsComposerMap *map = new QgsComposerMap ( this, id );
                mItems.push_back(map);
	    } else if ( name.compare("vectorlegend") == 0 ) {
	        QgsComposerVectorLegend *vl = new QgsComposerVectorLegend ( this, id );
                mItems.push_back(vl);
	    } else if ( name.compare("label") == 0 ) {
	        QgsComposerLabel *lab = new QgsComposerLabel ( this, id );
                mItems.push_back(lab);
	    }

	    if ( id >= mNextItemId ) mNextItemId = id + 1;
	}
    }


    mCanvas->update();

    return true;
}

