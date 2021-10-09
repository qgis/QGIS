/***************************************************************************
    qgsannotationpolygonitem.cpp
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

#include "qgsannotationpolygonitem.h"
#include "qgssymbol.h"
#include "qgssymbollayerutils.h"
#include "qgssurface.h"
#include "qgsfillsymbol.h"
#include "qgsannotationitemnode.h"
#include "qgsannotationitemeditoperation.h"

QgsAnnotationPolygonItem::QgsAnnotationPolygonItem( QgsCurvePolygon *polygon )
  : QgsAnnotationItem()
  , mPolygon( polygon )
  , mSymbol( std::make_unique< QgsFillSymbol >() )
{

}

QgsAnnotationPolygonItem::~QgsAnnotationPolygonItem() = default;

QString QgsAnnotationPolygonItem::type() const
{
  return QStringLiteral( "polygon" );
}

void QgsAnnotationPolygonItem::render( QgsRenderContext &context, QgsFeedback * )
{

  auto transformRing = [&context]( QPolygonF & pts )
  {
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
  };

  QPolygonF exterior = mPolygon->exteriorRing()->asQPolygonF();
  transformRing( exterior );
  QVector<QPolygonF> rings;
  rings.reserve( mPolygon->numInteriorRings() );
  for ( int i = 0; i < mPolygon->numInteriorRings(); ++i )
  {
    QPolygonF ring = mPolygon->interiorRing( i )->asQPolygonF();
    transformRing( ring );
    rings.append( ring );
  }

  mSymbol->startRender( context );
  mSymbol->renderPolygon( exterior, rings.empty() ? nullptr : &rings, nullptr, context );
  mSymbol->stopRender( context );
}

bool QgsAnnotationPolygonItem::writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  element.setAttribute( QStringLiteral( "wkt" ), mPolygon->asWkt() );
  element.appendChild( QgsSymbolLayerUtils::saveSymbol( QStringLiteral( "lineSymbol" ), mSymbol.get(), document, context ) );

  writeCommonProperties( element, document, context );
  return true;
}

QList<QgsAnnotationItemNode> QgsAnnotationPolygonItem::nodes() const
{
  QList< QgsAnnotationItemNode > res;

  auto processRing  = [&res]( const QgsCurve * ring, int ringId )
  {
    // we don't want a duplicate node for the closed ring vertex
    const int count = ring->isClosed() ? ring->numPoints() - 1 : ring->numPoints();
    res.reserve( res.size() + count );
    for ( int i = 0; i < count; ++i )
    {
      res << QgsAnnotationItemNode( QgsVertexId( 0, ringId, i ), QgsPointXY( ring->xAt( i ), ring->yAt( i ) ), Qgis::AnnotationItemNodeType::VertexHandle );
    }
  };

  if ( const QgsCurve *ring = mPolygon->exteriorRing() )
  {
    processRing( ring, 0 );
  }
  for ( int i = 0; i < mPolygon->numInteriorRings(); ++i )
  {
    processRing( mPolygon->interiorRing( i ), i + 1 );
  }

  return res;
}

Qgis::AnnotationItemEditOperationResult QgsAnnotationPolygonItem::applyEdit( QgsAbstractAnnotationItemEditOperation *operation )
{
  switch ( operation->type() )
  {
    case QgsAbstractAnnotationItemEditOperation::Type::MoveNode:
    {
      QgsAnnotationItemEditOperationMoveNode *moveOperation = dynamic_cast< QgsAnnotationItemEditOperationMoveNode * >( operation );
      if ( mPolygon->moveVertex( moveOperation->nodeId(), QgsPoint( moveOperation->after() ) ) )
        return Qgis::AnnotationItemEditOperationResult::Success;
      break;
    }

    case QgsAbstractAnnotationItemEditOperation::Type::DeleteNode:
    {
      QgsAnnotationItemEditOperationDeleteNode *deleteOperation = qgis::down_cast< QgsAnnotationItemEditOperationDeleteNode * >( operation );
      if ( mPolygon->deleteVertex( deleteOperation->nodeId() ) )
        return mPolygon->isEmpty() ? Qgis::AnnotationItemEditOperationResult::ItemCleared : Qgis::AnnotationItemEditOperationResult::Success;
      break;
    }

    case QgsAbstractAnnotationItemEditOperation::Type::AddNode:
    {
      QgsAnnotationItemEditOperationAddNode *addOperation = qgis::down_cast< QgsAnnotationItemEditOperationAddNode * >( operation );

      QgsPoint segmentPoint;
      QgsVertexId endOfSegmentVertex;
      mPolygon->closestSegment( addOperation->point(), segmentPoint, endOfSegmentVertex );
      if ( mPolygon->insertVertex( endOfSegmentVertex, segmentPoint ) )
        return Qgis::AnnotationItemEditOperationResult::Success;
      break;
    }

    case QgsAbstractAnnotationItemEditOperation::Type::TranslateItem:
    {
      QgsAnnotationItemEditOperationTranslateItem *moveOperation = qgis::down_cast< QgsAnnotationItemEditOperationTranslateItem * >( operation );
      const QTransform transform = QTransform::fromTranslate( moveOperation->translationX(), moveOperation->translationY() );
      mPolygon->transform( transform );
      return Qgis::AnnotationItemEditOperationResult::Success;
    }
  }

  return Qgis::AnnotationItemEditOperationResult::Invalid;
}

QgsAnnotationItemEditOperationTransientResults *QgsAnnotationPolygonItem::transientEditResults( QgsAbstractAnnotationItemEditOperation *operation )
{
  switch ( operation->type() )
  {
    case QgsAbstractAnnotationItemEditOperation::Type::MoveNode:
    {
      QgsAnnotationItemEditOperationMoveNode *moveOperation = dynamic_cast< QgsAnnotationItemEditOperationMoveNode * >( operation );
      std::unique_ptr< QgsCurvePolygon > modifiedPolygon( mPolygon->clone() );
      if ( modifiedPolygon->moveVertex( moveOperation->nodeId(), QgsPoint( moveOperation->after() ) ) )
      {
        return new QgsAnnotationItemEditOperationTransientResults( QgsGeometry( std::move( modifiedPolygon ) ) );
      }
      break;
    }

    case QgsAbstractAnnotationItemEditOperation::Type::TranslateItem:
    {
      QgsAnnotationItemEditOperationTranslateItem *moveOperation = qgis::down_cast< QgsAnnotationItemEditOperationTranslateItem * >( operation );
      const QTransform transform = QTransform::fromTranslate( moveOperation->translationX(), moveOperation->translationY() );
      std::unique_ptr< QgsCurvePolygon > modifiedPolygon( mPolygon->clone() );
      modifiedPolygon->transform( transform );
      return new QgsAnnotationItemEditOperationTransientResults( QgsGeometry( std::move( modifiedPolygon ) ) );
    }

    case QgsAbstractAnnotationItemEditOperation::Type::DeleteNode:
    case QgsAbstractAnnotationItemEditOperation::Type::AddNode:
      break;
  }
  return nullptr;
}

QgsAnnotationPolygonItem *QgsAnnotationPolygonItem::create()
{
  return new QgsAnnotationPolygonItem( new QgsPolygon() );
}

bool QgsAnnotationPolygonItem::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  const QString wkt = element.attribute( QStringLiteral( "wkt" ) );
  const QgsGeometry geometry = QgsGeometry::fromWkt( wkt );
  if ( const QgsCurvePolygon *polygon = qgsgeometry_cast< const QgsCurvePolygon * >( geometry.constGet() ) )
    mPolygon.reset( polygon->clone() );

  const QDomElement symbolElem = element.firstChildElement( QStringLiteral( "symbol" ) );
  if ( !symbolElem.isNull() )
    setSymbol( QgsSymbolLayerUtils::loadSymbol< QgsFillSymbol >( symbolElem, context ) );

  readCommonProperties( element, context );
  return true;
}

QgsAnnotationPolygonItem *QgsAnnotationPolygonItem::clone()
{
  std::unique_ptr< QgsAnnotationPolygonItem > item = std::make_unique< QgsAnnotationPolygonItem >( mPolygon->clone() );
  item->setSymbol( mSymbol->clone() );
  item->copyCommonProperties( this );;
  return item.release();
}

QgsRectangle QgsAnnotationPolygonItem::boundingBox() const
{
  return mPolygon->boundingBox();
}

const QgsFillSymbol *QgsAnnotationPolygonItem::symbol() const
{
  return mSymbol.get();
}

void QgsAnnotationPolygonItem::setSymbol( QgsFillSymbol *symbol )
{
  mSymbol.reset( symbol );
}
