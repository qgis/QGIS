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
    : QgsRasterInterface( input ),
    mZoomedInResampler( 0 ), mZoomedOutResampler( 0 ),
    mMaxOversampling( 2.0 )
{
}

QgsRasterResampleFilter::~QgsRasterResampleFilter()
{
  delete mZoomedInResampler;
  delete mZoomedOutResampler;
}

QgsRasterInterface * QgsRasterResampleFilter::clone() const
{
  QgsDebugMsg( "Entered" );
  QgsRasterResampleFilter * resampler = new QgsRasterResampleFilter( 0 );
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
  QgsDebugMsg( "Entered" );

  // Resampler can only work with single band ARGB32_Premultiplied
  if ( !input )
  {
    QgsDebugMsg( "No input" );
    return false;
  }

  if ( !mOn )
  {
    // In off mode we can connect to anything
    QgsDebugMsg( "OK" );
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
  QgsDebugMsg( "OK" );
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
  QgsDebugMsg( "Entered" );
  QgsRasterBlock *outputBlock = new QgsRasterBlock();
  if ( !mInput ) return outputBlock;

  double oversampling = 1.0; // approximate global oversampling factor

  if ( mZoomedInResampler || mZoomedOutResampler )
  {
    QgsRasterDataProvider *provider = dynamic_cast<QgsRasterDataProvider*>( mInput->srcInput() );
    // Do not oversample if data source does not have fixed resolution (WMS)
    if ( provider && ( provider->capabilities() & QgsRasterDataProvider::ExactResolution ) )
    {
      double xRes = extent.width() / width;
      double providerXRes = provider->extent().width() / provider->xSize();
      double pixelRatio = xRes / providerXRes;
      oversampling = ( pixelRatio > mMaxOversampling ) ? mMaxOversampling : pixelRatio;
      QgsDebugMsg( QString( "xRes = %1 providerXRes = %2 pixelRatio = %3 oversampling = %4" ).arg( xRes ).arg( providerXRes ).arg( pixelRatio ).arg( oversampling ) );
    }
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

  int resWidth = width * oversamplingX;
  int resHeight = height * oversamplingY;

  // At moment we know that we read rendered image
  int bandNumber = 1;
  //void *rasterData = mInput->block( bandNumber, extent, resWidth, resHeight );
  QgsRasterBlock *inputBlock = mInput->block( bandNumber, extent, resWidth, resHeight );
  if ( !inputBlock || inputBlock->isEmpty() )
  {
    QgsDebugMsg( "No raster data!" );
    delete inputBlock;
    return outputBlock;
  }

  if ( doubleNear( oversamplingX, 1.0 ) || doubleNear( oversamplingY, 1.0 ) )
  {
    QgsDebugMsg( "No oversampling." );
    delete outputBlock;
    return inputBlock;
  }

  if ( !outputBlock->reset( QGis::ARGB32_Premultiplied, width, height ) )
  {
    delete inputBlock;
    return outputBlock;
  }

  //resample image
  QImage img = inputBlock->image();

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

void QgsRasterResampleFilter::writeXML( QDomDocument& doc, QDomElement& parentElem )
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
