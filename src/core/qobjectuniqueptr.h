/***************************************************************************
                                  qobjectuniqueptr.h

                      A unique pointer to a QObject.

                              -------------------
  begin                : June 2019
  copyright            : (C) 2009 by Matthias Kuhn
  email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QOBJECTUNIQUEPTR_H
#define QOBJECTUNIQUEPTR_H

#define SIP_NO_FILE

#include <qsharedpointer.h>
#include <qtypeinfo.h>

class QVariant;

/**
 * Keeps a pointer to a QObject and deletes it whenever this object is deleted.
 * It keeps a weak pointer to the QObject internally and will be set to ``nullptr``
 * whenever the QObject is deleted.
 *
 * \ingroup core
 * \since QGIS 3.8
 */
template <class T>
class QObjectUniquePtr
{
    Q_STATIC_ASSERT_X( !std::is_pointer<T>::value, "QObjectUniquePtr's template type must not be a pointer type" );

    template<typename U>
    struct TypeSelector
    {
      typedef QObject Type;
    };
    template<typename U>
    struct TypeSelector<const U>
    {
      typedef const QObject Type;
    };
    typedef typename TypeSelector<T>::Type QObjectType;
    QWeakPointer<QObjectType> mPtr;
  public:

    /**
     * Creates a new empty QObjectUniquePtr.
     */
    inline QObjectUniquePtr()
    { }

    /**
     * Takes a new QObjectUniquePtr and assigned \a p to it.
     */
    inline QObjectUniquePtr( T *p ) : mPtr( p )
    { }
    // compiler-generated copy/move ctor/assignment operators are fine!

    /**
     * Will delete the contained QObject if it still exists.
     */
    ~QObjectUniquePtr()
    {
      // Will be a nullptr if the QObject has been deleted from somewhere else (e.g. through parent ownership)
      delete mPtr.data();
    }

    /**
     * Swaps the pointer managed by this instance with the pointer managed by \a other.
     */
    inline void swap( QObjectUniquePtr &other )
    {
      mPtr.swap( other.mPtr );
    }

    inline QObjectUniquePtr<T> &operator=( T *p )
    {
      mPtr.assign( static_cast<QObjectType *>( p ) );
      return *this;
    }

    /**
     * Returns the raw pointer to the managed QObject.
     */
    inline T *data() const
    {
      return static_cast<T *>( mPtr.data() );
    }

    /**
     * Returns the raw pointer to the managed QObject.
     */
    inline T *get() const
    {
      return static_cast<T *>( mPtr.data() );
    }

    /**
     * Returns a raw pointer to the managed QObject.
     */
    inline T *operator->() const
    {
      return data();
    }

    /**
     * Dereferences the managed QObject.
     */
    inline T &operator*() const
    {
      return *data();
    }

    /**
     * Const getter for the managed raw pointer.
     */
    inline operator T *() const
    {
      return data();
    }

    /**
     * Checks if the managed pointer is ``nullptr``.
     */
    inline bool isNull() const
    {
      return mPtr.isNull();
    }

    /**
     * Checks if the pointer managed by this object is ``nullptr``.
     * If it is not ``nullptr`` TRUE will be returned, if it is ``nullptr``
     * FALSE will be returned.
     */
    inline operator bool() const
    {
      return !mPtr.isNull();
    }

    /**
     * Clears the pointer. The managed object is set to ``nullptr`` and will not be deleted.
     */
    inline void clear()
    {
      mPtr.clear();
    }

    /**
     * Clears the pointer and returns it. The managed object will not be deleted and it is the callers
     * responsibility to guarantee that no memory is leaked.
     */
    inline T *release()
    {
      T *p = qobject_cast<T *>( mPtr.data() );
      mPtr.clear();
      return p;
    }

    /**
     * Will reset the managed pointer to ``p``. If there is already a QObject managed currently
     * it will be deleted. If ``p`` is not specified the managed QObject will be deleted and
     * this object reset to ``nullptr``.
     */
    void reset( T *p = nullptr )
    {
      delete mPtr.data();
      mPtr = p;
    }
};
template <class T> Q_DECLARE_TYPEINFO_BODY( QObjectUniquePtr<T>, Q_MOVABLE_TYPE );

template <class T>
inline bool operator==( const T *o, const QObjectUniquePtr<T> &p )
{
  return o == p.operator->();
}

template<class T>
inline bool operator==( const QObjectUniquePtr<T> &p, const T *o )
{
  return p.operator->() == o;
}

template <class T>
inline bool operator==( T *o, const QObjectUniquePtr<T> &p )
{
  return o == p.operator->();
}

template<class T>
inline bool operator==( const QObjectUniquePtr<T> &p, T *o )
{
  return p.operator->() == o;
}

template<class T>
inline bool operator==( const QObjectUniquePtr<T> &p1, const QObjectUniquePtr<T> &p2 )
{
  return p1.operator->() == p2.operator->();
}

template <class T>
inline bool operator!=( const T *o, const QObjectUniquePtr<T> &p )
{
  return o != p.operator->();
}

template<class T>
inline bool operator!= ( const QObjectUniquePtr<T> &p, const T *o )
{
  return p.operator->() != o;
}

template <class T>
inline bool operator!=( T *o, const QObjectUniquePtr<T> &p )
{
  return o != p.operator->();
}

template<class T>
inline bool operator!= ( const QObjectUniquePtr<T> &p, T *o )
{
  return p.operator->() != o;
}

template<class T>
inline bool operator!= ( const QObjectUniquePtr<T> &p1, const QObjectUniquePtr<T> &p2 )
{
  return p1.operator->() != p2.operator->() ;
}

template<typename T>
QObjectUniquePtr<T>
QObjectUniquePtrFromVariant( const QVariant &variant )
{
  return QObjectUniquePtr<T>( qobject_cast<T *>( QtSharedPointer::weakPointerFromVariant_internal( variant ).data() ) );
}

#endif // QOBJECTUNIQUEPTR_H
