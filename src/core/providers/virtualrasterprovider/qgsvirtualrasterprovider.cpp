#include "qgsvirtualrasterprovider.h"

#include "qgsmessagelog.h"
#include <QImage>

const QString QgsVirtualRasterProvider::VR_RASTER_PROVIDER_KEY = QStringLiteral( "virtualrasterprovider" );
const QString QgsVirtualRasterProvider::VR_RASTER_PROVIDER_DESCRIPTION =  QStringLiteral( "Virtual Raster data provider" );

QgsVirtualRasterProvider::QgsVirtualRasterProvider( const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions)
    : QgsRasterDataProvider( uri, providerOptions)
{

}

QgsVirtualRasterProvider::QgsVirtualRasterProvider(const QgsVirtualRasterProvider &other)
    : QgsRasterDataProvider (other.dataSourceUri(), QgsDataProvider::ProviderOptions() )
{
    mExtent = other.mExtent;
    mWidth = other.mWidth;
    mHeight = other.mHeight;
    mBandCount = other.mBandCount;
    mXBlockSize = other.mXBlockSize;
    mYBlockSize = other.mYBlockSize;

}


/*
//QgsRasterBlock *QgsVirtualRasterProvider::block( Qgis::DataType dataType, int width, int height )
QgsRasterBlock *QgsVirtualRasterProvider::block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback )
{
    //QgsRasterBlock *tblock = new QgsRasterBlock( dataType( bandNo ), width, height );
    QgsRasterBlock *tblock = new QgsRasterBlock( );


    unsigned int* outputData = ( unsigned int* )( tblock->bits() );
    //qgssize rasterSize = ( qgssize )width * height;

    for ( int i = 0; i < width * height; ++i )
    {
        outputData[i] = 42;
    }
    return tblock;
}

*/


QgsRasterBlock *QgsVirtualRasterProvider::block( Qgis::DataType dataType, int width, int height  )
{
    QgsRasterBlock *tblock = new QgsRasterBlock( dataType, width, height );
    //QgsRasterBlock *tblock = new QgsRasterBlock( );


    unsigned int* outputData = ( unsigned int* )( tblock->bits() );
    //qgssize rasterSize = ( qgssize )width * height;

    for ( int i = 0; i < width * height; ++i )
    {
        outputData[i] = 42;
    }
    return tblock;
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
  return QgsVirtualRasterProvider::VR_RASTER_PROVIDER_KEY;
}

QString QgsVirtualRasterProvider::description() const
{
  return QgsVirtualRasterProvider::VR_RASTER_PROVIDER_DESCRIPTION;
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
  : QgsProviderMetadata( QgsVirtualRasterProvider::VR_RASTER_PROVIDER_KEY, QgsVirtualRasterProvider::VR_RASTER_PROVIDER_DESCRIPTION )
{

}

QgsVirtualRasterProvider *QgsVirtualRasterProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options)//, QgsDataProvider::ReadFlags flags )

{
  return new QgsVirtualRasterProvider( uri, options);//, flags );

}
