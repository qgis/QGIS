/***************************************************************************
 qgsquickhighlightsgnode.cpp
  --------------------------------------
  Date                 : Nov 2017
  Copyright            : (C) 2017 by Matthias Kuhn
  Email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsquickhighlightsgnode.h"
#include "qgstessellator.h"
#include "qgsmultipolygon.h"
#include "qgsgeometrycollection.h"
#include "qgsgeometry.h"
#include "qgstriangle.h"

QgsQuickHighlightSGNode::QgsQuickHighlightSGNode( const QgsGeometry &geom,
    const QColor &color, float width )
  : QSGNode()
  , mWidth( width )
{
  mMaterial.setColor( color );
  handleGeometryCollection( geom );
}

void QgsQuickHighlightSGNode::handleGeometryCollection( const QgsGeometry &geom )
{
  const QgsGeometryCollection *collection = qgsgeometry_cast<const QgsGeometryCollection *>( geom.constGet() );
  if ( collection && !collection->isEmpty() )
  {
    for ( int i = 0; i < collection->numGeometries(); ++i )
    {
      QgsGeometry geomN( collection->geometryN( i )->clone() );
      handleSingleGeometry( geomN );
    }
  }
  else
  {
    handleSingleGeometry( geom );
  }
}

void QgsQuickHighlightSGNode::handleSingleGeometry( const QgsGeometry &geom )
{
  Q_ASSERT( !geom.isMultipart() );

  switch ( geom.type() )
  {
    case QgsWkbTypes::PointGeometry:
    {
      QVector<QgsPoint> points;
      for ( auto it = geom.vertices_begin(); it != geom.vertices_end(); ++it )
        points.append( *it );

      if ( !points.isEmpty() )
        appendChildNode( createPointGeometry( points.at( 0 ) ) );
      break;
    }

    case QgsWkbTypes::LineGeometry:
    {
      QVector<QgsPoint> points;
      for ( auto it = geom.vertices_begin(); it != geom.vertices_end(); ++it )
        points.append( *it );

      appendChildNode( createLineGeometry( points ) );
      break;
    }

    case QgsWkbTypes::PolygonGeometry:
    {
      const QgsPolygon *poly = qgsgeometry_cast<const QgsPolygon *>( geom.constGet() );
      if ( poly )
        appendChildNode( createPolygonGeometry( *poly ) );
      break;
    }

    case QgsWkbTypes::UnknownGeometry:
    case QgsWkbTypes::NullGeometry:
      break;
  }
}

QSGGeometryNode *QgsQuickHighlightSGNode::createLineGeometry( const QVector<QgsPoint> &points )
{
  std::unique_ptr<QSGGeometryNode> node = qgis::make_unique< QSGGeometryNode>();
  std::unique_ptr<QSGGeometry> sgGeom = qgis::make_unique< QSGGeometry>( QSGGeometry::defaultAttributes_Point2D(), points.count() );
  QSGGeometry::Point2D *vertices = sgGeom->vertexDataAsPoint2D();

  int i = 0;
  for ( const QgsPoint &pt : points )
  {
    vertices[i++].set(
      static_cast< float >( pt.x() ),
      static_cast< float >( pt.y() )
    );
  }

  sgGeom->setLineWidth( mWidth );
  sgGeom->setDrawingMode( GL_LINE_STRIP );
  node->setGeometry( sgGeom.release() );
  node->setMaterial( &mMaterial );
  node->setFlag( QSGNode::OwnsGeometry );
  node->setFlag( QSGNode::OwnedByParent );
  return node.release();
}

QSGGeometryNode *QgsQuickHighlightSGNode::createPointGeometry( const QgsPoint &point )
{
  std::unique_ptr<QSGGeometryNode> node = qgis::make_unique< QSGGeometryNode>();
  std::unique_ptr<QSGGeometry> sgGeom = qgis::make_unique<QSGGeometry>( QSGGeometry::defaultAttributes_Point2D(), 1 );

  QSGGeometry::Point2D *vertices = sgGeom->vertexDataAsPoint2D();
  vertices[0].set(
    static_cast< float >( point.x() ),
    static_cast< float >( point.y() )
  );
  sgGeom->setDrawingMode( GL_POINTS );
  sgGeom->setLineWidth( mWidth );

  node->setGeometry( sgGeom.release() );
  node->setMaterial( &mMaterial );
  node->setFlag( QSGNode::OwnsGeometry );
  node->setFlag( QSGNode::OwnedByParent );
  return node.release();
}

QSGGeometryNode *QgsQuickHighlightSGNode::createPolygonGeometry( const QgsPolygon &polygon )
{
  QgsRectangle bounds = polygon.boundingBox();
  QgsTessellator tes( bounds.xMinimum(), bounds.yMinimum(), false, false, false );
  std::unique_ptr< QgsPolygon > p( qgsgeometry_cast< QgsPolygon * >( polygon.segmentize() ) );
  tes.addPolygon( *p.get(), 0.0 );

  std::unique_ptr<QgsMultiPolygon> triangles = tes.asMultiPolygon();
  int ntris = triangles->numGeometries();

  QSGGeometryNode *node = new QSGGeometryNode;
  QSGGeometry *sgGeom = new QSGGeometry( QSGGeometry::defaultAttributes_Point2D(), ntris * 3 );

  QSGGeometry::Point2D *vertices = sgGeom->vertexDataAsPoint2D();

  for ( int j = 0; j < ntris; j++ )
  {
    const QgsTriangle *triangle =  qgsgeometry_cast<const QgsTriangle *>( triangles->geometryN( j ) );
    if ( triangle )
    {
      for ( int v = 0; v < 3; ++v )
      {
        const QgsPoint vertex = triangle->vertexAt( v );
        vertices[3 * j + v].x = static_cast< float >( vertex.x()  + bounds.xMinimum() ) ;
        vertices[3 * j + v].y = static_cast< float >( vertex.y()  + bounds.yMinimum() );
      }
    }
  }
  sgGeom->setDrawingMode( GL_TRIANGLES );
  node->setGeometry( sgGeom );
  node->setMaterial( &mMaterial );
  node->setFlag( QSGNode::OwnsGeometry );
  node->setFlag( QSGNode::OwnedByParent );
  return node;
}
