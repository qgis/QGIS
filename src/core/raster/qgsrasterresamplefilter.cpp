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

#include "qgsrasterdataprovider.h"
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

QgsRasterResampleFilter::QgsRasterResampleFilter( QgsRasterInterface *input )
  : QgsRasterInterface( input )
{
}

QgsRasterResampleFilter *QgsRasterResampleFilter::clone() const
{
  QgsDebugMsgLevel( QStringLiteral( "Entered" ), 4 );
  QgsRasterResampleFilter *resampler = new QgsRasterResampleFilter( nullptr );
  if ( mZoomedInResampler )
  {
    resampler->setZoomedInResampler( mZoomedInResampler->clone() );
  }
  if ( mZoomedOutResampler )
  {
    resampler->setZoomedOutResampler( mZoomedOutResampler->clone() );
  }
  resampler->setMaxOversampling( mMaxOversampling );
  return resampler;
}

int QgsRasterResampleFilter::bandCount() const
{
  if ( mOn ) return 1;

  if ( mInput ) return mInput->bandCount();

  return 0;
}

Qgis::DataType QgsRasterResampleFilter::dataType( int bandNo ) const
{
  if ( mOn ) return Qgis::DataType::ARGB32_Premultiplied;

  if ( mInput ) return mInput->dataType( bandNo );

  return Qgis::DataType::UnknownDataType;
}

bool QgsRasterResampleFilter::setInput( QgsRasterInterface *input )
{
  QgsDebugMsgLevel( QStringLiteral( "Entered" ), 4 );

  // Resampler can only work with single band ARGB32_Premultiplied
  if ( !input )
  {
    QgsDebugMsg( QStringLiteral( "No input" ) );
    return false;
  }

  if ( !mOn )
  {
    // In off mode we can connect to anything
    QgsDebugMsgLevel( QStringLiteral( "OK" ), 4 );
    mInput = input;
    return true;
  }

  if ( input->bandCount() < 1 )
  {
    QgsDebugMsg( QStringLiteral( "No input band" ) );
    return false;
  }

  if ( input->dataType( 1 ) != Qgis::DataType::ARGB32_Premultiplied &&
       input->dataType( 1 ) != Qgis::DataType::ARGB32 )
  {
    QgsDebugMsg( QStringLiteral( "Unknown input data type" ) );
    return false;
  }

  mInput = input;
  QgsDebugMsgLevel( QStringLiteral( "OK" ), 4 );
  return true;
}

void QgsRasterResampleFilter::setZoomedInResampler( QgsRasterResampler *r )
{
  mZoomedInResampler.reset( r );
}

void QgsRasterResampleFilter::setZoomedOutResampler( QgsRasterResampler *r )
{
  mZoomedOutResampler.reset( r );
}

QgsRasterBlock *QgsRasterResampleFilter::block( int bandNo, QgsRectangle  const &extent, int width, int height, QgsRasterBlockFeedback *feedback )
{
  if ( !mOn && mInput )
    return mInput->block( bandNo, extent, width, height, feedback );

  const int bandNumber = 1;

  QgsDebugMsgLevel( QStringLiteral( "width = %1 height = %2 extent = %3" ).arg( width ).arg( height ).arg( extent.toString() ), 4 );
  std::unique_ptr< QgsRasterBlock > outputBlock( new QgsRasterBlock() );
  if ( !mInput )
    return outputBlock.release();

  double oversampling = 1.0; // approximate global oversampling factor
  double outputXRes;
  double providerXRes = 0;
  if ( mZoomedInResampler || mZoomedOutResampler )
  {
    QgsRasterDataProvider *provider = dynamic_cast<QgsRasterDataProvider *>( mInput->sourceInput() );
    if ( provider && ( provider->capabilities() & QgsRasterDataProvider::Size ) )
    {
      outputXRes = extent.width() / width;
      providerXRes = provider->extent().width() / provider->xSize();
      const double pixelRatio = outputXRes / providerXRes;
      oversampling = ( pixelRatio > mMaxOversampling ) ? mMaxOversampling : pixelRatio;
      QgsDebugMsgLevel( QStringLiteral( "xRes = %1 providerXRes = %2 pixelRatio = %3 oversampling = %4" ).arg( outputXRes ).arg( providerXRes ).arg( pixelRatio ).arg( oversampling ), 4 );
    }
    else
    {
      // We don't know exact data source resolution (WMS) so we expect that
      // server data have higher resolution (which is not always true) and use
      // mMaxOversampling
      oversampling = mMaxOversampling;
    }
  }

  QgsDebugMsgLevel( QStringLiteral( "oversampling %1" ).arg( oversampling ), 4 );

  // Do no oversampling if no resampler for zoomed in / zoomed out (nearest neighbour)
  // We do mZoomedInResampler if oversampling == 1 (otherwise for example reprojected
  // zoom in rasters are never resampled because projector limits resolution.
  if ( ( ( oversampling < 1.0 || qgsDoubleNear( oversampling, 1.0 ) ) && !mZoomedInResampler ) || ( oversampling > 1.0 && !mZoomedOutResampler ) )
  {
    QgsDebugMsgLevel( QStringLiteral( "No oversampling." ), 4 );
    return mInput->block( bandNumber, extent, width, height, feedback );
  }

  //effective oversampling factors are different to global one because of rounding
  const double oversamplingX = ( static_cast< double >( width ) * oversampling ) / width;
  const double oversamplingY = ( static_cast< double >( height ) * oversampling ) / height;

  // we must also increase the extent to get correct result on borders of parts
  int tileBufferPixels = 0;
  if ( providerXRes != 0 )
  {
    if ( mZoomedInResampler && ( oversamplingX < 1.0 || qgsDoubleNear( oversampling, 1.0 ) ) )
    {
      tileBufferPixels = static_cast< int >( std::ceil( mZoomedInResampler->tileBufferPixels() * oversampling ) );
    }
    else if ( mZoomedOutResampler && oversamplingX > 1.0 )
    {
      tileBufferPixels = static_cast< int >( std::ceil( mZoomedOutResampler->tileBufferPixels() * oversampling ) );
    }
  }
  const double sourceTileBufferSize = providerXRes * tileBufferPixels;

  const QgsRectangle bufferedExtent( extent.xMinimum() - sourceTileBufferSize,
                                     extent.yMinimum() - sourceTileBufferSize,
                                     extent.xMaximum() + sourceTileBufferSize,
                                     extent.yMaximum() + sourceTileBufferSize
                                   );

  const int resWidth = static_cast< int >( std::round( width * oversamplingX ) ) + 2 * tileBufferPixels;
  const int resHeight = static_cast< int >( std::round( height * oversamplingY ) ) + 2 * tileBufferPixels;

  std::unique_ptr< QgsRasterBlock > inputBlock( mInput->block( bandNumber, bufferedExtent, resWidth, resHeight, feedback ) );
  if ( !inputBlock || inputBlock->isEmpty() )
  {
    QgsDebugMsg( QStringLiteral( "No raster data!" ) );
    return outputBlock.release();
  }

  if ( !outputBlock->reset( Qgis::DataType::ARGB32_Premultiplied, width, height ) )
  {
    return outputBlock.release();
  }

  //resample image
  const QImage img = inputBlock->image();

  const int resampleWidth = static_cast< int >( std::round( width * ( bufferedExtent.width() / extent.width() ) ) );
  const int resampleHeight = static_cast< int >( std::round( height * ( bufferedExtent.height() / extent.height() ) ) );

  QImage dstImg;
  if ( mZoomedInResampler && ( oversamplingX < 1.0 || qgsDoubleNear( oversampling, 1.0 ) ) )
  {
    QgsDebugMsgLevel( QStringLiteral( "zoomed in resampling" ), 4 );

    if ( QgsRasterResamplerV2 *resamplerV2 = dynamic_cast< QgsRasterResamplerV2 * >( mZoomedInResampler.get( ) ) )
    {
      dstImg = resamplerV2->resampleV2( img, QSize( resampleWidth, resampleHeight ) );
    }
    else
    {
      // old inefficient interface
      Q_NOWARN_DEPRECATED_PUSH
      QImage dstImg = QImage( resampleWidth, resampleHeight, QImage::Format_ARGB32_Premultiplied );
      mZoomedInResampler->resample( img, dstImg );
      Q_NOWARN_DEPRECATED_POP
    }
  }
  else if ( mZoomedOutResampler && oversamplingX > 1.0 )
  {
    QgsDebugMsgLevel( QStringLiteral( "zoomed out resampling" ), 4 );

    if ( QgsRasterResamplerV2 *resamplerV2 = dynamic_cast< QgsRasterResamplerV2 * >( mZoomedOutResampler.get( ) ) )
    {
      dstImg = resamplerV2->resampleV2( img, QSize( resampleWidth, resampleHeight ) );
    }
    else
    {
      // old inefficient interface
      Q_NOWARN_DEPRECATED_PUSH
      QImage dstImg = QImage( resampleWidth, resampleHeight, QImage::Format_ARGB32_Premultiplied );
      mZoomedOutResampler->resample( img, dstImg );
      Q_NOWARN_DEPRECATED_POP
    }
  }
  else
  {
    // Should not happen
    QgsDebugMsg( QStringLiteral( "Unexpected resampling" ) );
    dstImg = img.scaled( width, height );
  }

  // extract desired part of dstImage
  const QImage cropped = tileBufferPixels > 0 ? dstImg.copy( ( resampleWidth - width ) / 2, ( resampleHeight - height ) / 2, width, height )
                         : dstImg; // otherwise implicit copy, nice and cheap
  outputBlock->setImage( &cropped );

  return outputBlock.release(); // No resampling
}

void QgsRasterResampleFilter::writeXml( QDomDocument &doc, QDomElement &parentElem ) const
{
  if ( parentElem.isNull() )
  {
    return;
  }

  QDomElement rasterRendererElem = doc.createElement( QStringLiteral( "rasterresampler" ) );

  rasterRendererElem.setAttribute( QStringLiteral( "maxOversampling" ), QString::number( mMaxOversampling ) );
  if ( mZoomedInResampler )
  {
    rasterRendererElem.setAttribute( QStringLiteral( "zoomedInResampler" ), mZoomedInResampler->type() );
  }
  if ( mZoomedOutResampler )
  {
    rasterRendererElem.setAttribute( QStringLiteral( "zoomedOutResampler" ), mZoomedOutResampler->type() );
  }
  parentElem.appendChild( rasterRendererElem );
}

void QgsRasterResampleFilter::readXml( const QDomElement &filterElem )
{
  if ( filterElem.isNull() )
  {
    return;
  }

  mMaxOversampling = filterElem.attribute( QStringLiteral( "maxOversampling" ), QStringLiteral( "2.0" ) ).toDouble();

  const QString zoomedInResamplerType = filterElem.attribute( QStringLiteral( "zoomedInResampler" ) );
  if ( zoomedInResamplerType == QLatin1String( "bilinear" ) )
  {
    mZoomedInResampler.reset( new QgsBilinearRasterResampler() );
  }
  else if ( zoomedInResamplerType == QLatin1String( "cubic" ) )
  {
    mZoomedInResampler.reset( new QgsCubicRasterResampler() );
  }

  const QString zoomedOutResamplerType = filterElem.attribute( QStringLiteral( "zoomedOutResampler" ) );
  if ( zoomedOutResamplerType == QLatin1String( "bilinear" ) )
  {
    mZoomedOutResampler.reset( new QgsBilinearRasterResampler() );
  }
  else if ( zoomedOutResamplerType == QLatin1String( "cubic" ) )
  {
    mZoomedOutResampler.reset( new QgsCubicRasterResampler() );
  }
}
