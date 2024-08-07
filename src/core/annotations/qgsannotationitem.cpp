/***************************************************************************
                             qgsannotationitem.cpp
                             -----------------
    begin                : August 2021
    copyright            : (C) 2021 by Nyall Dawson
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

#include "qgsannotationitem.h"
#include "qgsannotationitemnode.h"
#include "qgscalloutsregistry.h"
#include "qgsapplication.h"
#include "qgsrendercontext.h"
#include "qgssymbollayerutils.h"
#include "qgsunittypes.h"

QgsAnnotationItem::QgsAnnotationItem() = default;

QgsAnnotationItem::~QgsAnnotationItem() = default;

Qgis::AnnotationItemFlags QgsAnnotationItem::flags() const
{
  return Qgis::AnnotationItemFlags();
}

Qgis::AnnotationItemEditOperationResult QgsAnnotationItem::applyEdit( QgsAbstractAnnotationItemEditOperation * )
{
  return Qgis::AnnotationItemEditOperationResult::Invalid;
}

Qgis::AnnotationItemEditOperationResult QgsAnnotationItem::applyEditV2( QgsAbstractAnnotationItemEditOperation *operation, const QgsAnnotationItemEditContext & )
{
  Q_NOWARN_DEPRECATED_PUSH
  return applyEdit( operation );
  Q_NOWARN_DEPRECATED_POP
}

QgsAnnotationItemEditOperationTransientResults *QgsAnnotationItem::transientEditResults( QgsAbstractAnnotationItemEditOperation * )
{
  return nullptr;
}

QgsAnnotationItemEditOperationTransientResults *QgsAnnotationItem::transientEditResultsV2( QgsAbstractAnnotationItemEditOperation *operation, const QgsAnnotationItemEditContext & )
{
  Q_NOWARN_DEPRECATED_PUSH
  return transientEditResults( operation );
  Q_NOWARN_DEPRECATED_POP
}

QList<QgsAnnotationItemNode> QgsAnnotationItem::nodes() const
{
  return {};
}

QList<QgsAnnotationItemNode> QgsAnnotationItem::nodesV2( const QgsAnnotationItemEditContext & ) const
{
  Q_NOWARN_DEPRECATED_PUSH
  return nodes();
  Q_NOWARN_DEPRECATED_POP
}

QgsCallout *QgsAnnotationItem::callout() const
{
  return mCallout.get();
}

void QgsAnnotationItem::setCallout( QgsCallout *callout )
{
  mCallout.reset( callout );
}

QgsGeometry QgsAnnotationItem::calloutAnchor() const
{
  return mCalloutAnchor;
}

void QgsAnnotationItem::setCalloutAnchor( const QgsGeometry &anchor )
{
  mCalloutAnchor = anchor;
}

void QgsAnnotationItem::copyCommonProperties( const QgsAnnotationItem *other )
{
  setEnabled( other->enabled() );
  setZIndex( other->zIndex() );
  setUseSymbologyReferenceScale( other->useSymbologyReferenceScale() );
  setSymbologyReferenceScale( other->symbologyReferenceScale() );
  if ( QgsCallout *callout = other->callout() )
    setCallout( callout->clone() );
  else
    setCallout( nullptr );
  setCalloutAnchor( other->calloutAnchor() );
  setOffsetFromCallout( other->offsetFromCallout() );
  setOffsetFromCalloutUnit( other->offsetFromCalloutUnit() );
}

bool QgsAnnotationItem::writeCommonProperties( QDomElement &element, QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  element.setAttribute( QStringLiteral( "enabled" ), static_cast<int>( enabled() ) );
  element.setAttribute( QStringLiteral( "zIndex" ), zIndex() );
  element.setAttribute( QStringLiteral( "useReferenceScale" ), useSymbologyReferenceScale() ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  element.setAttribute( QStringLiteral( "referenceScale" ), qgsDoubleToString( symbologyReferenceScale() ) );

  if ( mCallout )
  {
    element.setAttribute( QStringLiteral( "calloutType" ), mCallout->type() );
    mCallout->saveProperties( doc, element, context );
  }
  if ( !mCalloutAnchor.isEmpty() )
  {
    element.setAttribute( QStringLiteral( "calloutAnchor" ), mCalloutAnchor.asWkt() );
  }
  if ( mOffsetFromCallout.isValid() )
  {
    element.setAttribute( QStringLiteral( "offsetFromCallout" ), QgsSymbolLayerUtils::encodeSize( mOffsetFromCallout ) );
    element.setAttribute( QStringLiteral( "offsetFromCalloutUnit" ), QgsUnitTypes::encodeUnit( mOffsetFromCalloutUnit ) );
  }
  return true;
}

bool QgsAnnotationItem::readCommonProperties( const QDomElement &element, const QgsReadWriteContext &context )
{
  setEnabled( element.attribute( QStringLiteral( "enabled" ), QStringLiteral( "1" ) ).toInt() );
  setZIndex( element.attribute( QStringLiteral( "zIndex" ) ).toInt() );
  setUseSymbologyReferenceScale( element.attribute( QStringLiteral( "useReferenceScale" ), QStringLiteral( "0" ) ).toInt() );
  setSymbologyReferenceScale( element.attribute( QStringLiteral( "referenceScale" ) ).toDouble() );

  const QString calloutType = element.attribute( QStringLiteral( "calloutType" ) );
  if ( calloutType.isEmpty() )
  {
    mCallout.reset();
  }
  else
  {
    mCallout.reset( QgsApplication::calloutRegistry()->createCallout( calloutType, element.firstChildElement( QStringLiteral( "callout" ) ), context ) );
    if ( !mCallout )
      mCallout.reset( QgsCalloutRegistry::defaultCallout() );
  }

  const QString calloutAnchorWkt = element.attribute( QStringLiteral( "calloutAnchor" ) );
  setCalloutAnchor( calloutAnchorWkt.isEmpty() ? QgsGeometry() : QgsGeometry::fromWkt( calloutAnchorWkt ) );

  mOffsetFromCallout = element.attribute( QStringLiteral( "offsetFromCallout" ) ).isEmpty() ? QSizeF() : QgsSymbolLayerUtils::decodeSize( element.attribute( QStringLiteral( "offsetFromCallout" ) ) );
  bool ok = false;
  mOffsetFromCalloutUnit = QgsUnitTypes::decodeRenderUnit( element.attribute( QStringLiteral( "offsetFromCalloutUnit" ) ), &ok );
  if ( !ok )
    mOffsetFromCalloutUnit = Qgis::RenderUnit::Millimeters;

  return true;
}

void QgsAnnotationItem::renderCallout( QgsRenderContext &context, const QRectF &rect, double angle, QgsCallout::QgsCalloutContext &calloutContext, QgsFeedback * )
{
  if ( !mCallout || mCalloutAnchor.isEmpty() )
    return;

  // anchor must be in painter coordinates
  QgsGeometry anchor = mCalloutAnchor;
  if ( context.coordinateTransform().isValid() )
  {
    try
    {
      anchor.transform( context.coordinateTransform() );
    }
    catch ( QgsCsException & )
    {
      return;
    }
  }
  anchor.transform( context.mapToPixel().transform() );

  mCallout->startRender( context );
  mCallout->render( context, rect, angle, anchor, calloutContext );
  mCallout->stopRender( context );
}

Qgis::RenderUnit QgsAnnotationItem::offsetFromCalloutUnit() const
{
  return mOffsetFromCalloutUnit;
}

void QgsAnnotationItem::setOffsetFromCalloutUnit( Qgis::RenderUnit unit )
{
  mOffsetFromCalloutUnit = unit;
}

QSizeF QgsAnnotationItem::offsetFromCallout() const
{
  return mOffsetFromCallout;
}

void QgsAnnotationItem::setOffsetFromCallout( const QSizeF &offset )
{
  mOffsetFromCallout = offset;
}
