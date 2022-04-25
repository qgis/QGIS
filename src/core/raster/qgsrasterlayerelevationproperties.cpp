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
#include "qgssymbollayerutils.h"
#include "qgslinesymbollayer.h"
#include "qgsapplication.h"
#include "qgscolorschemeregistry.h"

QgsRasterLayerElevationProperties::QgsRasterLayerElevationProperties( QObject *parent )
  : QgsMapLayerElevationProperties( parent )
{
  setDefaultProfileLineSymbol();
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

  writeCommonProperties( element, document, context );
  element.setAttribute( QStringLiteral( "band" ), mBandNumber );

  QDomElement profileLineSymbolElement = document.createElement( QStringLiteral( "profileLineSymbol" ) );
  profileLineSymbolElement.appendChild( QgsSymbolLayerUtils::saveSymbol( QString(), mProfileLineSymbol.get(), document, context ) );
  element.appendChild( profileLineSymbolElement );

  parentElement.appendChild( element );
  return element;
}

bool QgsRasterLayerElevationProperties::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  const QDomElement elevationElement = element.firstChildElement( QStringLiteral( "elevation" ) ).toElement();
  mEnabled = elevationElement.attribute( QStringLiteral( "enabled" ), QStringLiteral( "0" ) ).toInt();

  readCommonProperties( elevationElement, context );
  mBandNumber = elevationElement.attribute( QStringLiteral( "band" ), QStringLiteral( "1" ) ).toInt();

  const QDomElement profileLineSymbolElement = elevationElement.firstChildElement( QStringLiteral( "profileLineSymbol" ) ).firstChildElement( QStringLiteral( "symbol" ) );
  mProfileLineSymbol.reset( QgsSymbolLayerUtils::loadSymbol< QgsLineSymbol >( profileLineSymbolElement, context ) );
  if ( !mProfileLineSymbol )
    setDefaultProfileLineSymbol();

  return true;
}

QgsRasterLayerElevationProperties *QgsRasterLayerElevationProperties::clone() const
{
  std::unique_ptr< QgsRasterLayerElevationProperties > res = std::make_unique< QgsRasterLayerElevationProperties >( nullptr );
  res->setEnabled( mEnabled );
  res->setProfileLineSymbol( mProfileLineSymbol->clone() );
  res->setBandNumber( mBandNumber );
  res->copyCommonProperties( this );
  return res.release();
}

QString QgsRasterLayerElevationProperties::htmlSummary() const
{
  QStringList properties;
  properties << tr( "Elevation band: %1" ).arg( mBandNumber );
  properties << tr( "Scale: %1" ).arg( mZScale );
  properties << tr( "Offset: %1" ).arg( mZOffset );
  return QStringLiteral( "<li>%1</li>" ).arg( properties.join( QStringLiteral( "</li><li>" ) ) );
}

bool QgsRasterLayerElevationProperties::isVisibleInZRange( const QgsDoubleRange & ) const
{
  // TODO -- test actual raster z range
  return true;
}

QgsDoubleRange QgsRasterLayerElevationProperties::calculateZRange( QgsMapLayer * ) const
{
  // TODO -- determine actual z range from raster statistics
  return QgsDoubleRange();
}

bool QgsRasterLayerElevationProperties::showByDefaultInElevationProfilePlots() const
{
  return mEnabled;
}

void QgsRasterLayerElevationProperties::setEnabled( bool enabled )
{
  if ( enabled == mEnabled )
    return;

  mEnabled = enabled;
  emit changed();
  emit profileGenerationPropertyChanged();
}

void QgsRasterLayerElevationProperties::setBandNumber( int band )
{
  if ( mBandNumber == band )
    return;

  mBandNumber = band;
  emit changed();
  emit profileGenerationPropertyChanged();
}

QgsLineSymbol *QgsRasterLayerElevationProperties::profileLineSymbol() const
{
  return mProfileLineSymbol.get();
}

void QgsRasterLayerElevationProperties::setProfileLineSymbol( QgsLineSymbol *symbol )
{
  mProfileLineSymbol.reset( symbol );
  emit changed();
  emit renderingPropertyChanged();
}

void QgsRasterLayerElevationProperties::setDefaultProfileLineSymbol()
{
  std::unique_ptr< QgsSimpleLineSymbolLayer > profileLineLayer = std::make_unique< QgsSimpleLineSymbolLayer >( QgsApplication::colorSchemeRegistry()->fetchRandomStyleColor(), 0.6 );
  mProfileLineSymbol = std::make_unique< QgsLineSymbol>( QgsSymbolLayerList( { profileLineLayer.release() } ) );
}
