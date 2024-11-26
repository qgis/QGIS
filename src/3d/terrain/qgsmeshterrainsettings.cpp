/***************************************************************************
  qgsmeshterrainsettings.cpp
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

#include "qgsmeshterrainsettings.h"
#include "qgsmeshlayer.h"
#include "qgsmeshterraingenerator.h"

QgsAbstractTerrainSettings *QgsMeshTerrainSettings::create()
{
  return new QgsMeshTerrainSettings();
}

QgsMeshTerrainSettings *QgsMeshTerrainSettings::clone() const
{
  return new QgsMeshTerrainSettings( *this );
}

QString QgsMeshTerrainSettings::type() const
{
  return QStringLiteral( "mesh" );
}

void QgsMeshTerrainSettings::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  mLayer = QgsMapLayerRef( element.attribute( QStringLiteral( "layer" ) ) );
  readCommonProperties( element, context );
}

void QgsMeshTerrainSettings::writeXml( QDomElement &element, const QgsReadWriteContext &context ) const
{
  element.setAttribute( QStringLiteral( "layer" ), mLayer.layerId );
  writeCommonProperties( element, context );
}

void QgsMeshTerrainSettings::resolveReferences( const QgsProject *project )
{
  mLayer.resolve( project );
}

bool QgsMeshTerrainSettings::equals( const QgsAbstractTerrainSettings *other ) const
{
  const QgsMeshTerrainSettings *otherMeshTerrain = dynamic_cast<const QgsMeshTerrainSettings *>( other );
  if ( !otherMeshTerrain )
    return false;

  if ( !equalsCommon( otherMeshTerrain ) )
    return false;

  return mLayer.layerId == otherMeshTerrain->mLayer.layerId;
}

std::unique_ptr<QgsTerrainGenerator> QgsMeshTerrainSettings::createTerrainGenerator() const
{
  std::unique_ptr<QgsMeshTerrainGenerator> generator = std::make_unique<QgsMeshTerrainGenerator>();
  generator->setLayer( layer() );
  std::unique_ptr<QgsMesh3DSymbol> symbol( generator->symbol()->clone() );
  symbol->setVerticalScale( verticalScale() );
  generator->setSymbol( symbol.release() );
  return generator;
}

void QgsMeshTerrainSettings::setLayer( QgsMeshLayer *layer )
{
  mLayer = QgsMapLayerRef( layer );
}

QgsMeshLayer *QgsMeshTerrainSettings::layer() const
{
  return qobject_cast<QgsMeshLayer *>( mLayer.layer.data() );
}
