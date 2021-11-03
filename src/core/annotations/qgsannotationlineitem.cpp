/***************************************************************************
    qgsannotationlineitem.cpp
    ----------------
    begin                : July 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsannotationlineitem.h"
#include "qgssymbol.h"
#include "qgssymbollayerutils.h"
#include "qgslinesymbol.h"
#include "qgsannotationitemnode.h"
#include "qgsannotationitemeditoperation.h"

QgsAnnotationLineItem::QgsAnnotationLineItem( QgsCurve *curve )
  : QgsAnnotationItem()
  , mCurve( curve )
  , mSymbol( std::make_unique< QgsLineSymbol >() )
{

}

QgsAnnotationLineItem::~QgsAnnotationLineItem() = default;

QString QgsAnnotationLineItem::type() const
{
  return QStringLiteral( "linestring" );
}

void QgsAnnotationLineItem::render( QgsRenderContext &context, QgsFeedback * )
{
  QPolygonF pts = mCurve->asQPolygonF();

  //transform the QPolygonF to screen coordinates
  if ( context.coordinateTransform().isValid() )
  {
    try
    {
      context.coordinateTransform().transformPolygon( pts );
    }
    catch ( QgsCsException & )
    {
      // we don't abort the rendering here, instead we remove any invalid points and just plot those which ARE valid
    }
  }

  // remove non-finite points, e.g. infinite or NaN points caused by reprojecting errors
  pts.erase( std::remove_if( pts.begin(), pts.end(),
                             []( const QPointF point )
  {
    return !std::isfinite( point.x() ) || !std::isfinite( point.y() );
  } ), pts.end() );

  QPointF *ptr = pts.data();
  for ( int i = 0; i < pts.size(); ++i, ++ptr )
  {
    context.mapToPixel().transformInPlace( ptr->rx(), ptr->ry() );
  }

  mSymbol->startRender( context );
  mSymbol->renderPolyline( pts, nullptr, context );
  mSymbol->stopRender( context );
}

bool QgsAnnotationLineItem::writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  element.setAttribute( QStringLiteral( "wkt" ), mCurve->asWkt() );
  element.appendChild( QgsSymbolLayerUtils::saveSymbol( QStringLiteral( "lineSymbol" ), mSymbol.get(), document, context ) );
  writeCommonProperties( element, document, context );

  return true;
}

QList<QgsAnnotationItemNode> QgsAnnotationLineItem::nodes() const
{
  QList< QgsAnnotationItemNode > res;
  int i = 0;
  for ( auto it = mCurve->vertices_begin(); it != mCurve->vertices_end(); ++it, ++i )
  {
    res.append( QgsAnnotationItemNode( it.vertexId(), QgsPointXY( ( *it ).x(), ( *it ).y() ), Qgis::AnnotationItemNodeType::VertexHandle ) );
  }
  return res;
}

Qgis::AnnotationItemEditOperationResult QgsAnnotationLineItem::applyEdit( QgsAbstractAnnotationItemEditOperation *operation )
{
  switch ( operation->type() )
  {
    case QgsAbstractAnnotationItemEditOperation::Type::MoveNode:
    {
      QgsAnnotationItemEditOperationMoveNode *moveOperation = qgis::down_cast< QgsAnnotationItemEditOperationMoveNode * >( operation );
      if ( mCurve->moveVertex( moveOperation->nodeId(), QgsPoint( moveOperation->after() ) ) )
        return Qgis::AnnotationItemEditOperationResult::Success;
      break;
    }

    case QgsAbstractAnnotationItemEditOperation::Type::DeleteNode:
    {
      QgsAnnotationItemEditOperationDeleteNode *deleteOperation = qgis::down_cast< QgsAnnotationItemEditOperationDeleteNode * >( operation );
      if ( mCurve->deleteVertex( deleteOperation->nodeId() ) )
        return mCurve->isEmpty() ? Qgis::AnnotationItemEditOperationResult::ItemCleared : Qgis::AnnotationItemEditOperationResult::Success;
      break;
    }

    case QgsAbstractAnnotationItemEditOperation::Type::AddNode:
    {
      QgsAnnotationItemEditOperationAddNode *addOperation = qgis::down_cast< QgsAnnotationItemEditOperationAddNode * >( operation );

      QgsPoint segmentPoint;
      QgsVertexId endOfSegmentVertex;
      mCurve->closestSegment( addOperation->point(), segmentPoint, endOfSegmentVertex );
      if ( mCurve->insertVertex( endOfSegmentVertex, segmentPoint ) )
        return Qgis::AnnotationItemEditOperationResult::Success;
      break;
    }

    case QgsAbstractAnnotationItemEditOperation::Type::TranslateItem:
    {
      QgsAnnotationItemEditOperationTranslateItem *moveOperation = qgis::down_cast< QgsAnnotationItemEditOperationTranslateItem * >( operation );
      const QTransform transform = QTransform::fromTranslate( moveOperation->translationX(), moveOperation->translationY() );
      mCurve->transform( transform );
      return Qgis::AnnotationItemEditOperationResult::Success;
    }
  }

  return Qgis::AnnotationItemEditOperationResult::Invalid;
}

QgsAnnotationItemEditOperationTransientResults *QgsAnnotationLineItem::transientEditResults( QgsAbstractAnnotationItemEditOperation *operation )
{
  switch ( operation->type() )
  {
    case QgsAbstractAnnotationItemEditOperation::Type::MoveNode:
    {
      QgsAnnotationItemEditOperationMoveNode *moveOperation = dynamic_cast< QgsAnnotationItemEditOperationMoveNode * >( operation );
      std::unique_ptr< QgsCurve > modifiedCurve( mCurve->clone() );
      if ( modifiedCurve->moveVertex( moveOperation->nodeId(), QgsPoint( moveOperation->after() ) ) )
      {
        return new QgsAnnotationItemEditOperationTransientResults( QgsGeometry( std::move( modifiedCurve ) ) );
      }
      break;
    }

    case QgsAbstractAnnotationItemEditOperation::Type::TranslateItem:
    {
      QgsAnnotationItemEditOperationTranslateItem *moveOperation = qgis::down_cast< QgsAnnotationItemEditOperationTranslateItem * >( operation );
      const QTransform transform = QTransform::fromTranslate( moveOperation->translationX(), moveOperation->translationY() );
      std::unique_ptr< QgsCurve > modifiedCurve( mCurve->clone() );
      modifiedCurve->transform( transform );
      return new QgsAnnotationItemEditOperationTransientResults( QgsGeometry( std::move( modifiedCurve ) ) );
    }

    case QgsAbstractAnnotationItemEditOperation::Type::DeleteNode:
    case QgsAbstractAnnotationItemEditOperation::Type::AddNode:
      break;
  }
  return nullptr;
}

QgsAnnotationLineItem *QgsAnnotationLineItem::create()
{
  return new QgsAnnotationLineItem( new QgsLineString() );
}

bool QgsAnnotationLineItem::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  const QString wkt = element.attribute( QStringLiteral( "wkt" ) );
  const QgsGeometry geometry = QgsGeometry::fromWkt( wkt );
  if ( const QgsCurve *curve = qgsgeometry_cast< const QgsCurve * >( geometry.constGet() ) )
    mCurve.reset( curve->clone() );

  const QDomElement symbolElem = element.firstChildElement( QStringLiteral( "symbol" ) );
  if ( !symbolElem.isNull() )
    setSymbol( QgsSymbolLayerUtils::loadSymbol< QgsLineSymbol >( symbolElem, context ) );

  readCommonProperties( element, context );

  return true;
}

QgsRectangle QgsAnnotationLineItem::boundingBox() const
{
  return mCurve->boundingBox();
}

QgsAnnotationLineItem *QgsAnnotationLineItem::clone()
{
  std::unique_ptr< QgsAnnotationLineItem > item = std::make_unique< QgsAnnotationLineItem >( mCurve->clone() );
  item->setSymbol( mSymbol->clone() );
  item->copyCommonProperties( this );
  return item.release();
}

const QgsLineSymbol *QgsAnnotationLineItem::symbol() const
{
  return mSymbol.get();
}

void QgsAnnotationLineItem::setSymbol( QgsLineSymbol *symbol )
{
  mSymbol.reset( symbol );
}
