#include "qgsvirtualrasterprovider.h"

#include "qgsmessagelog.h"
#include <QImage>

const QString QgsVirtualRasterProvider::VR_RASTER_PROVIDER_KEY = QStringLiteral( "virtualrasterprovider" );
const QString QgsVirtualRasterProvider::VR_RASTER_PROVIDER_DESCRIPTION =  QStringLiteral( "Virtual Raster data provider" );

QgsVirtualRasterProvider::QgsVirtualRasterProvider( const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions, QgsDataProvider::ReadFlags flags )
    : QgsRasterDataProvider( uri, providerOptions, flags )
{

}

QgsVirtualRasterProvider::QgsVirtualRasterProvider( const QgsVirtualRasterProvider &other, const QgsDataProvider::ProviderOptions &providerOptions, QgsDataProvider::ReadFlags flags )
  : QgsRasterDataProvider( other.dataSourceUri(), providerOptions, flags )
{

}

QgsCoordinateReferenceSystem QgsVirtualRasterProvider::crs() const
{
  return mCrs;
}

QgsRectangle QgsVirtualRasterProvider::extent() const
{
  return mExtent;
}

bool QgsVirtualRasterProvider::isValid() const
{
  return mValid;
}

QString QgsVirtualRasterProvider::name() const
{
  return QgsVirtualRasterProvider::VR_RASTER_PROVIDER_KEY;
}

QString QgsVirtualRasterProvider::description() const
{
  return QgsVirtualRasterProvider::VR_RASTER_PROVIDER_DESCRIPTION;
}


bool QgsVirtualRasterProvider::readBlock( int bandNo, const QgsRectangle &viewExtent, int width, int height, void *data, QgsRasterBlockFeedback * )
{
    return true;
}

///*
//QImage *QgsVirtualRasterProvider::draw( QgsRectangle const &viewExtent, int pixelWidth, int pixelHeight, QgsRasterBlockFeedback *feedback )
//QImage *QgsVirtualRasterProvider::draw( QgsRectangle const &viewExtent, int width, int height, QgsRasterBlockFeedback *)
QImage *QgsVirtualRasterProvider::draw( int width, int height)
{
  //QImage *image = new QImage( pixelWidth, pixelHeight, QImage::Format_ARGB32 );
  QImage *image = new QImage( width, height, QImage::Format_ARGB32 );
  image->fill( 0 );

  return image;
}
//*/


QgsVirtualRasterProviderMetadata::QgsVirtualRasterProviderMetadata()
  : QgsProviderMetadata( QgsVirtualRasterProvider::VR_RASTER_PROVIDER_KEY, QgsVirtualRasterProvider::VR_RASTER_PROVIDER_DESCRIPTION )
{

}


QgsVirtualRasterProvider *QgsVirtualRasterProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags )

{
  return new QgsVirtualRasterProvider( uri, options, flags );

}


Qgis::DataType QgsVirtualRasterProvider::dataType( int bandNo ) const
{
  if ( mDataTypes.size() < static_cast<unsigned long>( bandNo ) )
  {
    QgsMessageLog::logMessage( tr( "Data type size for band %1 could not be found: num bands is: %2 and the type size map for bands contains: %3 items" )
                               .arg( bandNo )
                               .arg( mBandCount )
                               .arg( mDataSizes.size() ),
                               QStringLiteral( "Virtual Raster Provider" ), Qgis::Warning );
    return Qgis::DataType::UnknownDataType;
  }
  // Band is 1-based
  return mDataTypes[ static_cast<unsigned long>( bandNo ) - 1 ];
}

int QgsVirtualRasterProvider::bandCount() const
{
  return mBandCount;
}

///*
QgsVirtualRasterProvider *QgsVirtualRasterProvider::clone() const
{
  QgsDataProvider::ProviderOptions options;
  options.transformContext = transformContext();
  QgsVirtualRasterProvider *provider = new QgsVirtualRasterProvider( *this, options );
  provider->copyBaseSettings( *this );
  return provider;
}
//*/


QString QgsVirtualRasterProvider::htmlMetadata()
{
  /*
  // This must return the content of a HTML table starting by tr and ending by tr
  QVariantMap overviews;
  for ( auto it = mOverViews.constBegin(); it != mOverViews.constEnd(); ++it )
  {
    overviews.insert( QString::number( it.key() ), it.value() );
  }

  const QVariantMap additionalInformation
  {
    { tr( "Is Tiled" ), mIsTiled },
    //{ tr( "Where Clause SQL" ), subsetString() },
    { tr( "Pixel Size" ), QStringLiteral( "%1, %2" ).arg( mScaleX ).arg( mScaleY ) },
    { tr( "Overviews" ),  overviews },
    //{ tr( "Primary Keys SQL" ),  pkSql() },
    //{ tr( "Temporal Column" ),  mTemporalFieldIndex >= 0 && mAttributeFields.exists( mTemporalFieldIndex ) ?  mAttributeFields.field( mTemporalFieldIndex ).name() : QString() },
  };
  return  dumpVariantMap( additionalInformation, tr( "Additional information" ) );
  */
  //only test
  return "Virtual Raster data provider";
}

QString QgsVirtualRasterProvider::lastErrorTitle()
{
  return mErrorTitle;
}

QString QgsVirtualRasterProvider::lastError()
{
  return mError;
}

int QgsVirtualRasterProvider::capabilities() const
{
  const int capability = QgsRasterDataProvider::Identify
                         | QgsRasterDataProvider::IdentifyValue
                         | QgsRasterDataProvider::Size
                         // TODO:| QgsRasterDataProvider::BuildPyramids
                         | QgsRasterDataProvider::Create
                         | QgsRasterDataProvider::Remove
                         | QgsRasterDataProvider::Prefetch;
  return capability;
}


int QgsVirtualRasterProvider::xSize() const
{
  return static_cast<int>( mWidth );
}

int QgsVirtualRasterProvider::ySize() const
{
  return static_cast<int>( mHeight );
}

Qgis::DataType QgsVirtualRasterProvider::sourceDataType( int bandNo ) const
{
  if ( bandNo <= mBandCount && static_cast<unsigned long>( bandNo ) <= mDataTypes.size() )
  {
    return mDataTypes[ static_cast<unsigned long>( bandNo - 1 ) ];
  }
  else
  {
    QgsMessageLog::logMessage( tr( "Data type is unknown" ), QStringLiteral( "PostGIS" ), Qgis::Warning );
    return Qgis::DataType::UnknownDataType;
  }
}

int QgsVirtualRasterProvider::xBlockSize() const
{
  if ( mInput )
  {
    return mInput->xBlockSize();
  }
  else
  {
    return static_cast<int>( mWidth );
  }
}

int QgsVirtualRasterProvider::yBlockSize() const
{
  if ( mInput )
  {
    return mInput->yBlockSize();
  }
  else
  {
    return static_cast<int>( mHeight );
  }
}
