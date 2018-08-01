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
// Set to 0.0 to support displaying small values (https://issues.qgis.org/issues/12581)
#define DOUBLE_DIFF_THRESHOLD 0.0 // 0.0000001

#include "qgslogger.h"
#include "qgis.h"
#include "qgscolorramp.h"
#include "qgscolorrampshader.h"
#include "qgsrasterinterface.h"
#include "qgsrasterminmaxorigin.h"

#include <cmath>
QgsColorRampShader::QgsColorRampShader( double minimumValue, double maximumValue, QgsColorRamp *colorRamp, Type type, ClassificationMode classificationMode )
  : QgsRasterShaderFunction( minimumValue, maximumValue )
  , mColorRampType( type )
  , mClassificationMode( classificationMode )
{
  QgsDebugMsgLevel( "called.", 4 );

  setSourceColorRamp( colorRamp );
}

QgsColorRampShader::QgsColorRampShader( const QgsColorRampShader &other )
  : QgsRasterShaderFunction( other )
  , mColorRampType( other.mColorRampType )
  , mClassificationMode( other.mClassificationMode )
  , mLUT( other.mLUT )
  , mLUTOffset( other.mLUTOffset )
  , mLUTFactor( other.mLUTFactor )
  , mLUTInitialized( other.mLUTInitialized )
  , mClip( other.mClip )
{
  mSourceColorRamp.reset( other.sourceColorRamp()->clone() );
}

QgsColorRampShader &QgsColorRampShader::operator=( const QgsColorRampShader &other )
{
  mSourceColorRamp.reset( other.sourceColorRamp()->clone() );
  mColorRampType = other.mColorRampType;
  mClassificationMode = other.mClassificationMode;
  mLUT = other.mLUT;
  mLUTOffset = other.mLUTOffset;
  mLUTFactor = other.mLUTFactor;
  mLUTInitialized = other.mLUTInitialized;
  mClip = other.mClip;
  return *this;
}

QString QgsColorRampShader::colorRampTypeAsQString()
{
  switch ( mColorRampType )
  {
    case Interpolated:
      return QStringLiteral( "INTERPOLATED" );
    case Discrete:
      return QStringLiteral( "DISCRETE" );
    case Exact:
      return QStringLiteral( "EXACT" );
  }
  return QStringLiteral( "Unknown" );
}

void QgsColorRampShader::setColorRampItemList( const QList<QgsColorRampShader::ColorRampItem> &list )
{
  mColorRampItemList = list.toVector();
  // Reset the look up table when the color ramp is changed
  mLUTInitialized = false;
  mLUT.clear();
}

void QgsColorRampShader::setColorRampType( QgsColorRampShader::Type colorRampType )
{
  mColorRampType = colorRampType;
}

void QgsColorRampShader::setColorRampType( const QString &type )
{
  if ( type == QLatin1String( "INTERPOLATED" ) )
  {
    mColorRampType = Interpolated;
  }
  else if ( type == QLatin1String( "DISCRETE" ) )
  {
    mColorRampType = Discrete;
  }
  else
  {
    mColorRampType = Exact;
  }
}

QgsColorRamp *QgsColorRampShader::sourceColorRamp() const
{
  return mSourceColorRamp.get();
}

void QgsColorRampShader::setSourceColorRamp( QgsColorRamp *colorramp )
{
  mSourceColorRamp.reset( colorramp );
}

void QgsColorRampShader::classifyColorRamp( const int classes, const int band, const QgsRectangle &extent, QgsRasterInterface *input )
{
  if ( minimumValue() >= maximumValue() )
  {
    return;
  }

  bool discrete = colorRampType() == Discrete;

  QList<double> entryValues;
  QVector<QColor> entryColors;

  double min = minimumValue();
  double max = maximumValue();

  if ( classificationMode() == Continuous )
  {
    if ( sourceColorRamp() &&  sourceColorRamp()->count() > 1 )
    {
      int numberOfEntries = sourceColorRamp()->count();
      entryValues.reserve( numberOfEntries );
      if ( discrete )
      {
        double intervalDiff = max - min;

        // remove last class when ColorRamp is gradient and discrete, as they are implemented with an extra stop
        QgsGradientColorRamp *colorGradientRamp = dynamic_cast<QgsGradientColorRamp *>( sourceColorRamp() );
        if ( colorGradientRamp && colorGradientRamp->isDiscrete() )
        {
          numberOfEntries--;
        }
        else
        {
          // if color ramp is continuous scale values to get equally distributed classes.
          // Doesn't work perfectly when stops are non equally distributed.
          intervalDiff *= ( numberOfEntries - 1 ) / ( double )numberOfEntries;
        }

        // skip first value (always 0.0)
        for ( int i = 1; i < numberOfEntries; ++i )
        {
          double value = sourceColorRamp()->value( i );
          entryValues.push_back( min + value * intervalDiff );
        }
        entryValues.push_back( std::numeric_limits<double>::infinity() );
      }
      else
      {
        for ( int i = 0; i < numberOfEntries; ++i )
        {
          double value = sourceColorRamp()->value( i );
          entryValues.push_back( min + value * ( max - min ) );
        }
      }
      // for continuous mode take original color map colors
      for ( int i = 0; i < numberOfEntries; ++i )
      {
        int idx = i;
        entryColors.push_back( sourceColorRamp()->color( sourceColorRamp()->value( idx ) ) );
      }
    }
  }
  else // for other classification modes interpolate colors linearly
  {
    if ( classes < 2 )
      return; // < 2 classes is not useful, shouldn't happen, but if it happens save it from crashing

    if ( classificationMode() == Quantile )
    {
      // Quantile
      if ( band < 0 || !input )
        return; // quantile classificationr requires a valid band, minMaxOrigin, and input

      double cut1 = std::numeric_limits<double>::quiet_NaN();
      double cut2 = std::numeric_limits<double>::quiet_NaN();
      int sampleSize = 250000;

      // set min and max from histogram, used later to calculate number of decimals to display
      input->cumulativeCut( band, 0.0, 1.0, min, max, extent, sampleSize );

      entryValues.reserve( classes );
      if ( discrete )
      {
        double intervalDiff = 1.0 / ( classes );
        for ( int i = 1; i < classes; ++i )
        {
          input->cumulativeCut( band, 0.0, i * intervalDiff, cut1, cut2, extent, sampleSize );
          entryValues.push_back( cut2 );
        }
        entryValues.push_back( std::numeric_limits<double>::infinity() );
      }
      else
      {
        double intervalDiff = 1.0 / ( classes - 1 );
        for ( int i = 0; i < classes; ++i )
        {
          input->cumulativeCut( band, 0.0, i * intervalDiff, cut1, cut2, extent, sampleSize );
          entryValues.push_back( cut2 );
        }
      }
    }
    else // EqualInterval
    {
      entryValues.reserve( classes );
      if ( discrete )
      {
        // in discrete mode the lowest value is not an entry and the highest
        // value is inf, there are ( numberOfEntries ) of which the first
        // and last are not used.
        double intervalDiff = ( max - min ) / ( classes );

        for ( int i = 1; i < classes; ++i )
        {
          entryValues.push_back( min + i * intervalDiff );
        }
        entryValues.push_back( std::numeric_limits<double>::infinity() );
      }
      else
      {
        //because the highest value is also an entry, there are (numberOfEntries - 1) intervals
        double intervalDiff = ( max - min ) / ( classes - 1 );

        for ( int i = 0; i < classes; ++i )
        {
          entryValues.push_back( min + i * intervalDiff );
        }
      }
    }

    if ( !sourceColorRamp() || sourceColorRamp()->count() == 1 )
    {
      //hard code color range from blue -> red (previous default)
      int colorDiff = 0;
      if ( classes != 0 )
      {
        colorDiff = ( int )( 255 / classes );
      }

      entryColors.reserve( classes );
      for ( int i = 0; i < classes; ++i )
      {
        QColor currentColor;
        int idx = i;
        currentColor.setRgb( colorDiff * idx, 0, 255 - colorDiff * idx );
        entryColors.push_back( currentColor );
      }
    }
    else
    {
      entryColors.reserve( classes );
      for ( int i = 0; i < classes; ++i )
      {
        int idx = i;
        entryColors.push_back( sourceColorRamp()->color( ( ( double ) idx ) / ( classes - 1 ) ) );
      }
    }
  }

  QList<double>::const_iterator value_it = entryValues.constBegin();
  QVector<QColor>::const_iterator color_it = entryColors.constBegin();

  // calculate a reasonable number of decimals to display
  double maxabs = std::log10( std::max( std::fabs( max ), std::fabs( min ) ) );
  int nDecimals = std::round( std::max( 3.0 + maxabs - std::log10( max - min ), maxabs <= 15.0 ? maxabs + 0.49 : 0.0 ) );

  QList<QgsColorRampShader::ColorRampItem> colorRampItems;
  for ( ; value_it != entryValues.constEnd(); ++value_it, ++color_it )
  {
    QgsColorRampShader::ColorRampItem newColorRampItem;
    newColorRampItem.value = *value_it;
    newColorRampItem.color = *color_it;
    newColorRampItem.label = QString::number( *value_it, 'g', nDecimals );
    colorRampItems.append( newColorRampItem );
  }

  std::sort( colorRampItems.begin(), colorRampItems.end() );
  setColorRampItemList( colorRampItems );
}

void QgsColorRampShader::classifyColorRamp( const int band, const QgsRectangle &extent, QgsRasterInterface *input )
{
  classifyColorRamp( colorRampItemList().count(), band, extent, input );
}

bool QgsColorRampShader::shade( double value, int *returnRedValue, int *returnGreenValue, int *returnBlueValue, int *returnAlphaValue )
{
  if ( mColorRampItemList.isEmpty() )
  {
    return false;
  }
  if ( std::isnan( value ) || std::isinf( value ) )
    return false;

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

  // overflow indicates that value > maximum value + DOUBLE_DIFF_THRESHOLD
  // that way idx can point to the last valid item
  bool overflow = false;

  // find index of the first ColorRampItem that is equal or higher to theValue
  int lutIndex = ( value - mLUTOffset ) * mLUTFactor;
  if ( value < mLUTOffset )
  {
    idx = 0;
  }
  else if ( lutIndex >= mLUT.count() )
  {
    idx = colorRampItemListCount - 1;
    if ( mColorRampItemList.at( idx ).value + DOUBLE_DIFF_THRESHOLD < value )
    {
      overflow = true;
    }
  }
  else if ( lutIndex < 0 )
  {
    return false;
  }
  else
  {
    // get initial value from LUT
    idx = mLUT.at( lutIndex );

    // check if it's correct and if not increase until correct
    // the LUT is made in such a way the index is always correct or too low, never too high
    while ( idx < colorRampItemListCount && mColorRampItemList.at( idx ).value + DOUBLE_DIFF_THRESHOLD < value )
    {
      idx++;
    }
    if ( idx >= colorRampItemListCount )
    {
      idx = colorRampItemListCount - 1;
      overflow = true;
    }
  }

  const QgsColorRampShader::ColorRampItem &currentColorRampItem = mColorRampItemList.at( idx );

  if ( colorRampType() == Interpolated )
  {
    // Interpolate the color between two class breaks linearly.
    if ( idx < 1 || overflow || currentColorRampItem.value - DOUBLE_DIFF_THRESHOLD <= value )
    {
      if ( mClip && ( overflow
                      || currentColorRampItem.value - DOUBLE_DIFF_THRESHOLD > value ) )
      {
        return false;
      }
      *returnRedValue   = currentColorRampItem.color.red();
      *returnGreenValue = currentColorRampItem.color.green();
      *returnBlueValue  = currentColorRampItem.color.blue();
      *returnAlphaValue = currentColorRampItem.color.alpha();
      return true;
    }

    const QgsColorRampShader::ColorRampItem &previousColorRampItem = mColorRampItemList.at( idx - 1 );

    double currentRampRange = currentColorRampItem.value - previousColorRampItem.value;
    double offsetInRange = value - previousColorRampItem.value;
    double scale = offsetInRange / currentRampRange;

    *returnRedValue   = static_cast< int >( static_cast< double >( previousColorRampItem.color.red() )   + ( static_cast< double >( currentColorRampItem.color.red()   - previousColorRampItem.color.red() )   * scale ) );
    *returnGreenValue = static_cast< int >( static_cast< double >( previousColorRampItem.color.green() ) + ( static_cast< double >( currentColorRampItem.color.green() - previousColorRampItem.color.green() ) * scale ) );
    *returnBlueValue  = static_cast< int >( static_cast< double >( previousColorRampItem.color.blue() )  + ( static_cast< double >( currentColorRampItem.color.blue()  - previousColorRampItem.color.blue() )  * scale ) );
    *returnAlphaValue = static_cast< int >( static_cast< double >( previousColorRampItem.color.alpha() ) + ( static_cast< double >( currentColorRampItem.color.alpha() - previousColorRampItem.color.alpha() ) * scale ) );
    return true;
  }
  else if ( colorRampType() == Discrete )
  {
    // Assign the color of the higher class for every pixel between two class breaks.
    // NOTE: The implementation has always been different than the documentation,
    //       which said lower class before, see https://issues.qgis.org/issues/13995
    if ( overflow )
    {
      return false;
    }
    *returnRedValue   = currentColorRampItem.color.red();
    *returnGreenValue = currentColorRampItem.color.green();
    *returnBlueValue  = currentColorRampItem.color.blue();
    *returnAlphaValue = currentColorRampItem.color.alpha();
    return true;
  }
  else // EXACT
  {
    // Assign the color of the exact matching value in the color ramp item list
    if ( !overflow && currentColorRampItem.value - DOUBLE_DIFF_THRESHOLD <= value )
    {
      *returnRedValue   = currentColorRampItem.color.red();
      *returnGreenValue = currentColorRampItem.color.green();
      *returnBlueValue  = currentColorRampItem.color.blue();
      *returnAlphaValue = currentColorRampItem.color.alpha();
      return true;
    }
    else
    {
      return false;
    }
  }
}

bool QgsColorRampShader::shade( double redValue, double greenValue,
                                double blueValue, double alphaValue,
                                int *returnRedValue, int *returnGreenValue,
                                int *returnBlueValue, int *returnAlphaValue )
{
  Q_UNUSED( redValue );
  Q_UNUSED( greenValue );
  Q_UNUSED( blueValue );
  Q_UNUSED( alphaValue );

  *returnRedValue = 0;
  *returnGreenValue = 0;
  *returnBlueValue = 0;
  *returnAlphaValue = 0;

  return false;
}

void QgsColorRampShader::legendSymbologyItems( QList< QPair< QString, QColor > > &symbolItems ) const
{
  QVector<QgsColorRampShader::ColorRampItem>::const_iterator colorRampIt = mColorRampItemList.constBegin();
  for ( ; colorRampIt != mColorRampItemList.constEnd(); ++colorRampIt )
  {
    symbolItems.push_back( qMakePair( colorRampIt->label, colorRampIt->color ) );
  }
}
