/***************************************************************************
                         qgshillshaderenderer.cpp
                         ---------------------------------
    begin                : May 2016
    copyright            : (C) 2016 by Nathan Woodrow
    email                : woodrow dot nathan at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QColor>

#include "qgshillshaderenderer.h"
#include "qgsrastertransparency.h"
#include "qgsrasterinterface.h"
#include "qgsrasterblock.h"
#include "qgsrectangle.h"
#include "qgssettings.h"
#include <memory>

#ifdef QGISDEBUG
#include "qgsmessagelog.h"
#include <chrono>
#endif

QgsHillshadeRenderer::QgsHillshadeRenderer( QgsRasterInterface *input, int band, double lightAzimuth, double lightAngle ):
  QgsRasterRenderer( input, QStringLiteral( "hillshade" ) )
  , mBand( band )
  , mZFactor( 1 )
  , mLightAngle( lightAngle )
  , mLightAzimuth( lightAzimuth )
  , mMultiDirectional( false )
{

}

QgsHillshadeRenderer *QgsHillshadeRenderer::clone() const
{
  QgsHillshadeRenderer *r = new QgsHillshadeRenderer( nullptr, mBand, mLightAzimuth, mLightAngle );
  r->copyCommonProperties( this );

  r->setZFactor( mZFactor );
  r->setMultiDirectional( mMultiDirectional );
  return r;
}

QgsRasterRenderer *QgsHillshadeRenderer::create( const QDomElement &elem, QgsRasterInterface *input )
{
  if ( elem.isNull() )
  {
    return nullptr;
  }

  int band = elem.attribute( QStringLiteral( "band" ), QStringLiteral( "0" ) ).toInt();
  double azimuth = elem.attribute( QStringLiteral( "azimuth" ), QStringLiteral( "315" ) ).toDouble();
  double angle = elem.attribute( QStringLiteral( "angle" ), QStringLiteral( "45" ) ).toDouble();
  double zFactor = elem.attribute( QStringLiteral( "zfactor" ), QStringLiteral( "1" ) ).toDouble();
  bool multiDirectional = elem.attribute( QStringLiteral( "multidirection" ), QStringLiteral( "0" ) ).toInt();
  QgsHillshadeRenderer *r = new QgsHillshadeRenderer( input, band, azimuth, angle );
  r->readXml( elem );

  r->setZFactor( zFactor );
  r->setMultiDirectional( multiDirectional );
  return r;
}

void QgsHillshadeRenderer::writeXml( QDomDocument &doc, QDomElement &parentElem ) const
{
  if ( parentElem.isNull() )
  {
    return;
  }

  QDomElement rasterRendererElem = doc.createElement( QStringLiteral( "rasterrenderer" ) );
  _writeXml( doc, rasterRendererElem );

  rasterRendererElem.setAttribute( QStringLiteral( "band" ), mBand );
  rasterRendererElem.setAttribute( QStringLiteral( "azimuth" ), QString::number( mLightAzimuth ) );
  rasterRendererElem.setAttribute( QStringLiteral( "angle" ), QString::number( mLightAngle ) );
  rasterRendererElem.setAttribute( QStringLiteral( "zfactor" ), QString::number( mZFactor ) );
  rasterRendererElem.setAttribute( QStringLiteral( "multidirection" ), QString::number( mMultiDirectional ) );
  parentElem.appendChild( rasterRendererElem );
}

QgsRasterBlock *QgsHillshadeRenderer::block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback )
{
  Q_UNUSED( bandNo );
  std::unique_ptr< QgsRasterBlock > outputBlock( new QgsRasterBlock() );
  if ( !mInput )
  {
    QgsDebugMsg( "No input raster!" );
    return outputBlock.release();
  }

  std::shared_ptr< QgsRasterBlock > inputBlock( mInput->block( mBand, extent, width, height, feedback ) );

  if ( !inputBlock || inputBlock->isEmpty() )
  {
    QgsDebugMsg( "No raster data!" );
    return outputBlock.release();
  }

  std::shared_ptr< QgsRasterBlock > alphaBlock;

  if ( mAlphaBand > 0 && mBand != mAlphaBand )
  {
    alphaBlock.reset( mInput->block( mAlphaBand, extent, width, height, feedback ) );
    if ( !alphaBlock || alphaBlock->isEmpty() )
    {
      // TODO: better to render without alpha
      return outputBlock.release();
    }
  }
  else if ( mAlphaBand > 0 )
  {
    alphaBlock = inputBlock;
  }

  if ( !outputBlock->reset( Qgis::ARGB32_Premultiplied, width, height ) )
  {
    return outputBlock.release();
  }

  float cellXSize = extent.width() / static_cast<float>( width );
  float cellYSize = extent.height() / static_cast<float>( height );
  float zenithRad = std::max( 0.0, 90 - mLightAngle ) * M_PI / 180.0;
  float azimuthRad = -1 * mLightAzimuth * M_PI / 180.0;
  float cosZenithRad = std::cos( zenithRad );
  float sinZenithRad = std::sin( zenithRad );

  // For fast formula from GDAL DEM
  float cos_alt_mul_z = cosZenithRad * mZFactor;
  float cos_az_mul_cos_alt_mul_z = std::cos( azimuthRad ) * cos_alt_mul_z;
  float sin_az_mul_cos_alt_mul_z = std::sin( azimuthRad ) * cos_alt_mul_z;
  float cos_az_mul_cos_alt_mul_z_mul_254 = 254.0 * cos_az_mul_cos_alt_mul_z;
  float sin_az_mul_cos_alt_mul_z_mul_254 = 254.0 * sin_az_mul_cos_alt_mul_z;
  float square_z = mZFactor * mZFactor;
  float sinZenithRad_mul_254 = 254.0 * sinZenithRad;

  // For multi directional
  float sinZenithRad_mul_127 = 127.0 * sinZenithRad;
  // 127.0 * std::cos(225.0 *  M_PI / 180.0) = -32.87001872802012
  float cos225_az_mul_cos_alt_mul_z_mul_127 = -32.87001872802012f * cos_alt_mul_z;
  float cos_alt_mul_z_mul_127 = 127.0 * cos_alt_mul_z;

  QRgb myDefaultColor = NODATA_COLOR;

#ifdef QGISDEBUG
  std::chrono::time_point<std::chrono::system_clock> startTime( std::chrono::system_clock::now() );
#endif

  for ( qgssize i = 0; i < ( qgssize )height; i++ )
  {

    for ( qgssize j = 0; j < ( qgssize )width; j++ )
    {

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

      float x11;
      float x21;
      float x31;
      float x12;
      float x22; // Working cell
      float x32;
      float x13;
      float x23;
      float x33;

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

      float derX = calcFirstDerX( x11, x21, x31, x12, x22, x32, x13, x23, x33, cellXSize );
      float derY = calcFirstDerY( x11, x21, x31, x12, x22, x32, x13, x23, x33, cellYSize );

      float grayValue;
      if ( !mMultiDirectional )
      {
        // Standard single direction hillshade
        // Fast formula from GDAL DEM
        grayValue = qBound( 0.0f, ( sinZenithRad_mul_254 -
                                    ( derY * cos_az_mul_cos_alt_mul_z_mul_254 -
                                      derX * sin_az_mul_cos_alt_mul_z_mul_254 ) ) /
                            std::sqrt( 1 + square_z * ( derX * derX + derY * derY ) )
                            , 255.0f );
      }
      else
      {
        // Weighted multi direction as in http://pubs.usgs.gov/of/1992/of92-422/of92-422.pdf
        // Fast formula from GDAL DEM
        const float xx = derX * derX;
        const float yy = derY * derY;
        const float xx_plus_yy = xx + yy;
        // Flat?
        if ( xx_plus_yy == 0.0 )
        {
          grayValue = qBound( 0.0f, static_cast<float>( 1.0 + sinZenithRad_mul_254 ), 255.0f );
        }
        else
        {
          // ... then the shade value from different azimuth
          float val225_mul_127 = sinZenithRad_mul_127 +
                                 ( derX - derY ) * cos225_az_mul_cos_alt_mul_z_mul_127;
          val225_mul_127 = ( val225_mul_127 <= 0.0 ) ? 0.0 : val225_mul_127;
          float val270_mul_127 = sinZenithRad_mul_127 -
                                 derX * cos_alt_mul_z_mul_127;
          val270_mul_127 = ( val270_mul_127 <= 0.0 ) ? 0.0 : val270_mul_127;
          float val315_mul_127 = sinZenithRad_mul_127 +
                                 ( derX + derY ) * cos225_az_mul_cos_alt_mul_z_mul_127;
          val315_mul_127 = ( val315_mul_127 <= 0.0 ) ? 0.0 : val315_mul_127;
          float val360_mul_127 = sinZenithRad_mul_127 -
                                 derY * cos_alt_mul_z_mul_127;
          val360_mul_127 = ( val360_mul_127 <= 0.0 ) ? 0.0 : val360_mul_127;

          // ... then the weighted shading
          const float weight_225 = 0.5 * xx_plus_yy - derX * derY;
          const float weight_270 = xx;
          const float weight_315 = xx_plus_yy - weight_225;
          const float weight_360 = yy;
          const float cang_mul_127 = (
                                       ( weight_225 * val225_mul_127 +
                                         weight_270 * val270_mul_127 +
                                         weight_315 * val315_mul_127 +
                                         weight_360 * val360_mul_127 ) / xx_plus_yy ) /
                                     ( 1 + square_z * xx_plus_yy );

          grayValue = qBound( 0.0f, 1.0f + cang_mul_127, 255.0f );
        }
      }

      float currentAlpha = mOpacity;
      if ( mRasterTransparency )
      {
        currentAlpha = mRasterTransparency->alphaValue( x22, mOpacity * 255 ) / 255.0;
      }
      if ( mAlphaBand > 0 )
      {
        currentAlpha *= alphaBlock->value( i ) / 255.0;
      }

      if ( qgsDoubleNear( currentAlpha, 1.0 ) )
      {
        outputBlock->setColor( i, j, qRgba( grayValue, grayValue, grayValue, 255 ) );
      }
      else
      {
        outputBlock->setColor( i, j, qRgba( currentAlpha * grayValue, currentAlpha * grayValue, currentAlpha * grayValue, currentAlpha * 255 ) );
      }
    }
  }

#ifdef QGISDEBUG
  if ( QgsSettings().value( QStringLiteral( "Map/logCanvasRefreshEvent" ), false ).toBool() )
  {
    QgsMessageLog::logMessage( QStringLiteral( "CPU processing time for hillshade (%2 x %3 ): %4 ms" )
                               .arg( width )
                               .arg( height )
                               .arg( std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::system_clock::now() - startTime ).count() ),
                               tr( "Rendering" ) );
  }
#endif

  return outputBlock.release();
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
  return ( ( x13 + x23 + x23 + x33 ) - ( x11 + x21 + x21 + x31 ) ) / ( 8 * cellsize );
}

double QgsHillshadeRenderer::calcFirstDerY( double x11, double x21, double x31, double x12, double x22, double x32, double x13, double x23, double x33, double cellsize )
{
  Q_UNUSED( x21 );
  Q_UNUSED( x22 );
  Q_UNUSED( x23 );
  return ( ( x31 + x32 + x32 + x33 ) - ( x11 + x12 + x12 + x13 ) ) / ( 8 * -cellsize );
}




