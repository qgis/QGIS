/***************************************************************************
  qgschunklist_p.cpp
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgschunklist_p.h"

#include "qgschunknode.h"

///@cond PRIVATE

int QgsChunkList::trueCount() const
{
  int len = 0;
  QgsChunkListEntry *entry = mHead;
  while ( entry )
  {
    ++len;
    entry = entry->next;
  }
  return len;
}

void QgsChunkList::insertEntry( QgsChunkListEntry *entry, QgsChunkListEntry *next )
{
  if ( !mHead )
  {
    Q_ASSERT( next == nullptr );
    mTail = mHead = entry;
  }
  else
  {
    entry->next = next;
    if ( !next )
    {
      entry->prev = mTail;
      mTail->next = entry;
      mTail = entry;
    }
    else
    {
      entry->prev = next->prev;
      next->prev = entry;
    }
    if ( next == mHead )
      mHead = entry; // update head if "entry" we was head before
  }
  ++mCount;
}

void QgsChunkList::takeEntry( QgsChunkListEntry *entry )
{
  Q_ASSERT( entry );

  if ( !entry->prev && !entry->next )
  {
    // last item in the list
    Q_ASSERT( mHead == entry && mTail == entry );
    mHead = mTail = nullptr;
  }
  else if ( !entry->prev )
  {
    // head item
    Q_ASSERT( mHead == entry );
    entry->next->prev = nullptr;
    mHead = entry->next;
    entry->next = nullptr;
  }
  else if ( !entry->next )
  {
    // tail item
    Q_ASSERT( mTail == entry );
    entry->prev->next = nullptr;
    mTail = entry->prev;
    entry->prev = nullptr;
  }
  else
  {
    // ordinary item
    entry->prev->next = entry->next;
    entry->next->prev = entry->prev;
    entry->next = nullptr;
    entry->prev = nullptr;
  }
  --mCount;
  Q_ASSERT( !entry->prev );
  Q_ASSERT( !entry->next );
}

QgsChunkListEntry *QgsChunkList::takeFirst()
{
  QgsChunkListEntry *entry = mHead;
  takeEntry( entry );
  return entry;
}

QgsChunkListEntry *QgsChunkList::takeLast()
{
  QgsChunkListEntry *entry = mTail;
  takeEntry( entry );
  return entry;
}

void QgsChunkList::insertFirst( QgsChunkListEntry *entry )
{
  insertEntry( entry, mHead );
}

void QgsChunkList::insertLast( QgsChunkListEntry *entry )
{
  insertEntry( entry, nullptr );
}

bool QgsChunkList::isEmpty() const
{
  return !mHead;
}

/// @endcond
