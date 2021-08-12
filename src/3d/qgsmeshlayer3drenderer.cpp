/***************************************************************************
  qgsmeshlayer3drenderer.cpp
  --------------------------
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

#include "qgsmeshlayer3drenderer.h"

#include "qgsmeshlayer3drenderer.h"
#include "qgsmesh3dsymbol.h"
#include "qgsmesh3dsymbol_p.h"

#include "qgsmeshlayer.h"
#include "qgsxmlutils.h"
#include "qgsmesh3dentity_p.h"

QgsMeshLayer3DRendererMetadata::QgsMeshLayer3DRendererMetadata()
  : Qgs3DRendererAbstractMetadata( QStringLiteral( "mesh" ) )
{
}

QgsAbstract3DRenderer *QgsMeshLayer3DRendererMetadata::createRenderer( QDomElement &elem, const QgsReadWriteContext &context )
{
  QgsMeshLayer3DRenderer *r = new QgsMeshLayer3DRenderer;
  r->readXml( elem, context );
  return r;
}

// ---------


QgsMeshLayer3DRenderer::QgsMeshLayer3DRenderer( QgsMesh3DSymbol *s )
  : mSymbol( s )
{
}

QgsMeshLayer3DRenderer *QgsMeshLayer3DRenderer::clone() const
{
  QgsMeshLayer3DRenderer *r = new QgsMeshLayer3DRenderer( mSymbol ? ( QgsMesh3DSymbol * )mSymbol->clone() : nullptr );
  r->mLayerRef = mLayerRef;
  return r;
}

void QgsMeshLayer3DRenderer::setLayer( QgsMeshLayer *layer )
{
  mLayerRef = QgsMapLayerRef( layer );
}

QgsMeshLayer *QgsMeshLayer3DRenderer::layer() const
{
  return qobject_cast<QgsMeshLayer *>( mLayerRef.layer );
}

void QgsMeshLayer3DRenderer::setSymbol( QgsMesh3DSymbol *symbol )
{
  mSymbol.reset( symbol );
}

const QgsMesh3DSymbol *QgsMeshLayer3DRenderer::symbol() const
{
  return mSymbol.get();
}

Qt3DCore::QEntity *QgsMeshLayer3DRenderer::createEntity( const Qgs3DMapSettings &map ) const
{
  QgsMeshLayer *meshLayer = layer();

  if ( !meshLayer || !meshLayer->dataProvider() )
    return nullptr;

  if ( meshLayer->dataProvider()->contains( QgsMesh::ElementType::Edge ) ||
       !mSymbol->isEnabled() )
  {
    // 3D not implemented for 1D meshes
    return nullptr;
  }

  Qt3DCore::QEntity *entity = nullptr;


  const QgsCoordinateTransform coordTrans( meshLayer->crs(), map.crs(), map.transformContext() );
  meshLayer->updateTriangularMesh( coordTrans );
  const QgsTriangularMesh triangularMesh = *meshLayer->triangularMeshByLodIndex( mSymbol->levelOfDetailIndex() );
  QgsMeshDataset3dEntity *meshEntity = new QgsMeshDataset3dEntity( map, triangularMesh, meshLayer, mSymbol.get() );
  meshEntity->build();
  entity = meshEntity;

  return entity;
}

void QgsMeshLayer3DRenderer::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  QDomDocument doc = elem.ownerDocument();

  elem.setAttribute( QStringLiteral( "layer" ), mLayerRef.layerId );

  QDomElement elemSymbol = doc.createElement( QStringLiteral( "symbol" ) );
  if ( mSymbol )
  {
    elemSymbol.setAttribute( QStringLiteral( "type" ), mSymbol->type() );
    mSymbol->writeXml( elemSymbol, context );
  }
  elem.appendChild( elemSymbol );
}

void QgsMeshLayer3DRenderer::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  mLayerRef = QgsMapLayerRef( elem.attribute( QStringLiteral( "layer" ) ) );

  const QDomElement elemSymbol = elem.firstChildElement( QStringLiteral( "symbol" ) );
  QgsMesh3DSymbol *symbol = new QgsMesh3DSymbol;
  symbol->readXml( elemSymbol, context );
  mSymbol.reset( symbol );
}

void QgsMeshLayer3DRenderer::resolveReferences( const QgsProject &project )
{
  mLayerRef.setLayer( project.mapLayer( mLayerRef.layerId ) );
}
