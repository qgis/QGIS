/***************************************************************************
                         qgshuesaturationfilter.cpp
                         ---------------------
    begin                : February 2013
    copyright            : (C) 2013 by Alexander Bruy, Nyall Dawson
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
#include "qgshuesaturationfilter.h"

#include <QDomDocument>
#include <QDomElement>


QgsHueSaturationFilter::QgsHueSaturationFilter( QgsRasterInterface *input )
  : QgsRasterInterface( input )
  , mColorizeColor( QColor::fromRgb( 255, 128, 128 ) )
{
}

QgsHueSaturationFilter *QgsHueSaturationFilter::clone() const
{
  QgsDebugMsgLevel( QStringLiteral( "Entered hue/saturation filter" ), 4 );
  QgsHueSaturationFilter *filter = new QgsHueSaturationFilter( nullptr );
  filter->setInvertColors( mInvertColors );
  filter->setSaturation( mSaturation );
  filter->setGrayscaleMode( mGrayscaleMode );
  filter->setColorizeOn( mColorizeOn );
  filter->setColorizeColor( mColorizeColor );
  filter->setColorizeStrength( mColorizeStrength );
  return filter;
}

int QgsHueSaturationFilter::bandCount() const
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

Qgis::DataType QgsHueSaturationFilter::dataType( int bandNo ) const
{
  if ( mOn )
  {
    return Qgis::DataType::ARGB32_Premultiplied;
  }

  if ( mInput )
  {
    return mInput->dataType( bandNo );
  }

  return Qgis::DataType::UnknownDataType;
}

bool QgsHueSaturationFilter::setInput( QgsRasterInterface *input )
{
  QgsDebugMsgLevel( QStringLiteral( "Entered" ), 4 );

  // Hue/saturation filter can only work with single band ARGB32_Premultiplied
  if ( !input )
  {
    QgsDebugMsg( QStringLiteral( "No input" ) );
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

  if ( input->dataType( 1 ) != Qgis::DataType::ARGB32_Premultiplied &&
       input->dataType( 1 ) != Qgis::DataType::ARGB32 )
  {
    QgsDebugMsg( QStringLiteral( "Unknown input data type" ) );
    return false;
  }

  mInput = input;
  QgsDebugMsgLevel( QStringLiteral( "OK" ), 4 );
  return true;
}

QgsRasterBlock *QgsHueSaturationFilter::block( int bandNo, QgsRectangle  const &extent, int width, int height, QgsRasterBlockFeedback *feedback )
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

  if ( !mInvertColors && mSaturation == 0 && mGrayscaleMode == GrayscaleOff && !mColorizeOn )
  {
    QgsDebugMsgLevel( QStringLiteral( "No hue/saturation change." ), 4 );
    return inputBlock.release();
  }

  if ( !outputBlock->reset( Qgis::DataType::ARGB32_Premultiplied, width, height ) )
  {
    return outputBlock.release();
  }

  // adjust image
  QRgb myNoDataColor = qRgba( 0, 0, 0, 0 );
  QRgb myRgb;
  QColor myColor;
  int h, s, l;
  int r, g, b, alpha;
  double alphaFactor = 1.0;

  for ( qgssize i = 0; i < ( qgssize )width * height; i++ )
  {
    if ( inputBlock->color( i ) == myNoDataColor )
    {
      outputBlock->setColor( i, myNoDataColor );
      continue;
    }

    myRgb = inputBlock->color( i );
    myColor = QColor( myRgb );

    // Alpha must be taken from QRgb, since conversion from QRgb->QColor loses alpha
    alpha = qAlpha( myRgb );

    if ( alpha == 0 )
    {
      // totally transparent, no changes required
      outputBlock->setColor( i, myRgb );
      continue;
    }

    // Get rgb for color
    myColor.getRgb( &r, &g, &b );

    if ( mInvertColors || alpha != 255 )
    {
      if ( mInvertColors )
      {
        r = 255 - r;
        g = 255 - g;
        b = 255 - b;
      }
      if ( alpha != 255 )
      {
        // Semi-transparent pixel. We need to adjust the colors since we are using Qgis::DataType::ARGB32_Premultiplied
        // and color values have been premultiplied by alpha
        alphaFactor = alpha / 255.;
        r /= alphaFactor;
        g /= alphaFactor;
        b /= alphaFactor;
      }
      myColor = QColor::fromRgb( r, g, b );
    }

    myColor.getHsl( &h, &s, &l );

    // Changing saturation?
    if ( ( mGrayscaleMode != GrayscaleOff ) || ( mSaturationScale != 1 ) )
    {
      processSaturation( r, g, b, h, s, l );
    }

    // Colorizing?
    if ( mColorizeOn )
    {
      processColorization( r, g, b, h, s, l );
    }

    // Convert back to rgb
    if ( alpha != 255 )
    {
      // Transparent pixel, need to premultiply color components
      r *= alphaFactor;
      g *= alphaFactor;
      b *= alphaFactor;
    }

    outputBlock->setColor( i, qRgba( r, g, b, alpha ) );
  }

  return outputBlock.release();
}

// Process a colorization and update resultant HSL & RGB values
void QgsHueSaturationFilter::processColorization( int &r, int &g, int &b, int &h, int &s, int &l ) const
{
  QColor myColor;

  // Overwrite hue and saturation with values from colorize color
  h = mColorizeH;
  s = mColorizeS;


  QColor colorizedColor = QColor::fromHsl( h, s, l );

  if ( mColorizeStrength == 100 )
  {
    // Full strength
    myColor = colorizedColor;

    // RGB may have changed, update them
    myColor.getRgb( &r, &g, &b );
  }
  else
  {
    // Get rgb for colorized color
    int colorizedR, colorizedG, colorizedB;
    colorizedColor.getRgb( &colorizedR, &colorizedG, &colorizedB );

    // Now, linearly scale by colorize strength
    double p = ( double ) mColorizeStrength / 100.;
    r = p * colorizedR + ( 1 - p ) * r;
    g = p * colorizedG + ( 1 - p ) * g;
    b = p * colorizedB + ( 1 - p ) * b;

    // RGB changed, so update HSL values
    myColor = QColor::fromRgb( r, g, b );
    myColor.getHsl( &h, &s, &l );
  }
}

// Process a change in saturation and update resultant HSL & RGB values
void QgsHueSaturationFilter::processSaturation( int &r, int &g, int &b, int &h, int &s, int &l )
{

  QColor myColor;

  // Are we converting layer to grayscale?
  switch ( mGrayscaleMode )
  {
    case GrayscaleLightness:
    {
      // Lightness mode, set saturation to zero
      s = 0;

      // Saturation changed, so update rgb values
      myColor = QColor::fromHsl( h, s, l );
      myColor.getRgb( &r, &g, &b );
      return;
    }
    case GrayscaleLuminosity:
    {
      // Grayscale by weighted rgb components
      int luminosity = 0.21 * r + 0.72 * g + 0.07 * b;
      r = g = b = luminosity;

      // RGB changed, so update HSL values
      myColor = QColor::fromRgb( r, g, b );
      myColor.getHsl( &h, &s, &l );
      return;
    }
    case GrayscaleAverage:
    {
      // Grayscale by average of rgb components
      int average = ( r + g + b ) / 3;
      r = g = b = average;

      // RGB changed, so update HSL values
      myColor = QColor::fromRgb( r, g, b );
      myColor.getHsl( &h, &s, &l );
      return;
    }
    case GrayscaleOff:
    {
      // Not being made grayscale, do saturation change
      if ( mSaturationScale < 1 )
      {
        // Lowering the saturation. Use a simple linear relationship
        s = std::min( ( int )( s * mSaturationScale ), 255 );
      }
      else
      {
        // Raising the saturation. Use a saturation curve to prevent
        // clipping at maximum saturation with ugly results.
        s = std::min( ( int )( 255. * ( 1 - std::pow( 1 - ( s / 255. ), std::pow( mSaturationScale, 2 ) ) ) ), 255 );
      }

      // Saturation changed, so update rgb values
      myColor = QColor::fromHsl( h, s, l );
      myColor.getRgb( &r, &g, &b );
      return;
    }
  }
}

void QgsHueSaturationFilter::setSaturation( int saturation )
{
  mSaturation = std::clamp( saturation, -100, 100 );

  // Scale saturation value to [0-2], where 0 = desaturated
  mSaturationScale = ( ( double ) mSaturation / 100 ) + 1;
}

void QgsHueSaturationFilter::setColorizeColor( const QColor &colorizeColor )
{
  mColorizeColor = colorizeColor;

  // Get hue, saturation for colorized color
  mColorizeH = mColorizeColor.hue();
  mColorizeS = mColorizeColor.saturation();
}

void QgsHueSaturationFilter::writeXml( QDomDocument &doc, QDomElement &parentElem ) const
{
  if ( parentElem.isNull() )
  {
    return;
  }

  QDomElement filterElem = doc.createElement( QStringLiteral( "huesaturation" ) );

  filterElem.setAttribute( QStringLiteral( "saturation" ), QString::number( mSaturation ) );
  filterElem.setAttribute( QStringLiteral( "grayscaleMode" ), QString::number( mGrayscaleMode ) );
  filterElem.setAttribute( QStringLiteral( "invertColors" ), QString::number( mInvertColors ) );
  filterElem.setAttribute( QStringLiteral( "colorizeOn" ), QString::number( mColorizeOn ) );
  filterElem.setAttribute( QStringLiteral( "colorizeRed" ), QString::number( mColorizeColor.red() ) );
  filterElem.setAttribute( QStringLiteral( "colorizeGreen" ), QString::number( mColorizeColor.green() ) );
  filterElem.setAttribute( QStringLiteral( "colorizeBlue" ), QString::number( mColorizeColor.blue() ) );
  filterElem.setAttribute( QStringLiteral( "colorizeStrength" ), QString::number( mColorizeStrength ) );

  parentElem.appendChild( filterElem );
}

void QgsHueSaturationFilter::readXml( const QDomElement &filterElem )
{
  if ( filterElem.isNull() )
  {
    return;
  }

  setSaturation( filterElem.attribute( QStringLiteral( "saturation" ), QStringLiteral( "0" ) ).toInt() );
  mGrayscaleMode = static_cast< QgsHueSaturationFilter::GrayscaleMode >( filterElem.attribute( QStringLiteral( "grayscaleMode" ), QStringLiteral( "0" ) ).toInt() );
  mInvertColors = static_cast< bool >( filterElem.attribute( QStringLiteral( "invertColors" ), QStringLiteral( "0" ) ).toInt() );

  mColorizeOn = static_cast< bool >( filterElem.attribute( QStringLiteral( "colorizeOn" ), QStringLiteral( "0" ) ).toInt() );
  int mColorizeRed = filterElem.attribute( QStringLiteral( "colorizeRed" ), QStringLiteral( "255" ) ).toInt();
  int mColorizeGreen = filterElem.attribute( QStringLiteral( "colorizeGreen" ), QStringLiteral( "128" ) ).toInt();
  int mColorizeBlue = filterElem.attribute( QStringLiteral( "colorizeBlue" ), QStringLiteral( "128" ) ).toInt();
  setColorizeColor( QColor::fromRgb( mColorizeRed, mColorizeGreen, mColorizeBlue ) );
  mColorizeStrength = filterElem.attribute( QStringLiteral( "colorizeStrength" ), QStringLiteral( "100" ) ).toInt();

}
