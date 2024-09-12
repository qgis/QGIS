/***************************************************************************
    qgsannotationrectangletextitem.cpp
    ----------------
    begin                : July 2024
    copyright            : (C) 2024 by Nyall Dawson
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

#include "qgsannotationrectangletextitem.h"
#include "qgsgeometry.h"
#include "qgsrendercontext.h"
#include "qgssymbollayerutils.h"
#include "qgslinesymbollayer.h"
#include "qgstextrenderer.h"
#include "qgsunittypes.h"
#include "qgscalloutsregistry.h"

QgsAnnotationRectangleTextItem::QgsAnnotationRectangleTextItem( const QString &text, const QgsRectangle &bounds )
  : QgsAnnotationRectItem( bounds )
  , mText( text )
{
  setBackgroundEnabled( true );
  setFrameEnabled( true );
}

QgsAnnotationRectangleTextItem::~QgsAnnotationRectangleTextItem() = default;

QString QgsAnnotationRectangleTextItem::type() const
{
  return QStringLiteral( "recttext" );
}

void QgsAnnotationRectangleTextItem::renderInBounds( QgsRenderContext &context, const QRectF &painterBounds, QgsFeedback * )
{
  const double marginLeft = context.convertToPainterUnits( mMargins.left(), mMarginUnit );
  const double marginTop = context.convertToPainterUnits( mMargins.top(), mMarginUnit );
  const double marginRight = context.convertToPainterUnits( mMargins.right(), mMarginUnit );
  const double marginBottom = context.convertToPainterUnits( mMargins.bottom(), mMarginUnit );

  const QRectF innerRect(
    painterBounds.left() + marginLeft,
    painterBounds.top() + marginTop,
    painterBounds.width() - marginLeft - marginRight,
    painterBounds.height() - marginTop - marginBottom );

  const QString displayText = QgsExpression::replaceExpressionText( mText, &context.expressionContext(), &context.distanceArea() );

  const bool prevWorkaroundFlag = context.testFlag( Qgis::RenderContextFlag::ApplyScalingWorkaroundForTextRendering );
  context.setFlag( Qgis::RenderContextFlag::ApplyScalingWorkaroundForTextRendering, true );
  QgsTextRenderer::drawText( innerRect, 0,
                             QgsTextRenderer::convertQtHAlignment( mAlignment ),
                             mTextFormat.allowHtmlFormatting() ? QStringList{displayText }: displayText.split( '\n' ), context, mTextFormat, true,
                             QgsTextRenderer::convertQtVAlignment( mAlignment ),
                             Qgis::TextRendererFlag::WrapLines );
  context.setFlag( Qgis::RenderContextFlag::ApplyScalingWorkaroundForTextRendering, prevWorkaroundFlag );
}

bool QgsAnnotationRectangleTextItem::writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  element.setAttribute( QStringLiteral( "text" ), mText );
  element.setAttribute( QStringLiteral( "alignment" ), QString::number( mAlignment ) );

  QDomElement textFormatElem = document.createElement( QStringLiteral( "rectTextFormat" ) );
  textFormatElem.appendChild( mTextFormat.writeXml( document, context ) );
  element.appendChild( textFormatElem );

  element.setAttribute( QStringLiteral( "margins" ), mMargins.toString() );
  element.setAttribute( QStringLiteral( "marginUnit" ), QgsUnitTypes::encodeUnit( mMarginUnit ) );

  writeCommonProperties( element, document, context );
  return true;
}

QgsAnnotationRectangleTextItem *QgsAnnotationRectangleTextItem::create()
{
  return new QgsAnnotationRectangleTextItem( QString(), QgsRectangle() );
}

bool QgsAnnotationRectangleTextItem::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  mText = element.attribute( QStringLiteral( "text" ) );

  const QDomElement textFormatElem = element.firstChildElement( QStringLiteral( "rectTextFormat" ) );
  if ( !textFormatElem.isNull() )
  {
    const QDomNodeList textFormatNodeList = textFormatElem.elementsByTagName( QStringLiteral( "text-style" ) );
    const QDomElement textFormatElem = textFormatNodeList.at( 0 ).toElement();
    mTextFormat.readXml( textFormatElem, context );
  }

  mMargins = QgsMargins::fromString( element.attribute( QStringLiteral( "margins" ) ) );
  mMarginUnit = QgsUnitTypes::decodeRenderUnit( element.attribute( QStringLiteral( "marginUnit" ), QgsUnitTypes::encodeUnit( Qgis::RenderUnit::Millimeters ) ) );

  mAlignment = static_cast< Qt::Alignment >( element.attribute( QStringLiteral( "alignment" ) ).toInt() );

  readCommonProperties( element, context );
  return true;
}

QgsAnnotationRectangleTextItem *QgsAnnotationRectangleTextItem::clone() const
{
  std::unique_ptr< QgsAnnotationRectangleTextItem > item = std::make_unique< QgsAnnotationRectangleTextItem >( mText, bounds() );

  item->setFormat( mTextFormat );
  item->setAlignment( mAlignment );

  item->setMargins( mMargins );
  item->setMarginsUnit( mMarginUnit );

  item->copyCommonProperties( this );
  return item.release();
}

Qgis::AnnotationItemFlags QgsAnnotationRectangleTextItem::flags() const
{
  switch ( placementMode() )
  {
    case Qgis::AnnotationPlacementMode::SpatialBounds:
      return Qgis::AnnotationItemFlag::SupportsReferenceScale
             | Qgis::AnnotationItemFlag::SupportsCallouts;
    case Qgis::AnnotationPlacementMode::FixedSize:
      return Qgis::AnnotationItemFlag::ScaleDependentBoundingBox
             | Qgis::AnnotationItemFlag::SupportsCallouts;
    case Qgis::AnnotationPlacementMode::RelativeToMapFrame:
      return Qgis::AnnotationItemFlag::ScaleDependentBoundingBox;
  }
  BUILTIN_UNREACHABLE
}

QgsTextFormat QgsAnnotationRectangleTextItem::format() const
{
  return mTextFormat;
}

void QgsAnnotationRectangleTextItem::setFormat( const QgsTextFormat &format )
{
  mTextFormat = format;
}

Qt::Alignment QgsAnnotationRectangleTextItem::alignment() const
{
  return mAlignment;
}

void QgsAnnotationRectangleTextItem::setAlignment( Qt::Alignment alignment )
{
  mAlignment = alignment;
}
