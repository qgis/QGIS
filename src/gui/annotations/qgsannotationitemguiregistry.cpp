/***************************************************************************
                            qgsannotationitemguiregistry.cpp
                            --------------------------
    begin                : September 2021
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

#include "qgsannotationitemguiregistry.h"
#include "qgsannotationitemregistry.h"
#include "qgsannotationitem.h"

#include "qgsannotationitemwidget_impl.h"

QgsAnnotationItem *QgsAnnotationItemAbstractGuiMetadata::createItem()
{
  return nullptr;
}

void QgsAnnotationItemAbstractGuiMetadata::newItemAddedToLayer( QgsAnnotationItem *, QgsAnnotationLayer * )
{

}

QgsAnnotationItemGuiRegistry::QgsAnnotationItemGuiRegistry( QObject *parent )
  : QObject( parent )
{
}


QgsAnnotationItemGuiRegistry::~QgsAnnotationItemGuiRegistry()
{
  qDeleteAll( mMetadata );
}

QgsAnnotationItemAbstractGuiMetadata *QgsAnnotationItemGuiRegistry::itemMetadata( int metadataId ) const
{
  return mMetadata.value( metadataId );
}

int QgsAnnotationItemGuiRegistry::metadataIdForItemType( const QString &type ) const
{
  for ( auto it = mMetadata.constBegin(); it != mMetadata.constEnd(); ++it )
  {
    if ( it.value()->type() == type )
      return it.key();
  }
  return -1;
}

bool QgsAnnotationItemGuiRegistry::addAnnotationItemGuiMetadata( QgsAnnotationItemAbstractGuiMetadata *metadata )
{
  if ( !metadata )
    return false;

  const int id = mMetadata.count();
  mMetadata[id] = metadata;
  emit typeAdded( id );
  return true;
}

bool QgsAnnotationItemGuiRegistry::addItemGroup( const QgsAnnotationItemGuiGroup &group )
{
  if ( mItemGroups.contains( group.id ) )
    return false;

  mItemGroups.insert( group.id, group );
  return true;
}

const QgsAnnotationItemGuiGroup &QgsAnnotationItemGuiRegistry::itemGroup( const QString &id )
{
  return mItemGroups[ id ];
}

QgsAnnotationItem *QgsAnnotationItemGuiRegistry::createItem( int metadataId ) const
{
  if ( !mMetadata.contains( metadataId ) )
    return nullptr;

  std::unique_ptr< QgsAnnotationItem > item( mMetadata.value( metadataId )->createItem() );
  if ( item )
    return item.release();

  const QString type = mMetadata.value( metadataId )->type();
  return QgsApplication::annotationItemRegistry()->createItem( type );
}

void QgsAnnotationItemGuiRegistry::newItemAddedToLayer( int metadataId, QgsAnnotationItem *item, QgsAnnotationLayer *layer )
{
  if ( !mMetadata.contains( metadataId ) )
    return;

  mMetadata.value( metadataId )->newItemAddedToLayer( item, layer );
}

QgsAnnotationItemBaseWidget *QgsAnnotationItemGuiRegistry::createItemWidget( QgsAnnotationItem *item ) const
{
  if ( !item )
    return nullptr;

  const QString &type = item->type();
  for ( auto it = mMetadata.constBegin(); it != mMetadata.constEnd(); ++it )
  {
    if ( it.value()->type() == type )
      return it.value()->createItemWidget( item );
  }

  return nullptr;
}

QList<int> QgsAnnotationItemGuiRegistry::itemMetadataIds() const
{
  return mMetadata.keys();
}

void QgsAnnotationItemGuiRegistry::addDefaultItems()
{
  addAnnotationItemGuiMetadata( new QgsAnnotationItemGuiMetadata( QStringLiteral( "polygon" ),
                                QObject::tr( "Polygon" ),
                                QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddImage.svg" ) ),
                                [ = ]( QgsAnnotationItem * item )->QgsAnnotationItemBaseWidget *
  {
    QgsAnnotationPolygonItemWidget *widget = new QgsAnnotationPolygonItemWidget( nullptr );
    widget->setItem( item );
    return widget;
  } ) );

  addAnnotationItemGuiMetadata( new QgsAnnotationItemGuiMetadata( QStringLiteral( "linestring" ),
                                QObject::tr( "Line" ),
                                QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddImage.svg" ) ),
                                [ = ]( QgsAnnotationItem * item )->QgsAnnotationItemBaseWidget *
  {
    QgsAnnotationLineItemWidget *widget = new QgsAnnotationLineItemWidget( nullptr );
    widget->setItem( item );
    return widget;
  } ) );

  addAnnotationItemGuiMetadata( new QgsAnnotationItemGuiMetadata( QStringLiteral( "marker" ),
                                QObject::tr( "Marker" ),
                                QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddImage.svg" ) ),
                                [ = ]( QgsAnnotationItem * item )->QgsAnnotationItemBaseWidget *
  {
    QgsAnnotationMarkerItemWidget *widget = new QgsAnnotationMarkerItemWidget( nullptr );
    widget->setItem( item );
    return widget;
  } ) );
}

QgsAnnotationItem *QgsAnnotationItemGuiMetadata::createItem()
{
  return mCreateFunc ? mCreateFunc() : QgsAnnotationItemAbstractGuiMetadata::createItem();
}

void QgsAnnotationItemGuiMetadata::newItemAddedToLayer( QgsAnnotationItem *item, QgsAnnotationLayer *layer )
{
  if ( mAddedToLayerFunc )
    mAddedToLayerFunc( item, layer );
}
