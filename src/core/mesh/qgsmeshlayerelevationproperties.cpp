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
#include "qgsmeshlayer.h"
#include "qgslinesymbol.h"
#include "qgssymbollayerutils.h"
#include "qgslinesymbollayer.h"
#include "qgsapplication.h"
#include "qgscolorschemeregistry.h"

QgsMeshLayerElevationProperties::QgsMeshLayerElevationProperties( QObject *parent )
  : QgsMapLayerElevationProperties( parent )
{
  setDefaultProfileLineSymbol();
}

QgsMeshLayerElevationProperties::~QgsMeshLayerElevationProperties() = default;

bool QgsMeshLayerElevationProperties::hasElevation() const
{
  return true;
}

QDomElement QgsMeshLayerElevationProperties::writeXml( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext &context )
{
  QDomElement element = document.createElement( QStringLiteral( "elevation" ) );
  writeCommonProperties( element, document, context );

  QDomElement profileLineSymbolElement = document.createElement( QStringLiteral( "profileLineSymbol" ) );
  profileLineSymbolElement.appendChild( QgsSymbolLayerUtils::saveSymbol( QString(), mProfileLineSymbol.get(), document, context ) );
  element.appendChild( profileLineSymbolElement );

  parentElement.appendChild( element );
  return element;
}

bool QgsMeshLayerElevationProperties::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  const QDomElement elevationElement = element.firstChildElement( QStringLiteral( "elevation" ) ).toElement();

  readCommonProperties( elevationElement, context );

  const QDomElement profileLineSymbolElement = elevationElement.firstChildElement( QStringLiteral( "profileLineSymbol" ) ).firstChildElement( QStringLiteral( "symbol" ) );
  mProfileLineSymbol.reset( QgsSymbolLayerUtils::loadSymbol< QgsLineSymbol >( profileLineSymbolElement, context ) );
  if ( !mProfileLineSymbol )
    setDefaultProfileLineSymbol();

  return true;
}

QgsMeshLayerElevationProperties *QgsMeshLayerElevationProperties::clone() const
{
  std::unique_ptr< QgsMeshLayerElevationProperties > res = std::make_unique< QgsMeshLayerElevationProperties >( nullptr );
  res->setProfileLineSymbol( mProfileLineSymbol->clone() );
  res->copyCommonProperties( this );
  return res.release();
}

bool QgsMeshLayerElevationProperties::isVisibleInZRange( const QgsDoubleRange & ) const
{
  // TODO -- test actual raster z range
  return true;
}

QgsDoubleRange QgsMeshLayerElevationProperties::calculateZRange( QgsMapLayer * ) const
{
  // TODO -- determine actual z range from raster statistics
  return QgsDoubleRange();
}

QgsLineSymbol *QgsMeshLayerElevationProperties::profileLineSymbol() const
{
  return mProfileLineSymbol.get();
}

void QgsMeshLayerElevationProperties::setProfileLineSymbol( QgsLineSymbol *symbol )
{
  mProfileLineSymbol.reset( symbol );
}

void QgsMeshLayerElevationProperties::setDefaultProfileLineSymbol()
{
  std::unique_ptr< QgsSimpleLineSymbolLayer > profileLineLayer = std::make_unique< QgsSimpleLineSymbolLayer >( QgsApplication::colorSchemeRegistry()->fetchRandomStyleColor(), 0.6 );
  mProfileLineSymbol = std::make_unique< QgsLineSymbol>( QgsSymbolLayerList( { profileLineLayer.release() } ) );
}
