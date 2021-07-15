#include "qgsvirtualrasterprovider.h"

#include "qgsrastercalculator.h"
#include "qgsrastercalcnode.h"
#include "qgsrastermatrix.h"
#include "qgsrasterlayer.h"

#include "qgsmessagelog.h"
#include "qgslogger.h"

#include "qgsrasterprojector.h"

#include <QUrl>
#include <QUrlQuery>
#define PROVIDER_KEY QStringLiteral( "virtualrasterprovider" )
#define PROVIDER_DESCRIPTION QStringLiteral( "Virtual Raster data provider" )

QgsVirtualRasterProvider::QgsVirtualRasterProvider( const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions )
  : QgsRasterDataProvider( uri, providerOptions )
{
  bool  ok = true;
  QgsRasterDataProvider::DecodedUriParameters decodedUriParams = QgsRasterDataProvider::decodeVirtualRasterProviderUri( uri, & ok );

  if (ok == false)
  {
    mValid = false;
    return;
  }

  if ( ! decodedUriParams.crs.isValid() )
  {
    QgsDebugMsg( "crs is not valid" );
    mValid = false;
    return;
  }
  mCrs = decodedUriParams.crs;

  if ( decodedUriParams.extent.isNull() )
  {
    QgsDebugMsg( "extent is null" );
    mValid = false;
    return;
  }
  mExtent = decodedUriParams.extent;

  mWidth = decodedUriParams.width;
  mHeight = decodedUriParams.height;

  if ( decodedUriParams.formula.isNull() )
  {
    QgsDebugMsg( "formula string provided is null" );
    mValid = false;
    return;
  }
  mFormulaString = decodedUriParams.formula;

  QStringList rasterRefs;
  mLastError.clear();
  mCalcNode.reset( QgsRasterCalcNode::parseRasterCalcString( mFormulaString, mLastError ) );

  if ( !mCalcNode )
  {
    mValid = false;
    return;
  }

  QList<const QgsRasterCalcNode *>::iterator i = mCalcNode->findNodes( QgsRasterCalcNode::Type::tRasterRef ).begin();
  for ( ; i != mCalcNode->findNodes( QgsRasterCalcNode::Type::tRasterRef ).end(); ++i )
  {
    QString r = ( *i )->toString();
    rasterRefs << r.mid( 1, r.size() - 2 );
  }

  QList<InputLayers>::iterator it;
  for ( it = decodedUriParams.rInputLayers.begin(); it != decodedUriParams.rInputLayers.end(); ++it )
  {
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
      if ( ! rasterRefs.contains( rProvidedLayer->name() % QStringLiteral( "@" ) % QString::number( j + 1 ) ) )
      {
        mValid = false;
        return;
      }

      QgsRasterCalculatorEntry entry;
      entry.raster = rProvidedLayer;
      entry.bandNumber = j + 1;
      entry.ref = rProvidedLayer->name() % QStringLiteral( "@" ) % QString::number( j + 1 );
      /*
      if ( ! uniqueRasterBandIdentifier( entry ) )
        break;
      */
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

{
  for ( const auto &it : other.mRasterLayers )
  {
    QgsRasterLayer *rcProvidedLayer = it->clone();
    for ( int j = 0; j < rcProvidedLayer->bandCount(); ++j )
    {
      QgsRasterCalculatorEntry entry;
      entry.raster = rcProvidedLayer;
      entry.bandNumber = j + 1;
      entry.ref = rcProvidedLayer->name() % QStringLiteral( "@" ) % QString::number( j + 1 );
      mRasterEntries.push_back( entry );
    }
  }

  mCalcNode.reset( QgsRasterCalcNode::parseRasterCalcString( mFormulaString, mLastError ) );
}

QgsVirtualRasterProvider::~QgsVirtualRasterProvider()
{
  for ( int i = 0; i < mRasterLayers.size(); ++i )
  {
    delete mRasterLayers[i];
  }
}

QString QgsVirtualRasterProvider::dataSourceUri( bool expandAuthConfig ) const
{
  Q_UNUSED( expandAuthConfig )
  return QgsDataProvider::dataSourceUri();
}

QgsRasterBlock *QgsVirtualRasterProvider::block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback )
{
  Q_UNUSED( bandNo );
  std::unique_ptr< QgsRasterBlock > tblock = std::make_unique< QgsRasterBlock >( Qgis::DataType::Float64, width, height );
  double *outputData = ( double * )( tblock->bits() );

  //else  // Original code (memory inefficient route)
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

      QgsRasterBlockFeedback *rasterBlockFeedback = new QgsRasterBlockFeedback();
      QObject::connect( feedback, &QgsFeedback::canceled, rasterBlockFeedback, &QgsRasterBlockFeedback::cancel );
      block.reset( proj.block( it->bandNumber, extent, width, height, rasterBlockFeedback ) );
      if ( rasterBlockFeedback->isCanceled() )
      {
        qDeleteAll( inputBlocks );
        QgsDebugMsg( "Canceled = 3, User canceled calculation" );
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
      QgsDebugMsg( "calcNode was not run in a correct way" );
    }
  }

  Q_ASSERT( tblock );
  return tblock.release();
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

QgsVirtualRasterProvider *QgsVirtualRasterProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags )
{
  Q_UNUSED( flags );
  return new QgsVirtualRasterProvider( uri, options );
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

QString QgsVirtualRasterProvider::htmlMetadata()
{
  //only test
  return "Virtual Raster data provider";
}

QString QgsVirtualRasterProvider::providerKey()
{
  return PROVIDER_KEY;
};

int QgsVirtualRasterProvider::capabilities() const
{
  const int capability = QgsRasterDataProvider::Identify
                         | QgsRasterDataProvider::IdentifyValue
                         | QgsRasterDataProvider::Size
                         //| QgsRasterDataProvider::BuildPyramids
                         | QgsRasterDataProvider::Create
                         | QgsRasterDataProvider::Remove
                         | QgsRasterDataProvider::Prefetch;
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
