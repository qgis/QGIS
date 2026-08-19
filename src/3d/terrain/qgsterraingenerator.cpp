/***************************************************************************
  qgsterraingenerator.cpp
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

#include "qgsterraingenerator.h"

#include "qgs3d.h"
#include "qgs3dmapsettings.h"
#include "qgs3dutils.h"
#include "qgsabstractterrainsettings.h"
#include "qgscoordinatetransform.h"
#include "qgsphongtexturedmaterial.h"
#include "qgsterrainentity.h"
#include "qgsterraintexturegenerator_p.h"
#include "qgsterraintextureimage_p.h"
#include "qgsterraintileentity_p.h"
#include "qgstexturematerial.h"
#include "qgsthreadingutils.h"

#include <QString>
#include <QTechnique>

#include "moc_qgsterraingenerator.cpp"

using namespace Qt::StringLiterals;

QgsTerrainGenerator::QgsTerrainGenerator() = default;
QgsTerrainGenerator::~QgsTerrainGenerator() = default;

QgsBox3D QgsTerrainGenerator::rootChunkBox3D( const Qgs3DMapSettings &map ) const
{
  QgsRectangle te = Qgs3DUtils::tryReprojectExtent2D( rootChunkExtent(), crs(), map.crs(), map.transformContext() );

  float hMin, hMax;
  rootChunkHeightRange( hMin, hMax );
  return QgsBox3D( te.xMinimum(), te.yMinimum(), hMin, te.xMaximum(), te.yMaximum(), hMax );
}

float QgsTerrainGenerator::rootChunkError( const Qgs3DMapSettings &map ) const
{
  QgsRectangle te = Qgs3DUtils::tryReprojectExtent2D( rootChunkExtent(), crs(), map.crs(), map.transformContext() );

  // use texel size as the error
  return te.width() / map.terrainSettings()->mapTileResolution();
}

void QgsTerrainGenerator::rootChunkHeightRange( float &hMin, float &hMax ) const
{
  // TODO: makes sense to have kind of default implementation?
  hMin = Qgs3DUtils::MINIMUM_VECTOR_Z_ESTIMATE;
  hMax = Qgs3DUtils::MAXIMUM_VECTOR_Z_ESTIMATE;
}

QString QgsTerrainGenerator::typeToString( QgsTerrainGenerator::Type type )
{
  switch ( type )
  {
    case QgsTerrainGenerator::Flat:
      return u"flat"_s;
    case QgsTerrainGenerator::Dem:
      return u"dem"_s;
    case QgsTerrainGenerator::Online:
      return u"online"_s;
    case QgsTerrainGenerator::Mesh:
      return u"mesh"_s;
    case QgsTerrainGenerator::QuantizedMesh:
      return u"quantizedmesh"_s;
  }
  return QString();
}

void QgsTerrainGenerator::setCrs( const QgsCoordinateReferenceSystem &, const QgsCoordinateTransformContext & )
{}

bool QgsTerrainGenerator::isValid() const
{
  return mIsValid;
}

QFuture<QgsChunkLoaderResult> QgsTerrainGenerator::updateChunk( QgsChunkNode *node )
{
  QgsTerrainTileEntity *entity = qobject_cast<QgsTerrainTileEntity *>( node->entity() );
  return mTerrain->textureGenerator()->render( entity->textureImage()->imageExtent(), node->tileId(), entity->textureImage()->imageDebugText() ).then( this, [entity]( QImage image ) {
    return QgsChunkLoaderResult { [entity, image]( Qt3DCore::QEntity *parent ) {
      QGIS_CHECK_MAIN_THREAD_ACCESS
      entity->textureImage()->setImage( image );
      entity->setParent( parent );
      return entity;
    } };
  } );
}

QgsTerrainGenerator::Capabilities QgsTerrainGenerator::capabilities() const
{
  return QgsTerrainGenerator::Capability::NoCapabilities;
}

QFuture<QgsTerrainGenerator::TerrainTextureResources> QgsTerrainGenerator::loadTextureResources( QgsChunkNode *node )
{
  const Qgs3DMapSettings *map = mTerrain->mapSettings();
  const QgsChunkNodeId nodeId = node->tileId();
  const QgsRectangle extentTerrainCrs = map->terrainGenerator()->tilingScheme().tileToExtent( nodeId );
  const QgsRectangle extentMapCrs = Qgs3DUtils::tryReprojectExtent2D( extentTerrainCrs, map->terrainGenerator()->crs(), map->crs(), map->transformContext() );
  const QString tileDebugText = node->tileId().text();

  return mTerrain->textureGenerator()->render( extentMapCrs, node->tileId(), tileDebugText ).then( this, [extentTerrainCrs, extentMapCrs, tileDebugText]( QImage img ) {
    return TerrainTextureResources {
      img,
      extentTerrainCrs,
      extentMapCrs,
      tileDebugText,
    };
  } );
}

void QgsTerrainGenerator::createTextureComponent(
  const TerrainTextureResources &resources, QgsTerrainTileEntity *entity, bool isShadingEnabled, const QgsPhongMaterialSettings &shadingMaterial, bool useTexture, const Qgs3DRenderContext &context
)
{
  QgsMaterialContext materialContext = QgsMaterialContext::fromRenderContext( context );
  Qt3DRender::QTexture2D *texture = useTexture || !isShadingEnabled ? createTexture( entity, materialContext, resources ) : nullptr;

  QgsMaterial *material = nullptr;
  if ( texture )
  {
    if ( isShadingEnabled )
    {
      QgsPhongTexturedMaterial *phongTexturedMaterial = new QgsPhongTexturedMaterial();
      phongTexturedMaterial->setAmbient( shadingMaterial.ambient() );
      phongTexturedMaterial->setSpecular( shadingMaterial.specular() );
      phongTexturedMaterial->setShininess( static_cast<float>( shadingMaterial.shininess() ) );
      phongTexturedMaterial->setDiffuseTexture( texture );
      phongTexturedMaterial->setOpacity( static_cast<float>( shadingMaterial.opacity() ) );
      material = phongTexturedMaterial;
    }
    else
    {
      QgsTextureMaterial *textureMaterial = new QgsTextureMaterial;
      textureMaterial->setTexture( texture );
      material = textureMaterial;
    }
  }
  else
  {
    materialContext.setIsSelected( false );
    material = Qgs3D::toMaterial( &shadingMaterial, Qgis::MaterialRenderingTechnique::Triangles, materialContext );
  }

  // no backface culling on terrain, to allow terrain to be viewed from underground
  const QVector<Qt3DRender::QTechnique *> techniques = material->effect()->techniques();
  for ( Qt3DRender::QTechnique *technique : techniques )
  {
    const QVector<Qt3DRender::QRenderPass *> passes = technique->renderPasses();
    for ( Qt3DRender::QRenderPass *pass : passes )
    {
      Qt3DRender::QCullFace *cullFace = new Qt3DRender::QCullFace;
      cullFace->setMode( Qt3DRender::QCullFace::NoCulling );
      pass->addRenderState( cullFace );
    }
  }

  entity->addComponent( material ); // takes ownership if the component has no parent
}

Qt3DRender::QTexture2D *QgsTerrainGenerator::createTexture( QgsTerrainTileEntity *entity, const QgsMaterialContext &context, const QgsTerrainGenerator::TerrainTextureResources &resources ) const
{
  Qt3DRender::QTexture2D *texture = new Qt3DRender::QTexture2D;
  QgsTerrainTextureImage *textureImage = new QgsTerrainTextureImage( resources.image, resources.extentMapCrs, resources.tileDebugText );
  Qgs3DUtils::setTextureFiltering( texture, context );
  texture->setFormat( Qt3DRender::QAbstractTexture::SRGB8_Alpha8 );
  texture->addTextureImage( textureImage ); //texture take the ownership of textureImage if has no parant

  entity->setTextureImage( textureImage );

  return texture;
}
