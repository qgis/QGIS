#ifndef CHUNKLIST_H
#define CHUNKLIST_H

class ChunkNode;

//! Element of a double-linked list
class ChunkListEntry
{
  public:
    //! Constructs entry for a particular node
    ChunkListEntry( ChunkNode *node )
      : prev( nullptr )
      , next( nullptr )
      , chunk( node )
    {
    }

    ChunkListEntry *prev;
    ChunkListEntry *next;

    ChunkNode *chunk;   //!< TODO: shared pointer
};


/** \ingroup 3d
 * Double linked list of chunks.
 * The list does not own entries.
 *
 * Why having another linked list structure if there is already QLinkedList template?
 * This list implementation allows tighter integration with chunk nodes: they keep
 * pointers to list entries, so it is possible to locate their entry in the list in constant
 * time (rather than having to search the whole list). This feature is very useful
 * in loader and replacement queues where entries are often taken out of the list
 * and inserted at the front again.
 * \since QGIS 3.0
 */
class ChunkList
{
  public:
    ChunkList();

    //! Counts the real number of entries by walking the list (for debugging purposes only)
    int trueCount() const;
    //! Returns number of entries in the list
    int count() const { return mCount; }

    //! Returns the first entry. Null will be returned if the list is empty.
    ChunkListEntry *first() const { return mHead; }
    //! Returns the last entry. Null will be returned if the list is empty.
    ChunkListEntry *last() const { return mTail; }
    //! Returns whether the list is empty or it contains some entries
    bool isEmpty() const;

    //! Inserts a new entry before the entry "next".
    //! If "next" is null, entry will be inserted at the end of the list
    void insertEntry( ChunkListEntry *entry, ChunkListEntry *next );

    //! Takes the entry out of the list (does not delete it)
    void takeEntry( ChunkListEntry *entry );
    //! Takes the first entry from the list and returns it
    ChunkListEntry *takeFirst();
    //! Takes the last entry from the list and returns it
    ChunkListEntry *takeLast();

    //! Inserts an entry at the start of the list
    void insertFirst( ChunkListEntry *entry );
    //! Inserts an entry at the end of the list
    void insertLast( ChunkListEntry *entry );

  private:
    ChunkListEntry *mHead;
    ChunkListEntry *mTail;
    int mCount;
};

#endif // CHUNKLIST_H
