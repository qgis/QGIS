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


QgsLayoutItemGuiRegistry::QgsLayoutItemGuiRegistry( QObject *parent )
  : QObject( parent )
{
}

QgsLayoutItemGuiRegistry::~QgsLayoutItemGuiRegistry()
{
  qDeleteAll( mMetadata );
}

QgsLayoutItemAbstractGuiMetadata *QgsLayoutItemGuiRegistry::itemMetadata( int type ) const
{
  return mMetadata.value( type );
}

bool QgsLayoutItemGuiRegistry::addLayoutItemGuiMetadata( QgsLayoutItemAbstractGuiMetadata *metadata )
{
  if ( !metadata || mMetadata.contains( metadata->type() ) )
    return false;

  mMetadata[metadata->type()] = metadata;
  emit typeAdded( metadata->type() );
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

QgsLayoutItemBaseWidget *QgsLayoutItemGuiRegistry::createItemWidget( QgsLayoutItem *item ) const
{
  if ( !item )
    return nullptr;

  if ( !mMetadata.contains( item->type() ) )
    return nullptr;

  return mMetadata[item->type()]->createItemWidget( item );
}

QgsLayoutViewRubberBand *QgsLayoutItemGuiRegistry::createItemRubberBand( int type, QgsLayoutView *view ) const
{
  if ( !mMetadata.contains( type ) )
    return nullptr;

  return mMetadata[type]->createRubberBand( view );
}

QList<int> QgsLayoutItemGuiRegistry::itemTypes() const
{
  return mMetadata.keys();
}
