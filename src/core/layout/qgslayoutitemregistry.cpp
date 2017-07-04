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

#include "qgslayoutitemregistry.h"

QgsLayoutItemRegistry::QgsLayoutItemRegistry( QObject *parent )
  : QObject( parent )
{

}

QgsLayoutItemRegistry::~QgsLayoutItemRegistry()
{
  qDeleteAll( mMetadata );
}

QgsLayoutItemAbstractMetadata *QgsLayoutItemRegistry::itemMetadata( int type ) const
{
  return mMetadata.value( type );
}

bool QgsLayoutItemRegistry::addLayoutItemType( QgsLayoutItemAbstractMetadata *metadata )
{
  if ( !metadata || mMetadata.contains( metadata->type() ) )
    return false;

  mMetadata[metadata->type()] = metadata;
  emit typeAdded( metadata->type(), metadata->visibleName() );
  return true;
}

QgsLayoutItem *QgsLayoutItemRegistry::createItem( int type, QgsLayout *layout, const QVariantMap &properties ) const
{
  if ( !mMetadata.contains( type ) )
    return nullptr;

  return mMetadata[type]->createItem( layout, properties );
}

QWidget *QgsLayoutItemRegistry::createItemWidget( int type ) const
{
  if ( !mMetadata.contains( type ) )
    return nullptr;

  return mMetadata[type]->createItemWidget();
}

QgsLayoutViewRubberBand *QgsLayoutItemRegistry::createItemRubberBand( int type, QgsLayoutView *view ) const
{
  if ( mRubberBandFunctions.contains( type ) )
    return mRubberBandFunctions.value( type )( view );

  if ( !mMetadata.contains( type ) )
    return nullptr;

  return mMetadata[type]->createRubberBand( view );
}

void QgsLayoutItemRegistry::resolvePaths( int type, QVariantMap &properties, const QgsPathResolver &pathResolver, bool saving ) const
{
  if ( !mMetadata.contains( type ) )
    return;

  mMetadata[type]->resolvePaths( properties, pathResolver, saving );

}

QMap<int, QString> QgsLayoutItemRegistry::itemTypes() const
{
  QMap<int, QString> types;
  QMap<int, QgsLayoutItemAbstractMetadata *>::ConstIterator it = mMetadata.constBegin();
  for ( ; it != mMetadata.constEnd(); ++it )
  {
    types.insert( it.key(), it.value()->visibleName() );
  }
  return types;
}
