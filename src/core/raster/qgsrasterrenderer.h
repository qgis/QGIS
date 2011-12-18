#ifndef QGSRASTERRENDERER_H
#define QGSRASTERRENDERER_H

#include "qgsrasterdataprovider.h"

class QPainter;
class QgsMapToPixel;
class QgsRasterViewPort;

class QgsRasterRenderer
{
  public:
    QgsRasterRenderer( QgsRasterDataProvider* provider );
    virtual ~QgsRasterRenderer();
    virtual void draw( QPainter* p, QgsRasterViewPort* viewPort, const QgsMapToPixel* theQgsMapToPixel ) = 0;

  protected:
    inline double readValue( void *data, QgsRasterDataProvider::DataType type, int index );

    QgsRasterDataProvider* mProvider;
};

inline double QgsRasterRenderer::readValue( void *data, QgsRasterDataProvider::DataType type, int index )
{
  if( !mProvider )
  {
    return 0;
  }

  if( !data )
  {
    return mProvider->noDataValue();
  }

  switch ( type )
  {
    case QgsRasterDataProvider::Byte:
      return ( double )(( GByte * )data )[index];
      break;
    case QgsRasterDataProvider::UInt16:
      return ( double )(( GUInt16 * )data )[index];
      break;
    case QgsRasterDataProvider::Int16:
      return ( double )(( GInt16 * )data )[index];
      break;
    case QgsRasterDataProvider::UInt32:
      return ( double )(( GUInt32 * )data )[index];
      break;
    case QgsRasterDataProvider::Int32:
      return ( double )(( GInt32 * )data )[index];
      break;
    case QgsRasterDataProvider::Float32:
      return ( double )(( float * )data )[index];
      break;
    case QgsRasterDataProvider::Float64:
      return ( double )(( double * )data )[index];
      break;
    default:
      //QgsMessageLog::logMessage( tr( "GDAL data type %1 is not supported" ).arg( type ), tr( "Raster" ) );
      break;
  }

  return mProvider->noDataValue();
}

#endif // QGSRASTERRENDERER_H
