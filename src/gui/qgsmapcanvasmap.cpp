/***************************************************************************
    qgsmapcanvasmap.cpp  -  draws the map in map canvas
    ----------------------
    begin                : February 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmapcanvasmap.h"
#include "qgsmaprenderer.h"
#include "qgsmaprendererv2.h"
#include "qgsmaplayer.h"

#include <QPainter>

QgsMapCanvasMap::QgsMapCanvasMap( QgsMapCanvas* canvas )
    : mCanvas( canvas )
    , mDirty(true)
{
  mRend = new QgsMapRendererV2();

  setZValue( -10 );
  setPos( 0, 0 );
  resize( QSize( 1, 1 ) );

  connect(mRend, SIGNAL(finished()), SLOT(finish()));

  connect(&mTimer, SIGNAL(timeout()), SLOT(onMapUpdateTimeout()));
  mTimer.setInterval(400);

}

QgsMapCanvasMap::~QgsMapCanvasMap()
{
  delete mRend;
  mRend = 0;
}

void QgsMapCanvasMap::refresh()
{
  if (mRend->isRendering())
  {
    qDebug("need to cancel first!");
    mRend->cancel();
  }

  mDirty = true;
  update();
}

void QgsMapCanvasMap::paint( QPainter* p, const QStyleOptionGraphicsItem*, QWidget* )
{
  qDebug("paint()");

  if (mDirty)
  {
    if (mRend->isRendering())
    {
      qDebug("already rendering");
    }
    else
    {
      qDebug("need to render");

      QStringList layerIds;
      foreach (QgsMapLayer* l, mCanvas->layers())
        layerIds.append(l->id());

      mRend->setLayers(layerIds);
      mRend->setExtent(mCanvas->extent());
      mRend->setOutputSize(mImage.size());
      mRend->setOutputDpi(120);
      mRend->updateDerived();

      const QgsMapRendererSettings& s = mRend->settings();
      mMapToPixel = QgsMapToPixel( s.mapUnitsPerPixel, s.size.height(), s.visibleExtent.yMinimum(), s.visibleExtent.xMinimum() );

      qDebug("----------> EXTENT %f,%f", mRend->extent().xMinimum(), mRend->extent().yMinimum());

      mImage.fill(mBgColor.rgb());

      mPainter = new QPainter(&mImage);

      // TODO[MD]: need to setup clipping?
      //paint.setClipRect( mImage.rect() );

      // antialiasing
      if ( mAntiAliasing )
        mPainter->setRenderHint( QPainter::Antialiasing );

      mRend->startWithCustomPainter(mPainter);

      mTimer.start();

      //p->drawImage(0,0, mLastImage);
      //return; // do not redraw the image
    }
  }

#ifdef EGA_MODE
  QImage i2( mImage.size()/3, mImage.format() );
  QPainter p2(&i2);
  p2.drawImage( QRect(0,0, mImage.width()/3, mImage.height()/3), mImage ); //, 0,0, mImage.width()/3, mImage.height()/3);
  p2.end();
  p->drawImage( QRect( 0, 0, mImage.width(), mImage.height()), i2 ) ;//, 0, 0, i2.width()*3, i2.height()*3 );
#else
  p->drawImage( 0, 0, mImage );
#endif

}

QRectF QgsMapCanvasMap::boundingRect() const
{
  return QRectF( 0, 0, mImage.width(), mImage.height() );
}

const QgsMapRendererSettings &QgsMapCanvasMap::settings() const
{
  return mRend->settings();
}

QgsMapToPixel *QgsMapCanvasMap::coordinateTransform()
{
  return &mMapToPixel;
}


void QgsMapCanvasMap::resize( QSize size )
{
  if (mRend->isRendering())
  {
    qDebug("need to cancel first!");
    mRend->cancel();
  }

  QgsDebugMsg( QString( "resizing to %1x%2" ).arg( size.width() ).arg( size.height() ) );
  prepareGeometryChange(); // to keep QGraphicsScene indexes up to date on size change

  mImage = QImage( size, QImage::Format_ARGB32_Premultiplied );
  //mCanvas->mapRenderer()->setOutputSize( size, mImage.logicalDpiX() );
}

void QgsMapCanvasMap::setPanningOffset( const QPoint& point )
{
  mOffset = point;
  setPos( mOffset );
}

QPaintDevice& QgsMapCanvasMap::paintDevice()
{
  return mImage;
}


void QgsMapCanvasMap::finish()
{
  qDebug("finish!");

  mTimer.stop();

  mDirty = false;

  delete mPainter;
  mPainter = 0;

  //mLastImage = mImage;

  update();
}


void QgsMapCanvasMap::onMapUpdateTimeout()
{
  qDebug("update timer!");

  update();
}
