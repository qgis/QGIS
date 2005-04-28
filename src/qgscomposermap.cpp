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
}

void QgsComposerMap::init ()
{
    mNumCachedLayers = 0;
    mSelected = false;
    mUserExtent = mMapCanvas->extent();
    mDrawing = false;

    // Cache
    mCacheUpdated = false;

    // Calculate
    mCalculateComboBox->insertItem( tr("Extent (calculate scale)"), Scale );
    mCalculateComboBox->insertItem( tr("Scale (calculate extent)"), Extent );
    mCalculate = Scale;

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

    connect ( mMapCanvas, SIGNAL(addedLayer(QgsMapLayer *)), this, SLOT(mapCanvasChanged()) );
    connect ( mMapCanvas, SIGNAL(removedLayer(QString)), this, SLOT(mapCanvasChanged()) );
    connect ( mMapCanvas, SIGNAL(removedAll()), this, SLOT(mapCanvasChanged()) );
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
	  vector->draw( painter, extent, transform, device, widthScale, symbolScale, 0 );

      } else {
	  layer->draw( painter, extent, transform, device );
      }
    }
    
    // Draw vector labels
    for ( int i = 0; i < nlayers; i++ ) {
      QgsMapLayer *layer = mMapCanvas->getZpos(i);
	
      if ( !layer->visible() ) continue;
      
      if ( layer->type() == QgsMapLayer::VECTOR ) {
	  QgsVectorLayer *vector = dynamic_cast <QgsVectorLayer*> (layer);

	  if ( vector->labelOn() ) {
	      double fontScale = 25.4 * mFontScale * mComposition->scale() / 72;
	      if ( plotStyle() == QgsComposition::Postscript ) {
		  // I have no idea why 2.54 - it is an empirical value
		  fontScale = 2.54 * 72.0 / mComposition->resolution();
	      }
	      vector->drawLabels (  painter, extent, transform, device, fontScale );
	  }

      }
    }
    
    mMapCanvas->freeze(false);
}

void QgsComposerMap::setUserExtent ( QgsRect const & rect )
{
    mUserExtent = rect;
    recalculate();
    
    QCanvasRectangle::canvas()->setChanged( QCanvasRectangle::boundingRect() );
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
           plotStyle() == QgsComposition::Print ||
       plotStyle() == QgsComposition::Postscript ) 
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
    if ( mSelected && plotStyle() == QgsComposition::Preview ) {
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

    QCanvasRectangle::canvas()->setChanged( QCanvasRectangle::boundingRect() );
    QCanvasRectangle::update();
    QCanvasRectangle::canvas()->update();
    
    writeSettings();
}

void QgsComposerMap::calculateChanged ( void ) 
{
    mCalculate = mCalculateComboBox->currentItem();
    
    if ( mCalculate == Scale ) { // return to extent defined by user
  recalculate();

  mCacheUpdated = false;
  //QCanvasRectangle::canvas()->setAllChanged(); // must be setAllChanged(), not sure why
      QCanvasRectangle::canvas()->setChanged( QCanvasRectangle::boundingRect() );
  QCanvasRectangle::canvas()->update();
    
  mComposition->emitMapChanged ( mId );
    }
    setOptions();
    writeSettings();
}

double QgsComposerMap::scaleFromUserScale ( double us ) 
{
    double s;
    
    switch ( QgsProject::instance()->mapUnits() ) {
  case QgsScaleCalculator::METERS :
      s = 1000. * mComposition->scale() / us;
      break;
  case QgsScaleCalculator::FEET :
      s = 304.8 * mComposition->scale() / us;
      break;
  case QgsScaleCalculator::DEGREES :
      s = mComposition->scale() / us;
      break;
    }
    return s;
}

double QgsComposerMap::userScaleFromScale ( double s )
{ 
    double us;
    
    switch ( QgsProject::instance()->mapUnits() ) {
  case QgsScaleCalculator::METERS :
      us = 1000. * mComposition->scale() / s; 
      break;
  case QgsScaleCalculator::FEET :
      us = 304.8 * mComposition->scale() / s; 
      break;
  case QgsScaleCalculator::DEGREES :
      us = mComposition->scale() / s;
      break;
    }
    
    return us;
}

void QgsComposerMap::mapScaleChanged ( void ) 
{
    std::cout << "QgsComposerMap::mapScaleChanged" << std::endl;

    mCalculate = mCalculateComboBox->currentItem();

    mUserScale = mScaleLineEdit->text().toDouble();

    mScale = scaleFromUserScale ( mUserScale );

    recalculate();

    mCacheUpdated = false;
    QCanvasRectangle::canvas()->setChanged( QCanvasRectangle::boundingRect() );
    QCanvasRectangle::update();
    QCanvasRectangle::canvas()->update();
    
    writeSettings();
    mComposition->emitMapChanged ( mId );
}

void QgsComposerMap::scaleChanged ( void ) 
{
    mWidthScale = mWidthScaleLineEdit->text().toDouble();
    mSymbolScale = mSymbolScaleLineEdit->text().toDouble();
    mFontScale = mFontScaleLineEdit->text().toDouble();

    mCacheUpdated = false;
    QCanvasRectangle::canvas()->setChanged( QCanvasRectangle::boundingRect() );
    QCanvasRectangle::update();
    QCanvasRectangle::canvas()->update();
    
    writeSettings();
    mComposition->emitMapChanged ( mId );
}

void QgsComposerMap::mapCanvasChanged ( void ) 
{
    std::cout << "QgsComposerMap::canvasChanged" << std::endl;

    mCacheUpdated = false;
    QCanvasRectangle::canvas()->setChanged( QCanvasRectangle::boundingRect() );
}

void QgsComposerMap::previewModeChanged ( int i )
{
    mPreviewMode = (PreviewMode) i;
    writeSettings();
}

void QgsComposerMap::recalculate ( void ) 
{
    std::cout << "QgsComposerMap::recalculate mCalculate = " << mCalculate << std::endl;

    if ( mCalculate == Scale ) 
    {
  // Calculate scale from extent and rectangle
  double xscale = QCanvasRectangle::width() / mUserExtent.width();
  double yscale = QCanvasRectangle::height() / mUserExtent.height();

  mExtent = mUserExtent;

  if ( xscale < yscale ) {
      mScale = xscale;
      // extend y
      double d = ( 1. * QCanvasRectangle::height() / mScale - mUserExtent.height() ) / 2 ;
      mExtent.setYmin ( mUserExtent.yMin() - d );
      mExtent.setYmax ( mUserExtent.yMax() + d );
  } else {
      mScale = yscale;
      // extend x
      double d = ( 1.* QCanvasRectangle::width() / mScale - mUserExtent.width() ) / 2 ;
      mExtent.setXmin ( mUserExtent.xMin() - d );
      mExtent.setXmax ( mUserExtent.xMax() + d );
  }

  mUserScale = userScaleFromScale ( mScale );
    } 
    else
    {
  // Calculate extent
  double xc = ( mUserExtent.xMax() + mUserExtent.xMin() ) / 2;
  double yc = ( mUserExtent.yMax() + mUserExtent.yMin() ) / 2;
    
  double width = QCanvasRectangle::width() / mScale;
  double height = QCanvasRectangle::height() / mScale;
  
  mExtent.setXmin ( xc - width/2  );
  mExtent.setXmax ( xc + width/2  );
  mExtent.setYmin ( yc - height/2  );
  mExtent.setYmax ( yc + height/2  );

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

    QCanvasRectangle::canvas()->setChanged( QCanvasRectangle::boundingRect() );
    QCanvasRectangle::update();
    QCanvasRectangle::canvas()->update();

    writeSettings();
}


void QgsComposerMap::setOptions ( void )
{ 
    std::cout << "QgsComposerMap::setOptions" << std::endl;
    
    mNameLabel->setText ( mName );
    
    mCalculateComboBox->setCurrentItem( mCalculate );
    
    mWidthLineEdit->setText ( QString("%1").arg( mComposition->toMM(QCanvasRectangle::width()), 0,'g') );
    mHeightLineEdit->setText ( QString("%1").arg( mComposition->toMM(QCanvasRectangle::height()),0,'g') );
    
    // Scale
    switch ( QgsProject::instance()->mapUnits() ) {
  case QgsScaleCalculator::METERS :
  case QgsScaleCalculator::FEET :
            mScaleLineEdit->setText ( QString("%1").arg((int)mUserScale) );
      break;
  case QgsScaleCalculator::DEGREES :
            mScaleLineEdit->setText ( QString("%1").arg(mUserScale,0,'f') );
      break;
    }
    if ( mCalculate == Scale ) {
  mScaleLineEdit->setEnabled(false);  
    } else {
  mScaleLineEdit->setEnabled(true); 
    }
    
    mWidthScaleLineEdit->setText ( QString("%1").arg(mWidthScale,0,'g',2) );
    mSymbolScaleLineEdit->setText ( QString("%1").arg(mSymbolScale,0,'g',2) );
    mFontScaleLineEdit->setText ( QString("%1").arg(mFontScale,0,'g',2) );

    mFrameCheckBox->setChecked ( mFrame );
    
    mPreviewModeComboBox->setCurrentItem( mPreviewMode );
}

void QgsComposerMap::setCurrentExtent ( void )
{ 
    mUserExtent = mMapCanvas->extent();
    recalculate();
    QCanvasRectangle::canvas()->setChanged( QCanvasRectangle::boundingRect() );
    QCanvasRectangle::update();
    QCanvasRectangle::canvas()->update();
    setOptions();
    writeSettings();
    mComposition->emitMapChanged ( mId );
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

void QgsComposerMap::setCacheUpdated ( bool u ) 
{
    mCacheUpdated = u;
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

    if ( mCalculate == Scale ) {
        QgsProject::instance()->writeEntry( "Compositions", path+"calculate", QString("scale") );
    } else {
        QgsProject::instance()->writeEntry( "Compositions", path+"calculate", QString("extent") );
    }
    
    QgsProject::instance()->writeEntry( "Compositions", path+"north", mUserExtent.yMax() );
    QgsProject::instance()->writeEntry( "Compositions", path+"south", mUserExtent.yMin() );
    QgsProject::instance()->writeEntry( "Compositions", path+"east", mUserExtent.xMax() );
    QgsProject::instance()->writeEntry( "Compositions", path+"west", mUserExtent.xMin() );

    QgsProject::instance()->writeEntry( "Compositions", path+"scale", mUserScale );

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

    QString calculate = QgsProject::instance()->readEntry("Compositions", path+"calculate", "scale", &ok);
    if ( calculate == "extent" ) {
        mCalculate = Extent;
    } else {
        mCalculate = Scale;
    }

    mUserExtent.setYmax ( QgsProject::instance()->readDoubleEntry( "Compositions", path+"north", 100, &ok) );
    mUserExtent.setYmin ( QgsProject::instance()->readDoubleEntry( "Compositions", path+"south", 0, &ok) );
    mUserExtent.setXmax ( QgsProject::instance()->readDoubleEntry( "Compositions", path+"east", 100, &ok) );
    mUserExtent.setXmin ( QgsProject::instance()->readDoubleEntry( "Compositions", path+"west", 0, &ok) );

    mUserScale =  QgsProject::instance()->readDoubleEntry( "Compositions", path+"scale", 1000., &ok);
    mScale = scaleFromUserScale ( mUserScale );

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
