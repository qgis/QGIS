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
#include "qgs3drendercontext.h"

QgsAbstractTerrainSettings *QgsMeshTerrainSettings::create()
{
  return new QgsMeshTerrainSettings();
}

QgsMeshTerrainSettings::QgsMeshTerrainSettings()
  : mSymbol( std::make_unique<QgsMesh3DSymbol>() )
{
}

QgsMeshTerrainSettings::~QgsMeshTerrainSettings() = default;

QgsMeshTerrainSettings *QgsMeshTerrainSettings::clone() const
{
  std::unique_ptr<QgsMeshTerrainSettings> cloned = std::make_unique<QgsMeshTerrainSettings>();
  cloned->mSymbol.reset( mSymbol->clone() );
  cloned->mLayer = mLayer;
  cloned->copyCommonProperties( this );
  return cloned.release();
}

QString QgsMeshTerrainSettings::type() const
{
  return QStringLiteral( "mesh" );
}

void QgsMeshTerrainSettings::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  if ( element.hasAttribute( QStringLiteral( "layer" ) ) )
  {
    mLayer = QgsMapLayerRef( element.attribute( QStringLiteral( "layer" ) ) );
    mSymbol->readXml( element.firstChildElement( "symbol" ), context );
  }
  else
  {
    // restore old project
    const QDomElement elemTerrainGenerator = element.firstChildElement( QStringLiteral( "generator" ) );
    mLayer = QgsMapLayerRef( elemTerrainGenerator.attribute( QStringLiteral( "layer" ) ) );
    mSymbol->readXml( elemTerrainGenerator.firstChildElement( "symbol" ), context );
  }

  readCommonProperties( element, context );
}

void QgsMeshTerrainSettings::writeXml( QDomElement &element, const QgsReadWriteContext &context ) const
{
  element.setAttribute( QStringLiteral( "layer" ), mLayer.layerId );

  {
    QDomElement elemSymbol = element.ownerDocument().createElement( "symbol" );
    mSymbol->writeXml( elemSymbol, context );
    element.appendChild( elemSymbol );
  }

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

  if ( *mSymbol != *otherMeshTerrain->mSymbol )
    return false;

  if ( !equalsCommon( otherMeshTerrain ) )
    return false;

  return mLayer.layerId == otherMeshTerrain->mLayer.layerId;
}

std::unique_ptr<QgsTerrainGenerator> QgsMeshTerrainSettings::createTerrainGenerator( const Qgs3DRenderContext &context ) const
{
  std::unique_ptr<QgsMeshTerrainGenerator> generator = std::make_unique<QgsMeshTerrainGenerator>();
  generator->setLayer( layer() );
  std::unique_ptr<QgsMesh3DSymbol> symbol( mSymbol->clone() );
  symbol->setVerticalScale( verticalScale() );
  generator->setSymbol( symbol.release() );
  generator->setCrs( context.crs(), context.transformContext() );
  generator->setExtent( context.extent() );
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

QgsMesh3DSymbol *QgsMeshTerrainSettings::symbol() const
{
  return mSymbol.get();
}

void QgsMeshTerrainSettings::setSymbol( QgsMesh3DSymbol *symbol )
{
  mSymbol.reset( symbol );
}
