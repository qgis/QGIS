/***************************************************************************
                         qgsrasterlayerelevationproperties.cpp
                         ---------------
    begin                : February 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrasterlayerelevationproperties.h"
#include "qgsrasterlayer.h"
#include "qgslinesymbol.h"
#include "qgsfillsymbol.h"
#include "qgssymbollayerutils.h"
#include "qgslinesymbollayer.h"
#include "qgsfillsymbollayer.h"
#include "qgsapplication.h"
#include "qgscolorschemeregistry.h"
#include "qgsexpressioncontextutils.h"

QgsRasterLayerElevationProperties::QgsRasterLayerElevationProperties( QObject *parent )
  : QgsMapLayerElevationProperties( parent )
{
  const QColor color = QgsApplication::colorSchemeRegistry()->fetchRandomStyleColor();
  setDefaultProfileLineSymbol( color );
  setDefaultProfileFillSymbol( color );
}

QgsRasterLayerElevationProperties::~QgsRasterLayerElevationProperties() = default;

bool QgsRasterLayerElevationProperties::hasElevation() const
{
  return mEnabled;
}

QDomElement QgsRasterLayerElevationProperties::writeXml( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext &context )
{
  QDomElement element = document.createElement( QStringLiteral( "elevation" ) );
  element.setAttribute( QStringLiteral( "enabled" ), mEnabled ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  element.setAttribute( QStringLiteral( "mode" ), qgsEnumValueToKey( mMode ) );
  element.setAttribute( QStringLiteral( "symbology" ), qgsEnumValueToKey( mSymbology ) );
  if ( !std::isnan( mElevationLimit ) )
    element.setAttribute( QStringLiteral( "elevationLimit" ), qgsDoubleToString( mElevationLimit ) );

  writeCommonProperties( element, document, context );

  switch ( mMode )
  {
    case Qgis::RasterElevationMode::FixedElevationRange:
      element.setAttribute( QStringLiteral( "lower" ), qgsDoubleToString( mFixedRange.lower() ) );
      element.setAttribute( QStringLiteral( "upper" ), qgsDoubleToString( mFixedRange.upper() ) );
      element.setAttribute( QStringLiteral( "includeLower" ), mFixedRange.includeLower() ? "1" : "0" );
      element.setAttribute( QStringLiteral( "includeUpper" ), mFixedRange.includeUpper() ? "1" : "0" );
      break;

    case Qgis::RasterElevationMode::FixedRangePerBand:
    {
      QDomElement ranges = document.createElement( QStringLiteral( "ranges" ) );
      for ( auto it = mRangePerBand.constBegin(); it != mRangePerBand.constEnd(); ++it )
      {
        QDomElement range = document.createElement( QStringLiteral( "range" ) );
        range.setAttribute( QStringLiteral( "band" ), it.key() );
        range.setAttribute( QStringLiteral( "lower" ), qgsDoubleToString( it.value().lower() ) );
        range.setAttribute( QStringLiteral( "upper" ), qgsDoubleToString( it.value().upper() ) );
        range.setAttribute( QStringLiteral( "includeLower" ), it.value().includeLower() ? "1" : "0" );
        range.setAttribute( QStringLiteral( "includeUpper" ), it.value().includeUpper() ? "1" : "0" );
        ranges.appendChild( range );
      }
      element.appendChild( ranges );
      break;
    }

    case Qgis::RasterElevationMode::DynamicRangePerBand:
      break;

    case Qgis::RasterElevationMode::RepresentsElevationSurface:
      element.setAttribute( QStringLiteral( "band" ), mBandNumber );
      break;
  }

  QDomElement profileLineSymbolElement = document.createElement( QStringLiteral( "profileLineSymbol" ) );
  profileLineSymbolElement.appendChild( QgsSymbolLayerUtils::saveSymbol( QString(), mProfileLineSymbol.get(), document, context ) );
  element.appendChild( profileLineSymbolElement );

  QDomElement profileFillSymbolElement = document.createElement( QStringLiteral( "profileFillSymbol" ) );
  profileFillSymbolElement.appendChild( QgsSymbolLayerUtils::saveSymbol( QString(), mProfileFillSymbol.get(), document, context ) );
  element.appendChild( profileFillSymbolElement );

  parentElement.appendChild( element );
  return element;
}

bool QgsRasterLayerElevationProperties::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  const QDomElement elevationElement = element.firstChildElement( QStringLiteral( "elevation" ) ).toElement();
  mEnabled = elevationElement.attribute( QStringLiteral( "enabled" ), QStringLiteral( "0" ) ).toInt();
  mMode = qgsEnumKeyToValue( elevationElement.attribute( QStringLiteral( "mode" ) ), Qgis::RasterElevationMode::RepresentsElevationSurface );
  mSymbology = qgsEnumKeyToValue( elevationElement.attribute( QStringLiteral( "symbology" ) ), Qgis::ProfileSurfaceSymbology::Line );
  if ( elevationElement.hasAttribute( QStringLiteral( "elevationLimit" ) ) )
    mElevationLimit = elevationElement.attribute( QStringLiteral( "elevationLimit" ) ).toDouble();
  else
    mElevationLimit = std::numeric_limits< double >::quiet_NaN();

  readCommonProperties( elevationElement, context );

  switch ( mMode )
  {
    case Qgis::RasterElevationMode::FixedElevationRange:
    {
      const double lower = elevationElement.attribute( QStringLiteral( "lower" ) ).toDouble();
      const double upper = elevationElement.attribute( QStringLiteral( "upper" ) ).toDouble();
      const bool includeLower = elevationElement.attribute( QStringLiteral( "includeLower" ) ).toInt();
      const bool includeUpper = elevationElement.attribute( QStringLiteral( "includeUpper" ) ).toInt();
      mFixedRange = QgsDoubleRange( lower, upper, includeLower, includeUpper );
      break;
    }

    case Qgis::RasterElevationMode::FixedRangePerBand:
    {
      mRangePerBand.clear();

      const QDomNodeList ranges = elevationElement.firstChildElement( QStringLiteral( "ranges" ) ).childNodes();
      for ( int i = 0; i < ranges.size(); ++i )
      {
        const QDomElement rangeElement = ranges.at( i ).toElement();
        const int band = rangeElement.attribute( QStringLiteral( "band" ) ).toInt();
        const double lower = rangeElement.attribute( QStringLiteral( "lower" ) ).toDouble();
        const double upper = rangeElement.attribute( QStringLiteral( "upper" ) ).toDouble();
        const bool includeLower = rangeElement.attribute( QStringLiteral( "includeLower" ) ).toInt();
        const bool includeUpper = rangeElement.attribute( QStringLiteral( "includeUpper" ) ).toInt();
        mRangePerBand.insert( band, QgsDoubleRange( lower, upper, includeLower, includeUpper ) );
      }
      break;
    }

    case Qgis::RasterElevationMode::DynamicRangePerBand:
      break;

    case Qgis::RasterElevationMode::RepresentsElevationSurface:
      mBandNumber = elevationElement.attribute( QStringLiteral( "band" ), QStringLiteral( "1" ) ).toInt();
      break;
  }

  const QColor defaultColor = QgsApplication::colorSchemeRegistry()->fetchRandomStyleColor();

  const QDomElement profileLineSymbolElement = elevationElement.firstChildElement( QStringLiteral( "profileLineSymbol" ) ).firstChildElement( QStringLiteral( "symbol" ) );
  mProfileLineSymbol.reset( QgsSymbolLayerUtils::loadSymbol< QgsLineSymbol >( profileLineSymbolElement, context ) );
  if ( !mProfileLineSymbol )
    setDefaultProfileLineSymbol( defaultColor );

  const QDomElement profileFillSymbolElement = elevationElement.firstChildElement( QStringLiteral( "profileFillSymbol" ) ).firstChildElement( QStringLiteral( "symbol" ) );
  mProfileFillSymbol.reset( QgsSymbolLayerUtils::loadSymbol< QgsFillSymbol >( profileFillSymbolElement, context ) );
  if ( !mProfileFillSymbol )
    setDefaultProfileFillSymbol( defaultColor );

  return true;
}

QgsRasterLayerElevationProperties *QgsRasterLayerElevationProperties::clone() const
{
  std::unique_ptr< QgsRasterLayerElevationProperties > res = std::make_unique< QgsRasterLayerElevationProperties >( nullptr );
  res->setEnabled( mEnabled );
  res->setMode( mMode );
  res->setProfileLineSymbol( mProfileLineSymbol->clone() );
  res->setProfileFillSymbol( mProfileFillSymbol->clone() );
  res->setProfileSymbology( mSymbology );
  res->setElevationLimit( mElevationLimit );
  res->setBandNumber( mBandNumber );
  res->setFixedRange( mFixedRange );
  res->setFixedRangePerBand( mRangePerBand );
  res->copyCommonProperties( this );
  return res.release();
}

QString QgsRasterLayerElevationProperties::htmlSummary() const
{
  QStringList properties;
  switch ( mMode )
  {
    case Qgis::RasterElevationMode::FixedElevationRange:
      properties << tr( "Elevation range: %1 to %2" ).arg( mFixedRange.lower() ).arg( mFixedRange.upper() );
      break;

    case Qgis::RasterElevationMode::FixedRangePerBand:
    {
      for ( auto it = mRangePerBand.constBegin(); it != mRangePerBand.constEnd(); ++it )
      {
        properties << tr( "Elevation for band %1: %2 to %3" ).arg( it.key() ).arg( it.value().lower() ).arg( it.value().upper() );
      }
      break;
    }

    case Qgis::RasterElevationMode::DynamicRangePerBand:
      break;

    case Qgis::RasterElevationMode::RepresentsElevationSurface:
      properties << tr( "Elevation band: %1" ).arg( mBandNumber );
      properties << tr( "Scale: %1" ).arg( mZScale );
      properties << tr( "Offset: %1" ).arg( mZOffset );
      break;
  }

  return QStringLiteral( "<li>%1</li>" ).arg( properties.join( QLatin1String( "</li><li>" ) ) );
}

bool QgsRasterLayerElevationProperties::isVisibleInZRange( const QgsDoubleRange &range, QgsMapLayer *layer ) const
{
  switch ( mMode )
  {
    case Qgis::RasterElevationMode::FixedElevationRange:
      return mFixedRange.overlaps( range );

    case Qgis::RasterElevationMode::FixedRangePerBand:
    {
      for ( auto it = mRangePerBand.constBegin(); it != mRangePerBand.constEnd(); ++it )
      {
        if ( it.value().overlaps( range ) )
          return true;
      }
      return false;
    }

    case Qgis::RasterElevationMode::DynamicRangePerBand:
    {
      if ( QgsRasterLayer *rl = qobject_cast< QgsRasterLayer * >( layer ) )
      {
        QgsExpressionContext context;
        context.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( layer ) );
        QgsExpressionContextScope *bandScope = new QgsExpressionContextScope();
        bandScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "band" ), 1, true, false, tr( "Band number" ) ) );
        bandScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "band_name" ), rl->dataProvider()->displayBandName( 1 ), true, false, tr( "Band name" ) ) );
        bandScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "band_description" ), rl->dataProvider()->bandDescription( 1 ), true, false, tr( "Band description" ) ) );
        context.appendScope( bandScope );

        QgsProperty lowerProperty = mDataDefinedProperties.property( Property::RasterPerBandLowerElevation );
        QgsProperty upperProperty = mDataDefinedProperties.property( Property::RasterPerBandUpperElevation );
        lowerProperty.prepare( context );
        upperProperty.prepare( context );
        for ( int band = 1; band <= rl->bandCount(); ++band )
        {
          bandScope->setVariable( QStringLiteral( "band" ), band );
          bandScope->setVariable( QStringLiteral( "band_name" ), rl->dataProvider()->displayBandName( band ) );
          bandScope->setVariable( QStringLiteral( "band_description" ), rl->dataProvider()->bandDescription( band ) );

          bool ok = false;
          const double lower = lowerProperty.valueAsDouble( context, 0, &ok );
          if ( !ok )
            continue;
          const double upper = upperProperty.valueAsDouble( context, 0, &ok );
          if ( !ok )
            continue;

          if ( QgsDoubleRange( lower, upper ).overlaps( range ) )
            return true;
        }
      }
      return false;
    }

    case Qgis::RasterElevationMode::RepresentsElevationSurface:
      // TODO -- test actual raster z range
      return true;
  }
  BUILTIN_UNREACHABLE
}

QgsDoubleRange QgsRasterLayerElevationProperties::calculateZRange( QgsMapLayer *layer ) const
{
  switch ( mMode )
  {
    case Qgis::RasterElevationMode::FixedElevationRange:
      return mFixedRange;

    case Qgis::RasterElevationMode::FixedRangePerBand:
    {
      double lower = std::numeric_limits< double >::max();
      double upper = std::numeric_limits< double >::min();
      bool includeLower = true;
      bool includeUpper = true;
      for ( auto it = mRangePerBand.constBegin(); it != mRangePerBand.constEnd(); ++it )
      {
        if ( it.value().lower() < lower )
        {
          lower = it.value().lower();
          includeLower = it.value().includeLower();
        }
        else if ( !includeLower && it.value().lower() == lower && it.value().includeLower() )
        {
          includeLower = true;
        }
        if ( it.value().upper() > upper )
        {
          upper = it.value().upper();
          includeUpper = it.value().includeUpper();
        }
        else if ( !includeUpper && it.value().upper() == upper && it.value().includeUpper() )
        {
          includeUpper = true;
        }
      }
      return QgsDoubleRange( lower, upper, includeLower, includeUpper );
    }

    case Qgis::RasterElevationMode::DynamicRangePerBand:
    {
      if ( QgsRasterLayer *rl = qobject_cast< QgsRasterLayer * >( layer ) )
      {
        QgsExpressionContext context;
        context.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( layer ) );
        QgsExpressionContextScope *bandScope = new QgsExpressionContextScope();
        bandScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "band" ), 1, true, false, tr( "Band number" ) ) );
        bandScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "band_name" ), rl->dataProvider()->displayBandName( 1 ), true, false, tr( "Band name" ) ) );
        bandScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "band_description" ), rl->dataProvider()->bandDescription( 1 ), true, false, tr( "Band description" ) ) );
        context.appendScope( bandScope );

        QgsProperty lowerProperty = mDataDefinedProperties.property( Property::RasterPerBandLowerElevation );
        QgsProperty upperProperty = mDataDefinedProperties.property( Property::RasterPerBandUpperElevation );
        lowerProperty.prepare( context );
        upperProperty.prepare( context );
        double minLower = std::numeric_limits<double>::max();
        double maxUpper = std::numeric_limits<double>::lowest();
        for ( int band = 1; band <= rl->bandCount(); ++band )
        {
          bandScope->setVariable( QStringLiteral( "band" ), band );
          bandScope->setVariable( QStringLiteral( "band_name" ), rl->dataProvider()->displayBandName( band ) );
          bandScope->setVariable( QStringLiteral( "band_description" ), rl->dataProvider()->bandDescription( band ) );

          bool ok = false;
          const double lower = lowerProperty.valueAsDouble( context, 0, &ok );
          if ( !ok )
            continue;
          const double upper = upperProperty.valueAsDouble( context, 0, &ok );
          if ( !ok )
            continue;

          minLower = std::min( minLower, lower );
          maxUpper = std::max( maxUpper, upper );
        }
        return ( minLower == std::numeric_limits<double>::max() && maxUpper == std::numeric_limits<double>::lowest() ) ? QgsDoubleRange() : QgsDoubleRange( minLower, maxUpper );
      }
      return QgsDoubleRange();
    }

    case Qgis::RasterElevationMode::RepresentsElevationSurface:
      // TODO -- determine actual z range from raster statistics
      return QgsDoubleRange();
  }
  BUILTIN_UNREACHABLE
}

bool QgsRasterLayerElevationProperties::showByDefaultInElevationProfilePlots() const
{
  return mEnabled;
}

QgsMapLayerElevationProperties::Flags QgsRasterLayerElevationProperties::flags() const
{
  if ( mEnabled )
  {
    switch ( mMode )
    {
      case Qgis::RasterElevationMode::FixedElevationRange:
        return QgsMapLayerElevationProperties::Flag::FlagDontInvalidateCachedRendersWhenRangeChanges;

      case Qgis::RasterElevationMode::RepresentsElevationSurface:
      case Qgis::RasterElevationMode::FixedRangePerBand:
      case Qgis::RasterElevationMode::DynamicRangePerBand:
        break;
    }
  }
  return QgsMapLayerElevationProperties::Flags();
}

void QgsRasterLayerElevationProperties::setEnabled( bool enabled )
{
  if ( enabled == mEnabled )
    return;

  mEnabled = enabled;
  emit changed();
  emit profileGenerationPropertyChanged();
}

Qgis::RasterElevationMode QgsRasterLayerElevationProperties::mode() const
{
  return mMode;
}

void QgsRasterLayerElevationProperties::setMode( Qgis::RasterElevationMode mode )
{
  if ( mMode == mode )
    return;

  mMode = mode;
  emit changed();
}

void QgsRasterLayerElevationProperties::setBandNumber( int band )
{
  if ( mBandNumber == band )
    return;

  mBandNumber = band;
  emit changed();
  emit profileGenerationPropertyChanged();
}

QgsDoubleRange QgsRasterLayerElevationProperties::elevationRangeForPixelValue( QgsRasterLayer *layer, int band, double pixelValue ) const
{
  if ( !mEnabled || std::isnan( pixelValue ) )
    return QgsDoubleRange();

  switch ( mMode )
  {
    case Qgis::RasterElevationMode::FixedElevationRange:
      return mFixedRange;

    case Qgis::RasterElevationMode::FixedRangePerBand:
    {
      auto it = mRangePerBand.constFind( band );
      if ( it != mRangePerBand.constEnd() )
        return it.value();
      return QgsDoubleRange();
    }

    case Qgis::RasterElevationMode::DynamicRangePerBand:
    {
      if ( layer && band > 0 && band <= layer->bandCount() )
      {
        QgsExpressionContext context;
        context.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( layer ) );
        QgsExpressionContextScope *bandScope = new QgsExpressionContextScope();
        bandScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "band" ), band, true, false, tr( "Band number" ) ) );
        bandScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "band_name" ), layer->dataProvider()->displayBandName( band ), true, false, tr( "Band name" ) ) );
        bandScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "band_description" ), layer->dataProvider()->bandDescription( band ), true, false, tr( "Band description" ) ) );
        context.appendScope( bandScope );

        QgsProperty lowerProperty = mDataDefinedProperties.property( Property::RasterPerBandLowerElevation );
        QgsProperty upperProperty = mDataDefinedProperties.property( Property::RasterPerBandUpperElevation );
        lowerProperty.prepare( context );
        upperProperty.prepare( context );

        bool ok = false;
        const double lower = lowerProperty.valueAsDouble( context, 0, &ok );
        if ( !ok )
          return QgsDoubleRange();
        const double upper = upperProperty.valueAsDouble( context, 0, &ok );
        if ( !ok )
          return QgsDoubleRange();

        return QgsDoubleRange( lower, upper );
      }

      return QgsDoubleRange();
    }

    case Qgis::RasterElevationMode::RepresentsElevationSurface:
    {
      if ( band != mBandNumber )
        return QgsDoubleRange();

      const double z = pixelValue * mZScale + mZOffset;
      return QgsDoubleRange( z, z );
    }
  }
  BUILTIN_UNREACHABLE
}

int QgsRasterLayerElevationProperties::bandForElevationRange( QgsRasterLayer *layer, const QgsDoubleRange &range ) const
{
  switch ( mMode )
  {
    case Qgis::RasterElevationMode::FixedElevationRange:
    case Qgis::RasterElevationMode::RepresentsElevationSurface:
      return -1;

    case Qgis::RasterElevationMode::FixedRangePerBand:
    {
      // find the top-most band which matches the map range
      int currentMatchingBand = -1;
      QgsDoubleRange currentMatchingRange;
      for ( auto it = mRangePerBand.constBegin(); it != mRangePerBand.constEnd(); ++it )
      {
        if ( it.value().overlaps( range ) )
        {
          if ( currentMatchingRange.isInfinite()
               || ( it.value().includeUpper() && it.value().upper() >= currentMatchingRange.upper() )
               || ( !currentMatchingRange.includeUpper() && it.value().upper() >= currentMatchingRange.upper() ) )
          {
            currentMatchingBand = it.key();
            currentMatchingRange = it.value();
          }
        }
      }
      return currentMatchingBand;
    }

    case Qgis::RasterElevationMode::DynamicRangePerBand:
    {
      if ( layer )
      {
        QgsExpressionContext context;
        context.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( layer ) );
        QgsExpressionContextScope *bandScope = new QgsExpressionContextScope();
        context.appendScope( bandScope );

        QgsProperty lowerProperty = mDataDefinedProperties.property( Property::RasterPerBandLowerElevation );
        QgsProperty upperProperty = mDataDefinedProperties.property( Property::RasterPerBandUpperElevation );
        lowerProperty.prepare( context );
        upperProperty.prepare( context );

        int currentMatchingBand = -1;
        QgsDoubleRange currentMatchingRange;

        for ( int band = 1; band <= layer->bandCount(); ++band )
        {
          bandScope->setVariable( QStringLiteral( "band" ), band );
          bandScope->setVariable( QStringLiteral( "band_name" ), layer->dataProvider()->displayBandName( band ) );
          bandScope->setVariable( QStringLiteral( "band_description" ), layer->dataProvider()->bandDescription( band ) );

          bool ok = false;
          const double lower = lowerProperty.valueAsDouble( context, 0, &ok );
          if ( !ok )
            continue;
          const double upper = upperProperty.valueAsDouble( context, 0, &ok );
          if ( !ok )
            continue;

          const QgsDoubleRange bandRange = QgsDoubleRange( lower, upper );
          if ( bandRange.overlaps( range ) )
          {
            if ( currentMatchingRange.isInfinite()
                 || ( bandRange.includeUpper() && bandRange.upper() >= currentMatchingRange.upper() )
                 || ( !currentMatchingRange.includeUpper() && bandRange.upper() >= currentMatchingRange.upper() ) )
            {
              currentMatchingBand = band;
              currentMatchingRange = bandRange;
            }
          }
        }
        return currentMatchingBand;
      }
      return -1;
    }
  }
  BUILTIN_UNREACHABLE
}

QgsLineSymbol *QgsRasterLayerElevationProperties::profileLineSymbol() const
{
  return mProfileLineSymbol.get();
}

void QgsRasterLayerElevationProperties::setProfileLineSymbol( QgsLineSymbol *symbol )
{
  mProfileLineSymbol.reset( symbol );
  emit changed();
  emit profileRenderingPropertyChanged();
}

QgsFillSymbol *QgsRasterLayerElevationProperties::profileFillSymbol() const
{
  return mProfileFillSymbol.get();
}

void QgsRasterLayerElevationProperties::setProfileFillSymbol( QgsFillSymbol *symbol )
{
  mProfileFillSymbol.reset( symbol );
  emit changed();
  emit profileRenderingPropertyChanged();
}

void QgsRasterLayerElevationProperties::setProfileSymbology( Qgis::ProfileSurfaceSymbology symbology )
{
  if ( mSymbology == symbology )
    return;

  mSymbology = symbology;
  emit changed();
  emit profileRenderingPropertyChanged();
}

double QgsRasterLayerElevationProperties::elevationLimit() const
{
  return mElevationLimit;
}

void QgsRasterLayerElevationProperties::setElevationLimit( double limit )
{
  if ( qgsDoubleNear( mElevationLimit, limit ) )
    return;

  mElevationLimit = limit;
  emit changed();
  emit profileRenderingPropertyChanged();
}

bool QgsRasterLayerElevationProperties::layerLooksLikeDem( QgsRasterLayer *layer )
{
  // multiple bands => unlikely to be a DEM
  if ( layer->bandCount() > 1 )
    return false;

  // raster attribute table => unlikely to be a DEM
  if ( layer->attributeTable( 1 ) )
    return false;

  if ( QgsRasterDataProvider *dataProvider = layer->dataProvider() )
  {
    // filter out data types which aren't likely to be DEMs
    switch ( dataProvider->dataType( 1 ) )
    {
      case Qgis::DataType::Byte:
      case Qgis::DataType::UnknownDataType:
      case Qgis::DataType::CInt16:
      case Qgis::DataType::CInt32:
      case Qgis::DataType::CFloat32:
      case Qgis::DataType::CFloat64:
      case Qgis::DataType::ARGB32:
      case Qgis::DataType::ARGB32_Premultiplied:
        return false;

      case Qgis::DataType::Int8:
      case Qgis::DataType::UInt16:
      case Qgis::DataType::Int16:
      case Qgis::DataType::UInt32:
      case Qgis::DataType::Int32:
      case Qgis::DataType::Float32:
      case Qgis::DataType::Float64:
        break;
    }
  }

  // Check the layer's name for DEM-ish hints.
  // See discussion at https://github.com/qgis/QGIS/pull/30245 - this list must NOT be translated,
  // but adding hardcoded localized variants of the strings is encouraged.
  static const QStringList sPartialCandidates{ QStringLiteral( "dem" ),
      QStringLiteral( "dtm" ),
      QStringLiteral( "dsm" ),
      QStringLiteral( "height" ),
      QStringLiteral( "elev" ),
      QStringLiteral( "srtm" ),
      // French hints
      QStringLiteral( "mne" ),
      QStringLiteral( "mnt" ),
      QStringLiteral( "mns" ),
      QStringLiteral( "rge" ),
      QStringLiteral( "alti" ),
      // German hints
      QStringLiteral( "dhm" ),
      QStringLiteral( "dgm" ),
      QStringLiteral( "dom" ),
      QStringLiteral( "Höhe" ),
      QStringLiteral( "Hoehe" ) };
  const QString layerName = layer->name();
  for ( const QString &candidate : sPartialCandidates )
  {
    if ( layerName.contains( candidate, Qt::CaseInsensitive ) )
      return true;
  }

  // these candidates must occur with word boundaries (we don't want to find "aster" in "raster"!)
  static const QStringList sWordCandidates{ QStringLiteral( "aster" ) };
  for ( const QString &candidate : sWordCandidates )
  {
    const thread_local QRegularExpression re( QStringLiteral( "\\b%1\\b" ).arg( candidate ) );
    if ( re.match( layerName, Qt::CaseInsensitive ).hasMatch() )
      return true;
  }

  return false;
}

void QgsRasterLayerElevationProperties::setDefaultProfileLineSymbol( const QColor &color )
{
  std::unique_ptr< QgsSimpleLineSymbolLayer > profileLineLayer = std::make_unique< QgsSimpleLineSymbolLayer >( color, 0.6 );
  mProfileLineSymbol = std::make_unique< QgsLineSymbol>( QgsSymbolLayerList( { profileLineLayer.release() } ) );
}

void QgsRasterLayerElevationProperties::setDefaultProfileFillSymbol( const QColor &color )
{
  std::unique_ptr< QgsSimpleFillSymbolLayer > profileFillLayer = std::make_unique< QgsSimpleFillSymbolLayer >( color );
  profileFillLayer->setStrokeStyle( Qt::NoPen );
  mProfileFillSymbol = std::make_unique< QgsFillSymbol>( QgsSymbolLayerList( { profileFillLayer.release() } ) );
}

QMap<int, QgsDoubleRange> QgsRasterLayerElevationProperties::fixedRangePerBand() const
{
  return mRangePerBand;
}

void QgsRasterLayerElevationProperties::setFixedRangePerBand( const QMap<int, QgsDoubleRange> &ranges )
{
  if ( ranges == mRangePerBand )
    return;

  mRangePerBand = ranges;
  emit changed();
}

QgsDoubleRange QgsRasterLayerElevationProperties::fixedRange() const
{
  return mFixedRange;
}

void QgsRasterLayerElevationProperties::setFixedRange( const QgsDoubleRange &range )
{
  if ( range == mFixedRange )
    return;

  mFixedRange = range;
  emit changed();
}
