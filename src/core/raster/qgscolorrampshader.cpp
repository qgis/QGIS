/* **************************************************************************
                qgscolorrampshader.cpp -  description
                       -------------------
begin                : Fri Dec 28 2007
copyright            : (C) 2007 by Peter J. Ersts
email                : ersts@amnh.org

This class is based off of code that was originally written by Marco Hugentobler and
originally part of the larger QgsRasterLayer class
****************************************************************************/

/* **************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// Threshold for treating values as exact match.
// Set to 0.0 to support displaying small values (http://hub.qgis.org/issues/12581)
#define DOUBLE_DIFF_THRESHOLD 0.0 // 0.0000001

#include "qgslogger.h"
#include "qgis.h"
#include "qgscolorrampshader.h"

#include <cmath>

QgsColorRampShader::QgsColorRampShader( double theMinimumValue, double theMaximumValue )
    : QgsRasterShaderFunction( theMinimumValue, theMaximumValue )
    , mColorRampType( INTERPOLATED )
    , mLUTOffset( 0.0 )
    , mLUTFactor( 1.0 )
    , mLUTInitialized( false )
    , mClip( false )
{
  QgsDebugMsgLevel( "called.", 4 );
}

QString QgsColorRampShader::colorRampTypeAsQString()
{
  switch ( mColorRampType )
  {
    case INTERPOLATED:
      return QString( "INTERPOLATED" );
    case DISCRETE:
      return QString( "DISCRETE" );
    case EXACT:
      return QString( "EXACT" );
  }
  return QString( "Unknown" );
}

void QgsColorRampShader::setColorRampItemList( const QList<QgsColorRampShader::ColorRampItem>& theList )
{
  mColorRampItemList = theList.toVector();
  // Reset the look up table when the color ramp is changed
  mLUTInitialized = false;
  mLUT.clear();
}

void QgsColorRampShader::setColorRampType( QgsColorRampShader::ColorRamp_TYPE theColorRampType )
{
  mColorRampType = theColorRampType;
}

void QgsColorRampShader::setColorRampType( const QString& theType )
{
  if ( theType == "INTERPOLATED" )
  {
    mColorRampType = INTERPOLATED;
  }
  else if ( theType == "DISCRETE" )
  {
    mColorRampType = DISCRETE;
  }
  else
  {
    mColorRampType = EXACT;
  }
}

bool QgsColorRampShader::shade( double theValue, int* theReturnRedValue, int* theReturnGreenValue, int* theReturnBlueValue, int *theReturnAlphaValue )
{
  if ( mColorRampItemList.isEmpty() )
  {
    return false;
  }
  int colorRampItemListCount = mColorRampItemList.count();
  int idx;
  if ( !mLUTInitialized )
  {
    // calculate LUT for faster index recovery
    mLUTFactor = 1.0;
    double minimumValue = mColorRampItemList.first().value;
    mLUTOffset = minimumValue + DOUBLE_DIFF_THRESHOLD;
    // Only make lut if at least 3 items, with 2 items the low and high cases handle both
    if ( colorRampItemListCount >= 3 )
    {
      double rangeValue = mColorRampItemList.at( colorRampItemListCount - 2 ).value - minimumValue;
      if ( rangeValue > 0 )
      {
        int lutSize = 256; // TODO: test if speed can be increased with a different LUT size
        mLUTFactor = ( lutSize - 0.0000001 ) / rangeValue; // decrease slightly to make sure last LUT category is correct
        idx = 0;
        double val;
        mLUT.reserve( lutSize );
        for ( int i = 0; i < lutSize; i++ )
        {
          val = ( i / mLUTFactor ) + mLUTOffset;
          while ( idx < colorRampItemListCount
                  && mColorRampItemList.at( idx ).value - DOUBLE_DIFF_THRESHOLD < val )
          {
            idx++;
          }
          mLUT.push_back( idx );
        }
      }
    }
    mLUTInitialized = true;
  }

  // overflow indicates that theValue > maximum value + DOUBLE_DIFF_THRESHOLD
  // that way idx can point to the last valid item
  bool overflow = false;

  // find index of the first ColorRampItem that is equal or higher to theValue
  int lutIndex = ( theValue - mLUTOffset ) * mLUTFactor;
  if ( theValue < mLUTOffset )
  {
    idx = 0;
  }
  else if ( lutIndex >= mLUT.count() )
  {
    idx = colorRampItemListCount - 1;
    if ( mColorRampItemList.at( idx ).value + DOUBLE_DIFF_THRESHOLD < theValue )
    {
      overflow = true;
    }
  }
  else
  {
    // get initial value from LUT
    idx = mLUT.at( lutIndex );

    // check if it's correct and if not increase untill correct
    // the LUT is made in such a way the index is always correct or too low, never too high
    while ( idx < colorRampItemListCount && mColorRampItemList.at( idx ).value + DOUBLE_DIFF_THRESHOLD < theValue )
    {
      idx++;
    }
    if ( idx >= colorRampItemListCount )
    {
      idx = colorRampItemListCount - 1;
      overflow = true;
    }
  }

  const QgsColorRampShader::ColorRampItem& currentColorRampItem = mColorRampItemList.at( idx );

  if ( QgsColorRampShader::INTERPOLATED == mColorRampType )
  { // Interpolate the color between two class breaks linearly.
    if ( idx < 1 || overflow || currentColorRampItem.value - DOUBLE_DIFF_THRESHOLD <= theValue )
    {
      if ( mClip && ( overflow
                      || currentColorRampItem.value - DOUBLE_DIFF_THRESHOLD > theValue ) )
      {
        return false;
      }
      *theReturnRedValue   = currentColorRampItem.color.red();
      *theReturnGreenValue = currentColorRampItem.color.green();
      *theReturnBlueValue  = currentColorRampItem.color.blue();
      *theReturnAlphaValue = currentColorRampItem.color.alpha();
      return true;
    }

    const QgsColorRampShader::ColorRampItem& previousColorRampItem = mColorRampItemList.at( idx - 1 );

    double currentRampRange = currentColorRampItem.value - previousColorRampItem.value;
    double offsetInRange = theValue - previousColorRampItem.value;
    double scale = offsetInRange / currentRampRange;

    *theReturnRedValue   = static_cast< int >( static_cast< double >( previousColorRampItem.color.red() )   + ( static_cast< double >( currentColorRampItem.color.red()   - previousColorRampItem.color.red() )   * scale ) );
    *theReturnGreenValue = static_cast< int >( static_cast< double >( previousColorRampItem.color.green() ) + ( static_cast< double >( currentColorRampItem.color.green() - previousColorRampItem.color.green() ) * scale ) );
    *theReturnBlueValue  = static_cast< int >( static_cast< double >( previousColorRampItem.color.blue() )  + ( static_cast< double >( currentColorRampItem.color.blue()  - previousColorRampItem.color.blue() )  * scale ) );
    *theReturnAlphaValue = static_cast< int >( static_cast< double >( previousColorRampItem.color.alpha() ) + ( static_cast< double >( currentColorRampItem.color.alpha() - previousColorRampItem.color.alpha() ) * scale ) );
    return true;
  }
  else if ( QgsColorRampShader::DISCRETE == mColorRampType )
  { // Assign the color of the higher class for every pixel between two class breaks.
    // NOTE: The implementation has always been different than the documentation,
    //       which said lower class before, see http://hub.qgis.org/issues/13995
    if ( overflow )
    {
      return false;
    }
    *theReturnRedValue   = currentColorRampItem.color.red();
    *theReturnGreenValue = currentColorRampItem.color.green();
    *theReturnBlueValue  = currentColorRampItem.color.blue();
    *theReturnAlphaValue = currentColorRampItem.color.alpha();
    return true;
  }
  else // EXACT
  { // Assign the color of the exact matching value in the color ramp item list
    if ( !overflow && currentColorRampItem.value - DOUBLE_DIFF_THRESHOLD <= theValue )
    {
      *theReturnRedValue   = currentColorRampItem.color.red();
      *theReturnGreenValue = currentColorRampItem.color.green();
      *theReturnBlueValue  = currentColorRampItem.color.blue();
      *theReturnAlphaValue = currentColorRampItem.color.alpha();
      return true;
    }
    else
    {
      return false;
    }
  }
}

bool QgsColorRampShader::shade( double theRedValue, double theGreenValue,
                                double theBlueValue, double theAlphaValue,
                                int* theReturnRedValue, int* theReturnGreenValue,
                                int* theReturnBlueValue, int* theReturnAlphaValue )
{
  Q_UNUSED( theRedValue );
  Q_UNUSED( theGreenValue );
  Q_UNUSED( theBlueValue );
  Q_UNUSED( theAlphaValue );

  *theReturnRedValue = 0;
  *theReturnGreenValue = 0;
  *theReturnBlueValue = 0;
  *theReturnAlphaValue = 0;

  return false;
}

void QgsColorRampShader::legendSymbologyItems( QList< QPair< QString, QColor > >& symbolItems ) const
{
  QVector<QgsColorRampShader::ColorRampItem>::const_iterator colorRampIt = mColorRampItemList.constBegin();
  for ( ; colorRampIt != mColorRampItemList.constEnd(); ++colorRampIt )
  {
    symbolItems.push_back( qMakePair( colorRampIt->label, colorRampIt->color ) );
  }
}
