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

#include "qgsannotationitem.h"
#include "qgsannotationlineitem.h"
#include "qgsannotationlinetextitem.h"
#include "qgsannotationmarkeritem.h"
#include "qgsannotationpictureitem.h"
#include "qgsannotationpointtextitem.h"
#include "qgsannotationpolygonitem.h"
#include "qgsannotationrectangletextitem.h"

#include <QDomElement>

#include "moc_qgsannotationitemregistry.cpp"

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

  mMetadata.insert( u"marker"_s, new QgsAnnotationItemMetadata( u"marker"_s, QObject::tr( "Marker" ), QObject::tr( "Markers" ),
                    QgsAnnotationMarkerItem::create ) );
  mMetadata.insert( u"linestring"_s, new QgsAnnotationItemMetadata( u"linestring"_s, QObject::tr( "Polyline" ), QObject::tr( "Polylines" ),
                    QgsAnnotationLineItem::create ) );
  mMetadata.insert( u"polygon"_s, new QgsAnnotationItemMetadata( u"polygon"_s, QObject::tr( "Polygon" ), QObject::tr( "Polygons" ),
                    QgsAnnotationPolygonItem::create ) );
  mMetadata.insert( u"pointtext"_s, new QgsAnnotationItemMetadata( u"pointtext"_s, QObject::tr( "Text at point" ), QObject::tr( "Text at points" ),
                    QgsAnnotationPointTextItem::create ) );
  mMetadata.insert( u"linetext"_s, new QgsAnnotationItemMetadata( u"linetext"_s, QObject::tr( "Text along line" ), QObject::tr( "Text along lines" ),
                    QgsAnnotationLineTextItem::create ) );
  mMetadata.insert( u"recttext"_s, new QgsAnnotationItemMetadata( u"recttext"_s, QObject::tr( "Text in rectangle" ), QObject::tr( "Text in rectangles" ),
                    QgsAnnotationRectangleTextItem::create ) );
  mMetadata.insert( u"picture"_s, new QgsAnnotationItemMetadata( u"picture"_s, QObject::tr( "Picture" ), QObject::tr( "Pictures" ),
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
