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
  element.setAttribute( u"exaggeration"_s, mTerrainVerticalScale );
  element.setAttribute( u"texture-size"_s, mMapTileResolution );
  element.setAttribute( u"max-terrain-error"_s, mMaxTerrainScreenError );
  element.setAttribute( u"max-ground-error"_s, mMaxTerrainGroundError );
  element.setAttribute( u"elevation-offset"_s, mTerrainElevationOffset );
}

void QgsAbstractTerrainSettings::readCommonProperties( const QDomElement &element, const QgsReadWriteContext & )
{
  mTerrainVerticalScale = element.attribute( u"exaggeration"_s, u"1"_s ).toDouble();
  mMapTileResolution = element.attribute( u"texture-size"_s, u"512"_s ).toInt();
  mMaxTerrainScreenError = element.attribute( u"max-terrain-error"_s, u"3"_s ).toDouble();
  mMaxTerrainGroundError = element.attribute( u"max-ground-error"_s, u"1"_s ).toDouble();
  mTerrainElevationOffset = element.attribute( u"elevation-offset"_s, u"0.0"_s ).toDouble();
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
