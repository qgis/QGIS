/***************************************************************************
  qgsquantizedmeshterrainsettings.cpp
  --------------------------------------
  Date                 : August 2024
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

#include "qgsquantizedmeshterrainsettings.h"
#include "qgstiledscenelayer.h"
#include "qgsquantizedmeshterraingenerator.h"
#include "qgs3drendercontext.h"

QgsAbstractTerrainSettings *QgsQuantizedMeshTerrainSettings::create()
{
  return new QgsQuantizedMeshTerrainSettings();
}

QgsQuantizedMeshTerrainSettings *QgsQuantizedMeshTerrainSettings::clone() const
{
  return new QgsQuantizedMeshTerrainSettings( *this );
}

QString QgsQuantizedMeshTerrainSettings::type() const
{
  return QStringLiteral( "quantizedmesh" );
}

void QgsQuantizedMeshTerrainSettings::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  if ( element.hasAttribute( QStringLiteral( "layer" ) ) )
  {
    mLayer = QgsMapLayerRef( element.attribute( QStringLiteral( "layer" ) ) );
  }
  else
  {
    // restore old project
    const QDomElement elemTerrainGenerator = element.firstChildElement( QStringLiteral( "generator" ) );
    mLayer = QgsMapLayerRef( elemTerrainGenerator.attribute( QStringLiteral( "layer" ) ) );
  }

  readCommonProperties( element, context );
}

void QgsQuantizedMeshTerrainSettings::writeXml( QDomElement &element, const QgsReadWriteContext &context ) const
{
  element.setAttribute( QStringLiteral( "layer" ), mLayer.layerId );
  writeCommonProperties( element, context );
}

void QgsQuantizedMeshTerrainSettings::resolveReferences( const QgsProject *project )
{
  mLayer.resolve( project );
}

bool QgsQuantizedMeshTerrainSettings::equals( const QgsAbstractTerrainSettings *other ) const
{
  const QgsQuantizedMeshTerrainSettings *otherMeshTerrain = dynamic_cast<const QgsQuantizedMeshTerrainSettings *>( other );
  if ( !otherMeshTerrain )
    return false;

  if ( !equalsCommon( otherMeshTerrain ) )
    return false;

  return mLayer.layerId == otherMeshTerrain->mLayer.layerId;
}

std::unique_ptr<QgsTerrainGenerator> QgsQuantizedMeshTerrainSettings::createTerrainGenerator( const Qgs3DRenderContext &context ) const
{
  std::unique_ptr<QgsQuantizedMeshTerrainGenerator> generator = std::make_unique<QgsQuantizedMeshTerrainGenerator>();
  generator->setLayer( layer() );
  generator->setCrs( context.crs(), context.transformContext() );
  generator->setExtent( context.extent() );
  return generator;
}

void QgsQuantizedMeshTerrainSettings::setLayer( QgsTiledSceneLayer *layer )
{
  mLayer = QgsMapLayerRef( layer );
}

QgsTiledSceneLayer *QgsQuantizedMeshTerrainSettings::layer() const
{
  return qobject_cast<QgsTiledSceneLayer *>( mLayer.layer.data() );
}
