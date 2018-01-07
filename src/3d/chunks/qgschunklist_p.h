/***************************************************************************
  qgschunklist_p.h
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

#ifndef QGSCHUNKLIST_P_H
#define QGSCHUNKLIST_P_H

///@cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

class QgsChunkNode;

/**
 * \ingroup 3d
 * Element of a double-linked list
 * \since QGIS 3.0
 */
struct QgsChunkListEntry
{
  //! Constructs entry for a particular node
  QgsChunkListEntry( QgsChunkNode *node )
    : chunk( node )
  {
  }

  QgsChunkListEntry *prev = nullptr;
  QgsChunkListEntry *next = nullptr;
  QgsChunkNode *chunk;   //!< TODO: shared pointer
};


/**
 * \ingroup 3d
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
class QgsChunkList
{
  public:
    //! Constructor for QgsChunkList
    QgsChunkList() = default;

    //! Counts the real number of entries by walking the list (for debugging purposes only)
    int trueCount() const;
    //! Returns number of entries in the list
    int count() const { return mCount; }

    //! Returns the first entry. Null will be returned if the list is empty.
    QgsChunkListEntry *first() const { return mHead; }
    //! Returns the last entry. Null will be returned if the list is empty.
    QgsChunkListEntry *last() const { return mTail; }
    //! Returns whether the list is empty or it contains some entries
    bool isEmpty() const;

    /**
     * Inserts a new entry before the entry "next".
     * If "next" is null, entry will be inserted at the end of the list
     */
    void insertEntry( QgsChunkListEntry *entry, QgsChunkListEntry *next );

    //! Takes the entry out of the list (does not delete it)
    void takeEntry( QgsChunkListEntry *entry );
    //! Takes the first entry from the list and returns it
    QgsChunkListEntry *takeFirst();
    //! Takes the last entry from the list and returns it
    QgsChunkListEntry *takeLast();

    //! Inserts an entry at the start of the list
    void insertFirst( QgsChunkListEntry *entry );
    //! Inserts an entry at the end of the list
    void insertLast( QgsChunkListEntry *entry );

  private:
    QgsChunkListEntry *mHead = nullptr;
    QgsChunkListEntry *mTail = nullptr;
    int mCount = 0;
};

/// @endcond

#endif // QGSCHUNKLIST_P_H
