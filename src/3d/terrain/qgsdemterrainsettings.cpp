/***************************************************************************
  qgsdemterrainsettings.cpp
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

#include "qgsdemterrainsettings.h"

#include "qgs3drendercontext.h"
#include "qgsdemterraingenerator.h"
#include "qgsrasterlayer.h"

QgsAbstractTerrainSettings *QgsDemTerrainSettings::create()
{
  return new QgsDemTerrainSettings();
}

QgsDemTerrainSettings *QgsDemTerrainSettings::clone() const
{
  return new QgsDemTerrainSettings( *this );
}

QString QgsDemTerrainSettings::type() const
{
  return u"dem"_s;
}

void QgsDemTerrainSettings::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  if ( element.hasAttribute( u"layer"_s ) )
  {
    mLayer = QgsMapLayerRef( element.attribute( u"layer"_s ) );
    mResolution = element.attribute( u"resolution"_s ).toInt();
    mSkirtHeight = element.attribute( u"skirt-height"_s ).toDouble();
  }
  else
  {
    // restore old project
    const QDomElement elemTerrainGenerator = element.firstChildElement( u"generator"_s );
    mLayer = QgsMapLayerRef( elemTerrainGenerator.attribute( u"layer"_s ) );
    mResolution = elemTerrainGenerator.attribute( u"resolution"_s ).toInt();
    mSkirtHeight = elemTerrainGenerator.attribute( u"skirt-height"_s ).toDouble();
  }

  readCommonProperties( element, context );
}

void QgsDemTerrainSettings::writeXml( QDomElement &element, const QgsReadWriteContext &context ) const
{
  element.setAttribute( u"layer"_s, mLayer.layerId );
  element.setAttribute( u"resolution"_s, mResolution );
  element.setAttribute( u"skirt-height"_s, mSkirtHeight );
  writeCommonProperties( element, context );
}

void QgsDemTerrainSettings::resolveReferences( const QgsProject *project )
{
  mLayer.resolve( project );
}

bool QgsDemTerrainSettings::equals( const QgsAbstractTerrainSettings *other ) const
{
  const QgsDemTerrainSettings *otherTerrain = dynamic_cast<const QgsDemTerrainSettings *>( other );
  if ( !otherTerrain )
    return false;

  if ( !equalsCommon( other ) )
    return false;

  return mResolution == otherTerrain->mResolution
         && qgsDoubleNear( mSkirtHeight, otherTerrain->mSkirtHeight )
         && mLayer.layerId == otherTerrain->mLayer.layerId;
}

std::unique_ptr<QgsTerrainGenerator> QgsDemTerrainSettings::createTerrainGenerator( const Qgs3DRenderContext &context ) const
{
  auto generator = std::make_unique<QgsDemTerrainGenerator>();
  generator->setCrs( context.crs(), context.transformContext() );
  generator->setExtent( context.extent() );
  generator->setLayer( layer() );
  generator->setResolution( mResolution );
  generator->setSkirtHeight( static_cast<float>( mSkirtHeight ) );
  return generator;
}

void QgsDemTerrainSettings::setLayer( QgsRasterLayer *layer )
{
  mLayer = QgsMapLayerRef( layer );
}

QgsRasterLayer *QgsDemTerrainSettings::layer() const
{
  return qobject_cast<QgsRasterLayer *>( mLayer.layer.data() );
}
