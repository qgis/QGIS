/***************************************************************************
    qgsmaprender.cpp  -  class for rendering map layer set
    ----------------------
    begin                : January 2006
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

#include <cmath>
#include <cfloat>

#include "qgscoordinatetransform.h"
#include "qgscrscache.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsmaprenderer.h"
#include "qgsscalecalculator.h"
#include "qgsmaptopixel.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsmapsettings.h"
#include "qgsdistancearea.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"

#include <QDomDocument>
#include <QDomNode>
#include <QMutexLocker>
#include <QPainter>
#include <QListIterator>
#include <QSettings>
#include <QTime>
#include <QCoreApplication>

QgsMapRenderer::QgsMapRenderer()
{
  mScale = 1.0;
  mRotation = 0.0;
  mScaleCalculator = new QgsScaleCalculator;
  mDistArea = new QgsDistanceArea;

  mDrawing = false;
  mOverview = false;

  // set default map units - we use WGS 84 thus use degrees
  setMapUnits( QGis::Degrees );

  mSize = QSize( 0, 0 );

  mProjectionsEnabled = false;
  mDestCRS = new QgsCoordinateReferenceSystem( QgsCRSCache::instance()->crsBySrsId( GEOCRS_ID ) ); //WGS 84

  mOutputUnits = QgsMapRenderer::Millimeters;
  mFullExtent.setMinimal();

  mLabelingEngine = nullptr;
  readDefaultDatumTransformations();
}

QgsMapRenderer::~QgsMapRenderer()
{
  delete mScaleCalculator;
  delete mDistArea;
  delete mDestCRS;
  delete mLabelingEngine;
}

QgsRectangle QgsMapRenderer::extent() const
{
  return mExtent;
}

void QgsMapRenderer::updateScale()
{
  mScale = mScaleCalculator->calculate( mExtent, mSize.width() );
}

bool QgsMapRenderer::setExtent( const QgsRectangle& extent )
{
  //remember the previous extent
  mLastExtent = mExtent;

  // Don't allow zooms where the current extent is so small that it
  // can't be accurately represented using a double (which is what
  // currentExtent uses). Excluding 0 avoids a divide by zero and an
  // infinite loop when rendering to a new canvas. Excluding extents
  // greater than 1 avoids doing unnecessary calculations.

  // The scheme is to compare the width against the mean x coordinate
  // (and height against mean y coordinate) and only allow zooms where
  // the ratio indicates that there is more than about 12 significant
  // figures (there are about 16 significant figures in a double).

  if ( extent.width()  > 0 &&
       extent.height() > 0 &&
       extent.width()  < 1 &&
       extent.height() < 1 )
  {
    // Use abs() on the extent to avoid the case where the extent is
    // symmetrical about 0.
    double xMean = ( qAbs( extent.xMinimum() ) + qAbs( extent.xMaximum() ) ) * 0.5;
    double yMean = ( qAbs( extent.yMinimum() ) + qAbs( extent.yMaximum() ) ) * 0.5;

    double xRange = extent.width() / xMean;
    double yRange = extent.height() / yMean;

    static const double minProportion = 1e-12;
    if ( xRange < minProportion || yRange < minProportion )
      return false;
  }

  mExtent = extent;
  if ( !extent.isEmpty() )
    adjustExtentToSize();

  emit extentsChanged();
  return true;
}

void QgsMapRenderer::setRotation( double rotation )
{
  mRotation = rotation;
  // TODO: adjust something ?

  emit rotationChanged( rotation );
}

double QgsMapRenderer::rotation() const
{
  return mRotation;
}


void QgsMapRenderer::setOutputSize( QSize size, double dpi )
{
  mSize = QSizeF( size.width(), size.height() );
  mScaleCalculator->setDpi( dpi );
  adjustExtentToSize();
}

void QgsMapRenderer::setOutputSize( QSizeF size, double dpi )
{
  mSize = size;
  mScaleCalculator->setDpi( dpi );
  adjustExtentToSize();
}

double QgsMapRenderer::outputDpi()
{
  return mScaleCalculator->dpi();
}

QSize QgsMapRenderer::outputSize() const
{
  return mSize.toSize();
}

QSizeF QgsMapRenderer::outputSizeF() const
{
  return mSize;
}

void QgsMapRenderer::adjustExtentToSize()
{
  double myHeight = mSize.height();
  double myWidth = mSize.width();

  QgsMapToPixel newCoordXForm;

  if ( !myWidth || !myHeight )
  {
    mScale = 1.0;
    newCoordXForm.setParameters( 1, 0, 0, 0, 0, 0 );
    return;
  }

  // calculate the translation and scaling parameters
  // mapUnitsPerPixel = map units per pixel
  double mapUnitsPerPixelY = mExtent.height() / myHeight;
  double mapUnitsPerPixelX = mExtent.width() / myWidth;
  mMapUnitsPerPixel = mapUnitsPerPixelY > mapUnitsPerPixelX ? mapUnitsPerPixelY : mapUnitsPerPixelX;

  // calculate the actual extent of the mapCanvas
  double dxmin, dxmax, dymin, dymax, whitespace;

  if ( mapUnitsPerPixelY > mapUnitsPerPixelX )
  {
    dymin = mExtent.yMinimum();
    dymax = mExtent.yMaximum();
    whitespace = (( myWidth * mMapUnitsPerPixel ) - mExtent.width() ) * 0.5;
    dxmin = mExtent.xMinimum() - whitespace;
    dxmax = mExtent.xMaximum() + whitespace;
  }
  else
  {
    dxmin = mExtent.xMinimum();
    dxmax = mExtent.xMaximum();
    whitespace = (( myHeight * mMapUnitsPerPixel ) - mExtent.height() ) * 0.5;
    dymin = mExtent.yMinimum() - whitespace;
    dymax = mExtent.yMaximum() + whitespace;
  }

  QgsDebugMsg( QString( "Map units per pixel (x,y) : %1, %2" ).arg( qgsDoubleToString( mapUnitsPerPixelX ), qgsDoubleToString( mapUnitsPerPixelY ) ) );
  QgsDebugMsg( QString( "Pixmap dimensions (x,y) : %1, %2" ).arg( qgsDoubleToString( myWidth ), qgsDoubleToString( myHeight ) ) );
  QgsDebugMsg( QString( "Extent dimensions (x,y) : %1, %2" ).arg( qgsDoubleToString( mExtent.width() ), qgsDoubleToString( mExtent.height() ) ) );
  QgsDebugMsg( mExtent.toString() );

  // update extent
  mExtent.setXMinimum( dxmin );
  mExtent.setXMaximum( dxmax );
  mExtent.setYMinimum( dymin );
  mExtent.setYMaximum( dymax );

  QgsDebugMsg( QString( "Adjusted map units per pixel (x,y) : %1, %2" ).arg( qgsDoubleToString( mExtent.width() / myWidth ), qgsDoubleToString( mExtent.height() / myHeight ) ) );

  QgsDebugMsg( QString( "Recalced pixmap dimensions (x,y) : %1, %2" ).arg( qgsDoubleToString( mExtent.width() / mMapUnitsPerPixel ), qgsDoubleToString( mExtent.height() / mMapUnitsPerPixel ) ) );

  // update the scale
  updateScale();

  QgsDebugMsg( QString( "Scale (assuming meters as map units) = 1:%1" ).arg( qgsDoubleToString( mScale ) ) );

  Q_NOWARN_DEPRECATED_PUSH
  newCoordXForm.setParameters( mMapUnitsPerPixel, dxmin, dymin, myHeight );
  Q_NOWARN_DEPRECATED_POP
  mRenderContext.setMapToPixel( newCoordXForm );
  mRenderContext.setExtent( mExtent );
}


void QgsMapRenderer::render( QPainter* painter, double* forceWidthScale )
{
  //Lock render method for concurrent threads (e.g. from globe)
  QMutexLocker renderLock( &mRenderMutex );

  QgsDebugMsg( "========== Rendering ==========" );

  if ( mExtent.isEmpty() )
  {
    QgsDebugMsg( "empty extent... not rendering" );
    return;
  }

  if ( qgsDoubleNear( mSize.width(), 1.0 ) && qgsDoubleNear( mSize.height(), 1.0 ) )
  {
    QgsDebugMsg( "size 1x1... not rendering" );
    return;
  }

  QPaintDevice* thePaintDevice = painter->device();
  if ( !thePaintDevice )
  {
    return;
  }

  // wait
  if ( mDrawing )
  {
    QgsDebugMsg( "already rendering" );
    QCoreApplication::processEvents();
  }

  if ( mDrawing )
  {
    QgsDebugMsg( "still rendering - skipping" );
    return;
  }

  mDrawing = true;

  const QgsCoordinateTransform *ct;

#ifdef QGISDEBUG
  QgsDebugMsg( "Starting to render layer stack." );
  QTime renderTime;
  renderTime.start();
#endif

  if ( mOverview )
    mRenderContext.setDrawEditingInformation( !mOverview );

  mRenderContext.setPainter( painter );
  mRenderContext.setCoordinateTransform( nullptr );
  //this flag is only for stopping during the current rendering progress,
  //so must be false at every new render operation
  mRenderContext.setRenderingStopped( false );

  // set selection color
  QgsProject* prj = QgsProject::instance();
  int myRed = prj->readNumEntry( "Gui", "/SelectionColorRedPart", 255 );
  int myGreen = prj->readNumEntry( "Gui", "/SelectionColorGreenPart", 255 );
  int myBlue = prj->readNumEntry( "Gui", "/SelectionColorBluePart", 0 );
  int myAlpha = prj->readNumEntry( "Gui", "/SelectionColorAlphaPart", 255 );
  mRenderContext.setSelectionColor( QColor( myRed, myGreen, myBlue, myAlpha ) );

  //calculate scale factor
  //use the specified dpi and not those from the paint device
  //because sometimes QPainter units are in a local coord sys (e.g. in case of QGraphicsScene)
  double sceneDpi = mScaleCalculator->dpi();
  double scaleFactor = 1.0;
  if ( mOutputUnits == QgsMapRenderer::Millimeters )
  {
    if ( forceWidthScale )
    {
      scaleFactor = *forceWidthScale;
    }
    else
    {
      scaleFactor = sceneDpi / 25.4;
    }
  }
  double rasterScaleFactor = ( thePaintDevice->logicalDpiX() + thePaintDevice->logicalDpiY() ) / 2.0 / sceneDpi;
  if ( !qgsDoubleNear( mRenderContext.rasterScaleFactor(), rasterScaleFactor ) )
  {
    mRenderContext.setRasterScaleFactor( rasterScaleFactor );
  }
  if ( !qgsDoubleNear( mRenderContext.scaleFactor(), scaleFactor ) )
  {
    mRenderContext.setScaleFactor( scaleFactor );
  }
  if ( !qgsDoubleNear( mRenderContext.rendererScale(), mScale ) )
  {
    //add map scale to render context
    mRenderContext.setRendererScale( mScale );
  }
  if ( mLastExtent != mExtent )
  {
    mLastExtent = mExtent;
  }

  mRenderContext.setLabelingEngine( mLabelingEngine );
  if ( mLabelingEngine )
    mLabelingEngine->init( mapSettings() );

  // render all layers in the stack, starting at the base
  QListIterator<QString> li( mLayerSet );
  li.toBack();

  QgsRectangle r1, r2;

  while ( li.hasPrevious() )
  {
    if ( mRenderContext.renderingStopped() )
    {
      break;
    }

    // Store the painter in case we need to swap it out for the
    // cache painter
    QPainter * mypContextPainter = mRenderContext.painter();
    // Flattened image for drawing when a blending mode is set
    QImage * mypFlattenedImage = nullptr;

    QString layerId = li.previous();

    QgsDebugMsg( "Rendering at layer item " + layerId );

    // This call is supposed to cause the progress bar to
    // advance. However, it seems that updating the progress bar is
    // incompatible with having a QPainter active (the one that is
    // passed into this function), as Qt produces a number of errors
    // when try to do so. I'm (Gavin) not sure how to fix this, but
    // added these comments and debug statement to help others...
    QgsDebugMsg( "If there is a QPaintEngine error here, it is caused by an emit call" );

    //emit drawingProgress(myRenderCounter++, mLayerSet.size());
    QgsMapLayer *ml = QgsMapLayerRegistry::instance()->mapLayer( layerId );

    if ( !ml )
    {
      QgsDebugMsg( "Layer not found in registry!" );
      continue;
    }

    QgsDebugMsg( QString( "layer %1:  minscale:%2  maxscale:%3  scaledepvis:%4  extent:%5  blendmode:%6" )
                 .arg( ml->name() )
                 .arg( ml->minimumScale() )
                 .arg( ml->maximumScale() )
                 .arg( ml->hasScaleBasedVisibility() )
                 .arg( ml->extent().toString() )
                 .arg( ml->blendMode() )
               );

    if ( mRenderContext.useAdvancedEffects() )
    {
      // Set the QPainter composition mode so that this layer is rendered using
      // the desired blending mode
      mypContextPainter->setCompositionMode( ml->blendMode() );
    }

    if ( ml->isInScaleRange( mScale ) || mOverview )
    {
      connect( ml, SIGNAL( drawingProgress( int, int ) ), this, SLOT( onDrawingProgress( int, int ) ) );

      //
      // Now do the call to the layer that actually does
      // the rendering work!
      //

      bool split = false;

      if ( hasCrsTransformEnabled() )
      {
        r1 = mExtent;
        split = splitLayersExtent( ml, r1, r2 );
        ct = transformation( ml );
        mRenderContext.setExtent( r1 );
        QgsDebugMsg( "  extent 1: " + r1.toString() );
        QgsDebugMsg( "  extent 2: " + r2.toString() );
        if ( !r1.isFinite() || !r2.isFinite() ) //there was a problem transforming the extent. Skip the layer
        {
          continue;
        }
      }
      else
      {
        ct = nullptr;
      }

      mRenderContext.setCoordinateTransform( ct );

      //decide if we have to scale the raster
      //this is necessary in case QGraphicsScene is used
      bool scaleRaster = false;
      QgsMapToPixel rasterMapToPixel;
      QgsMapToPixel bk_mapToPixel;

      if ( ml->type() == QgsMapLayer::RasterLayer && qAbs( rasterScaleFactor - 1.0 ) > 0.000001 )
      {
        scaleRaster = true;
      }

      QSettings mySettings;

      // If we are drawing with an alternative blending mode then we need to render to a separate image
      // before compositing this on the map. This effectively flattens the layer and prevents
      // blending occurring between objects on the layer
      // (this is not required for raster layers or when layer caching is enabled, since that has the same effect)
      bool flattenedLayer = false;
      if (( mRenderContext.useAdvancedEffects() ) && ( ml->type() == QgsMapLayer::VectorLayer ) )
      {
        QgsVectorLayer* vl = qobject_cast<QgsVectorLayer *>( ml );
        if ((( vl->blendMode() != QPainter::CompositionMode_SourceOver )
             || ( vl->featureBlendMode() != QPainter::CompositionMode_SourceOver )
             || ( vl->layerTransparency() != 0 ) ) )
        {
          flattenedLayer = true;
          mypFlattenedImage = new QImage( mRenderContext.painter()->device()->width(),
                                          mRenderContext.painter()->device()->height(), QImage::Format_ARGB32 );
          if ( mypFlattenedImage->isNull() )
          {
            QgsDebugMsg( "insufficient memory for image " + QString::number( mRenderContext.painter()->device()->width() ) + 'x' + QString::number( mRenderContext.painter()->device()->height() ) );
            emit drawError( ml );
            painter->end(); // drawError is not caught by anyone, so we end painting to notify caller
            return;
          }
          mypFlattenedImage->fill( 0 );
          QPainter * mypPainter = new QPainter( mypFlattenedImage );
          if ( mySettings.value( "/qgis/enable_anti_aliasing", true ).toBool() )
          {
            mypPainter->setRenderHint( QPainter::Antialiasing );
          }
          mypPainter->scale( rasterScaleFactor, rasterScaleFactor );
          mRenderContext.setPainter( mypPainter );
        }
      }

      // Per feature blending mode
      if (( mRenderContext.useAdvancedEffects() ) && ( ml->type() == QgsMapLayer::VectorLayer ) )
      {
        QgsVectorLayer* vl = qobject_cast<QgsVectorLayer *>( ml );
        if ( vl->featureBlendMode() != QPainter::CompositionMode_SourceOver )
        {
          // set the painter to the feature blend mode, so that features drawn
          // on this layer will interact and blend with each other
          mRenderContext.painter()->setCompositionMode( vl->featureBlendMode() );
        }
      }

      if ( scaleRaster )
      {
        bk_mapToPixel = mRenderContext.mapToPixel();
        rasterMapToPixel = mRenderContext.mapToPixel();
        rasterMapToPixel.setMapUnitsPerPixel( mRenderContext.mapToPixel().mapUnitsPerPixel() / rasterScaleFactor );
        Q_NOWARN_DEPRECATED_PUSH
        rasterMapToPixel.setYMaximum( mSize.height() * rasterScaleFactor );
        Q_NOWARN_DEPRECATED_POP
        mRenderContext.setMapToPixel( rasterMapToPixel );
        mRenderContext.painter()->save();
        mRenderContext.painter()->scale( 1.0 / rasterScaleFactor, 1.0 / rasterScaleFactor );
      }

      if ( !ml->draw( mRenderContext ) )
      {
        emit drawError( ml );
      }
      else
      {
        QgsDebugMsg( "Layer rendered without issues" );
      }

      if ( split )
      {
        mRenderContext.setExtent( r2 );
        if ( !ml->draw( mRenderContext ) )
        {
          emit drawError( ml );
        }
      }

      if ( scaleRaster )
      {
        mRenderContext.setMapToPixel( bk_mapToPixel );
        mRenderContext.painter()->restore();
      }

      //apply layer transparency for vector layers
      if (( mRenderContext.useAdvancedEffects() ) && ( ml->type() == QgsMapLayer::VectorLayer ) )
      {
        QgsVectorLayer* vl = qobject_cast<QgsVectorLayer *>( ml );
        if ( vl->layerTransparency() != 0 )
        {
          // a layer transparency has been set, so update the alpha for the flattened layer
          // by combining it with the layer transparency
          QColor transparentFillColor = QColor( 0, 0, 0, 255 - ( 255 * vl->layerTransparency() / 100 ) );
          // use destination in composition mode to merge source's alpha with destination
          mRenderContext.painter()->setCompositionMode( QPainter::CompositionMode_DestinationIn );
          mRenderContext.painter()->fillRect( 0, 0, mRenderContext.painter()->device()->width(),
                                              mRenderContext.painter()->device()->height(), transparentFillColor );
        }
      }

      if ( flattenedLayer )
      {
        // If we flattened this layer for alternate blend modes, composite it now
        delete mRenderContext.painter();
        mRenderContext.setPainter( mypContextPainter );
        mypContextPainter->save();
        mypContextPainter->scale( 1.0 / rasterScaleFactor, 1.0 / rasterScaleFactor );
        mypContextPainter->drawImage( 0, 0, *( mypFlattenedImage ) );
        mypContextPainter->restore();
        delete mypFlattenedImage;
        mypFlattenedImage = nullptr;
      }

      disconnect( ml, SIGNAL( drawingProgress( int, int ) ), this, SLOT( onDrawingProgress( int, int ) ) );
    }
    else // layer not visible due to scale
    {
      QgsDebugMsg( "Layer not rendered because it is not within the defined "
                   "visibility scale range" );
    }

  } // while (li.hasPrevious())

  QgsDebugMsg( "Done rendering map layers" );

  // Reset the composition mode before rendering the labels
  mRenderContext.painter()->setCompositionMode( QPainter::CompositionMode_SourceOver );

  if ( !mOverview )
  {
    // render all labels for vector layers in the stack, starting at the base
    li.toBack();
    while ( li.hasPrevious() )
    {
      if ( mRenderContext.renderingStopped() )
      {
        break;
      }

      QString layerId = li.previous();

      // TODO: emit drawingProgress((myRenderCounter++),zOrder.size());
      QgsMapLayer *ml = QgsMapLayerRegistry::instance()->mapLayer( layerId );

      if ( ml && ( ml->type() != QgsMapLayer::RasterLayer ) )
      {
        // only make labels if the layer is visible
        // after scale dep viewing settings are checked
        if ( ml->isInScaleRange( mScale ) )
        {
          bool split = false;

          if ( hasCrsTransformEnabled() )
          {
            QgsRectangle r1 = mExtent;
            split = splitLayersExtent( ml, r1, r2 );
            ct = transformation( ml );
            mRenderContext.setExtent( r1 );
          }
          else
          {
            ct = nullptr;
          }

          mRenderContext.setCoordinateTransform( ct );

          ml->drawLabels( mRenderContext );
          if ( split )
          {
            mRenderContext.setExtent( r2 );
            ml->drawLabels( mRenderContext );
          }
        }
      }
    }
  } // if (!mOverview)

  // make sure progress bar arrives at 100%!
  emit drawingProgress( 1, 1 );

  if ( mLabelingEngine )
  {
    // set correct extent
    mRenderContext.setExtent( mExtent );
    mRenderContext.setCoordinateTransform( nullptr );

    mLabelingEngine->drawLabeling( mRenderContext );
    mLabelingEngine->exit();
  }

  QgsDebugMsg( "Rendering completed in (seconds): " + QString( "%1" ).arg( renderTime.elapsed() / 1000.0 ) );

  mDrawing = false;
}

void QgsMapRenderer::setMapUnits( QGis::UnitType u )
{
  mScaleCalculator->setMapUnits( u );

  // Since the map units have changed, force a recalculation of the scale.
  updateScale();

  emit mapUnitsChanged();
}

QGis::UnitType QgsMapRenderer::mapUnits() const
{
  return mScaleCalculator->mapUnits();
}

void QgsMapRenderer::onDrawingProgress( int current, int total )
{
  Q_UNUSED( current );
  Q_UNUSED( total );
}

void QgsMapRenderer::setProjectionsEnabled( bool enabled )
{
  if ( mProjectionsEnabled != enabled )
  {
    mProjectionsEnabled = enabled;
    QgsDebugMsg( "Adjusting DistArea projection on/off" );
    mDistArea->setEllipsoidalMode( enabled );
    updateFullExtent();
    mLastExtent.setMinimal();

    Q_NOWARN_DEPRECATED_PUSH
    emit hasCrsTransformEnabled( enabled ); // deprecated
    Q_NOWARN_DEPRECATED_POP

    emit hasCrsTransformEnabledChanged( enabled );
  }
}

bool QgsMapRenderer::hasCrsTransformEnabled() const
{
  return mProjectionsEnabled;
}

void QgsMapRenderer::setDestinationCrs( const QgsCoordinateReferenceSystem& crs, bool refreshCoordinateTransformInfo, bool transformExtent )
{
  QgsDebugMsg( "* Setting destCRS : = " + crs.toProj4() );
  QgsDebugMsg( "* DestCRS.srsid() = " + QString::number( crs.srsid() ) );
  if ( *mDestCRS != crs )
  {
    if ( refreshCoordinateTransformInfo )
    {
      mLayerCoordinateTransformInfo.clear();
    }
    QgsRectangle rect;
    if ( transformExtent && !mExtent.isEmpty() )
    {
      QgsCoordinateTransform transform( *mDestCRS, crs );
      try
      {
        rect = transform.transformBoundingBox( mExtent );
      }
      catch ( QgsCsException &e )
      {
        Q_UNUSED( e );
        QgsDebugMsg( QString( "Transform error caught: %1" ).arg( e.what() ) );
      }
    }

    QgsDebugMsg( "Setting DistArea CRS to " + QString::number( crs.srsid() ) );
    mDistArea->setSourceCrs( crs.srsid() );
    *mDestCRS = crs;
    updateFullExtent();

    if ( !rect.isEmpty() )
    {
      setExtent( rect );
    }

    emit destinationSrsChanged();
  }
}

const QgsCoordinateReferenceSystem& QgsMapRenderer::destinationCrs() const
{
  QgsDebugMsgLevel( "* Returning destCRS", 3 );
  QgsDebugMsgLevel( "* DestCRS.srsid() = " + QString::number( mDestCRS->srsid() ), 3 );
  QgsDebugMsgLevel( "* DestCRS.proj4() = " + mDestCRS->toProj4(), 3 );
  return *mDestCRS;
}


bool QgsMapRenderer::splitLayersExtent( QgsMapLayer* layer, QgsRectangle& extent, QgsRectangle& r2 )
{
  bool split = false;

  if ( hasCrsTransformEnabled() )
  {
    try
    {
#ifdef QGISDEBUG
      // QgsLogger::debug<QgsRectangle>("Getting extent of canvas in layers CS. Canvas is ", extent, __FILE__, __FUNCTION__, __LINE__);
#endif
      // Split the extent into two if the source CRS is
      // geographic and the extent crosses the split in
      // geographic coordinates (usually +/- 180 degrees,
      // and is assumed to be so here), and draw each
      // extent separately.
      static const double splitCoord = 180.0;

      const QgsCoordinateTransform *transform = transformation( layer );
      if ( layer->crs().geographicFlag() )
      {
        // Note: ll = lower left point
        //   and ur = upper right point

        QgsPoint ll( extent.xMinimum(), extent.yMinimum() );
        QgsPoint ur( extent.xMaximum(), extent.yMaximum() );

        if ( transform )
        {
          ll = transform->transform( ll.x(), ll.y(),
                                     QgsCoordinateTransform::ReverseTransform );
          ur = transform->transform( ur.x(), ur.y(),
                                     QgsCoordinateTransform::ReverseTransform );
          extent = transform->transformBoundingBox( extent, QgsCoordinateTransform::ReverseTransform );
        }

        if ( ll.x() > ur.x() )
        {
          r2 = extent;
          extent.setXMinimum( splitCoord );
          r2.setXMaximum( splitCoord );
          split = true;
        }
      }
      else // can't cross 180
      {
        if ( transform )
        {
          extent = transform->transformBoundingBox( extent, QgsCoordinateTransform::ReverseTransform );
        }
      }
    }
    catch ( QgsCsException &cse )
    {
      Q_UNUSED( cse );
      QgsDebugMsg( "Transform error caught" );
      extent = QgsRectangle( -DBL_MAX, -DBL_MAX, DBL_MAX, DBL_MAX );
      r2     = QgsRectangle( -DBL_MAX, -DBL_MAX, DBL_MAX, DBL_MAX );
    }
  }
  return split;
}

QgsRectangle QgsMapRenderer::layerExtentToOutputExtent( QgsMapLayer* theLayer, QgsRectangle extent )
{
  //QgsDebugMsg( QString( "sourceCrs = " + tr( theLayer )->sourceCrs().authid() ) );
  //QgsDebugMsg( QString( "destCRS = " + tr( theLayer )->destCRS().authid() ) );
  //QgsDebugMsg( QString( "extent = " + extent.toString() ) );
  if ( hasCrsTransformEnabled() )
  {
    try
    {
      const QgsCoordinateTransform *transform = transformation( theLayer );
      if ( transform )
      {
        extent = transform->transformBoundingBox( extent );
      }
    }
    catch ( QgsCsException &cse )
    {
      QgsMessageLog::logMessage( tr( "Transform error caught: %1" ).arg( cse.what() ), tr( "CRS" ) );
    }
  }

  QgsDebugMsg( QString( "proj extent = " + extent.toString() ) );

  return extent;
}

QgsRectangle QgsMapRenderer::outputExtentToLayerExtent( QgsMapLayer* theLayer, QgsRectangle extent )
{
#if QGISDEBUG
  const QgsCoordinateTransform *transform = transformation( theLayer );
  QgsDebugMsg( QString( "layer sourceCrs = " + ( transform ? transform->sourceCrs().authid() : "none" ) ) );
  QgsDebugMsg( QString( "layer destCRS = " + ( transform ? transform->destCRS().authid() : "none" ) ) );
  QgsDebugMsg( QString( "extent = " + extent.toString() ) );
#endif
  if ( hasCrsTransformEnabled() )
  {
    try
    {
      const QgsCoordinateTransform *transform = transformation( theLayer );
      if ( transform )
      {
        extent = transform->transformBoundingBox( extent, QgsCoordinateTransform::ReverseTransform );
      }
    }
    catch ( QgsCsException &cse )
    {
      QgsMessageLog::logMessage( tr( "Transform error caught: %1" ).arg( cse.what() ), tr( "CRS" ) );
    }
  }

  QgsDebugMsg( QString( "proj extent = " + extent.toString() ) );

  return extent;
}

QgsPoint QgsMapRenderer::layerToMapCoordinates( QgsMapLayer* theLayer, QgsPoint point )
{
  if ( hasCrsTransformEnabled() )
  {
    try
    {
      const QgsCoordinateTransform *transform = transformation( theLayer );
      if ( transform )
      {
        point = transform->transform( point, QgsCoordinateTransform::ForwardTransform );
      }
    }
    catch ( QgsCsException &cse )
    {
      QgsMessageLog::logMessage( QString( "Transform error caught: %1" ).arg( cse.what() ) );
    }
  }
  else
  {
    // leave point without transformation
  }
  return point;
}

QgsRectangle QgsMapRenderer::layerToMapCoordinates( QgsMapLayer* theLayer, QgsRectangle rect )
{
  if ( hasCrsTransformEnabled() )
  {
    try
    {
      const QgsCoordinateTransform *transform = transformation( theLayer );
      if ( transform )
      {
        rect = transform->transform( rect, QgsCoordinateTransform::ForwardTransform );
      }
    }
    catch ( QgsCsException &cse )
    {
      QgsMessageLog::logMessage( QString( "Transform error caught: %1" ).arg( cse.what() ) );
    }
  }
  else
  {
    // leave point without transformation
  }
  return rect;
}

QgsPoint QgsMapRenderer::mapToLayerCoordinates( QgsMapLayer* theLayer, QgsPoint point )
{
  if ( hasCrsTransformEnabled() )
  {
    try
    {
      const QgsCoordinateTransform *transform = transformation( theLayer );
      if ( transform )
        point = transform->transform( point, QgsCoordinateTransform::ReverseTransform );
    }
    catch ( QgsCsException &cse )
    {
      QgsMessageLog::logMessage( QString( "Transform error caught: %1" ).arg( cse.what() ) );
    }
  }
  else
  {
    // leave point without transformation
  }
  return point;
}

QgsRectangle QgsMapRenderer::mapToLayerCoordinates( QgsMapLayer* theLayer, QgsRectangle rect )
{
  if ( hasCrsTransformEnabled() )
  {
    try
    {
      const QgsCoordinateTransform *transform = transformation( theLayer );
      if ( transform )
        rect = transform->transform( rect, QgsCoordinateTransform::ReverseTransform );
    }
    catch ( QgsCsException &cse )
    {
      QgsMessageLog::logMessage( QString( "Transform error caught: %1" ).arg( cse.what() ) );
    }
  }
  return rect;
}

void QgsMapRenderer::updateFullExtent()
{
  mFullExtent.setMinimal();
}

QgsRectangle QgsMapRenderer::fullExtent()
{
  QgsDebugMsg( "called." );

  if ( !mFullExtent.isNull() )
    return mFullExtent;

  QgsMapLayerRegistry* registry = QgsMapLayerRegistry::instance();

  // iterate through the map layers and test each layers extent
  // against the current min and max values
  QgsDebugMsg( QString( "Layer count: %1" ).arg( mLayerSet.count() ) );
  Q_FOREACH ( const QString layerId, mLayerSet )
  {
    QgsMapLayer * lyr = registry->mapLayer( layerId );
    if ( !lyr )
    {
      QgsDebugMsg( QString( "WARNING: layer '%1' not found in map layer registry!" ).arg( layerId ) );
    }
    else
    {
      QgsDebugMsg( "Updating extent using " + lyr->name() );
      QgsDebugMsg( "Input extent: " + lyr->extent().toString() );

      if ( lyr->extent().isNull() )
      {
        continue;
      }

      // Layer extents are stored in the coordinate system (CS) of the
      // layer. The extent must be projected to the canvas CS
      QgsRectangle extent = layerExtentToOutputExtent( lyr, lyr->extent() );

      QgsDebugMsg( "Output extent: " + extent.toString() );
      mFullExtent.unionRect( extent );

    }
  }

  if ( mFullExtent.width() == 0.0 || mFullExtent.height() == 0.0 )
  {
    // If all of the features are at the one point, buffer the
    // rectangle a bit. If they are all at zero, do something a bit
    // more crude.

    if ( mFullExtent.xMinimum() == 0.0 && mFullExtent.xMaximum() == 0.0 &&
         mFullExtent.yMinimum() == 0.0 && mFullExtent.yMaximum() == 0.0 )
    {
      mFullExtent.set( -1.0, -1.0, 1.0, 1.0 );
    }
    else
    {
      const double padFactor = 1e-8;
      double widthPad = mFullExtent.xMinimum() * padFactor;
      double heightPad = mFullExtent.yMinimum() * padFactor;
      double xmin = mFullExtent.xMinimum() - widthPad;
      double xmax = mFullExtent.xMaximum() + widthPad;
      double ymin = mFullExtent.yMinimum() - heightPad;
      double ymax = mFullExtent.yMaximum() + heightPad;
      mFullExtent.set( xmin, ymin, xmax, ymax );
    }
  }

  QgsDebugMsg( "Full extent: " + mFullExtent.toString() );

  return mFullExtent;
}

void QgsMapRenderer::setLayerSet( const QStringList& layers )
{
  QgsDebugMsg( QString( "Entering: %1" ).arg( layers.join( ", " ) ) );
  mLayerSet = layers;
  updateFullExtent();
}

QStringList& QgsMapRenderer::layerSet()
{
  return mLayerSet;
}

bool QgsMapRenderer::readXML( QDomNode & theNode )
{
  QgsMapSettings tmpSettings;
  tmpSettings.readXML( theNode );
  //load coordinate transform into
  mLayerCoordinateTransformInfo.clear();
  QDomElement layerCoordTransformInfoElem = theNode.firstChildElement( "layer_coordinate_transform_info" );
  if ( !layerCoordTransformInfoElem.isNull() )
  {
    QDomNodeList layerCoordinateTransformList = layerCoordTransformInfoElem.elementsByTagName( "layer_coordinate_transform" );
    QDomElement layerCoordTransformElem;
    for ( int i = 0; i < layerCoordinateTransformList.size(); ++i )
    {
      layerCoordTransformElem = layerCoordinateTransformList.at( i ).toElement();
      QString layerId = layerCoordTransformElem.attribute( "layerid" );
      if ( layerId.isEmpty() )
      {
        continue;
      }

      QgsLayerCoordinateTransform lct;
      lct.srcAuthId = layerCoordTransformElem.attribute( "srcAuthId" );
      lct.destAuthId = layerCoordTransformElem.attribute( "destAuthId" );
      lct.srcDatumTransform = layerCoordTransformElem.attribute( "srcDatumTransform", "-1" ).toInt();
      lct.destDatumTransform = layerCoordTransformElem.attribute( "destDatumTransform", "-1" ).toInt();
      mLayerCoordinateTransformInfo.insert( layerId, lct );
    }
  }


  setMapUnits( tmpSettings.mapUnits() );
  setExtent( tmpSettings.extent() );
  setProjectionsEnabled( tmpSettings.hasCrsTransformEnabled() );
  setDestinationCrs( tmpSettings.destinationCrs() );


  return true;
}

bool QgsMapRenderer::writeXML( QDomNode & theNode, QDomDocument & theDoc )
{
  QgsMapSettings tmpSettings;
  tmpSettings.setOutputDpi( outputDpi() );
  tmpSettings.setOutputSize( outputSize() );
  tmpSettings.setMapUnits( mapUnits() );
  tmpSettings.setExtent( extent() );
  tmpSettings.setCrsTransformEnabled( hasCrsTransformEnabled() );
  tmpSettings.setDestinationCrs( destinationCrs() );

  tmpSettings.writeXML( theNode, theDoc );
  // layer coordinate transform infos
  QDomElement layerCoordTransformInfo = theDoc.createElement( "layer_coordinate_transform_info" );
  QHash< QString, QgsLayerCoordinateTransform >::const_iterator coordIt = mLayerCoordinateTransformInfo.constBegin();
  for ( ; coordIt != mLayerCoordinateTransformInfo.constEnd(); ++coordIt )
  {
    QDomElement layerCoordTransformElem = theDoc.createElement( "layer_coordinate_transform" );
    layerCoordTransformElem.setAttribute( "layerid", coordIt.key() );
    layerCoordTransformElem.setAttribute( "srcAuthId", coordIt->srcAuthId );
    layerCoordTransformElem.setAttribute( "destAuthId", coordIt->destAuthId );
    layerCoordTransformElem.setAttribute( "srcDatumTransform", QString::number( coordIt->srcDatumTransform ) );
    layerCoordTransformElem.setAttribute( "destDatumTransform", QString::number( coordIt->destDatumTransform ) );
    layerCoordTransformInfo.appendChild( layerCoordTransformElem );
  }
  theNode.appendChild( layerCoordTransformInfo );
  return true;
}

void QgsMapRenderer::setLabelingEngine( QgsLabelingEngineInterface* iface )
{
  if ( mLabelingEngine )
    delete mLabelingEngine;

  mLabelingEngine = iface;
}

const QgsCoordinateTransform *QgsMapRenderer::transformation( const QgsMapLayer *layer ) const
{
  if ( !layer || !mDestCRS )
  {
    return nullptr;
  }

  if ( layer->crs().authid() == mDestCRS->authid() )
  {
    return nullptr;
  }

  QHash< QString, QgsLayerCoordinateTransform >::const_iterator ctIt = mLayerCoordinateTransformInfo.find( layer->id() );
  if ( ctIt != mLayerCoordinateTransformInfo.constEnd()
       && ctIt->srcAuthId == layer->crs().authid()
       && ctIt->destAuthId == mDestCRS->authid() )
  {
    return QgsCoordinateTransformCache::instance()->transform( ctIt->srcAuthId, ctIt->destAuthId, ctIt->srcDatumTransform, ctIt->destDatumTransform );
  }
  else
  {
    //is there a defined datum transformation?
    QHash< QPair< QString, QString >, QPair< int, int > >::const_iterator it = mDefaultDatumTransformations.find( qMakePair( layer->crs().authid(), mDestCRS->authid() ) );
    if ( it != mDefaultDatumTransformations.constEnd() )
    {
      return QgsCoordinateTransformCache::instance()->transform( it.key().first, it.key().second, it.value().first, it.value().second );
    }
    emit datumTransformInfoRequested( layer, layer->crs().authid(), mDestCRS->authid() );
  }

  //still not present? get coordinate transformation with -1/-1 datum transform as default
  ctIt = mLayerCoordinateTransformInfo.find( layer->id() );
  if ( ctIt == mLayerCoordinateTransformInfo.constEnd()
       || ctIt->srcAuthId == layer->crs().authid()
       || ctIt->destAuthId == mDestCRS->authid()
     )
  {
    return QgsCoordinateTransformCache::instance()->transform( layer->crs().authid(), mDestCRS->authid() );
  }
  return QgsCoordinateTransformCache::instance()->transform( ctIt->srcAuthId, ctIt->destAuthId, ctIt->srcDatumTransform, ctIt->destDatumTransform );
}

/** Returns a QPainter::CompositionMode corresponding to a QgsMapRenderer::BlendMode
 */
QPainter::CompositionMode QgsMapRenderer::getCompositionMode( QgsMapRenderer::BlendMode blendMode )
{
  // Map QgsMapRenderer::BlendNormal to QPainter::CompositionMode
  switch ( blendMode )
  {
    case QgsMapRenderer::BlendNormal:
      return QPainter::CompositionMode_SourceOver;
    case QgsMapRenderer::BlendLighten:
      return QPainter::CompositionMode_Lighten;
    case QgsMapRenderer::BlendScreen:
      return QPainter::CompositionMode_Screen;
    case QgsMapRenderer::BlendDodge:
      return QPainter::CompositionMode_ColorDodge;
    case QgsMapRenderer::BlendAddition:
      return QPainter::CompositionMode_Plus;
    case QgsMapRenderer::BlendDarken:
      return QPainter::CompositionMode_Darken;
    case QgsMapRenderer::BlendMultiply:
      return QPainter::CompositionMode_Multiply;
    case QgsMapRenderer::BlendBurn:
      return QPainter::CompositionMode_ColorBurn;
    case QgsMapRenderer::BlendOverlay:
      return QPainter::CompositionMode_Overlay;
    case QgsMapRenderer::BlendSoftLight:
      return QPainter::CompositionMode_SoftLight;
    case QgsMapRenderer::BlendHardLight:
      return QPainter::CompositionMode_HardLight;
    case QgsMapRenderer::BlendDifference:
      return QPainter::CompositionMode_Difference;
    case QgsMapRenderer::BlendSubtract:
      return QPainter::CompositionMode_Exclusion;
    case QgsMapRenderer::BlendSource:
      return QPainter::CompositionMode_Source;
    case QgsMapRenderer::BlendDestinationOver:
      return QPainter::CompositionMode_DestinationOver;
    case QgsMapRenderer::BlendClear:
      return QPainter::CompositionMode_Clear;
    case QgsMapRenderer::BlendDestination:
      return QPainter::CompositionMode_Destination;
    case QgsMapRenderer::BlendSourceIn:
      return QPainter::CompositionMode_SourceIn;
    case QgsMapRenderer::BlendDestinationIn:
      return QPainter::CompositionMode_DestinationIn;
    case QgsMapRenderer::BlendSourceOut:
      return QPainter::CompositionMode_SourceOut;
    case QgsMapRenderer::BlendDestinationOut:
      return QPainter::CompositionMode_DestinationOut;
    case QgsMapRenderer::BlendSourceAtop:
      return QPainter::CompositionMode_SourceAtop;
    case QgsMapRenderer::BlendDestinationAtop:
      return QPainter::CompositionMode_DestinationAtop;
    case QgsMapRenderer::BlendXor:
      return QPainter::CompositionMode_Xor;
    default:
      QgsDebugMsg( QString( "Blend mode %1 mapped to SourceOver" ).arg( blendMode ) );
      return QPainter::CompositionMode_SourceOver;
  }
}

QgsMapRenderer::BlendMode QgsMapRenderer::getBlendModeEnum( QPainter::CompositionMode blendMode )
{
  // Map QPainter::CompositionMode to QgsMapRenderer::BlendNormal
  switch ( blendMode )
  {
    case QPainter::CompositionMode_SourceOver:
      return QgsMapRenderer::BlendNormal;
    case QPainter::CompositionMode_Lighten:
      return QgsMapRenderer::BlendLighten;
    case QPainter::CompositionMode_Screen:
      return QgsMapRenderer::BlendScreen;
    case QPainter::CompositionMode_ColorDodge:
      return QgsMapRenderer::BlendDodge;
    case QPainter::CompositionMode_Plus:
      return QgsMapRenderer::BlendAddition;
    case QPainter::CompositionMode_Darken:
      return QgsMapRenderer::BlendDarken;
    case QPainter::CompositionMode_Multiply:
      return QgsMapRenderer::BlendMultiply;
    case QPainter::CompositionMode_ColorBurn:
      return QgsMapRenderer::BlendBurn;
    case QPainter::CompositionMode_Overlay:
      return QgsMapRenderer::BlendOverlay;
    case QPainter::CompositionMode_SoftLight:
      return QgsMapRenderer::BlendSoftLight;
    case QPainter::CompositionMode_HardLight:
      return QgsMapRenderer::BlendHardLight;
    case QPainter::CompositionMode_Difference:
      return QgsMapRenderer::BlendDifference;
    case QPainter::CompositionMode_Exclusion:
      return QgsMapRenderer::BlendSubtract;
    case QPainter::CompositionMode_Source:
      return QgsMapRenderer::BlendSource;
    case QPainter::CompositionMode_DestinationOver:
      return QgsMapRenderer::BlendDestinationOver;
    case QPainter::CompositionMode_Clear:
      return QgsMapRenderer::BlendClear;
    case QPainter::CompositionMode_Destination:
      return QgsMapRenderer::BlendDestination;
    case QPainter::CompositionMode_SourceIn:
      return QgsMapRenderer::BlendSourceIn;
    case QPainter::CompositionMode_DestinationIn:
      return QgsMapRenderer::BlendDestinationIn;
    case QPainter::CompositionMode_SourceOut:
      return QgsMapRenderer::BlendSourceOut;
    case QPainter::CompositionMode_DestinationOut:
      return QgsMapRenderer::BlendDestinationOut;
    case QPainter::CompositionMode_SourceAtop:
      return QgsMapRenderer::BlendSourceAtop;
    case QPainter::CompositionMode_DestinationAtop:
      return QgsMapRenderer::BlendDestinationAtop;
    case QPainter::CompositionMode_Xor:
      return QgsMapRenderer::BlendXor;
    default:
      QgsDebugMsg( QString( "Composition mode %1 mapped to Normal" ).arg( blendMode ) );
      return QgsMapRenderer::BlendNormal;
  }
}

Q_GUI_EXPORT extern int qt_defaultDpiX();

const QgsMapSettings& QgsMapRenderer::mapSettings()
{
  // make sure the settings object is up-to-date
  mMapSettings.setExtent( extent() );
  mMapSettings.setOutputSize( outputSize() );
  mMapSettings.setOutputDpi( !qgsDoubleNear( outputDpi(), 0 ) ? outputDpi() : qt_defaultDpiX() );
  mMapSettings.setLayers( layerSet() );
  mMapSettings.setCrsTransformEnabled( hasCrsTransformEnabled() );
  mMapSettings.setDestinationCrs( destinationCrs() );
  mMapSettings.setMapUnits( mapUnits() );
  return mMapSettings;
}

void QgsMapRenderer::addLayerCoordinateTransform( const QString& layerId, const QString& srcAuthId, const QString& destAuthId, int srcDatumTransform, int destDatumTransform )
{
  QgsLayerCoordinateTransform lt;
  lt.srcAuthId = srcAuthId;
  lt.destAuthId = destAuthId;
  lt.srcDatumTransform = srcDatumTransform;
  lt.destDatumTransform = destDatumTransform;
  mLayerCoordinateTransformInfo.insert( layerId, lt );
}

void QgsMapRenderer::clearLayerCoordinateTransforms()
{
  mLayerCoordinateTransformInfo.clear();
}

void QgsMapRenderer::readDefaultDatumTransformations()
{
  const char* envChar = getenv( "DEFAULT_DATUM_TRANSFORM" );
  if ( envChar )
  {
    QString envString( envChar );
    QStringList transformSplit = envString.split( ";" );
    for ( int i = 0; i < transformSplit.size(); ++i )
    {
      QStringList slashSplit = transformSplit.at( i ).split( "/" );
      if ( slashSplit.size() < 4 )
      {
        continue;
      }

      mDefaultDatumTransformations.insert( qMakePair( slashSplit.at( 0 ), slashSplit.at( 1 ) ), qMakePair( slashSplit.at( 2 ).toInt(), slashSplit.at( 3 ).toInt() ) );
    }
  }
}

bool QgsMapRenderer::mDrawing = false;
