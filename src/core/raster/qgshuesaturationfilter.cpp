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

#include "qgshuesaturationfilter.h"

#include "qgsrasterdataprovider.h"

#include <QDomDocument>
#include <QDomElement>

QgsHueSaturationFilter::QgsHueSaturationFilter( QgsRasterInterface *input )
  : QgsRasterInterface( input )
  , mColorizeColor( QColor::fromRgb( 255, 128, 128 ) )
{
}

QgsHueSaturationFilter *QgsHueSaturationFilter::clone() const
{
  QgsDebugMsgLevel( u"Entered hue/saturation filter"_s, 4 );
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
  QgsDebugMsgLevel( u"Entered"_s, 4 );

  // Hue/saturation filter can only work with single band ARGB32_Premultiplied
  if ( !input )
  {
    QgsDebugError( u"No input"_s );
    return false;
  }

  if ( !mOn )
  {
    // In off mode we can connect to anything
    QgsDebugMsgLevel( u"OK"_s, 4 );
    mInput = input;
    return true;
  }

  if ( input->bandCount() < 1 )
  {
    QgsDebugError( u"No input band"_s );
    return false;
  }

  if ( input->dataType( 1 ) != Qgis::DataType::ARGB32_Premultiplied &&
       input->dataType( 1 ) != Qgis::DataType::ARGB32 )
  {
    QgsDebugError( u"Unknown input data type"_s );
    return false;
  }

  mInput = input;
  QgsDebugMsgLevel( u"OK"_s, 4 );
  return true;
}

QgsRasterBlock *QgsHueSaturationFilter::block( int bandNo, QgsRectangle  const &extent, int width, int height, QgsRasterBlockFeedback *feedback )
{
  Q_UNUSED( bandNo )
  QgsDebugMsgLevel( u"width = %1 height = %2 extent = %3"_s.arg( width ).arg( height ).arg( extent.toString() ), 4 );

  if ( !mInput )
  {
    return nullptr;
  }

  // At this moment we know that we read rendered image
  int bandNumber = 1;
  std::unique_ptr< QgsRasterBlock > inputBlock( mInput->block( bandNumber, extent, width, height, feedback ) );
  if ( !inputBlock || inputBlock->isEmpty() )
  {
    QgsDebugError( u"No raster data!"_s );
    return nullptr;
  }

  if ( !mInvertColors && mSaturation == 0 && mGrayscaleMode == GrayscaleOff && !mColorizeOn )
  {
    QgsDebugMsgLevel( u"No hue/saturation change."_s, 4 );
    return inputBlock.release();
  }

  auto outputBlock = std::make_unique<QgsRasterBlock>();

  if ( !outputBlock->reset( Qgis::DataType::ARGB32_Premultiplied, width, height ) )
  {
    return nullptr;
  }

  // adjust image
  const QRgb myNoDataColor = qRgba( 0, 0, 0, 0 );
  int h, s, l;
  int r, g, b;
  double alphaFactor = 1.0;

  const QRgb *inputColorData = inputBlock->colorData();
  const int imageHeight = inputBlock->image().height();
  const int imageWidth = inputBlock->image().width();

  QRgb *outputColorData = outputBlock->colorData();

  for ( int row = 0; row < height; ++row )
  {
    if ( feedback->isCanceled() )
      return nullptr;

    for ( int col = 0; col < width; ++col )
    {
      const qgssize i = static_cast< qgssize >( row ) * width + static_cast< qgssize >( col );

      if ( !inputColorData || row >= imageHeight || col >= imageWidth || inputColorData[i] == myNoDataColor )
      {
        outputColorData[i] = myNoDataColor;
        continue;
      }

      const QRgb inputColor = inputColorData[i];
      QColor myColor = QColor( inputColor );

      // Alpha must be taken from QRgb, since conversion from QRgb->QColor loses alpha
      const int alpha = qAlpha( inputColor );

      if ( alpha == 0 )
      {
        // totally transparent, no changes required
        outputColorData[i] = inputColor;
        continue;
      }

      // Get rgb for color
      myColor.getRgb( &r, &g, &b );

      if ( mInvertColors || alpha != 255 )
      {
        if ( alpha != 255 )
        {
          // Semi-transparent pixel. We need to adjust the colors since we are using Qgis::DataType::ARGB32_Premultiplied
          // and color values have been premultiplied by alpha
          alphaFactor = alpha / 255.;
          r /= alphaFactor;
          g /= alphaFactor;
          b /= alphaFactor;
        }
        if ( mInvertColors )
        {
          r = 255 - r;
          g = 255 - g;
          b = 255 - b;
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

      outputColorData[i] = qRgba( r, g, b, alpha );
    }
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
void QgsHueSaturationFilter::processSaturation( int &r, int &g, int &b, int &h, int &s, int &l ) const
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

  QDomElement filterElem = doc.createElement( u"huesaturation"_s );

  filterElem.setAttribute( u"saturation"_s, QString::number( mSaturation ) );
  filterElem.setAttribute( u"grayscaleMode"_s, QString::number( mGrayscaleMode ) );
  filterElem.setAttribute( u"invertColors"_s, QString::number( mInvertColors ) );
  filterElem.setAttribute( u"colorizeOn"_s, QString::number( mColorizeOn ) );
  filterElem.setAttribute( u"colorizeRed"_s, QString::number( mColorizeColor.red() ) );
  filterElem.setAttribute( u"colorizeGreen"_s, QString::number( mColorizeColor.green() ) );
  filterElem.setAttribute( u"colorizeBlue"_s, QString::number( mColorizeColor.blue() ) );
  filterElem.setAttribute( u"colorizeStrength"_s, QString::number( mColorizeStrength ) );

  parentElem.appendChild( filterElem );
}

void QgsHueSaturationFilter::readXml( const QDomElement &filterElem )
{
  if ( filterElem.isNull() )
  {
    return;
  }

  setSaturation( filterElem.attribute( u"saturation"_s, u"0"_s ).toInt() );
  mGrayscaleMode = static_cast< QgsHueSaturationFilter::GrayscaleMode >( filterElem.attribute( u"grayscaleMode"_s, u"0"_s ).toInt() );
  mInvertColors = static_cast< bool >( filterElem.attribute( u"invertColors"_s, u"0"_s ).toInt() );

  mColorizeOn = static_cast< bool >( filterElem.attribute( u"colorizeOn"_s, u"0"_s ).toInt() );
  int mColorizeRed = filterElem.attribute( u"colorizeRed"_s, u"255"_s ).toInt();
  int mColorizeGreen = filterElem.attribute( u"colorizeGreen"_s, u"128"_s ).toInt();
  int mColorizeBlue = filterElem.attribute( u"colorizeBlue"_s, u"128"_s ).toInt();
  setColorizeColor( QColor::fromRgb( mColorizeRed, mColorizeGreen, mColorizeBlue ) );
  mColorizeStrength = filterElem.attribute( u"colorizeStrength"_s, u"100"_s ).toInt();

}
