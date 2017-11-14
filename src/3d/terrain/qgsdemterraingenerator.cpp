/***************************************************************************
  qgsdemterraingenerator.cpp
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdemterraingenerator.h"

#include "qgsdemterraintileloader_p.h"

#include "qgsrasterlayer.h"

QgsDemTerrainGenerator::~QgsDemTerrainGenerator()
{
  delete mHeightMapGenerator;
}

void QgsDemTerrainGenerator::setLayer( QgsRasterLayer *layer )
{
  mLayer = QgsMapLayerRef( layer );
  updateGenerator();
}

QgsRasterLayer *QgsDemTerrainGenerator::layer() const
{
  return qobject_cast<QgsRasterLayer *>( mLayer.layer.data() );
}

QgsTerrainGenerator *QgsDemTerrainGenerator::clone() const
{
  QgsDemTerrainGenerator *cloned = new QgsDemTerrainGenerator;
  cloned->mLayer = mLayer;
  cloned->mResolution = mResolution;
  cloned->mSkirtHeight = mSkirtHeight;
  cloned->updateGenerator();
  return cloned;
}

QgsTerrainGenerator::Type QgsDemTerrainGenerator::type() const
{
  return QgsTerrainGenerator::Dem;
}

QgsRectangle QgsDemTerrainGenerator::extent() const
{
  return mTerrainTilingScheme.tileToExtent( 0, 0, 0 );
}

float QgsDemTerrainGenerator::heightAt( double x, double y, const Qgs3DMapSettings &map ) const
{
  Q_UNUSED( map );
  return mHeightMapGenerator->heightAt( x, y );
}

void QgsDemTerrainGenerator::writeXml( QDomElement &elem ) const
{
  elem.setAttribute( "layer", mLayer.layerId );
  elem.setAttribute( "resolution", mResolution );
  elem.setAttribute( "skirt-height", mSkirtHeight );
}

void QgsDemTerrainGenerator::readXml( const QDomElement &elem )
{
  mLayer = QgsMapLayerRef( elem.attribute( "layer" ) );
  mResolution = elem.attribute( "resolution" ).toInt();
  mSkirtHeight = elem.attribute( "skirt-height" ).toFloat();
}

void QgsDemTerrainGenerator::resolveReferences( const QgsProject &project )
{
  mLayer = QgsMapLayerRef( project.mapLayer( mLayer.layerId ) );
  updateGenerator();
}

QgsChunkLoader *QgsDemTerrainGenerator::createChunkLoader( QgsChunkNode *node ) const
{
  return new QgsDemTerrainTileLoader( mTerrain, node );
}

void QgsDemTerrainGenerator::updateGenerator()
{
  QgsRasterLayer *dem = layer();
  if ( dem )
  {
    mTerrainTilingScheme = QgsTilingScheme( dem->extent(), dem->crs() );
    delete mHeightMapGenerator;
    mHeightMapGenerator = new QgsDemHeightMapGenerator( dem, mTerrainTilingScheme, mResolution );
  }
  else
  {
    mTerrainTilingScheme = QgsTilingScheme();
    delete mHeightMapGenerator;
    mHeightMapGenerator = nullptr;
  }
}
