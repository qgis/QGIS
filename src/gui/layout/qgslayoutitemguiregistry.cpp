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
#include <QPainter>


QgsLayoutViewRubberBand *QgsLayoutItemAbstractGuiMetadata::createRubberBand( QgsLayoutView *view )
{
  return new QgsLayoutViewRectangularRubberBand( view );
}

QgsLayoutItem *QgsLayoutItemAbstractGuiMetadata::createItem( QgsLayout * )
{
  return nullptr;
}

QgsLayoutItemGuiRegistry::QgsLayoutItemGuiRegistry( QObject *parent )
  : QObject( parent )
{
}

QgsLayoutItemGuiRegistry::~QgsLayoutItemGuiRegistry()
{
  qDeleteAll( mMetadata );
}

QgsLayoutItemAbstractGuiMetadata *QgsLayoutItemGuiRegistry::itemMetadata( const QString &uuid ) const
{
  return mMetadata.value( uuid );
}

bool QgsLayoutItemGuiRegistry::addLayoutItemGuiMetadata( QgsLayoutItemAbstractGuiMetadata *metadata )
{
  if ( !metadata )
    return false;

  QString uuid = QUuid::createUuid().toString();
  mMetadata[uuid] = metadata;
  emit typeAdded( uuid );
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

QgsLayoutItem *QgsLayoutItemGuiRegistry::createItem( const QString &uuid, QgsLayout *layout ) const
{
  if ( !mMetadata.contains( uuid ) )
    return nullptr;

  std::unique_ptr< QgsLayoutItem > item( mMetadata.value( uuid )->createItem( layout ) );
  if ( item )
    return item.release();

  int type = mMetadata.value( uuid )->type();
  return QgsApplication::layoutItemRegistry()->createItem( type, layout );
}

QgsLayoutItemBaseWidget *QgsLayoutItemGuiRegistry::createItemWidget( QgsLayoutItem *item ) const
{
  if ( !item )
    return nullptr;

  for ( auto it = mMetadata.constBegin(); it != mMetadata.constEnd(); ++it )
  {
    if ( it.value()->type() == item->type() )
      return it.value()->createItemWidget( item );
  }

  return nullptr;
}

QgsLayoutViewRubberBand *QgsLayoutItemGuiRegistry::createItemRubberBand( const QString &uuid, QgsLayoutView *view ) const
{
  if ( !mMetadata.contains( uuid ) )
    return nullptr;

  return mMetadata[uuid]->createRubberBand( view );
}

QList<QString> QgsLayoutItemGuiRegistry::itemUuids() const
{
  return mMetadata.keys();
}

QgsLayoutItem *QgsLayoutItemGuiMetadata::createItem( QgsLayout *layout )
{
  return mCreateFunc ? mCreateFunc( layout ) : QgsLayoutItemAbstractGuiMetadata::createItem( layout );
}
