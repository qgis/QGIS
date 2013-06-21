/***************************************************************************
    qgsfeatureiterator.h
    ---------------------
    begin                : Juli 2012
    copyright            : (C) 2012 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSFEATUREITERATOR_H
#define QGSFEATUREITERATOR_H

#include "qgsfeaturerequest.h"


/** \ingroup core
 * Internal feature iterator to be implemented within data providers
 */
class CORE_EXPORT QgsAbstractFeatureIterator
{
  public:
    //! base class constructor - stores the iteration parameters
    QgsAbstractFeatureIterator( const QgsFeatureRequest& request );

    //! destructor makes sure that the iterator is closed properly
    virtual ~QgsAbstractFeatureIterator();

    //! fetch next feature, return true on success
    virtual bool nextFeature( QgsFeature& f ) = 0;
    //! reset the iterator to the starting position
    virtual bool rewind() = 0;
    //! end of iterating: free the resources / lock
    virtual bool close() = 0;

  protected:
    QgsFeatureRequest mRequest;

    bool mClosed;

    // reference counting (to allow seamless copying of QgsFeatureIterator instances)
    int refs;
    void ref(); // add reference
    void deref(); // remove reference, delete if refs == 0
    friend class QgsFeatureIterator;
};


/**
 * \ingroup core
 * Wrapper for iterator of features from vector data provider or vector layer
 */
class CORE_EXPORT QgsFeatureIterator
{
  public:
    //! construct invalid iterator
    QgsFeatureIterator();
    //! construct a valid iterator
    QgsFeatureIterator( QgsAbstractFeatureIterator* iter );
    //! copy constructor copies the iterator, increases ref.count
    QgsFeatureIterator( const QgsFeatureIterator& fi );
    //! destructor deletes the iterator if it has no more references
    ~QgsFeatureIterator();

    QgsFeatureIterator& operator=( const QgsFeatureIterator& other );

    bool nextFeature( QgsFeature& f );
    bool rewind();
    bool close();

    //! find out whether the iterator is still valid or closed already
    bool isClosed();

    friend bool operator== ( const QgsFeatureIterator &fi1, const QgsFeatureIterator &fi2 );
    friend bool operator!= ( const QgsFeatureIterator &fi1, const QgsFeatureIterator &fi2 );

  protected:
    QgsAbstractFeatureIterator* mIter;
};

////////

inline QgsFeatureIterator::QgsFeatureIterator()
    : mIter( NULL )
{
}

inline QgsFeatureIterator::QgsFeatureIterator( QgsAbstractFeatureIterator* iter )
    : mIter( iter )
{
  if ( iter )
    iter->ref();
}

inline QgsFeatureIterator::QgsFeatureIterator( const QgsFeatureIterator& fi )
    : mIter( fi.mIter )
{
  if ( mIter )
    mIter->ref();
}

inline QgsFeatureIterator::~QgsFeatureIterator()
{
  if ( mIter )
    mIter->deref();
}

inline bool QgsFeatureIterator::nextFeature( QgsFeature& f )
{
  return mIter ? mIter->nextFeature( f ) : false;
}

inline bool QgsFeatureIterator::rewind()
{
  return mIter ? mIter->rewind() : false;
}

inline bool QgsFeatureIterator::close()
{
  return mIter ? mIter->close() : false;
}

inline bool QgsFeatureIterator::isClosed()
{
  return mIter ? mIter->mClosed : true;
}

inline bool operator== ( const QgsFeatureIterator &fi1, const QgsFeatureIterator &fi2 )
{
  return ( fi1.mIter == fi2.mIter );
}

inline bool operator!= ( const QgsFeatureIterator &fi1, const QgsFeatureIterator &fi2 )
{
  return !( fi1 == fi2 );
}

#endif // QGSFEATUREITERATOR_H
