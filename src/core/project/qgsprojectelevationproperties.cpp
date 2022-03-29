/***************************************************************************
                         qgsprojectelevationproperties.cpp
                         ---------------
    begin                : February 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsprojectelevationproperties.h"
#include "qgis.h"
#include "qgsterrainprovider.h"

#include <QDomElement>

QgsProjectElevationProperties::QgsProjectElevationProperties( QObject *parent )
  : QObject( parent )
  , mTerrainProvider( std::make_unique< QgsFlatTerrainProvider >() )
{

}

QgsProjectElevationProperties::~QgsProjectElevationProperties() = default;

void QgsProjectElevationProperties::reset()
{
  mTerrainProvider = std::make_unique< QgsFlatTerrainProvider >();
  emit changed();
}

void QgsProjectElevationProperties::resolveReferences( const QgsProject *project )
{
  if ( mTerrainProvider )
    mTerrainProvider->resolveReferences( project );
}

bool QgsProjectElevationProperties::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  const QDomElement providerElement = element.firstChildElement( QStringLiteral( "terrainProvider" ) );
  if ( !providerElement.isNull() )
  {
    const QString type = providerElement.attribute( QStringLiteral( "type" ) );
    if ( type.compare( QLatin1String( "flat" ) ) == 0 )
      mTerrainProvider = std::make_unique< QgsFlatTerrainProvider >();
    else if ( type.compare( QLatin1String( "raster" ) ) == 0 )
      mTerrainProvider = std::make_unique< QgsRasterDemTerrainProvider >();
    else if ( type.compare( QLatin1String( "mesh" ) ) == 0 )
      mTerrainProvider = std::make_unique< QgsMeshTerrainProvider >();
    else
      mTerrainProvider = std::make_unique< QgsFlatTerrainProvider >();

    mTerrainProvider->readXml( providerElement, context );
  }
  else
  {
    mTerrainProvider = std::make_unique< QgsFlatTerrainProvider >();
  }

  emit changed();
  return true;
}

QDomElement QgsProjectElevationProperties::writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const
{
  QDomElement element = document.createElement( QStringLiteral( "ElevationProperties" ) );

  if ( mTerrainProvider )
  {
    QDomElement providerElement = document.createElement( QStringLiteral( "terrainProvider" ) );
    providerElement.setAttribute( QStringLiteral( "type" ), mTerrainProvider->type() );
    providerElement.appendChild( mTerrainProvider->writeXml( document, context ) );
    element.appendChild( providerElement );
  }
  return element;
}

QgsAbstractTerrainProvider *QgsProjectElevationProperties::terrainProvider()
{
  return mTerrainProvider.get();
}

void QgsProjectElevationProperties::setTerrainProvider( QgsAbstractTerrainProvider *provider )
{
  if ( mTerrainProvider.get() == provider )
    return;

  mTerrainProvider.reset( provider );
  emit changed();
}
