/***************************************************************************
                            qgshistoryentrynode.cpp
                            --------------------------
    begin                : Aprial 2023
    copyright            : (C) 2023 by Nyall Dawson
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
#include "qgshistoryentrynode.h"

//
// QgsHistoryEntryNode
//

QgsHistoryEntryNode::~QgsHistoryEntryNode() = default;

int QgsHistoryEntryNode::childCount() const
{
  return 0;
}

#if 0
QString QgsHistoryEntryNode::html() const
{
  return QString();
}

QWidget *QgsHistoryEntryNode::createWidget()
{
  return nullptr;
}

QList<QAction *> QgsHistoryEntryNode::actions( QWidget * )
{
  return {};
}

bool QgsHistoryEntryNode::matchesString( const QString & )
{
  return false;
}
#endif

//
// QgsHistoryEntryGroup
//

QgsHistoryEntryGroup::~QgsHistoryEntryGroup() = default;

void QgsHistoryEntryGroup::addChild( QgsHistoryEntryNode *child )
{
  if ( !child )
    return;

  Q_ASSERT( !child->mParent );
  child->mParent = this;

  mChildren.emplace_back( child );
}

void QgsHistoryEntryGroup::insertChild( int index, QgsHistoryEntryNode *child )
{
  if ( !child )
    return;

  Q_ASSERT( !child->mParent );
  child->mParent = this;

  mChildren.insert( mChildren.begin() + index, std::unique_ptr< QgsHistoryEntryNode >( child ) );
}

int QgsHistoryEntryGroup::indexOf( QgsHistoryEntryNode *child ) const
{
  if ( child->mParent != this )
    return -1;

  auto it = std::find_if( mChildren.begin(), mChildren.end(), [&]( const std::unique_ptr<QgsHistoryEntryNode> &p )
  {
    return p.get() == child;
  } );
  if ( it != mChildren.end() )
    return std::distance( mChildren.begin(), it );
  return -1;
}

QgsHistoryEntryNode *QgsHistoryEntryGroup::childAt( int index )
{
  if ( static_cast< std::size_t >( index ) >= mChildren.size() )
    return nullptr;

  return mChildren[ index ].get();

}

void QgsHistoryEntryGroup::removeChildAt( int index )
{
  if ( static_cast< std::size_t >( index ) >= mChildren.size() )
    return;

  mChildren.erase( mChildren.begin() + index );
}

void QgsHistoryEntryGroup::clear()
{
  mChildren.clear();
}

int QgsHistoryEntryGroup::childCount() const
{
  return mChildren.size();
}
