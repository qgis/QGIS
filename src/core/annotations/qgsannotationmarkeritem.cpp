/***************************************************************************
    qgsannotationmarkeritem.cpp
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

#include "qgsannotationmarkeritem.h"
#include "qgssymbol.h"
#include "qgssymbollayerutils.h"
#include "qgsmarkersymbol.h"
#include "qgsannotationitemnode.h"
#include "qgsannotationitemeditoperation.h"

QgsAnnotationMarkerItem::QgsAnnotationMarkerItem( const QgsPoint &point )
  : QgsAnnotationItem()
  , mPoint( point )
  , mSymbol( std::make_unique< QgsMarkerSymbol >() )
{

}

QgsAnnotationMarkerItem::~QgsAnnotationMarkerItem() = default;

QString QgsAnnotationMarkerItem::type() const
{
  return QStringLiteral( "marker" );
}

void QgsAnnotationMarkerItem::render( QgsRenderContext &context, QgsFeedback * )
{
  QPointF pt;
  if ( context.coordinateTransform().isValid() )
  {
    double x = mPoint.x();
    double y = mPoint.y();
    double z = 0.0;
    context.coordinateTransform().transformInPlace( x, y, z );
    pt = QPointF( x, y );
  }
  else
    pt = mPoint.toQPointF();

  context.mapToPixel().transformInPlace( pt.rx(), pt.ry() );

  mSymbol->startRender( context );
  mSymbol->renderPoint( pt, nullptr, context );
  mSymbol->stopRender( context );
}

bool QgsAnnotationMarkerItem::writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  element.setAttribute( QStringLiteral( "x" ), qgsDoubleToString( mPoint.x() ) );
  element.setAttribute( QStringLiteral( "y" ), qgsDoubleToString( mPoint.y() ) );
  element.appendChild( QgsSymbolLayerUtils::saveSymbol( QStringLiteral( "markerSymbol" ), mSymbol.get(), document, context ) );

  writeCommonProperties( element, document, context );

  return true;
}

Qgis::AnnotationItemFlags QgsAnnotationMarkerItem::flags() const
{
  // in truth this should depend on whether the marker symbol is scale dependent or not!
  return Qgis::AnnotationItemFlag::ScaleDependentBoundingBox;
}

QList<QgsAnnotationItemNode> QgsAnnotationMarkerItem::nodes() const
{
  return { QgsAnnotationItemNode( QgsVertexId( 0, 0, 0 ), mPoint, Qgis::AnnotationItemNodeType::VertexHandle )};
}

Qgis::AnnotationItemEditOperationResult QgsAnnotationMarkerItem::applyEdit( QgsAbstractAnnotationItemEditOperation *operation )
{
  switch ( operation->type() )
  {
    case QgsAbstractAnnotationItemEditOperation::Type::MoveNode:
    {
      QgsAnnotationItemEditOperationMoveNode *moveOperation = qgis::down_cast< QgsAnnotationItemEditOperationMoveNode * >( operation );
      mPoint = QgsPoint( moveOperation->after() );
      return Qgis::AnnotationItemEditOperationResult::Success;
    }

    case QgsAbstractAnnotationItemEditOperation::Type::DeleteNode:
    {
      return Qgis::AnnotationItemEditOperationResult::ItemCleared;
    }

    case QgsAbstractAnnotationItemEditOperation::Type::TranslateItem:
    {
      QgsAnnotationItemEditOperationTranslateItem *moveOperation = qgis::down_cast< QgsAnnotationItemEditOperationTranslateItem * >( operation );
      mPoint.setX( mPoint.x() + moveOperation->translationX() );
      mPoint.setY( mPoint.y() + moveOperation->translationY() );
      return Qgis::AnnotationItemEditOperationResult::Success;
    }

    case QgsAbstractAnnotationItemEditOperation::Type::AddNode:
      break;
  }

  return Qgis::AnnotationItemEditOperationResult::Invalid;
}

QgsAnnotationItemEditOperationTransientResults *QgsAnnotationMarkerItem::transientEditResults( QgsAbstractAnnotationItemEditOperation *operation )
{
  switch ( operation->type() )
  {
    case QgsAbstractAnnotationItemEditOperation::Type::MoveNode:
    {
      QgsAnnotationItemEditOperationMoveNode *moveOperation = dynamic_cast< QgsAnnotationItemEditOperationMoveNode * >( operation );
      return new QgsAnnotationItemEditOperationTransientResults( QgsGeometry( moveOperation->after().clone() ) );
    }

    case QgsAbstractAnnotationItemEditOperation::Type::TranslateItem:
    {
      QgsAnnotationItemEditOperationTranslateItem *moveOperation = qgis::down_cast< QgsAnnotationItemEditOperationTranslateItem * >( operation );
      return new QgsAnnotationItemEditOperationTransientResults( QgsGeometry( new QgsPoint( mPoint.x() + moveOperation->translationX(), mPoint.y() + moveOperation->translationY() ) ) );
    }

    case QgsAbstractAnnotationItemEditOperation::Type::DeleteNode:
    case QgsAbstractAnnotationItemEditOperation::Type::AddNode:
      break;
  }
  return nullptr;
}

QgsAnnotationMarkerItem *QgsAnnotationMarkerItem::create()
{
  return new QgsAnnotationMarkerItem( QgsPoint() );
}

bool QgsAnnotationMarkerItem::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  const double x = element.attribute( QStringLiteral( "x" ) ).toDouble();
  const double y = element.attribute( QStringLiteral( "y" ) ).toDouble();
  mPoint = QgsPoint( x, y );

  const QDomElement symbolElem = element.firstChildElement( QStringLiteral( "symbol" ) );
  if ( !symbolElem.isNull() )
    setSymbol( QgsSymbolLayerUtils::loadSymbol< QgsMarkerSymbol >( symbolElem, context ) );

  readCommonProperties( element, context );
  return true;
}

QgsAnnotationMarkerItem *QgsAnnotationMarkerItem::clone()
{
  std::unique_ptr< QgsAnnotationMarkerItem > item = std::make_unique< QgsAnnotationMarkerItem >( mPoint );
  item->setSymbol( mSymbol->clone() );
  item->copyCommonProperties( this );
  return item.release();
}

QgsRectangle QgsAnnotationMarkerItem::boundingBox() const
{
  return QgsRectangle( mPoint.x(), mPoint.y(), mPoint.x(), mPoint.y() );
}

QgsRectangle QgsAnnotationMarkerItem::boundingBox( QgsRenderContext &context ) const
{
  QPointF pt;
  if ( context.coordinateTransform().isValid() )
  {
    double x = mPoint.x();
    double y = mPoint.y();
    double z = 0.0;
    context.coordinateTransform().transformInPlace( x, y, z );
    pt = QPointF( x, y );
  }
  else
    pt = mPoint.toQPointF();

  context.mapToPixel().transformInPlace( pt.rx(), pt.ry() );

  mSymbol->startRender( context );
  const QRectF boundsInPixels = mSymbol->bounds( pt, context );
  mSymbol->stopRender( context );

  const QgsPointXY topLeft = context.mapToPixel().toMapCoordinates( boundsInPixels.left(), boundsInPixels.top() );
  const QgsPointXY topRight = context.mapToPixel().toMapCoordinates( boundsInPixels.right(), boundsInPixels.top() );
  const QgsPointXY bottomLeft = context.mapToPixel().toMapCoordinates( boundsInPixels.left(), boundsInPixels.bottom() );
  const QgsPointXY bottomRight = context.mapToPixel().toMapCoordinates( boundsInPixels.right(), boundsInPixels.bottom() );

  const QgsRectangle boundsMapUnits = QgsRectangle( topLeft.x(), bottomLeft.y(), bottomRight.x(), topRight.y() );
  return context.coordinateTransform().transformBoundingBox( boundsMapUnits, Qgis::TransformDirection::Reverse );
}

const QgsMarkerSymbol *QgsAnnotationMarkerItem::symbol() const
{
  return mSymbol.get();
}

void QgsAnnotationMarkerItem::setSymbol( QgsMarkerSymbol *symbol )
{
  mSymbol.reset( symbol );
}
