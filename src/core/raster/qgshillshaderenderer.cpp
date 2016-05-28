#include <QColor>

#include "qgshillshaderenderer.h"

#include "qgsrasterinterface.h"
#include "qgsrasterblock.h"
#include "qgsrectangle.h"


QgsHillshadeRenderer::QgsHillshadeRenderer( QgsRasterInterface *input, int band, double lightAzimuth, double lightAngle ):
    QgsRasterRenderer( input, "hillshade" ),
    mBand( band ),
    mZFactor( 1 ),
    mLightAngle( lightAngle ),
    mLightAzimuth( lightAzimuth )
{

}

QgsHillshadeRenderer *QgsHillshadeRenderer::clone() const
{
  QgsHillshadeRenderer* r = new QgsHillshadeRenderer( nullptr, mBand, mLightAzimuth, mLightAngle );
  r->setZFactor( mZFactor );
  return r;
}

QgsRasterRenderer *QgsHillshadeRenderer::create( const QDomElement &elem, QgsRasterInterface *input )
{
  if ( elem.isNull() )
  {
    return nullptr;
  }

  int band = elem.attribute( "band", "0" ).toInt();
  double azimuth = elem.attribute( "azimuth", "315" ).toDouble();
  double angle = elem.attribute( "angle", "45" ).toDouble();
  double zFactor = elem.attribute( "zfactor", "1" ).toDouble();
  QgsHillshadeRenderer* r = new QgsHillshadeRenderer( input, band, azimuth , angle );
  r->setZFactor( zFactor );
  return r;
}

void QgsHillshadeRenderer::writeXML( QDomDocument &doc, QDomElement &parentElem ) const
{
  if ( parentElem.isNull() )
  {
    return;
  }

  QDomElement rasterRendererElem = doc.createElement( "rasterrenderer" );
  _writeXML( doc, rasterRendererElem );

  rasterRendererElem.setAttribute( "band", mBand );
  rasterRendererElem.setAttribute( "azimuth", QString::number( mLightAzimuth ) );
  rasterRendererElem.setAttribute( "angle", QString::number( mLightAngle ) );
  rasterRendererElem.setAttribute( "zfactor", QString::number( mZFactor ) );
  parentElem.appendChild( rasterRendererElem );
}

QgsRasterBlock *QgsHillshadeRenderer::block( int bandNo, const QgsRectangle &extent, int width, int height )
{
  Q_UNUSED( bandNo );
  QgsRasterBlock *outputBlock = new QgsRasterBlock();
  if ( !mInput )
  {
    QgsDebugMsg( "No input raster!" );
    return outputBlock;
  }

  QgsRasterBlock *inputBlock = mInput->block( mBand, extent, width, height );

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

  double cellxsize = extent.width() / double( width );
  double cellysize = extent.height() / double( height );
  double zenith_rad = qMax( 0.0, 90 - mLightAngle ) * M_PI / 180.0;
  double azimuth_rad = -1 * mLightAzimuth * M_PI / 180.0;
  double aspect_rad = 0;
  double coszenith_rad = cos( zenith_rad );
  double sinzenith_rad = cos( zenith_rad );

  QRgb myDefaultColor = NODATA_COLOR;

  for ( qgssize i = 0; i < ( qgssize )height; i++ )
  {

    for ( qgssize j = 0; j < ( qgssize )width; j++ )
    {

      if ( inputBlock->isNoData( i, j ) )
      {
        outputBlock->setColor( i, j, myDefaultColor );
        continue;
      }

      if ( inputBlock->isNoData( i, j ) )
      {
        outputBlock->setColor( i, j, myDefaultColor );
        continue;
      }

      qgssize iUp, iDown, jLeft, jRight;
      if ( i == 0 )
      {
        iUp = i;
        iDown = i + 1;
      }
      else if ( i < ( qgssize )height - 1 )
      {
        iUp = i - 1;
        iDown = i + 1;
      }
      else
      {
        iUp = i - 1;
        iDown = i;
      }

      if ( j == 0 )
      {
        jLeft = j;
        jRight = j + 1;
      }
      else if ( j < ( qgssize )width - 1 )
      {
        jLeft = j - 1;
        jRight = j + 1;
      }
      else
      {
        jLeft = j - 1;
        jRight = j;
      }

      double x11;
      double x21;
      double x31;
      double x12;
      double x22; // Working cell
      double x32;
      double x13;
      double x23;
      double x33;

      // This is center cell. It is not nodata. Use this in place of nodata neighbors
      x22 = inputBlock->value( i, j );

      x11 = inputBlock->isNoData( iUp, jLeft )  ? x22 : inputBlock->value( iUp, jLeft );
      x21 = inputBlock->isNoData( i, jLeft )     ? x22 : inputBlock->value( i, jLeft );
      x31 = inputBlock->isNoData( iDown, jLeft ) ? x22 : inputBlock->value( iDown, jLeft );

      x12 = inputBlock->isNoData( iUp, j )       ? x22 : inputBlock->value( iUp, j );
      // x22
      x32 = inputBlock->isNoData( iDown, j )     ? x22 : inputBlock->value( iDown, j );

      x13 = inputBlock->isNoData( iUp, jRight )   ? x22 : inputBlock->value( iUp, jRight );
      x23 = inputBlock->isNoData( i, jRight )     ? x22 : inputBlock->value( i, jRight );
      x33 = inputBlock->isNoData( iDown, jRight ) ? x22 : inputBlock->value( iDown, jRight );

      double derX = calcFirstDerX( x11, x21, x31, x12, x22, x32, x13, x23, x33, cellxsize );
      double derY = calcFirstDerY( x11, x21, x31, x12, x22, x32, x13, x23, x33, cellysize );

      double slope_rad = atan( mZFactor * sqrt( derX * derX + derY * derY ) );

      if ( derX != 0 )
      {
        aspect_rad = atan2( derX, -derY );
        if ( aspect_rad < 0 )
        {
          aspect_rad = 2 * M_PI + aspect_rad;
        }
      }
      else if ( derX == 0 )
      {
        if ( derY > 0 )
        {
          aspect_rad = M_PI / 2;
        }
        else if ( derY < 0 )
        {
          aspect_rad = 2 * M_PI - M_PI / 2;
        }
        else
        {
          aspect_rad = aspect_rad;
        }
      }


      double colorvalue = qMax( 0.0, 255.0 * (( coszenith_rad * cos( slope_rad ) ) +
                                              ( sinzenith_rad * sin( slope_rad ) * cos( azimuth_rad - aspect_rad ) ) ) );

      outputBlock->setColor( i, j, qRgb( colorvalue, colorvalue, colorvalue ) );
    }
  }
  return outputBlock;
}

QList<int> QgsHillshadeRenderer::usesBands() const
{
  QList<int> bandList;
  if ( mBand != -1 )
  {
    bandList << mBand;
  }
  return bandList;

}

void QgsHillshadeRenderer::setBand( int bandNo )
{
  if ( bandNo > mInput->bandCount() || bandNo <= 0 )
  {
    return;
  }
  mBand = bandNo;
}

double QgsHillshadeRenderer::calcFirstDerX( double x11, double x21, double x31, double x12, double x22, double x32, double x13, double x23, double x33, double cellsize )
{
  Q_UNUSED( x12 );
  Q_UNUSED( x22 );
  Q_UNUSED( x32 );
  return (( x13 + x23 + x23 + x33 ) - ( x11 + x21 + x21 + x31 ) ) / ( 8 * cellsize );
}

double QgsHillshadeRenderer::calcFirstDerY( double x11, double x21, double x31, double x12, double x22, double x32, double x13, double x23, double x33, double cellsize )
{
  Q_UNUSED( x22 );
  Q_UNUSED( x32 );
  Q_UNUSED( x21 );
  Q_UNUSED( x23 );
  return (( x31 + x32 + x32 + x33 ) - ( x11 + x12 + x12 + x13 ) ) / ( 8 * -cellsize );
}




