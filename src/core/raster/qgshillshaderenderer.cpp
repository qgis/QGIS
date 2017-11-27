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
#include <memory>

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

  double cellXSize = extent.width() / double( width );
  double cellYSize = extent.height() / double( height );
  double zenithRad = std::max( 0.0, 90 - mLightAngle ) * M_PI / 180.0;
  double azimuthRad = -1 * mLightAzimuth * M_PI / 180.0;
  double cosZenithRad = std::cos( zenithRad );
  double sinZenithRad = std::sin( zenithRad );

  // Multi direction hillshade: http://pubs.usgs.gov/of/1992/of92-422/of92-422.pdf
  double angle0Rad = ( -1 * mLightAzimuth - 45 - 45 * 0.5 ) * M_PI / 180.0;
  double angle1Rad = ( -1 * mLightAzimuth - 45 * 0.5 ) * M_PI / 180.0;
  double angle2Rad = ( -1 * mLightAzimuth + 45 * 0.5 ) * M_PI / 180.0;
  double angle3Rad = ( -1 * mLightAzimuth + 45 + 45 * 0.5 ) * M_PI / 180.0;

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

      double derX = calcFirstDerX( x11, x21, x31, x12, x22, x32, x13, x23, x33, cellXSize );
      double derY = calcFirstDerY( x11, x21, x31, x12, x22, x32, x13, x23, x33, cellYSize );

      double slopeRad = std::atan( mZFactor * std::sqrt( derX * derX + derY * derY ) );
      double aspectRad = std::atan2( derX, -derY );


      double grayValue;
      if ( !mMultiDirectional )
      {
        // Standard single direction hillshade
        grayValue = qBound( 0.0, 255.0 * ( cosZenithRad * std::cos( slopeRad )
                                           + sinZenithRad * std::sin( slopeRad )
                                           * std::cos( azimuthRad - aspectRad ) ), 255.0 );
      }
      else
      {
        // Weighted multi direction as in http://pubs.usgs.gov/of/1992/of92-422/of92-422.pdf
        double weight0 = std::sin( aspectRad - angle0Rad );
        double weight1 = std::sin( aspectRad - angle1Rad );
        double weight2 = std::sin( aspectRad - angle2Rad );
        double weight3 = std::sin( aspectRad - angle3Rad );
        weight0 = weight0 * weight0;
        weight1 = weight1 * weight1;
        weight2 = weight2 * weight2;
        weight3 = weight3 * weight3;

        double cosSlope = cosZenithRad * std::cos( slopeRad );
        double sinSlope = sinZenithRad * std::sin( slopeRad );
        double color0 = cosSlope + sinSlope * std::cos( angle0Rad - aspectRad );
        double color1 = cosSlope + sinSlope * std::cos( angle1Rad - aspectRad );
        double color2 = cosSlope + sinSlope * std::cos( angle2Rad - aspectRad );
        double color3 = cosSlope + sinSlope * std::cos( angle3Rad - aspectRad );
        grayValue = qBound( 0.0, 255 * ( weight0 * color0 + weight1 * color1 + weight2 * color2 + weight3 * color3 ) * 0.5, 255.0 );
      }

      double currentAlpha = mOpacity;
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




