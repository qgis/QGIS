/***************************************************************************
  qgslinevertexdata_p.cpp
  --------------------------------------
  Date                 : Apr 2019
  Copyright            : (C) 2019 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslinevertexdata_p.h"

#include "qgs3dutils.h"
#include "qgslinematerial_p.h"
#include "qgslinestring.h"
#include "qgsmaterial.h"
#include "qgsmaterial3dhandler.h"

#include <QString>
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QGeometry>
#include <Qt3DRender/QGeometryRenderer>

using namespace Qt::StringLiterals;

/// @cond PRIVATE

static QByteArray toByteArray( const QVector<QVector3D> &points )
{
  QByteArray data;
  data.resize( points.size() * 3 * sizeof( float ) );
  float *raw = reinterpret_cast<float *>( data.data() );
  int idx = 0;
  for ( const QVector3D &p : points )
  {
    raw[idx++] = p.x();
    raw[idx++] = p.y();
    raw[idx++] = p.z();
  }
  return data;
}

void QgsLineVertexData::init( Qgis::AltitudeClamping clamping, Qgis::AltitudeBinding binding, float height, const Qgs3DRenderContext &context, const QgsVector3D &chunkOrigin )
{
  altClamping = clamping;
  altBinding = binding;
  baseHeight = height;
  renderContext = context;
  origin = chunkOrigin;
}

void QgsLineVertexData::addLineString( const QgsLineString &lineStringIn, float extraHeightOffset, bool closePolygon )
{
  std::unique_ptr<QgsLineString> line( lineStringIn.clone() );
  line->removeDuplicateNodes();

  const int vertexCount = line->vertexCount();
  if ( vertexCount == 0 )
    return;

  QgsPoint centroid;
  switch ( altBinding )
  {
    case Qgis::AltitudeBinding::Vertex:
      break;
    case Qgis::AltitudeBinding::Centroid:
      centroid = line->centroid();
      break;
  }

  const auto pointAt = [&]( int i ) -> QVector3D {
    const QgsPoint p = line->pointN( i );
    if ( geocentricCoordinates )
    {
      // TODO: implement altitude clamping when dealing with geocentric coordinates
      // where Z coordinate is not altitude and can't be used directly...
      return QVector3D( static_cast<float>( p.x() - origin.x() ), static_cast<float>( p.y() - origin.y() ), static_cast<float>( p.z() - origin.z() ) );
    }
    const float z = Qgs3DUtils::clampAltitude( p, altClamping, altBinding, baseHeight + extraHeightOffset, centroid, renderContext );
    return QVector3D( static_cast<float>( p.x() - origin.x() ), static_cast<float>( p.y() - origin.y() ), static_cast<float>( z - origin.z() ) );
  };

  const QVector3D first = pointAt( 0 );
  vertices << first;
  QVector3D prevPrev = first;
  QVector3D prev = first;
  bool havePrevPrev = false;
  for ( int i = 1; i < vertexCount; ++i )
  {
    const QVector3D curr = pointAt( i );
    pointsA << prev;
    pointsB << curr;
    vertices << curr;

    if ( i == 1 )
      pointsPrev << ( closePolygon ? pointAt( vertexCount - 1 ) : prev + ( prev - curr ) );
    else
      pointsPrev << prevPrev;

    if ( i == vertexCount - 1 )
      pointsNext << ( closePolygon ? first : curr + ( curr - prev ) );
    else
      pointsNext << pointAt( i + 1 );

    if ( havePrevPrev )
    {
      joinPointA << prevPrev;
      joinPointB << prev;
      joinPointC << curr;
    }

    prevPrev = prev;
    prev = curr;
    havePrevPrev = true;
  }

  if ( closePolygon && vertexCount >= 2 )
  {
    pointsA << prev;
    pointsB << first;
    pointsPrev << prevPrev;
    pointsNext << pointAt( 1 );

    if ( vertexCount >= 3 )
    {
      joinPointA << prevPrev;
      joinPointB << prev;
      joinPointC << first;

      joinPointA << prev;
      joinPointB << first;
      joinPointC << pointAt( 1 );
    }
  }
}

void QgsLineVertexData::addVerticalLines( const QgsLineString &lineString, float verticalLength, float extraHeightOffset )
{
  QgsPoint centroid;
  switch ( altBinding )
  {
    case Qgis::AltitudeBinding::Vertex:
      break;
    case Qgis::AltitudeBinding::Centroid:
      centroid = lineString.centroid();
      break;
  }

  for ( int i = 0; i < lineString.vertexCount(); ++i )
  {
    const QgsPoint p = lineString.pointN( i );
    const float z = Qgs3DUtils::clampAltitude( p, altClamping, altBinding, baseHeight + extraHeightOffset, centroid, renderContext );
    const float x = static_cast<float>( p.x() - origin.x() );
    const float y = static_cast<float>( p.y() - origin.y() );
    const QVector3D a( x, y, z );
    const QVector3D b( x, y, z + verticalLength );
    pointsA << a;
    pointsB << b;
    pointsPrev << a + ( a - b );
    pointsNext << b + ( b - a );
  }
}

QByteArray QgsLineVertexData::createPointABuffer() const
{
  return toByteArray( pointsA );
}

QByteArray QgsLineVertexData::createPointBBuffer() const
{
  return toByteArray( pointsB );
}

QByteArray QgsLineVertexData::createPointPrevBuffer() const
{
  return toByteArray( pointsPrev );
}

QByteArray QgsLineVertexData::createPointNextBuffer() const
{
  return toByteArray( pointsNext );
}

QByteArray QgsLineVertexData::createJoinPointABuffer() const
{
  return toByteArray( joinPointA );
}

QByteArray QgsLineVertexData::createJoinPointBBuffer() const
{
  return toByteArray( joinPointB );
}

QByteArray QgsLineVertexData::createJoinPointCBuffer() const
{
  return toByteArray( joinPointC );
}

Qt3DCore::QGeometry *QgsLineVertexData::createGeometry( Qt3DCore::QNode *parent )
{
  Q_ASSERT( pointsA.size() == pointsB.size() );
  Q_ASSERT( pointsA.size() == pointsPrev.size() );
  Q_ASSERT( pointsA.size() == pointsNext.size() );

  static constexpr float quadVertices[] = {
    0.0f,
    -0.5f,
    1.0f,
    -0.5f,
    1.0f,
    0.5f,
    0.0f,
    -0.5f,
    1.0f,
    0.5f,
    0.0f,
    0.5f,
  };

  Qt3DCore::QBuffer *quadBuffer = new Qt3DCore::QBuffer( parent );
  quadBuffer->setData( QByteArray( reinterpret_cast<const char *>( quadVertices ), sizeof( quadVertices ) ) );

  Qt3DCore::QAttribute *positionAttribute = new Qt3DCore::QAttribute( parent );
  positionAttribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
  positionAttribute->setBuffer( quadBuffer );
  positionAttribute->setVertexBaseType( Qt3DCore::QAttribute::Float );
  positionAttribute->setVertexSize( 2 );
  positionAttribute->setByteStride( 2 * sizeof( float ) );
  positionAttribute->setByteOffset( 0 );
  positionAttribute->setCount( 6 );
  positionAttribute->setName( Qt3DCore::QAttribute::defaultPositionAttributeName() );

  Qt3DCore::QBuffer *pointABuffer = new Qt3DCore::QBuffer( parent );
  pointABuffer->setData( createPointABuffer() );

  Qt3DCore::QAttribute *pointAAttribute = new Qt3DCore::QAttribute( parent );
  pointAAttribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
  pointAAttribute->setBuffer( pointABuffer );
  pointAAttribute->setVertexBaseType( Qt3DCore::QAttribute::Float );
  pointAAttribute->setVertexSize( 3 );
  pointAAttribute->setByteStride( 3 * sizeof( float ) );
  pointAAttribute->setByteOffset( 0 );
  pointAAttribute->setCount( pointsA.size() );
  pointAAttribute->setDivisor( 1 );
  pointAAttribute->setName( u"pointA"_s );

  Qt3DCore::QBuffer *pointBBuffer = new Qt3DCore::QBuffer( parent );
  pointBBuffer->setData( createPointBBuffer() );

  Qt3DCore::QAttribute *pointBAttribute = new Qt3DCore::QAttribute( parent );
  pointBAttribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
  pointBAttribute->setBuffer( pointBBuffer );
  pointBAttribute->setVertexBaseType( Qt3DCore::QAttribute::Float );
  pointBAttribute->setVertexSize( 3 );
  pointBAttribute->setByteStride( 3 * sizeof( float ) );
  pointBAttribute->setByteOffset( 0 );
  pointBAttribute->setCount( pointsB.size() );
  pointBAttribute->setDivisor( 1 );
  pointBAttribute->setName( u"pointB"_s );

  Qt3DCore::QBuffer *pointPrevBuffer = new Qt3DCore::QBuffer( parent );
  pointPrevBuffer->setData( createPointPrevBuffer() );

  Qt3DCore::QAttribute *pointPrevAttribute = new Qt3DCore::QAttribute( parent );
  pointPrevAttribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
  pointPrevAttribute->setBuffer( pointPrevBuffer );
  pointPrevAttribute->setVertexBaseType( Qt3DCore::QAttribute::Float );
  pointPrevAttribute->setVertexSize( 3 );
  pointPrevAttribute->setByteStride( 3 * sizeof( float ) );
  pointPrevAttribute->setByteOffset( 0 );
  pointPrevAttribute->setCount( pointsPrev.size() );
  pointPrevAttribute->setDivisor( 1 );
  pointPrevAttribute->setName( u"pointPrev"_s );

  Qt3DCore::QBuffer *pointNextBuffer = new Qt3DCore::QBuffer( parent );
  pointNextBuffer->setData( createPointNextBuffer() );

  Qt3DCore::QAttribute *pointNextAttribute = new Qt3DCore::QAttribute( parent );
  pointNextAttribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
  pointNextAttribute->setBuffer( pointNextBuffer );
  pointNextAttribute->setVertexBaseType( Qt3DCore::QAttribute::Float );
  pointNextAttribute->setVertexSize( 3 );
  pointNextAttribute->setByteStride( 3 * sizeof( float ) );
  pointNextAttribute->setByteOffset( 0 );
  pointNextAttribute->setCount( pointsNext.size() );
  pointNextAttribute->setDivisor( 1 );
  pointNextAttribute->setName( u"pointNext"_s );

  Qt3DCore::QGeometry *geom = new Qt3DCore::QGeometry;
  geom->addAttribute( positionAttribute );
  geom->addAttribute( pointAAttribute );
  geom->addAttribute( pointBAttribute );
  geom->addAttribute( pointPrevAttribute );
  geom->addAttribute( pointNextAttribute );

  geom->setBoundingVolumePositionAttribute( pointAAttribute );

  return geom;
}

Qt3DCore::QGeometry *QgsLineVertexData::createJoinGeometry( Qt3DCore::QNode *parent )
{
  static constexpr float selectorVertices[] = {
    // triangle: p0, p1, p2
    1.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f,
    1.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f,
    1.0f,
    0.0f,
    // triangle: p0, p2, p1Inner
    1.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f,
    1.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f,
    1.0f,
  };

  Qt3DCore::QBuffer *selectorBuffer = new Qt3DCore::QBuffer( parent );
  selectorBuffer->setData( QByteArray( reinterpret_cast<const char *>( selectorVertices ), sizeof( selectorVertices ) ) );

  Qt3DCore::QAttribute *positionAttribute = new Qt3DCore::QAttribute( parent );
  positionAttribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
  positionAttribute->setBuffer( selectorBuffer );
  positionAttribute->setVertexBaseType( Qt3DCore::QAttribute::Float );
  positionAttribute->setVertexSize( 4 );
  positionAttribute->setByteStride( 4 * sizeof( float ) );
  positionAttribute->setByteOffset( 0 );
  positionAttribute->setCount( 6 );
  positionAttribute->setName( Qt3DCore::QAttribute::defaultPositionAttributeName() );

  Qt3DCore::QBuffer *pointABuffer = new Qt3DCore::QBuffer( parent );
  pointABuffer->setData( createJoinPointABuffer() );

  Qt3DCore::QAttribute *pointAAttribute = new Qt3DCore::QAttribute( parent );
  pointAAttribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
  pointAAttribute->setBuffer( pointABuffer );
  pointAAttribute->setVertexBaseType( Qt3DCore::QAttribute::Float );
  pointAAttribute->setVertexSize( 3 );
  pointAAttribute->setByteStride( 3 * sizeof( float ) );
  pointAAttribute->setByteOffset( 0 );
  pointAAttribute->setCount( joinPointA.size() );
  pointAAttribute->setDivisor( 1 );
  pointAAttribute->setName( u"pointA"_s );

  Qt3DCore::QBuffer *pointBBuffer = new Qt3DCore::QBuffer( parent );
  pointBBuffer->setData( createJoinPointBBuffer() );

  Qt3DCore::QAttribute *pointBAttribute = new Qt3DCore::QAttribute( parent );
  pointBAttribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
  pointBAttribute->setBuffer( pointBBuffer );
  pointBAttribute->setVertexBaseType( Qt3DCore::QAttribute::Float );
  pointBAttribute->setVertexSize( 3 );
  pointBAttribute->setByteStride( 3 * sizeof( float ) );
  pointBAttribute->setByteOffset( 0 );
  pointBAttribute->setCount( joinPointB.size() );
  pointBAttribute->setDivisor( 1 );
  pointBAttribute->setName( u"pointB"_s );

  Qt3DCore::QBuffer *pointCBuffer = new Qt3DCore::QBuffer( parent );
  pointCBuffer->setData( createJoinPointCBuffer() );

  Qt3DCore::QAttribute *pointCAttribute = new Qt3DCore::QAttribute( parent );
  pointCAttribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
  pointCAttribute->setBuffer( pointCBuffer );
  pointCAttribute->setVertexBaseType( Qt3DCore::QAttribute::Float );
  pointCAttribute->setVertexSize( 3 );
  pointCAttribute->setByteStride( 3 * sizeof( float ) );
  pointCAttribute->setByteOffset( 0 );
  pointCAttribute->setCount( joinPointC.size() );
  pointCAttribute->setDivisor( 1 );
  pointCAttribute->setName( u"pointC"_s );

  Qt3DCore::QGeometry *geom = new Qt3DCore::QGeometry;
  geom->addAttribute( positionAttribute );
  geom->addAttribute( pointAAttribute );
  geom->addAttribute( pointBAttribute );
  geom->addAttribute( pointCAttribute );

  geom->setBoundingVolumePositionAttribute( pointBAttribute );

  return geom;
}

Qt3DCore::QEntity *QgsLineVertexData::createSegmentEntity( QgsMaterial *material, const QgsAbstractMaterialSettings *materialSettings, const QgsAbstractMaterial3DHandler *materialHandler )
{
  if ( pointsA.isEmpty() )
    return nullptr;

  Qt3DCore::QEntity *entity = new Qt3DCore::QEntity;

  Qt3DRender::QGeometryRenderer *renderer = new Qt3DRender::QGeometryRenderer;
  renderer->setPrimitiveType( Qt3DRender::QGeometryRenderer::Triangles );
  Qt3DCore::QGeometry *geometry = createGeometry( entity );

  if ( materialHandler && materialSettings && !materialDataDefined.isEmpty() )
    materialHandler->applyDataDefinedToGeometry( materialSettings, geometry, pointsA.size(), materialDataDefined );

  renderer->setGeometry( geometry );
  renderer->setVertexCount( 6 );
  renderer->setInstanceCount( pointsA.size() );

  entity->addComponent( renderer );
  if ( material )
    entity->addComponent( material );

  return entity;
}

Qt3DCore::QEntity *QgsLineVertexData::createJoinEntity( const QgsLineMaterial *segmentMaterial, const QgsAbstractMaterialSettings *materialSettings, const QgsAbstractMaterial3DHandler *materialHandler )
{
  if ( joinPointA.isEmpty() )
    return nullptr;

  Qt3DCore::QEntity *entity = new Qt3DCore::QEntity;

  Qt3DRender::QGeometryRenderer *renderer = new Qt3DRender::QGeometryRenderer;
  renderer->setPrimitiveType( Qt3DRender::QGeometryRenderer::Triangles );
  Qt3DCore::QGeometry *geometry = createJoinGeometry( entity );

  if ( materialHandler && materialSettings && !materialDataDefinedJoins.isEmpty() )
    materialHandler->applyDataDefinedToGeometry( materialSettings, geometry, joinPointA.size(), materialDataDefinedJoins );

  renderer->setGeometry( geometry );
  renderer->setVertexCount( 6 );
  renderer->setInstanceCount( joinPointA.size() );

  QgsLineMaterial *joinMaterial = new QgsLineMaterial( QgsLineMaterial::LinePart::Join );
  if ( segmentMaterial )
    segmentMaterial->copyLineParametersTo( joinMaterial );

  entity->addComponent( renderer );
  entity->addComponent( joinMaterial );

  return entity;
}

/// @endcond
