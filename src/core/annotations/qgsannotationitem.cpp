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

QgsAnnotationItemEditOperationTransientResults *QgsAnnotationItem::transientEditResults( QgsAbstractAnnotationItemEditOperation * )
{
  return nullptr;
}

QList<QgsAnnotationItemNode> QgsAnnotationItem::nodes() const
{
  return {};
}

void QgsAnnotationItem::copyCommonProperties( const QgsAnnotationItem *other )
{
  setZIndex( other->zIndex() );
  setUseSymbologyReferenceScale( other->useSymbologyReferenceScale() );
  setSymbologyReferenceScale( other->symbologyReferenceScale() );
}

bool QgsAnnotationItem::writeCommonProperties( QDomElement &element, QDomDocument &, const QgsReadWriteContext & ) const
{
  element.setAttribute( QStringLiteral( "zIndex" ), zIndex() );
  element.setAttribute( QStringLiteral( "useReferenceScale" ), useSymbologyReferenceScale() ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  element.setAttribute( QStringLiteral( "referenceScale" ), qgsDoubleToString( symbologyReferenceScale() ) );
  return true;
}

bool QgsAnnotationItem::readCommonProperties( const QDomElement &element, const QgsReadWriteContext & )
{
  setZIndex( element.attribute( QStringLiteral( "zIndex" ) ).toInt() );
  setUseSymbologyReferenceScale( element.attribute( QStringLiteral( "useReferenceScale" ), QStringLiteral( "0" ) ).toInt() );
  setSymbologyReferenceScale( element.attribute( QStringLiteral( "referenceScale" ) ).toDouble() );
  return true;
}
