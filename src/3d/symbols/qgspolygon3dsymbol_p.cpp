/***************************************************************************
  qgspolygon3dsymbol_p.cpp
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

#include "qgspolygon3dsymbol_p.h"

#include "qgspolygon3dsymbol.h"
#include "qgstessellatedpolygongeometry.h"
#include "qgs3dmapsettings.h"
#include "qgs3dutils.h"
#include "qgstessellator.h"
#include "qgsapplication.h"
#include "qgsimagecache.h"

#include <Qt3DCore/QTransform>
#include <Qt3DRender/QMaterial>
#include <Qt3DExtras/QPhongMaterial>

#include <Qt3DExtras/QDiffuseMapMaterial>
#include <Qt3DRender/QAbstractTextureImage>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/QPaintedTextureImage>

#include <Qt3DRender/QEffect>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QCullFace>
#include <Qt3DRender/QGeometryRenderer>

#include "qgsvectorlayer.h"
#include "qgslinestring.h"
#include "qgsmultipolygon.h"

#include "qgslinevertexdata_p.h"
#include "qgslinematerial_p.h"

/// @cond PRIVATE


class QgsPolygon3DSymbolHandler : public QgsFeature3DHandler
{
  public:
    QgsPolygon3DSymbolHandler( const QgsPolygon3DSymbol &symbol, const QgsFeatureIds &selectedIds )
      : mSymbol( symbol ), mSelectedIds( selectedIds ) {}

    bool prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames ) override;
    void processFeature( QgsFeature &feature, const Qgs3DRenderContext &context ) override;
    void finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context ) override;

  private:

    //! temporary data we will pass to the tessellator
    struct PolygonData
    {
      std::unique_ptr<QgsTessellator> tessellator;
      QVector<QgsFeatureId> triangleIndexFids;
      QVector<uint> triangleIndexStartingIndices;
    };

    void processPolygon( QgsPolygon *polyClone, QgsFeatureId fid, float height, float extrusionHeight, const Qgs3DRenderContext &context, PolygonData &out );
    void makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, PolygonData &out, bool selected );
    Qt3DRender::QMaterial *material( const QgsPolygon3DSymbol &symbol, bool isSelected, const Qgs3DRenderContext &context ) const;

    // input specific for this class
    const QgsPolygon3DSymbol &mSymbol;
    // inputs - generic
    QgsFeatureIds mSelectedIds;

    // outputs
    PolygonData outNormal;  //!< Features that are not selected
    PolygonData outSelected;  //!< Features that are selected

    QgsLineVertexData outEdges;  //!< When highlighting edges, this holds data for vertex/index buffer
};


bool QgsPolygon3DSymbolHandler::prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames )
{
  outEdges.withAdjacency = true;
  outEdges.init( mSymbol.altitudeClamping(), mSymbol.altitudeBinding(), mSymbol.height(), &context.map() );

  outNormal.tessellator.reset( new QgsTessellator( context.map().origin().x(), context.map().origin().y(), true, mSymbol.invertNormals(), mSymbol.addBackFaces(), false, mSymbol.material().shouldUseDiffuseTexture(), mSymbol.renderedFacade(), mSymbol.material().textureRotation() ) );
  outSelected.tessellator.reset( new QgsTessellator( context.map().origin().x(), context.map().origin().y(), true, mSymbol.invertNormals(), mSymbol.addBackFaces(), false, mSymbol.material().shouldUseDiffuseTexture(), mSymbol.renderedFacade(), mSymbol.material().textureRotation() ) );

  QSet<QString> attrs = mSymbol.dataDefinedProperties().referencedFields( context.expressionContext() );
  attributeNames.unite( attrs );
  return true;
}

void QgsPolygon3DSymbolHandler::processPolygon( QgsPolygon *polyClone, QgsFeatureId fid, float height, float extrusionHeight, const Qgs3DRenderContext &context, PolygonData &out )
{
  if ( mSymbol.edgesEnabled() )
  {
    // add edges before the polygon gets the Z values modified because addLineString() does its own altitude handling
    outEdges.addLineString( *static_cast<const QgsLineString *>( polyClone->exteriorRing() ), height );
    for ( int i = 0; i < polyClone->numInteriorRings(); ++i )
      outEdges.addLineString( *static_cast<const QgsLineString *>( polyClone->interiorRing( i ) ), height );

    if ( extrusionHeight )
    {
      // add roof and wall edges
      const QgsLineString *exterior = static_cast<const QgsLineString *>( polyClone->exteriorRing() );
      outEdges.addLineString( *exterior, extrusionHeight + height );
      outEdges.addVerticalLines( *exterior, extrusionHeight, height );
      for ( int i = 0; i < polyClone->numInteriorRings(); ++i )
      {
        const QgsLineString *interior = static_cast<const QgsLineString *>( polyClone->interiorRing( i ) );
        outEdges.addLineString( *interior, extrusionHeight + height );
        outEdges.addVerticalLines( *interior, extrusionHeight, height );
      }
    }
  }

  Qgs3DUtils::clampAltitudes( polyClone, mSymbol.altitudeClamping(), mSymbol.altitudeBinding(), height, context.map() );

  Q_ASSERT( out.tessellator->dataVerticesCount() % 3 == 0 );
  uint startingTriangleIndex = static_cast<uint>( out.tessellator->dataVerticesCount() / 3 );
  out.triangleIndexStartingIndices.append( startingTriangleIndex );
  out.triangleIndexFids.append( fid );
  out.tessellator->addPolygon( *polyClone, extrusionHeight );
  delete polyClone;
}

void QgsPolygon3DSymbolHandler::processFeature( QgsFeature &f, const Qgs3DRenderContext &context )
{
  if ( f.geometry().isNull() )
    return;

  PolygonData &out = mSelectedIds.contains( f.id() ) ? outSelected : outNormal;

  QgsGeometry geom = f.geometry();

  // segmentize curved geometries if necessary
  if ( QgsWkbTypes::isCurvedType( geom.constGet()->wkbType() ) )
    geom = QgsGeometry( geom.constGet()->segmentize() );

  const QgsAbstractGeometry *g = geom.constGet();

  const QgsPropertyCollection &ddp = mSymbol.dataDefinedProperties();
  bool hasDDHeight = ddp.isActive( QgsAbstract3DSymbol::PropertyHeight );
  bool hasDDExtrusion = ddp.isActive( QgsAbstract3DSymbol::PropertyExtrusionHeight );

  float height = mSymbol.height();
  float extrusionHeight = mSymbol.extrusionHeight();
  if ( hasDDHeight )
    height = ddp.valueAsDouble( QgsAbstract3DSymbol::PropertyHeight, context.expressionContext(), height );
  if ( hasDDExtrusion )
    extrusionHeight = ddp.valueAsDouble( QgsAbstract3DSymbol::PropertyExtrusionHeight, context.expressionContext(), extrusionHeight );

  if ( const QgsPolygon *poly = qgsgeometry_cast< const QgsPolygon *>( g ) )
  {
    QgsPolygon *polyClone = poly->clone();
    processPolygon( polyClone, f.id(), height, extrusionHeight, context, out );
  }
  else if ( const QgsMultiPolygon *mpoly = qgsgeometry_cast< const QgsMultiPolygon *>( g ) )
  {
    for ( int i = 0; i < mpoly->numGeometries(); ++i )
    {
      const QgsAbstractGeometry *g2 = mpoly->geometryN( i );
      Q_ASSERT( QgsWkbTypes::flatType( g2->wkbType() ) == QgsWkbTypes::Polygon );
      QgsPolygon *polyClone = static_cast< const QgsPolygon *>( g2 )->clone();
      processPolygon( polyClone, f.id(), height, extrusionHeight, context, out );
    }
  }
  else if ( const QgsGeometryCollection *gc = qgsgeometry_cast< const QgsGeometryCollection *>( g ) )
  {
    for ( int i = 0; i < gc->numGeometries(); ++i )
    {
      const QgsAbstractGeometry *g2 = gc->geometryN( i );
      if ( QgsWkbTypes::flatType( g2->wkbType() ) == QgsWkbTypes::Polygon )
      {
        QgsPolygon *polyClone = static_cast< const QgsPolygon *>( g2 )->clone();
        processPolygon( polyClone, f.id(), height, extrusionHeight, context, out );
      }
    }
  }
  else
    qDebug() << "not a polygon";
}


void QgsPolygon3DSymbolHandler::finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context )
{
  // create entity for selected and not selected
  makeEntity( parent, context, outNormal, false );
  makeEntity( parent, context, outSelected, true );

  mZMin = std::min( outNormal.tessellator->zMinimum(), outSelected.tessellator->zMinimum() );
  mZMax = std::max( outNormal.tessellator->zMaximum(), outSelected.tessellator->zMaximum() );

  // add entity for edges
  if ( mSymbol.edgesEnabled() && !outEdges.indexes.isEmpty() )
  {
    QgsLineMaterial *mat = new QgsLineMaterial;
    mat->setLineColor( mSymbol.edgeColor() );
    mat->setLineWidth( mSymbol.edgeWidth() );

    Qt3DCore::QEntity *entity = new Qt3DCore::QEntity;

    // geometry renderer
    Qt3DRender::QGeometryRenderer *renderer = new Qt3DRender::QGeometryRenderer;
    renderer->setPrimitiveType( Qt3DRender::QGeometryRenderer::LineStripAdjacency );
    renderer->setGeometry( outEdges.createGeometry( entity ) );
    renderer->setVertexCount( outEdges.indexes.count() );
    renderer->setPrimitiveRestartEnabled( true );
    renderer->setRestartIndexValue( 0 );

    // make entity
    entity->addComponent( renderer );
    entity->addComponent( mat );
    entity->setParent( parent );
  }
}


void QgsPolygon3DSymbolHandler::makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, PolygonData &out, bool selected )
{
  if ( out.tessellator->dataVerticesCount() == 0 )
    return;  // nothing to show - no need to create the entity

  Qt3DRender::QMaterial *mat = material( mSymbol, selected, context );

  // extract vertex buffer data from tessellator
  QByteArray data( ( const char * )out.tessellator->data().constData(), out.tessellator->data().count() * sizeof( float ) );
  int nVerts = data.count() / out.tessellator->stride();

  QgsTessellatedPolygonGeometry *geometry = new QgsTessellatedPolygonGeometry( true, mSymbol.invertNormals(), mSymbol.addBackFaces(), mSymbol.material().shouldUseDiffuseTexture() );
  geometry->setData( data, nVerts, out.triangleIndexFids, out.triangleIndexStartingIndices );

  Qt3DRender::QGeometryRenderer *renderer = new Qt3DRender::QGeometryRenderer;
  renderer->setGeometry( geometry );

  // make entity
  Qt3DCore::QEntity *entity = new Qt3DCore::QEntity;
  entity->addComponent( renderer );
  entity->addComponent( mat );
  entity->setParent( parent );

  if ( !selected )
    entity->findChild<Qt3DRender::QGeometryRenderer *>()->setObjectName( QStringLiteral( "main" ) ); // temporary measure to distinguish between "selected" and "main"
// cppcheck wrongly believes entity will leak
// cppcheck-suppress memleak
}


static Qt3DRender::QCullFace::CullingMode _qt3DcullingMode( Qgs3DTypes::CullingMode mode )
{
  switch ( mode )
  {
    case Qgs3DTypes::NoCulling:    return Qt3DRender::QCullFace::NoCulling;
    case Qgs3DTypes::Front:        return Qt3DRender::QCullFace::Front;
    case Qgs3DTypes::Back:         return Qt3DRender::QCullFace::Back;
    case Qgs3DTypes::FrontAndBack: return Qt3DRender::QCullFace::FrontAndBack;
  }
  return Qt3DRender::QCullFace::NoCulling;
}

// front/back side culling
static void applyCullingMode( Qgs3DTypes::CullingMode cullingMode, Qt3DRender::QMaterial *material )
{
  auto techniques = material->effect()->techniques();
  for ( auto tit = techniques.constBegin(); tit != techniques.constEnd(); ++tit )
  {
    auto renderPasses = ( *tit )->renderPasses();
    for ( auto rpit = renderPasses.begin(); rpit != renderPasses.end(); ++rpit )
    {
      Qt3DRender::QCullFace *cullFace = new Qt3DRender::QCullFace;
      cullFace->setMode( _qt3DcullingMode( cullingMode ) );
      ( *rpit )->addRenderState( cullFace );
    }
  }
}


class QgsQImageTextureImage : public Qt3DRender::QPaintedTextureImage
{
  public:
    QgsQImageTextureImage( const QImage &image, Qt3DCore::QNode *parent = nullptr )
      : Qt3DRender::QPaintedTextureImage( parent )
      , mImage( image )
    {
      setSize( mImage.size() );
    }

    void paint( QPainter *painter ) override
    {
      painter->drawImage( mImage.rect(), mImage, mImage.rect() );
    }

  private:

    QImage mImage;

};


Qt3DRender::QMaterial *QgsPolygon3DSymbolHandler::material( const QgsPolygon3DSymbol &symbol, bool isSelected, const Qgs3DRenderContext &context ) const
{
  bool fitsInCache = false;
  QImage textureSourceImage;

  if ( symbol.material().shouldUseDiffuseTexture() )
    textureSourceImage = QgsApplication::imageCache()->pathAsImage( symbol.material().texturePath(), QSize(), true, 1.0, fitsInCache );
  ( void )fitsInCache;

  Qt3DRender::QMaterial *retMaterial = nullptr;
  if ( !textureSourceImage.isNull() )
  {
    Qt3DExtras::QDiffuseMapMaterial *material = new Qt3DExtras::QDiffuseMapMaterial;

    applyCullingMode( symbol.cullingMode(), material );

    QgsQImageTextureImage *textureImage = new QgsQImageTextureImage( textureSourceImage );
    material->diffuse()->addTextureImage( textureImage );

    material->diffuse()->wrapMode()->setX( Qt3DRender::QTextureWrapMode::Repeat );
    material->diffuse()->wrapMode()->setY( Qt3DRender::QTextureWrapMode::Repeat );
    material->diffuse()->wrapMode()->setZ( Qt3DRender::QTextureWrapMode::Repeat );
    material->setSpecular( symbol.material().specular() );
    material->setShininess( symbol.material().shininess() );
    material->setTextureScale( symbol.material().textureScale() );

    if ( isSelected )
    {
      // update the material with selection colors
      // TODO : dampen the color of diffuse texture
//      mat->setDiffuse( context.map().selectionColor() );
      material->setAmbient( context.map().selectionColor().darker() );
    }

    retMaterial = material;
  }
  else
  {
    Qt3DExtras::QPhongMaterial *material = new Qt3DExtras::QPhongMaterial;

    applyCullingMode( symbol.cullingMode(), material );

    material->setAmbient( symbol.material().ambient() );
    material->setDiffuse( symbol.material().diffuse() );
    material->setSpecular( symbol.material().specular() );
    material->setShininess( symbol.material().shininess() );

    if ( isSelected )
    {
      // update the material with selection colors
      material->setDiffuse( context.map().selectionColor() );
      material->setAmbient( context.map().selectionColor().darker() );
    }
    retMaterial = material;
  }

  return retMaterial;
}


// --------------


namespace Qgs3DSymbolImpl
{


  QgsFeature3DHandler *handlerForPolygon3DSymbol( QgsVectorLayer *layer, const QgsPolygon3DSymbol &symbol )
  {
    return new QgsPolygon3DSymbolHandler( symbol, layer->selectedFeatureIds() );
  }

  Qt3DCore::QEntity *entityForPolygon3DSymbol( const Qgs3DMapSettings &map, QgsVectorLayer *layer, const QgsPolygon3DSymbol &symbol )
  {
    QgsFeature3DHandler *handler = handlerForPolygon3DSymbol( layer, symbol );
    Qt3DCore::QEntity *e = entityFromHandler( handler, map, layer );
    delete handler;
    return e;
  }

}

/// @endcond
