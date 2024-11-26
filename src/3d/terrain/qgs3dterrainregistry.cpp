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

#include <QDomElement>


Qgs3DTerrainRegistry::Qgs3DTerrainRegistry()
{
  addType( new Qgs3DTerrainMetadata( QStringLiteral( "flat" ), QObject::tr( "Flat Terrain" ), &QgsFlatTerrainSettings::create ) );
  addType( new Qgs3DTerrainMetadata( QStringLiteral( "dem" ), QObject::tr( "DEM (Raster Layer)" ), &QgsDemTerrainSettings::create ) );
  addType( new Qgs3DTerrainMetadata( QStringLiteral( "online" ), QObject::tr( "Online" ), &QgsOnlineDemTerrainSettings::create ) );
  addType( new Qgs3DTerrainMetadata( QStringLiteral( "mesh" ), QObject::tr( "Mesh" ), &QgsMeshTerrainSettings::create ) );
  addType( new Qgs3DTerrainMetadata( QStringLiteral( "quantizedmesh" ), QObject::tr( "Quantized Mesh" ), &QgsQuantizedMeshTerrainSettings::create ) );
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
