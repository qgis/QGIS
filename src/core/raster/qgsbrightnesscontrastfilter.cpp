/***************************************************************************
                         qgsbrightnesscontrastfilter.cpp
                         ---------------------
    begin                : February 2013
    copyright            : (C) 2013 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrasterdataprovider.h"
#include "qgsbrightnesscontrastfilter.h"

#include <QDomDocument>
#include <QDomElement>

QgsBrightnessContrastFilter::QgsBrightnessContrastFilter( QgsRasterInterface *input )
  : QgsRasterInterface( input )
{
}

QgsBrightnessContrastFilter *QgsBrightnessContrastFilter::clone() const
{
  QgsDebugMsgLevel( QStringLiteral( "Entered" ), 4 );
  QgsBrightnessContrastFilter *filter = new QgsBrightnessContrastFilter( nullptr );
  filter->setBrightness( mBrightness );
  filter->setContrast( mContrast );
  filter->setGamma( mGamma );
  return filter;
}

int QgsBrightnessContrastFilter::bandCount() const
{
  if ( mOn )
  {
    return 1;
  }

  if ( mInput )
  {
    return mInput->bandCount();
  }

  return 0;
}

Qgis::DataType QgsBrightnessContrastFilter::dataType( int bandNo ) const
{
  if ( mOn )
  {
    return Qgis::ARGB32_Premultiplied;
  }

  if ( mInput )
  {
    return mInput->dataType( bandNo );
  }

  return Qgis::UnknownDataType;
}

bool QgsBrightnessContrastFilter::setInput( QgsRasterInterface *input )
{
  QgsDebugMsgLevel( QStringLiteral( "Entered" ), 4 );

  // Brightness filter can only work with single band ARGB32_Premultiplied
  if ( !input )
  {
    QgsDebugMsgLevel( QStringLiteral( "No input" ), 4 );
    return false;
  }

  if ( !mOn )
  {
    // In off mode we can connect to anything
    QgsDebugMsgLevel( QStringLiteral( "OK" ), 4 );
    mInput = input;
    return true;
  }

  if ( input->bandCount() < 1 )
  {
    QgsDebugMsg( QStringLiteral( "No input band" ) );
    return false;
  }

  if ( input->dataType( 1 ) != Qgis::ARGB32_Premultiplied &&
       input->dataType( 1 ) != Qgis::ARGB32 )
  {
    QgsDebugMsg( QStringLiteral( "Unknown input data type" ) );
    return false;
  }

  mInput = input;
  QgsDebugMsgLevel( QStringLiteral( "OK" ), 4 );
  return true;
}

QgsRasterBlock *QgsBrightnessContrastFilter::block( int bandNo, QgsRectangle  const &extent, int width, int height, QgsRasterBlockFeedback *feedback )
{
  Q_UNUSED( bandNo )
  QgsDebugMsgLevel( QStringLiteral( "width = %1 height = %2 extent = %3" ).arg( width ).arg( height ).arg( extent.toString() ), 4 );

  std::unique_ptr< QgsRasterBlock > outputBlock( new QgsRasterBlock() );
  if ( !mInput )
  {
    return outputBlock.release();
  }

  // At this moment we know that we read rendered image
  int bandNumber = 1;
  std::unique_ptr< QgsRasterBlock > inputBlock( mInput->block( bandNumber, extent, width, height, feedback ) );
  if ( !inputBlock || inputBlock->isEmpty() )
  {
    QgsDebugMsg( QStringLiteral( "No raster data!" ) );
    return outputBlock.release();
  }

  if ( mBrightness == 0 && mContrast == 0 && mGamma == 1.0 )
  {
    QgsDebugMsgLevel( QStringLiteral( "No brightness/contrast/gamma changes." ), 4 );
    return inputBlock.release();
  }

  if ( !outputBlock->reset( Qgis::ARGB32_Premultiplied, width, height ) )
  {
    return outputBlock.release();
  }

  // adjust image
  QRgb myNoDataColor = qRgba( 0, 0, 0, 0 );
  QRgb myColor;

  int r, g, b, alpha;
  double f = std::pow( ( mContrast + 100 ) / 100.0, 2 );
  double gammaCorrection = 1.0 / mGamma;

  for ( qgssize i = 0; i < ( qgssize )width * height; i++ )
  {
    if ( inputBlock->color( i ) == myNoDataColor )
    {
      outputBlock->setColor( i, myNoDataColor );
      continue;
    }

    myColor = inputBlock->color( i );
    alpha = qAlpha( myColor );

    r = adjustColorComponent( qRed( myColor ), alpha, mBrightness, f, gammaCorrection );
    g = adjustColorComponent( qGreen( myColor ), alpha, mBrightness, f, gammaCorrection );
    b = adjustColorComponent( qBlue( myColor ), alpha, mBrightness, f, gammaCorrection );

    outputBlock->setColor( i, qRgba( r, g, b, alpha ) );
  }

  return outputBlock.release();
}

int QgsBrightnessContrastFilter::adjustColorComponent( int colorComponent, int alpha, int brightness, double contrastFactor, double gammaCorrection ) const
{
  if ( alpha == 255 )
  {
    // Opaque pixel, do simpler math
    return qBound( 0, ( int )( 255 * std::pow( ( ( ( ( ( ( colorComponent / 255.0 ) - 0.5 ) * contrastFactor ) + 0.5 ) * 255 ) + brightness ) / 255.0, gammaCorrection ) ), 255 );
  }
  else if ( alpha == 0 )
  {
    // Totally transparent pixel
    return 0;
  }
  else
  {
    // Semi-transparent pixel. We need to adjust the math since we are using Qgis::ARGB32_Premultiplied
    // and color values have been premultiplied by alpha
    double alphaFactor = alpha / 255.;
    double adjustedColor = colorComponent / alphaFactor;

    // Make sure to return a premultiplied color
    return alphaFactor * qBound( 0., 255 * std::pow( ( ( ( ( ( ( adjustedColor / 255.0 ) - 0.5 ) * contrastFactor ) + 0.5 ) * 255 ) + brightness ) / 255, gammaCorrection ), 255. );
  }
}

void QgsBrightnessContrastFilter::writeXml( QDomDocument &doc, QDomElement &parentElem ) const
{
  if ( parentElem.isNull() )
  {
    return;
  }

  QDomElement filterElem = doc.createElement( QStringLiteral( "brightnesscontrast" ) );

  filterElem.setAttribute( QStringLiteral( "brightness" ), QString::number( mBrightness ) );
  filterElem.setAttribute( QStringLiteral( "contrast" ), QString::number( mContrast ) );
  filterElem.setAttribute( QStringLiteral( "gamma" ), QString::number( mGamma ) );
  parentElem.appendChild( filterElem );
}

void QgsBrightnessContrastFilter::readXml( const QDomElement &filterElem )
{
  if ( filterElem.isNull() )
  {
    return;
  }

  mBrightness = filterElem.attribute( QStringLiteral( "brightness" ), QStringLiteral( "0" ) ).toInt();
  mContrast = filterElem.attribute( QStringLiteral( "contrast" ), QStringLiteral( "0" ) ).toInt();
  mGamma = filterElem.attribute( QStringLiteral( "gamma" ), QStringLiteral( "1" ) ).toDouble();
}
