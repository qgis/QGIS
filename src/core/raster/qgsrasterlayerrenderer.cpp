/***************************************************************************
  qgsrasterlayerrenderer.cpp
  --------------------------------------
  Date                 : December 2013
  Copyright            : (C) 2013 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrasterlayerrenderer.h"

#include "qgsmessagelog.h"
#include "qgsrasterdrawer.h"
#include "qgsrasteriterator.h"
#include "qgsrasterlayer.h"


QgsRasterLayerRenderer::QgsRasterLayerRenderer( QgsRasterLayer* layer, QgsRenderContext& rendererContext )
    : QgsMapLayerRenderer( layer->id() )
    , mRasterViewPort( nullptr )
    , mPipe( nullptr )
    , mContext( rendererContext )
{

  mPainter = rendererContext.painter();
  const QgsMapToPixel& theQgsMapToPixel = rendererContext.mapToPixel();
  mMapToPixel = &theQgsMapToPixel;

  QgsMapToPixel mapToPixel = theQgsMapToPixel;
  if ( mapToPixel.mapRotation() )
  {
    // unset rotation for the sake of local computations.
    // Rotation will be handled by QPainter later
    // TODO: provide a method of QgsMapToPixel to fetch map center
    //       in geographical units
    QgsPoint center = mapToPixel.toMapCoordinates(
                        mapToPixel.mapWidth() / 2.0,
                        mapToPixel.mapHeight() / 2.0
                      );
    mapToPixel.setMapRotation( 0, center.x(), center.y() );
  }

  QgsRectangle myProjectedViewExtent;
  QgsRectangle myProjectedLayerExtent;

  if ( rendererContext.coordinateTransform() )
  {
    QgsDebugMsgLevel( "coordinateTransform set -> project extents.", 4 );
    try
    {
      myProjectedViewExtent = rendererContext.coordinateTransform()->transformBoundingBox( rendererContext.extent() );
    }
    catch ( QgsCsException &cs )
    {
      QgsMessageLog::logMessage( QObject::tr( "Could not reproject view extent: %1" ).arg( cs.what() ), QObject::tr( "Raster" ) );
      myProjectedViewExtent.setMinimal();
    }

    try
    {
      myProjectedLayerExtent = rendererContext.coordinateTransform()->transformBoundingBox( layer->extent() );
    }
    catch ( QgsCsException &cs )
    {
      QgsMessageLog::logMessage( QObject::tr( "Could not reproject layer extent: %1" ).arg( cs.what() ), QObject::tr( "Raster" ) );
      myProjectedLayerExtent.setMinimal();
    }
  }
  else
  {
    QgsDebugMsgLevel( "coordinateTransform not set", 4 );
    myProjectedViewExtent = rendererContext.extent();
    myProjectedLayerExtent = layer->extent();
  }

  // clip raster extent to view extent
  QgsRectangle myRasterExtent = myProjectedViewExtent.intersect( &myProjectedLayerExtent );
  if ( myRasterExtent.isEmpty() )
  {
    QgsDebugMsg( "draw request outside view extent." );
    // nothing to do
    return;
  }

  QgsDebugMsgLevel( "theViewExtent is " + rendererContext.extent().toString(), 4 );
  QgsDebugMsgLevel( "myProjectedViewExtent is " + myProjectedViewExtent.toString(), 4 );
  QgsDebugMsgLevel( "myProjectedLayerExtent is " + myProjectedLayerExtent.toString(), 4 );
  QgsDebugMsgLevel( "myRasterExtent is " + myRasterExtent.toString(), 4 );

  //
  // The first thing we do is set up the QgsRasterViewPort. This struct stores all the settings
  // relating to the size (in pixels and coordinate system units) of the raster part that is
  // in view in the map window. It also stores the origin.
  //
  //this is not a class level member because every time the user pans or zooms
  //the contents of the rasterViewPort will change
  mRasterViewPort = new QgsRasterViewPort();

  mRasterViewPort->mDrawnExtent = myRasterExtent;
  if ( rendererContext.coordinateTransform() )
  {
    mRasterViewPort->mSrcCRS = layer->crs();
    mRasterViewPort->mDestCRS = rendererContext.coordinateTransform()->destCRS();
    mRasterViewPort->mSrcDatumTransform = rendererContext.coordinateTransform()->sourceDatumTransform();
    mRasterViewPort->mDestDatumTransform = rendererContext.coordinateTransform()->destinationDatumTransform();
  }
  else
  {
    mRasterViewPort->mSrcCRS = QgsCoordinateReferenceSystem(); // will be invalid
    mRasterViewPort->mDestCRS = QgsCoordinateReferenceSystem(); // will be invalid
    mRasterViewPort->mSrcDatumTransform = -1;
    mRasterViewPort->mDestDatumTransform = -1;
  }

  // get dimensions of clipped raster image in device coordinate space (this is the size of the viewport)
  mRasterViewPort->mTopLeftPoint = mapToPixel.transform( myRasterExtent.xMinimum(), myRasterExtent.yMaximum() );
  mRasterViewPort->mBottomRightPoint = mapToPixel.transform( myRasterExtent.xMaximum(), myRasterExtent.yMinimum() );

  // align to output device grid, i.e. floor/ceil to integers
  // TODO: this should only be done if paint device is raster - screen, image
  // for other devices (pdf) it can have floating point origin
  // we could use floating point for raster devices as well, but respecting the
  // output device grid should make it more effective as the resampling is done in
  // the provider anyway
  mRasterViewPort->mTopLeftPoint.setX( floor( mRasterViewPort->mTopLeftPoint.x() ) );
  mRasterViewPort->mTopLeftPoint.setY( floor( mRasterViewPort->mTopLeftPoint.y() ) );
  mRasterViewPort->mBottomRightPoint.setX( ceil( mRasterViewPort->mBottomRightPoint.x() ) );
  mRasterViewPort->mBottomRightPoint.setY( ceil( mRasterViewPort->mBottomRightPoint.y() ) );
  // recalc myRasterExtent to aligned values
  myRasterExtent.set(
    mapToPixel.toMapCoordinatesF( mRasterViewPort->mTopLeftPoint.x(),
                                  mRasterViewPort->mBottomRightPoint.y() ),
    mapToPixel.toMapCoordinatesF( mRasterViewPort->mBottomRightPoint.x(),
                                  mRasterViewPort->mTopLeftPoint.y() )
  );

  //raster viewport top left / bottom right are already rounded to int
  mRasterViewPort->mWidth = static_cast<int>( mRasterViewPort->mBottomRightPoint.x() - mRasterViewPort->mTopLeftPoint.x() );
  mRasterViewPort->mHeight = static_cast<int>( mRasterViewPort->mBottomRightPoint.y() - mRasterViewPort->mTopLeftPoint.y() );

  //the drawable area can start to get very very large when you get down displaying 2x2 or smaller, this is becasue
  //mapToPixel.mapUnitsPerPixel() is less then 1,
  //so we will just get the pixel data and then render these special cases differently in paintImageToCanvas()

  QgsDebugMsgLevel( QString( "mapUnitsPerPixel = %1" ).arg( mapToPixel.mapUnitsPerPixel() ), 3 );
  QgsDebugMsgLevel( QString( "mWidth = %1" ).arg( layer->width() ), 3 );
  QgsDebugMsgLevel( QString( "mHeight = %1" ).arg( layer->height() ), 3 );
  QgsDebugMsgLevel( QString( "myRasterExtent.xMinimum() = %1" ).arg( myRasterExtent.xMinimum() ), 3 );
  QgsDebugMsgLevel( QString( "myRasterExtent.xMaximum() = %1" ).arg( myRasterExtent.xMaximum() ), 3 );
  QgsDebugMsgLevel( QString( "myRasterExtent.yMinimum() = %1" ).arg( myRasterExtent.yMinimum() ), 3 );
  QgsDebugMsgLevel( QString( "myRasterExtent.yMaximum() = %1" ).arg( myRasterExtent.yMaximum() ), 3 );

  QgsDebugMsgLevel( QString( "mTopLeftPoint.x() = %1" ).arg( mRasterViewPort->mTopLeftPoint.x() ), 3 );
  QgsDebugMsgLevel( QString( "mBottomRightPoint.x() = %1" ).arg( mRasterViewPort->mBottomRightPoint.x() ), 3 );
  QgsDebugMsgLevel( QString( "mTopLeftPoint.y() = %1" ).arg( mRasterViewPort->mTopLeftPoint.y() ), 3 );
  QgsDebugMsgLevel( QString( "mBottomRightPoint.y() = %1" ).arg( mRasterViewPort->mBottomRightPoint.y() ), 3 );

  QgsDebugMsgLevel( QString( "mWidth = %1" ).arg( mRasterViewPort->mWidth ), 3 );
  QgsDebugMsgLevel( QString( "mHeight = %1" ).arg( mRasterViewPort->mHeight ), 3 );

  // /\/\/\ - added to handle zoomed-in rasters

  // TODO R->mLastViewPort = *mRasterViewPort;

  // TODO: is it necessary? Probably WMS only?
  layer->dataProvider()->setDpi( rendererContext.rasterScaleFactor() * 25.4 * rendererContext.scaleFactor() );


  // copy the whole raster pipe!
  mPipe = new QgsRasterPipe( *layer->pipe() );
}

QgsRasterLayerRenderer::~QgsRasterLayerRenderer()
{
  delete mRasterViewPort;
  delete mPipe;
}

bool QgsRasterLayerRenderer::render()
{
  if ( !mRasterViewPort )
    return true; // outside of layer extent - nothing to do

  //R->draw( mPainter, mRasterViewPort, &mMapToPixel );

  QTime time;
  time.start();
  //
  //
  // The goal here is to make as many decisions as possible early on (outside of the rendering loop)
  // so that we can maximise performance of the rendering process. So now we check which drawing
  // procedure to use :
  //

  QgsRasterProjector *projector = mPipe->projector();

  // TODO add a method to interface to get provider and get provider
  // params in QgsRasterProjector
  if ( projector )
  {
    projector->setCRS( mRasterViewPort->mSrcCRS, mRasterViewPort->mDestCRS );
  }

  // Drawer to pipe?
  QgsRasterIterator iterator( mPipe->last() );
  QgsRasterDrawer drawer( &iterator );
  drawer.draw( mPainter, mRasterViewPort, mMapToPixel, &mContext );

  QgsDebugMsgLevel( QString( "total raster draw time (ms):     %1" ).arg( time.elapsed(), 5 ), 4 );

  return true;
}
