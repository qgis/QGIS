#include "qgsvirtualrasterprovider.h"

#include "qgsmessagelog.h"
//#include <QImage>
#include <QPainter>

#include "qgslogger.h"

#define PROVIDER_KEY QStringLiteral( "virtualrasterprovider" )
#define PROVIDER_DESCRIPTION QStringLiteral( "Virtual Raster data provider" )

//const QString QgsVirtualRasterProvider::VR_RASTER_PROVIDER_KEY = QStringLiteral( "virtualrasterprovider" );
//const QString QgsVirtualRasterProvider::VR_RASTER_PROVIDER_DESCRIPTION =  QStringLiteral( "Virtual Raster data provider" );

QgsVirtualRasterProvider::QgsVirtualRasterProvider( const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions)
    : QgsRasterDataProvider( uri, providerOptions)
{
    QgsDebugMsg("QgsVirtualRasterProvider was called constructor");
    //mUri = uri;
    bool check = true;
    if (check){
        mValid = true;
    } else {
        mValid = false;
    }

}


QgsVirtualRasterProvider::QgsVirtualRasterProvider(const QgsVirtualRasterProvider &other)
    : QgsRasterDataProvider (other.dataSourceUri(), QgsDataProvider::ProviderOptions() )

    /*
    , mValid( other.mValid )
    , mCrs( other.mCrs )
    , mDataTypes( other.mDataTypes )
    //
    , mExtent( other.mExtent )
    , mWidth( other.mWidth )
    , mHeight( other.mHeight )
    , mBandCount( other.mBandCount )
    , mXBlockSize( other.mXBlockSize )
    , mYBlockSize( other.mYBlockSize )
    */
{
    mValid = other.mValid;
    mCrs = other.mCrs;
    mDataTypes = other.mDataTypes;
    mExtent = other.mExtent;
    mWidth = other.mWidth;
    mHeight = other.mHeight;
    mBandCount = other.mBandCount;
    mXBlockSize = other.mXBlockSize;
    mYBlockSize = other.mYBlockSize;

}


QString QgsVirtualRasterProvider::dataSourceUri( bool expandAuthConfig ) const
{
    Q_UNUSED( expandAuthConfig )
    return QgsDataProvider::dataSourceUri();
}

/*
QImage *QgsVirtualRasterProvider::draw( QgsRectangle const &extent, int width, int height, QgsRasterBlockFeedback *feedback )
{
    QImage *image = new QImage( width, height, QImage::Format_ARGB32 );
    const QImage &nimage = QImage( 5, 5, QImage::Format_ARGB32 );
    image->fill( 0 );
    QPoint pt = QPoint(2, 5);
    QPainter p( image );
    p.drawImage(pt,  nimage);
    QgsDebugMsg("hello draw");

    return image;
}

bool QgsVirtualRasterProvider::readBlock( int bandNo, QgsRectangle  const &extent, int width, int height, void *block, QgsRasterBlockFeedback *feedback )
{
    Q_UNUSED( bandNo )
    // TODO: optimize to avoid writing to QImage
    std::unique_ptr< QImage > image( draw( mExtent, mWidth, mHeight, feedback ) );
    QgsDebugMsg("hello readblock");

    return true;
}
*/


//qgiscrash

QgsRasterBlock *QgsVirtualRasterProvider::block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback )
{
    //QgsRasterBlock *block = new QgsRasterBlock( dataType( bandNo ), width, height );
    QgsRasterBlock *block = new QgsRasterBlock( Qgis::DataType::UInt32, width, height );

    QgsDebugMsg("QgsVirtualRasterProvider::block method was called");

    unsigned int* outputData = ( unsigned int* )( block->bits() );

    for ( int i = 0; i < width * height; ++i )
    {
        //QgsDebugMsg("inside for loop");
        outputData[i] = 42;


    }
    Q_ASSERT( block );

    return block;
}




/*
//it works
QgsRasterBlock *QgsVirtualRasterProvider::block( Qgis::DataType dataType, int width, int height  )
{
    QgsRasterBlock *block = new QgsRasterBlock( dataType, width, height );
    unsigned int* outputData = ( unsigned int* )( block->bits() );
    QgsDebugMsg("hello from block method 2");
    for ( int i = 0; i < width * height; ++i )
    {
        outputData[i] = 42;
    }
    return block;
}
*/


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
  //return QgsVirtualRasterProvider::VR_RASTER_PROVIDER_KEY;
  return PROVIDER_KEY;
}

QString QgsVirtualRasterProvider::description() const
{
  //return QgsVirtualRasterProvider::VR_RASTER_PROVIDER_DESCRIPTION;
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
  //: QgsProviderMetadata( QgsVirtualRasterProvider::VR_RASTER_PROVIDER_KEY, QgsVirtualRasterProvider::VR_RASTER_PROVIDER_DESCRIPTION )
    : QgsProviderMetadata( PROVIDER_KEY, PROVIDER_DESCRIPTION )
{

}


//QgsVirtualRasterProvider *QgsVirtualRasterProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options)//, QgsDataProvider::ReadFlags flags )
QgsVirtualRasterProvider *QgsVirtualRasterProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags )
{
    Q_UNUSED( flags );
    return new QgsVirtualRasterProvider( uri, options);//, flags );
}




QgsVirtualRasterProvider *QgsVirtualRasterProvider::clone() const
{
/*
  QgsDataProvider::ProviderOptions options;
  options.transformContext = transformContext();
  QgsVirtualRasterProvider *provider = new QgsVirtualRasterProvider( *this, options );
  provider->copyBaseSettings( *this );
  return provider;
*/
    return new QgsVirtualRasterProvider( *this );
}


//__________________________________________________________________________________________________________________________________________________________

Qgis::DataType QgsVirtualRasterProvider::sourceDataType( int bandNo ) const
{
  if ( bandNo <= mBandCount && static_cast<unsigned long>( bandNo ) <= mDataTypes.size() )
  {
    return mDataTypes[ static_cast<unsigned long>( bandNo - 1 ) ];
  }
  else
  {
    QgsMessageLog::logMessage( tr( "Data type is unknown" ), QStringLiteral( "VirtualRasterProvider" ), Qgis::Warning );
    return Qgis::DataType::UnknownDataType;
  }
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
//-----------------------------create raster data prov
/*
QgsVirtualRasterProvider *createRasterDataProvider(
      const QString &uri,
      const QString &format,
      int nBands,
      Qgis::DataType type,
      int width,
      int height,
      double *geoTransform,
      const QgsCoordinateReferenceSystem &crs,
      const QStringList &createOptions )
{

}
*/

QString QgsVirtualRasterProvider::providerKey()
{
  return PROVIDER_KEY;
};

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
