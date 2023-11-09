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
// Set to 0.0 to support displaying small values (https://github.com/qgis/QGIS/issues/20706)
#define DOUBLE_DIFF_THRESHOLD 0.0 // 0.0000001

#include "qgslogger.h"
#include "qgis.h"
#include "qgscolorrampimpl.h"
#include "qgscolorrampshader.h"
#include "qgsrasterinterface.h"
#include "qgsrasterminmaxorigin.h"
#include "qgssymbollayerutils.h"
#include "qgsreadwritecontext.h"
#include "qgscolorramplegendnodesettings.h"

#include <cmath>
QgsColorRampShader::QgsColorRampShader( double minimumValue, double maximumValue, QgsColorRamp *colorRamp, Type type, ClassificationMode classificationMode )
  : QgsRasterShaderFunction( minimumValue, maximumValue )
  , mColorRampType( type )
  , mClassificationMode( classificationMode )
  , mLegendSettings( std::make_unique< QgsColorRampLegendNodeSettings >() )
{
  QgsDebugMsgLevel( QStringLiteral( "called." ), 4 );

  setSourceColorRamp( colorRamp );
}

QgsColorRampShader::~QgsColorRampShader() = default;

QgsColorRampShader::QgsColorRampShader( const QgsColorRampShader &other )
  : QgsRasterShaderFunction( other )
  , mColorRampType( other.mColorRampType )
  , mClassificationMode( other.mClassificationMode )
  , mLUT( other.mLUT )
  , mLUTOffset( other.mLUTOffset )
  , mLUTFactor( other.mLUTFactor )
  , mLUTInitialized( other.mLUTInitialized )
  , mClip( other.mClip )
  , mLegendSettings( other.legendSettings() ? new QgsColorRampLegendNodeSettings( *other.legendSettings() ) : new QgsColorRampLegendNodeSettings() )
{
  if ( auto *lSourceColorRamp = other.sourceColorRamp() )
    mSourceColorRamp.reset( lSourceColorRamp->clone() );
  mColorRampItemList = other.mColorRampItemList;
}

QgsColorRampShader &QgsColorRampShader::operator=( const QgsColorRampShader &other )
{
  QgsRasterShaderFunction::operator=( other );
  if ( auto *lSourceColorRamp = other.sourceColorRamp() )
    mSourceColorRamp.reset( lSourceColorRamp->clone() );
  else
    mSourceColorRamp.reset();

  mColorRampType = other.mColorRampType;
  mClassificationMode = other.mClassificationMode;
  mLUT = other.mLUT;
  mLUTOffset = other.mLUTOffset;
  mLUTFactor = other.mLUTFactor;
  mLUTInitialized = other.mLUTInitialized;
  mClip = other.mClip;
  mColorRampItemList = other.mColorRampItemList;
  mLegendSettings.reset( other.legendSettings() ? new QgsColorRampLegendNodeSettings( *other.legendSettings() ) : new QgsColorRampLegendNodeSettings() );
  return *this;
}

QString QgsColorRampShader::colorRampTypeAsQString() const
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

bool QgsColorRampShader::isEmpty() const
{
  return mColorRampItemList.isEmpty();
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

QgsColorRamp *QgsColorRampShader::createColorRamp() const
{
  std::unique_ptr<QgsGradientColorRamp> ramp = std::make_unique< QgsGradientColorRamp >();
  const int count = mColorRampItemList.size();
  if ( count == 0 )
  {
    const QColor none( 0, 0, 0, 0 );
    ramp->setColor1( none );
    ramp->setColor2( none );
  }
  else if ( count == 1 )
  {
    ramp->setColor1( mColorRampItemList[0].color );
    ramp->setColor2( mColorRampItemList[0].color );
  }
  else
  {
    QgsGradientStopsList stops;
    // minimum and maximum values can fall outside the range of the item list
    const double min = minimumValue();
    const double max = maximumValue();
    for ( int i = 0; i < count; i++ )
    {
      const double offset = ( mColorRampItemList[i].value - min ) / ( max - min );
      if ( i == 0 )
      {
        ramp->setColor1( mColorRampItemList[i].color );
        if ( offset <= 0.0 )
          continue;
      }
      else if ( i == count - 1 )
      {
        ramp->setColor2( mColorRampItemList[i].color );
        if ( offset >= 1.0 )
          continue;
      }
      stops << QgsGradientStop( offset, mColorRampItemList[i].color );
    }
    ramp->setStops( stops );
  }

  return ramp.release();
}

void QgsColorRampShader::setSourceColorRamp( QgsColorRamp *colorramp )
{
  mSourceColorRamp.reset( colorramp );
}

void QgsColorRampShader::classifyColorRamp( const int classes, const int band, const QgsRectangle &extent, QgsRasterInterface *input )
{
  if ( minimumValue() > maximumValue() )
    return;

  const bool discrete = colorRampType() == Discrete;

  QList<double> entryValues;
  QVector<QColor> entryColors;

  double min = minimumValue();
  double max = maximumValue();

  if ( minimumValue() == maximumValue() )
  {
    if ( sourceColorRamp() &&  sourceColorRamp()->count() > 1 )
    {
      entryValues.push_back( min );
      if ( discrete )
        entryValues.push_back( std::numeric_limits<double>::infinity() );
      for ( int i = 0; i < entryValues.size(); ++i )
        entryColors.push_back( sourceColorRamp()->color( sourceColorRamp()->value( i ) ) );
    }
  }
  else if ( classificationMode() == Continuous )
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
          intervalDiff *= ( numberOfEntries - 1 ) / static_cast<double>( numberOfEntries );
        }

        // skip first value (always 0.0)
        for ( int i = 1; i < numberOfEntries; ++i )
        {
          const double value = sourceColorRamp()->value( i );
          entryValues.push_back( min + value * intervalDiff );
        }
        entryValues.push_back( std::numeric_limits<double>::infinity() );
      }
      else
      {
        for ( int i = 0; i < numberOfEntries; ++i )
        {
          const double value = sourceColorRamp()->value( i );
          entryValues.push_back( min + value * ( max - min ) );
        }
      }
      // for continuous mode take original color map colors
      for ( int i = 0; i < numberOfEntries; ++i )
      {
        const int idx = i;
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
        return; // quantile classification requires a valid band, minMaxOrigin, and input

      double cut1 = std::numeric_limits<double>::quiet_NaN();
      double cut2 = std::numeric_limits<double>::quiet_NaN();
      // Note: the sample size in other parts of QGIS appears to be 25000, it is ten times here.
      const int sampleSize = 250000 * 10;

      // set min and max from histogram, used later to calculate number of decimals to display
      input->cumulativeCut( band, 0.0, 1.0, min, max, extent, sampleSize );

      entryValues.reserve( classes );
      if ( discrete )
      {
        const double intervalDiff = 1.0 / ( classes );
        for ( int i = 1; i < classes; ++i )
        {
          input->cumulativeCut( band, 0.0, i * intervalDiff, cut1, cut2, extent, sampleSize );
          entryValues.push_back( cut2 );
        }
        entryValues.push_back( std::numeric_limits<double>::infinity() );
      }
      else
      {
        const double intervalDiff = 1.0 / ( classes - 1 );
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
        const double intervalDiff = ( max - min ) / ( classes );

        for ( int i = 1; i < classes; ++i )
        {
          entryValues.push_back( min + i * intervalDiff );
        }
        entryValues.push_back( std::numeric_limits<double>::infinity() );
      }
      else
      {
        //because the highest value is also an entry, there are (numberOfEntries - 1) intervals
        const double intervalDiff = ( max - min ) / ( classes - 1 );

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
        const int idx = i;
        currentColor.setRgb( colorDiff * idx, 0, 255 - colorDiff * idx );
        entryColors.push_back( currentColor );
      }
    }
    else
    {
      entryColors.reserve( classes );
      for ( int i = 0; i < classes; ++i )
      {
        const int idx = i;
        entryColors.push_back( sourceColorRamp()->color( ( ( double ) idx ) / ( classes - 1 ) ) );
      }
    }
  }

  QList<double>::const_iterator value_it = entryValues.constBegin();
  QVector<QColor>::const_iterator color_it = entryColors.constBegin();

  // calculate a reasonable number of decimals to display
  const double maxabs = std::log10( std::max( std::fabs( max ), std::fabs( min ) ) );
  const int nDecimals = std::round( std::max( 3.0 + maxabs - std::log10( max - min ), maxabs <= 15.0 ? maxabs + 0.49 : 0.0 ) );

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

bool QgsColorRampShader::shade( double value, int *returnRedValue, int *returnGreenValue, int *returnBlueValue, int *returnAlphaValue ) const
{
  if ( mColorRampItemList.isEmpty() )
  {
    return false;
  }
  if ( std::isnan( value ) || std::isinf( value ) )
    return false;

  const int colorRampItemListCount = mColorRampItemList.count();
  const QgsColorRampShader::ColorRampItem *colorRampItems = mColorRampItemList.constData();
  int idx;
  if ( !mLUTInitialized )
  {
    // calculate LUT for faster index recovery
    mLUTFactor = 1.0;
    const double minimumValue = colorRampItems[0].value;
    mLUTOffset = minimumValue + DOUBLE_DIFF_THRESHOLD;
    // Only make lut if at least 3 items, with 2 items the low and high cases handle both
    if ( colorRampItemListCount >= 3 )
    {
      const double rangeValue = colorRampItems[colorRampItemListCount - 2].value - minimumValue;
      if ( rangeValue > 0 )
      {
        const int lutSize = 256; // TODO: test if speed can be increased with a different LUT size
        mLUTFactor = ( lutSize - 0.0000001 ) / rangeValue; // decrease slightly to make sure last LUT category is correct
        idx = 0;
        double val;
        mLUT.reserve( lutSize );
        for ( int i = 0; i < lutSize; i++ )
        {
          val = ( i / mLUTFactor ) + mLUTOffset;
          while ( idx < colorRampItemListCount
                  && colorRampItems[idx].value - DOUBLE_DIFF_THRESHOLD < val )
          {
            idx++;
          }
          mLUT.emplace_back( idx );
        }
      }
    }
    mLUTInitialized = true;
  }

  // overflow indicates that value > maximum value + DOUBLE_DIFF_THRESHOLD
  // that way idx can point to the last valid item
  bool overflow = false;

  // find index of the first ColorRampItem that is equal or higher to theValue
  const int lutIndex = ( value - mLUTOffset ) * mLUTFactor;
  if ( value <= mLUTOffset )
  {
    idx = 0;
  }
  else if ( static_cast< std::size_t>( lutIndex ) >= mLUT.size() )
  {
    idx = colorRampItemListCount - 1;
    if ( colorRampItems[idx].value + DOUBLE_DIFF_THRESHOLD < value )
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
    idx = mLUT[ lutIndex ];

    // check if it's correct and if not increase until correct
    // the LUT is made in such a way the index is always correct or too low, never too high
    while ( idx < colorRampItemListCount && colorRampItems[idx].value + DOUBLE_DIFF_THRESHOLD < value )
    {
      idx++;
    }
    if ( idx >= colorRampItemListCount )
    {
      idx = colorRampItemListCount - 1;
      overflow = true;
    }
  }

  const QgsColorRampShader::ColorRampItem &currentColorRampItem = colorRampItems[idx];

  switch ( colorRampType() )
  {
    case Interpolated:
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

      const QgsColorRampShader::ColorRampItem &previousColorRampItem = colorRampItems[idx - 1];

      const float currentRampRange = currentColorRampItem.value - previousColorRampItem.value;
      const float offsetInRange = value - previousColorRampItem.value;
      const float scale = offsetInRange / currentRampRange;

      const QRgb c1 = previousColorRampItem.color.rgba();
      const QRgb c2 = currentColorRampItem.color.rgba();

      *returnRedValue   = qRed( c1 )   + static_cast< int >( ( qRed( c2 )   - qRed( c1 ) )   * scale );
      *returnGreenValue = qGreen( c1 ) + static_cast< int >( ( qGreen( c2 ) - qGreen( c1 ) ) * scale );
      *returnBlueValue  = qBlue( c1 )  + static_cast< int >( ( qBlue( c2 )  - qBlue( c1 ) )  * scale );
      *returnAlphaValue = qAlpha( c1 ) + static_cast< int >( ( qAlpha( c2 ) - qAlpha( c1 ) ) * scale );
      return true;
    };
    case Discrete:
    {
      // Assign the color of the higher class for every pixel between two class breaks.
      // NOTE: The implementation has always been different than the documentation,
      //       which said lower class before, see https://github.com/qgis/QGIS/issues/22009
      if ( overflow )
      {
        return false;
      }
      *returnRedValue   = currentColorRampItem.color.red();
      *returnGreenValue = currentColorRampItem.color.green();
      *returnBlueValue  = currentColorRampItem.color.blue();
      *returnAlphaValue = currentColorRampItem.color.alpha();
      return true;
    };
    case Exact:
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
  return false;
}

bool QgsColorRampShader::shade( double redValue, double greenValue,
                                double blueValue, double alphaValue,
                                int *returnRedValue, int *returnGreenValue,
                                int *returnBlueValue, int *returnAlphaValue ) const
{
  Q_UNUSED( redValue )
  Q_UNUSED( greenValue )
  Q_UNUSED( blueValue )
  Q_UNUSED( alphaValue )

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

QDomElement QgsColorRampShader::writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement colorRampShaderElem = doc.createElement( QStringLiteral( "colorrampshader" ) );
  colorRampShaderElem.setAttribute( QStringLiteral( "colorRampType" ), colorRampTypeAsQString() );
  colorRampShaderElem.setAttribute( QStringLiteral( "classificationMode" ), classificationMode() );
  colorRampShaderElem.setAttribute( QStringLiteral( "clip" ), clip() );
  colorRampShaderElem.setAttribute( QStringLiteral( "minimumValue" ), mMinimumValue );
  colorRampShaderElem.setAttribute( QStringLiteral( "maximumValue" ), mMaximumValue );
  colorRampShaderElem.setAttribute( QStringLiteral( "labelPrecision" ), mLabelPrecision );

  // save source color ramp
  if ( sourceColorRamp() )
  {
    const QDomElement colorRampElem = QgsSymbolLayerUtils::saveColorRamp( QStringLiteral( "[source]" ), sourceColorRamp(), doc );
    colorRampShaderElem.appendChild( colorRampElem );
  }

  //items
  const QList<QgsColorRampShader::ColorRampItem> itemList = colorRampItemList();
  QList<QgsColorRampShader::ColorRampItem>::const_iterator itemIt = itemList.constBegin();
  for ( ; itemIt != itemList.constEnd(); ++itemIt )
  {
    QDomElement itemElem = doc.createElement( QStringLiteral( "item" ) );
    itemElem.setAttribute( QStringLiteral( "label" ), itemIt->label );
    itemElem.setAttribute( QStringLiteral( "value" ), QgsRasterBlock::printValue( itemIt->value ) );
    itemElem.setAttribute( QStringLiteral( "color" ), itemIt->color.name() );
    itemElem.setAttribute( QStringLiteral( "alpha" ), itemIt->color.alpha() );
    colorRampShaderElem.appendChild( itemElem );
  }

  if ( mLegendSettings )
    mLegendSettings->writeXml( doc, colorRampShaderElem, context );

  return colorRampShaderElem;
}

void QgsColorRampShader::readXml( const QDomElement &colorRampShaderElem, const QgsReadWriteContext &context )
{
  // try to load color ramp (optional)
  QDomElement sourceColorRampElem = colorRampShaderElem.firstChildElement( QStringLiteral( "colorramp" ) );
  if ( !sourceColorRampElem.isNull() && sourceColorRampElem.attribute( QStringLiteral( "name" ) ) == QLatin1String( "[source]" ) )
  {
    setSourceColorRamp( QgsSymbolLayerUtils::loadColorRamp( sourceColorRampElem ) );
  }

  setColorRampType( colorRampShaderElem.attribute( QStringLiteral( "colorRampType" ), QStringLiteral( "INTERPOLATED" ) ) );
  setClassificationMode( static_cast< QgsColorRampShader::ClassificationMode >( colorRampShaderElem.attribute( QStringLiteral( "classificationMode" ), QStringLiteral( "1" ) ).toInt() ) );
  setClip( colorRampShaderElem.attribute( QStringLiteral( "clip" ), QStringLiteral( "0" ) ) == QLatin1String( "1" ) );
  setMinimumValue( colorRampShaderElem.attribute( QStringLiteral( "minimumValue" ) ).toDouble() );
  setMaximumValue( colorRampShaderElem.attribute( QStringLiteral( "maximumValue" ) ).toDouble() );
  setLabelPrecision( colorRampShaderElem.attribute( QStringLiteral( "labelPrecision" ), QStringLiteral( "6" ) ).toDouble() );

  QList<QgsColorRampShader::ColorRampItem> itemList;
  QDomElement itemElem;
  QString itemLabel;
  double itemValue;
  QColor itemColor;

  const QDomNodeList itemNodeList = colorRampShaderElem.elementsByTagName( QStringLiteral( "item" ) );
  itemList.reserve( itemNodeList.size() );
  for ( int i = 0; i < itemNodeList.size(); ++i )
  {
    itemElem = itemNodeList.at( i ).toElement();
    itemValue = itemElem.attribute( QStringLiteral( "value" ) ).toDouble();
    itemLabel = itemElem.attribute( QStringLiteral( "label" ) );
    itemColor.setNamedColor( itemElem.attribute( QStringLiteral( "color" ) ) );
    itemColor.setAlpha( itemElem.attribute( QStringLiteral( "alpha" ), QStringLiteral( "255" ) ).toInt() );

    itemList.push_back( QgsColorRampShader::ColorRampItem( itemValue, itemColor, itemLabel ) );
  }
  setColorRampItemList( itemList );

  if ( !mLegendSettings )
    mLegendSettings = std::make_unique< QgsColorRampLegendNodeSettings >();

  mLegendSettings->readXml( colorRampShaderElem, context );
}

const QgsColorRampLegendNodeSettings *QgsColorRampShader::legendSettings() const
{
  return mLegendSettings.get();
}

void QgsColorRampShader::setLegendSettings( QgsColorRampLegendNodeSettings *settings )
{
  if ( settings == mLegendSettings.get() )
    return;
  mLegendSettings.reset( settings );
}
