/***************************************************************************
                         qgsmesh3dentity.cpp
                         -------------------------
    begin                : january 2020
    copyright            : (C) 2020 by Vincent Cloarec
    email                : vcloarec at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmesh3dentity.h"

#include <QOpenGLContext>
#include <qopenglfunctions_3_2_core.h>

#include <Qt3DRender/QTexture>
#include <Qt3DRender/QParameter>
#include <Qt3DExtras/QTextureMaterial>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QMetalRoughMaterial>

#include "qgsmeshlayer.h"
#include "qgsmapsettings.h"
#include "qgs3dmapsettings.h"
#include "qgsmeshlayerrenderer.h"
#include "qgsterraintextureimage_p.h"
#include "qgsmesh3dmaterial.h"

int QgsMesh3dEntity::mMesh3DEntityCount = 0;

QgsMesh3dEntity::QgsMesh3dEntity( const Qgs3DMapSettings &map, QgsMeshLayer *layer, const QgsMesh3DSymbol &symbol ):
  mSymbol( symbol ),
  mMapSettings( map ),
  mLayer( layer )
{
  mMesh3DEntityCount++;
  name = layer->name();
  name.append( " " ).append( QString::number( mMesh3DEntityCount ) );
  qDebug() << "Create mesh Entity " << name;
  qDebug() << "Mesh 3D entity existing " << mMesh3DEntityCount;
}

void QgsMesh3dEntity::build()
{
  buildGeometry();
  applyMaterial();
}

void QgsMesh3dEntity::buildGeometry()
{
  Qt3DRender::QGeometryRenderer *mesh = new Qt3DRender::QGeometryRenderer;
  mesh->setGeometry( new QgsMesh3dGeometry_p( *mLayer->triangularMesh(), mLayer->extent(), mSymbol.verticaleScale(), mesh ) );
  addComponent( mesh );

  Qt3DCore::QTransform *tform = new Qt3DCore::QTransform;
  tform->setTranslation( QVector3D( float( -mMapSettings.origin().x() ), 0, float( mMapSettings.origin().y() ) ) );
  addComponent( tform );
}

void QgsMesh3dEntity::applyMaterial()
{
  mMaterial = new QgsMesh3dMaterial( mSymbol );
  addComponent( mMaterial );
}

///////////
int QgsMesh3dTerrainTileEntity::mMesh3DEntityCount = 0;

QgsMesh3dTerrainTileEntity::QgsMesh3dTerrainTileEntity( const Qgs3DMapSettings &map,
    QgsMeshLayer *layer,
    const QgsMesh3DSymbol &symbol,
    QgsChunkNodeId nodeId,
    Qt3DCore::QNode *parent ):  QgsTerrainTileEntity( nodeId, parent ),
  mSymbol( symbol ),
  mMapSettings( map ),
  mLayer( layer )
{
  mMesh3DEntityCount++;
  name = layer->name();
  name.append( " " ).append( QString::number( mMesh3DEntityCount ) );
  qDebug() << "Create mesh Terrain Entity " << name;
  qDebug() << "Mesh 3D entity Terrain existing " << mMesh3DEntityCount;
}

void QgsMesh3dTerrainTileEntity::build()
{
  buildGeometry();
  applyMaterial();
}

void QgsMesh3dTerrainTileEntity::buildGeometry()
{
  Qt3DRender::QGeometryRenderer *mesh = new Qt3DRender::QGeometryRenderer;
  mesh->setGeometry( new QgsMesh3dGeometry_p( *mLayer->triangularMesh(), mLayer->extent(), mSymbol.verticaleScale(), mesh ) );
  addComponent( mesh );

  Qt3DCore::QTransform *tform = new Qt3DCore::QTransform;
  tform->setTranslation( QVector3D( float( -mMapSettings.origin().x() ), 0, float( mMapSettings.origin().y() ) ) );
  addComponent( tform );
}

void QgsMesh3dTerrainTileEntity::applyMaterial()
{
  mMaterial = new QgsMesh3dMaterial( mSymbol );
  addComponent( mMaterial );
}

