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
    std::unique_ptr< QgsRasterBlock > block = std::make_unique< QgsRasterBlock >( dataType( bandNo ), width, height );
}
*/


QgsRasterBlock *QgsVirtualRasterProvider::block( Qgis::DataType dataType, int width, int height  )
{

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


