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

#include "qgsapplication.h"
#include "qgscolorschemeregistry.h"
#include "qgsexpressioncontextutils.h"
#include "qgsfillsymbol.h"
#include "qgsfillsymbollayer.h"
#include "qgslinesymbol.h"
#include "qgslinesymbollayer.h"
#include "qgsrasterlayer.h"
#include "qgssymbollayerutils.h"

#include "moc_qgsrasterlayerelevationproperties.cpp"

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
  QDomElement element = document.createElement( u"elevation"_s );
  element.setAttribute( u"enabled"_s, mEnabled ? u"1"_s : u"0"_s );
  element.setAttribute( u"mode"_s, qgsEnumValueToKey( mMode ) );
  element.setAttribute( u"symbology"_s, qgsEnumValueToKey( mSymbology ) );
  if ( !std::isnan( mElevationLimit ) )
    element.setAttribute( u"elevationLimit"_s, qgsDoubleToString( mElevationLimit ) );

  writeCommonProperties( element, document, context );

  switch ( mMode )
  {
    case Qgis::RasterElevationMode::FixedElevationRange:
      element.setAttribute( u"lower"_s, qgsDoubleToString( mFixedRange.lower() ) );
      element.setAttribute( u"upper"_s, qgsDoubleToString( mFixedRange.upper() ) );
      element.setAttribute( u"includeLower"_s, mFixedRange.includeLower() ? "1" : "0" );
      element.setAttribute( u"includeUpper"_s, mFixedRange.includeUpper() ? "1" : "0" );
      break;

    case Qgis::RasterElevationMode::FixedRangePerBand:
    {
      QDomElement ranges = document.createElement( u"ranges"_s );
      for ( auto it = mRangePerBand.constBegin(); it != mRangePerBand.constEnd(); ++it )
      {
        QDomElement range = document.createElement( u"range"_s );
        range.setAttribute( u"band"_s, it.key() );
        range.setAttribute( u"lower"_s, qgsDoubleToString( it.value().lower() ) );
        range.setAttribute( u"upper"_s, qgsDoubleToString( it.value().upper() ) );
        range.setAttribute( u"includeLower"_s, it.value().includeLower() ? "1" : "0" );
        range.setAttribute( u"includeUpper"_s, it.value().includeUpper() ? "1" : "0" );
        ranges.appendChild( range );
      }
      element.appendChild( ranges );
      break;
    }

    case Qgis::RasterElevationMode::DynamicRangePerBand:
      break;

    case Qgis::RasterElevationMode::RepresentsElevationSurface:
      element.setAttribute( u"band"_s, mBandNumber );
      break;
  }

  QDomElement profileLineSymbolElement = document.createElement( u"profileLineSymbol"_s );
  profileLineSymbolElement.appendChild( QgsSymbolLayerUtils::saveSymbol( QString(), mProfileLineSymbol.get(), document, context ) );
  element.appendChild( profileLineSymbolElement );

  QDomElement profileFillSymbolElement = document.createElement( u"profileFillSymbol"_s );
  profileFillSymbolElement.appendChild( QgsSymbolLayerUtils::saveSymbol( QString(), mProfileFillSymbol.get(), document, context ) );
  element.appendChild( profileFillSymbolElement );

  parentElement.appendChild( element );
  return element;
}

bool QgsRasterLayerElevationProperties::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  const QDomElement elevationElement = element.firstChildElement( u"elevation"_s ).toElement();
  mEnabled = elevationElement.attribute( u"enabled"_s, u"0"_s ).toInt();
  mMode = qgsEnumKeyToValue( elevationElement.attribute( u"mode"_s ), Qgis::RasterElevationMode::RepresentsElevationSurface );
  mSymbology = qgsEnumKeyToValue( elevationElement.attribute( u"symbology"_s ), Qgis::ProfileSurfaceSymbology::Line );
  if ( elevationElement.hasAttribute( u"elevationLimit"_s ) )
    mElevationLimit = elevationElement.attribute( u"elevationLimit"_s ).toDouble();
  else
    mElevationLimit = std::numeric_limits< double >::quiet_NaN();

  readCommonProperties( elevationElement, context );

  switch ( mMode )
  {
    case Qgis::RasterElevationMode::FixedElevationRange:
    {
      const double lower = elevationElement.attribute( u"lower"_s ).toDouble();
      const double upper = elevationElement.attribute( u"upper"_s ).toDouble();
      const bool includeLower = elevationElement.attribute( u"includeLower"_s ).toInt();
      const bool includeUpper = elevationElement.attribute( u"includeUpper"_s ).toInt();
      mFixedRange = QgsDoubleRange( lower, upper, includeLower, includeUpper );
      break;
    }

    case Qgis::RasterElevationMode::FixedRangePerBand:
    {
      mRangePerBand.clear();

      const QDomNodeList ranges = elevationElement.firstChildElement( u"ranges"_s ).childNodes();
      for ( int i = 0; i < ranges.size(); ++i )
      {
        const QDomElement rangeElement = ranges.at( i ).toElement();
        const int band = rangeElement.attribute( u"band"_s ).toInt();
        const double lower = rangeElement.attribute( u"lower"_s ).toDouble();
        const double upper = rangeElement.attribute( u"upper"_s ).toDouble();
        const bool includeLower = rangeElement.attribute( u"includeLower"_s ).toInt();
        const bool includeUpper = rangeElement.attribute( u"includeUpper"_s ).toInt();
        mRangePerBand.insert( band, QgsDoubleRange( lower, upper, includeLower, includeUpper ) );
      }
      break;
    }

    case Qgis::RasterElevationMode::DynamicRangePerBand:
      break;

    case Qgis::RasterElevationMode::RepresentsElevationSurface:
      mBandNumber = elevationElement.attribute( u"band"_s, u"1"_s ).toInt();
      break;
  }

  const QColor defaultColor = QgsApplication::colorSchemeRegistry()->fetchRandomStyleColor();

  const QDomElement profileLineSymbolElement = elevationElement.firstChildElement( u"profileLineSymbol"_s ).firstChildElement( u"symbol"_s );
  mProfileLineSymbol = QgsSymbolLayerUtils::loadSymbol< QgsLineSymbol >( profileLineSymbolElement, context );
  if ( !mProfileLineSymbol )
    setDefaultProfileLineSymbol( defaultColor );

  const QDomElement profileFillSymbolElement = elevationElement.firstChildElement( u"profileFillSymbol"_s ).firstChildElement( u"symbol"_s );
  mProfileFillSymbol = QgsSymbolLayerUtils::loadSymbol< QgsFillSymbol >( profileFillSymbolElement, context );
  if ( !mProfileFillSymbol )
    setDefaultProfileFillSymbol( defaultColor );

  return true;
}

QgsRasterLayerElevationProperties *QgsRasterLayerElevationProperties::clone() const
{
  auto res = std::make_unique< QgsRasterLayerElevationProperties >( nullptr );
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

  return u"<li>%1</li>"_s.arg( properties.join( "</li><li>"_L1 ) );
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
        bandScope->addVariable( QgsExpressionContextScope::StaticVariable( u"band"_s, 1, true, false, tr( "Band number" ) ) );
        bandScope->addVariable( QgsExpressionContextScope::StaticVariable( u"band_name"_s, rl->dataProvider()->displayBandName( 1 ), true, false, tr( "Band name" ) ) );
        bandScope->addVariable( QgsExpressionContextScope::StaticVariable( u"band_description"_s, rl->dataProvider()->bandDescription( 1 ), true, false, tr( "Band description" ) ) );
        context.appendScope( bandScope );

        QgsProperty lowerProperty = mDataDefinedProperties.property( Property::RasterPerBandLowerElevation );
        QgsProperty upperProperty = mDataDefinedProperties.property( Property::RasterPerBandUpperElevation );
        lowerProperty.prepare( context );
        upperProperty.prepare( context );
        for ( int band = 1; band <= rl->bandCount(); ++band )
        {
          bandScope->setVariable( u"band"_s, band );
          bandScope->setVariable( u"band_name"_s, rl->dataProvider()->displayBandName( band ) );
          bandScope->setVariable( u"band_description"_s, rl->dataProvider()->bandDescription( band ) );

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
        bandScope->addVariable( QgsExpressionContextScope::StaticVariable( u"band"_s, 1, true, false, tr( "Band number" ) ) );
        bandScope->addVariable( QgsExpressionContextScope::StaticVariable( u"band_name"_s, rl->dataProvider()->displayBandName( 1 ), true, false, tr( "Band name" ) ) );
        bandScope->addVariable( QgsExpressionContextScope::StaticVariable( u"band_description"_s, rl->dataProvider()->bandDescription( 1 ), true, false, tr( "Band description" ) ) );
        context.appendScope( bandScope );

        QgsProperty lowerProperty = mDataDefinedProperties.property( Property::RasterPerBandLowerElevation );
        QgsProperty upperProperty = mDataDefinedProperties.property( Property::RasterPerBandUpperElevation );
        lowerProperty.prepare( context );
        upperProperty.prepare( context );
        double minLower = std::numeric_limits<double>::max();
        double maxUpper = std::numeric_limits<double>::lowest();
        for ( int band = 1; band <= rl->bandCount(); ++band )
        {
          bandScope->setVariable( u"band"_s, band );
          bandScope->setVariable( u"band_name"_s, rl->dataProvider()->displayBandName( band ) );
          bandScope->setVariable( u"band_description"_s, rl->dataProvider()->bandDescription( band ) );

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

QList<double> QgsRasterLayerElevationProperties::significantZValues( QgsMapLayer *layer ) const
{
  switch ( mMode )
  {
    case Qgis::RasterElevationMode::FixedElevationRange:
    {
      if ( !mFixedRange.isInfinite() && mFixedRange.lower() != mFixedRange.upper() )
        return { mFixedRange.lower(), mFixedRange.upper() };
      else if ( !mFixedRange.isInfinite() )
        return { mFixedRange.lower() };

      return {};
    }

    case Qgis::RasterElevationMode::FixedRangePerBand:
    {
      QList< double > res;
      for ( auto it = mRangePerBand.constBegin(); it != mRangePerBand.constEnd(); ++it )
      {
        if ( it.value().isInfinite() )
          continue;

        if ( !res.contains( it.value().lower( ) ) )
          res.append( it.value().lower() );
        if ( !res.contains( it.value().upper( ) ) )
          res.append( it.value().upper() );
      }
      std::sort( res.begin(), res.end() );
      return res;
    }

    case Qgis::RasterElevationMode::DynamicRangePerBand:
    {
      QList< double > res;
      if ( QgsRasterLayer *rl = qobject_cast< QgsRasterLayer * >( layer ) )
      {
        QgsExpressionContext context;
        context.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( layer ) );
        QgsExpressionContextScope *bandScope = new QgsExpressionContextScope();
        bandScope->addVariable( QgsExpressionContextScope::StaticVariable( u"band"_s, 1, true, false, tr( "Band number" ) ) );
        bandScope->addVariable( QgsExpressionContextScope::StaticVariable( u"band_name"_s, rl->dataProvider()->displayBandName( 1 ), true, false, tr( "Band name" ) ) );
        bandScope->addVariable( QgsExpressionContextScope::StaticVariable( u"band_description"_s, rl->dataProvider()->bandDescription( 1 ), true, false, tr( "Band description" ) ) );
        context.appendScope( bandScope );

        QgsProperty lowerProperty = mDataDefinedProperties.property( Property::RasterPerBandLowerElevation );
        QgsProperty upperProperty = mDataDefinedProperties.property( Property::RasterPerBandUpperElevation );
        lowerProperty.prepare( context );
        upperProperty.prepare( context );
        for ( int band = 1; band <= rl->bandCount(); ++band )
        {
          bandScope->setVariable( u"band"_s, band );
          bandScope->setVariable( u"band_name"_s, rl->dataProvider()->displayBandName( band ) );
          bandScope->setVariable( u"band_description"_s, rl->dataProvider()->bandDescription( band ) );

          bool ok = false;
          const double lower = lowerProperty.valueAsDouble( context, 0, &ok );
          if ( ok && !res.contains( lower ) )
            res.append( lower );
          const double upper = upperProperty.valueAsDouble( context, 0, &ok );
          if ( ok && !res.contains( upper ) )
            res.append( upper );
        }
      }
      return res;
    }

    case Qgis::RasterElevationMode::RepresentsElevationSurface:
      return {};
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
        bandScope->addVariable( QgsExpressionContextScope::StaticVariable( u"band"_s, band, true, false, tr( "Band number" ) ) );
        bandScope->addVariable( QgsExpressionContextScope::StaticVariable( u"band_name"_s, layer->dataProvider()->displayBandName( band ), true, false, tr( "Band name" ) ) );
        bandScope->addVariable( QgsExpressionContextScope::StaticVariable( u"band_description"_s, layer->dataProvider()->bandDescription( band ), true, false, tr( "Band description" ) ) );
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
          bandScope->setVariable( u"band"_s, band );
          bandScope->setVariable( u"band_name"_s, layer->dataProvider()->displayBandName( band ) );
          bandScope->setVariable( u"band_description"_s, layer->dataProvider()->bandDescription( band ) );

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
  static const QStringList sPartialCandidates{ u"dem"_s,
      u"dtm"_s,
      u"dsm"_s,
      u"height"_s,
      u"elev"_s,
      u"srtm"_s,
      u"dted"_s,
      // French hints
      u"mne"_s,
      u"mnt"_s,
      u"mns"_s,
      u"rge"_s,
      u"alti"_s,
      // German hints
      u"dhm"_s,
      u"dgm"_s,
      u"dom"_s,
      u"Höhe"_s,
      u"Hoehe"_s };
  const QString layerName = layer->name();
  for ( const QString &candidate : sPartialCandidates )
  {
    if ( layerName.contains( candidate, Qt::CaseInsensitive ) )
      return true;
  }

  // these candidates must occur with word boundaries (we don't want to find "aster" in "raster"!)
  static const QStringList sWordCandidates{ u"aster"_s };
  for ( const QString &candidate : sWordCandidates )
  {
    const thread_local QRegularExpression re( u"\\b%1\\b"_s.arg( candidate ) );
    if ( re.match( layerName, Qt::CaseInsensitive ).hasMatch() )
      return true;
  }

  return false;
}

void QgsRasterLayerElevationProperties::setDefaultProfileLineSymbol( const QColor &color )
{
  auto profileLineLayer = std::make_unique< QgsSimpleLineSymbolLayer >( color, 0.6 );
  mProfileLineSymbol = std::make_unique< QgsLineSymbol>( QgsSymbolLayerList( { profileLineLayer.release() } ) );
}

void QgsRasterLayerElevationProperties::setDefaultProfileFillSymbol( const QColor &color )
{
  auto profileFillLayer = std::make_unique< QgsSimpleFillSymbolLayer >( color );
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
