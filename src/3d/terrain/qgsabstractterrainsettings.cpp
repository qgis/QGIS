/***************************************************************************
  qgsabstractterrainsettings.cpp
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

#include "qgsabstractterrainsettings.h"
#include "qgis.h"
#include <QDomElement>

QgsAbstractTerrainSettings::~QgsAbstractTerrainSettings() = default;

void QgsAbstractTerrainSettings::resolveReferences( const QgsProject * )
{
}

void QgsAbstractTerrainSettings::writeCommonProperties( QDomElement &element, const QgsReadWriteContext & ) const
{
  element.setAttribute( QStringLiteral( "exaggeration" ), mTerrainVerticalScale );
  element.setAttribute( QStringLiteral( "texture-size" ), mMapTileResolution );
  element.setAttribute( QStringLiteral( "max-terrain-error" ), mMaxTerrainScreenError );
  element.setAttribute( QStringLiteral( "max-ground-error" ), mMaxTerrainGroundError );
  element.setAttribute( QStringLiteral( "elevation-offset" ), mTerrainElevationOffset );
}

void QgsAbstractTerrainSettings::readCommonProperties( const QDomElement &element, const QgsReadWriteContext & )
{
  mTerrainVerticalScale = element.attribute( QStringLiteral( "exaggeration" ), QStringLiteral( "1" ) ).toDouble();
  mMapTileResolution = element.attribute( QStringLiteral( "texture-size" ), QStringLiteral( "512" ) ).toInt();
  mMaxTerrainScreenError = element.attribute( QStringLiteral( "max-terrain-error" ), QStringLiteral( "3" ) ).toDouble();
  mMaxTerrainGroundError = element.attribute( QStringLiteral( "max-ground-error" ), QStringLiteral( "1" ) ).toDouble();
  mTerrainElevationOffset = element.attribute( QStringLiteral( "elevation-offset" ), QStringLiteral( "0.0" ) ).toDouble();
}

void QgsAbstractTerrainSettings::copyCommonProperties( const QgsAbstractTerrainSettings *source )
{
  mTerrainVerticalScale = source->mTerrainVerticalScale;
  mMapTileResolution = source->mMapTileResolution;
  mMaxTerrainScreenError = source->mMaxTerrainScreenError;
  mMaxTerrainGroundError = source->mMaxTerrainGroundError;
  mTerrainElevationOffset = source->mTerrainElevationOffset;
}

bool QgsAbstractTerrainSettings::equalsCommon( const QgsAbstractTerrainSettings *other ) const
{
  return mMapTileResolution == other->mMapTileResolution
         && qgsDoubleNear( mTerrainVerticalScale, other->mTerrainVerticalScale )
         && qgsDoubleNear( mMaxTerrainScreenError, other->mMaxTerrainScreenError )
         && qgsDoubleNear( mMaxTerrainGroundError, other->mMaxTerrainGroundError )
         && qgsDoubleNear( mTerrainElevationOffset, other->mTerrainElevationOffset );
}
