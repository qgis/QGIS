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


QgsQuickHighlightSGNode::QgsQuickHighlightSGNode( const QVector<QgsPoint> &points, QgsWkbTypes::GeometryType type, const QColor &color, qreal width )
  : QSGNode()
{
  mMaterial.setColor( color );

  // TODO: support multi-part geometries
  switch ( type )
  {
    case QgsWkbTypes::PointGeometry:
    {
      if ( !points.isEmpty() )
        appendChildNode( createPointGeometry( points.at( 0 ), width ) );
      break;
    }

    case QgsWkbTypes::LineGeometry:
    {
      appendChildNode( createLineGeometry( points, width ) );
      break;
    }

    case QgsWkbTypes::PolygonGeometry:
    {
      // TODO: support polygon geometries
      break;
    }

    case QgsWkbTypes::UnknownGeometry:
    case QgsWkbTypes::NullGeometry:
      break;
  }
}

QgsQuickHighlightSGNode::~QgsQuickHighlightSGNode()
{
}

QSGGeometryNode *QgsQuickHighlightSGNode::createLineGeometry( const QVector<QgsPoint> &points, qreal width )
{
  QSGGeometryNode *node = new QSGGeometryNode;
  QSGGeometry *sgGeom = new QSGGeometry( QSGGeometry::defaultAttributes_Point2D(), points.count() );
  QSGGeometry::Point2D *vertices = sgGeom->vertexDataAsPoint2D();

  int i = 0;
  Q_FOREACH ( const QgsPoint &pt, points )
  {
    vertices[i++].set( pt.x(), pt.y() );
  }

  sgGeom->setLineWidth( width );
  sgGeom->setDrawingMode( GL_LINE_STRIP );
  node->setGeometry( sgGeom );
  node->setMaterial( &mMaterial );
  node->setFlag( QSGNode::OwnsGeometry );
  node->setFlag( QSGNode::OwnedByParent );
  return node;
}

QSGGeometryNode *QgsQuickHighlightSGNode::createPointGeometry( const QgsPoint &point, qreal width )
{
  QSGGeometryNode *node = new QSGGeometryNode;

  QSGGeometry *sgGeom = new QSGGeometry( QSGGeometry::defaultAttributes_Point2D(), 1 );

  QSGGeometry::Point2D *vertices = sgGeom->vertexDataAsPoint2D();
  vertices[0].set( point.x(), point.y() );
  sgGeom->setDrawingMode( GL_POINTS );
  sgGeom->setLineWidth( width );

  node->setGeometry( sgGeom );
  node->setMaterial( &mMaterial );
  node->setFlag( QSGNode::OwnsGeometry );
  node->setFlag( QSGNode::OwnedByParent );
  return node;
}

QSGGeometryNode *QgsQuickHighlightSGNode::createPolygonGeometry( const QVector<QgsPoint> &points )
{
  Q_UNUSED( points );
  return 0;
}
