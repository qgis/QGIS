/*
 *   libpal - Automated Placement of Labels Library
 *
 *   Copyright (C) 2008 Maxence Laurent, MIS-TIC, HEIG-VD
 *                      University of Applied Sciences, Western Switzerland
 *                      http://www.hes-so.ch
 *
 *   Contact:
 *      maxence.laurent <at> heig-vd <dot> ch
 *    or
 *      eric.taillard <at> heig-vd <dot> ch
 *
 * This file is part of libpal.
 *
 * libpal is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libpal is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libpal.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _HASHTABLE_HPP_
#define _HASHTABLE_HPP_

#include <cassert>
#include <cstdlib>
#include <cstring>

#include "linkedlist.hpp"

namespace pal
{

// Each hash entry stores a key, object pair
  template <typename Data>
  struct HashTableEntry
  {
    char * _key;
    Data _data;
  };


// This is a Hash table that maps string keys to objects of type Data
  template <typename Data>
  class HashTable
  {
    public:
      int tableSize;

      LinkedList<HashTableEntry<Data>*> **_hashtable;

      unsigned long hash( const char * key );

    public:
      HashTable( int tableSize );
      ~HashTable();
      bool insertItem( const char * key, Data data );
      Data* find( const char * key );
      bool removeElement( const char * key );

      void printStat();
  };

  template <typename Data> HashTable<Data>::~HashTable()
  {
    int i;
    LinkedList<HashTableEntry<Data>*> *list;
    HashTableEntry <Data> *entry;

    for ( i = 0; i < tableSize; i++ )
    {
      list = _hashtable[i];
      if ( list )
      {
        while ( list->size() > 0 )
        {
          entry = list->pop_front();
          delete[] entry->_key;
          delete entry;
        }
        delete list;
      }
    }
    delete[] _hashtable;

  }
  template <typename Data>
  unsigned long HashTable<Data>::hash( const char * key )
  {
    unsigned long hash = 5381;
    int c;

    while (( c = *key++ ) )
    {
      hash = (( hash << 5 ) + hash ) + c; // hash*33 + c
    }

    return hash % tableSize;
  }


  template <typename Data>
  HashTable<Data>::HashTable( int tableSize ) : tableSize( tableSize )
  {
    // TODO: Initialize the hash table
    _hashtable = new LinkedList<HashTableEntry<Data>*>*[tableSize];


    for ( int i = 0; i < tableSize; i++ )
    {
      _hashtable[i] = NULL;
    }
  }

  template <class Data>
  bool hashEntryCompare( HashTableEntry<Data> *a, HashTableEntry<Data> *b )
  {
    return strcmp( a->_key, b->_key ) == 0;
  }

  template <typename Data>
  bool HashTable<Data>::insertItem( const char * key, Data data )
  {
    unsigned long i = hash( key );

    LinkedList<HashTableEntry<Data>*> *e = _hashtable[i];

    HashTableEntry<Data> *entry = new HashTableEntry<Data>();
    entry->_key = new char[strlen( key ) +1];
    strcpy( entry->_key, key );
    entry->_data = data;

    if ( e )
    {
      Cell<HashTableEntry<Data>*> *elem = e->search( entry );
      if ( elem ) // change data
      {
        elem->item->_data = data;
        delete[] entry->_key;
        delete entry;
      }
      else
      {
        e->push_back( entry );
        return true;
      }
    }
    else
    {
      e = _hashtable[i] = new LinkedList<HashTableEntry<Data>*> ( hashEntryCompare );
      e->push_back( entry );
      return true;
    }


    return false;
  }


  template <typename Data>
  Data* HashTable<Data>::find( const char * key )
  {
    unsigned long i = hash( key );

    HashTableEntry<Data> * entry = new HashTableEntry<Data> ();
    entry->_key = new char[strlen( key ) +1];
    strcpy( entry->_key, key );

    LinkedList<HashTableEntry<Data>*> *e = _hashtable[i];

    Cell<HashTableEntry<Data>*> *cell;

    if ( !e )
    {
      delete[] entry->_key;
      delete entry;
      return NULL;
    }
    else
    {
      if (( cell = e->search( entry ) ) == NULL )
      {
        delete[] entry->_key;
        delete entry;
        return NULL;
      }
      else
      {
        delete[] entry->_key;
        delete entry;
        return & ( cell->item->_data );
      }
    }
  }

  template<typename Data>
  void HashTable<Data>::printStat()
  {

    double c = 0;
    int i;
    int n = 0;

    for ( i = 0; i < tableSize; i++ )
    {
      if ( _hashtable[i] )
      {
        n++;
        c += _hashtable[i]->size();
      }
    }

    std::cout << "# elem: " << c << std::endl;
    std::cout << "# linked list : " << n << " / " << tableSize << std::endl;
    std::cout << "nb elem / real list" << c / n << std::endl;
    std::cout << "nb elem / tableSize" << c / tableSize << std::endl;
  }


  template <typename Data>
  bool HashTable<Data>::removeElement( const char * )
  {
    // TODO: Remove the element that has this key from the hash table.
    // Return true if the entry is found or false otherwise.
    /*int i = hash(key);
    HashTableEntry<Data> *eC = _buckets[i];
    HashTableEntry<Data> *eP = NULL;

    while(eC != NULL && strcmp(key, eC->_key) != 0)
    {
      eP = eC;
      eC = eC->_next;
    }

    //key is found
    if(eC != NULL)
    {
      if(eP != NULL)
        eP->_next = eC->_next;
      else
        _buckets[i] = eC->_next;
      return true;
    }

    //key is not found*/
    return false;
  }


/////////////////////////////////////
//
// Class used to iterate over the hash table
//
/////////////////////////////////////
  /*template <typename Data>
  class HashTableIterator {
    int _currentBucket; // Current bucket that is being iterated
    HashTableEntry<Data> *_currentEntry; // Current entry iterated
    HashTable<Data> * _hashTable;  // Pointer to the hash table being iterated
  public:
    HashTableIterator(HashTable<Data> * hashTable);
    bool next(const char * & key, Data & data);
  };


  template <typename Data>
  HashTableIterator<Data>::HashTableIterator(HashTable<Data> * hashTable)
  {
    // TODO: Initialize iterator. "hashTable" is the table to be iterated.
     _hashTable =  hashTable;
  }

  template <typename Data>
  bool HashTableIterator<Data>::next(const char * & key, Data & data)
  {
    // TODO: Returns the next element in the hash table.
    // The key and data values are stored in the references passed
    // as argument. It returns true if there are more entries or
    // false otherwise.
    return false;
  }

  */

} // end namespace

#endif

