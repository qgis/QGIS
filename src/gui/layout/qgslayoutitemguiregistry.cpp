/***************************************************************************
                            qgslayoutitemregistry.cpp
                            -------------------------
    begin                : June 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgslayoutitemguiregistry.h"
#include "qgslayoutviewrubberband.h"
#include "qgslayoutitemregistry.h"
#include "qgslayoutframe.h"
#include "qgslayoutmultiframe.h"
#include <QPainter>


QgsLayoutViewRubberBand *QgsLayoutItemAbstractGuiMetadata::createRubberBand( QgsLayoutView *view )
{
  return new QgsLayoutViewRectangularRubberBand( view );
}

QAbstractGraphicsShapeItem *QgsLayoutItemAbstractGuiMetadata::createNodeRubberBand( QgsLayoutView * )
{
  return nullptr;
}

QgsLayoutItem *QgsLayoutItemAbstractGuiMetadata::createItem( QgsLayout * )
{
  return nullptr;
}

void QgsLayoutItemAbstractGuiMetadata::newItemAddedToLayout( QgsLayoutItem * )
{

}

QgsLayoutItemGuiRegistry::QgsLayoutItemGuiRegistry( QObject *parent )
  : QObject( parent )
{
}

QgsLayoutItemGuiRegistry::~QgsLayoutItemGuiRegistry()
{
  qDeleteAll( mMetadata );
}

QgsLayoutItemAbstractGuiMetadata *QgsLayoutItemGuiRegistry::itemMetadata( int metadataId ) const
{
  return mMetadata.value( metadataId );
}

int QgsLayoutItemGuiRegistry::metadataIdForItemType( int type ) const
{
  for ( auto it = mMetadata.constBegin(); it != mMetadata.constEnd(); ++it )
  {
    if ( it.value()->type() == type )
      return it.key();
  }
  return -1;
}

bool QgsLayoutItemGuiRegistry::addLayoutItemGuiMetadata( QgsLayoutItemAbstractGuiMetadata *metadata )
{
  if ( !metadata )
    return false;

  const int id = mMetadata.count();
  mMetadata[id] = metadata;
  emit typeAdded( id );
  return true;
}

bool QgsLayoutItemGuiRegistry::addItemGroup( const QgsLayoutItemGuiGroup &group )
{
  if ( mItemGroups.contains( group.id ) )
    return false;

  mItemGroups.insert( group.id, group );
  return true;
}

const QgsLayoutItemGuiGroup &QgsLayoutItemGuiRegistry::itemGroup( const QString &id )
{
  return mItemGroups[ id ];
}

QgsLayoutItem *QgsLayoutItemGuiRegistry::createItem( int metadataId, QgsLayout *layout ) const
{
  if ( !mMetadata.contains( metadataId ) )
    return nullptr;

  std::unique_ptr< QgsLayoutItem > item( mMetadata.value( metadataId )->createItem( layout ) );
  if ( item )
    return item.release();

  const int type = mMetadata.value( metadataId )->type();
  return QgsApplication::layoutItemRegistry()->createItem( type, layout );
}

void QgsLayoutItemGuiRegistry::newItemAddedToLayout( int metadataId, QgsLayoutItem *item, const QVariantMap &properties )
{
  if ( !mMetadata.contains( metadataId ) )
    return;

  if ( QgsLayoutItemGuiMetadata *metadata = dynamic_cast<QgsLayoutItemGuiMetadata *>( mMetadata.value( metadataId ) ) )
  {
    metadata->newItemAddedToLayout( item, properties );
  }
  else
  {
    mMetadata.value( metadataId )->newItemAddedToLayout( item );
  }
}

QgsLayoutItemBaseWidget *QgsLayoutItemGuiRegistry::createItemWidget( QgsLayoutItem *item ) const
{
  if ( !item )
    return nullptr;

  int type = item->type();
  if ( type == QgsLayoutItemRegistry::LayoutFrame )
  {
    QgsLayoutMultiFrame *multiFrame = qobject_cast< QgsLayoutFrame * >( item )->multiFrame();
    if ( multiFrame )
      type = multiFrame->type();
  }
  for ( auto it = mMetadata.constBegin(); it != mMetadata.constEnd(); ++it )
  {
    if ( it.value()->type() == type )
      return it.value()->createItemWidget( item );
  }

  return nullptr;
}

QgsLayoutViewRubberBand *QgsLayoutItemGuiRegistry::createItemRubberBand( int metadataId, QgsLayoutView *view ) const
{
  if ( !mMetadata.contains( metadataId ) )
    return nullptr;

  return mMetadata[metadataId]->createRubberBand( view );
}

QAbstractGraphicsShapeItem *QgsLayoutItemGuiRegistry::createNodeItemRubberBand( int metadataId, QgsLayoutView *view )
{
  if ( !mMetadata.contains( metadataId ) )
    return nullptr;

  return mMetadata[metadataId]->createNodeRubberBand( view );
}

QList<int> QgsLayoutItemGuiRegistry::itemMetadataIds() const
{
  return mMetadata.keys();
}

QgsLayoutItem *QgsLayoutItemGuiMetadata::createItem( QgsLayout *layout )
{
  return mCreateFunc ? mCreateFunc( layout ) : QgsLayoutItemAbstractGuiMetadata::createItem( layout );
}

void QgsLayoutItemGuiMetadata::newItemAddedToLayout( QgsLayoutItem *item )
{
  if ( mAddedToLayoutFunc )
    mAddedToLayoutFunc( item, QVariantMap() );
}

void QgsLayoutItemGuiMetadata::newItemAddedToLayout( QgsLayoutItem *item, const QVariantMap &properties )
{
  if ( mAddedToLayoutFunc )
    mAddedToLayoutFunc( item, properties );
}
