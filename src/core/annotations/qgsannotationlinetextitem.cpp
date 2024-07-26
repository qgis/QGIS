/***************************************************************************
    qgsannotationlinetextitem.cpp
    ----------------
    begin                : March 2023
    copyright            : (C) 2023 by Nyall Dawson
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

#include "qgsannotationlinetextitem.h"
#include "qgsannotationitemnode.h"
#include "qgsannotationitemeditoperation.h"
#include "qgsrendercontext.h"
#include "qgscurve.h"
#include "qgslinestring.h"
#include "qgstextrenderer.h"
#include "qgsunittypes.h"
#include "qgssymbollayerutils.h"

QgsAnnotationLineTextItem::QgsAnnotationLineTextItem( const QString &text, QgsCurve *curve )
  : QgsAnnotationItem()
  , mText( text )
  , mCurve( curve )
{

}

Qgis::AnnotationItemFlags QgsAnnotationLineTextItem::flags() const
{
  // in truth this should depend on whether the text format is scale dependent or not!
  return Qgis::AnnotationItemFlag::ScaleDependentBoundingBox
         | Qgis::AnnotationItemFlag::SupportsReferenceScale;
}

QgsAnnotationLineTextItem::~QgsAnnotationLineTextItem() = default;

QString QgsAnnotationLineTextItem::type() const
{
  return QStringLiteral( "linetext" );
}

void QgsAnnotationLineTextItem::render( QgsRenderContext &context, QgsFeedback * )
{
  // TODO -- expose as an option!
  QgsGeometry smoothed( mCurve->clone() );
  smoothed = smoothed.smooth( );

  QPolygonF pts = smoothed.asQPolygonF();

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

  const QString displayText = QgsExpression::replaceExpressionText( mText, &context.expressionContext(), &context.distanceArea() );

  const double offsetFromLine = context.convertToPainterUnits( mOffsetFromLineDistance, mOffsetFromLineUnit, mOffsetFromLineScale );

  QgsTextRenderer::drawTextOnLine( pts, displayText, context, mTextFormat, 0, offsetFromLine );
}

bool QgsAnnotationLineTextItem::writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  element.setAttribute( QStringLiteral( "wkt" ), mCurve->asWkt() );
  element.setAttribute( QStringLiteral( "text" ), mText );

  element.setAttribute( QStringLiteral( "offsetFromLine" ), qgsDoubleToString( mOffsetFromLineDistance ) );
  element.setAttribute( QStringLiteral( "offsetFromLineUnit" ), QgsUnitTypes::encodeUnit( mOffsetFromLineUnit ) );
  element.setAttribute( QStringLiteral( "offsetFromLineScale" ), QgsSymbolLayerUtils::encodeMapUnitScale( mOffsetFromLineScale ) );

  QDomElement textFormatElem = document.createElement( QStringLiteral( "lineTextFormat" ) );
  textFormatElem.appendChild( mTextFormat.writeXml( document, context ) );
  element.appendChild( textFormatElem );

  writeCommonProperties( element, document, context );

  return true;
}

QList<QgsAnnotationItemNode> QgsAnnotationLineTextItem::nodesV2( const QgsAnnotationItemEditContext & ) const
{
  QList< QgsAnnotationItemNode > res;
  int i = 0;
  for ( auto it = mCurve->vertices_begin(); it != mCurve->vertices_end(); ++it, ++i )
  {
    res.append( QgsAnnotationItemNode( it.vertexId(), QgsPointXY( ( *it ).x(), ( *it ).y() ), Qgis::AnnotationItemNodeType::VertexHandle ) );
  }
  return res;
}

Qgis::AnnotationItemEditOperationResult QgsAnnotationLineTextItem::applyEditV2( QgsAbstractAnnotationItemEditOperation *operation, const QgsAnnotationItemEditContext & )
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

QgsAnnotationItemEditOperationTransientResults *QgsAnnotationLineTextItem::transientEditResultsV2( QgsAbstractAnnotationItemEditOperation *operation, const QgsAnnotationItemEditContext & )
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

QgsAnnotationLineTextItem *QgsAnnotationLineTextItem::create()
{
  return new QgsAnnotationLineTextItem( QString(), new QgsLineString() );
}

bool QgsAnnotationLineTextItem::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  const QString wkt = element.attribute( QStringLiteral( "wkt" ) );
  const QgsGeometry geometry = QgsGeometry::fromWkt( wkt );
  if ( const QgsCurve *curve = qgsgeometry_cast< const QgsCurve * >( geometry.constGet() ) )
    mCurve.reset( curve->clone() );

  mText = element.attribute( QStringLiteral( "text" ) );
  const QDomElement textFormatElem = element.firstChildElement( QStringLiteral( "lineTextFormat" ) );
  if ( !textFormatElem.isNull() )
  {
    const QDomNodeList textFormatNodeList = textFormatElem.elementsByTagName( QStringLiteral( "text-style" ) );
    const QDomElement textFormatElem = textFormatNodeList.at( 0 ).toElement();
    mTextFormat.readXml( textFormatElem, context );
  }

  mOffsetFromLineDistance = element.attribute( QStringLiteral( "offsetFromLine" ) ).toDouble();
  bool ok = false;
  mOffsetFromLineUnit = QgsUnitTypes::decodeRenderUnit( element.attribute( QStringLiteral( "offsetFromLineUnit" ) ), &ok );
  if ( !ok )
    mOffsetFromLineUnit = Qgis::RenderUnit::Millimeters;

  mOffsetFromLineScale =  QgsSymbolLayerUtils::decodeMapUnitScale( element.attribute( QStringLiteral( "offsetFromLineScale" ) ) );

  readCommonProperties( element, context );

  return true;
}

QgsRectangle QgsAnnotationLineTextItem::boundingBox() const
{
  return mCurve->boundingBox();
}

QgsRectangle QgsAnnotationLineTextItem::boundingBox( QgsRenderContext &context ) const
{
  const QString displayText = QgsExpression::replaceExpressionText( mText, &context.expressionContext(), &context.distanceArea() );

  const double lineOffsetInMapUnits = std::fabs( context.convertToMapUnits( mOffsetFromLineDistance, mOffsetFromLineUnit, mOffsetFromLineScale ) );

  const double heightInPixels = QgsTextRenderer::textHeight( context, mTextFormat, { displayText} );

  // text size has already been calculated using any symbology reference scale factor above -- we need
  // to temporarily remove the reference scale here or we'll be undoing the scaling
  QgsScopedRenderContextReferenceScaleOverride resetScaleFactor( context, -1.0 );
  const double heightInMapUnits = context.convertToMapUnits( heightInPixels, Qgis::RenderUnit::Pixels );

  return mCurve->boundingBox().buffered( heightInMapUnits + lineOffsetInMapUnits );
}

QgsAnnotationLineTextItem *QgsAnnotationLineTextItem::clone() const
{
  std::unique_ptr< QgsAnnotationLineTextItem > item = std::make_unique< QgsAnnotationLineTextItem >( mText, mCurve->clone() );
  item->setFormat( mTextFormat );
  item->setOffsetFromLine( mOffsetFromLineDistance );
  item->setOffsetFromLineUnit( mOffsetFromLineUnit );
  item->setOffsetFromLineMapUnitScale( mOffsetFromLineScale );
  item->copyCommonProperties( this );
  return item.release();
}

void QgsAnnotationLineTextItem::setGeometry( QgsCurve *geometry )
{
  mCurve.reset( geometry );
}

QgsTextFormat QgsAnnotationLineTextItem::format() const
{
  return mTextFormat;
}

void QgsAnnotationLineTextItem::setFormat( const QgsTextFormat &format )
{
  mTextFormat = format;
}
