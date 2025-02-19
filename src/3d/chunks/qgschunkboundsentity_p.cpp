/***************************************************************************
  qgschunkboundsentity_p.cpp
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

#include "qgschunkboundsentity_p.h"
#include "moc_qgschunkboundsentity_p.cpp"

#include <Qt3DExtras/QPhongMaterial>

#include "qgsaabb.h"
#include "qgs3dwiredmesh_p.h"
#include "qgsbox3d.h"
#include "qgsgeotransform.h"


///@cond PRIVATE

QgsChunkBoundsEntity::QgsChunkBoundsEntity( const QgsVector3D &vertexDataOrigin, Qt3DCore::QNode *parent )
  : Qt3DCore::QEntity( parent )
  , mVertexDataOrigin( vertexDataOrigin )
{
  mAabbMesh = new Qgs3DWiredMesh;
  addComponent( mAabbMesh );

  Qt3DExtras::QPhongMaterial *bboxesMaterial = new Qt3DExtras::QPhongMaterial;
  bboxesMaterial->setAmbient( Qt::red );
  addComponent( bboxesMaterial );

  QgsGeoTransform *transform = new QgsGeoTransform;
  transform->setGeoTranslation( mVertexDataOrigin );
  addComponent( transform );
}

void QgsChunkBoundsEntity::setBoxes( const QList<QgsBox3D> &bboxes )
{
  QList<QgsAABB> aabbBoxes;
  for ( const QgsBox3D &box : bboxes )
  {
    aabbBoxes << QgsAABB::fromBox3D( box, mVertexDataOrigin );
  }
  mAabbMesh->setVertices( aabbBoxes );
}

/// @endcond
