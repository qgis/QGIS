/***************************************************************************
  qgsmesh3dsymbol_p.cpp
  ---------------------
  Date                 : January 2019
  Copyright            : (C) 2019 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmesh3dsymbol_p.h"

#include "qgsmesh3dsymbol.h"
#include "qgstessellatedpolygongeometry.h"
#include "qgs3dmapsettings.h"
#include "qgs3dutils.h"

#include <Qt3DCore/QTransform>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QCullFace>

#include "qgsmultipolygon.h"
#include "qgsmeshlayer.h"
#include "qgstriangularmesh.h"
#include "qgsexpressioncontextutils.h"

static QgsExpressionContext _expressionContext3D()
{
  QgsExpressionContext ctx;
  ctx << QgsExpressionContextUtils::globalScope()
      << QgsExpressionContextUtils::projectScope( QgsProject::instance() );
  return ctx;
}

/// @cond PRIVATE

QgsMesh3DSymbolEntity::QgsMesh3DSymbolEntity( const Qgs3DMapSettings &map,
    QgsMeshLayer *layer,
    const QgsMesh3DSymbol &symbol,
    Qt3DCore::QNode *parent )
  : Qt3DCore::QEntity( parent )
{
  // build the default material
  Qt3DExtras::QPhongMaterial *mat = material( symbol );

  // build a transform function
  Qt3DCore::QTransform *tform = new Qt3DCore::QTransform;
  tform->setTranslation( QVector3D( 0, 0, 0 ) );

  // build the entity
  QgsMesh3DSymbolEntityNode *entity = new QgsMesh3DSymbolEntityNode( map, layer, symbol );
  entity->findChild<Qt3DRender::QGeometryRenderer *>()->setObjectName( QStringLiteral( "main" ) ); // temporary measure to distinguish between "selected" and "main"
  entity->addComponent( mat );
  entity->addComponent( tform );
  entity->setParent( this );
}

Qt3DExtras::QPhongMaterial *QgsMesh3DSymbolEntity::material( const QgsMesh3DSymbol &symbol ) const
{
  Qt3DExtras::QPhongMaterial *material = new Qt3DExtras::QPhongMaterial;

  // front/back side culling
  auto techniques = material->effect()->techniques();
  for ( auto tit = techniques.constBegin(); tit != techniques.constEnd(); ++tit )
  {
    auto renderPasses = ( *tit )->renderPasses();
    for ( auto rpit = renderPasses.begin(); rpit != renderPasses.end(); ++rpit )
    {
      Qt3DRender::QCullFace *cullFace = new Qt3DRender::QCullFace;
      cullFace->setMode( Qt3DRender::QCullFace::Back );
      ( *rpit )->addRenderState( cullFace );
    }
  }

  material->setAmbient( symbol.material().ambient() );
  material->setDiffuse( symbol.material().diffuse() );
  material->setSpecular( symbol.material().specular() );
  material->setShininess( symbol.material().shininess() );
  return material;
}

QgsMesh3DSymbolEntityNode::QgsMesh3DSymbolEntityNode( const Qgs3DMapSettings &map,
    QgsMeshLayer *layer,
    const QgsMesh3DSymbol &symbol,
    Qt3DCore::QNode *parent ) : Qt3DCore::QEntity( parent )
{
  addComponent( renderer( map, symbol, layer ) );
}

Qt3DRender::QGeometryRenderer *QgsMesh3DSymbolEntityNode::renderer( const Qgs3DMapSettings &map,
    const QgsMesh3DSymbol &symbol,
    const QgsMeshLayer *layer )
{
  QgsPointXY origin( map.origin().x(), map.origin().y() );
  QList<QgsPolygon *> polygons;
  QList<QgsFeatureId> fids;

  QgsExpressionContext ctx( _expressionContext3D() );
  const QgsPropertyCollection &ddp = symbol.dataDefinedProperties();
  bool hasDDHeight = ddp.isActive( QgsAbstract3DSymbol::PropertyHeight );
  float height = symbol.height();
  if ( hasDDHeight )
  {
    height = static_cast<float>( ddp.valueAsDouble( QgsAbstract3DSymbol::PropertyHeight,
                                 ctx,
                                 static_cast<double>( height )
                                                  )
                               );
  }

  const QgsTriangularMesh *mesh = layer->triangularMesh();
  if ( mesh )
  {
    const QVector<QgsMeshFace> &triangles = mesh->triangles();
    const QVector<QgsMeshVertex> &vertices = mesh->vertices();
    for ( int i = 0; i < triangles.size(); ++i )
    {
      const QgsMeshFace &triangle = triangles.at( i );
      Q_ASSERT( triangle.size() == 3 );
      std::unique_ptr< QgsPolygon > polygon = QgsMeshUtils::toPolygon( triangle, vertices );
      Qgs3DUtils::clampAltitudes( polygon.get(),
                                  symbol.altitudeClamping(),
                                  Qgs3DTypes::AltitudeBinding::AltBindVertex,
                                  height,
                                  map );
      polygons.append( polygon.release() );
      fids.append( i );
    }
  }

  // Polygons from mesh are already triangles, but
  // call QgsTessellatedPolygonGeometry to
  // use symbol settings for back faces, normals, etc
  mGeometry = new QgsTessellatedPolygonGeometry( true, false, symbol.addBackFaces(), symbol.material().shouldUseDiffuseTexture() );
  QList<float> extrusionHeightPerPolygon;
  mGeometry->setPolygons( polygons, fids, origin, 0.0, extrusionHeightPerPolygon );

  Qt3DRender::QGeometryRenderer *renderer = new Qt3DRender::QGeometryRenderer;
  renderer->setGeometry( mGeometry );

  return renderer;
}
/// @endcond
