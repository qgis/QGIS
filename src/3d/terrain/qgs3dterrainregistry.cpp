/***************************************************************************
  qgs3dterrainregistry.cpp
  --------------------------------------
  Date                 : November 2024
  Copyright            : (C) 2024 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3dterrainregistry.h"
#include "qgis.h"

#include "qgsflatterrainsettings.h"
#include "qgsdemterrainsettings.h"
#include "qgsonlinedemterrainsettings.h"
#include "qgsmeshterrainsettings.h"
#include "qgsquantizedmeshterrainsettings.h"
#include "qgsflatterraingenerator.h"
#include "qgsdemterraingenerator.h"
#include "qgsonlineterraingenerator.h"
#include "qgsmeshterraingenerator.h"
#include "qgsquantizedmeshterraingenerator.h"
#include "qgsprojectelevationproperties.h"
#include "qgsterrainprovider.h"

#include <QDomElement>


Qgs3DTerrainRegistry::Qgs3DTerrainRegistry()
{
  addType( new Qgs3DTerrainMetadata( QStringLiteral( "flat" ), QObject::tr( "Flat Terrain" ), &QgsFlatTerrainSettings::create, &QgsFlatTerrainGenerator::create ) );
  addType( new Qgs3DTerrainMetadata( QStringLiteral( "dem" ), QObject::tr( "DEM (Raster Layer)" ), &QgsDemTerrainSettings::create, &QgsDemTerrainGenerator::create ) );
  addType( new Qgs3DTerrainMetadata( QStringLiteral( "online" ), QObject::tr( "Online" ), &QgsOnlineDemTerrainSettings::create, &QgsOnlineTerrainGenerator::create ) );
  addType( new Qgs3DTerrainMetadata( QStringLiteral( "mesh" ), QObject::tr( "Mesh" ), &QgsMeshTerrainSettings::create, &QgsMeshTerrainGenerator::create ) );
  addType( new Qgs3DTerrainMetadata( QStringLiteral( "quantizedmesh" ), QObject::tr( "Quantized Mesh" ), &QgsQuantizedMeshTerrainSettings::create, &QgsQuantizedMeshTerrainGenerator::create ) );
}

Qgs3DTerrainRegistry::~Qgs3DTerrainRegistry()
{
  qDeleteAll( mMetadata );
}

bool Qgs3DTerrainRegistry::addType( Qgs3DTerrainAbstractMetadata *metadata )
{
  if ( !metadata || mMetadata.contains( metadata->type() ) )
    return false;

  mMetadata[metadata->type()] = metadata;
  mTerrainOrder << metadata->type();
  return true;
}

QgsAbstractTerrainSettings *Qgs3DTerrainRegistry::createTerrainSettings( const QString &type ) const
{
  if ( !mMetadata.contains( type ) )
    return nullptr;

  return mMetadata[type]->createTerrainSettings();
}

QgsTerrainGenerator *Qgs3DTerrainRegistry::createTerrainGenerator( const QString &type ) const
{
  if ( !mMetadata.contains( type ) )
    return nullptr;

  return mMetadata[type]->createTerrainGenerator();
}

QgsAbstractTerrainSettings *Qgs3DTerrainRegistry::configureTerrainFromProject( QgsProjectElevationProperties *properties )
{
  if ( properties->terrainProvider()->type() == QLatin1String( "flat" ) )
  {
    std::unique_ptr<QgsFlatTerrainSettings> flatTerrain = std::make_unique<QgsFlatTerrainSettings>();
    flatTerrain->setElevationOffset( properties->terrainProvider()->offset() );
    return flatTerrain.release();
  }
  else if ( properties->terrainProvider()->type() == QLatin1String( "raster" ) )
  {
    QgsRasterDemTerrainProvider *rasterProvider = qgis::down_cast<QgsRasterDemTerrainProvider *>( properties->terrainProvider() );

    std::unique_ptr<QgsDemTerrainSettings> demTerrain = std::make_unique<QgsDemTerrainSettings>();
    demTerrain->setLayer( rasterProvider->layer() );
    demTerrain->setElevationOffset( properties->terrainProvider()->offset() );
    demTerrain->setVerticalScale( properties->terrainProvider()->scale() );
    return demTerrain.release();
  }
  else if ( properties->terrainProvider()->type() == QLatin1String( "mesh" ) )
  {
    QgsMeshTerrainProvider *meshProvider = qgis::down_cast<QgsMeshTerrainProvider *>( properties->terrainProvider() );

    std::unique_ptr<QgsMeshTerrainSettings> meshTerrain = std::make_unique<QgsMeshTerrainSettings>();
    meshTerrain->setLayer( meshProvider->layer() );
    meshTerrain->setElevationOffset( properties->terrainProvider()->offset() );
    meshTerrain->setVerticalScale( properties->terrainProvider()->scale() );
    return meshTerrain.release();
  }
  else
  {
    std::unique_ptr<QgsFlatTerrainSettings> flatTerrain = std::make_unique<QgsFlatTerrainSettings>();
    return flatTerrain.release();
  }
}

Qgs3DTerrainAbstractMetadata *Qgs3DTerrainRegistry::terrainMetadata( const QString &type ) const
{
  return mMetadata.value( type );
}

QStringList Qgs3DTerrainRegistry::types() const
{
  QStringList types;
  for ( const QString &type : mTerrainOrder )
  {
    if ( mMetadata.value( type ) )
      types << type;
  }
  return types;
}
