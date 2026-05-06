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

#include "qgs3drendercontext.h"

#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QString>
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QConeMesh>
#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QPointLight>
#include <Qt3DRender/QTechnique>

using namespace Qt::StringLiterals;


QgsMaterialContext QgsMaterialContext::fromRenderContext( const Qgs3DRenderContext &context )
{
  QgsMaterialContext res;
  res.mSelectedColor = context.selectionColor();
  res.mTextureFilterQuality = context.textureFilterQuality();
  return res;
}

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

QList<QgsAbstractMaterial3DHandler::PreviewMeshType> QgsAbstractMaterial3DHandler::previewMeshTypes() const
{
  PreviewMeshType sphere;
  sphere.type = u"sphere"_s;
  sphere.displayName = QObject::tr( "Sphere" );

  PreviewMeshType cube;
  cube.type = u"cube"_s;
  cube.displayName = QObject::tr( "Cube" );

  PreviewMeshType cone;
  cone.type = u"cone"_s;
  cone.displayName = QObject::tr( "Cone" );

  return { sphere, cube, cone };
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

Qt3DCore::QEntity *QgsAbstractMaterial3DHandler::createPreviewMesh( const QString &type, Qt3DCore::QEntity *parent ) const
{
  auto *entity = new Qt3DCore::QEntity( parent );
  if ( type == "sphere"_L1 )
  {
    auto *mesh = new Qt3DExtras::QSphereMesh( entity );
    mesh->setRadius( 1.0f );
    mesh->setRings( 32 );
    mesh->setSlices( 32 );
    mesh->setGenerateTangents( true );
    entity->addComponent( mesh );
  }
  else if ( type == "cube"_L1 )
  {
    auto *mesh = new Qt3DExtras::QCuboidMesh( entity );
    mesh->setXExtent( 1.8f );
    mesh->setYExtent( 1.8f );
    mesh->setZExtent( 1.8f );

    auto *transform = new Qt3DCore::QTransform( mesh );
    transform->setRotation( QQuaternion::fromEulerAngles( 15, 35, 15 ) );

    entity->addComponent( mesh );
    entity->addComponent( transform );
  }
  else if ( type == "cone"_L1 )
  {
    auto *mesh = new Qt3DExtras::QConeMesh( entity );
    mesh->setBottomRadius( 1.2f );
    mesh->setLength( 1.8f );
    mesh->setRings( 32 );
    mesh->setSlices( 32 );
    auto *transform = new Qt3DCore::QTransform( mesh );
    transform->setRotation( QQuaternion::fromEulerAngles( 5, 0, 0 ) );

    entity->addComponent( mesh );
    entity->addComponent( transform );
  }
  return entity;
}

Qt3DCore::QEntity *QgsAbstractMaterial3DHandler::createPreviewScene(
  const QgsAbstractMaterialSettings *settings, const QString &type, const QgsMaterialContext &context, QWindow *, Qt3DCore::QEntity *parent
) const
{
  auto *root = new Qt3DCore::QEntity( parent );

  Qt3DCore::QEntity *meshEntity = createPreviewMesh( type, root );
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

  auto *animGroup = new QSequentialAnimationGroup( lightEntity );
  animGroup->setLoopCount( -1 );

  auto *swingLeft = new QPropertyAnimation( lightTransform, "translation", animGroup );
  swingLeft->setDuration( 8000 );
  swingLeft->setStartValue( QVector3D( 3, 3, 3 ) );
  swingLeft->setEndValue( QVector3D( -5, 5, 3 ) );
  swingLeft->setEasingCurve( QEasingCurve::InOutSine );

  auto *swingRight = new QPropertyAnimation( lightTransform, "translation", animGroup );
  swingRight->setDuration( 8000 );
  swingRight->setStartValue( QVector3D( -5, 5, 3 ) );
  swingRight->setEndValue( QVector3D( 3, 3, 3 ) );
  swingRight->setEasingCurve( QEasingCurve::InOutSine );

  animGroup->addAnimation( swingLeft );
  animGroup->addAnimation( swingRight );
  animGroup->start();

  return root;
}
