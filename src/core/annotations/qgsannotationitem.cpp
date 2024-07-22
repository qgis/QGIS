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

void QgsAnnotationItem::copyCommonProperties( const QgsAnnotationItem *other )
{
  setEnabled( other->enabled() );
  setZIndex( other->zIndex() );
  setUseSymbologyReferenceScale( other->useSymbologyReferenceScale() );
  setSymbologyReferenceScale( other->symbologyReferenceScale() );
}

bool QgsAnnotationItem::writeCommonProperties( QDomElement &element, QDomDocument &, const QgsReadWriteContext & ) const
{
  element.setAttribute( QStringLiteral( "enabled" ), static_cast<int>( enabled() ) );
  element.setAttribute( QStringLiteral( "zIndex" ), zIndex() );
  element.setAttribute( QStringLiteral( "useReferenceScale" ), useSymbologyReferenceScale() ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  element.setAttribute( QStringLiteral( "referenceScale" ), qgsDoubleToString( symbologyReferenceScale() ) );
  return true;
}

bool QgsAnnotationItem::readCommonProperties( const QDomElement &element, const QgsReadWriteContext & )
{
  setEnabled( element.attribute( QStringLiteral( "enabled" ), QStringLiteral( "1" ) ).toInt() );
  setZIndex( element.attribute( QStringLiteral( "zIndex" ) ).toInt() );
  setUseSymbologyReferenceScale( element.attribute( QStringLiteral( "useReferenceScale" ), QStringLiteral( "0" ) ).toInt() );
  setSymbologyReferenceScale( element.attribute( QStringLiteral( "referenceScale" ) ).toDouble() );
  return true;
}
