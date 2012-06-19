/***************************************************************************
                         qgsrasterresamplefilter.cpp
                         ---------------------
    begin                : December 2011
    copyright            : (C) 2011 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrasterresamplefilter.h"
#include "qgsrasterresampler.h"
#include "qgsrasterprojector.h"
#include "qgsrastertransparency.h"
#include "qgsrasterviewport.h"
#include "qgsmaptopixel.h"

//resamplers
#include "qgsbilinearrasterresampler.h"
#include "qgscubicrasterresampler.h"

#include <QDomDocument>
#include <QDomElement>
#include <QImage>
#include <QPainter>

QgsRasterResampleFilter::QgsRasterResampleFilter( QgsRasterInterface* input ): QgsRasterInterface( input ),
    mZoomedInResampler( 0 ), mZoomedOutResampler( 0 ),
    mMaxOversampling( 2.0 )
{
}

QgsRasterResampleFilter::~QgsRasterResampleFilter()
{
  // TODO: currently we are using pointer to renderer, enable once moved here
  //delete mZoomedInResampler;
  //delete mZoomedOutResampler;
  //delete mRasterTransparency;
}

void QgsRasterResampleFilter::setZoomedInResampler( QgsRasterResampler* r )
{
  //delete mZoomedInResampler;
  mZoomedInResampler = r;
}

void QgsRasterResampleFilter::setZoomedOutResampler( QgsRasterResampler* r )
{
  //delete mZoomedOutResampler;
  mZoomedOutResampler = r;
}

void * QgsRasterResampleFilter::readBlock( int bandNo, QgsRectangle  const & extent, int width, int height )
{
  QgsDebugMsg( "Entered" );
  if ( !mInput ) return 0;

  double oversampling = 1.0; // approximate global oversampling factor

  if ( mZoomedInResampler || mZoomedOutResampler )
  {
    // TODO: we must get it somehow from pipe (via projector), for now
    oversampling = 2.;
    /* 
    QgsRectangle providerExtent = mInput->extent();
    if ( viewPort->mSrcCRS.isValid() && viewPort->mDestCRS.isValid() && viewPort->mSrcCRS != viewPort->mDestCRS )
    {
      QgsCoordinateTransform t( viewPort->mSrcCRS, viewPort->mDestCRS );
      providerExtent = t.transformBoundingBox( providerExtent );
    }
    double pixelRatio = mapToPixel->mapUnitsPerPixel() / ( providerExtent.width() / mInput->xSize() );
    oversampling = ( pixelRatio > mMaxOversampling ) ? mMaxOversampling : pixelRatio;
    */
  }

  //set oversampling back to 1.0 if no resampler for zoomed in / zoomed out (nearest neighbour)
  if (( oversampling < 1.0 && !mZoomedInResampler ) || ( oversampling > 1.0 && !mZoomedOutResampler ) )
  {
    oversampling = 1.0;
  }

  QgsDebugMsg( QString( "oversampling %1" ).arg( oversampling ) );

  //effective oversampling factors are different to global one because of rounding
  double oversamplingX = (( double )width * oversampling ) / width;
  double oversamplingY = (( double )height * oversampling ) / height;

  // TODO: we must also increase the extent to get correct result on borders of parts


  int resWidth = width*oversamplingX;
  int resHeight = height*oversamplingY;

  // At moment we know that we read rendered image 
  int bandNumber = 0;
  void *rasterData = mInput->readBlock( bandNumber, extent, resWidth, resHeight );

  //resample image
  if (( mZoomedInResampler || mZoomedOutResampler ) && !doubleNear( oversamplingX, 1.0 ) && !doubleNear( oversamplingY, 1.0 ) )
  {
    QImage img(( uchar * ) rasterData, resWidth, resHeight, QImage::Format_ARGB32_Premultiplied );

    QImage dstImg = QImage( width, height, QImage::Format_ARGB32_Premultiplied );

    if ( mZoomedInResampler && oversamplingX < 1.0 )
    {
      QgsDebugMsg( "zoomed in resampling" );
      mZoomedInResampler->resample( img, dstImg );
    }
    else if ( mZoomedOutResampler && oversamplingX > 1.0 )
    {
      QgsDebugMsg( "zoomed out resampling" );
      mZoomedOutResampler->resample( img, dstImg );
    }

    // QImage does not delete data block passed to constructor
    free( rasterData );

    void * data = VSIMalloc( dstImg.byteCount() );
    return memcpy( data, dstImg.bits(), dstImg.byteCount() );
  }

  return rasterData; // No resampling
}

void QgsRasterResampleFilter::writeXML( QDomDocument& doc, QDomElement& parentElem ) 
{
  if ( parentElem.isNull() )
  {
    return;
  }

  QDomElement rasterRendererElem = doc.createElement( "rasterresampler" );

  rasterRendererElem.setAttribute( "maxOversampling", mMaxOversampling );
  if ( mZoomedInResampler )
  {
    rasterRendererElem.setAttribute( "zoomedInResampler", mZoomedInResampler->type() );
  }
  if ( mZoomedOutResampler )
  {
    rasterRendererElem.setAttribute( "zoomedOutResampler", mZoomedOutResampler->type() );
  }
  parentElem.appendChild( rasterRendererElem );
}

void QgsRasterResampleFilter::readXML( const QDomElement& rendererElem )
{
  if ( rendererElem.isNull() )
  {
    return;
  }

  mMaxOversampling = rendererElem.attribute( "maxOversampling", "2.0" ).toDouble();

  QString zoomedInResamplerType = rendererElem.attribute( "zoomedInResampler" );
  if ( zoomedInResamplerType == "bilinear" )
  {
    mZoomedInResampler = new QgsBilinearRasterResampler();
  }
  else if ( zoomedInResamplerType == "cubic" )
  {
    mZoomedInResampler = new QgsCubicRasterResampler();
  }

  QString zoomedOutResamplerType = rendererElem.attribute( "zoomedOutResampler" );
  if ( zoomedOutResamplerType == "bilinear" )
  {
    mZoomedOutResampler = new QgsBilinearRasterResampler();
  }
}
