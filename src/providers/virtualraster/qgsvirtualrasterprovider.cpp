/***************************************************************************
   qgsvirtualrasterprovider.cpp - QgsRasterDataProvider
     --------------------------------------
    Date                 : June 10, 2021
    Copyright            : (C) 2021 by Francesco Bursi
    email                : francesco dot bursi at hotmail dot it
***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsvirtualrasterprovider.h"
#include "qgsrastermatrix.h"
#include "qgsrasterlayer.h"
#include "qgsrasterprojector.h"
#include "qgsapplication.h"

#define PROVIDER_KEY QStringLiteral( "virtualraster" )
#define PROVIDER_DESCRIPTION QStringLiteral( "Virtual Raster data provider" )

QgsVirtualRasterProvider::QgsVirtualRasterProvider( const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions )
  : QgsRasterDataProvider( uri, providerOptions )
{
  bool  ok;
  QgsRasterDataProvider::VirtualRasterParameters decodedUriParams = QgsRasterDataProvider::decodeVirtualRasterProviderUri( uri, & ok );

  if ( ! ok )
  {
    mValid = false;
    return;
  }

  if ( ! decodedUriParams.crs.isValid() )
  {
    QgsDebugError( "crs is not valid" );
    mValid = false;
    return;
  }
  mCrs = decodedUriParams.crs;

  if ( decodedUriParams.extent.isNull() )
  {
    QgsDebugError( "extent is null" );
    mValid = false;
    return;
  }
  mExtent = decodedUriParams.extent;

  mWidth = decodedUriParams.width;
  mHeight = decodedUriParams.height;

  mFormulaString = decodedUriParams.formula;

  mLastError.clear();
  mCalcNode.reset( QgsRasterCalcNode::parseRasterCalcString( mFormulaString, mLastError ) );

  if ( !mCalcNode )
  {
    mValid = false;
    return;
  }

  QStringList rLayerDict = mCalcNode->referencedLayerNames();
  QStringList rasterRefs = mCalcNode->cleanRasterReferences();

  QList<VirtualRasterInputLayers>::iterator it;
  for ( it = decodedUriParams.rInputLayers.begin(); it != decodedUriParams.rInputLayers.end(); ++it )
  {

    if ( ! rLayerDict.contains( it->name ) )
    {
      mValid = false;
      return;
    }

    QgsRasterLayer *rProvidedLayer = new QgsRasterLayer( it->uri, it->name, it->provider );

    if ( ! rProvidedLayer->isValid() )
    {
      mValid = false;
      return;
    }

    if ( mRasterLayers.contains( rProvidedLayer ) )
    {
      continue;
    }

    //this var is not useful right now except for the copy constructor
    mRasterLayers << rProvidedLayer;

    for ( int j = 0; j < rProvidedLayer->bandCount(); ++j )
    {
      if ( ! rasterRefs.contains( rProvidedLayer->name() + QStringLiteral( "@" ) + QString::number( j + 1 ) ) )
      {
        continue;
      }

      QgsRasterCalculatorEntry entry;
      entry.raster = rProvidedLayer;
      entry.bandNumber = j + 1;
      entry.ref = rProvidedLayer->name() + QStringLiteral( "@" ) + QString::number( j + 1 );
      mRasterEntries.push_back( entry );
    }
  }
  mValid = true;
}

QgsVirtualRasterProvider::QgsVirtualRasterProvider( const QgsVirtualRasterProvider &other )
  : QgsRasterDataProvider( other.dataSourceUri(), QgsDataProvider::ProviderOptions() )
  , mValid( other.mValid )
  , mCrs( other.mCrs )
  , mExtent( other.mExtent )
  , mWidth( other.mWidth )
  , mHeight( other.mHeight )
  , mBandCount( other.mBandCount )
  , mXBlockSize( other.mXBlockSize )
  , mYBlockSize( other.mYBlockSize )
  , mFormulaString( other.mFormulaString )
  , mLastError( other.mLastError )
  , mRasterLayers{} // see note in other constructor above

{
  for ( const auto &it : other.mRasterLayers )
  {
    QgsRasterLayer *rcProvidedLayer = it->clone();
    for ( int j = 0; j < rcProvidedLayer->bandCount(); ++j )
    {
      QgsRasterCalculatorEntry entry;
      entry.raster = rcProvidedLayer;
      entry.bandNumber = j + 1;
      entry.ref = rcProvidedLayer->name() % QStringLiteral( "@" ) + QString::number( j + 1 );
      mRasterEntries.push_back( entry );
    }
  }

  mCalcNode.reset( QgsRasterCalcNode::parseRasterCalcString( mFormulaString, mLastError ) );
}

QgsVirtualRasterProvider::~QgsVirtualRasterProvider()
{
  qDeleteAll( mRasterLayers );
}

QgsRasterBlock *QgsVirtualRasterProvider::block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback )
{
  Q_UNUSED( bandNo );
  std::unique_ptr< QgsRasterBlock > tblock = std::make_unique< QgsRasterBlock >( Qgis::DataType::Float64, width, height );
  double *outputData = ( double * )( tblock->bits() );

  QMap< QString, QgsRasterBlock * > inputBlocks;
  QVector<QgsRasterCalculatorEntry>::const_iterator it = mRasterEntries.constBegin();

  for ( ; it != mRasterEntries.constEnd(); ++it )
  {
    std::unique_ptr< QgsRasterBlock > block;

    if ( it->raster->crs() != mCrs )
    {
      QgsRasterProjector proj;
      proj.setCrs( it->raster->crs(), mCrs, it->raster->transformContext() );
      proj.setInput( it->raster->dataProvider() );
      proj.setPrecision( QgsRasterProjector::Exact );

      std::unique_ptr< QgsRasterBlockFeedback > rasterBlockFeedback( new QgsRasterBlockFeedback() );
      QObject::connect( feedback, &QgsFeedback::canceled, rasterBlockFeedback.get(), &QgsRasterBlockFeedback::cancel );
      block.reset( proj.block( it->bandNumber, extent, width, height, rasterBlockFeedback.get() ) );
      if ( rasterBlockFeedback->isCanceled() )
      {
        qDeleteAll( inputBlocks );
        QgsDebugMsgLevel( "Canceled = 3, User canceled calculation", 2 );
      }
    }
    else
    {
      block.reset( it->raster->dataProvider()->block( it->bandNumber, extent, width, height ) );
    }

    inputBlocks.insert( it->ref, block.release() );
  }

  QgsRasterMatrix resultMatrix( width, 1, nullptr, -FLT_MAX );

  for ( int i = 0; i < height; ++i )
  {
    if ( feedback )
    {
      feedback->setProgress( 100.0 * static_cast< double >( i ) / height );
    }

    if ( feedback && feedback->isCanceled() )
    {
      break;
    }

    if ( mCalcNode->calculate( inputBlocks, resultMatrix, i ) )
    {
      for ( int j = 0; j < width; ++j )
      {
        outputData [ i * width + j ] = resultMatrix.data()[j];
      }
    }
    else
    {
      qDeleteAll( inputBlocks );
      inputBlocks.clear();
      QgsDebugError( "calcNode was not run in a correct way" );
    }
  }

  Q_ASSERT( tblock );
  return tblock.release();
}

Qgis::DataProviderFlags QgsVirtualRasterProvider::flags() const
{
  return Qgis::DataProviderFlag::FastExtent2D;
}

QgsRectangle QgsVirtualRasterProvider::extent() const
{
  return mExtent;
}

int QgsVirtualRasterProvider::xSize() const
{
  return mWidth;
}

int QgsVirtualRasterProvider::ySize() const
{
  return mHeight;
}


int QgsVirtualRasterProvider::bandCount() const
{
  return mBandCount;
}

QString QgsVirtualRasterProvider::name() const
{
  return PROVIDER_KEY;
}

QString QgsVirtualRasterProvider::description() const
{
  return PROVIDER_DESCRIPTION;
}

int QgsVirtualRasterProvider::xBlockSize() const
{
  return mXBlockSize;
}

int QgsVirtualRasterProvider::yBlockSize() const
{
  return mYBlockSize;
}

QgsVirtualRasterProviderMetadata::QgsVirtualRasterProviderMetadata()
  : QgsProviderMetadata( PROVIDER_KEY, PROVIDER_DESCRIPTION )
{

}

QIcon QgsVirtualRasterProviderMetadata::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconRaster.svg" ) );
}

QgsVirtualRasterProvider *QgsVirtualRasterProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags )
{
  Q_UNUSED( flags );
  return new QgsVirtualRasterProvider( uri, options );
}

QString QgsVirtualRasterProviderMetadata::absoluteToRelativeUri( const QString &uri, const QgsReadWriteContext &context ) const
{
  QgsRasterDataProvider::VirtualRasterParameters decodedVirtualParams = QgsRasterDataProvider::decodeVirtualRasterProviderUri( uri );

  for ( auto &it : decodedVirtualParams.rInputLayers )
  {
    it.uri = context.pathResolver().writePath( it.uri );
  }
  return QgsRasterDataProvider::encodeVirtualRasterProviderUri( decodedVirtualParams ) ;
}

QString QgsVirtualRasterProviderMetadata::relativeToAbsoluteUri( const QString &uri, const QgsReadWriteContext &context ) const
{
  QgsRasterDataProvider::VirtualRasterParameters decodedVirtualParams = QgsRasterDataProvider::decodeVirtualRasterProviderUri( uri );

  for ( auto &it : decodedVirtualParams.rInputLayers )
  {
    it.uri = context.pathResolver().readPath( it.uri );
  }
  return QgsRasterDataProvider::encodeVirtualRasterProviderUri( decodedVirtualParams ) ;
}


QList<Qgis::LayerType> QgsVirtualRasterProviderMetadata::supportedLayerTypes() const
{
  return { Qgis::LayerType::Raster };
}

QgsVirtualRasterProvider *QgsVirtualRasterProvider::clone() const
{
  return new QgsVirtualRasterProvider( *this );
}

Qgis::DataType QgsVirtualRasterProvider::sourceDataType( int bandNo ) const
{
  Q_UNUSED( bandNo )
  return Qgis::DataType::Float64;
}

Qgis::DataType QgsVirtualRasterProvider::dataType( int bandNo ) const
{
  return sourceDataType( bandNo );
}

QgsCoordinateReferenceSystem QgsVirtualRasterProvider::crs() const
{
  return mCrs;
}

bool QgsVirtualRasterProvider::isValid() const
{
  return mValid;
}

QString QgsVirtualRasterProvider::lastErrorTitle()
{
  return QStringLiteral( "Not implemented" );
}

QString QgsVirtualRasterProvider::lastError()
{
  return QStringLiteral( "Not implemented" );
}

QString QgsVirtualRasterProvider::htmlMetadata() const
{
  //only test
  return "Virtual Raster data provider";
}

QString QgsVirtualRasterProvider::providerKey()
{
  return PROVIDER_KEY;
};

Qgis::RasterInterfaceCapabilities QgsVirtualRasterProvider::capabilities() const
{
  const Qgis::RasterInterfaceCapabilities capability = Qgis::RasterInterfaceCapability::Identify
      | Qgis::RasterInterfaceCapability::IdentifyValue
      | Qgis::RasterInterfaceCapability::Size
      //| Qgis::RasterInterfaceCapability::BuildPyramids
      | Qgis::RasterInterfaceCapability::Prefetch;
  return capability;
}

QString QgsVirtualRasterProvider::formulaString()
{
  return mFormulaString;
}

QGISEXTERN QgsVirtualRasterProviderMetadata *providerMetadataFactory()
{
  return new QgsVirtualRasterProviderMetadata();
}
