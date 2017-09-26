#include "qgschunklist_p.h"

#include "qgschunknode_p.h"

///@cond PRIVATE

QgsChunkList::QgsChunkList()
  : mHead( nullptr )
  , mTail( nullptr )
  , mCount( 0 )
{
}

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
    if ( next == nullptr )
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
      mHead = entry;   // update head if "entry" we was head before
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
  return mHead == nullptr;
}

/// @endcond
