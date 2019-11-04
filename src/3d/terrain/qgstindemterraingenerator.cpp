/***************************************************************************
  qgistindemterraingenerator.cpp
  --------------------------------------
  Date                 : october 2019
  Copyright            : (C) 2019 by Vincent Cloarec
  Email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QEntity>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DExtras/qspheregeometry.h>
#include <QApplication>

#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QTextureMaterial>
#include <Qt3DExtras/QTorusMesh>
#include <Qt3DExtras/QMetalRoughMaterial>

#include "qgschunknode_p.h"
#include "qgstindemterraingenerator.h"
#include "qgstindemterraintileloader_p.h"
#include "qgsterrainentity_p.h"
#include "qgs3dmapsettings.h"
#include "qgstriangularmesh.h"
#include "qgsterraintileentity_p.h"

QgsMeshLayer *QgsTinDemTerrainGenerator::layer() const
{
  return qobject_cast<QgsMeshLayer *>( mLayer.layer.data() );
}

void QgsTinDemTerrainGenerator::setLayer( QgsMeshLayer *layer )
{
  mLayer = QgsMapLayerRef( layer );
  updateGenerator();
}

void QgsTinDemTerrainGenerator::setCrs( const QgsCoordinateReferenceSystem &crs, const QgsCoordinateTransformContext &context )
{
  mCrs = crs;
  mTransformContext = context;
  updateGenerator();
}

void QgsTinDemTerrainGenerator::setLayerAndCrs( QgsMeshLayer *layer, const QgsCoordinateReferenceSystem &crs, const QgsCoordinateTransformContext &context )
{
  mLayer = QgsMapLayerRef( layer );
  mCrs = crs;
  mTransformContext = context;
  updateGenerator();
}

QgsChunkLoader *QgsTinDemTerrainGenerator::createChunkLoader( QgsChunkNode *node ) const
{
  return new QgsTinDemTerrainTileLoader( mTerrain, node, mTriangularMesh );
}

QgsTerrainGenerator *QgsTinDemTerrainGenerator::clone() const
{
  return new QgsTinDemTerrainGenerator;
}

QgsTerrainGenerator::Type QgsTinDemTerrainGenerator::type() const
{
  return QgsTerrainGenerator::Tin;
}

QgsRectangle QgsTinDemTerrainGenerator::extent() const
{
  if ( mLayer )
    return mLayer->extent();
  else
    return QgsRectangle();
}

void QgsTinDemTerrainGenerator::writeXml( QDomElement &elem ) const
{
  elem.setAttribute( QStringLiteral( "layer" ), mLayer.layerId );
}

void QgsTinDemTerrainGenerator::readXml( const QDomElement &elem )
{
  mLayer = QgsMapLayerRef( elem.attribute( QStringLiteral( "layer" ) ) );
}

void QgsTinDemTerrainGenerator::resolveReferences( const QgsProject &project )
{
  mLayer = QgsMapLayerRef( project.mapLayer( mLayer.layerId ) );
  layer()->reload();
  updateGenerator();
}

void QgsTinDemTerrainGenerator::updateGenerator()
{
  QgsMeshLayer *dem = layer();
  if ( dem )
  {
    QgsRectangle te = dem->extent();
    QgsCoordinateTransform terrainToMapTransform( dem->crs(), mCrs, mTransformContext );
    te = terrainToMapTransform.transformBoundingBox( te );
    mTerrainTilingScheme = QgsTilingScheme( te, mCrs );

    QgsRenderContext renderContext;
    renderContext.setTransformContext( mTransformContext );
    renderContext.setCoordinateTransform( terrainToMapTransform );
    mTriangularMesh.update( dem->nativeMesh(), &renderContext );
  }
  else
  {
    mTerrainTilingScheme = QgsTilingScheme();
    mTriangularMesh = QgsTriangularMesh();
  }
}


