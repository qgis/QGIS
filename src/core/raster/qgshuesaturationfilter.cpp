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


QgsHueSaturationFilter::QgsHueSaturationFilter( QgsRasterInterface* input )
    : QgsRasterInterface( input ),
    mSaturation( 0 ),
    mGrayscaleMode( QgsHueSaturationFilter::GrayscaleOff ),
    mColorizeOn( false ),
    mColorizeColor( QColor::fromRgb( 255, 128, 128 ) ),
    mColorizeStrength( 100 )
{
}

QgsHueSaturationFilter::~QgsHueSaturationFilter()
{
}

QgsRasterInterface * QgsHueSaturationFilter::clone() const
{
  QgsDebugMsg( "Entered hue/saturation filter" );
  QgsHueSaturationFilter * filter = new QgsHueSaturationFilter( 0 );
  filter->setSaturation( mSaturation );
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

QGis::DataType QgsHueSaturationFilter::dataType( int bandNo ) const
{
  if ( mOn )
  {
    return QGis::ARGB32_Premultiplied;
  }

  if ( mInput )
  {
    return mInput->dataType( bandNo );
  }

  return QGis::UnknownDataType;
}

bool QgsHueSaturationFilter::setInput( QgsRasterInterface* input )
{
  QgsDebugMsg( "Entered" );

  // Hue/saturation filter can only work with single band ARGB32_Premultiplied
  if ( !input )
  {
    QgsDebugMsg( "No input" );
    return false;
  }

  if ( !mOn )
  {
    // In off mode we can connect to anything
    QgsDebugMsg( "OK" );
    mInput = input;
    return true;
  }

  if ( input->bandCount() < 1 )
  {
    QgsDebugMsg( "No input band" );
    return false;
  }

  if ( input->dataType( 1 ) != QGis::ARGB32_Premultiplied &&
       input->dataType( 1 ) != QGis::ARGB32 )
  {
    QgsDebugMsg( "Unknown input data type" );
    return false;
  }

  mInput = input;
  QgsDebugMsg( "OK" );
  return true;
}

QgsRasterBlock * QgsHueSaturationFilter::block( int bandNo, QgsRectangle  const & extent, int width, int height )
{
  Q_UNUSED( bandNo );
  QgsDebugMsg( "Entered hue/saturation filter block" );

  QgsRasterBlock *outputBlock = new QgsRasterBlock();
  if ( !mInput )
  {
    return outputBlock;
  }

  // At this moment we know that we read rendered image
  int bandNumber = 1;
  QgsRasterBlock *inputBlock = mInput->block( bandNumber, extent, width, height );
  if ( !inputBlock || inputBlock->isEmpty() )
  {
    QgsDebugMsg( "No raster data!" );
    delete inputBlock;
    return outputBlock;
  }

  if ( mSaturation == 0 && mGrayscaleMode == GrayscaleOff && !mColorizeOn )
  {
    QgsDebugMsg( "No hue/saturation change." );
    delete outputBlock;
    return inputBlock;
  }

  if ( !outputBlock->reset( QGis::ARGB32_Premultiplied, width, height ) )
  {
    delete inputBlock;
    return outputBlock;
  }

  // adjust image
  QRgb myNoDataColor = qRgba( 0, 0, 0, 0 );
  QColor myColor;
  int h, s, l;
  int r, g, b;

  // Scale saturation value to [0-2], where 0 = desaturated
  double saturationScale = (( double ) mSaturation / 100 ) + 1;

  // Get hue, saturation for colorized color
  int colorizeH, colorizeS;
  colorizeH = mColorizeColor.hue();
  colorizeS = mColorizeColor.saturation();

  for ( size_t i = 0; i < ( size_t )width*height; i++ )
  {
    if ( inputBlock->color( i ) == myNoDataColor )
    {
      outputBlock->setColor( i, myNoDataColor );
      continue;
    }

    // Get hsv and rgb for color
    myColor = QColor( inputBlock->color( i ) );
    myColor.getHsl( &h, &s, &l );
    myColor.getRgb( &r, &g, &b );

    switch ( mGrayscaleMode )
    {
      case GrayscaleLightness:
      {
        // Lightness mode, set saturation to zero
        s = 0;
        myColor = QColor::fromHsl( h, s, l );
        break;
      }
      case GrayscaleLuminosity:
      {
        // Grayscale by weighted rgb components
        int luminosity = 0.21 * r + 0.72 * g + 0.07 * b;
        r = g = b = luminosity;
        myColor = QColor::fromRgb( r, g, b );
        break;
      }
      case GrayscaleAverage:
      {
        // Grayscale by average of rgb components
        int average = ( r + g + b ) / 3;
        r = g = b = average;
        myColor = QColor::fromRgb( r, g, b );
        break;
      }
      case GrayscaleOff:
      {
        // Not being made grayscale, do saturation change
        if ( saturationScale < 1 )
        {
          // Lowering the saturation. Use a simple linear relationship
          s = qMin(( int )( s * saturationScale ), 255 );
        }
        else
        {
          // Raising the saturation. Use a saturation curve to prevent
          // clipping at maximum saturation with ugly results.
          s = qMin(( int )( 255. * ( 1 - pow( 1 - (( double )s / 255. )  , saturationScale * 2 ) ) ), 255 );
        }
        myColor = QColor::fromHsl( h, s, l );
        break;
      }
    }

    // Colorizing?
    if ( mColorizeOn )
    {
      // Update hsl, rgb values (these may have changed with saturation/grayscale adjustments)
      myColor.getHsl( &h, &s, &l );
      myColor.getRgb( &r, &g, &b );

      // Overwrite hue and saturation with values from colorize color
      h = colorizeH;
      s = colorizeS;
      QColor colorizedColor = QColor::fromHsl( h, s, l );

      if ( mColorizeStrength == 100 )
      {
        // Full strength
        myColor = colorizedColor;
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
        myColor = QColor::fromRgb( r, g, b );
      }
    }

    // Convert back to rgb
    outputBlock->setColor( i, myColor.rgb() );
  }

  delete inputBlock;
  return outputBlock;
}

void QgsHueSaturationFilter::writeXML( QDomDocument& doc, QDomElement& parentElem )
{
  if ( parentElem.isNull() )
  {
    return;
  }

  QDomElement filterElem = doc.createElement( "huesaturation" );

  filterElem.setAttribute( "saturation", QString::number( mSaturation ) );
  filterElem.setAttribute( "grayscaleMode", QString::number( mGrayscaleMode ) );
  filterElem.setAttribute( "colorizeOn", QString::number( mColorizeOn ) );
  filterElem.setAttribute( "colorizeRed", QString::number( mColorizeColor.red() ) );
  filterElem.setAttribute( "colorizeGreen", QString::number( mColorizeColor.green() ) );
  filterElem.setAttribute( "colorizeBlue", QString::number( mColorizeColor.blue() ) );
  filterElem.setAttribute( "colorizeStrength", QString::number( mColorizeStrength ) );

  parentElem.appendChild( filterElem );
}

void QgsHueSaturationFilter::readXML( const QDomElement& filterElem )
{
  if ( filterElem.isNull() )
  {
    return;
  }

  mSaturation = filterElem.attribute( "saturation", "0" ).toInt();
  mGrayscaleMode = ( QgsHueSaturationFilter::GrayscaleMode )filterElem.attribute( "grayscaleMode", "0" ).toInt();

  mColorizeOn = ( bool )filterElem.attribute( "colorizeOn", "0" ).toInt();
  int mColorizeRed = filterElem.attribute( "colorizeRed", "255" ).toInt();
  int mColorizeGreen = filterElem.attribute( "colorizeGreen", "0" ).toInt();
  int mColorizeBlue = filterElem.attribute( "colorizeBlue", "0" ).toInt();
  mColorizeColor = QColor::fromRgb( mColorizeRed, mColorizeGreen, mColorizeBlue );
  mColorizeStrength = filterElem.attribute( "colorizeStrength", "100" ).toInt();

}
