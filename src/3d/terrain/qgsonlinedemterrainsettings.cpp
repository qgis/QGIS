/***************************************************************************
  qgsonlinedemterrainsettings.cpp
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

#include "qgsonlinedemterrainsettings.h"
#include "qgsonlineterraingenerator.h"
#include "qgis.h"
#include "qgs3drendercontext.h"

#include <QDomElement>

QgsAbstractTerrainSettings *QgsOnlineDemTerrainSettings::create()
{
  return new QgsOnlineDemTerrainSettings();
}

QgsOnlineDemTerrainSettings *QgsOnlineDemTerrainSettings::clone() const
{
  return new QgsOnlineDemTerrainSettings( *this );
}

QString QgsOnlineDemTerrainSettings::type() const
{
  return QStringLiteral( "online" );
}

void QgsOnlineDemTerrainSettings::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  if ( element.hasAttribute( QStringLiteral( "resolution" ) ) )
  {
    mResolution = element.attribute( QStringLiteral( "resolution" ) ).toInt();
    mSkirtHeight = element.attribute( QStringLiteral( "skirt-height" ) ).toDouble();
  }
  else
  {
    // restore old project
    const QDomElement elemTerrainGenerator = element.firstChildElement( QStringLiteral( "generator" ) );
    mResolution = elemTerrainGenerator.attribute( QStringLiteral( "resolution" ) ).toInt();
    mSkirtHeight = elemTerrainGenerator.attribute( QStringLiteral( "skirt-height" ) ).toDouble();
  }

  readCommonProperties( element, context );
}

void QgsOnlineDemTerrainSettings::writeXml( QDomElement &element, const QgsReadWriteContext &context ) const
{
  element.setAttribute( QStringLiteral( "resolution" ), mResolution );
  element.setAttribute( QStringLiteral( "skirt-height" ), mSkirtHeight );

  writeCommonProperties( element, context );
}

bool QgsOnlineDemTerrainSettings::equals( const QgsAbstractTerrainSettings *other ) const
{
  const QgsOnlineDemTerrainSettings *otherTerrain = dynamic_cast<const QgsOnlineDemTerrainSettings *>( other );
  if ( !otherTerrain )
    return false;

  if ( !equalsCommon( other ) )
    return false;

  return mResolution == otherTerrain->mResolution
         && qgsDoubleNear( mSkirtHeight, otherTerrain->mSkirtHeight );
}

std::unique_ptr<QgsTerrainGenerator> QgsOnlineDemTerrainSettings::createTerrainGenerator( const Qgs3DRenderContext &context ) const
{
  std::unique_ptr<QgsOnlineTerrainGenerator> generator = std::make_unique<QgsOnlineTerrainGenerator>();
  generator->setResolution( mResolution );
  generator->setSkirtHeight( static_cast<float>( mSkirtHeight ) );
  generator->setCrs( context.crs(), context.transformContext() );
  generator->setExtent( context.extent() );
  return generator;
}
