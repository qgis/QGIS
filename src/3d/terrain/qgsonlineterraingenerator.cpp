/***************************************************************************
  qgsonlineterraingenerator.cpp
  --------------------------------------
  Date                 : March 2019
  Copyright            : (C) 2019 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsonlineterraingenerator.h"

#include <memory>

#include "qgsdemterraintileloader_p.h"

#include "moc_qgsonlineterraingenerator.cpp"

QgsOnlineTerrainGenerator::QgsOnlineTerrainGenerator() = default;

QgsOnlineTerrainGenerator::~QgsOnlineTerrainGenerator() = default;

QgsTerrainGenerator::Type QgsOnlineTerrainGenerator::type() const
{
  return QgsTerrainGenerator::Online;
}

// cppcheck-suppress duplInheritedMember
QgsTerrainGenerator *QgsOnlineTerrainGenerator::create()
{
  return new QgsOnlineTerrainGenerator();
}

void QgsOnlineTerrainGenerator::updateGenerator()
{
  if ( mExtent.isNull() )
  {
    mTerrainTilingScheme = QgsTilingScheme();
  }
  else
  {
    // the real extent will be a square where the given extent fully fits
    mTerrainTilingScheme = QgsTilingScheme( mExtent, mCrs );
  }

  mHeightMapGenerator = std::make_unique<QgsDemHeightMapGenerator>( nullptr, mTerrainTilingScheme, mResolution, mTransformContext );
}
