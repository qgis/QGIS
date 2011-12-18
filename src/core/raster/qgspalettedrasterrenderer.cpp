#include "qgspalettedrasterrenderer.h"
#include "qgsrasterviewport.h"
#include <QColor>
#include <QImage>
#include <QPainter>

QgsPalettedRasterRenderer::QgsPalettedRasterRenderer( QgsRasterDataProvider* provider, int bandNumber, QColor* colorArray, int nColors ):
  QgsRasterRenderer( provider ), mBandNumber( bandNumber), mColors( colorArray ), mNColors( nColors )
{
}

QgsPalettedRasterRenderer::~QgsPalettedRasterRenderer()
{
  delete[] mColors;
}

void QgsPalettedRasterRenderer::draw( QPainter* p, QgsRasterViewPort* viewPort, const QgsMapToPixel* theQgsMapToPixel )
{
  if( !p || !mProvider || !viewPort || !theQgsMapToPixel )
  {
    return;
  }

  //read data from provider
  int typeSize = mProvider->dataTypeSize( mBandNumber ) / 8;
  QgsRasterDataProvider::DataType rasterType = (QgsRasterDataProvider::DataType)mProvider->dataType( mBandNumber );
  void* rasterData = VSIMalloc( typeSize * viewPort->drawableAreaXDim *  viewPort->drawableAreaYDim);
  mProvider->readBlock( mBandNumber, viewPort->mDrawnExtent, viewPort->drawableAreaXDim, viewPort->drawableAreaYDim,
             viewPort->mSrcCRS, viewPort->mDestCRS, rasterData );
  int currentRasterPos = 0;

  //raster image
  QImage img( viewPort->drawableAreaXDim, viewPort->drawableAreaYDim, QImage::Format_ARGB32_Premultiplied );
  QRgb* imageScanLine = 0;

  for( int i = 0; i < viewPort->drawableAreaYDim; ++i )
  {
    imageScanLine = ( QRgb* )( img.scanLine( i ) );
    for( int j = 0; j < viewPort->drawableAreaXDim; ++j )
    {
      int val = readValue( rasterData, rasterType, currentRasterPos );
      imageScanLine[j] = mColors[ val ].rgba();
      ++currentRasterPos;
    }
  }
  CPLFree( rasterData );
  p->drawImage( QPointF( viewPort->topLeftPoint.x(), viewPort->topLeftPoint.y() ), img );
}
