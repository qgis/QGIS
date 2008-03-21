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
#include <typeinfo>

#include "qgscomposition.h"

#include "qgscomposer.h"
#include "qgscomposeritem.h"
#include "qgscomposerlabel.h"

#include "qgscomposermap.h"
#include "qgscomposerpicture.h"
#include "qgscomposerscalebar.h"
#include "qgscomposervectorlegend.h"

#include "qgscomposerview.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"

#include <QGraphicsRectItem>
#include <QMatrix>
#include <QMessageBox>

#include <iostream>
#include <math.h>

QgsCompositionPaper::QgsCompositionPaper ( QString name, int w, int h, bool c) 
  :mName(name), mWidth(w), mHeight(h), mCustom(c)
{
}

QgsCompositionPaper::~QgsCompositionPaper ( )
{
}


QgsComposition::QgsComposition( QgsComposer *c, int id ) 
{
  setupUi(this);
  connect(mPaperWidthLineEdit, SIGNAL(editingFinished()), this, SLOT(paperSizeChanged()));
  connect(mPaperHeightLineEdit, SIGNAL(editingFinished()), this, SLOT(paperSizeChanged()));
  connect(mPaperOrientationComboBox, SIGNAL(activated(int)), this, SLOT(paperSizeChanged()));
  connect(mPaperSizeComboBox, SIGNAL(activated(int)), this, SLOT(paperSizeChanged()));
  connect(mResolutionLineEdit, SIGNAL(editingFinished()), this, SLOT(resolutionChanged()));

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
  
  // Attention: Qt4.1 writes line width to PS/PDF still as integer 
  // (using QPen->width() to get the value) so we MUST use mScale > 1
  
  // Note: It seems that Qt4.2 is using widthF so that we can set mScale to 1
  //       and hopefuly we can remove mScale completely

  // Note: scale 10 make it inacceptable slow: QgsComposerMap 900x900mm on paper 1500x1000
  //          cannot be smoothly moved even if mPreviewMode == Rectangle and no zoom in
  //       scale 2 results in minimum line width 0.5 mmm which is too much
  mScale = 1;

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

  mPaper = mDefaultPaper = mCustomPaper = 0;
  for( int i = 0; i < (int)mPapers.size(); i++ ) {
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
  mPaperOrientation = Landscape;

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

  // Add the map to composition
  /*
     QgsComposerMap *m = new QgsComposerMap ( this, mNextItemId++, 
     mScale*15, mScale*15, mScale*180, mScale*180 );
     mItems.push_back(m);
     */

  // Add vector legend
  /*
     QgsComposerVectorLegend *vl = new QgsComposerVectorLegend ( this, mNextItemId++, 
     mScale*210, mScale*100, 10 );
     mItems.push_back(vl);
     */

  // Title
  /*
     QgsComposerLabel *tit = new QgsComposerLabel ( this, mNextItemId++, 
     mScale*238, mScale*40, "Map", 24 );
     mItems.push_back(tit);
     */

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

  mCanvas = new QGraphicsScene (0, 0, (int) mPaperWidth, (int) mPaperHeight);//top, left, width, height
  mCanvas->setBackgroundBrush( QColor(180,180,180) );

  // Paper
  if ( mPaperItem ) delete mPaperItem;
  mPaperItem = new QGraphicsRectItem( 0, 0, (int) mPaperWidth, (int) mPaperHeight, mPaperItem, mCanvas );

  mPaperItem->setBrush( QColor(255,255,255) );
  mPaperItem->setPen( QPen(QColor(0,0,0), 0) ); // 0 line width makes it use a cosmetic pen - 1px, regardless of scale.
  mPaperItem->setZValue(0);
  mPaperItem->show();
}

void QgsComposition::resizeCanvas(void) 
{
  mCanvas->setSceneRect(0, 0, (int) mPaperWidth * mScale, (int) mPaperHeight * mScale );
#ifdef QGISDEBUG
  std::cout << "mCanvas width = " << mCanvas->width() << " height = " << mCanvas->height() << std::endl;
#endif
  mPaperItem->setRect(QRectF(0, 0, (int) mPaperWidth * mScale, (int) mPaperHeight * mScale ));
}

QgsComposition::~QgsComposition()
{
#ifdef QGISDEBUG
  std::cerr << "QgsComposition::~QgsComposition" << std::endl;
#endif
  mView->setScene ( 0 );

  if ( mPaperItem ) delete mPaperItem;

  for (std::list < QgsComposerItem * >::iterator it = mItems.begin(); 
      it != mItems.end(); ++it) 
  {
    delete *it;
  }

  if ( mCanvas ) delete mCanvas;
}

QgsMapCanvas *QgsComposition::mapCanvas(void) { return mMapCanvas; }

void QgsComposition::setActive (  bool active )
{
  if ( active ) {
    mView->setScene ( mCanvas );
    mComposer->showCompositionOptions ( this );
  } else {
    // TODO
  }
}

void QgsComposition::mousePressEvent(QMouseEvent* e)
{
#ifdef QGISDEBUG
  std::cerr << "QgsComposition::mousePressEvent() mTool = " << mTool << " mToolStep = "
    << mToolStep << std::endl;
#endif

  QPointF p = mView->mapToScene(e->pos());

//  mGrabPoint = mCanvas.mapToItem(p);

  switch ( mTool ) {
    case Select:
      {

        QGraphicsItem* newItem = 0;
        newItem = mCanvas->itemAt(p);
        if(newItem == mPaperItem) //ignore clicks on the paper
        {
	      newItem = 0;
        }

//what is this doing?  Grabbing the first item in the list?
/*
        QList<QGraphicsItem*> l = mCanvas->items(p);
        for ( QList<QGraphicsItem*>::Iterator it=l.fromLast(); it!=l.end(); --it) {
          if (! (*it)->isActive() ) continue;
          newItem = *it;
        }
*/

        if ( newItem ) { // found
          mGrabPoint = newItem->mapFromScene(p);
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

#ifdef QGISDEBUG
      std::cerr << "AddMap" << std::endl;
#endif
      if ( mToolStep == 0 ) {
        mRectangleItem = new QGraphicsRectItem( p.x(), p.y(), 0, 0, 0, mCanvas );//null parent
        mRectangleItem->setBrush( Qt::NoBrush );
        mRectangleItem->setPen( QPen(QColor(0,0,0), 0) );
        mRectangleItem->setZValue(100);
        //mRectangleItem->setActive(false);
        mRectangleItem->show();
        mToolStep = 1;
      }
#ifdef QGISDEBUG
      std::cerr << "mToolStep = " << mToolStep << std::endl;
#endif
      break;

    case AddVectorLegend:
      {
        mNewCanvasItem->setPos(p);
        QgsComposerVectorLegend *vl = dynamic_cast <QgsComposerVectorLegend*> (mNewCanvasItem);
        vl->writeSettings();
        mItems.push_back(vl);
        mNewCanvasItem = 0;
        mComposer->selectItem(); // usually just one legend

        // Select and show options
        vl->setSelected ( true );
        mComposer->showItemOptions ( vl->options() );
        mSelectedItem = dynamic_cast <QGraphicsItem*> (vl);

        mCanvas->update();
      }
      break;

    case AddLabel:
      {
        mNewCanvasItem->setPos(p);

        QgsComposerLabel *lab = dynamic_cast <QgsComposerLabel*> (mNewCanvasItem);
        lab->writeSettings();
        mItems.push_back(lab);
        mNewCanvasItem = 0;
        mComposer->selectItem(); // usually just one ???

        // Select and show options
        lab->setSelected ( true );
        mComposer->showItemOptions ( lab->options() );
        mSelectedItem = dynamic_cast <QGraphicsItem*> (lab);

        mCanvas->update();
      }
      break;

    case AddScalebar:
      {
        mNewCanvasItem->setPos(p);
        QgsComposerScalebar *sb = dynamic_cast <QgsComposerScalebar*> (mNewCanvasItem);
        sb->writeSettings();
        mItems.push_back(sb);
        mNewCanvasItem = 0;
        mComposer->selectItem(); // usually just one ???

        // Select and show options
        sb->setSelected ( true );
        mComposer->showItemOptions ( sb->options() );
        mSelectedItem = dynamic_cast <QGraphicsItem*> (sb);

        mCanvas->update();
      }
      break;

    case AddPicture:
      {
        //set up the bounding rectangle
        //note: this rectangle gets used to keep track of the beginning click point for a picture
        mRectangleItem = new QGraphicsRectItem( p.x(), p.y(), 0, 0, 0, mCanvas );//null parent
        mRectangleItem->setBrush( Qt::NoBrush );
        mRectangleItem->setPen( QPen(QColor(0,0,0), 0, Qt::DashLine) );
        mRectangleItem->setZValue(100);
        mRectangleItem->show();


        mNewCanvasItem->setPos(p);

        QgsComposerPicture *pi = dynamic_cast <QgsComposerPicture*> (mNewCanvasItem);

        pi->setSize( 0, 0 );

        mCanvas->update();

        mToolStep = 1;
      }
      break;
 }
}

void QgsComposition::mouseMoveEvent(QMouseEvent* e)
{
#ifdef QGISDEBUG
#endif
  std::cerr << "QgsComposition::mouseMoveEvent() mTool = " << mTool << " mToolStep = "
    << mToolStep << std::endl;


  QPointF p = mView->mapToScene(e->pos());

  switch ( mTool ) {
    case Select:
      if ( mSelectedItem ) {
        mSelectedItem->setPos(p - mGrabPoint);
        mCanvas->update();
      }
      break;

    case AddMap:
      if ( mToolStep == 1 ) // draw rectangle while dragging
      {
        double x, y;
        double w, h;

        x = p.x() < mRectangleItem->x() ? p.x() : mRectangleItem->rect().x();
        y = p.y() < mRectangleItem->y() ? p.y() : mRectangleItem->rect().y();

        w = fabs ( p.x() - mRectangleItem->rect().x() );
        h = fabs ( p.y() - mRectangleItem->rect().y() );

        mRectangleItem->setRect(QRectF(x, y, w, h));

        mCanvas->update();
      }//END if(mToolStep == 1)
      break;

    case AddPicture:
      if ( mToolStep == 1 )
      {
        QgsComposerPicture *pi = dynamic_cast <QgsComposerPicture*> (mNewCanvasItem);

        double x, y;
        double w, h;

        x = p.x() < mRectangleItem->x() ? p.x() : mRectangleItem->rect().x();
        y = p.y() < mRectangleItem->y() ? p.y() : mRectangleItem->rect().y();

        w = fabs ( p.x() - mRectangleItem->rect().x() );
        h = fabs ( p.y() - mRectangleItem->rect().y() );

        pi->setPos(x + w/2, y + h/2);
        pi->setSize(w, h);

        mRectangleItem->setRect(QRectF(x, y, w, h));

        mCanvas->update();
      }//END if(mToolStep == 1)
      break;

    case AddScalebar:
    case AddVectorLegend:
    case AddLabel:

      mNewCanvasItem->setPos(p);
      mCanvas->update();
      break;

  }
}

void QgsComposition::mouseReleaseEvent(QMouseEvent* e)
{
#ifdef QGISDEBUG
  std::cerr << "QgsComposition::mouseReleaseEvent() mTool = " << mTool 
    << " mToolStep = " << mToolStep << std::endl;
#endif

  QPoint p = mView->mapFromScene(e->pos());

  switch ( mTool ) {
    case AddMap: // mToolStep should be always 1 but rectangle can be 0 size
      {
        int x = (int) mRectangleItem->rect().x();//use doubles?
        int y = (int) mRectangleItem->rect().y();
        int w = (int)mRectangleItem->rect().width();
        int h = (int)mRectangleItem->rect().height();
        delete mRectangleItem;
        mRectangleItem = 0;

        if ( w > 0 && h > 0 ) {
          mComposer->selectItem(); // usually just one map

          QgsComposerMap *m = new QgsComposerMap ( this, mNextItemId++, x, y, w, h );

          m->setPos(x, y);

          m->setUserExtent( mMapCanvas->extent());
          mItems.push_back(m);
          m->setSelected ( true );//do we need this twice?

          if ( mSelectedItem ) {
            QgsComposerItem *coi = dynamic_cast <QgsComposerItem *> (mSelectedItem);
            coi->setSelected ( false );
          }

          m->setSelected ( true );
          mComposer->showItemOptions ( m->options() );
          mSelectedItem = dynamic_cast <QGraphicsItem *> (m);
        } else {
          mToolStep = 0;
        }
        mCanvas->update();
      }
      break;

    case AddPicture:
      {
        double w = mRectangleItem->rect().width();
        double h = mRectangleItem->rect().height();
        delete mRectangleItem;
        mRectangleItem = 0;

        QgsComposerPicture *pi = dynamic_cast <QgsComposerPicture*> (mNewCanvasItem);

        if ( w > 0 && h > 0 )
        {
	      mNewCanvasItem = 0; // !!! Must be before mComposer->selectItem()
	      mComposer->selectItem(); // usually just one ???

          pi->writeSettings();
          mItems.push_back(pi);

          pi->setSelected ( true );
          mComposer->showItemOptions ( pi->options() );
          mSelectedItem = dynamic_cast <QGraphicsItem*> (pi);

          mCanvas->update();

        } else {
            mToolStep = 0;
        }
      }
      break;

    case Select:
      if ( mSelectedItem ) {
        // the object was probably moved
        QgsComposerItem *ci = dynamic_cast <QgsComposerItem *> (mSelectedItem);
        ci->writeSettings();
      }
      break;

    //We don't do anything special for labels, scalebars, or vector legends
    case AddLabel:
    case AddScalebar:
    case AddVectorLegend:
      break;

  }
}

void QgsComposition::keyPressEvent ( QKeyEvent * e )
{
#ifdef QGISDEBUG
  std::cout << "QgsComposition::keyPressEvent() key = " << e->key() << std::endl;
#endif

  if ( e->key() == Qt::Key_Delete && mSelectedItem ) { // delete

    QgsComposerItem *coi = dynamic_cast <QgsComposerItem *> (mSelectedItem);
    coi->setSelected ( false );
    coi->removeSettings();
    for (std::list < QgsComposerItem * >::iterator it = mItems.begin(); 
        it != mItems.end(); ++it) 
    {
      if ( (*it) == coi ) {
        mItems.erase ( it );
        break;
      }
    }
    delete (mSelectedItem);
    mSelectedItem = 0;
    mCanvas->update();
  }
}

void QgsComposition::paperSizeChanged ( void )
{
#ifdef QGISDEBUG
  std::cout << "QgsComposition::paperSizeChanged" << std::endl;
#endif

  mPaper = mPaperSizeComboBox->currentItem();
  mPaperOrientation = mPaperOrientationComboBox->currentItem();
#ifdef QGISDEBUG
  std::cout << "custom = " << mPapers[mPaper].mCustom << std::endl;
  std::cout << "orientation = " << mPaperOrientation << std::endl;
#endif
  if ( mPapers[mPaper].mCustom ) {
    mUserPaperWidth = mPaperWidthLineEdit->text().toDouble();
    mUserPaperHeight = mPaperHeightLineEdit->text().toDouble();
    mPaperWidthLineEdit->setEnabled( TRUE );
    mPaperHeightLineEdit->setEnabled( TRUE );
  } else {
    mUserPaperWidth = mPapers[mPaper].mWidth;
    mUserPaperHeight = mPapers[mPaper].mHeight;
    mPaperWidthLineEdit->setEnabled( FALSE );
    mPaperHeightLineEdit->setEnabled( FALSE );
    setOptions();
  }

  try
  {
    recalculate();
  }
  catch (std::bad_alloc& ba)
  {
    UNUSED(ba);
    // A better solution here would be to set the canvas back to the
    // original size and carry on, but for the moment this will
    // prevent a crash due to an uncaught exception.
    QMessageBox::critical( 0, tr("Out of memory"),
                           tr("Qgis is unable to resize the paper size due to "
                              "insufficient memory.\n It is best that you avoid "
                              "using the map composer until you restart qgis.\n") );
  }

//  mView->repaintContents(); //just repaint();?
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
#ifdef QGISDEBUG
  std::cout << "mPaperWidth = " << mPaperWidth << " mPaperHeight = " << mPaperHeight << std::endl;
#endif
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

  // Remove paper if Print, reset if Preview
  if ( mPlotStyle == Print ) {
//    mPaperItem->setScene(0);
//    mCanvas->setBackgroundColor( Qt::white );
  } else { 
//    mPaperItem->setScene(mCanvas);
//    mCanvas->setBackgroundColor( QColor(180,180,180) );

  }
}

double QgsComposition::viewScale ( void ) 
{
  double scale = mView->matrix().m11();
  return scale; 
}

//does this even work?
void QgsComposition::refresh()
{
  // TODO add signals to map canvas
  for (std::list < QgsComposerItem * >::iterator it = mItems.begin(); it != mItems.end(); ++it)    {
    QgsComposerItem *ci = (*it);
    if (  typeid (*ci) == typeid(QgsComposerMap) ) {
      QgsComposerMap *cm = dynamic_cast<QgsComposerMap*>(ci);
      cm->setCacheUpdated(false);
    } else if (  typeid (*ci) == typeid(QgsComposerVectorLegend) ) {
      QgsComposerVectorLegend *vl = dynamic_cast<QgsComposerVectorLegend*>(ci);
      vl->recalculate();
    }
  }
  mCanvas->update();
}

int QgsComposition::id ( void ) { return mId; }

QgsComposer *QgsComposition::composer(void) { return mComposer; }

QGraphicsScene *QgsComposition::canvas(void) { return mCanvas; }

double QgsComposition::paperWidth ( void ) { return mPaperWidth; }

double QgsComposition::paperHeight ( void ) { return mPaperHeight; }

int QgsComposition::paperOrientation ( void ) { return mPaperOrientation; }

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
    mNewCanvasItem->setPos(-1000, -1000);
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
    QgsComposerVectorLegend *vl = new QgsComposerVectorLegend ( this, mNextItemId++, -1000, -1000, (int) (mPaperHeight/50));
    mNewCanvasItem = dynamic_cast <QGraphicsItem *> (vl);
    mComposer->showItemOptions ( vl->options() );

    mView->viewport()->setMouseTracking ( true ); // to recieve mouse move

  }
  else if ( tool == AddLabel ) {
    if ( mNewCanvasItem ) delete mNewCanvasItem;

    // Create new object outside the visible area
    QgsComposerLabel *lab = new QgsComposerLabel ( this, mNextItemId++, -1000, -1000, tr("Label"), (int) (mPaperHeight/20));

    mNewCanvasItem = dynamic_cast <QGraphicsItem *> (lab);
    mComposer->showItemOptions ( lab->options() );

    mView->viewport()->setMouseTracking ( true ); // to recieve mouse move
  }
 else if ( tool == AddScalebar ) {
    if ( mNewCanvasItem ) delete mNewCanvasItem;

    // Create new object outside the visible area
    QgsComposerScalebar *sb = new QgsComposerScalebar ( this, mNextItemId++, -1000, -1000);
    mNewCanvasItem = dynamic_cast <QGraphicsItem*> (sb);
    mComposer->showItemOptions ( sb->options() );

    mView->viewport()->setMouseTracking ( true ); // to recieve mouse move
  }
  else if ( tool == AddPicture )
  {
    if ( mNewCanvasItem ) delete mNewCanvasItem;

    while ( 1 ) // keep trying until we get a valid image or the user clicks cancel
    {
	  QString file = QgsComposerPicture::pictureDialog();

      if ( file.isNull() ) //user clicked cancel
	  {
	    // TODO: This is not nice, because selectItem() calls 
            //       this function, do it better
	    mComposer->selectItem();
	    tool = Select;
	    break; //quit the loop
      }

	  // Create new object outside the visible area
	  QgsComposerPicture *pi = new QgsComposerPicture ( this, mNextItemId++, file );

	  if ( pi->pictureValid() )
	  {
#ifdef QGISDEBUG
  	    std::cout << "picture is valid" << std::endl;
#endif
	    mNewCanvasItem = dynamic_cast <QGraphicsItem *> (pi);
	    mComposer->showItemOptions ( pi->options() );

	    mView->viewport()->setMouseTracking ( true ); // start tracking the mouse
        break; //quit the loop
	  }
	  else
      {
	    QMessageBox::warning( this, tr("Warning"),
                         tr("Cannot load picture.") );
	    delete pi;
	  }
    }

  }//END if(tool == AddPicture)*/

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

double QgsComposition::selectionBoxSize ( void )
{
  // Scale rectangle based on the zoom level, so we keep the rectangle a fixed size on the screen
  return 7.0/viewScale();
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
#ifdef QGISDEBUG
  std::cout << "QgsComposition::readSettings" << std::endl;
#endif

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

  // First create the map(s) because they are often required by other objects
  for ( QStringList::iterator it = el.begin(); it != el.end(); ++it ) {
#ifdef QGISDEBUG
    std::cout << "key: " << (*it).toLocal8Bit().data() << std::endl;
#endif

    QStringList l = QStringList::split( '_', (*it) );
    if ( l.size() == 2 ) {
      QString name = l.first();
      QString ids = l.last();
      int id = ids.toInt();

      if ( name.compare("map") == 0 ) {
        QgsComposerMap *map = new QgsComposerMap ( this, id );
        mItems.push_back(map);
      }

      if ( id >= mNextItemId ) mNextItemId = id + 1;
    }
  }

  for ( QStringList::iterator it = el.begin(); it != el.end(); ++it ) {
#ifdef QGISDEBUG
    std::cout << "key: " << (*it).toLocal8Bit().data() << std::endl;
#endif

    QStringList l = QStringList::split( '_', (*it) );
    if ( l.size() == 2 ) {
      QString name = l.first();
      QString ids = l.last();
      int id = ids.toInt();

      if ( name.compare("vectorlegend") == 0 ) {
        QgsComposerVectorLegend *vl = new QgsComposerVectorLegend ( this, id );
        mItems.push_back(vl);
      } else if ( name.compare("label") == 0 ) {
        QgsComposerLabel *lab = new QgsComposerLabel ( this, id );
        mItems.push_back(lab);
      } else if ( name.compare("scalebar") == 0 ) {
        QgsComposerScalebar *sb = new QgsComposerScalebar ( this, id );
        mItems.push_back(sb);
      } else if ( name.compare("picture") == 0 ) {
        QgsComposerPicture *pi = new QgsComposerPicture ( this, id );
        mItems.push_back(pi);
      }

      if ( id >= mNextItemId ) mNextItemId = id + 1;
    }
  }


  mCanvas->update();

  return true;
}

