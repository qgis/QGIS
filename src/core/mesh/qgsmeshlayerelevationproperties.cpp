/***************************************************************************
                         qgsmeshlayerelevationproperties.cpp
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

#include "qgsmeshlayerelevationproperties.h"
#include "moc_qgsmeshlayerelevationproperties.cpp"
#include "qgsmeshlayer.h"
#include "qgslinesymbol.h"
#include "qgsfillsymbol.h"
#include "qgssymbollayerutils.h"
#include "qgslinesymbollayer.h"
#include "qgsfillsymbollayer.h"
#include "qgsapplication.h"
#include "qgscolorschemeregistry.h"

QgsMeshLayerElevationProperties::QgsMeshLayerElevationProperties( QObject *parent )
  : QgsMapLayerElevationProperties( parent )
{
  const QColor color = QgsApplication::colorSchemeRegistry()->fetchRandomStyleColor();
  setDefaultProfileLineSymbol( color );
  setDefaultProfileFillSymbol( color );
}

QgsMeshLayerElevationProperties::~QgsMeshLayerElevationProperties() = default;

bool QgsMeshLayerElevationProperties::hasElevation() const
{
  return true;
}

QDomElement QgsMeshLayerElevationProperties::writeXml( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext &context )
{
  QDomElement element = document.createElement( QStringLiteral( "elevation" ) );
  element.setAttribute( QStringLiteral( "mode" ), qgsEnumValueToKey( mMode ) );
  element.setAttribute( QStringLiteral( "symbology" ), qgsEnumValueToKey( mSymbology ) );
  if ( !std::isnan( mElevationLimit ) )
    element.setAttribute( QStringLiteral( "elevationLimit" ), qgsDoubleToString( mElevationLimit ) );

  writeCommonProperties( element, document, context );

  switch ( mMode )
  {
    case Qgis::MeshElevationMode::FixedElevationRange:
      element.setAttribute( QStringLiteral( "lower" ), qgsDoubleToString( mFixedRange.lower() ) );
      element.setAttribute( QStringLiteral( "upper" ), qgsDoubleToString( mFixedRange.upper() ) );
      element.setAttribute( QStringLiteral( "includeLower" ), mFixedRange.includeLower() ? "1" : "0" );
      element.setAttribute( QStringLiteral( "includeUpper" ), mFixedRange.includeUpper() ? "1" : "0" );
      break;

    case Qgis::MeshElevationMode::FixedRangePerGroup:
    {
      QDomElement ranges = document.createElement( QStringLiteral( "ranges" ) );
      for ( auto it = mRangePerGroup.constBegin(); it != mRangePerGroup.constEnd(); ++it )
      {
        QDomElement range = document.createElement( QStringLiteral( "range" ) );
        range.setAttribute( QStringLiteral( "group" ), it.key() );
        range.setAttribute( QStringLiteral( "lower" ), qgsDoubleToString( it.value().lower() ) );
        range.setAttribute( QStringLiteral( "upper" ), qgsDoubleToString( it.value().upper() ) );
        range.setAttribute( QStringLiteral( "includeLower" ), it.value().includeLower() ? "1" : "0" );
        range.setAttribute( QStringLiteral( "includeUpper" ), it.value().includeUpper() ? "1" : "0" );
        ranges.appendChild( range );
      }
      element.appendChild( ranges );
      break;
    }

    case Qgis::MeshElevationMode::FromVertices:
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

bool QgsMeshLayerElevationProperties::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  const QDomElement elevationElement = element.firstChildElement( QStringLiteral( "elevation" ) ).toElement();
  mMode = qgsEnumKeyToValue( elevationElement.attribute( QStringLiteral( "mode" ) ), Qgis::MeshElevationMode::FromVertices );
  mSymbology = qgsEnumKeyToValue( elevationElement.attribute( QStringLiteral( "symbology" ) ), Qgis::ProfileSurfaceSymbology::Line );
  if ( elevationElement.hasAttribute( QStringLiteral( "elevationLimit" ) ) )
    mElevationLimit = elevationElement.attribute( QStringLiteral( "elevationLimit" ) ).toDouble();
  else
    mElevationLimit = std::numeric_limits< double >::quiet_NaN();

  readCommonProperties( elevationElement, context );

  switch ( mMode )
  {
    case Qgis::MeshElevationMode::FixedElevationRange:
    {
      const double lower = elevationElement.attribute( QStringLiteral( "lower" ) ).toDouble();
      const double upper = elevationElement.attribute( QStringLiteral( "upper" ) ).toDouble();
      const bool includeLower = elevationElement.attribute( QStringLiteral( "includeLower" ) ).toInt();
      const bool includeUpper = elevationElement.attribute( QStringLiteral( "includeUpper" ) ).toInt();
      mFixedRange = QgsDoubleRange( lower, upper, includeLower, includeUpper );
      break;
    }

    case Qgis::MeshElevationMode::FixedRangePerGroup:
    {
      mRangePerGroup.clear();

      const QDomNodeList ranges = elevationElement.firstChildElement( QStringLiteral( "ranges" ) ).childNodes();
      for ( int i = 0; i < ranges.size(); ++i )
      {
        const QDomElement rangeElement = ranges.at( i ).toElement();
        const int group = rangeElement.attribute( QStringLiteral( "group" ) ).toInt();
        const double lower = rangeElement.attribute( QStringLiteral( "lower" ) ).toDouble();
        const double upper = rangeElement.attribute( QStringLiteral( "upper" ) ).toDouble();
        const bool includeLower = rangeElement.attribute( QStringLiteral( "includeLower" ) ).toInt();
        const bool includeUpper = rangeElement.attribute( QStringLiteral( "includeUpper" ) ).toInt();
        mRangePerGroup.insert( group, QgsDoubleRange( lower, upper, includeLower, includeUpper ) );
      }
      break;
    }

    case Qgis::MeshElevationMode::FromVertices:
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

QString QgsMeshLayerElevationProperties::htmlSummary() const
{
  QStringList properties;
  switch ( mMode )
  {
    case Qgis::MeshElevationMode::FixedElevationRange:
      properties << tr( "Elevation range: %1 to %2" ).arg( mFixedRange.lower() ).arg( mFixedRange.upper() );
      break;

    case Qgis::MeshElevationMode::FixedRangePerGroup:
    {
      for ( auto it = mRangePerGroup.constBegin(); it != mRangePerGroup.constEnd(); ++it )
      {
        properties << tr( "Elevation for group %1: %2 to %3" ).arg( it.key() ).arg( it.value().lower() ).arg( it.value().upper() );
      }
      break;
    }

    case Qgis::MeshElevationMode::FromVertices:
      properties << tr( "Scale: %1" ).arg( mZScale );
      properties << tr( "Offset: %1" ).arg( mZOffset );
      break;
  }
  return QStringLiteral( "<li>%1</li>" ).arg( properties.join( QLatin1String( "</li><li>" ) ) );
}

QgsMeshLayerElevationProperties *QgsMeshLayerElevationProperties::clone() const
{
  std::unique_ptr< QgsMeshLayerElevationProperties > res = std::make_unique< QgsMeshLayerElevationProperties >( nullptr );
  res->setMode( mMode );
  res->setProfileLineSymbol( mProfileLineSymbol->clone() );
  res->setProfileFillSymbol( mProfileFillSymbol->clone() );
  res->setProfileSymbology( mSymbology );
  res->setElevationLimit( mElevationLimit );
  res->setFixedRange( mFixedRange );
  res->setFixedRangePerGroup( mRangePerGroup );
  res->copyCommonProperties( this );
  return res.release();
}

bool QgsMeshLayerElevationProperties::isVisibleInZRange( const QgsDoubleRange &range, QgsMapLayer * ) const
{
  switch ( mMode )
  {
    case Qgis::MeshElevationMode::FixedElevationRange:
      return mFixedRange.overlaps( range );

    case Qgis::MeshElevationMode::FixedRangePerGroup:
    {
      for ( auto it = mRangePerGroup.constBegin(); it != mRangePerGroup.constEnd(); ++it )
      {
        if ( it.value().overlaps( range ) )
          return true;
      }
      return false;
    }

    case Qgis::MeshElevationMode::FromVertices:
      // TODO -- test actual mesh z range
      return true;
  }
  BUILTIN_UNREACHABLE
}

QgsDoubleRange QgsMeshLayerElevationProperties::calculateZRange( QgsMapLayer * ) const
{
  switch ( mMode )
  {
    case Qgis::MeshElevationMode::FixedElevationRange:
      return mFixedRange;

    case Qgis::MeshElevationMode::FixedRangePerGroup:
    {
      double lower = std::numeric_limits< double >::max();
      double upper = std::numeric_limits< double >::min();
      bool includeLower = true;
      bool includeUpper = true;
      for ( auto it = mRangePerGroup.constBegin(); it != mRangePerGroup.constEnd(); ++it )
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

    case Qgis::MeshElevationMode::FromVertices:
      // TODO -- determine actual z range from mesh statistics
      return QgsDoubleRange();
  }
  BUILTIN_UNREACHABLE
}

QList<double> QgsMeshLayerElevationProperties::significantZValues( QgsMapLayer * ) const
{
  switch ( mMode )
  {
    case Qgis::MeshElevationMode::FixedElevationRange:
    {
      if ( !mFixedRange.isInfinite() && mFixedRange.lower() != mFixedRange.upper() )
        return { mFixedRange.lower(), mFixedRange.upper() };
      else if ( !mFixedRange.isInfinite() )
        return { mFixedRange.lower() };

      return {};
    }

    case Qgis::MeshElevationMode::FixedRangePerGroup:
    {
      QList< double > res;
      for ( auto it = mRangePerGroup.constBegin(); it != mRangePerGroup.constEnd(); ++it )
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

    case Qgis::MeshElevationMode::FromVertices:
      return {};
  }
  BUILTIN_UNREACHABLE
}

bool QgsMeshLayerElevationProperties::showByDefaultInElevationProfilePlots() const
{
  return true;
}

QgsMapLayerElevationProperties::Flags QgsMeshLayerElevationProperties::flags() const
{
  switch ( mMode )
  {
    case Qgis::MeshElevationMode::FixedElevationRange:
      return QgsMapLayerElevationProperties::Flag::FlagDontInvalidateCachedRendersWhenRangeChanges;

    case Qgis::MeshElevationMode::FixedRangePerGroup:
    case Qgis::MeshElevationMode::FromVertices:
      break;
  }
  return QgsMapLayerElevationProperties::Flags();
}

Qgis::MeshElevationMode QgsMeshLayerElevationProperties::mode() const
{
  return mMode;
}

void QgsMeshLayerElevationProperties::setMode( Qgis::MeshElevationMode mode )
{
  if ( mMode == mode )
    return;

  mMode = mode;
  emit changed();
}

QgsDoubleRange QgsMeshLayerElevationProperties::fixedRange() const
{
  return mFixedRange;
}

void QgsMeshLayerElevationProperties::setFixedRange( const QgsDoubleRange &range )
{
  if ( range == mFixedRange )
    return;

  mFixedRange = range;
  emit changed();
}

QMap<int, QgsDoubleRange> QgsMeshLayerElevationProperties::fixedRangePerGroup() const
{
  return mRangePerGroup;
}

void QgsMeshLayerElevationProperties::setFixedRangePerGroup( const QMap<int, QgsDoubleRange> &ranges )
{
  if ( ranges == mRangePerGroup )
    return;

  mRangePerGroup = ranges;
  emit changed();
}

QgsLineSymbol *QgsMeshLayerElevationProperties::profileLineSymbol() const
{
  return mProfileLineSymbol.get();
}

void QgsMeshLayerElevationProperties::setProfileLineSymbol( QgsLineSymbol *symbol )
{
  mProfileLineSymbol.reset( symbol );
  emit changed();
  emit profileRenderingPropertyChanged();
}

QgsFillSymbol *QgsMeshLayerElevationProperties::profileFillSymbol() const
{
  return mProfileFillSymbol.get();
}

void QgsMeshLayerElevationProperties::setProfileFillSymbol( QgsFillSymbol *symbol )
{
  mProfileFillSymbol.reset( symbol );
  emit changed();
  emit profileRenderingPropertyChanged();
}

void QgsMeshLayerElevationProperties::setProfileSymbology( Qgis::ProfileSurfaceSymbology symbology )
{
  if ( mSymbology == symbology )
    return;

  mSymbology = symbology;
  emit changed();
  emit profileRenderingPropertyChanged();
}

double QgsMeshLayerElevationProperties::elevationLimit() const
{
  return mElevationLimit;
}

void QgsMeshLayerElevationProperties::setElevationLimit( double limit )
{
  if ( qgsDoubleNear( mElevationLimit, limit ) )
    return;

  mElevationLimit = limit;
  emit changed();
  emit profileRenderingPropertyChanged();
}

void QgsMeshLayerElevationProperties::setDefaultProfileLineSymbol( const QColor &color )
{
  std::unique_ptr< QgsSimpleLineSymbolLayer > profileLineLayer = std::make_unique< QgsSimpleLineSymbolLayer >( color, 0.6 );
  mProfileLineSymbol = std::make_unique< QgsLineSymbol>( QgsSymbolLayerList( { profileLineLayer.release() } ) );
}

void QgsMeshLayerElevationProperties::setDefaultProfileFillSymbol( const QColor &color )
{
  std::unique_ptr< QgsSimpleFillSymbolLayer > profileFillLayer = std::make_unique< QgsSimpleFillSymbolLayer >( color );
  profileFillLayer->setStrokeStyle( Qt::NoPen );
  mProfileFillSymbol = std::make_unique< QgsFillSymbol>( QgsSymbolLayerList( { profileFillLayer.release() } ) );
}
