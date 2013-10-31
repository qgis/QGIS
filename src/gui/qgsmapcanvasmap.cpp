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
#include "qgsmapsettings.h"
#include "qgsmaplayer.h"

#include <QPainter>

QgsMapCanvasMap::QgsMapCanvasMap( QgsMapCanvas* canvas )
    : mCanvas( canvas )
    , mDirty(true)
    , mJob(0)
{
  setZValue( -10 );
  setPos( 0, 0 );
  resize( QSize( 1, 1 ) );

  connect(&mTimer, SIGNAL(timeout()), SLOT(onMapUpdateTimeout()));
  mTimer.setInterval(400);

}

QgsMapCanvasMap::~QgsMapCanvasMap()
{
  delete mJob;
}

void QgsMapCanvasMap::refresh()
{
  if (mJob)
  {
    qDebug("need to cancel first!");
    mJob->cancel();
    mJob->deleteLater();
    mJob = 0;
  }

  mDirty = true;
  update();
}

void QgsMapCanvasMap::paint( QPainter* p, const QStyleOptionGraphicsItem*, QWidget* )
{
  qDebug("paint()");

  bool paintedAlready = false;

  if (mDirty)
  {
    if (mJob)
    {
      qDebug("already rendering");
    }
    else
    {
      qDebug("need to render");

      // draw the image before it will be wiped out
      // TODO: does not work correctly with panning by dragging
      p->drawImage( 0, 0, mImage );
      paintedAlready = true;

      QStringList layerIds;
      foreach (QgsMapLayer* l, mCanvas->layers())
        layerIds.append(l->id());

      mSettings.setLayers(layerIds);
      mSettings.setExtent(mCanvas->extent());
      mSettings.setOutputSize(mImage.size());
      mSettings.setOutputDpi(120);
      mSettings.updateDerived();

      const QgsMapSettings& s = mSettings;
      mMapToPixel = QgsMapToPixel( s.mapUnitsPerPixel(), s.outputSize().height(), s.visibleExtent().yMinimum(), s.visibleExtent().xMinimum() );

      qDebug("----------> EXTENT %f,%f", mSettings.extent().xMinimum(), mSettings.extent().yMinimum());

      mImage.fill(mBgColor.rgb());

      mPainter = new QPainter(&mImage);

      // TODO[MD]: need to setup clipping?
      //paint.setClipRect( mImage.rect() );

      // antialiasing
      if ( mAntiAliasing )
        mPainter->setRenderHint( QPainter::Antialiasing );

      // create the renderer job
      Q_ASSERT(mJob == 0);
      mJob = new QgsMapRendererCustomPainterJob(mSettings, mPainter);
      connect(mJob, SIGNAL(finished()), SLOT(finish()));
      mJob->start();

      mTimer.start();
    }
  }

  if (!paintedAlready)
  {
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
}

QRectF QgsMapCanvasMap::boundingRect() const
{
  return QRectF( 0, 0, mImage.width(), mImage.height() );
}

const QgsMapSettings &QgsMapCanvasMap::settings() const
{
  return mSettings;
}

QgsMapToPixel *QgsMapCanvasMap::coordinateTransform()
{
  return &mMapToPixel;
}


void QgsMapCanvasMap::resize( QSize size )
{
  if (mJob)
  {
    qDebug("need to cancel first!");
    mJob->cancel();
    mJob->deleteLater();
    mJob = 0;
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
