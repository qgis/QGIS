/***************************************************************************
                            qgsannotationitemregistry.cpp
                            -------------------------
    begin                : October 2019
    copyright            : (C) 2019 by Nyall Dawson
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

#include "qgsannotationitemregistry.h"
#include "moc_qgsannotationitemregistry.cpp"
#include "qgsannotationitem.h"
#include "qgsannotationmarkeritem.h"
#include "qgsannotationlineitem.h"
#include "qgsannotationpolygonitem.h"
#include "qgsannotationpointtextitem.h"
#include "qgsannotationlinetextitem.h"
#include "qgsannotationrectangletextitem.h"
#include "qgsannotationpictureitem.h"
#include <QDomElement>

QgsAnnotationItemRegistry::QgsAnnotationItemRegistry( QObject *parent )
  : QObject( parent )
{
}

QgsAnnotationItemRegistry::~QgsAnnotationItemRegistry()
{
  qDeleteAll( mMetadata );
}

bool QgsAnnotationItemRegistry::populate()
{
  if ( !mMetadata.isEmpty() )
    return false;

  mMetadata.insert( QStringLiteral( "marker" ), new QgsAnnotationItemMetadata( QStringLiteral( "marker" ), QObject::tr( "Marker" ), QObject::tr( "Markers" ),
                    QgsAnnotationMarkerItem::create ) );
  mMetadata.insert( QStringLiteral( "linestring" ), new QgsAnnotationItemMetadata( QStringLiteral( "linestring" ), QObject::tr( "Polyline" ), QObject::tr( "Polylines" ),
                    QgsAnnotationLineItem::create ) );
  mMetadata.insert( QStringLiteral( "polygon" ), new QgsAnnotationItemMetadata( QStringLiteral( "polygon" ), QObject::tr( "Polygon" ), QObject::tr( "Polygons" ),
                    QgsAnnotationPolygonItem::create ) );
  mMetadata.insert( QStringLiteral( "pointtext" ), new QgsAnnotationItemMetadata( QStringLiteral( "pointtext" ), QObject::tr( "Text at point" ), QObject::tr( "Text at points" ),
                    QgsAnnotationPointTextItem::create ) );
  mMetadata.insert( QStringLiteral( "linetext" ), new QgsAnnotationItemMetadata( QStringLiteral( "linetext" ), QObject::tr( "Text along line" ), QObject::tr( "Text along lines" ),
                    QgsAnnotationLineTextItem::create ) );
  mMetadata.insert( QStringLiteral( "recttext" ), new QgsAnnotationItemMetadata( QStringLiteral( "recttext" ), QObject::tr( "Text in rectangle" ), QObject::tr( "Text in rectangles" ),
                    QgsAnnotationRectangleTextItem::create ) );
  mMetadata.insert( QStringLiteral( "picture" ), new QgsAnnotationItemMetadata( QStringLiteral( "picture" ), QObject::tr( "Picture" ), QObject::tr( "Pictures" ),
                    QgsAnnotationPictureItem::create ) );
  return true;
}

QgsAnnotationItemAbstractMetadata *QgsAnnotationItemRegistry::itemMetadata( const QString &type ) const
{
  return mMetadata.value( type );
}

bool QgsAnnotationItemRegistry::addItemType( QgsAnnotationItemAbstractMetadata *metadata )
{
  if ( !metadata || mMetadata.contains( metadata->type() ) )
    return false;

  mMetadata[metadata->type()] = metadata;
  emit typeAdded( metadata->type(), metadata->visibleName() );
  return true;
}

QgsAnnotationItem *QgsAnnotationItemRegistry::createItem( const QString &type ) const
{
  if ( !mMetadata.contains( type ) )
    return nullptr;

  return mMetadata[type]->createItem();
}

QMap<QString, QString> QgsAnnotationItemRegistry::itemTypes() const
{
  QMap<QString, QString> types;
  for ( auto it = mMetadata.constBegin(); it != mMetadata.constEnd(); ++it )
  {
    types.insert( it.key(), it.value()->visibleName() );
  }
  return types;
}
