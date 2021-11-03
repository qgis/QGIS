/***************************************************************************
    qgsannotationpointtextitem.cpp
    ----------------
    begin                : August 2020
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

#include "qgsannotationpointtextitem.h"
#include "qgstextrenderer.h"
#include "qgsannotationitemnode.h"
#include "qgsannotationitemeditoperation.h"
#include "qgsrendercontext.h"

QgsAnnotationPointTextItem::QgsAnnotationPointTextItem( const QString &text, QgsPointXY point )
  : QgsAnnotationItem()
  , mText( text )
  , mPoint( point )
{

}

Qgis::AnnotationItemFlags QgsAnnotationPointTextItem::flags() const
{
  // in truth this should depend on whether the text format is scale dependent or not!
  return Qgis::AnnotationItemFlag::ScaleDependentBoundingBox;
}

QgsAnnotationPointTextItem::~QgsAnnotationPointTextItem() = default;

QString QgsAnnotationPointTextItem::type() const
{
  return QStringLiteral( "pointtext" );
}

void QgsAnnotationPointTextItem::render( QgsRenderContext &context, QgsFeedback * )
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

  const QString displayText = QgsExpression::replaceExpressionText( mText, &context.expressionContext(), &context.distanceArea() );
  QgsTextRenderer::drawText( pt, mAngle * M_PI / 180.0,
                             QgsTextRenderer::convertQtHAlignment( mAlignment ),
                             displayText.split( '\n' ), context, mTextFormat );
}

bool QgsAnnotationPointTextItem::writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  element.setAttribute( QStringLiteral( "x" ), qgsDoubleToString( mPoint.x() ) );
  element.setAttribute( QStringLiteral( "y" ), qgsDoubleToString( mPoint.y() ) );
  element.setAttribute( QStringLiteral( "text" ), mText );
  element.setAttribute( QStringLiteral( "angle" ), qgsDoubleToString( mAngle ) );
  element.setAttribute( QStringLiteral( "alignment" ), QString::number( mAlignment ) );

  QDomElement textFormatElem = document.createElement( QStringLiteral( "pointTextFormat" ) );
  textFormatElem.appendChild( mTextFormat.writeXml( document, context ) );
  element.appendChild( textFormatElem );

  writeCommonProperties( element, document, context );
  return true;
}

QgsAnnotationPointTextItem *QgsAnnotationPointTextItem::create()
{
  return new QgsAnnotationPointTextItem( QString(), QgsPointXY() );
}

bool QgsAnnotationPointTextItem::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  const double x = element.attribute( QStringLiteral( "x" ) ).toDouble();
  const double y = element.attribute( QStringLiteral( "y" ) ).toDouble();
  mPoint = QgsPointXY( x, y );
  mText = element.attribute( QStringLiteral( "text" ) );
  mAngle = element.attribute( QStringLiteral( "angle" ) ).toDouble();
  mAlignment = static_cast< Qt::Alignment >( element.attribute( QStringLiteral( "alignment" ) ).toInt() );

  const QDomElement textFormatElem = element.firstChildElement( QStringLiteral( "pointTextFormat" ) );
  if ( !textFormatElem.isNull() )
  {
    const QDomNodeList textFormatNodeList = textFormatElem.elementsByTagName( QStringLiteral( "text-style" ) );
    const QDomElement textFormatElem = textFormatNodeList.at( 0 ).toElement();
    mTextFormat.readXml( textFormatElem, context );
  }

  readCommonProperties( element, context );
  return true;
}

QgsAnnotationPointTextItem *QgsAnnotationPointTextItem::clone()
{
  std::unique_ptr< QgsAnnotationPointTextItem > item = std::make_unique< QgsAnnotationPointTextItem >( mText, mPoint );
  item->setFormat( mTextFormat );
  item->setAngle( mAngle );
  item->setAlignment( mAlignment );
  item->copyCommonProperties( this );
  return item.release();
}

QgsRectangle QgsAnnotationPointTextItem::boundingBox() const
{
  return QgsRectangle( mPoint.x(), mPoint.y(), mPoint.x(), mPoint.y() );
}

QgsRectangle QgsAnnotationPointTextItem::boundingBox( QgsRenderContext &context ) const
{
  const QString displayText = QgsExpression::replaceExpressionText( mText, &context.expressionContext(), &context.distanceArea() );

  const double widthInPixels = QgsTextRenderer::textWidth( context, mTextFormat, displayText.split( '\n' ) );
  const double heightInPixels = QgsTextRenderer::textHeight( context, mTextFormat, displayText.split( '\n' ) );

  // text size has already been calculated using any symbology reference scale factor above -- we need
  // to temporarily remove the reference scale here or we'll be undoing the scaling
  QgsScopedRenderContextReferenceScaleOverride resetScaleFactor( context, -1.0 );
  const double widthInMapUnits = context.convertToMapUnits( widthInPixels, QgsUnitTypes::RenderPixels );
  const double heightInMapUnits = context.convertToMapUnits( heightInPixels, QgsUnitTypes::RenderPixels );

  return QgsRectangle( mPoint.x(), mPoint.y(), mPoint.x() + widthInMapUnits, mPoint.y() + heightInMapUnits );
}

QList<QgsAnnotationItemNode> QgsAnnotationPointTextItem::nodes() const
{
  return { QgsAnnotationItemNode( QgsVertexId( 0, 0, 0 ), mPoint, Qgis::AnnotationItemNodeType::VertexHandle )};
}

Qgis::AnnotationItemEditOperationResult QgsAnnotationPointTextItem::applyEdit( QgsAbstractAnnotationItemEditOperation *operation )
{
  switch ( operation->type() )
  {
    case QgsAbstractAnnotationItemEditOperation::Type::MoveNode:
    {
      QgsAnnotationItemEditOperationMoveNode *moveOperation = dynamic_cast< QgsAnnotationItemEditOperationMoveNode * >( operation );
      mPoint = moveOperation->after();
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

QgsAnnotationItemEditOperationTransientResults *QgsAnnotationPointTextItem::transientEditResults( QgsAbstractAnnotationItemEditOperation *operation )
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

QgsTextFormat QgsAnnotationPointTextItem::format() const
{
  return mTextFormat;
}

void QgsAnnotationPointTextItem::setFormat( const QgsTextFormat &format )
{
  mTextFormat = format;
}

Qt::Alignment QgsAnnotationPointTextItem::alignment() const
{
  return mAlignment;
}

void QgsAnnotationPointTextItem::setAlignment( Qt::Alignment alignment )
{
  mAlignment = alignment;
}
