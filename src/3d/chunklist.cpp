#include "chunklist.h"

#include "chunknode.h"

ChunkList::ChunkList()
  : mHead( nullptr )
  , mTail( nullptr )
  , mCount( 0 )
{
}

int ChunkList::trueCount() const
{
  int len = 0;
  ChunkListEntry *entry = mHead;
  while ( entry )
  {
    ++len;
    entry = entry->next;
  }
  return len;
}

void ChunkList::insertEntry( ChunkListEntry *entry, ChunkListEntry *next )
{
  if ( !mHead )
  {
    Q_ASSERT( next == nullptr );
    mTail = mHead = entry;
  }
  else
  {
    entry->next = next;
    entry->prev = next->prev;
    next->prev = entry;
    if ( next == mHead )
      mHead = entry;   // update head if "entry" we was head before
  }
  ++mCount;
}

void ChunkList::takeEntry( ChunkListEntry *entry )
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

ChunkListEntry *ChunkList::takeFirst()
{
  ChunkListEntry *entry = mHead;
  takeEntry( entry );
  return entry;
}

ChunkListEntry *ChunkList::takeLast()
{
  ChunkListEntry *entry = mTail;
  takeEntry( entry );
  return entry;
}

void ChunkList::insertFirst( ChunkListEntry *entry )
{
  insertEntry( entry, mHead );
}

bool ChunkList::isEmpty() const
{
  return mHead == nullptr;
}
