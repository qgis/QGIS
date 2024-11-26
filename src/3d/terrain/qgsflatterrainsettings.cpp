/***************************************************************************
  qgsflatterrainsettings.cpp
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

#include "qgsflatterrainsettings.h"

QgsAbstractTerrainSettings *QgsFlatTerrainSettings::create()
{
  return new QgsFlatTerrainSettings();
}

QgsFlatTerrainSettings *QgsFlatTerrainSettings::clone() const
{
  return new QgsFlatTerrainSettings( *this );
}

QString QgsFlatTerrainSettings::type() const
{
  return QStringLiteral( "flat" );
}

void QgsFlatTerrainSettings::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  readCommonProperties( element, context );
}

void QgsFlatTerrainSettings::writeXml( QDomElement &element, const QgsReadWriteContext &context ) const
{
  writeCommonProperties( element, context );
}

bool QgsFlatTerrainSettings::equals( const QgsAbstractTerrainSettings *other ) const
{
  const QgsFlatTerrainSettings *otherFlatTerrain = dynamic_cast<const QgsFlatTerrainSettings *>( other );
  if ( !otherFlatTerrain )
    return false;

  return equalsCommon( otherFlatTerrain );
}
