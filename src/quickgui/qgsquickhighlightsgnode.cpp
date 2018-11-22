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
#include "qgsgeometrycollection.h"
#include "qgsgeometry.h"
#include "qgslinestring.h"
#include "qgspoint.h"
#include "qgspolygon.h"

QgsQuickHighlightSGNode::QgsQuickHighlightSGNode( const QgsGeometry &geom,
    const QColor &color, float width )
  : QSGNode()
  , mWidth( width )
{
  mMaterial.setColor( color );
  handleGeometryCollection( geom.constGet(), geom.type() );
}

void QgsQuickHighlightSGNode::handleGeometryCollection( const QgsAbstractGeometry *geom, QgsWkbTypes::GeometryType type )
{
  const QgsGeometryCollection *collection = qgsgeometry_cast<const QgsGeometryCollection *>( geom );
  if ( collection && !collection->isEmpty() )
  {
    for ( int i = 0; i < collection->numGeometries(); ++i )
    {
      const QgsAbstractGeometry *geomN = collection->geometryN( i );
      handleSingleGeometry( geomN, type );
    }
  }
  else
  {
    handleSingleGeometry( geom, type );
  }
}

void QgsQuickHighlightSGNode::handleSingleGeometry( const QgsAbstractGeometry *geom, QgsWkbTypes::GeometryType type )
{
  switch ( type )
  {
    case QgsWkbTypes::PointGeometry:
    {
      const QgsPoint *point = qgsgeometry_cast<const QgsPoint *>( geom );
      if ( point )
        appendChildNode( createPointGeometry( point ) );
      break;
    }

    case QgsWkbTypes::LineGeometry:
    {
      const QgsLineString *line = qgsgeometry_cast<const QgsLineString *>( geom );
      if ( line )
        appendChildNode( createLineGeometry( line ) );
      break;
    }

    case QgsWkbTypes::PolygonGeometry:
    {
      const QgsPolygon *poly = qgsgeometry_cast<const QgsPolygon *>( geom );
      if ( poly )
        appendChildNode( createPolygonGeometry( poly ) );
      break;
    }

    case QgsWkbTypes::UnknownGeometry:
    case QgsWkbTypes::NullGeometry:
      break;
  }
}

QSGGeometryNode *QgsQuickHighlightSGNode::createLineGeometry( const QgsLineString *line )
{
  Q_ASSERT( line );

  std::unique_ptr<QSGGeometryNode> node = qgis::make_unique< QSGGeometryNode>();
  std::unique_ptr<QSGGeometry> sgGeom = qgis::make_unique< QSGGeometry>( QSGGeometry::defaultAttributes_Point2D(), line->numPoints() );
  QSGGeometry::Point2D *vertices = sgGeom->vertexDataAsPoint2D();

  const double *x = line->xData();
  const double *y = line->yData();

  for ( int i = 0; i < line->numPoints(); ++i )
  {
    vertices[i].set(
      static_cast< float >( x[i] ),
      static_cast< float >( y[i] )
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

QSGGeometryNode *QgsQuickHighlightSGNode::createPointGeometry( const QgsPoint *point )
{
  Q_ASSERT( point );

  std::unique_ptr<QSGGeometryNode> node = qgis::make_unique< QSGGeometryNode>();
  std::unique_ptr<QSGGeometry> sgGeom = qgis::make_unique<QSGGeometry>( QSGGeometry::defaultAttributes_Point2D(), 1 );

  QSGGeometry::Point2D *vertices = sgGeom->vertexDataAsPoint2D();
  vertices[0].set(
    static_cast< float >( point->x() ),
    static_cast< float >( point->y() )
  );
  sgGeom->setDrawingMode( GL_POINTS );
  sgGeom->setLineWidth( mWidth );

  node->setGeometry( sgGeom.release() );
  node->setMaterial( &mMaterial );
  node->setFlag( QSGNode::OwnsGeometry );
  node->setFlag( QSGNode::OwnedByParent );
  return node.release();
}

QSGGeometryNode *QgsQuickHighlightSGNode::createPolygonGeometry( const QgsPolygon *polygon )
{
  Q_ASSERT( polygon );

  const QgsRectangle bounds = polygon->boundingBox();
  QgsTessellator tes( bounds.xMinimum(), bounds.yMinimum(), false, false, false );
  tes.addPolygon( *polygon, 0.0 );

  QSGGeometryNode *node = new QSGGeometryNode;
  QSGGeometry *sgGeom = new QSGGeometry( QSGGeometry::defaultAttributes_Point2D(), tes.dataVerticesCount() );

  QSGGeometry::Point2D *vertices = sgGeom->vertexDataAsPoint2D();

  // we need to revert translation in tessellator
  float translateX = static_cast< float >( bounds.xMinimum() );
  float translateY = static_cast< float >( bounds.yMinimum() );

  const QVector<float> data = tes.data();
  int i = 0;
  for ( auto it = data.constBegin(); it != data.constEnd(); )
  {
    float x = *it;
    vertices[i].x = translateX + x;
    ++it;

    ++it; // we do not need z coordinate

    float y = -( *it );
    vertices[i].y = translateY + y;
    ++it;

    ++i;
  }
  sgGeom->setDrawingMode( GL_TRIANGLES );
  node->setGeometry( sgGeom );
  node->setMaterial( &mMaterial );
  node->setFlag( QSGNode::OwnsGeometry );
  node->setFlag( QSGNode::OwnedByParent );
  return node;
}
