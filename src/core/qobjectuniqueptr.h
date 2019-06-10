#ifndef QOBJECTUNIQUEPTR_H
#define QOBJECTUNIQUEPTR_H

#include <qsharedpointer.h>
#include <qtypeinfo.h>

class QVariant;

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
    QWeakPointer<QObjectType> wp;
  public:
    inline QObjectUniquePtr() { }
    inline QObjectUniquePtr( T *p ) : wp( p ) { }
    // compiler-generated copy/move ctor/assignment operators are fine!

    ~QObjectUniquePtr() { delete wp.data(); }

    inline void swap( QObjectUniquePtr &other ) { wp.swap( other.wp ); }

    inline QObjectUniquePtr<T> &operator=( T *p )
    { wp.assign( static_cast<QObjectType *>( p ) ); return *this; }

    inline T *data() const
    { return static_cast<T *>( wp.data() ); }
    inline T *operator->() const
    { return data(); }
    inline T &operator*() const
    { return *data(); }
    inline operator T *() const
    { return data(); }

    inline bool isNull() const
    { return wp.isNull(); }

    inline void clear()
    { wp.clear(); }

    T *release() { T *p = qobject_cast<T *>( wp.data() ); wp.clear(); return p; }

    void reset( T *p = nullptr ) { delete wp.data(); wp = p; }
};
template <class T> Q_DECLARE_TYPEINFO_BODY( QObjectUniquePtr<T>, Q_MOVABLE_TYPE );

template <class T>
inline bool operator==( const T *o, const QObjectUniquePtr<T> &p )
{ return o == p.operator->(); }

template<class T>
inline bool operator==( const QObjectUniquePtr<T> &p, const T *o )
{ return p.operator->() == o; }

template <class T>
inline bool operator==( T *o, const QObjectUniquePtr<T> &p )
{ return o == p.operator->(); }

template<class T>
inline bool operator==( const QObjectUniquePtr<T> &p, T *o )
{ return p.operator->() == o; }

template<class T>
inline bool operator==( const QObjectUniquePtr<T> &p1, const QObjectUniquePtr<T> &p2 )
{ return p1.operator->() == p2.operator->(); }

template <class T>
inline bool operator!=( const T *o, const QObjectUniquePtr<T> &p )
{ return o != p.operator->(); }

template<class T>
inline bool operator!= ( const QObjectUniquePtr<T> &p, const T *o )
{ return p.operator->() != o; }

template <class T>
inline bool operator!=( T *o, const QObjectUniquePtr<T> &p )
{ return o != p.operator->(); }

template<class T>
inline bool operator!= ( const QObjectUniquePtr<T> &p, T *o )
{ return p.operator->() != o; }

template<class T>
inline bool operator!= ( const QObjectUniquePtr<T> &p1, const QObjectUniquePtr<T> &p2 )
{ return p1.operator->() != p2.operator->() ; }

template<typename T>
QObjectUniquePtr<T>
QObjectUniquePtrFromVariant( const QVariant &variant )
{
  return QObjectUniquePtr<T>( qobject_cast<T *>( QtSharedPointer::weakPointerFromVariant_internal( variant ).data() ) );
}

#endif // QOBJECTUNIQUEPTR_H
