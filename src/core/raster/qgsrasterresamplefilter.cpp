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

QgsRasterResampleFilter::QgsRasterResampleFilter( QgsRasterInterface* input )
    : QgsRasterInterface( input )
    , mZoomedInResampler( nullptr )
    , mZoomedOutResampler( nullptr )
    , mMaxOversampling( 2.0 )
{
}

QgsRasterResampleFilter::~QgsRasterResampleFilter()
{
  delete mZoomedInResampler;
  delete mZoomedOutResampler;
}

QgsRasterResampleFilter* QgsRasterResampleFilter::clone() const
{
  QgsDebugMsgLevel( "Entered", 4 );
  QgsRasterResampleFilter * resampler = new QgsRasterResampleFilter( nullptr );
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

QGis::DataType QgsRasterResampleFilter::dataType( int bandNo ) const
{
  if ( mOn ) return QGis::ARGB32_Premultiplied;

  if ( mInput ) return mInput->dataType( bandNo );

  return QGis::UnknownDataType;
}

bool QgsRasterResampleFilter::setInput( QgsRasterInterface* input )
{
  QgsDebugMsgLevel( "Entered", 4 );

  // Resampler can only work with single band ARGB32_Premultiplied
  if ( !input )
  {
    QgsDebugMsg( "No input" );
    return false;
  }

  if ( !mOn )
  {
    // In off mode we can connect to anything
    QgsDebugMsgLevel( "OK", 4 );
    mInput = input;
    return true;
  }

  if ( input->bandCount() < 1 )
  {
    QgsDebugMsg( "No input band" );
    return false;
  }

  if ( input->dataType( 1 ) != QGis::ARGB32_Premultiplied &&
       input->dataType( 1 ) != QGis::ARGB32 )
  {
    QgsDebugMsg( "Unknown input data type" );
    return false;
  }

  mInput = input;
  QgsDebugMsgLevel( "OK", 4 );
  return true;
}

void QgsRasterResampleFilter::setZoomedInResampler( QgsRasterResampler* r )
{
  delete mZoomedInResampler;
  mZoomedInResampler = r;
}

void QgsRasterResampleFilter::setZoomedOutResampler( QgsRasterResampler* r )
{
  delete mZoomedOutResampler;
  mZoomedOutResampler = r;
}

QgsRasterBlock * QgsRasterResampleFilter::block( int bandNo, QgsRectangle  const & extent, int width, int height )
{
  Q_UNUSED( bandNo );
  QgsDebugMsgLevel( QString( "width = %1 height = %2 extent = %3" ).arg( width ).arg( height ).arg( extent.toString() ), 4 );
  QgsRasterBlock *outputBlock = new QgsRasterBlock();
  if ( !mInput ) return outputBlock;

  double oversampling = 1.0; // approximate global oversampling factor

  if ( mZoomedInResampler || mZoomedOutResampler )
  {
    QgsRasterDataProvider *provider = dynamic_cast<QgsRasterDataProvider*>( mInput->srcInput() );
    if ( provider && ( provider->capabilities() & QgsRasterDataProvider::Size ) )
    {
      double xRes = extent.width() / width;
      double providerXRes = provider->extent().width() / provider->xSize();
      double pixelRatio = xRes / providerXRes;
      oversampling = ( pixelRatio > mMaxOversampling ) ? mMaxOversampling : pixelRatio;
      QgsDebugMsgLevel( QString( "xRes = %1 providerXRes = %2 pixelRatio = %3 oversampling = %4" ).arg( xRes ).arg( providerXRes ).arg( pixelRatio ).arg( oversampling ), 4 );
    }
    else
    {
      // We don't know exact data source resolution (WMS) so we expect that
      // server data have higher resolution (which is not always true) and use
      // mMaxOversampling
      oversampling = mMaxOversampling;
    }
  }

  QgsDebugMsgLevel( QString( "oversampling %1" ).arg( oversampling ), 4 );

  int bandNumber = 1;

  // Do no oversampling if no resampler for zoomed in / zoomed out (nearest neighbour)
  // We do mZoomedInResampler if oversampling == 1 (otherwise for example reprojected
  // zoom in rasters are never resampled because projector limits resolution.
  if ((( oversampling < 1.0 || qgsDoubleNear( oversampling, 1.0 ) ) && !mZoomedInResampler ) || ( oversampling > 1.0 && !mZoomedOutResampler ) )
  {
    QgsDebugMsgLevel( "No oversampling.", 4 );
    delete outputBlock;
    return mInput->block( bandNumber, extent, width, height );
  }

  //effective oversampling factors are different to global one because of rounding
  double oversamplingX = ( static_cast< double >( width ) * oversampling ) / width;
  double oversamplingY = ( static_cast< double >( height ) * oversampling ) / height;

  // TODO: we must also increase the extent to get correct result on borders of parts

  int resWidth = width * oversamplingX;
  int resHeight = height * oversamplingY;

  QgsRasterBlock *inputBlock = mInput->block( bandNumber, extent, resWidth, resHeight );
  if ( !inputBlock || inputBlock->isEmpty() )
  {
    QgsDebugMsg( "No raster data!" );
    delete inputBlock;
    return outputBlock;
  }

  if ( !outputBlock->reset( QGis::ARGB32_Premultiplied, width, height ) )
  {
    delete inputBlock;
    return outputBlock;
  }

  //resample image
  QImage img = inputBlock->image();

  QImage dstImg = QImage( width, height, QImage::Format_ARGB32_Premultiplied );

  if ( mZoomedInResampler && ( oversamplingX < 1.0 || qgsDoubleNear( oversampling, 1.0 ) ) )
  {
    QgsDebugMsgLevel( "zoomed in resampling", 4 );
    mZoomedInResampler->resample( img, dstImg );
  }
  else if ( mZoomedOutResampler && oversamplingX > 1.0 )
  {
    QgsDebugMsgLevel( "zoomed out resampling", 4 );
    mZoomedOutResampler->resample( img, dstImg );
  }
  else
  {
    // Should not happen
    QgsDebugMsg( "Unexpected resampling" );
    dstImg = img.scaled( width, height );
  }

  outputBlock->setImage( &dstImg );

  delete inputBlock;
  return outputBlock; // No resampling
}

void QgsRasterResampleFilter::writeXML( QDomDocument& doc, QDomElement& parentElem ) const
{
  if ( parentElem.isNull() )
  {
    return;
  }

  QDomElement rasterRendererElem = doc.createElement( "rasterresampler" );

  rasterRendererElem.setAttribute( "maxOversampling", QString::number( mMaxOversampling ) );
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

void QgsRasterResampleFilter::readXML( const QDomElement& filterElem )
{
  if ( filterElem.isNull() )
  {
    return;
  }

  mMaxOversampling = filterElem.attribute( "maxOversampling", "2.0" ).toDouble();

  QString zoomedInResamplerType = filterElem.attribute( "zoomedInResampler" );
  if ( zoomedInResamplerType == "bilinear" )
  {
    mZoomedInResampler = new QgsBilinearRasterResampler();
  }
  else if ( zoomedInResamplerType == "cubic" )
  {
    mZoomedInResampler = new QgsCubicRasterResampler();
  }

  QString zoomedOutResamplerType = filterElem.attribute( "zoomedOutResampler" );
  if ( zoomedOutResamplerType == "bilinear" )
  {
    mZoomedOutResampler = new QgsBilinearRasterResampler();
  }
}
