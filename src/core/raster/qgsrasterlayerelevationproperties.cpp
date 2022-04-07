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
  element.setAttribute( QStringLiteral( "zoffset" ), qgsDoubleToString( mZOffset ) );
  element.setAttribute( QStringLiteral( "zscale" ), qgsDoubleToString( mZScale ) );
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
  mZOffset = elevationElement.attribute( QStringLiteral( "zoffset" ), QStringLiteral( "0" ) ).toDouble();
  mZScale = elevationElement.attribute( QStringLiteral( "zscale" ), QStringLiteral( "1" ) ).toDouble();
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
  res->setZOffset( mZOffset );
  res->setZScale( mZScale );
  res->setProfileLineSymbol( mProfileLineSymbol->clone() );
  res->setBandNumber( mBandNumber );
  return res.release();
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

QgsLineSymbol *QgsRasterLayerElevationProperties::profileLineSymbol() const
{
  return mProfileLineSymbol.get();
}

void QgsRasterLayerElevationProperties::setProfileLineSymbol( QgsLineSymbol *symbol )
{
  mProfileLineSymbol.reset( symbol );
}

void QgsRasterLayerElevationProperties::setDefaultProfileLineSymbol()
{
  std::unique_ptr< QgsSimpleLineSymbolLayer > profileLineLayer = std::make_unique< QgsSimpleLineSymbolLayer >( QgsApplication::colorSchemeRegistry()->fetchRandomStyleColor(), 0.6 );
  mProfileLineSymbol = std::make_unique< QgsLineSymbol>( QgsSymbolLayerList( { profileLineLayer.release() } ) );
}
