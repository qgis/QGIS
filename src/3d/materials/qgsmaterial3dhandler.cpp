/***************************************************************************
  qgsmaterial3dhandler.cpp
  --------------------------------------
  Date                 : March 2026
  Copyright            : (C) 2026 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaterial3dhandler.h"

#include <QString>
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QPointLight>
#include <Qt3DRender/QTechnique>

using namespace Qt::StringLiterals;


QByteArray QgsAbstractMaterial3DHandler::dataDefinedVertexColorsAsByte( const QgsAbstractMaterialSettings *settings, const QgsExpressionContext &expressionContext ) const
{
  Q_UNUSED( settings )
  Q_UNUSED( expressionContext )
  return QByteArray();
}

void QgsAbstractMaterial3DHandler::applyDataDefinedToGeometry( const QgsAbstractMaterialSettings *settings, Qt3DCore::QGeometry *geometry, int vertexCount, const QByteArray &dataDefinedBytes ) const
{
  Q_UNUSED( settings )
  Q_UNUSED( geometry )
  Q_UNUSED( vertexCount )
  Q_UNUSED( dataDefinedBytes )
}

int QgsAbstractMaterial3DHandler::dataDefinedByteStride( const QgsAbstractMaterialSettings * ) const
{
  return 0;
}

Qt3DRender::QParameter *QgsAbstractMaterial3DHandler::findParameter( Qt3DRender::QEffect *effect, const QString &name )
{
  const QList<Qt3DRender::QParameter *> parameters = effect->parameters();
  for ( Qt3DRender::QParameter *parameter : parameters )
  {
    if ( parameter->name() == name )
    {
      return parameter;
    }
  }

  const QList< Qt3DRender::QTechnique *> techniques = effect->techniques();
  for ( Qt3DRender::QTechnique *technique : techniques )
  {
    const QList<Qt3DRender::QParameter *> parameters = technique->parameters();
    for ( Qt3DRender::QParameter *parameter : parameters )
    {
      if ( parameter->name() == name )
      {
        return parameter;
      }
    }
  }

  return nullptr;
}

Qt3DCore::QEntity *QgsAbstractMaterial3DHandler::createPreviewMesh( Qt3DCore::QEntity *parent ) const
{
  auto *entity = new Qt3DCore::QEntity( parent );
  auto *mesh = new Qt3DExtras::QSphereMesh( entity );
  mesh->setRadius( 1.0f );
  mesh->setRings( 32 );
  mesh->setSlices( 32 );
  entity->addComponent( mesh );
  return entity;
}

Qt3DCore::QEntity *QgsAbstractMaterial3DHandler::createPreviewScene( const QgsAbstractMaterialSettings *settings, const QgsMaterialContext &context, Qt3DExtras::Qt3DWindow *, Qt3DCore::QEntity *parent ) const
{
  auto *root = new Qt3DCore::QEntity( parent );

  Qt3DCore::QEntity *meshEntity = createPreviewMesh( root );
  Q_ASSERT( meshEntity );
  meshEntity->setObjectName( "mesh" );
  if ( QgsMaterial *mat = toMaterial( settings, Qgis::MaterialRenderingTechnique::Triangles, context ) )
  {
    mat->setParent( meshEntity );
    meshEntity->addComponent( mat );
  }

  auto *lightEntity = new Qt3DCore::QEntity( root );
  auto *light = new Qt3DRender::QPointLight( lightEntity );
  light->setColor( Qt::white );
  light->setIntensity( 1.0f );
  auto *lightTransform = new Qt3DCore::QTransform( lightEntity );
  lightTransform->setTranslation( QVector3D( 3, 3, 3 ) );
  lightEntity->addComponent( light );
  lightEntity->addComponent( lightTransform );

  return root;
}
